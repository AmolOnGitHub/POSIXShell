#ifndef __HELPER_H__
#define __HELPER_H__
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <regex.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>


#define PATH_MAX 4096
#define DEFAULT_STRING_SIZE 4096
#define MAX_ARGS 128


typedef struct st_alias {
    char* name;
    char* command;
} alias_st;


typedef struct st_func {
    char* name;
    char* command;
    int arg_count;
} func_st;


typedef struct st_process {
    pid_t pid;
    char* name;
    char* status;
    int group_id;
    unsigned long vm_size;
    char* exe_path;
    struct st_process* next;
} process_st;


extern char *LOG_PATH, *MYSHRC_PATH;
extern char *cwd, *cur_cwd, *prev_cwd;
extern char *terminal_add;
extern process_st *processes;
extern pid_t current_foreground;
extern char* current_foreground_name;
extern alias_st *alias_commands;
extern int alias_count;
extern func_st *func_commands;
extern int func_count;


#include "methods.h"


#define BLACK   "\x1b[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"


void chop(char* str, int chop_length);

void strip(char* str);

int path_exists(char* path);

int dir_exists(char* path);

int file_exists(char* path);

int is_dir(char* path);

int is_file(char* path);

char* process_relative_path(char* relative);

char* replace_substrings(char* str, char* substr, char* replacement);

process_st* is_process_pid(pid_t pid);

#endif