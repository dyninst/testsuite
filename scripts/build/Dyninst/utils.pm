package Dyninst::utils;

use base 'Exporter';
our @EXPORT_OK = qw(execute list_unique canonicalize make_root upload);

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
