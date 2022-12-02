package Dyninst::testsuite;

use strict;
use warnings;
use base 'Exporter';
our @EXPORT_OK = qw(run);

use Dyninst::utils qw(execute list_unique canonicalize);
use Dyninst::cmake qw(load_from_cache);
use Dyninst::git;
use Dyninst::logs qw(save_compiler_config);
use Cwd qw(realpath);
use File::Path qw(make_path);
use Time::HiRes;
use Try::Tiny;

sub setup {
	my ($root_dir, $args) = @_;

	# Create the build directory
	make_path("$root_dir/testsuite/build");

	my $base_dir  = realpath("$root_dir/testsuite");
	my $build_dir = "$base_dir/build";
	
	$args->{'test-src'} //= "$args->{'prefix'}/testsuite";
	$args->{'test-src'} = canonicalize($args->{'test-src'});
	
	symlink($args->{'test-src'}, "$base_dir/src");
	symlink(realpath("$root_dir/dyninst"), "$base_dir/dyninst");

	# This is for internal use only
	$args->{'testsuite-cmake-cache-dir'} = $build_dir;

	my $git_config = Dyninst::git::get_config($args->{'test-src'}, $base_dir);

	# Check out the PR, if specified
	if ($args->{'testsuite-pr'}) {
		Dyninst::git::checkout_pr($args->{'test-src'}, $args->{'testsuite-pr'}, $git_config->{'branch'});
		$git_config = Dyninst::git::get_config($args->{'test-src'}, $base_dir);
	}

	Dyninst::git::save_config($base_dir, $git_config);

	return ($base_dir, $build_dir);
}

sub configure {
	my ($args, $base_dir, $build_dir) = @_;

	# Configure the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute("cd $build_dir\n"
			  . "$args->{'cmake'} "
			  . "../src "
			  . "-DCMAKE_INSTALL_PREFIX=$base_dir "
			  . "$args->{'cmake-args'} "
			  . "$args->{'testsuite-cmake-args'} "
			  . "-DINSTALL_DIR=$base_dir/tests "
			  . "-DDyninst_DIR=../dyninst/lib/cmake/Dyninst "
			  . "1>config.out 2>config.err");
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
}

sub build {
	my ($args, $build_dir) = @_;

	my $njobs = $args->{'njobs'};

	# Build the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval { execute("cd $build_dir\n" . "make VERBOSE=1 -j$njobs 1>build.out 2>build.err"); };
	die "Error building: see $build_dir/build.err for details" if $@;

	# Install the Testsuite
	# We need an 'eval' here since we are manually piping stderr
	eval { execute("cd $build_dir\n" . "make VERBOSE=1 install 1>build-install.out 2>build-install.err"); };
	die "Error installing: see $build_dir/build-install.err for details" if $@;
}

sub _killed_by_watchdog {
	my ($file) = @_;

	# Check if we were killed by the watchdog timer
	if (-f $file) {
		open my $fdIn, '<', $file;
		while (<$fdIn>) {
			return 1 if m/Process exceeded time limit/;
		}
	}
	return 0;
}

sub _run_single {
	my ($paths, $args, $base_dir, $run_log) = @_;

	my $test_names_file = "$base_dir/../build/test_names.txt";
	open my $fdTests, '<', $test_names_file     or die "Unable to open '$test_names_file': $!\n";
	open my $fdLog,   '>', "$base_dir/test.log" or die "Unable to open '$base_dir/test.log': $!\n";

	open my $fdOut, '>', "$base_dir/stdout.log";
	open my $fdErr, '>', "$base_dir/stderr.log";

	while (my $test_name = <$fdTests>) {
		chomp($test_name);

		# Put a marker in the stderr log
		print $fdErr "++Running $test_name\n";

		my $limit = defined($args->{'limit'}) ? "-limit $args->{'limit'}" : '';

		my $start = Time::HiRes::gettimeofday();

		try {
			execute("cd $base_dir\n"
				  . "export DYNINSTAPI_RT_LIB=$base_dir/../dyninst/lib/libdyninstAPI_RT.so\n"
				  . "export OMP_NUM_THREADS=$args->{'nompthreads'}\n"
				  . "LD_LIBRARY_PATH=$paths:\$LD_LIBRARY_PATH "
				  . "./runTests -64 -all -test $test_name $limit -log tmp.log 1>stdout.tmp 2>stderr.tmp");
		} catch {
			print $fdErr "\n$test_name failed in testsuite::run", '-' x 10, "\n";
		};

		if ($Dyninst::utils::debug_mode) {
			my $end = Time::HiRes::gettimeofday();
			$run_log->write(sprintf("$test_name took %.2f seconds", $end - $start));
		}

		if (_killed_by_watchdog("$base_dir/stderr.tmp")) {
			$run_log->write("$test_name exceeded time limit");
			Dyninst::logs::append_result("$base_dir/stdout.tmp", $test_name, 'HANGED');
		}

		# Concatenate the temporary logs with the permanent ones
		for my $f (['stdout.tmp', $fdOut], ['stderr.tmp', $fdErr], ['tmp.log', $fdLog]) {
			if (-f "$base_dir/$f->[0]") {
				open my $fdIn, '<', "$base_dir/$f->[0]";
				my $x = $f->[1];
				print $x (<$fdIn>);
			}
		}
	}
}

sub run_tests {
	my ($args, $base_dir, $run_log) = @_;

	# Grab the paths in the Dyninst build cache
	my @lib_dirs = ('Boost_LIBRARY_DIRS', 'ElfUtils_LIBRARY_DIRS', 'LibIberty_LIBRARY_DIRS');
	my $cache    = "$args->{'dyninst-cmake-cache-dir'}/CMakeCache.txt";
	my @libs     = load_from_cache($cache, \@lib_dirs);

	push @libs, ($base_dir, realpath("$base_dir/../dyninst/lib"));
	my $paths = join(':', list_unique(@libs));

	# If user explicitly requests single-stepping, then only run that mode
	if ($args->{'single-stepping'}) {
		_run_single($paths, $args, $base_dir, $run_log);
		return;
	}

	# By default, run the tests in group mode
	# If that fails, fall back to single-stepping mode
	try {
		my $limit = defined($args->{'limit'}) ? "-limit $args->{'limit'}" : '';

		execute("cd $base_dir\n"
			  . "export DYNINSTAPI_RT_LIB=$base_dir/../dyninst/lib/libdyninstAPI_RT.so\n"
			  . "export OMP_NUM_THREADS=$args->{'nompthreads'}\n"
			  . "LD_LIBRARY_PATH=$paths:\$LD_LIBRARY_PATH "
			  . "./runTests -64 -all -log test.log -j$args->{'ntestjobs'} $limit 1>stdout.log 2>stderr.log");

		## XXX do a better job here, otherwise
		## dual watchdog messages, but at least tell the truth
		# Being killed by the watchdog timer _should_ cause 'execute' to throw, but
		# check it here explicitly just to be sure
		if (_killed_by_watchdog("$base_dir/stderr.log")) {
			$run_log->write("group test exceeded time limit, uncaught, forcing...");
			die;
		}
		$run_log->write("Running in group mode succeeded, NO REPLAY NEEDED.\n");
	} catch {
		## make sure we know it is this failure
		if (_killed_by_watchdog("$base_dir/stderr.log")) {
			$run_log->write("group test exceeded time limit, caught");
		}
		if ($args->{'replay'}) {
			$run_log->write("Running in group mode failed. Running single-step mode.\n");
			_run_single($paths, $args, $base_dir, $run_log);
		} else {
			$run_log->write("Running in group mode failed, NO REPLAY.\n");
		}
	};
}

sub run {
	my ($args, $root_dir, $logger) = @_;

	# Don't do anything (not even the logs), if --no-tests was given
	return if !$args->{'tests'};

	# Always set up logs, even if doing a restart
	my ($base_dir, $build_dir) = setup($root_dir, $args);

	try {
		if ($args->{'build-tests'}) {
			$logger->write("Configuring Testsuite... ", 'eol' => '');
			configure($args, $base_dir, $build_dir);
			$logger->write("done\n");
			
			#NB: This only leaves the 'try' block, it does _NOT_ return from 'run'!
			return if $args->{'only-config'};

			save_compiler_config("$build_dir/config.out", "$base_dir/build/compilers.conf");

			$logger->write("Building Testsuite... ", 'eol' => '');
			build($args, $build_dir);
			$logger->write("done\n");
		}

		if ($args->{'run-tests'}) {
			make_path("$root_dir/testsuite/tests");
			my $base_dir = realpath("$root_dir/testsuite/tests");

			my $run_log = Dyninst::logs->new("$base_dir/run.log");

			$logger->write("running Testsuite... ", 'eol' => '');
			run_tests($args, $base_dir, $run_log);
			$logger->write("done.");
		}
	} catch {
		$logger->write($_);
		open my $fdOut, '>', "$root_dir/testsuite/Build.FAILED";
		die $_;
	}
}

1;
