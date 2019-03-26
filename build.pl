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

#-- Main
{
	my %args = (
	'prefix'				=> cwd(),
	'dyninst-src'			=> undef,
	'test-src'				=> undef,
	'boost-dir'				=> undef,
	'log-file'      		=> undef,
	'njobs' 				=> 1,
	'help' 					=> 0
	);

	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'test-src=s',
		'boost-dir=s', 'boost-inc=s', 'boost-lib=s',
		'log-file=s', 'njobs=i', 'help'
	) or (pod2usage(2), exit);

	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99) and exit;
	}

	# Default directory and file locations
	$args{'dyninst-src'} //= "$args{'prefix'}/dyninst";
	$args{'test-src'} //= "$args{'prefix'}/testsuite";
	$args{'log-file'} //= "$args{'prefix'}/build.log";

	# Canonicalize user-specified files and directories
	for my $d ('dyninst-src','test-src','log-file','boost-inc','boost-lib') {
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
		
		#	# Set up the Boost environment
		{
			$args{'boost-inc'} = "$args{'boost-dir'}/include" if $args{'boost-dir'};
			$args{'boost-lib'} = "$args{'boost-dir'}/lib" if $args{'boost-dir'};
			
			# If the user didn't specify a Boost location, then provide some defaults
			# NB: This will be fixed by https://github.com/dyninst/dyninst/issues/563
			unless($args{'boost-inc'}) {
				$args{'boost-inc'} = "$build_dir/boost/src/boost";
			}
			unless($args{'boost-lib'}) {
				$args{'boost-lib'} = "$build_dir/boost/src/boost/stage/lib";
			}
		}
		
		print $fdLog "Building Dyninst($hash)... ";
		eval { &build_dyninst(\%args, $base_dir, $build_dir); };
		print $fdLog $@ and die $@ if $@;
		print $fdLog "done.\n";
	}

	# Build the test suite
	{
		eval { &build_tests(\%args); };
		print $fdLog $@ and die $@ if $@;
	}

	# Run the tests
	{
		eval { &run_tests(\%args); };
		print $fdLog $@ and die $@ if $@;
	}
}

sub build_dyninst {
	my ($args, $base_dir, $build_dir) = @_;

	my $src_dir = $args->{'dyninst-src'};
	my $njobs = $args->{'njobs'};
	my $boost_inc = $args->{'boost-inc'};
	my $boost_lib = $args->{'boost-lib'};

	# Create symlink to source
	symlink("$src_dir", "$base_dir/src");

	# Save the Dyninst git configuration
	{
		# If the user didn't give a branch name, use the current one
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

	# Configure the build
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $build_dir\n" .
			"cmake -H$base_dir/src -B$build_dir " .
			"-DCMAKE_INSTALL_PREFIX=$base_dir " .
			"-DBoost_INCLUDE_DIR=$boost_inc " .
			"-DBoost_LIBRARY_DIR_DEBUG=$boost_lib " .
			"-DBoost_LIBRARY_DIR_RELEASE=$boost_lib " .
			"-DUSE_GNU_DEMANGLER:BOOL=ON " .
			"1>config.out 2>config.err "
		);
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
	
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
	};	die "Error installing: see $build_dir/build-install.err for details" if $@;

	# Symlinking libdw is broken in the config system right now
	# See https://github.com/dyninst/dyninst/issues/547
	symlink("$build_dir/elfutils/lib/libdw.so", "$base_dir/lib/libdw.so");
	symlink("$build_dir/elfutils/lib/libdw.so.1", "$base_dir/lib/libdw.so.1");
}

sub build_tests {
	my ($args, $hash) = @_;
	
	my $src_dir = $args->{'test-src'};
	my $branch = $args->{'test-branch'};
	my $rel_branch = $args->{'test-relative-to'};
	
	execute("git -C $src_dir checkout $branch");
	
	# Create the build directory
	make_path("$hash/testsuite/build");
	
	my $base_dir = "$hash/testsuite";
	my $build_dir = "$base_dir/build";
	
	symlink("$src_dir", "$base_dir/src");
	symlink("$")
#cd $testsuite_build_base_dir
#echo dyninst: $dyninst_branch $dyninst_hash\ntestsuite: $testsuite_branch > git.log
#ln -s $(abspath $testsuite_src_dir) src
#ln -s $(abspath $dyninst_build_base_dir) dyninst
#cd $testsuite_build_base_dir/build
#Dyninst_DIR=../dyninst/lib/cmake/Dyninst ccmake ../src -DCMAKE_INSTALL_PREFIX=$(realpath ..) -DINSTALL_DIR=$(realpath ../tests) -DBoost_INCLUDE_DIR=$(realpath ../dyninst/build/boost/src/boost)
#Dyninst_DIR=../dyninst/lib/cmake/Dyninst ccmake ../src -DCMAKE_INSTALL_PREFIX=$(realpath ..) -DINSTALL_DIR=$(realpath ../tests) -DBoost_INCLUDE_DIR=/usr/local/lib/boost-1.69/include
#LD_LIBRARY_PATH=$(realpath ../dyninst/build/boost/src/boost/stage/lib) make -j8
#LD_LIBRARY_PATH=/usr/local/lib/boost-1.69/lib make -j8
}

sub run_tests {
#export DYNINSTAPI_RT_LIB=$(realpath ../dyninst/lib/libdyninstAPI_RT.so)
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib:$(realpath ../dyninst/build/boost/src/boost/stage/lib) ./runTests -all -log test.log 1>stdout.log 2>stderr.log
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib:/usr/local/lib/boost-1.69/lib ./runTests -all -log test.log 1>stdout.log 2>stderr.log
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib ./runTests -all -log test.log 1>stdout.log 2>stderr.log
}

sub execute($) {
	my $cmd = shift;
	print "$cmd\n";
	return 'foo';
#	my ($stdout,$stderr,$exit) = capture { system($cmd); };
#	$exit = (( $exit >> 8 ) != 0 || $exit == -1 || ( $exit & 127 ) != 0);
#	die "Error executing '$cmd'\n$stderr\n" if $exit;
#	return $stdout;
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
   --dyninst-dir=PATH      Path to pre-built Dyninst (only used for --no-dyninst)
   --test-src=PATH         Source directory for Testsuite (default: prefix/testsuite)
   --boost-dir=PATH        Base directory for Boost
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --njobs=N               Number of make jobs (default: N=1)
   --help                  Print this help message
=cut
