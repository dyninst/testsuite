package Dyninst::cmake;

use base 'Exporter';
our @EXPORT_OK = qw(load_from_cache);

sub parse_cmake_cache {
	my $filename = shift;
	my %defines  = ();

	open my $fdIn, '<', $filename or die "Unable to open CMake cache '$filename': $!\n";
	while (<$fdIn>) {
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

1;