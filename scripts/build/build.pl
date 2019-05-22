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

#-- Main
{
	my %args = (
	'prefix'				=> cwd(),
	'dyninst-src'			=> undef,
	'test-src'				=> undef,
	'boost-dir'				=> '',
	'elfutils-dir'			=> '',
	'tbb-dir'				=> '',
	'log-file'      		=> undef,
	'dyninst-pr'			=> undef,
	'testsuite-pr'			=> undef,
	'dyninst-cmake-args'	=> undef,
	'testsuite-cmake-args'	=> undef,
	'run-tests'				=> 1,
	'njobs' 				=> 1,
	'quiet'					=> 0,
	'purge'					=> 0,
	'help' 					=> 0,
	'restart'				=> undef,
	'debug-mode'			=> 0	# undocumented debug mode
	);

	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'test-src=s',
		'boost-dir=s', 'elfutils-dir=s', 'tbb-dir=s',
		'log-file=s', 'dyninst-pr=s', 'testsuite-pr=s',
		'dyninst-cmake-args=s', 'testsuite-cmake-args=s',
		'run-tests!', 'njobs=i', 'quiet', 'purge', 'help',
		'restart=s', 'debug-mode'
	) or pod2usage(-exitval=>2);

	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99);
	}

	$Dyninst::utils::debug_mode = $args{'debug-mode'};
	
	if($args{'restart'}) {
		if(!-d $args{'restart'}) {
			die "Requested restart directory ($args{'restart'}) does not exist\n";
		}
		$args{'run-tests'} = 1;
	}

	# Default directory and file locations
	$args{'dyninst-src'} //= "$args{'prefix'}/dyninst";
	$args{'test-src'} //= "$args{'prefix'}/testsuite";
	$args{'log-file'} //= "$args{'prefix'}/build.log";

	# Canonicalize user-specified files and directories
	for my $d ('dyninst-src','test-src','log-file',
				'boost-dir', 'elfutils-dir', 'tbb-dir')
	{
		# NB: realpath(undef|'') eq cwd()
		$args{$d} = realpath($args{$d}) if defined($args{$d}) && $args{$d} ne '';
	}

	# Save a backup, if the log file already exists
	move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};

	open my $fdLog, '>', $args{'log-file'} or die "$args{'log-file'}: $!\n";

	my $root_dir = ($args{'restart'}) ? $args{'restart'} : undef;

	eval {
		if($Dyninst::utils::debug_mode) {
			use Data::Dumper;
			print Dumper(\%args), "\n";
		}
		
		Dyninst::logs::save_system_info(\%args, $fdLog);
		
		# Generate a unique name for the current build
		$root_dir = tempdir('XXXXXXXX', CLEANUP=>0) unless $args{'restart'};
		Dyninst::logs::write($fdLog, !$args{'quiet'}, "root_dir: $root_dir\n");
		
		# Dyninst
		{
			# Always set up logs, even if doing a restart
			my ($base_dir, $build_dir) = Dyninst::dyninst::setup($root_dir, \%args, $fdLog);
	
			unless($args{'restart'}) {
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "Configuring Dyninst... ");
				Dyninst::dyninst::configure(\%args, $base_dir, $build_dir);
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "done.\n");
			
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "Building Dyninst... ");
				Dyninst::dyninst::build(\%args, $build_dir);
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "done.\n");
			}
		}
		
		# Testsuite
		{
			# Always set up logs, even if doing a restart
			my ($base_dir, $build_dir) = Dyninst::testsuite::setup($root_dir, \%args, $fdLog);

			unless($args{'restart'}) {
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "Configuring Testsuite... ");
				Dyninst::testsuite::configure(\%args, $base_dir, $build_dir);
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "done\n");
				
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "Building Testsuite... ");
				Dyninst::testsuite::build(\%args, $build_dir);
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "done\n");
			}
		}		
		
		# Run the tests
		if($args{'run-tests'}) {
			make_path("$root_dir/testsuite/tests");
			my $base_dir = realpath("$root_dir/testsuite/tests");

			Dyninst::logs::write($fdLog, !$args{'quiet'}, "running Testsuite... ");
			my $max_attempts = 3;
			while(!Dyninst::testsuite::run(\%args, $base_dir)) {
				$max_attempts--;
				if($max_attempts <= 0) {
					die "Maximum number of Testsuite retries exceeded\n";
				}
				Dyninst::logs::write($fdLog, !$args{'quiet'}, "\nTestsuite killed; restarting... ");
			}
			Dyninst::logs::write($fdLog, !$args{'quiet'}, "done.\n");
		}
	};
	if($@) {
		Dyninst::logs::write($fdLog, !$args{'quiet'}, $@);
		open my $fdOut, '>', "$root_dir/FAILED";
	}
	
	my $results_log = "$root_dir/testsuite/tests/results.log";
	# Parse the raw output
	if(-f "$root_dir/testsuite/tests/stdout.log") {
		my @res = Dyninst::logs::parse("$root_dir/testsuite/tests/stdout.log");
		open my $fdOut, '>', $results_log or die "$results_log: $!\n";
		$\ = "\n";
		print $fdOut @res;
	}

	# Create the exportable tarball of results
	my @log_files = (
		File::Spec->abs2rel($args{'log-file'}),
		"$root_dir/FAILED",
		"$root_dir/dyninst/git.log",
		"$root_dir/dyninst/build/config.out",
		"$root_dir/dyninst/build/config.err",
		"$root_dir/dyninst/build/build.out",
		"$root_dir/dyninst/build/build.err",
		"$root_dir/dyninst/build/build-install.out",
		"$root_dir/dyninst/build/build-install.err",
		"$root_dir/testsuite/git.log",
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
   --boost-dir=PATH        Base directory for Boost
   --elfutils-dir=PATH     Base directory for libelf/libdwarf
   --tbb-dir=PATH          Base directory for Intel's Threading Building Blocks
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --dyninst-pr            The Dyninst pull request formatted as 'remote/ID' with 'remote' being optional
   --testsuite-pr          The Testsuite pull request formatted as 'remote/ID' with 'remote' being optional
   --dyninst-cmake-args    Additional CMake arguments for Dyninst
   --testsuite-cmake-args  Additional CMake arguments for the Testsuite
   --njobs=N               Number of make jobs (default: N=1)
   --[no-]run-tests        Run the Testsuite (default: yes)
   --quiet                 Don't echo logging information to stdout (default: no)
   --purge                 Remove all files after running testsuite (default: no)
   --restart=ID            Restart the Testsuite for run 'ID' (implies --run-tests; does not build Dyninst or Testsuite)
   --help                  Print this help message
=cut
