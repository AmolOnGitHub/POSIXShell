#include "helper.h"

int find_max_arg_count(char* command);

// Function to get all the alias commands from .myshrc

void get_alias_commands() {
    alias_commands = (alias_st*) calloc(256, sizeof(alias_st));
    alias_count = 0;

    FILE* myshrc = fopen(MYSHRC_PATH, "r");
    if (myshrc == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " .myshrc not found.\n" RESET);
        return;
    }

    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, myshrc) != -1) {
        if (strncmp(line, "alias", 5) == 0) {
            char* rem = strdup(line + 6);
            char* token = strtok_r(rem, "=", &rem);
            alias_commands[alias_count].name = strdup(token);
            alias_commands[alias_count].command = strdup(rem);
            strip(alias_commands[alias_count].name);
            strip(alias_commands[alias_count].command);
            alias_count++;
        }
    }

    fclose(myshrc);
}


// Function to get all the function commands from .myshrc

void get_func_commands() {
    func_commands = (func_st*) calloc(256, sizeof(func_st));
    func_count = 0;

    FILE* myshrc = fopen(MYSHRC_PATH, "r");
    if (myshrc == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " .myshrc not found.\n" RESET);
        return;
    }

    char* line = NULL;
    size_t len = 0;
    bool reading_func = false;
    char* command = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
    int arg_count = 0;


    // Read .myshrc line by line, and store function commands

    while (getline(&line, &len, myshrc) != -1) {
        if (strncmp(line, "{", 1) == 0) {
            reading_func = true;
        }

        else if (strncmp(line, "}", 1) == 0) {
            reading_func = false;
            func_commands[func_count].command = strdup(command + 3);
            func_commands[func_count].arg_count = find_max_arg_count(command);
            memset(command, '\0', DEFAULT_STRING_SIZE);
            func_count++;
        }

        else if (reading_func) {
            strip(line);
            snprintf(command, DEFAULT_STRING_SIZE, "%s ; %s", command, line);
        }
        
        else if (strncmp(line, "func", 4) == 0) {
            char* rem = strdup(line + 5);
            char* token = strtok_r(rem, "(", &rem);
            func_commands[func_count].name = strdup(token);
            strip(func_commands[func_count].name);
        }
    }

    fclose(myshrc);
}


// Function to find the maximum number of arguments in a function, by counting "$x"

int find_max_arg_count(char* command) {
    int max_arg_count = 0;
    char* substring = strstr(command, "$");
    
    while (substring != NULL) {
        int arg_count = atoi(substring + 1);
        if (arg_count > max_arg_count) {
            max_arg_count = arg_count;
        }
        substring = strstr(substring + 1, "$");
    }
    
    return max_arg_count;
}