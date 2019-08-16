package Dyninst::utils;

use base 'Exporter';
our @EXPORT_OK = qw(execute list_unique parse_cmake_cache load_from_cache);

use Capture::Tiny qw(capture);

our $debug_mode;

sub execute {
	my $cmd = shift;

	print "\n$cmd\n" if $debug_mode;

	my ($stdout,$stderr,$exit) = capture { system($cmd); };
	$exit = (( $exit >> 8 ) != 0 || $exit == -1 || ( $exit & 127 ) != 0);
	die "Error executing '$cmd'\n$stderr\n" if $exit;
	return $stdout;
}

sub list_unique {
	my %y;
	@y{@_} = 1;
	return keys %y;
}

sub parse_cmake_cache {
	my $filename = shift;
	my %defines = ();

	open my $fdIn, '<', $filename or die "Unable to open CMake cache '$filename': $!\n";
	while(<$fdIn>) {
		chomp;
		next if /^#/;
		next if /^\/\//;
		next if $_ eq '';

		# Format is KEY:TYPE=VALUE
		my ($key, $value) = split('=');
		($key, undef) = split('\:', $key);
		$defines{$key} = $value;
	}
	return \%defines;
}

sub load_from_cache {
	my ($filename, $var_names) = @_;
	my $cache = parse_cmake_cache($filename);
	map { split(';', $cache->{$_}); } @{$var_names};
}

sub save_compiler_config {
	my ($cmake_log, $out_file) = @_;
	
	open my $fdIn, '<', $cmake_log or die "Unable to open CMake log '$cmake_log': $!\n";
	
	my %compilers = (
		'cxx' => {'path'=>'', 'version'=>''},
		'c'   => {'path'=>'', 'version'=>''}
	);

	while(<$fdIn>) {
		if(/Check for working CXX compiler: (.+)? -- works/) {
			$compilers{'cxx'}{'path'} = $1;
			next;
		}
		if(/Check for working C compiler: (.+)? -- works/) {
			$compilers{'c'}{'path'} = $1;
			next;
		}
		if(/The C compiler identification is (.+)/) {
			$compilers{'c'}{'version'} = $1;
		}
		if(/The CXX compiler identification is (.+)/) {
			$compilers{'cxx'}{'version'} = $1;
		}
	}
	
	open my $fdOut, '>', $out_file or die "Couldn't open '$out_file': $!\n";
	local $, = "\n";
	print $fdOut
		"c_path: $compilers{'c'}{'path'}",
		"c_version: $compilers{'c'}{'version'}",
		"cxx_path: $compilers{'cxx'}{'path'}",
		"cxx_version: $compilers{'cxx'}{'version'}";
}

1;