use strict;
use warnings;

die "Usage: $0 in.log out.res\n" unless @ARGV == 2;

open my $fdIn, '<', $ARGV[0] or die "$!\n";
open my $fdOut, '>', $ARGV[1] or die "$!\n";
while(<$fdIn>) {
	chomp;
	my @x = unpack('a27 a7 a5 a4 a9 a8 a8 a8 a23');
	my $status = pop @x;
	$status = (split(' ', $status))[0];
	push @x, $status;
	my @line = map {s/\s+//g; $_;} @x;
	print $fdOut join(',', @line), "\n";
}
