package Dyninst::utils;

use base 'Exporter';
our @EXPORT_OK = qw(execute list_unique parse_cmake_cache);

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

1;