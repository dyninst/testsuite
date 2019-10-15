use strict;
use warnings;
use Cwd qw(cwd realpath);
use Getopt::Long qw(GetOptions);
use File::Copy qw(move);
use File::Path qw(make_path remove_tree);
use Pod::Usage;
use File::Temp qw(tempdir);
use Dyninst::logs;
use Dyninst::dyninst;
use Dyninst::testsuite;
use Dyninst::utils;

my $invocation_args = join(' ', 
	map {
		my ($l,$r)=split('=',$_,2);
		$l .= "=\"$r\"" if $r;
	} @ARGV
);

my %args = (
	'prefix'				=> cwd(),
	'dyninst-src'			=> undef,
	'test-src'				=> undef,
	'log-file'      		=> undef,
	'dyninst-pr'			=> undef,
	'testsuite-pr'			=> undef,
	'cmake-args'			=> '',
	'dyninst-cmake-args'	=> '',
	'testsuite-cmake-args'	=> '',
	'build-tests'			=> 1,
	'run-tests'				=> 1,
	'tests'					=> 1,
	'njobs' 				=> 1,
	'quiet'					=> 0,
	'purge'					=> 0,
	'help' 					=> 0,
	'restart'				=> undef,
	'upload'				=> 0,
	'ntestjobs'				=> 1,
	'nompthreads'			=> 2,
	'single-stepping'		=> 0,
	'max-attempts'			=> 3,
	'auth-token'			=> undef,
	'debug-mode'			=> 0	# undocumented debug mode
);

GetOptions(\%args,
	'prefix=s', 'dyninst-src=s', 'test-src=s',
	'log-file=s', 'dyninst-pr=s', 'testsuite-pr=s',
	'cmake-args=s', 'dyninst-cmake-args=s',
	'testsuite-cmake-args=s', 'build-tests!',
	'run-tests!', 'tests!', 'njobs=i', 'quiet', 'purge',
	'help', 'restart=s', 'upload!', 'ntestjobs=i',
	'nompthreads=i', 'single-stepping', 'max-attempts=i',
	'auth-token=s', 'debug-mode'
) or pod2usage(-exitval=>2);

if($args{'help'}) {
	pod2usage(-exitval => 0, -verbose => 99);
}

if($args{'upload'} && !$args{'auth-token'}) {
	die "Must specify authentication token when uploading\n";
}

$Dyninst::utils::debug_mode = $args{'debug-mode'};

# Default directory and file locations
$args{'dyninst-src'} //= "$args{'prefix'}/dyninst";
$args{'test-src'} //= "$args{'prefix'}/testsuite";
$args{'log-file'} //= "$args{'prefix'}/build.log";

# Canonicalize user-specified files and directories
for my $d ('dyninst-src','test-src','log-file') {
	# NB: realpath(undef|'') eq cwd()
	$args{$d} = realpath($args{$d}) if defined($args{$d}) && $args{$d} ne '';
}

# By default, build Dyninst
$args{'build-dyninst'} = 1;

# By default, build and run tests
# --no-tests is an alias for "--no-build-tests --no-run-tests"
if(!$args{'tests'}) {
	$args{'build-tests'} = 0;
	$args{'run-tests'} = 0;
}

if($args{'restart'}) {
	if(!-d $args{'restart'}) {
		die "Requested restart directory ($args{'restart'}) does not exist\n";
	}

	# If the Dyninst build is missing or failed, then rebuild it		
	my $dyninst_ok = -d "$args{'restart'}/dyninst" &&
					!-e "$args{'restart'}/dyninst/Build.FAILED";
	$args{'build-dyninst'} = !$dyninst_ok;
	
	# If the Testsuite build is missing or failed, then rebuild it
	# The Testsuite cannot be good if Dyninst is not good
	my $testsuite_ok = -d "$args{'restart'}/testsuite" &&
					  !-e "$args{'restart'}/testsuite/Build.FAILED" &&
					  $dyninst_ok;

	my $user_wants_to_build_tests = $args{'build-tests'};
	$args{'build-tests'} = !$testsuite_ok && $user_wants_to_build_tests;
	
	# Sanity check: If the existing Testsuite is bad and the user
	#				wants to run the tests, but the user requested not
	#				to build the Testsuite, then we can't continue
	if(!$testsuite_ok && $args{'run-tests'} && !$user_wants_to_build_tests) {
		print "The Testsuite in '$args{'restart'}' must be rebuilt\n";
		exit 1;
	}
	
	# Remove the FAILED files (if present)
	unlink(
		"$args{'restart'}/dyninst/Build.FAILED",
		"$args{'restart'}/testsuite/Build.FAILED",
		"$args{'restart'}/Tests.FAILED"
	);
} else {
	if($args{'run-tests'} && !$args{'build-tests'}) {
		print "The Testsuite must be built before it can be run. ",
		      "Use --restart to reuse a previous build.\n";
		exit 1;
	}
}

# Save a backup, if the log file already exists
move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};

open my $fdLog, '>', $args{'log-file'} or die "$args{'log-file'}: $!\n";

my $root_dir = ($args{'restart'}) ? $args{'restart'} : undef;

if($Dyninst::utils::debug_mode) {
	use Data::Dumper;
	print Dumper(\%args), "\n";
}

Dyninst::logs::save_system_info(\%args, $fdLog);

# Generate a unique name for the current build
$root_dir = tempdir('XXXXXXXX', CLEANUP=>0) unless $args{'restart'};
Dyninst::logs::write($fdLog, !$args{'quiet'}, "root_dir: $root_dir\n");

# Display the invocation arguments
print $fdLog "Invoked using '$invocation_args'\n";

eval {		
	# Dyninst
	# Always set up logs, even if doing a restart
	my ($base_dir, $build_dir) = Dyninst::dyninst::setup($root_dir, \%args, $fdLog);

	if($args{'build-dyninst'}) {
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "Configuring Dyninst... ");
		Dyninst::dyninst::configure(\%args, $base_dir, $build_dir);
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "done.\n");
		
		Dyninst::utils::save_compiler_config("$build_dir/config.out", "$base_dir/build/compilers.conf");

		Dyninst::logs::write($fdLog, !$args{'quiet'}, "Building Dyninst... ");
		Dyninst::dyninst::build(\%args, $build_dir);
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "done.\n");
	}
};
if($@) {
	Dyninst::logs::write($fdLog, !$args{'quiet'}, $@);
	open my $fdOut, '>', "$root_dir/dyninst/Build.FAILED";
	$args{'build-tests'} = 0;
	$args{'run-tests'} = 0;
}

eval {
	# Testsuite
	# Always set up logs, even if doing a restart
	my ($base_dir, $build_dir) = Dyninst::testsuite::setup($root_dir, \%args, $fdLog);
	
	if($args{'build-tests'}) {
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "Configuring Testsuite... ");
		Dyninst::testsuite::configure(\%args, $base_dir, $build_dir);
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "done\n");
		
		Dyninst::utils::save_compiler_config("$build_dir/config.out", "$base_dir/build/compilers.conf");

		Dyninst::logs::write($fdLog, !$args{'quiet'}, "Building Testsuite... ");
		Dyninst::testsuite::build(\%args, $build_dir);
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "done\n");
	}
};
if($@) {
	Dyninst::logs::write($fdLog, !$args{'quiet'}, $@);
	open my $fdOut, '>', "$root_dir/testsuite/Build.FAILED";
	$args{'run-tests'} = 0;
}

eval {
	# Run the tests
	if($args{'run-tests'}) {
		make_path("$root_dir/testsuite/tests");
		my $base_dir = realpath("$root_dir/testsuite/tests");

		Dyninst::logs::write($fdLog, !$args{'quiet'}, "running Testsuite... ");
		my $max_attempts = $args{'max-attempts'};
		while(!Dyninst::testsuite::run(\%args, $base_dir)) {
			$max_attempts--;
			if($max_attempts <= 0) {
				die "Maximum number of Testsuite run attempts exceeded\n";
			}
			Dyninst::logs::write($fdLog, !$args{'quiet'}, "\nTestsuite killed; restarting... ");
		}
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "done.\n");
	}
};
if($@) {
	Dyninst::logs::write($fdLog, !$args{'quiet'}, $@);
	open my $fdOut, '>', "$root_dir/Tests.FAILED";
}

my $results_log = "$root_dir/testsuite/tests/results.log";
# Parse the raw output
if(-f "$root_dir/testsuite/tests/stdout.log") {
	my @res = Dyninst::logs::parse("$root_dir/testsuite/tests/stdout.log");
	open my $fdOut, '>', $results_log or die "$results_log: $!\n";
	local $\ = "\n";
	print $fdOut @res;
}

# Create the exportable tarball of results
my @log_files = (
	File::Spec->abs2rel($args{'log-file'}),
	"$root_dir/dyninst/Build.FAILED",
	"$root_dir/testsuite/Build.FAILED",
	"$root_dir/Tests.FAILED",
	"$root_dir/dyninst/git.log",
	"$root_dir/dyninst/build/compilers.conf",
	"$root_dir/dyninst/build/config.out",
	"$root_dir/dyninst/build/config.err",
	"$root_dir/dyninst/build/build.out",
	"$root_dir/dyninst/build/build.err",
	"$root_dir/dyninst/build/build-install.out",
	"$root_dir/dyninst/build/build-install.err",
	"$root_dir/testsuite/git.log",
	"$root_dir/testsuite/build/compilers.conf",
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
Dyninst::utils::execute("tar -zcf $root_dir.results.tar.gz $files");

# Remove the generated files, if requested
if($args{'purge'}) {
	remove_tree($root_dir);
}

# Upload the results to the dashboard, if requested
if($args{'upload'}) {
	eval {
		Dyninst::utils::execute(
			"curl -F \"upload=\@$root_dir.results.tar.gz\" ".
			"-F \"token=$args{'auth-token'}\" ".
			"https://bottle.cs.wisc.edu/upload"
		);
	};
	if($@) {
		print "An error occurred when uploading the results\n$@\n";
	}
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
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --dyninst-pr            The Dyninst pull request formatted as 'remote/ID' with 'remote' being optional
   --testsuite-pr          The Testsuite pull request formatted as 'remote/ID' with 'remote' being optional
   --cmake-args            CMake options passed to both Dyninst and the test suite (format '-DVAR=VALUE')
   --dyninst-cmake-args    Additional CMake arguments for Dyninst
   --testsuite-cmake-args  Additional CMake arguments for the Testsuite
   --[no-]build-tests      Build the Testsuite (default: yes)
   --[no-]run-tests        Run the Testsuite (default: yes)
   --[no-]tests            Alias for "--[no-]build-tests --[no-]run-tests"
   --njobs=N               Number of make jobs (default: N=1)
   --quiet                 Don't echo logging information to stdout (default: no)
   --purge                 Remove all files after running testsuite (default: no)
   --restart=ID            Restart the script for run 'ID'
   --[no-]upload           Upload the results to the Dyninst dashboard (default: no)
   --ntestjobs             Number of tests to run in parallel (default: 1)
   --nompthreads           Number of OpenMP threads to use for parallel parsing when running tests (default: 2)
   --single-stepping       Run the tests one at a time (i.e., not in 'group' mode) (default: no)
   --max-attempts=N        Run the test suite a maximum of N failed attempts before giving up (default: 3)
   --auth-token=STRING     The authentication token string. Required when uploading the results.
   --help                  Print this help message
=cut
