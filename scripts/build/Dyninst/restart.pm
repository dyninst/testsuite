package Dyninst::restart;

use strict;
use warnings;
use base 'Exporter';
our @EXPORT_OK = qw(setup);

sub setup {
	my $args = shift;

	if (!-d $args->{'restart'}) {
		die "Requested restart directory ($args->{'restart'}) does not exist\n";
	}

	# If the Dyninst build is missing or failed, then rebuild it
	my $dyninst_ok = -d "$args->{'restart'}/dyninst" && !-e "$args->{'restart'}/dyninst/Build.FAILED";
	$args->{'build-dyninst'} = !$dyninst_ok;

	# If the Testsuite build is missing or failed, then rebuild it
	# The Testsuite cannot be good if Dyninst is not good
	my $testsuite_ok =
		 -d "$args->{'restart'}/testsuite"
	  && !-e "$args->{'restart'}/testsuite/Build.FAILED"
	  && $dyninst_ok;

	my $user_wants_to_build_tests = $args->{'build-tests'};
	$args->{'build-tests'} = !$testsuite_ok && $user_wants_to_build_tests;

	# Sanity check: If the existing Testsuite is bad and the user
	#				wants to run the tests, but the user requested not
	#				to build the Testsuite, then we can't continue
	if (!$testsuite_ok && $args->{'run-tests'} && !$user_wants_to_build_tests) {
		die "The Testsuite in '$args->{'restart'}' must be rebuilt\n";
	}

	# Remove the FAILED files (if present)
	unlink(
		"$args->{'restart'}/dyninst/Build.FAILED",
		"$args->{'restart'}/testsuite/Build.FAILED",
		"$args->{'restart'}/Tests.FAILED"
	);
}

1;
