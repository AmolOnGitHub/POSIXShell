#include "shell_methods/helper.h"

/*
Important notes:

*/

char *cwd, *cur_cwd, *prev_cwd;
char *username, *sysname;
char *LOG_PATH, *MYSHRC_PATH;
char *terminal_add;
process_st *processes;
pid_t current_foreground;
char* current_foreground_name;
alias_st *alias_commands;
int alias_count;
func_st *func_commands;
int func_count;

// Function to print the terminal prompt

void term_print() {
    int rootlen = strlen(cwd);

    if (strncmp(cur_cwd, cwd, rootlen) == 0) {
        char* temp = strdup(cur_cwd);
        chop(temp, rootlen);
        printf(CYAN "<%s@%s:~%s%s> " RESET, username, sysname, temp, terminal_add);
        free(temp);
    }
    else {
        printf(CYAN "<%s@%s:%s%s> " RESET, username, sysname, cur_cwd, terminal_add);
    }

    memset(terminal_add, '\0', DEFAULT_STRING_SIZE);
}

int main() {

    // Set up signal handlers

    setup_signal_handlers();
    current_foreground = 0;
    

    // Create log file if it doesn't exist already

    FILE* log_file = fopen("log.txt", "a");
    close(fileno(log_file));

    
    // Initialise the process list with the shell process
    
    process_st* process = (process_st*) calloc(1, sizeof(process_st));
    process->name = strdup("\0");
    process->pid = getpid();
    process->next = NULL;
    add_process(process);
    current_foreground = 0;
    current_foreground_name = calloc(DEFAULT_STRING_SIZE, sizeof(char));

    
    // Allocate space for all global variables

    cwd = (char*) calloc(PATH_MAX, sizeof(char)); // Path from root of the system
    username = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char)); 
    sysname = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
    terminal_add = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char)); // Additional info to be printed in the terminal


    // Get current working directory

    if (getcwd(cwd, PATH_MAX) == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " getcwd() failed.\n" RESET);
        return 1;
    }


    // Set relative path to the root directory

    cur_cwd = strdup(cwd); // Path from home directory of our shell
    prev_cwd = (char*) calloc(PATH_MAX, sizeof(char));


    // Set log file path and .myshrc path

    LOG_PATH = (char*) calloc(PATH_MAX, sizeof(char));
    snprintf(LOG_PATH, PATH_MAX, "%s/log.txt", cwd);
    MYSHRC_PATH = (char*) calloc(PATH_MAX, sizeof(char));
    snprintf(MYSHRC_PATH, PATH_MAX, "%s/.myshrc", cwd);

    
    // Get username and system name

    if (getlogin_r(username, DEFAULT_STRING_SIZE) == -1) {
        fprintf(stderr, BOLD "ERROR:" RESET YELLOW " getlogin_r() failed.\n" RESET);
        return 1;
    }

    if (gethostname(sysname, DEFAULT_STRING_SIZE) == -1) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " gethostname() failed.\n" RESET);
        return 1;
    }


    // Find all aliases and functions in .myshrc and store them

    get_alias_commands();
    get_func_commands();


    // Main shell loop

    char* input = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
    while (true) {
        term_print();
        char* ret = fgets(input, DEFAULT_STRING_SIZE, stdin);
        
        if (ret == NULL) {
            printf("Logging out.\n");
            kill_all_processes();
            exit(0);
        }
        
        input[strlen(input) - 1] = '\0';
        printf("\n");

        execute_command(input, false, NULL, NULL);

        memset(input, '\0', DEFAULT_STRING_SIZE);
    }

    free(input);
    return 0;
}