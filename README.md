# Testsuite for the Dyninst tool for binary instrumentation, analysis, and modification

## Usage

Because Dyninst and its testsuite are tightly integrated, it is highly recommended to use the build script located in `scripts/build/build.pl`.

Example usage on Linux:

	> export PERL5LIB=testsuite/scripts/build
	> perl testsuite/scripts/build/build.pl --njobs=4
	
The build script has several options for configuring library locations. See `build.pl --help` for details.

### Running tests for pull requests

By default, the build script uses the current state of the repositories contained in the directories specified by `--dyninst-src` and `--test-src`. The test script comes with the ability to fetch and update pull requests directly from the Dyninst Github repository. These are controlled through the `--dyninst-pr` and `--testsuite-pr` switches for dyninst and the testsuite, respectively. Note, however, that if the current git tree has any outstanding changes, the script will raise an error accordingly.

The format of the input is *remote/id* where *remote* is the name given to the remote repository (this is usually "origin") and *id* is the pull request id from Github. In the case that the remote's name is 'origin', it can be omitted. Hence, `--dyninst-pr=origin/123` is the same as `--dyninst-pr=123`; the same for `--testsuite-pr`.

The build script will also update the pull request (via a squashed merge commit) to the current HEAD of the remote's `master` branch the first time the PR is tested locally. As such, this feature should only be used to test PRs against the official Dyninst repository. This process creates a local branch name `PR<ID>` where `<ID>` is the *id* of the PR specified on the command line. If this local branch already exists, then a `pull` is issued and no merge commit against the remote's master is performed. This can cause trouble if a local branch with that name already exists.

	NOTE: It is best not to have any local branches with names of the form 'PR<ID>' when using the build script

