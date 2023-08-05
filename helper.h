#ifndef helper_h


//My data structure is broken up into a graph, which holds any number of branches. These branches hold uncommitted and commits. When the user commits, they transfer the uncommitted files into the committed files of a new commit. As stated, the commits store files, where files contain a hash value, file path and its content it contained when it was tracked. The branch also stores un_tracked files which hold files that were deleted.
struct file {

	int hash;
	char file_path[261];
	char* content;
	int n_content;
};

struct commit {

	char id[7];
	char* message;
	struct commit* prev_commit;
	int n_committed_files;
	struct file** committed_files;

};

struct branch {

	char name[51];
	struct commit* last_commit;

	int n_uncommitted_files;
	struct file **uncommitted_files;

	int n_untracked_files;
	struct file **untracked_files;

};

struct graph {

	struct branch* head;
	int n_branches;
	struct branch** branches;
	int n_commits;
	struct commit** commits;
	int n_files;
	struct file** files;

};

#endif