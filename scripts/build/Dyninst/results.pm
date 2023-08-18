package Dyninst::results;

use strict;
use warnings;
use base 'Exporter';
our @EXPORT_OK = qw(save);

use Archive::Tar;
use File::Spec;

sub save {
	my ($args, $root_dir) = @_;
	my $results_log = "$root_dir/testsuite/tests/results.log";

	# Parse the raw output
	if (-f "$root_dir/testsuite/tests/stdout.log") {
		my @res = Dyninst::logs::parse("$root_dir/testsuite/tests/stdout.log");
		open my $fdOut, '>', $results_log or die "$results_log: $!\n";
		local $\ = "\n";
		print $fdOut @res;
	}
}

1;
