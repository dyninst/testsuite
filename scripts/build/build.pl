use strict;
use warnings;

## automatically locate the dependencies!
## less dependencies
# use File::Basename;
# use lib dirname(__FILE__);
## JK recommends this version
use FindBin;
use lib "$FindBin::Bin";

use Cwd qw(cwd);
use File::Copy qw(move);
use File::Path qw(make_path remove_tree);
use File::Temp qw(tempdir);

use Dyninst::logs;
use Dyninst::dyninst;
use Dyninst::testsuite;
use Dyninst::utils;
use Dyninst::options;
use Dyninst::restart;
use Archive::Tar;

my $args = Dyninst::options::parse();

if($args->{'help'}) {
	Dyninst::options::show_help();
	exit 0;
}

if($args->{'upload'} && !$args->{'auth-token'}) {
	die "Must specify authentication token when uploading\n";
}

if (defined($args->{'restart'}) && defined($args->{'root'})) {
	die "Options --restart and --root are mutually exclusive\n";
}

$Dyninst::utils::debug_mode = $args->{'debug-mode'};

# By default, build Dyninst
$args->{'build-dyninst'} = 1;

# By default, build and run tests
# --no-tests is an alias for "--no-build-tests --no-run-tests"
if(!$args->{'tests'}) {
	$args->{'build-tests'} = 0;
	$args->{'run-tests'} = 0;
}

# Configure restart, if requested
if($args->{'restart'}) {
	Dyninst::restart::setup($args);
} else {
	if($args->{'run-tests'} && !$args->{'build-tests'}) {
		die "The Testsuite must be built before it can be run. ".
		    "Use --restart to reuse a previous build.\n";
	}
}

# Do a variable dump in debug mode
if($Dyninst::utils::debug_mode) {
	use Data::Dumper;
	print Dumper($args), "\n";
}

#--------------------------------------------------------------------------------------------------
my $logger = Dyninst::logs->new($args);

# Display the invocation arguments
$logger->write("Invoked using '$Dyninst::options::invocation_args'");

Dyninst::logs::save_system_info($logger, $args->{'hostname'});

my $root_dir = Dyninst::utils::make_root($args);

$logger->write("root_dir: $root_dir");

if(Dyninst::dyninst::run($args, $root_dir, $logger)) {
	Dyninst::testsuite::run($args, $root_dir, $logger);
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
{
	my @log_files = (
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
		"$root_dir/testsuite/tests/run.log",
		$results_log
	);

	my $tar = Archive::Tar->new();
	
	# Only add the files that exist
	# Non-existent files indicate an error occurred
	$tar->add_files(grep {-f $_ } @log_files);
	
	# The dashboard assumes the build log is name 'build.log'
	# Rename the file accordingly
	open my $fdBuildLog, '<', File::Spec->abs2rel($args->{'log-file'});
	local $/ = undef;
	$tar->add_data('build.log', <$fdBuildLog>);
	
	$tar->write("$root_dir.results.tar.gz", COMPRESS_GZIP);
}

# Remove the generated files, if requested
if($args->{'purge'}) {
	remove_tree($root_dir);
}

# Upload the results to the dashboard, if requested
if($args->{'upload'}) {
	eval {
		Dyninst::utils::execute(
			"curl --insecure -F \"upload=\@$root_dir.results.tar.gz\" ".
			"-F \"token=$args->{'auth-token'}\" ".
			"https://bottle.cs.wisc.edu/upload"
		);
	};
	if($@) {
		print "An error occurred when uploading the results\n$@\n";
	}
}
