package Dyninst::options;

use base 'Exporter';
our @EXPORT_OK = qw(parse show_help);

use Pod::Usage;
use Pod::Find qw(pod_where);
use Getopt::Long qw(GetOptions);
use Cwd qw(cwd);

my %args = (
	'prefix'               => cwd(),
	'dyninst-src'          => undef,
	'test-src'             => undef,
	'log-file'             => undef,
	'dyninst-pr'           => undef,
	'testsuite-pr'         => undef,
	'cmake-args'           => '',
	'cmake'                => 'cmake',
	'dyninst-cmake-args'   => '',
	'testsuite-cmake-args' => '',
	'build-tests'          => 1,
	'run-tests'            => 1,
	'tests'                => 1,
	'njobs'                => 1,
	'quiet'                => 0,
	'purge'                => 0,
	'help'                 => 0,
	'restart'              => undef,
	'upload'               => 0,
	'ntestjobs'            => 1,
	'nompthreads'          => 2,
	'single-stepping'      => 0,
	'auth-token'           => undef,
	'sterile'              => 1,
	'hostname'             => undef,
	'debug-mode'           => 0,         # undocumented debug mode
	'limit'                => undef,     # change group limit
	'root'                 => undef,     # root directory name
	'replay'               => 1,         # replay fails in single-step
	'config-only'          => 0
);

our $invocation_args;

sub parse {
	$invocation_args = join(
		' ',
		map {
			my ($l, $r) = split('=', $_, 2);
			$l .= "=\"$r\"" if $r;
			$l;
		} @ARGV
	);

	GetOptions(
		\%args,                 'prefix=s',      'dyninst-src=s',          'test-src=s',
		'log-file=s',           'dyninst-pr=s',  'testsuite-pr=s',         'cmake-args=s',
		'dyninst-cmake-args=s', 'cmake=s',       'testsuite-cmake-args=s', 'build-tests!',
		'run-tests!',           'tests!',        'njobs=i',                'quiet',
		'purge',                'help',          'restart=s',              'upload!',
		'ntestjobs=i',          'nompthreads=i', 'single-stepping',        'auth-token=s',
		'sterile!',             'hostname=s',    'debug-mode',             'limit=i',
		'root=s',               'replay!',       'config-only!'
	) or pod2usage(-input => pod_where({ -inc => 1 }, __PACKAGE__), -exitval => 2);

	return \%args;
}

sub show_help {
	pod2usage(-input => pod_where({ -inc => 1 }, __PACKAGE__));
}

1;

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
   --cmake                 CMake to use.
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
   --auth-token=STRING     The authentication token string. Required when uploading the results.
   --[no-]sterile          Use a sterile build- don't download dependencies (default: yes)
   --hostname              Override the hostname provided by `uname`
   --limit=n               Change group test limit in testsuite.
   --root=dir              Set name/ID of root of test directory
   --no-replay             Turn off automatic replay of failed tests.
   --config-only           Only run the configure step of the build
   --help                  Print this help message
=cut

