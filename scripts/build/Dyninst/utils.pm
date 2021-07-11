package Dyninst::utils;

use base 'Exporter';
our @EXPORT_OK = qw(execute list_unique parse_cmake_cache load_from_cache canonicalize make_root upload);

use Capture::Tiny qw(capture);
use Cwd qw(realpath);
use File::Temp qw(tempdir);

our $debug_mode;

sub canonicalize {
	my $dir = shift;
	$dir = realpath($dir) if defined($dir) && $dir ne '';
	return $dir;
}

sub execute {
	my $cmd = shift;

	print "\n$cmd\n" if $debug_mode;

	my ($stdout, $stderr, $exit) = capture { system($cmd); };
	$exit = (($exit >> 8) != 0 || $exit == -1 || ($exit & 127) != 0);
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

sub make_root {
	my $args = shift;

    # XXX would like to check that 'root' and 'restart' are good paths,
    # relying upon developers to be correct for now.
    
	return $args->{'restart'} if $args->{'restart'};

	if (defined($args->{'root'})) {
		unless (-e $args->{'root'} or mkdir $args->{'root'}) {
			die "Unable to create $args->{'root'}\n";
		}
		return $args->{'root'};
	}
	
	# Generate a unique name
	return tempdir('XXXXXXXX', CLEANUP => 0);
}

sub upload {
	my ($filename, $token) = @_;

	try {
		execute("curl --insecure -F \"upload=\@$filename\" "
			  . "-F \"token=$token\" "
			  . "https://bottle.cs.wisc.edu/upload");
	} catch {
		warn "An error occurred when uploading the results\n$_\n";
	}
}

1;
