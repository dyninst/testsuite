use strict;
use warnings;
use Cwd qw(cwd realpath);
use lib cwd();
use Getopt::Long qw(GetOptions);
use File::Copy qw(copy move);
use File::Path qw(make_path);
use Digest::MD5 qw(md5_base64);
use Pod::Usage;
use Capture::Tiny qw(capture);
use File::Basename qw(dirname);

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
	'help' 					=> 0
	);

	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'test-src=s',
		'boost-dir=s', 'log-file=s', 'njobs=i',
		'run-tests', 'help'
	) or (pod2usage(2), exit);

	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99) and exit;
	}

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

	# Generate a unique name for the current build
	my $hash = md5_base64(localtime());
	$hash =~ s|/|_|g;
	
	# Build Dyninst
	{
		# Create the build directory
		make_path("$hash/dyninst/build");
	
		# The path must exist before using 'realpath'
		my $base_dir = realpath("$hash/dyninst");
		my $build_dir = "$base_dir/build";

		symlink($args{'dyninst-src'}, "$base_dir/src");
		
		print $fdLog "Building Dyninst($hash)... ";
		eval {
			&configure_dyninst(\%args, $base_dir, $build_dir);
			my $cmake_cache = &parse_cmake_cache("$build_dir/CMakeCache.txt");
			$args{'boost-lib'} = $cmake_cache->{'Boost_LIBRARY_DIR_RELEASE'};
			&build_dyninst(\%args, $build_dir);
			
			# Symlinking libdw is broken in the config system right now
			# See https://github.com/dyninst/dyninst/issues/547
			my $libdwarf_dir = $cmake_cache->{'LIBDWARF_LIBRARIES'};
			if($libdwarf_dir =~ /NOTFOUND/) {
				$libdwarf_dir = "$build_dir/elfutils/lib";
			} else {
				$libdwarf_dir = dirname($libdwarf_dir);
			}
			symlink("$libdwarf_dir/libdw.so", "$base_dir/lib/libdw.so");
			symlink("$libdwarf_dir/libdw.so.1", "$base_dir/lib/libdw.so.1");
			
		};
		print $fdLog $@ and die $@ if $@;
		print $fdLog "done.\n";
	}

#	# Build the test suite
#	{
#		# Create the build directory
#		make_path("$hash/testsuite/build");
#		
#		my $base_dir = realpath("$hash/testsuite");
#		my $build_dir = "$base_dir/build";
#		symlink($args{'test-src'}, "$base_dir/src");
#		symlink(realpath("$hash/dyninst"), "$base_dir/dyninst");
#		
#		print $fdLog "Building Testsuite... ";
#		eval {
#			&configure_tests(\%args, $base_dir, $build_dir);
#			&build_tests(\%args, $build_dir);
#		};
#		print $fdLog $@ and die $@ if $@;
#		print $fdLog "done.\n";
#	}
#
#	# Run the tests
#	if($args{'run-tests'}) {
#		make_path("$hash/testsuite/tests");
#		my $base_dir = realpath("$hash/testsuite/tests");
#		
#		print $fdLog "running Testsuite... ";
#		eval { &run_tests(\%args, $base_dir); };
#		print $fdLog $@ and die $@ if $@;
#		print $fdLog "done.\n";
#	}
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
			"LD_LIBRARY_PATH=$boost_lib;\n" .
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
			"Dyninst_DIR=../dyninst/lib/cmake/Dyninst ".
			"cmake ../src -DCMAKE_INSTALL_PREFIX=$base_dir " .
			"-DINSTALL_DIR=$base_dir/tests ".
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
			"LD_LIBRARY_PATH=$boost_lib ".
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
}

sub execute($) {
	my $cmd = shift;

	print "$cmd\n";

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
   --help                  Print this help message
=cut
