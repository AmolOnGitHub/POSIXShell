#include "../methods.h"

char* get_nth_recent_command(char* file_path, int n);

int get_num_lines(char* file_path);


// Function to update log file with the given command

int update_log(char* command) {
    strip(command);
    FILE* file;
    char* last_command = get_nth_recent_command(LOG_PATH, 1);
    int log_num = get_num_lines(LOG_PATH);

    // If the command is same as the last command, don't update log

    if (strcmp(last_command, command) == 0) return 0;


    // If log file has more than 15 commands, remove the oldest command

    char* prevcommands = (char*) calloc(DEFAULT_STRING_SIZE * 15, sizeof(char));
    if (log_num >= 15) {
        file = fopen(LOG_PATH, "r");
        if (file == NULL) {
            fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n" RESET);
            return 1;
        }

        char* line = NULL;
        size_t line_size = 0;

        getline(&line, &line_size, file);

        while (getline(&line, &line_size, file) != -1)
            snprintf(prevcommands, DEFAULT_STRING_SIZE * 15, "%s%s", prevcommands, line);
        
        free(line);
        fclose(file);

        file = fopen(LOG_PATH, "w");
        if (file == NULL) {
            fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n" RESET);
            return 1;
        }

        fprintf(file, "%s", prevcommands);
        fclose(file);
    }


    // Append the new command to the log file

    file = fopen(LOG_PATH, "a+");
    if (file == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n" RESET);
        return 1;
    }

    fprintf(file, "%s\n", command);
    fclose(file);
    return 0;
}


// Function to handle log command

int log_c(char** args, FILE* OUTPUT) {

    // If no arguments are given, print all commands in log

    if (args[1] == NULL) {
        FILE* file = fopen(LOG_PATH, "r");
        if (file == NULL) {
            fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n\n" RESET);
            return 1;
        }

        char* line = NULL;
        size_t line_size = 0;
        int i = 0;
        for (i = 0; getline(&line, &line_size, file) != -1; i++)
            fprintf(OUTPUT, "%s", line);
        
        if (i == 0)
            fprintf(stderr, RED BOLD "ERROR:" RESET RED " No commands in log.\n" RESET);
        
        free(line);
        fclose(file);
        fprintf(OUTPUT, "\n");
        return 0;
    }


    // If the argument is "purge", clear the log file

    if (strcmp(args[1], "purge") == 0) {
        FILE* file = fopen(LOG_PATH, "w");
        if (file == NULL) {
            fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n\n" RESET);
            return 1;
        }

        fclose(file);
        return 0;
    }


    // If the argument is "execute", execute the command at the given index

    if (strcmp(args[1], "execute") == 0) {
        bool temp;
        int index = atoi(args[2]);
        int num_lines = get_num_lines(LOG_PATH);
        
        if (index > num_lines) {
            fprintf(stderr, RED BOLD "ERROR:" RESET RED " Given index is not valid.\n\n" RESET);
            return 1;
        }

        char* command = get_nth_recent_command(LOG_PATH, index);
        execute_command(command, true, OUTPUT, NULL);
    }
    return 0;
}


// Function to get the nth most recent command from the log file

char* get_nth_recent_command(char* file_path, int n) {
    char* command = NULL;

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n" RESET);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long pos = ftell(file) - 1, c;

    for (int i = 0; i < n && pos > 0;) {
        fseek(file, --pos, SEEK_SET);
        c = getc(file);
        if (c == '\n') {
            i++;
        }
    }

    if (pos == 0) fseek(file, 0, SEEK_SET);

    size_t line_size = 0;
    getline(&command, &line_size, file);
    command[strlen(command) - 1] = '\0'; // Removes \n

    fclose(file);
    return command;
}


// Function to get the number of lines in the log file

int get_num_lines(char* file_path) {
    int num_lines = 0;

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " log file not found.\n" RESET);
        return -1;
    }

    char* line = NULL;
    size_t line_size = 0;

    while (getline(&line, &line_size, file) != -1) num_lines++;
    
    free(line);
    fclose(file);
    return num_lines;
}
