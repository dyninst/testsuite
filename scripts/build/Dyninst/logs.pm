package Dyninst::logs;

use strict;
use warnings;
use base 'Exporter';
our @EXPORT_OK = qw(new parse save_system_info save_compiler_config);

use POSIX;
use Dyninst::utils qw(execute canonicalize);
use File::Copy qw(move);
use Cwd qw(realpath);

# ------------- Module methods -----------------------

sub append_result {
	my ($log_file, $test_name, $result) = @_;
	open my $fdOut, '>>', $log_file or die "$log_file: $!\n";
	print $fdOut pack(
		'A27 A7 A5 A4 A9 A8 A8 A8 A' . length($result),
		$test_name, '', '', '', '', '', '', '', $result
	  ),
	  "\n";
}

sub parse {
	open my $fdIn, '<', $_[0] or die "$_[0]: $!\n";
	my @results;
	while (<$fdIn>) {
		chomp;

		# Parse the fixed-width format
		my @x = unpack('a27 a7 a5 a4 a9 a8 a8 a8 a50');

		# Ignore the header line
		next if $x[0] =~ /^TEST/;

		# Grab the status field (it's at the end)
		my $status = pop @x;
		my $failed = 0;
		for my $s ('FAILED', 'CRASHED') {

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
		if ($status eq 'RESULT,') {
			$status .= 'REASON';
		}

		# Strip the extra whitespace (except for the failure reason)
		@x = map { s/\s+//g; $_; } @x;

		# Add the failure status back to the record
		push @x, $status;

		# Make it a CSV record
		push @results, join(',', @x), "\n";
	}
	return @results;
}

sub get_system_info {
	my %sysinfo = ();

	# Save some information about the system
	@sysinfo{ 'sysname', 'nodename', 'release', 'version', 'machine' } = POSIX::uname();

	# Strip trailing digits from the hostname (these are usually from login nodes)
	$sysinfo{'nodename'} =~ s/\d+$//;

	# Try to get the vendor name
	$sysinfo{'vendor'} = 'Unknown';

	# Use an eval just in case we don't have access to '/proc/cpuinfo'
	eval {
		my $cpuinfo = execute("cat /proc/cpuinfo");
		my @lines   = grep { /vendor_id/i } split("\n", $cpuinfo);
		unless (@lines) {
			@lines = grep { /cpu\s+\:/i } split("\n", $cpuinfo);
		}
		if (@lines) {
			(undef, $sysinfo{'vendor'}) = split(':', $lines[0]);

			# On linux, Power has the form 'POWERXX, altivec...'
			if ($sysinfo{'vendor'} =~ /power/i) {
				$sysinfo{'vendor'} = (split(',', $sysinfo{'vendor'}))[0];
			}

			# Remove all whitespace
			$sysinfo{'vendor'} =~ s/\s//g;
		}
	};

	# Find the version of libc
	my $libc_info = (split("\n", &execute('ldd --version')))[0];
	if ($libc_info =~ /gnu/i || $libc_info =~ /glibc/i) {

		# We have a GNU libc, the version is at the end
		$libc_info = (split(' ', $libc_info))[-1];
	} else {
		$libc_info = "Unknown";
	}
	$sysinfo{'libc'} = $libc_info;

	return \%sysinfo;
}

sub save_system_info {
	my ($logger, $hostname) = @_;

	my $sysinfo = get_system_info();

	# Allow user to override the hostname
	if (defined $hostname) {
		$sysinfo->{'nodename'} = $hostname;
	}

	$logger->write("os: $sysinfo->{'sysname'}\n"
		  . "hostname: $sysinfo->{'nodename'}\n"
		  . "kernel: $sysinfo->{'release'}\n"
		  . "version: $sysinfo->{'version'}\n"
		  . "arch: $sysinfo->{'machine'}/$sysinfo->{'vendor'}\n"
		  . "libc: $sysinfo->{'libc'}");

	# UTC datetime
	$logger->write(POSIX::strftime("date: %Y-%m-%dT%H:%M:%S.\n", gmtime()));
	$logger->write('*' x 20);
}

sub save_compiler_config {
	my ($cmake_log, $out_file) = @_;

	open my $fdIn, '<', $cmake_log or die "Unable to open CMake log '$cmake_log': $!\n";

	my %compilers = (
		'cxx' => { 'path' => '', 'version' => '' },
		'c'   => { 'path' => '', 'version' => '' }
	);

	while (<$fdIn>) {
		if (/Check for working CXX compiler:\s*(.+)?\s*[-]+\s*works/) {
			$compilers{'cxx'}{'path'} = realpath($1);
			next;
		}
		if (/Check for working C compiler:\s*(.+)?\s*[-]+\s*works/) {
			$compilers{'c'}{'path'} = realpath($1);
			next;
		}
		if (/The C compiler identification is (.+)/) {
			$compilers{'c'}{'version'} = $1;
			next;
		}
		if (/The CXX compiler identification is (.+)/) {
			$compilers{'cxx'}{'version'} = $1;
			next;
		}
	}

	# Verify we got everything
	for my $c (keys %compilers) {
		for my $t (keys %{ $compilers{$c} }) {
			unless ($compilers{$c}{$t}) {
				warn "$cmake_log is missing $c/$t\n";
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

	# Save a backup, if the log file already exists
	move($filename, "$filename.bak") if -e $filename;

	open my $fdLog, '>', $filename or die "$filename: $!\n";

	bless {
		'filename' => $filename,
		'quiet'    => $quiet // 1,
		'fd'       => $fdLog
	}, $class;
}

sub write {
	my ($self, $msg, %opts) = @_;

	my $fd  = $self->{'fd'};
	my $eol = $opts{'eol'} // "\n";
	print $fd $msg, $eol;

	if (!$self->{'quiet'}) {
		print $msg, "\n";
	}
}

1;
