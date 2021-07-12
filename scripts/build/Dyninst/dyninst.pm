package Dyninst::dyninst;

use strict;
use warnings;

use base 'Exporter';
our @EXPORT_OK = qw(run);

use Dyninst::utils qw(execute canonicalize);
use Dyninst::git;
use Dyninst::logs qw(save_compiler_config);
use Cwd qw(realpath);
use File::Path qw(make_path);
use Try::Tiny;

sub setup {
	my ($root_dir, $args) = @_;

	# Build Dyninst
	# Create the build directory
	make_path("$root_dir/dyninst/build");

	# The path must exist before using 'realpath'
	my $base_dir  = realpath("$root_dir/dyninst");
	my $build_dir = "$base_dir/build";

	$args->{'dyninst-src'} //= "$args->{'prefix'}/dyninst";
	$args->{'dyninst-src'} = canonicalize($args->{'dyninst-src'});

	symlink($args->{'dyninst-src'}, "$base_dir/src");

	# This is for internal use only
	$args->{'dyninst-cmake-cache-dir'} = $build_dir;

	my $git_config = Dyninst::git::get_config($args->{'dyninst-src'}, $base_dir);

	# Check out the PR, if specified
	if ($args->{'dyninst-pr'}) {
		Dyninst::git::checkout_pr($args->{'dyninst-src'}, $args->{'dyninst-pr'}, $git_config->{'branch'});
		$git_config = Dyninst::git::get_config($args->{'dyninst-src'}, $base_dir);
	}

	Dyninst::git::save_config($base_dir, $git_config);

	return ($base_dir, $build_dir);
}

sub configure {
	my ($args, $base_dir, $build_dir) = @_;

	my $sterile = $args->{'sterile'} ? '-DSTERILE_BUILD=ON' : '';

	# Configure the build
	# We need an 'eval' here since we are manually piping stderr
	eval {
		execute("cd $build_dir\n"
			  . "$args->{'cmake'} "
			  . "-H$base_dir/src "
			  . "-B$build_dir "
			  . "$args->{'cmake-args'} "
			  . "$args->{'dyninst-cmake-args'} "
			  . "-DCMAKE_INSTALL_PREFIX=$base_dir "
			  . "$sterile "
			  . "1>config.out 2>config.err ");
	};
	die "Error configuring: see $build_dir/config.err for details" if $@;
}

sub build {
	my ($args, $build_dir) = @_;

	my $njobs = $args->{'njobs'};

	# Run the build
	# We need an 'eval' here since we are manually piping stderr
	eval { execute("cd $build_dir\n" . "make VERBOSE=1 -j$njobs 1>build.out 2>build.err"); };
	die "Error building: see $build_dir/build.err for details" if $@;

	# Install
	# We need an 'eval' here since we are manually piping stderr
	eval { execute("cd $build_dir\n" . "make VERBOSE=1 install 1>build-install.out 2>build-install.err"); };
	die "Error installing: see $build_dir/build-install.err for details" if $@;
}

sub run {
	my ($args, $root_dir, $logger) = @_;

	# Always set up logs, even if doing a restart
	my ($base_dir, $build_dir) = setup($root_dir, $args);

	return 1 unless $args->{'build-dyninst'};

	my $error = 0;

	try {
		$logger->write("Configuring Dyninst... ", 'eol' => '');
		configure($args, $base_dir, $build_dir);
		$logger->write("done.");

		save_compiler_config("$build_dir/config.out", "$base_dir/build/compilers.conf");

		$logger->write("Building Dyninst... ", 'eol' => '');
		build($args, $build_dir);
		$logger->write("done.");
	} catch {
		$logger->write($_);
		open my $fdOut, '>', "$root_dir/dyninst/Build.FAILED";
		$error = 1;
	}

	return !$error;
}

1;
