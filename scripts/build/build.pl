use strict;
use warnings;
use Cwd qw(cwd realpath);
use lib cwd();
use Getopt::Long qw(GetOptions);
use File::Copy qw(copy move);
use File::Path qw(make_path remove_tree);
use Pod::Usage;
use Capture::Tiny qw(capture);
use File::Basename qw(dirname basename);
use File::Temp qw(tempdir);
use Archive::Tar;
use POSIX;

my $debug_mode = 0;

#-- Main
{
	my %args = (
	'prefix'				=> cwd(),
	'dyninst-src'			=> undef,
	'test-src'				=> undef,
	'boost-dir'				=> undef,
	'log-file'      		=> undef,
	'njobs' 				=> 1,
	'run-tests'				=> 1,
	'quiet'					=> 0,
	'purge'					=> 0,
	'help' 					=> 0,
	'debug-mode'			=> 0	# undocumented debug mode
	);

	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'test-src=s',
		'boost-dir=s', 'log-file=s', 'njobs=i',
		'run-tests', 'quiet', 'purge', 'help',
		'debug-mode'
	) or pod2usage(-exitval=>2);

	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99);
	}

	$debug_mode = $args{'debug-mode'};

	# Default directory and file locations
	$args{'dyninst-src'} //= "$args{'prefix'}/dyninst";
	$args{'test-src'} //= "$args{'prefix'}/testsuite";
	$args{'log-file'} //= "$args{'prefix'}/build.log";

	# Canonicalize user-specified files and directories
	for my $d ('dyninst-src','test-src','log-file') {
		# NB: realpath(undef) eq cwd()
		$args{$d} = realpath($args{$d}) if defined($args{$d});
	}

	# Save a backup, if the log file already exists
	move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};

	open my $fdLog, '>', $args{'log-file'} or die "$args{'log-file'}: $!\n";

	my $root_dir;

	eval {
		# Save some information about the system
		my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
		print_log($fdLog, !$args{'quiet'},
			"os: $sysname\n" .
			"hostname: $nodename\n" .
			"kernel: $release\n" .
			"version: $version\n" .
			"arch: $machine\n" .
			'*'x20 . "\n"
		);

		# Generate a unique name for the current build
		$root_dir = tempdir('XXXXXXXX', CLEANUP=>0);

		# Build Dyninst
		{
			# Create the build directory
			make_path("$root_dir/dyninst/build");

			# The path must exist before using 'realpath'
			my $base_dir = realpath("$root_dir/dyninst");
			my $build_dir = "$base_dir/build";

			symlink($args{'dyninst-src'}, "$base_dir/src");

			print_log($fdLog, !$args{'quiet'}, "Configuring Dyninst($root_dir)... ");
			&configure_dyninst(\%args, $base_dir, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done.\n");

			my $cmake_cache = &parse_cmake_cache("$build_dir/CMakeCache.txt");
			$args{'boost-lib'} = $cmake_cache->{'Boost_LIBRARY_DIR_RELEASE'};

			print_log($fdLog, !$args{'quiet'}, "Building Dyninst... ");
			&build_dyninst(\%args, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done.\n");

			# Symlinking libdw is broken in the config system right now
			# See https://github.com/dyninst/dyninst/issues/547
			my $libdwarf_dir = $cmake_cache->{'LIBDWARF_LIBRARIES'};
			if($libdwarf_dir =~ /NOTFOUND/ || $nodename =~ /leela/i) {
				$libdwarf_dir = "$build_dir/elfutils/lib";
			} else {
				$libdwarf_dir = dirname($libdwarf_dir);
			}
			symlink("$libdwarf_dir/libdw.so", "$base_dir/lib/libdw.so");
			symlink("$libdwarf_dir/libdw.so.1", "$base_dir/lib/libdw.so.1");
		}

		# Build the test suite
		{
			# Create the build directory
			make_path("$root_dir/testsuite/build");

			my $base_dir = realpath("$root_dir/testsuite");
			my $build_dir = "$base_dir/build";
			symlink($args{'test-src'}, "$base_dir/src");
			symlink(realpath("$root_dir/dyninst"), "$base_dir/dyninst");

			print_log($fdLog, !$args{'quiet'}, "Configuring Testsuite... ");
			&configure_tests(\%args, $base_dir, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done\n");

			print_log($fdLog, !$args{'quiet'}, "Building Testsuite... ");
			&build_tests(\%args, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done\n");
		}

		# Run the tests
		if($args{'run-tests'}) {
			make_path("$root_dir/testsuite/tests");
			my $base_dir = realpath("$root_dir/testsuite/tests");

			print_log($fdLog, !$args{'quiet'}, "running Testsuite... ");
			my $max_attempts = 3;
			while(!&run_tests(\%args, $base_dir)) {
				$max_attempts--;
				if($max_attempts <= 0) {
					die "Maximum number of Testsuite retries exceeded\n";
				}
				print_log($fdLog, !$args{'quiet'}, "\nTestsuite killed; restarting... ");
			}
			print_log($fdLog, !$args{'quiet'}, "done.\n");
		}
	};
	if($@) {
		print_log($fdLog, !$args{'quiet'}, $@);
		open my $fdOut, '>', "$root_dir/FAILED";
	}

	# Create the exportable tarball of results
	my @log_files = (
		File::Spec->abs2rel($args{'log-file'}),
		"$root_dir/FAILED",
		"$root_dir/dyninst/git.log",
		"$root_dir/dyninst/build/config.out",
		"$root_dir/dyninst/build/config.err",
		"$root_dir/dyninst/build/build.out",
		"$root_dir/dyninst/build/build.err",
		"$root_dir/dyninst/build/build-install.out",
		"$root_dir/dyninst/build/build-install.err",
		"$root_dir/testsuite/git.log",
		"$root_dir/testsuite/build/config.out",
		"$root_dir/testsuite/build/config.err",
		"$root_dir/testsuite/build/build.out",
		"$root_dir/testsuite/build/build.err",
		"$root_dir/testsuite/build/build-install.out",
		"$root_dir/testsuite/build/build-install.err",
		"$root_dir/testsuite/tests/stdout.log",
		"$root_dir/testsuite/tests/stderr.log",
		"$root_dir/testsuite/tests/test.log"
	);
	my $tar = Archive::Tar->new();

	# Only add the files that exist
	# Non-existent files indicate an error occurred
	$tar->add_files(grep {-f $_ } @log_files);
	$tar->write('results.tar.gz', COMPRESS_GZIP);

	# Remove the generated files, if requested
	if($args{'purge'}) {
		remove_tree($root_dir);
	}
}

sub print_log {
	my ($fd, $echo_stdout, $msg) = @_;

	print $fd $msg;

	if($echo_stdout) {
		print $msg;
	}
}
sub parse_cmake_cache {
	my $filename = shift;
	my %defines = ();

	open my $fdIn, '<', $filename or die "Unable to open $filename: $!\n";
	while(<$fdIn>) {
		chomp;
		next if /^#/;
		next if /^\/\//;
		next if $_ eq '';

		# Format is KEY:TYPE=VALUE
		my ($key, $value) = split('=');
		($key, undef) = split('\:', $key);
		$defines{$key} = $value;
	}
	return \%defines;
}

sub configure_dyninst {
	my ($args, $base_dir, $build_dir) = @_;

	my $src_dir = $args->{'dyninst-src'};

	# Save the git configuration
	{
		# Fetch the current branch name
		# NB: This will return 'HEAD' if in a detached-head state
		my $branch = execute("git -C $src_dir rev-parse --abbrev-ref HEAD");

		# Fetch the commitID for HEAD
		my $commit_head = execute("git -C $src_dir rev-parse HEAD");

		open my $fdOut, '>', "$base_dir/git.log" or die "$base_dir/git.log: $!";
		local $, = "\n";
		local $\ = "\n";
		print $fdOut "branch: $branch",
					 "commit: $commit_head";
	}

	my $path_boost = $args->{'boost-dir'} // '';

	# Configure the build
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"cmake -H$base_dir/src -B$build_dir " .
			"-DPATH_BOOST=$path_boost " .
			"-DCMAKE_INSTALL_PREFIX=$base_dir " .
			"-DUSE_GNU_DEMANGLER:BOOL=ON " .
			"1>config.out 2>config.err "
		);
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
}
sub build_dyninst {
	my ($args, $build_dir) = @_;

	my $boost_lib = $args->{'boost-lib'};
	my $njobs = $args->{'njobs'};

	# Run the build
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"make -j$njobs 1>build.out 2>build.err"
		);
	};
	die "Error building: see $build_dir/build.err for details" if $@;

	# Install
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"make install 1>build-install.out 2>build-install.err"
		);
	};
	die "Error installing: see $build_dir/build-install.err for details" if $@;
}

sub configure_tests {
	my ($args, $base_dir, $build_dir) = @_;

	my $src_dir = $args->{'test-src'};

	# Save the git configuration
	{
		# Fetch the current branch name
		# NB: This will return 'HEAD' if in a detached-head state
		my $branch = execute("git -C $src_dir rev-parse --abbrev-ref HEAD");

		# Fetch the commitID for HEAD
		my $commit_head = execute("git -C $src_dir rev-parse HEAD");

		open my $fdOut, '>', "$base_dir/git.log" or die "$base_dir/git.log: $!";
		local $, = "\n";
		local $\ = "\n";
		print $fdOut "branch: $branch",
					 "commit: $commit_head";
	}

	# Configure the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"cmake ../src -DCMAKE_INSTALL_PREFIX=$base_dir " .
			"-DINSTALL_DIR=$base_dir/tests ".
			"-DDyninst_DIR=../dyninst/lib/cmake/Dyninst ".
			"1>config.out 2>config.err"
		);
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
}
sub build_tests {
	my ($args, $build_dir) = @_;

	my $boost_lib = $args->{'boost-lib'};
	my $njobs = $args->{'njobs'};

	# Build the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"make -j$njobs 1>build.out 2>build.err"
		);
	};
	die "Error building: see $build_dir/build.err for details" if $@;

	# Install the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
		"cd $build_dir\n" .
		"make install 1>build-install.out 2>build-install.err"
		);
	};
	die "Error installing: see $build_dir/build-install.err for details" if $@;
}

sub run_tests {
	my ($args, $base_dir) = @_;

	my $boost_lib = $args->{'boost-lib'};

	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $base_dir\n" .
			"export DYNINSTAPI_RT_LIB=$base_dir/../dyninst/lib/libdyninstAPI_RT.so\n".
			"LD_LIBRARY_PATH=$base_dir:$base_dir/../dyninst/lib:$boost_lib " .
			"./runTests -all -log test.log 1>stdout.log 2>stderr.log"
		);
	};

	# Check if we were killed by the watchdog timer
	open my $fdIn, '<', "$base_dir/stderr.log";
	while(<$fdIn>) {
		return !!0 if m/Process exceeded time limit/;
	}
	return !!1;
}

sub execute($) {
	my $cmd = shift;

	print "$cmd\n" if $debug_mode;

	my ($stdout,$stderr,$exit) = capture { system($cmd); };
	$exit = (( $exit >> 8 ) != 0 || $exit == -1 || ( $exit & 127 ) != 0);
	die "Error executing '$cmd'\n$stderr\n" if $exit;
	return $stdout;
}

sub parse_log {
	open my $fdIn, '<', $_[0] or die "$_[0]: $!\n";
	my @results;
	while(<$fdIn>) {
		chomp;
		my @x = unpack('a27 a7 a5 a4 a9 a8 a8 a8 a23');
		my $status = pop @x;
		$status = (split(' ', $status))[0];
		push @x, $status;
		push @results, map {s/\s+//g; $_;} @x;
	}
	return @results;
}
__END__

=head1 DESCRIPTION

A tool for automating building Dyninst and its test suite

=head1 SYNOPSIS

build [options]

 Options:
   --prefix                Base directory for the source and build directories (default: pwd)
   --dyninst-src=PATH      Source directory for Dyninst (default: prefix/dyninst)
   --test-src=PATH         Source directory for Testsuite (default: prefix/testsuite)
   --boost-dir=PATH        Base directory for Boost
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --njobs=N               Number of make jobs (default: N=1)
   --[no-]run-tests        Run the Testsuite after building it (default: yes)
   --quiet                 Don't echo logging information to stdout (default: no)
   --purge                 Remove all files after running testsuite (default: no)
   --help                  Print this help message
=cut