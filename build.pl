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
	if($args{'tests'}) {
		&build_tests($fdLog);
	}
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
#$dyninst_build_base_dir = $build_root . $dyninst_hash
#mkdir -p $dyninst_build_base_dir/build
#cd $dyninst_build_base_dir
#echo $dyninst_branch > git.log
#ln -s $(abspath $dyninst_src_dir) src
#cd $dyninst_build_base_dir/build
#[libIberty is broken without ccmake] cmake $dyninst_src_dir -DCMAKE_INSTALL_PREFIX=$dyninst_build_base_dir -DPATH_BOOST=$boost_dir 1>config.out 2>config.err
#make -j8 1>build.out 2>build.err
#make install 1>build-install.out 2>build-install.err
# BEGIN -- HACK GARBAGE --
#cd ../lib
#ln -s ../build/elfutils/lib/libdw.so libdw.so
#ln -s ../build/elfutils/lib/libdw.so.1 libdw.so.1
# END -- HACK GARBAGE --

	
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
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib:$(realpath ../dyninst/build/boost/src/boost/stage/lib) ./runTests -all -log test.log 1>stdout.log 2>stderr.log
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

#use DBI;
#use DBD::SQLite;
## Open a database handle to a SQLite database
#my $dbh = DBI->connect('DBI:SQLite:dbname=:memory:');
#
## Make available extra functions like decode, stddev, et al. to SQLite
#sqlite_more($dbh);
#
## Add a concat function (it's available in neither SQLite nor SQLite::More)
#$dbh->sqlite_create_function('concat',-1,sub { join('',@_); });
#
## Set the fetched names to lower case; important: a necessary flag to conserve
## variable names produced by SQL independent of case
#$dbh->{'FetchHashKeyName'} = 'NAME_lc';
#
## Import the CSV file into the table 'morph_csv'
#&importCSV($infilename,$dbh,'morph_csv');
#
## Create the decode tables
#&createTable($dbh,\%patch_weight_decode);
#&createTable($dbh,\%clump_weight_decode);
#
#### Define counts and weights used for collapsing raw values into single 
#### values per object.
#### ------------------
## DBI operation (prepare) to make a table named morph_tbl using SQL CREATE and
## SELECT operators which are grouped by object TFIT ID. 
## See MyNoteBook for SQL syntax to DECODE operator;
## SUM addes entries in table column, COUNT(1) counts rows in group
#my $sth = $dbh->prepare(<<END
#SELECT
#END
#);
#
#$sth->execute() or die $sth->errstr();
##while (my $row = $sth->fetchrow_hashref){
#	#print "$row->{'id_tfit'},$row->{'nummm'},$row->{'clumpweightsum'},$row->{'patchweightsum'}\n";
##}
## cleanup
#$sth->finish();
#$dbh->disconnect();
## Create a SQL table from a hash with a table struct (cols,types,vals)
#sub createTable(){
#	my ($dbh,$table,$opt) = @_;
#	
#	my $str = "CREATE TABLE $table->{'name'} (";
#	$str .= "$table->{'cols'}->[0] $table->{'types'}->[0]";
#	
#	for my $i (1..@{$table->{'cols'}}-1) {
#		$str .= ",$table->{'cols'}->[$i] $table->{'types'}->[$i]";
#	}
#	$str .= ")";
#	
#	$dbh->do($str);
#
#	$str = "INSERT INTO $table->{'name'} (" . join(',', @{$table->{'cols'}}) . ") VALUES ( " .  
#				join(',',('?') x scalar @{$table->{'cols'}}) . " )";
#	
#	my $sth = $dbh->prepare($str);
#	$dbh->begin_work();
#	
#	if ($table->{'vals'}) {
#		for my $row (@{$table->{'vals'}}) {
#			$sth->execute(@$row);
#		}
#	} elsif ($opt && $opt->{'loader'}){
#		&{$opt->{'loader'}}($sth);
#	} else {
#		die "Table $table->{'name'} has no values to insert!\n";
#	}
#	
#	$sth->finish();
#	$dbh->commit();
#}
#
## Import a CSV file into a SQLite database
#sub importCSV($$$){
#	my ($fileName,$dbh,$tableName) = @_;
#	
#	use Text::CSV;
#	my $csv = Text::CSV->new({'binary'=>1});
#	open my $fd, '<', $fileName or die "Unable to open file $fileName: $!\n";
#	$csv->column_names($csv->getline($fd));
#	my $row = $csv->getline($fd);
#	my @datatypes = ();
#	
#	use 5.10.1; # for given/when
#	
#	# Use the values in the first row to determine the data types for each column
#	# This is arbitrary and somewhat dangerous. For example, if a column is really
#	# a 'float' type, but the first row has a 0 instead of 0.0, the type will be inferred
#	# as 'int'.
#	for my $val (@$row) {
#		given( $val ) {
#			when( /^[+-]?\d+\z/ ){ push @datatypes, 'INTEGER';}
#			when( /^[+-]?(?=\.?\d)\d*\.?\d*(?:e[+-]?\d+)?\z/i) { push @datatypes, 'REAL';}
#			default {push @datatypes, 'TEXT';}
#		}
#	}
#	
#	use List::MoreUtils qw(first_index);
#	
#	# Make sure the comments are stored as strings
#	$datatypes[first_index {uc $_ eq 'COMMENT'} $csv->column_names()] = 'TEXT';
#	
#	# make a table structure to pass to createTable
#	my %csvTable = (
#		'name'=>$tableName,
#		'cols'=>[$csv->column_names()],
#		'types'=>\@datatypes,
#		'vals'=>undef
#	);
#	
#	# Build the table
#	&createTable($dbh,\%csvTable,{'loader'=>sub {
#		my $sth = shift;
#		while($row) {
#			$sth->execute(@$row);
#			$row = $csv->getline($fd);
#		}
#	}});
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
   --[no-]run-tests        Run the tests (default: yes)
   --help                  Print this help message
=cut
