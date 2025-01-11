#ifndef __METHODS_H__
#define __METHODS_H__

#include "helper.h"

void execute_command(char* input, bool log_override, FILE* OUTPUT, FILE* INPUT);

int hop(char** args, FILE* OUTPUT);

int reveal(char** args, FILE* OUTPUT);

int log_c(char** args, FILE* OUTPUT);

int general_bash(char** args, bool foreground, FILE* OUTPUT, FILE* INPUT);

int proclore(char** args, FILE* OUTPUT);

int seek(char** args, FILE* OUTPUT);

void activities(FILE* OUTPUT);

int ping(char** args, FILE* OUTPUT);

int fg(char** args);

int bg(char** args);

void neonate(char** args, FILE* OUTPUT);

int iman(char** args, FILE* OUTPUT);

void get_alias_commands();

void get_func_commands();

int update_log(char* command);

int get_num_lines(char* file_path);

void add_process(process_st* process);

void update_process_info(pid_t pid);

void kill_all_processes();

void setup_signal_handlers();

void handle_sigtstp(int sig);

void handle_foreground(process_st* process);

#endif