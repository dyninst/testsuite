package Dyninst::results;

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

	# Create the exportable tarball of results
	my @log_files = (
		"$root_dir/dyninst/Build.FAILED",              "$root_dir/testsuite/Build.FAILED",
		"$root_dir/Tests.FAILED",                      "$root_dir/dyninst/git.log",
		"$root_dir/dyninst/build/compilers.conf",      "$root_dir/dyninst/build/config.out",
		"$root_dir/dyninst/build/config.err",          "$root_dir/dyninst/build/build.out",
		"$root_dir/dyninst/build/build.err",           "$root_dir/dyninst/build/build-install.out",
		"$root_dir/dyninst/build/build-install.err",   "$root_dir/testsuite/git.log",
		"$root_dir/testsuite/build/compilers.conf",    "$root_dir/testsuite/build/config.out",
		"$root_dir/testsuite/build/config.err",        "$root_dir/testsuite/build/build.out",
		"$root_dir/testsuite/build/build.err",         "$root_dir/testsuite/build/build-install.out",
		"$root_dir/testsuite/build/build-install.err", "$root_dir/testsuite/tests/stdout.log",
		"$root_dir/testsuite/tests/stderr.log",        "$root_dir/testsuite/tests/test.log",
		"$root_dir/testsuite/tests/run.log",           $results_log
	);

	my $tar = Archive::Tar->new();

	# Only add the files that exist
	# Non-existent files indicate an error occurred
	$tar->add_files(grep { -f $_ } @log_files);

	# The dashboard assumes the build log is name 'build.log'
	# Rename the file accordingly
	open my $fdBuildLog, '<', File::Spec->abs2rel($args->{'log-file'});
	local $/ = undef;
	$tar->add_data('build.log', <$fdBuildLog>);

	$tar->write("$root_dir.results.tar.gz", COMPRESS_GZIP);
}

1;
