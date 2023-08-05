#ifndef svc_h
#define svc_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "helper.h"


typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char *file_name; // the file is conflicted
    char *resolved_file; // the file with resolved contents
} resolution;

void *svc_init(void);

int check_for_file_in_uncommitted(struct branch *head, char *file_name);

int check_for_file_in_committed(struct branch *head, char *file_name);

int svc_add(void *helper, char *file_name);

void cleanup(void *helper);

char *read_byte_file(char *file_path, int *file_len);

int hash_file(void *helper, char *file_path);

void reverse(char *x, int begin, int end);

char *convert_dec_hex(int decimal);

int is_letter(char x);

int comparator(const void *p, const void *q);

void init_commit(struct graph *g, char *message, struct commit *c);

int is_removed(struct graph *g);

char *svc_commit(void *helper, char *message);

void *get_commit(void *helper, char *commit_id);

char **get_prev_commits(void *helper, void *commit, int *n_prev);

void print_commit(void *helper, char *commit_id);

int svc_branch(void *helper, char *branch_name);

int svc_checkout(void *helper, char *branch_name);

char **list_branches(void *helper, int *n_branches);

int svc_rm(void *helper, char *file_name);

void *get_commit_from_head(void *helper, char *commit_id);

void write_to_file(char *file_path, char *contents, int n_content);

int svc_reset(void *helper, char *commit_id);

char *svc_merge(void *helper, char *branch_name, resolution *resolutions, int n_resolutions);

#endif