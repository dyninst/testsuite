use strict;
use warnings;
use Cwd qw(cwd);
use lib cwd();
use Getopt::Long qw(GetOptions);
use File::Copy qw(copy move);
use File::Path qw(make_path);
use Pod::Usage;

#sub build_dyninst {
#	my ($fdLog, $charm_src, $id, $debug, $opts) = @_;
#	print $fdLog "Building ChaNGa($id) using '$debug $opts -j$args{'njobs'}' and Charm++ '$charm_src'... ";
#	
#	my $dest = "$args{'build-dir'}/changa/$id";
#	make_path($dest);
#
#	my $res = execute("
#		cd $dest
#		export CHARM_DIR=\"$charm_src\"
#		$args{'changa-dir'}/configure STRUCT_DIR=structures $opts 1>config.out 2>config.err
#		make $debug -j$args{'njobs'} 1>build.out 2>build.err
#	");
#	if (!$res) {
#		print $fdLog "FAILED\n" and die;
#	}
#	print $fdLog "OK\n";
#}

#-- Main
{
	my %args = (
	'prefix'		=> cwd(),
	'dyninst-src'	=> undef,
	'dyninst-dir'	=> undef,
	'test-src'		=> undef,
	'log-file'      => undef,
	'njobs' 		=> 1,
	'dyninst'		=> 1,
	'tests'			=> 1,
	'run-tests'		=> 1,
	'help' 			=> 0
	);
	
	GetOptions(\%args,
		'prefix=s', 'dyninst-src=s', 'dyninst-dir=s',
		'test-src=s', 'log-file=s', 'njobs=i',
		'dyninst!', 'tests!', 'run-tests!', 'help'
	) or (pod2usage(2), exit);
	
	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99) and exit;
	}
	
	#   --prefix             Base directory for the source and build directories (default: pwd)
	#   --dyninst-src=PATH   Source directory for Dyninst (default: prefix/dyninst/src)
	#   --dyninst-dir=PATH   Path to pre-built Dyninst (only used for --no-dyninst)
	#   --test-src=PATH      Source directory for Testsuite (default: prefix/testsuite/src)
	#   --log-file=FILE      Store logging data in FILE (default: prefix/build.log)
	#   --njobs=N            Number of make jobs (default: N=1)
	#   --[no-]dyninst       Build Dyninst (default: yes)
	#   --[no-]tests         Build the Testsuite (default: yes)
	#	--[no-]run-tests     Run the tests after building them (default: yes)
	#   --help               Print this help message
	
	# Default directory and file locations
	$args{'dyninst-src'} //= "$args{'prefix'}/dyninst/src";
	$args{'dyninst-src'} = "$args{'prefix'}/dyninst";
	$args{'test-src'} //= "$args{'prefix'}/testsuite/src";
	$args{'log-file'} //= "$args{'prefix'}/build.log";
	
	if(!$args{'dyninst'} && !$args{'dyninst-dir'}) {
		print STDERR "Must specify dyninst-dir when not building Dyninst\n";
		pod2usage(2) and exit;
	}
	
	# Save a backup, if the log file already exists
	move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};
	
	use Data::Dumper;
	print Dumper(\%args);
}

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
#
#
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
#
#
#!! TEST THESE !!
#cmake $dyninst_src_dir -DINSTALL_DIR=$(pwd)/../tests -DCMAKE_INSTALL_PREFIX=$testsuite_build_base_dir -DPATH_BOOST=$boost_dir 1>config.out 2>config.err
#make -j8 1>build.out 2>build.err
#make install 1>build-install.out 2>build-install.err
#
#cd ../tests
#export DYNINSTAPI_RT_LIB=$(realpath ../dyninst/lib/libdyninstAPI_RT.so)
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib ./runTests -all -log test.log 1>stdout.log 2>stderr.log

__END__

=head1 DESCRIPTION

A tool for automating building Dyninst and its test suite

=head1 SYNOPSIS

build [options]

 Options:
   --prefix             Base directory for the source and build directories (default: pwd)
   --dyninst-src=PATH   Source directory for Dyninst (default: prefix/dyninst/src)
   --dyninst-dir=PATH   Path to pre-built Dyninst (only used for --no-dyninst)
   --test-src=PATH      Source directory for Testsuite (default: prefix/testsuite/src)
   --log-file=FILE      Store logging data in FILE (default: prefix/build.log)
   --njobs=N            Number of make jobs (default: N=1)
   --[no-]dyninst       Build Dyninst (default: yes)
   --[no-]tests         Build the Testsuite (default: yes)
   --[no-]run-tests     Run the tests after building them (default: yes)
   --help               Print this help message
=cut
