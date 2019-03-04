use strict;
use warnings;
use Cwd qw(cwd);
use lib cwd();
use Getopt::Long qw(GetOptions);
use File::Copy qw(copy move);
use File::Path qw(make_path);
use Digest::MD5 qw(md5_base64);
use Pod::Usage;

#-- Main
{
	my %args = (
	'prefix'			=> cwd(),
	'dyninst-src'		=> undef,
	'dyninst-dir'		=> undef,
	'dyninst-branch' 	=> 'master',
	'test-src'			=> undef,
	'test-branch'		=> 'master',
	'log-file'      	=> undef,
	'njobs' 			=> 1,
	'dyninst'			=> 1,
	'tests'				=> 1,
	'run-tests'			=> 1,
	'help' 				=> 0
	);
	
	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'dyninst-dir=s',
		'dyninst-branch=s', 'test-src=s', 'test-branch=s',
		'log-file=s', 'njobs=i', 'dyninst!', 'tests!',
		'run-tests!', 'help'
	) or (pod2usage(2), exit);
	
	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99) and exit;
	}
	
	# Default directory and file locations
	$args{'dyninst-src'} //= "$args{'prefix'}/dyninst";
	$args{'test-src'} //= "$args{'prefix'}/testsuite";
	$args{'log-file'} //= "$args{'prefix'}/build.log";
	
	if(!$args{'dyninst'} && !$args{'dyninst-dir'}) {
		print STDERR "Must specify dyninst-dir when not building Dyninst\n";
		pod2usage(2) and exit;
	}
	
	# Save a backup, if the log file already exists
	move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};
	
	open my $fdLog, '>', $args{'log-file'} or die "$args{'log-file'}: $!\n";
	
	if($args{'dyninst'}) {
		&build_dyninst($fdLog, @args{'dyninst-src','dyninst-branch','njobs'});
	}
	&build_tests($fdLog);
	if($args{'run-tests'}) {
		&run_tests($fdLog);
	}
}

sub build_dyninst {
	my ($fdLog, $src_dir, $branch, $njobs) = @_;
	print $fdLog "Building Dyninst($branch)... ";
	
#cd $dyninst_src_dir
#git checkout $dyninst_branch
#cd $build_root
#$dyninst_hash = $(md5sum $dyninst_branch . cur_time)
#$dyninst_build_base_dir = $build_root . '/dyninst/' . $dyninst_hash
#mkdir -p $dyninst_build_base_dir/build
#cd $dyninst_build_base_dir
#echo $dyninst_branch > git.log
#ln -s $(abspath $dyninst_src_dir) src
#cd $dyninst_build_base_dir/build
#[libIberty is broken without ccmake] cmake $dyninst_src_dir -DCMAKE_INSTALL_PREFIX=$dyninst_build_base_dir -DPATH_BOOST=$boost_dir 1>config.out 2>config.err
#make -j8 1>build.out 2>build.err
#make install 1>build-install.out 2>build-install.err
	
	my $build_dir = md5_base64(localtime . $branch);
	$build_dir =~ s|/|_|g;
	
	make_path($build_dir);

	my $res = execute("
		cd $src_dir
		
	");
	if (!$res) {
		print $fdLog "FAILED\n" and die;
	}
	print $fdLog "OK\n";
}

sub build_tests {
#cd $testsuite_src_dir
#git checkout $testsuite_branch
#cd $build_root
#$testsuite_hash = $(md5sum $testsuite_branch . $dyninst_branch . cur_time)
#$testsuite_build_base_dir = $build_root . '/testsuite/' . $testsuite_hash
#mkdir -p $testsuite_build_base_dir/build
#cd $testsuite_build_base_dir
#echo dyninst: $dyninst_branch $dyninst_hash\ntestsuite: $testsuite_branch > git.log
#ln -s $(abspath $testsuite_src_dir) src
#ln -s $(abspath $dyninst_build_base_dir) dyninst
#cd $testsuite_build_base_dir/build
}

sub run_tests {
#!! TEST THESE !!
#cmake $dyninst_src_dir -DINSTALL_DIR=$(pwd)/../tests -DCMAKE_INSTALL_PREFIX=$testsuite_build_base_dir -DPATH_BOOST=$boost_dir 1>config.out 2>config.err
#make -j8 1>build.out 2>build.err
#make install 1>build-install.out 2>build-install.err
#cd ../tests
#export DYNINSTAPI_RT_LIB=$(realpath ../dyninst/lib/libdyninstAPI_RT.so)
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib ./runTests -all -log test.log 1>stdout.log 2>stderr.log
}

sub execute($) {
	my $cmd = shift;
	system($cmd);
	return !(( $? >> 8 ) != 0 || $? == -1 || ( $? & 127 ) != 0);
}

sub parse_log {
#die "Usage: $0 in.log out.res\n" unless @ARGV == 2;
#
#open my $fdIn, '<', $ARGV[0] or die "$!\n";
#open my $fdOut, '>', $ARGV[1] or die "$!\n";
#while(<$fdIn>) {
#	chomp;
#	my @x = unpack('a27 a7 a5 a4 a9 a8 a8 a8 a23');
#	my $status = pop @x;
#	$status = (split(' ', $status))[0];
#	push @x, $status;
#	my @line = map {s/\s+//g; $_;} @x;
#	print $fdOut join(',', @line), "\n";
#}
}
__END__

=head1 DESCRIPTION

A tool for automating building Dyninst and its test suite

=head1 SYNOPSIS

build [options]

 Options:
   --prefix                Base directory for the source and build directories (default: pwd)
   --dyninst-src=PATH      Source directory for Dyninst (default: prefix/dyninst)
   --dyninst-dir=PATH      Path to pre-built Dyninst (only used for --no-dyninst)
   --test-src=PATH         Source directory for Testsuite (default: prefix/testsuite)
   --dyninst-branch=BRANCH Check out git BRANCH for dyninst (default: master)
   --tests-branch=BRANCH   Check out git BRANCH for Testsuite (default: master)
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --njobs=N               Number of make jobs (default: N=1)
   --[no-]dyninst          Build Dyninst (default: yes)
   --[no-]tests            Build the Testsuite (default: yes)
   --[no-]run-tests        Run the tests after building them (default: yes)
   --help                  Print this help message
=cut
