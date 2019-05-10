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
use POSIX;

my $debug_mode = 0;

#-- Main
{
	my %args = (
	'prefix'				=> cwd(),
	'dyninst-src'			=> undef,
	'test-src'				=> undef,
	'boost-dir'				=> '',
	'elfutils-dir'			=> '',
	'tbb-dir'				=> '',
	'log-file'      		=> undef,
	'dyninst-pr'			=> undef,
	'testsuite-pr'			=> undef,
	'dyninst-cmake-args'	=> undef,
	'testsuite-cmake-args'	=> undef,
	'njobs' 				=> 1,
	'quiet'					=> 0,
	'purge'					=> 0,
	'help' 					=> 0,
	'debug-mode'			=> 0	# undocumented debug mode
	);

	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'test-src=s',
		'boost-dir=s', 'elfutils-dir=s', 'tbb-dir=s',
		'log-file=s', 'dyninst-pr=s', 'testsuite-pr=s',
		'dyninst-cmake-args=s', 'testsuite-cmake-args=s',
		'njobs=i', 'quiet', 'purge', 'help', 'debug-mode'
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
	for my $d ('dyninst-src','test-src','log-file',
				'boost-dir', 'elfutils-dir', 'tbb-dir')
	{
		# NB: realpath(undef|'') eq cwd()
		$args{$d} = realpath($args{$d}) if defined($args{$d}) && $args{$d} ne '';
	}

	# Save a backup, if the log file already exists
	move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};

	open my $fdLog, '>', $args{'log-file'} or die "$args{'log-file'}: $!\n";

	my $root_dir;

	eval {
		if($debug_mode) {
			use Data::Dumper;
			print Dumper(\%args), "\n";
		}
		
		# Save some information about the system
		my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
		print_log($fdLog, !$args{'quiet'},
			"os: $sysname\n" .
			"hostname: $nodename\n" .
			"kernel: $release\n" .
			"version: $version\n" .
			"arch: $machine\n"
		);
		
		# Find and save the version of libc
		my $libc_info = (split("\n", &execute('ldd --version')))[0];
		if($libc_info =~ /gnu/i || $libc_info =~ /glibc/i) {
			# We have a GNU libc, the version is at the end
			$libc_info = (split(' ', $libc_info))[-1];
		} else {
			$libc_info = "Unknown";
		}
		print_log($fdLog, !$args{'quiet'}, "libc: $libc_info\n");
		print_log($fdLog, !$args{'quiet'}, '*'x20 . "\n");

		# Generate a unique name for the current build
		$root_dir = tempdir('XXXXXXXX', CLEANUP=>0);
		print_log($fdLog, !$args{'quiet'}, "root_dir: $root_dir\n");

		# Build Dyninst
		{
			# Create the build directory
			make_path("$root_dir/dyninst/build");

			# The path must exist before using 'realpath'
			my $base_dir = realpath("$root_dir/dyninst");
			my $build_dir = "$base_dir/build";
			
			# This is for internal use only
			$args{'cmake-cache-dir'} = $build_dir;

			symlink($args{'dyninst-src'}, "$base_dir/src");
			
			my $git_config = get_git_config($args{'dyninst-src'}, $base_dir);
			
			# Check out the PR, if specified
			if($args{'dyninst-pr'}) {
				&checkout_pr($args{'dyninst-src'}, $args{'dyninst-pr'}, $git_config->{'branch'});
				$git_config = get_git_config($args{'dyninst-src'}, $base_dir);
			}
			
			save_git_config($base_dir, $git_config->{'branch'},$git_config->{'commit'});

			print_log($fdLog, !$args{'quiet'}, "Configuring Dyninst... ");
			&configure_dyninst(\%args, $base_dir, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done.\n");

			print_log($fdLog, !$args{'quiet'}, "Building Dyninst... ");
			&build_dyninst(\%args, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done.\n");
		}

		# Build the test suite
		{
			# Create the build directory
			make_path("$root_dir/testsuite/build");

			my $base_dir = realpath("$root_dir/testsuite");
			my $build_dir = "$base_dir/build";
			symlink($args{'test-src'}, "$base_dir/src");
			symlink(realpath("$root_dir/dyninst"), "$base_dir/dyninst");
			
			my $git_config = get_git_config($args{'test-src'}, $base_dir);
			
			# Check out the PR, if specified
			if($args{'testsuite-pr'}) {
				&checkout_pr($args{'test-src'}, $args{'testsuite-pr'}, $git_config->{'branch'});
				$git_config = get_git_config($args{'test-src'}, $base_dir);
			}
			
			save_git_config($base_dir, $git_config->{'branch'},$git_config->{'commit'});

			print_log($fdLog, !$args{'quiet'}, "Configuring Testsuite... ");
			&configure_tests(\%args, $base_dir, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done\n");

			print_log($fdLog, !$args{'quiet'}, "Building Testsuite... ");
			&build_tests(\%args, $build_dir);
			print_log($fdLog, !$args{'quiet'}, "done\n");
		}

		# Run the tests
		{
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
	
	my $results_log = "$root_dir/testsuite/tests/results.log";
	# Parse the raw output
	if(-f "$root_dir/testsuite/tests/stdout.log") {
		my @res = &parse_log("$root_dir/testsuite/tests/stdout.log");
		open my $fdOut, '>', $results_log or die "$results_log: $!\n";
		$\ = "\n";
		print $fdOut @res;
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
		"$root_dir/testsuite/tests/test.log",
		$results_log
	);

	# Only add the files that exist
	# Non-existent files indicate an error occurred
	my $files = join(' ', grep {-f $_ } @log_files);
	&execute("tar -zcf $root_dir.results.tar.gz $files");

	# Remove the generated files, if requested
	if($args{'purge'}) {
		remove_tree($root_dir);
	}
}

sub get_git_config {
	my ($src_dir, $base_dir) = @_;
	
	# Fetch the current branch name
	# NB: This will return 'HEAD' if in a detached-head state
	my $branch = execute("cd $src_dir && git rev-parse --abbrev-ref HEAD");
	chomp($branch);

	# Fetch the commitID for HEAD
	my $commit = execute("cd $src_dir && git rev-parse HEAD");
	chomp($commit);

	return {'branch'=>$branch, 'commit'=>$commit};
}
sub save_git_config {
	my ($base_dir, $branch, $commit) = @_;
	
	open my $fdOut, '>', "$base_dir/git.log" or die "$base_dir/git.log: $!";
	local $, = "\n";
	local $\ = "\n";
	print $fdOut "branch: $branch",
				 "commit: $commit";
	
}

sub checkout_pr {
	my ($src_dir, $pr, $current_branch) = @_;
	
	# The PR format is 'remote/ID'
	my ($remote, $id) = split('/', $pr);
	if(!defined($id)) {
		# The user only specified an ID, so assume the remote is 'origin'
		$id = $remote;
		$remote = 'origin';
	}
	
	# The branch we want to create for the PR
	my $target_branch = "PR$id";
	
	eval {
		if($target_branch eq $current_branch) {
			# Just pull any changes from the remote
			&execute(
				"cd $src_dir \n" .
				"git pull $remote pull/$id/head \n"
			);
		} else {
			# Check if the target branch exists
			my $target_exists = undef;
			eval{ &execute("cd $src_dir && git checkout $target_branch"); };
			$target_exists = 1 unless $@;
			
			if($target_exists) {
				# Do a checkout/pull
				&execute(
					"cd $src_dir \n" .
					"git checkout $target_branch \n" .
					"git pull $remote pull/$id/head \n"
				);
			} else {
				# Do a fetch/checkout
				&execute(
					"cd $src_dir \n" .
					"git fetch $remote pull/$id/head:$target_branch \n" .
					"git checkout $target_branch \n" .
					"git merge --squash -Xignore-all-space $remote/master \n" .
					"git commit -m 'Merge $remote/master' \n"
				);
			}
		}
	};
	if($@) {
		my $msg = $@;
		$msg =~ s/\n/\n\t/g;
		die "\nERROR: Unable to checkout pull request '$remote/$id'\n\n\t$msg\n";
	}
}

sub print_log {
	my ($fd, $echo_stdout, $msg) = @_;

	print $fd $msg;

	if($echo_stdout) {
		print $msg;
	}
}

sub configure_dyninst {
	my ($args, $base_dir, $build_dir) = @_;
	
	my $extra_args = $args->{'dyninst-cmake-args'} // '';

	# Configure the build
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"cmake -H$base_dir/src -B$build_dir " .
			"-DElfUtils_ROOT_DIR=$args->{'elfutils-dir'} " .
			"-DTBB_ROOT_DIR=$args->{'tbb-dir'} " .
			"-DBoost_ROOT_DIR=$args->{'boost-dir'} " .
			"-DCMAKE_INSTALL_PREFIX=$base_dir " .
			"-DUSE_GNU_DEMANGLER:BOOL=ON " .
			"$extra_args " .
			"1>config.out 2>config.err "
		);
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
}
sub build_dyninst {
	my ($args, $build_dir) = @_;

	my $njobs = $args->{'njobs'};

	# Run the build
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"make VERBOSE=1 -j$njobs 1>build.out 2>build.err"
		);
	};
	die "Error building: see $build_dir/build.err for details" if $@;

	# Install
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"make VERBOSE=1 install 1>build-install.out 2>build-install.err"
		);
	};
	die "Error installing: see $build_dir/build-install.err for details" if $@;
}

sub configure_tests {
	my ($args, $base_dir, $build_dir) = @_;
	
	my $extra_args = $args->{'testsuite-cmake-args'} // '';

	# Configure the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"cmake ../src -DCMAKE_INSTALL_PREFIX=$base_dir " .
			"-DINSTALL_DIR=$base_dir/tests ".
			"-DDyninst_DIR=../dyninst/lib/cmake/Dyninst ".
			"$extra_args " .
			"1>config.out 2>config.err"
		);
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
}
sub build_tests {
	my ($args, $build_dir) = @_;

	my $njobs = $args->{'njobs'};

	# Build the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"make VERBOSE=1 -j$njobs 1>build.out 2>build.err"
		);
	};
	die "Error building: see $build_dir/build.err for details" if $@;

	# Install the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
		"cd $build_dir\n" .
		"make VERBOSE=1 install 1>build-install.out 2>build-install.err"
		);
	};
	die "Error installing: see $build_dir/build-install.err for details" if $@;
}
sub run_tests {
	my ($args, $base_dir) = @_;

	# Construct LD_LIBRARY_PATH from the paths in the Dyninst build cache	
	my $cmake_cache = &parse_cmake_cache("$args->{'cmake-cache-dir'}/CMakeCache.txt");
	my $libs = '';
	for my $l ('Boost_LIBRARY_DIRS','TBB_LIBRARY_DIRS','ElfUtils_LIBRARY_DIRS') {
		$cmake_cache->{$l} =~ s/;/\:/g;
		$libs .= ':' . $cmake_cache->{$l};
	}
	my $paths = join(':',
		$base_dir,
		realpath("$base_dir/../dyninst/lib"),
		list_unique(split(':', $libs))
	);

	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $base_dir\n" .
			"export DYNINSTAPI_RT_LIB=$base_dir/../dyninst/lib/libdyninstAPI_RT.so\n".
			"LD_LIBRARY_PATH=$paths:\$LD_LIBRARY_PATH " .
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

	print "\n$cmd\n" if $debug_mode;

	my ($stdout,$stderr,$exit) = capture { system($cmd); };
	$exit = (( $exit >> 8 ) != 0 || $exit == -1 || ( $exit & 127 ) != 0);
	die "Error executing '$cmd'\n$stderr\n" if $exit;
	return $stdout;
}

sub parse_cmake_cache {
	my $filename = shift;
	my %defines = ();

	open my $fdIn, '<', $filename or die "Unable to open CMake cache '$filename': $!\n";
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

sub list_unique {
	my %y;
	@y{@_} = 1;
	return keys %y;
}

sub parse_log {
	open my $fdIn, '<', $_[0] or die "$_[0]: $!\n";
	my @results;
	while(<$fdIn>) {
		chomp;
		
		# Parse the fixed-width format
		my @x = unpack('a27 a7 a5 a4 a9 a8 a8 a8 a23');
		
		# Grab the status field (it's at the end)
		my $status = pop @x;
		if ($status =~ /FAILED\s*\((.+)\)\s*$/) {
			# Split the status from the reason
			# The format is 'FAILED (reason)'
			$status = "FAILED,$1";
		} else {
			# Add an empty field to the CSV since there
			# isn't a failure reason here
			$status .= ',';
		}
		# Strip the extra whitespace (except for the failure reason)
		@x = map {s/\s+//g; $_;} @x;
		
		# Add the failure status back to the record
		push @x, $status;
		
		# Make it a CSV record
		push @results, join(',', @x), "\n";
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
   --elfutils-dir=PATH     Base directory for libelf/libdwarf
   --tbb-dir=PATH          Base directory for Intel's Threading Building Blocks
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --dyninst-pr            The Dyninst pull request formatted as 'remote/ID' with 'remote' being optional
   --testsuite-pr          The Testsuite pull request formatted as 'remote/ID' with 'remote' being optional
   --dyninst-cmake-args    Additional CMake arguments for Dyninst
   --testsuite-cmake-args  Additional CMake arguments for the Testsuite
   --njobs=N               Number of make jobs (default: N=1)
   --quiet                 Don't echo logging information to stdout (default: no)
   --purge                 Remove all files after running testsuite (default: no)
   --help                  Print this help message
=cut
