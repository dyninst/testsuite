package Dyninst::testsuite;

use base 'Exporter';
our @EXPORT_OK = qw(setup configure build run);

use Dyninst::utils qw(execute list_unique parse_cmake_cache);
use Dyninst::git;
use Cwd qw(realpath);
use File::Path qw(make_path);


sub setup {
	my ($root_dir, $args, $fdLog) = @_;
	
	# Create the build directory
	make_path("$root_dir/testsuite/build");

	my $base_dir = realpath("$root_dir/testsuite");
	my $build_dir = "$base_dir/build";
	symlink($args->{'test-src'}, "$base_dir/src");
	symlink(realpath("$root_dir/dyninst"), "$base_dir/dyninst");
	
	my $git_config = Dyninst::git::get_config($args->{'test-src'}, $base_dir);
	
	# Check out the PR, if specified
	if($args->{'testsuite-pr'}) {
		Dyninst::git::checkout_pr($args->{'test-src'}, $args->{'testsuite-pr'}, $git_config->{'branch'});
		$git_config = Dyninst::git::get_config($args->{'test-src'}, $base_dir);
	}
	
	Dyninst::git::save_config($base_dir, $git_config->{'branch'},$git_config->{'commit'});
	
	return ($base_dir, $build_dir);
}


sub configure {
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

sub build {
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
sub run {
	my ($args, $base_dir) = @_;

	# Construct LD_LIBRARY_PATH from the paths in the Dyninst build cache	
	my $cmake_cache = parse_cmake_cache("$args->{'cmake-cache-dir'}/CMakeCache.txt");
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

	my $err = undef;
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute(
			"cd $base_dir\n" .
			"export DYNINSTAPI_RT_LIB=$base_dir/../dyninst/lib/libdyninstAPI_RT.so\n".
			"LD_LIBRARY_PATH=$paths:\$LD_LIBRARY_PATH " .
			"./runTests -all -log test.log 1>stdout.log 2>stderr.log"
		);
	};
	$err = $@ if $@;

	# Check if we were killed by the watchdog timer
	open my $fdIn, '<', "$base_dir/stderr.log";
	while(<$fdIn>) {
		return !!0 if m/Process exceeded time limit/;
	}

	# The run failed for some reason other than the watchdog timer
	chomp($err);
	if($err && $err eq '') {
		# runTest returned a non-zero value, but no actual error
		# message. Check the log for an error message
		open my $fdIn, '<', "$base_dir/stderr.log" or die "$!\n";
		while(<$fdIn>) {
			if(/\berror\b/i) {
				return !!0;
			}
		}
	}
	
	return !!1;
}

1;