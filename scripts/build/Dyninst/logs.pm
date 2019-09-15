package Dyninst::logs;

use base 'Exporter';
our @EXPORT_OK = qw(save_system_info parse write);

use POSIX;
use Dyninst::utils qw(execute);

sub parse {
	open my $fdIn, '<', $_[0] or die "$_[0]: $!\n";
	my @results;
	while(<$fdIn>) {
		chomp;
		
		# Parse the fixed-width format
		my @x = unpack('a27 a7 a5 a4 a9 a8 a8 a8 a23');

		# Grab the status field (it's at the end)
		my $status = pop @x;
		my $failed = 0;
		for my $s ('FAILED','CRASHED') {
			# Split the status from the reason
			# The format is 'FAILED (reason)'
			if ($status =~ /$s\s*\((.+)\)\s*$/) {
				$status = "$s,$1";
				$failed = 1;
			}
		}
		if (!$failed) {
			# Add an empty field to the CSV since there
			# isn't a failure reason here
			$status .= ',';
		}
		
		# Update the header line
		if($status eq 'RESULT,') {
			$status .= 'REASON';
		}
		
		# Strip the extra whitespace (except for the failure reason)
		@x = map {s/\s+//g; $_;} @x;
				
		# Add the failure status back to the record
		push @x, $status;
		
		# Make it a CSV record
		push @results, join(',', @x), "\n";
	}
	return @results;
}

sub write {
	my ($fd, $echo_stdout, $msg) = @_;

	print $fd $msg;

	if($echo_stdout) {
		print $msg;
	}
}

sub save_system_info {
	my ($args, $fdLog) = @_;
	
	# Save some information about the system
	my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
	    
	# Strip trailing digits from the hostname (these are usually from login nodes)
	$nodename =~ s/\d+$//;
	
	# Save the hostname so the caller can use it
	$args->{'hostname'} = $nodename;
	
	# Try to get the vendor name
	my $vendor_name = 'unknown';
	
	# Use an eval just in case we don't have access to '/proc/cpuinfo'
	eval {
		my $cpuinfo = execute("cat /proc/cpuinfo");
		my @lines = grep {/vendor_id/i} split("\n", $cpuinfo);
		unless(@lines) {
			@lines = grep {/cpu\s+\:/i} split("\n", $cpuinfo);
		}
		if(@lines) {
			(undef, $vendor_name) = split(':', $lines[0]);
			
			# On linux, Power has the form 'POWERXX, altivec...'
			if($vendor_name =~ /power/i) {                
				$vendor_name = (split(',',$vendor_name))[0];
			}
			
			# Remove all whitespace
			$vendor_name =~ s/\s//g;
		}
	};
	
	Dyninst::logs::write($fdLog, !$args->{'quiet'},
		"os: $sysname\n" .
		"hostname: $nodename\n" .
		"kernel: $release\n" .
		"version: $version\n" .
		"arch: $machine/$vendor_name\n"
	);
	
	# Find and save the version of libc
	my $libc_info = (split("\n", &execute('ldd --version')))[0];
	if($libc_info =~ /gnu/i || $libc_info =~ /glibc/i) {
		# We have a GNU libc, the version is at the end
		$libc_info = (split(' ', $libc_info))[-1];
	} else {
		$libc_info = "Unknown";
	}
	Dyninst::logs::write($fdLog, !$args->{'quiet'}, "libc: $libc_info\n");

	# UTC datetime	
	Dyninst::logs::write($fdLog, !$args->{'quiet'}, POSIX::strftime("date: %Y-%m-%dT%H:%M:%S.\n", gmtime()));
	
	Dyninst::logs::write($fdLog, !$args->{'quiet'}, '*'x20 . "\n");
}

1;