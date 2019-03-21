use strict;
use warnings;
use Cwd qw(cwd realpath);
use lib cwd();
use Getopt::Long qw(GetOptions);
use File::Copy qw(copy move);
use File::Path qw(make_path);
use Digest::MD5 qw(md5_base64);
use Pod::Usage;
use Capture::Tiny qw(capture);

#-- Main
{
	my %args = (
	'prefix'				=> cwd(),
	'dyninst-dir'			=> undef,
	'dyninst-branch' 		=> undef,
	'dyninst-relative-to'	=> 'master',
	'test-src'				=> undef,
	'test-branch'			=> undef,
	'test-relative-to'		=> 'master',
	'boost-dir'				=> undef,
	'boost-inc'				=> undef,
	'boost-lib'				=> undef,
	'log-file'      		=> undef,
	'njobs' 				=> 1,
	'dyninst'				=> 1,
	'tests'					=> 1,
	'run-tests'				=> 1,
	'help' 					=> 0
	);
	
	GetOptions(\%args,
		'prefix=s', 'dyninst-dir=s',
		'dyninst-branch=s', 'dyninst-relative-to=s',
		'test-src=s', 'test-branch=s', 'test-relative-to=s',
		'boost-dir=s', 'boost-inc=s', 'boost-lib=s',
		'log-file=s', 'njobs=i', 'dyninst!', 'tests!',
		'run-tests!', 'help'
	) or (pod2usage(2), exit);
	
	if($args{'help'}) {
		pod2usage(-exitval => 0, -verbose => 99) and exit;
	}
	
	# Default directory and file locations
	$args{'dyninst-dir'} //= "$args{'prefix'}/dyninst";
	$args{'test-src'} //= "$args{'prefix'}/testsuite";
	$args{'log-file'} //= "$args{'prefix'}/build.log";

	# Boost directories
	$args{'boost-inc'} = "$args{'boost-dir'}/include" if $args{'boost-dir'};
	$args{'boost-lib'} = "$args{'boost-dir'}/lib" if $args{'boost-dir'};
	
	if(!$args{'dyninst'} && !$args{'dyninst-dir'}) {
		print STDERR "Must specify dyninst-dir when not building Dyninst\n";
		pod2usage(2) and exit;
	}
	
	# Canonicalize user-specified files and directories
	for my $d ('dyninst-dir','test-src','log-file','boost-inc','boost-lib') {
		# NB: realpath(undef) eq cwd()
		$args{$d} = realpath($d) if defined($args{$d});
	}
	
	# Save a backup, if the log file already exists
	move($args{'log-file'}, "$args{'log-file'}.bak") if -e $args{'log-file'};
	
	open my $fdLog, '>', $args{'log-file'} or die "$args{'log-file'}: $!\n";
	
	# Build Dyninst, if requested
	if($args{'dyninst'}) {
		print $fdLog "Building Dyninst($args{'dyninst-branch'})... ";
		eval { &build_dyninst(\%args); };
		print $fdLog $@ and die $@ if $@;
	}

	# Build the test suite, if requested
	if($args{'tests'}) {
		&build_tests($fdLog);
		print $fdLog $@ and die $@ if $@;
	}

	# Run the tests, if requested
	if($args{'run-tests'}) {
		&run_tests($fdLog);
		print $fdLog $@ and die $@ if $@;
	}
}

sub build_dyninst {
	my ($args) = @_;
	
	my $src_dir = $args->{'dyninst-dir'};
	my $branch = $args->{'dyninst-branch'};
	my $boost_inc = $args->{'boost-inc'} // "";
	my $boost_lib = $args->{'boost-lib'} // "";
	my $njobs = $args->{'njobs'};
	my $rel_branch = $args->{'dyninst-relative-to'};
	
	# Generate a unique name for the current build
	my $build_dir = md5_base64(localtime . $branch);
	$build_dir =~ s|/|_|g;
	
	# Create the build directory
	make_path("$build_dir/build");
	
	# Create symlink to source
	symlink("$src_dir", "$build_dir/src");
	
	# If branch is specified, check it out in the source
	if($branch) {
		execute("
			cd $src_dir
			git checkout $branch
		");
		
		# Save the branch information
		execute("
			cd $src_dir
			echo $branch > $build_dir/git.log
			git log --oneline $rel_branch..$branch >> $build_dir/git.log
		");
	}
	
	# Configure the build
	execute("
		cd build
		cmake $src_dir -DCMAKE_INSTALL_PREFIX=$(realpath ..) 
	") or die "Configuring $build_dir failed";
	
	execute("
		cd $build_dir/build
		make -j$njobs 1>build.out 2>build.err"
	) or die "Building $build_dir failed";

	execute("make install 1>build-install.out 2>build-install.err") or die "Installing in $build_dir failed";

#		cd ../lib
#		ln -s ../build/elfutils/lib/libdw.so libdw.so
#		ln -s ../build/elfutils/lib/libdw.so.1 libdw.so.1
#	");

	#[libIberty is broken without ccmake]
#LD_LIBRARY_PATH=/usr/local/lib/boost-1.69/lib make -j8
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
#Dyninst_DIR=../dyninst/lib/cmake/Dyninst ccmake ../src -DCMAKE_INSTALL_PREFIX=$(realpath ..) -DINSTALL_DIR=$(realpath ../tests) -DBoost_INCLUDE_DIR=$(realpath ../dyninst/build/boost/src/boost)
#Dyninst_DIR=../dyninst/lib/cmake/Dyninst ccmake ../src -DCMAKE_INSTALL_PREFIX=$(realpath ..) -DINSTALL_DIR=$(realpath ../tests) -DBoost_INCLUDE_DIR=/usr/local/lib/boost-1.69/include
#LD_LIBRARY_PATH=$(realpath ../dyninst/build/boost/src/boost/stage/lib) make -j8
#LD_LIBRARY_PATH=/usr/local/lib/boost-1.69/lib make -j8
}

sub run_tests {
#export DYNINSTAPI_RT_LIB=$(realpath ../dyninst/lib/libdyninstAPI_RT.so)
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib:$(realpath ../dyninst/build/boost/src/boost/stage/lib) ./runTests -all -log test.log 1>stdout.log 2>stderr.log
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib:/usr/local/lib/boost-1.69/lib ./runTests -all -log test.log 1>stdout.log 2>stderr.log
#LD_LIBRARY_PATH=$(pwd):../dyninst/lib ./runTests -all -log test.log 1>stdout.log 2>stderr.log
}

sub execute($) {
	my $cmd = shift;
	my ($stdout,$stderr,$exit) = capture { system($cmd); };
	$exit = (( $exit >> 8 ) != 0 || $exit == -1 || ( $exit & 127 ) != 0);
	die "Error executing '$cmd'\n$stderr\n" if $exit;
	return $stdout;
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
   --boost-dir=PATH        Base directory for Boost
   --boost-inc=PATH        Include directory for Boost (ignored if --boost-dir is given)
   --boost-lib=PATH        Library directory for Boost (ignore if --boost-lib is given)
   --dyninst-branch=BRANCH Check out git BRANCH for dyninst
   --tests-branch=BRANCH   Check out git BRANCH for Testsuite
   --log-file=FILE         Store logging data in FILE (default: prefix/build.log)
   --njobs=N               Number of make jobs (default: N=1)
   --[no-]dyninst          Build Dyninst (default: yes)
   --[no-]tests            Build the Testsuite (default: yes)
   --[no-]run-tests        Run the tests (default: yes)
   --help                  Print this help message
=cut
