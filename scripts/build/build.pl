use strict;
use warnings;

if (eval { require FindBin; 1;}) {
	use FindBin;
	use lib "$FindBin::Bin";
} else {
	use File::Basename qw(dirname);
	use lib dirname(__FILE__);
}

use Dyninst::logs;
use Dyninst::dyninst;
use Dyninst::testsuite;
use Dyninst::utils qw(make_root upload canonicalize);
use Dyninst::options;
use Dyninst::restart;
use Dyninst::results;
use File::Path qw(remove_tree);

my $args = Dyninst::options::parse();

if ($args->{'help'}) {
	Dyninst::options::show_help();
	exit 0;
}

# ------- Configure Build -------------------------------------------

$Dyninst::utils::debug_mode = $args->{'debug-mode'};

# By default, build Dyninst
$args->{'build-dyninst'} = 1;

# By default, build and run tests
# --no-tests is an alias for "--no-build-tests --no-run-tests"
if (!$args->{'tests'}) {
	$args->{'build-tests'} = 0;
	$args->{'run-tests'}   = 0;
}

# Configure restart, if requested
if ($args->{'restart'}) {
	Dyninst::restart::setup($args);
} else {
	if ($args->{'run-tests'} && !$args->{'build-tests'}) {
		die "The Testsuite must be built before it can be run. " . "Use --restart to reuse a previous build.\n";
	}
}

# Do a variable dump in debug mode
if ($Dyninst::utils::debug_mode) {
	use Data::Dumper;
	print Dumper($args), "\n";
}

# ------- Run the builds and tests ----------------------------------
$args->{'log-file'} //= "$args->{'prefix'}/build.log";
my $logger = Dyninst::logs->new(canonicalize($args->{'log-file'}), $args->{'quiet'});

# Display the invocation arguments
$logger->write("Invoked using '$Dyninst::options::invocation_args'\n\n");

Dyninst::logs::save_system_info($logger, $args->{'hostname'});

my $root_dir = make_root($args);

$logger->write("root_dir: $root_dir");

# Build Dyninst and the test suite.
if (Dyninst::dyninst::run($args, $root_dir, $logger)) {

	# This also runs the test suite if everything is good.
	Dyninst::testsuite::run($args, $root_dir, $logger);
}

# Save the results in a tarball
my $tarball_name = Dyninst::results::save($args, $root_dir);

# Remove the generated files, if requested
if ($args->{'purge'}) {
	remove_tree($root_dir);
}

# Upload the results to the dashboard, if requested
if ($args->{'upload'}) {
	upload($tarball_name, $args->{'auth-token'});
}
