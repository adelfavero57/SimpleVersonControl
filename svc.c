#include "svc.h"


void *svc_init(void) {

    struct graph *g = (struct graph *)malloc(sizeof(struct graph) * 1);
    struct branch *b = (struct branch *)malloc(sizeof(struct branch) * 1);

    b->last_commit = NULL;
    strcpy(b->name, "master");
    b->n_uncommitted_files = 0;
    b->n_untracked_files = 0;
    b->uncommitted_files = (struct file **)malloc(sizeof(struct file *) * 0);
    b->untracked_files = (struct file **)malloc(sizeof(struct file *) * 0);

    g->head = b;
    g->n_branches = 1;
    g->branches = (struct branch **)malloc(sizeof(struct branch *) * 1);
    g->branches[0] = b;
    //commits and files lists are made to avoid heap-use-after free and for ease of cleanup.
    g->n_commits = 0;
    g->commits = (struct commit **)malloc(sizeof(struct commit *) * 0);
    g->n_files = 0;
    g->files = (struct file **)malloc(sizeof(struct file *) * 0);

    return (void *)g;
}

//checks whether a given file is in a given branch's uncommitted files
int check_for_file_in_uncommitted(struct branch *head, char *file_name) {
    for (int i = 0; i < head->n_uncommitted_files; ++i) {
        if (strcmp(head->uncommitted_files[i]->file_path, file_name) == 0) {
            return i;
        }
    }

    return -1;
}

//checks wether a give file is in a given branch's last commit's committed file
int check_for_file_in_committed(struct branch *head, char *file_name) {
    for (int i = 0; i < head->last_commit->n_committed_files; ++i) {
        if (strcmp(head->last_commit->committed_files[i]->file_path,
                   file_name) == 0) {
            return i;
        }
    }

    return -1;
}

int svc_add(void *helper, char *file_name) {

    if (file_name == NULL) {
        return -1;
    }

    //checking if file has already been added
    struct graph *g = (struct graph *)helper;
    for (int i = 0; i < g->head->n_uncommitted_files; ++i) {
        if (strcmp(g->head->uncommitted_files[i]->file_path, file_name) == 0) {
            return -2;
        }
    }

    //if the current commit is not the first, then check if file has alreadt been committed
    if (g->head->last_commit != NULL){
        for (int i = 0; i < g->head->last_commit->n_committed_files; ++i) {
            if (strcmp(g->head->last_commit->committed_files[i]->file_path,
                       file_name) == 0) {
                return -2;
            }
        }
    }

    FILE *x = fopen(file_name, "rb");
    if (x == NULL) {
        return -3;
    }
    fclose(x);

    (g->head->n_uncommitted_files)++;
    if (g->head->uncommitted_files == NULL) {
        g->head->uncommitted_files =
            (struct file **)malloc(sizeof(struct file *) * 1);
    }
    else {
      g->head->uncommitted_files = (struct file **)realloc(
          g->head->uncommitted_files,
          sizeof(struct file *) * g->head->n_uncommitted_files);
    }

    struct file *f = (struct file *)malloc(sizeof(struct file) * 1);
    int file_len;
    char *buffer = read_byte_file(file_name, &file_len);
    f->content = (char *)malloc(sizeof(char) * file_len);
    strncpy(f->content, buffer, file_len);
    f->n_content = file_len;
    free(buffer);

    g->head->uncommitted_files[g->head->n_uncommitted_files - 1] = f;
    strcpy(
        g->head->uncommitted_files[g->head->n_uncommitted_files - 1]->file_path,
        file_name);
    g->head->uncommitted_files[g->head->n_uncommitted_files - 1]->hash =
        hash_file(helper, file_name);

    (g->n_files)++;
    g->files =
        (struct file **)realloc(g->files, sizeof(struct file *) * g->n_files);
    g->files[g->n_files - 1] = f;

    return g->head->uncommitted_files[g->head->n_uncommitted_files - 1]->hash;
}

//storing the lists of all allocated memory allows for trivial cleanup
void cleanup(void *helper) {
    struct graph *g = (struct graph *)helper;

    for (int i = 0; i < g->n_branches; ++i) {
        free(g->branches[i]->uncommitted_files);
        free(g->branches[i]->untracked_files);
        free(g->branches[i]);
    }
    free(g->branches);

    for (int i = 0; i < g->n_commits; i++) {
        free(g->commits[i]->committed_files);
        free(g->commits[i]->message);
        free(g->commits[i]);
    }
    free(g->commits);

    for (int i = 0; i < g->n_files; i++) {
        free(g->files[i]->content);
        free(g->files[i]);
    }

    free(g->files);
    free(g);
}
/*
USYD CODE CITATION ACKNOWLEDGEMENT
  I declare that the following lines of code have been copied from the website titled: "Read file byte by byte using fread"

  Original URL
      https://stackoverflow.com/questions/28269995/c-read-file-byte-by-byte-using-fread
*/

char *read_byte_file(char *file_path, int *file_len) {
    FILE *ptr;
    char *buffer;

    ptr = fopen(file_path, "rb");
    fseek(ptr, 0, SEEK_END);
    *file_len = ftell(ptr);
    rewind(ptr);

    buffer = (char *)malloc(sizeof(char) *
                            (*file_len));
    fread(buffer, *file_len, 1, ptr);
    fclose(ptr);
    return buffer;
}

int hash_file(void *helper, char *file_path) {
    if (file_path == NULL) {
        return -1;
    }

    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        return -2;
    }
    fclose(f);

    int file_len;
    char *file_contents = read_byte_file(file_path, &file_len);
    int hash = 0;
    for (size_t i = 0; i < strlen(file_path); ++i){

        hash = (hash + (unsigned char)file_path[i]) % 1000;
    }

    for (int i = 0; i < file_len; ++i){

        hash = (hash + (unsigned char)file_contents[i]) % 2000000000;
    }

    free(file_contents);
    return hash;
}

void reverse(char *x, int begin, int end) {
    char c;

    if (begin >= end){
      return;
    }

    c = *(x + begin);
    *(x + begin) = *(x + end);
    *(x + end) = c;

    reverse(x, ++begin, --end);
}

/**
  USYD CODE CITATION ACKNOWLEDGEMENT
    I declare that the following lines of code have been copied from the website titled:
      How can I convert from decimal to hexadecimal in C
  Original URL:
 * https://www.quora.com/How-can-I-convert-from-decimal-to-hexadecimal-in-C-language
 Last accessed 3/5/2020
 */
char *convert_dec_hex(int decimal) {
    int quotient = decimal;
    char *hexadecimal = (char *)malloc(sizeof(char) * 7);
    for (int i = 0; i < 6; ++i) hexadecimal[i] = '0';
    hexadecimal[6] = '\0';
    int j = 0;

    while (quotient != 0) {

        int remainder = quotient % 16;
        if (remainder < 10){
            hexadecimal[j++] = 48 + remainder;
        }
        else{
            hexadecimal[j++] = 55 + remainder;
        }
        quotient = quotient / 16;
    }

    reverse(hexadecimal, 0, 5);
    for (int i = 0; i < 6; ++i) {
        if (hexadecimal[i] >= 'A' && hexadecimal[i] <= 'Z') {
            hexadecimal[i] += 32;
        }
    }
    return hexadecimal;
}


int is_letter(char x) {

    if (x >= 'a' && x <= 'z') return 1;
    if (x >= 'A' && x <= 'Z') return 1;
    return 0;
}

int comparator(const void *p, const void *q) {
    char *a = (*(struct file **)p)->file_path;
    char *b = (*(struct file **)q)->file_path;
    int len_a = strlen(a);
    int len_b = strlen(b);
    int min_len = len_a;

    if (min_len > len_b) {
        min_len = len_b;
    }

    for (int i = 0; i < min_len; ++i) {
        char x = a[i];
        char y = b[i];

        //case where either x or y or both AREN'T letters
        if (is_letter(x) == 0 || is_letter(y) == 0) {
            if (x < y) {
              return -1;
            }
            if (x > y) {
              return 1;
            }
            //case where both x and y are letters
        } else {
            if (a[i] >= 'a' && a[i] <= 'z') {
                x -= 32;
            }
            if (b[i] >= 'a' && b[i] <= 'z') {
                y -= 32;
            }
            if (x < y) {
                return -1;
            } else if (x > y) {
                return 1;
            }

            if (a[i] < b[i])
                return -1;
            else if (a[i] > b[i])
                return 1;
        }
    }
    return len_a - len_b;
}

char *get_commit_id(struct commit *commit) {
    int id = 0;

    for (size_t i = 0; i < strlen(commit->message); ++i) {
        id = (id + (unsigned char)commit->message[i]) % 1000;
    }

    qsort(commit->committed_files, commit->n_committed_files,
          sizeof(struct file *), comparator);

    // if this is the first commit, all changes are additions
    if (commit->prev_commit == NULL) {
        for (int i = 0; i < commit->n_committed_files; i++) {
            id = id + 376591;

            for (size_t j = 0;
                 j < strlen(commit->committed_files[i]->file_path); j++) {
                id = (id *
                      ((unsigned char)commit->committed_files[i]->file_path[j] %
                       37)) %
                         15485863 +
                     1;
            }
        }
        return convert_dec_hex(id);
    }

    // data needed for keeping track of changes to commit
    struct file **change_list =
        (struct file **)malloc(sizeof(struct file *) * 50);
    int change_id_list[50];
    int change_counter = 0;

    //If flag == 1, the file is not in the previous commit and is therefore an addition
    int flag = 0;

    //adding additions to the change list
    for (int i = 0; i < commit->n_committed_files; ++i) {
        flag = 0;
        for (int j = 0; j < commit->prev_commit->n_committed_files; ++j) {
            if (strcmp(commit->committed_files[i]->file_path,
                       commit->prev_commit->committed_files[j]->file_path) ==
                0) {
                flag = 1;
            }
        }
        if (flag == 0) {
            change_id_list[change_counter] = 1;
            change_list[change_counter] = commit->committed_files[i];
            change_counter++;
        }
    }

    // adding deletions from the previous commit to the change list
    for (int i = 0; i < commit->prev_commit->n_committed_files; ++i) {
        //if flag == 1, file is not in new commit but is in previous, therefore it is a deletion
        flag = 0;
        for (int j = 0; j < commit->n_committed_files; ++j) {
            if (strcmp(commit->prev_commit->committed_files[i]->file_path,
                       commit->committed_files[j]->file_path) == 0) {
                flag = 1;
                // if the file hasn't been deleted, check whether it has changed
                // by comparing hash values
                if (commit->prev_commit->committed_files[i]->hash !=
                    commit->committed_files[j]->hash) {
                    change_id_list[change_counter] = 3;
                    change_list[change_counter] = commit->committed_files[j];
                    change_counter++;
                }
            }
        }
        if (flag == 0) {
            change_id_list[change_counter] = 2;

            change_list[change_counter] =
                commit->prev_commit->committed_files[i];
            change_counter++;
        }
    }

    // checking for additions from the previous commit to theb
    // analysing the list of changes and adjusting the commit ID accordingly.
    for (int i = 0; i < change_counter; i++) {
        if (change_id_list[i] == 1) {
            id = id + 376591;
        }

        if (change_id_list[i] == 2) {
            id = id + 85973;
        }

        if (change_id_list[i] == 3) {
            id = id + 9573681;
        }

        for (size_t j = 0; j < strlen(change_list[i]->file_path); j++) {
            id = (id * (((unsigned char)change_list[i]->file_path[j]) % 37)) %
                     15485863 +
                 1;
        }
    }

    free(change_list);

    return convert_dec_hex(id);
}

void init_commit(struct graph *g, char *message, struct commit *c) {
    c->message = (char *)malloc(sizeof(char) * (strlen(message) + 1));
    strcpy(c->message, message);
    g->head->last_commit = c;
    c->prev_commit = NULL;
    c->committed_files = (struct file **)malloc(sizeof(struct file *) * g->head->n_uncommitted_files);
    c->n_committed_files = g->head->n_uncommitted_files;
    for (int i = 0; i < g->head->n_uncommitted_files; i++) {

        c->committed_files[i] = g->head->uncommitted_files[i];

    }

    g->head->n_uncommitted_files = 0;
    g->head->uncommitted_files =
        (struct file **)realloc(g->head->uncommitted_files, 0);
    char *temp_id = get_commit_id(c);
    strcpy(c->id, temp_id);
    free(temp_id);
}

int check_changes_in_committed_files(struct graph *g) {
    for (int i = 0; i < g->head->last_commit->n_committed_files; ++i) {
        int hash = hash_file(
            (void *)g, g->head->last_commit->committed_files[i]->file_path);
         //if hash < 0, the file can't open and has therefore been manually deleted
        if (hash < 0) {
            continue;
        }
        //if the hash of the file in my data structure is different to that in the file system, then the file has changed manually and needs to be re-added to the uncommitted list.
        if (g->head->last_commit->committed_files[i]->hash != hash) {
            (g->head->n_uncommitted_files)++;
            g->head->uncommitted_files = (struct file **)realloc(
                g->head->uncommitted_files,
                sizeof(struct file *) * g->head->n_uncommitted_files);

            struct file *f = (struct file *)malloc(sizeof(struct file) * 1);

            int file_len;
            char *buffer = read_byte_file(
                g->head->last_commit->committed_files[i]->file_path, &file_len);
            f->content = (char *)malloc(sizeof(char) * file_len);
            strncpy(f->content, buffer, file_len);
            f->n_content = file_len;
            free(buffer);

            f->hash = hash_file(
                (void *)g, g->head->last_commit->committed_files[i]->file_path);
            strcpy(f->file_path,
                   g->head->last_commit->committed_files[i]->file_path);
            g->head->uncommitted_files[g->head->n_uncommitted_files - 1] = f;

            (g->n_files)++;
            g->files = (struct file **)realloc(
                g->files, sizeof(struct file *) * g->n_files);
            g->files[g->n_files - 1] = f;

            return 1;
        }
    }
    return 0;
}

//Checking if the file is in untracked files list
int is_removed(struct graph *g) {
    if (g->head->last_commit == NULL) return 0;
    for (int i = 0; i < g->head->n_untracked_files; ++i) {
        for (int j = 0; j < g->head->last_commit->n_committed_files; ++j) {
            if (strcmp(g->head->untracked_files[i]->file_path,
                       g->head->last_commit->committed_files[j]->file_path) ==
                0) {
                return 1;
            }
        }
    }
    return 0;
}


char *svc_commit(void *helper, char *message) {
    if (message == NULL) {
        return NULL;
    }

    struct graph *g = (struct graph *)helper;
    struct commit *c = (struct commit *)malloc(sizeof(struct commit));

    //first commit
    if (g->head->last_commit == NULL) {
        if (g->head->n_uncommitted_files == 0) {
            free(c);
            return NULL;
        }

        init_commit(g, message, c);

        g->head->last_commit = c;
        (g->n_commits)++;
        g->commits = (struct commit **)realloc(
            g->commits, sizeof(struct commits *) * g->n_commits);
        g->commits[g->n_commits - 1] = c;
        return c->id;
    }

    check_changes_in_committed_files(g);

    if (is_removed(g) == 0 && g->head->n_uncommitted_files == 0) {
        free(c);
        return NULL;
    }

    c->message = (char *)malloc(sizeof(char) * strlen(message) + 1);
    strcpy(c->message, message);
    c->prev_commit = g->head->last_commit;
    c->n_committed_files = 0;
    c->committed_files = (struct file **)malloc(sizeof(struct file *) * 0);

    // move all uncommited to committed
    for(int i = 0; i < g->head->n_uncommitted_files; ++i) {
      (c->n_committed_files)++;
      c->committed_files = (struct file **)realloc(
          c->committed_files, sizeof(struct file *) * c->n_committed_files);
      c->committed_files[c->n_committed_files - 1] =
          g->head->uncommitted_files[i];
    }

    //moving files from last commit to current commit
    //if flag == 1, file is untracked and should not be committed
    int flag = 0;
    for (int i = 0; i < g->head->last_commit->n_committed_files; ++i) {

        flag = 0;
        //checking if file is untracked
        for (int j = 0; j < g->head->n_untracked_files; ++j) {
            if (strcmp(g->head->last_commit->committed_files[i]->file_path,
                       g->head->untracked_files[j]->file_path) == 0) {
                flag = 1;
            }
        }
        //checking if file is in uncommitted as well as in the last commit
        for (int j = 0; j < g->head->n_uncommitted_files; ++j) {
            if (strcmp(g->head->last_commit->committed_files[i]->file_path,
                       g->head->uncommitted_files[j]->file_path) == 0 &&
                g->head->last_commit->committed_files[i]->hash !=
                    g->head->uncommitted_files[j]->hash) {
                flag = 1;
            }
        }
        if (flag == 0) {
            FILE* fp = fopen(g->head->last_commit->committed_files[i]->file_path, "r");
            if(fp != NULL) {
              (c->n_committed_files)++;
              c->committed_files = (struct file **)realloc(
                  c->committed_files,
                  sizeof(struct file *) * c->n_committed_files);
              c->committed_files[c->n_committed_files - 1] =
                  g->head->last_commit->committed_files[i];
              fclose(fp);
            }
        }
    }

    g->head->n_uncommitted_files = 0;
    g->head->n_untracked_files = 0;
    g->head->uncommitted_files = (struct file**) realloc(g->head->uncommitted_files, 0);
    g->head->untracked_files = (struct file**) realloc(g->head->untracked_files, 0);

    char *temp_id = get_commit_id(c);
    strcpy(c->id, temp_id);
    free(temp_id);
    (g->n_commits)++;
    g->commits = (struct commit **)realloc(
        g->commits, sizeof(struct commits *) * g->n_commits);
    g->commits[g->n_commits - 1] = c;
    g->head->last_commit = c;
    return c->id;
}

void *get_commit(void *helper, char *commit_id) {
    struct graph *g = (struct graph *)helper;

    for (int i = 0; i < g->n_branches; ++i) {
        struct commit *temp = g->branches[i]->last_commit;
        while (temp != NULL) {
            if (strcmp(temp->id, commit_id) == 0) {
                return temp;
            }
            temp = temp->prev_commit;
        }
    }

    return NULL;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    struct graph *g = (struct graph *)helper;

    if (n_prev == NULL) {
        return NULL;
    }

    if (commit == NULL || g->head->last_commit->prev_commit == NULL) {
        (*n_prev) = 0;
        return NULL;
    }

    struct commit *cur = ((struct commit *)commit)->prev_commit;
    if(cur == NULL) {
      *n_prev = 0;
      return NULL;
    }

    char **ret = (char **)malloc(sizeof(char *) * 0);

    int count = 0;
    while (cur != NULL) {
        ret = (char **)realloc(ret, sizeof(char *) * (count + 1));
        ret[count] = cur->id;
        cur = cur->prev_commit;
        count++;
    }
    (*n_prev) = count;
    return ret;
}

void print_commit(void *helper, char *commit_id) {
    if (commit_id == NULL) {
        printf("Invalid commit id\n");
        return;
    }
    struct graph *g = (struct graph *)helper;
    struct commit *c = get_commit(helper, commit_id);
    if (c == NULL) {
        printf("Invalid commit id\n");
        return;
    }
    // x is either the index of the branch we are looking for, or if it remains -1, indicates that the branch doesn't exist
    int x = -1;
    for (int i = 0; i < g->n_branches; ++i) {
        struct commit *temp = g->branches[i]->last_commit;
        while (temp != NULL) {
            if (strcmp(commit_id, temp->id) == 0) {
                x = i;
                break;
            }
            temp = temp->prev_commit;
        }
    }

    if(x == -1) {
      printf("Invalid commit id\n");
      return;
    }

    printf("%s [%s]: %s\n", c->id, g->branches[x]->name, c->message);

    if (c->prev_commit == NULL) {
        for (int i = 0; i < c->n_committed_files; i++) {
            printf("    + %s\n", c->committed_files[i]->file_path);
        }
    } else {
        // printing added files
        for (int i = 0; i < c->n_committed_files; ++i) {
            //if flag == 1, committed file is also in previous commit and therefore is not added
            int flag = 0;
            for (int j = 0; j < c->prev_commit->n_committed_files; ++j) {
                if (strcmp(c->committed_files[i]->file_path,
                           c->prev_commit->committed_files[j]->file_path) ==
                    0) {
                    flag = 1;
                }
            }
            if (flag == 0) {
                printf("    + %s\n", c->committed_files[i]->file_path);
            }
        }
        // printing deleted files
        for (int i = 0; i < c->prev_commit->n_committed_files; ++i) {
          //if flag == 1, file is in current commit and is therefore not deleted
            int flag = 0;
            for (int j = 0; j < c->n_committed_files; ++j) {
                if (strcmp(c->committed_files[j]->file_path,
                           c->prev_commit->committed_files[i]->file_path) ==
                    0) {
                    flag = 1;
                }
            }
            if (flag == 0) {
                printf("    - %s\n",
                       c->prev_commit->committed_files[i]->file_path);
            }
        }
        // printing modified files
        for (int i = 0; i < c->n_committed_files; ++i) {
            for (int j = 0; j < c->prev_commit->n_committed_files; ++j) {
                if (strcmp(c->committed_files[i]->file_path,
                           c->prev_commit->committed_files[j]->file_path) ==
                    0) {
                    if (c->committed_files[i]->hash !=
                        c->prev_commit->committed_files[j]->hash) {
                        printf("    / %s [%d --> %d]\n",
                               c->committed_files[i]->file_path,
                               c->prev_commit->committed_files[j]->hash,
                               c->committed_files[i]->hash);
                    }
                }
            }
        }
    }
    printf("\n    Tracked files (%d):\n", c->n_committed_files);
    for (int i = 0; i < c->n_committed_files; ++i) {
        printf("    [%10d] %s\n", c->committed_files[i]->hash,
               c->committed_files[i]->file_path);
    }
}

int svc_branch(void *helper, char *branch_name) {
    if (branch_name == NULL) {
        return -1;
    }

    struct graph *g = (struct graph *)helper;

    //if flag is not changed, the character is not within the accepted characters for a branch name.
    int flag = 0;
    for (size_t i = 0; i < strlen(branch_name); i++) {
        flag = 0;
        if ((branch_name[i] >= 'a' && branch_name[i] <= 'z') ||
            (branch_name[i] >= 'A' && branch_name[i] <= 'Z') ||
            (branch_name[i] >= '0' && branch_name[i] <= '9') ||
            branch_name[i] == '/' || branch_name[i] == '-' ||
            branch_name[i] == '_') {
            flag = 1;
        }

        if (flag == 0) {
            return -1;
        }
    }

    for (int i = 0; i < g->n_branches; i++) {
      if (strcmp(branch_name, g->branches[i]->name) == 0) {
            return -2;
        }
    }

    if (g->head->n_uncommitted_files > 0) {
        return -3;
    }

    struct branch *b = (struct branch *)malloc(sizeof(struct branch));
    b->last_commit = g->head->last_commit;
    b->n_uncommitted_files = 0;
    b->uncommitted_files = (struct file **)malloc(sizeof(struct file *) * 0);
    b->untracked_files = (struct file **)malloc(sizeof(struct file *) * 0);
    b->n_untracked_files = 0;
    strcpy(b->name, branch_name);

    (g->n_branches)++;
    g->branches = (struct branch **)realloc(
        g->branches, sizeof(struct branch *) * g->n_branches);
    g->branches[g->n_branches - 1] = b;
    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    if (branch_name == NULL) {
        return -1;
    }

    struct graph *g = (struct graph *)helper;
    int index = -1;

    for (int i = 0; i < g->n_branches; i++) {
        if (strcmp(g->branches[i]->name, branch_name) == 0) {
            index = i;
        }
    }

    if (index == -1) {
        return -1;
    }

    if (g->branches[index]->n_uncommitted_files > 0) {
        return -2;
    }

    g->head = g->branches[index];
    return 0;
}

char **list_branches(void *helper, int *n_branches) {
    if (n_branches == NULL) {
        return NULL;
    }

    struct graph *g = (struct graph *)helper;
    char **branch_list = (char **)malloc(sizeof(char *) * g->n_branches);
    (*n_branches) = g->n_branches;

    for (int i = 0; i < g->n_branches; ++i) {
        printf("%s\n", g->branches[i]->name);
        branch_list[i] = g->branches[i]->name;
    }

    return branch_list;
}

int svc_rm(void *helper, char *file_name) {
    if (file_name == NULL) {
        return -1;
    }

    struct graph *g = (struct graph *)helper;
    for (int i = 0; i < g->head->n_untracked_files; ++i) {
        if (strcmp(g->head->untracked_files[i]->file_path, file_name) == 0) {
            return -2;
        }
    }

    int file_in_uncommitted = check_for_file_in_uncommitted(g->head, file_name);
    if (file_in_uncommitted > -1) {
        int hash = g->head->uncommitted_files[file_in_uncommitted]->hash;
        for (int i = file_in_uncommitted; i < g->head->n_uncommitted_files - 1;
             i++) {
            g->head->uncommitted_files[i] = g->head->uncommitted_files[i + 1];
        }

        (g->head->n_uncommitted_files)--;
        g->head->uncommitted_files = (struct file **)realloc(
            g->head->uncommitted_files,
            sizeof(struct file *) * g->head->n_uncommitted_files);
        return hash;
    }

    if (g->head->last_commit == NULL) {
        return -2;
    }

    int file_in_committed = check_for_file_in_committed(g->head, file_name);
    if (file_in_committed > -1) {
        struct commit *ptr = g->head->last_commit;
        int hash = ptr->committed_files[file_in_committed]->hash;
        (g->head->n_untracked_files)++;
        g->head->untracked_files = (struct file **)realloc(
            g->head->untracked_files,
            (sizeof(struct file *) * g->head->n_untracked_files));
        g->head->untracked_files[g->head->n_untracked_files - 1] =
            ptr->committed_files[file_in_committed];

        return hash;
    }

    return -2;
}

void *get_commit_from_head(void *helper, char *commit_id) {
    struct graph *g = (struct graph *)helper;

    struct commit *temp = g->head->last_commit;
    while (temp != NULL) {
        if (strcmp(temp->id, commit_id) == 0) {
            return temp;
        }
        temp = temp->prev_commit;
    }

    return NULL;
}

void write_to_file(char *file_path, char *contents, int n_content) {
    FILE *f = fopen(file_path, "w");
    for (int i = 0; i < n_content; ++i) fputc(contents[i], f);
    fclose(f);
}

int svc_reset(void *helper, char *commit_id) {
    if (commit_id == NULL) {
        return -1;
    }

    struct commit *c = get_commit_from_head(helper, commit_id);
    if (c == NULL) {
        return -2;
    }

    struct graph *g = (struct graph *)helper;

    g->head->last_commit = c;
    g->head->uncommitted_files = (struct file **)realloc(
        g->head->uncommitted_files, sizeof(struct file *) * 0);
    g->head->n_uncommitted_files = 0;

    for (int i = 0; i < g->head->last_commit->n_committed_files; ++i) {
        write_to_file(g->head->last_commit->committed_files[i]->file_path,
                      g->head->last_commit->committed_files[i]->content,
                      g->head->last_commit->committed_files[i]->n_content);
    }
    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions,
                int n_resolutions) {
    if(branch_name == NULL) {
      printf("Invalid branch name\n");
      return NULL;
    }
    struct graph* g= (struct graph*)helper;

    //if flag is not changed, branch is not in the graph
    int flag = 0;
    int index = -1;
    for(int i = 0; i < g->n_branches; ++i) {

      if(strcmp(g->branches[i]->name, branch_name) == 0) {
        index = i;
        flag = 1;
      }

    }
    if(flag == 0) {
      printf("Branch not found\n");
      return NULL;
    }

    if(strcmp(g->head->name, branch_name) == 0) {
      printf("Cannot merge a branch with itself\n");
      return NULL;
    }

    //if flag == 1, a file has been manually changed and not committed in the master branch
    flag = 0;
    int hash;
    for(int i = 0; i < g->head->last_commit->n_committed_files; ++i) {

      hash = hash_file(helper, g->head->last_commit->committed_files[i]->file_path);

      if(hash < 0 || g->head->last_commit->committed_files[i]->hash != hash) {
        flag = 1;
      }

    }

    if(g->head->n_uncommitted_files > 0 || g->branches[index]->n_uncommitted_files > 0 || flag == 1) {
      printf("Changes must be committed\n");
      return NULL;
    }

    struct commit* c = (struct commit*)malloc(sizeof(struct commit)*1);
    c->committed_files = (struct file**)malloc(sizeof(struct file*)*0);
    c->n_committed_files = 0;
    c->prev_commit = g->head->last_commit;

    int string_length = 14 + strlen(branch_name);
    c->message = (char*)malloc(sizeof(char)*string_length+2);
    strcpy(c->message, "Merged branch ");
    for(size_t i = 0; i < strlen(branch_name); ++i) {
      c->message[15+i] = branch_name[i];
    }

    //merging last commit from master branch
    for(int i = 0; i < g->head->last_commit->n_committed_files; ++i) {
      // if flag == 1, file was unresolved but has now been resolved and added to the list of committed files. it therefore shouldnt be added again.
      flag = 0;
      for(int j = 0; j < n_resolutions; ++j) {

        if(strcmp(g->head->last_commit->committed_files[i]->file_path, resolutions[j].file_name) == 0){
          if(resolutions[j].resolved_file == NULL) {
            continue;
          }

          struct file* f = (struct file*)malloc(sizeof(struct file)*1);

          strcpy(f->file_path, resolutions[j].resolved_file);

          f->hash = hash_file(helper, resolutions[j].resolved_file);

          char* buffer = read_byte_file(resolutions[j].resolved_file, &f->n_content);
          f->content = (char*)malloc(sizeof(char)*(f->n_content)+1);
          strncpy(f->content, buffer, f->n_content);
          free(buffer);

          g->n_files++;
          g->files = (struct file**)realloc(g->files, sizeof(struct file*)*g->n_files);
          g->files[g->n_files-1] = f;

          (c->n_committed_files)++;
          c->committed_files = (struct file**)realloc(c->committed_files, sizeof(struct file*)* c->n_committed_files);
          c->committed_files[c->n_committed_files-1] = f;

          flag = 1;
        }
      }
      if(flag == 0){
        (c->n_committed_files)++;
        c->committed_files = (struct file**)realloc(c->committed_files, sizeof(struct file*)* c->n_committed_files);
        c->committed_files[c->n_committed_files-1] = g->head->last_commit->committed_files[i];
      }
    }
    //if flag2 == 1 the file has already been resolved and should not be added again to the committed files of the new commit
    int flag2 = 0;
    //merging last commit from feature branch
    for(int i = 0; i < g->branches[index]->last_commit->n_committed_files; ++i) {
      // if flag == 1, file was unresolved but has now been resolved and added to the list of committed files. it therefore shouldnt be added again.
      flag = 0;
      for(int j = 0; j < n_resolutions; ++j) {

        if(strcmp(g->branches[index]->last_commit->committed_files[i]->file_path, resolutions[j].file_name) == 0) {

          if(resolutions[j].resolved_file == NULL) {
            continue;
          }

          //checking if the file has already been resolved
          //if flag2 == 1 the file has already been resolved and should not be added again to the committed files of the new commit
          flag2 = 0;
          for(int k = 0; k < c->n_committed_files; ++k) {

            if(strcmp(c->committed_files[k]->file_path, resolutions[j].resolved_file) == 0) {
              flag2 = 1;
            }
          }
          if(flag2 == 1){
            continue;
          }

          struct file* f = (struct file*)malloc(sizeof(struct file)*1);

          strcpy(f->file_path, resolutions[j].resolved_file);

          f->hash = hash_file(helper, resolutions[j].resolved_file);

          char* buffer = read_byte_file(resolutions[j].resolved_file, &f->n_content);
          f->content = (char*)malloc(sizeof(char)*(f->n_content)+1);
          strncpy(f->content, buffer, f->n_content);
          free(buffer);

          (g->n_files)++;
          g->files = (struct file**)realloc(g->files, sizeof(struct file*)*g->n_files);
          g->files[g->n_files-1] = f;

          (c->n_committed_files)++;
          c->committed_files = (struct file**)realloc(c->committed_files, sizeof(struct file*)* c->n_committed_files);
          c->committed_files[c->n_committed_files-1] = f;

          flag = 1;
        }
      }
      if(flag == 0){
        //if flag2 == 1, the file has already been added to the committed files and should not be added again.
        flag2 = 0;
        for(int k = 0; k < c->n_committed_files; ++k) {
          if(strcmp(c->committed_files[k]->file_path, g->branches[index]->last_commit->committed_files[i]->file_path)== 0) {
            flag2 = 1;
          }
        }
        if(flag2 == 0) {
          continue;
        }

        (c->n_committed_files)++;
        c->committed_files = (struct file**)realloc(c->committed_files, sizeof(struct file*)* c->n_committed_files);
        c->committed_files[c->n_committed_files-1] = g->branches[index]->last_commit->committed_files[i];
      }
    }

    char* temp_id = get_commit_id(c);
    strcpy(c->id, temp_id);
    free(temp_id);

    (g->n_commits)++;
    g->commits = (struct commit**)realloc(g->commits, sizeof(struct commit*)*g->n_commits);
    g->commits[g->n_commits-1] = c;

    return c->id;
}