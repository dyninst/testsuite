use strict;
use warnings;

## automatically locate the dependencies!
## less dependencies
# use File::Basename;
# use lib dirname(__FILE__);
## JK recommends this version
use FindBin;
use lib "$FindBin::Bin";

use Dyninst::logs;
use Dyninst::dyninst;
use Dyninst::testsuite;
use Dyninst::utils;
use Dyninst::options;
use Dyninst::restart;

use File::Path qw(remove_tree);

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

# Build Dyninst and the test suite.
if(Dyninst::dyninst::run($args, $root_dir, $logger)) {
	# This also runs the test suite if everything is good.
	Dyninst::testsuite::run($args, $root_dir, $logger);
}

# Save the results in a tarball
Dyninst::results::save($args, $root_dir);

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
