package Dyninst::git;

use base 'Exporter';
our @EXPORT_OK = qw(get_config save_config checkout_pr);

use Dyninst::utils qw(execute);

sub get_config {
	my ($src_dir, $base_dir) = @_;
	
	# Fetch the current branch name
	# NB: This will return 'HEAD' if in a detached-head state
	my $branch = execute("cd $src_dir && git rev-parse --abbrev-ref HEAD");
	chomp($branch);

	# Fetch the commitID for HEAD
	my $commit = execute("cd $src_dir && git rev-parse HEAD");
	chomp($commit);
	
	# Get the most recent commits
	# This is useful for seeing the base commits a PR is on top of
	my $recent = execute("cd $src_dir && git log --oneline -10");

	return {'branch'=>$branch, 'commit'=>$commit, 'recent'=>$recent};
}

sub save_config {
	my ($base_dir, $config) = @_;
	
	# Make each output line a standard CSV entry
	my $recent = join(',', map {s/"/\\"/g; "\"$_\""} split("\n", $config->{'recent'}));
	
	open my $fdOut, '>', "$base_dir/git.log" or die "$base_dir/git.log: $!";
	local $, = "\n";
	local $\ = "\n";
	print $fdOut "branch: $config->{'branch'}",
				 "commit: $config->{'commit'}",
				 "recent: $recent";
}

sub checkout_pr {
	my ($src_dir, $pr, $current_branch) = @_;
	
	# The PR format is 'remote/ID'
	my ($remote, $id) = split('/', $pr);
	if(!defined($id)) {
		# The user only specified an ID, so assume the remote is 'origin'
		$id = $remote;
		$remote = 'origin';
	}
	
	# The branch we want to create for the PR
	my $target_branch = "PR$id";
	
	eval {
		if($target_branch eq $current_branch) {
			# Just pull any changes from the remote
			&execute(
				"cd $src_dir \n" .
				"git pull $remote pull/$id/head \n"
			);
		} else {
			# Check if the target branch exists
			my $target_exists = undef;
			eval{ &execute("cd $src_dir && git checkout $target_branch"); };
			$target_exists = 1 unless $@;
			
			if($target_exists) {
				# Do a checkout/pull
				&execute(
					"cd $src_dir \n" .
					"git checkout $target_branch \n" .
					"git pull $remote pull/$id/head \n"
				);
			} else {
				# Do a fetch/checkout
				my $stdout = &execute(
					"cd $src_dir \n" .
					"git fetch $remote pull/$id/head:$target_branch \n" .
					"git checkout $target_branch \n" .
					"git merge --squash -Xignore-all-space $remote/master \n"
				);
				
				# Running 'git commit' with nothing to commit returns a non-zero value.
				# I think this is a bug in git, but just work around it for now.
				if($stdout !~ /Already up[- ]*to[- ]*date/i) {
					execute(
						"cd $src_dir \n" .
						"git commit -m 'Merge $remote/master'"
					);
				}
			}
		}
	};
	if($@) {
		my $msg = $@;
		$msg =~ s/\n/\n\t/g;
		die "\nERROR: Unable to checkout pull request '$remote/$id'\n\n\t$msg\n";
	}
}

1;