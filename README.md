# Testsuite for the Dyninst tool for binary instrumentation, analysis, and modification

## Usage

Because Dyninst and its testsuite are tightly integrated, it is highly recommended to use the build script located in `scripts/build/build.pl`.

Example usage on Linux:

	> export PERL5LIB=testsuite/scripts/build
	> perl testsuite/scripts/build/build.pl --njobs=4
	
The build script has several options for configuring library locations. See `build.pl --help` for details.

### Running tests for pull requests

The test script comes with the ability to fetch and update pull requests directly from the Dyninst Github repository. These are controlled through the `--dyninst-pr` and `--testsuite-pr` switches for dyninst and the testsuite, respectively.

The format of the input is *remote/id* where *remote* is the name given to the remote repository (this is usually "origin") and *id* is the pull request id from Github. In the case that the remote's name is 'origin', it can be omitted. Hence, `--dyninst-pr=origin/123` is the same as `--dyninst-pr=123`; the same for `--testsuite-pr`.