# Testsuite for the Dyninst tool for binary instrumentation, analysis, and modification

## Usage

Because Dyninst and its testsuite are tightly integrated, it is highly recommended to use the build script located in `scripts/build/build.pl`.

Example usage on Linux:

	> export PERL5LIB=testsuite/scripts/build
	> perl testsuite/scripts/build/build.pl --njobs=4
	
The build script has several options for configuring library locations. See `build.pl --help` for details.
