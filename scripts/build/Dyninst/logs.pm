package Dyninst::logs;

use base 'Exporter';
our @EXPORT_OK = qw(new parse save_system_info save_compiler_info);

use POSIX;
use Dyninst::utils qw(execute);
use File::Copy qw(move);

# ------------- Module methods -----------------------

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

sub save_system_info {
	my ($logger) = @_;
	
	# Save some information about the system
	my ($sysname, $nodename, $release, $version, $machine) = POSIX::uname();
	    
	# Strip trailing digits from the hostname (these are usually from login nodes)
	$nodename =~ s/\d+$//;
		
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
	
	$logger->write(
		"os: $sysname\n" .
		"hostname: $nodename\n" .
		"kernel: $release\n" .
		"version: $version\n" .
		"arch: $machine/$vendor_name"
	);
	
	# Find and save the version of libc
	my $libc_info = (split("\n", &execute('ldd --version')))[0];
	if($libc_info =~ /gnu/i || $libc_info =~ /glibc/i) {
		# We have a GNU libc, the version is at the end
		$libc_info = (split(' ', $libc_info))[-1];
	} else {
		$libc_info = "Unknown";
	}
	
	$logger->write("libc: $libc_info");

	# UTC datetime	
	$logger->write(POSIX::strftime("date: %Y-%m-%dT%H:%M:%S.\n", gmtime()));
	$logger->write('*'x20);
	
	# Return the hostname so the caller can use it
	return $nodename;	
}

sub save_compiler_info {
	my ($cmake_log, $out_file) = @_;
	
	open my $fdIn, '<', $cmake_log or die "Unable to open CMake log '$cmake_log': $!\n";
	
	my %compilers = (
		'cxx' => {'path'=>'', 'version'=>''},
		'c'   => {'path'=>'', 'version'=>''}
	);

	while(<$fdIn>) {
		if(/Check for working CXX compiler: (.+)? -- works/) {
			$compilers{'cxx'}{'path'} = realpath($1);
			next;
		}
		if(/Check for working C compiler: (.+)? -- works/) {
			$compilers{'c'}{'path'} = realpath($1);
			next;
		}
		if(/The C compiler identification is (.+)/) {
			$compilers{'c'}{'version'} = $1;
			next;
		}
		if(/The CXX compiler identification is (.+)/) {
			$compilers{'cxx'}{'version'} = $1;
			next;
		}
	}
	
	# Verify we got everything
	for my $c (keys %compilers) {
		for my $t (keys %{$compilers{$c}}) {
			unless($compilers{$c}{$t}) {
				die "$cmake_log is missing $c/$t\n";
			}
		}
	}
	
	open my $fdOut, '>', $out_file or die "Couldn't open '$out_file': $!\n";
	local $, = "\n";
	print $fdOut
		"c_path: $compilers{'c'}{'path'}",
		"c_version: $compilers{'c'}{'version'}",
		"cxx_path: $compilers{'cxx'}{'path'}",
		"cxx_version: $compilers{'cxx'}{'version'}\n";
}

# ------------- Class methods -------------------------

sub new {
	my ($class, $filename, $quiet) = @_;
	
	open my $fdLog, '>', $filename or die "$filename: $!\n";
	
	# Save a backup, if the log file already exists
	move($filename, "$filename.bak") if -e $filename;
	
	bless {
		'filename' => $filename,
		'quiet'=> $quiet,
		'fd' => $fdLog
	}, $class;
}

sub write {
	my ($self, $msg, %opts) = @_;

	my $fd = $self->{'fd'};
	my $eol = $opts{'eol'} // "\n";
	print $fd $msg, $eol;

	if(!$self->{'quiet'}) {
		print $msg, "\n";
	}
}

1;