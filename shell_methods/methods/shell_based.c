#include "../methods.h"

// Function to hop from one directory to another

int hop(char** args, FILE* OUTPUT) {

    // Handle the case where no arguments are provided

    if (args[1] == NULL) {
        prev_cwd = strdup(cur_cwd);
        chdir(cwd);
        cur_cwd = strdup(cwd);
        return 0;
    }


    for (int i = 1; args[i] != NULL; i++) {
        
        // Handle absolute path case

        if (args[i][0] == '/') {
            if (!dir_exists(args[i])) {
                return 1;
            }
            chdir(args[i]);
            prev_cwd = strdup(cur_cwd);
            cur_cwd = strdup(args[i]);
            fprintf(OUTPUT, "%s\n\n", cur_cwd);
            continue;
        }


        // Handle relative path case
        // Tokenize the path and change directory accordingly

        char* init_cur_cwd = strdup(cur_cwd);
        char* temp_path = strdup(args[i]);
        char* dir;

        while ((dir = strtok_r(temp_path, "/", &temp_path))) {
            if (strcmp(dir, ".") == 0) continue;
            else if (strcmp(dir, "..") == 0) {
                chdir("..");
            }
            else if (strcmp(dir, "~") == 0) {
                chdir(cwd);
            }
            else if (strcmp(dir, "-") == 0) {
                if (strcmp(prev_cwd, "") == 0) {
                    fprintf(stderr, RED BOLD "ERROR:" RESET RED " No previous directory exists.\n\n" RESET);
                    return 1;
                }
                chdir(prev_cwd);
            }
            else {

                // Check if dir exists, then change directory

                char *temp_path = (char*) calloc(PATH_MAX, sizeof(char));
                snprintf(temp_path, PATH_MAX, "%s/%s", cur_cwd, dir);
                
                if (!dir_exists(temp_path)) {
                    free(temp_path);
                    return 1;
                }

                free(temp_path);
                chdir(dir);
            }

            if (getcwd(cur_cwd, PATH_MAX) == NULL) {
                fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " getcwd() failed.\n\n" RESET);
                return 1;
            }
        }

        // Update the previous directory and print the current directory

        prev_cwd = strdup(init_cur_cwd);
        free(init_cur_cwd);
        
        fprintf(OUTPUT, "%s\n\n", cur_cwd);
    }

    return 0;
}


// Function to execute a general bash command

int general_bash(char** args, bool foreground, FILE* OUTPUT, FILE* INPUT) {

    // Change stdout and stdin to the given file streams

    int save_stdin = dup(STDIN_FILENO);
    int save_stdout = dup(STDOUT_FILENO);

    if (dup2(fileno(INPUT), STDIN_FILENO) == -1) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " dup2() failed.\n\n" RESET);
        return 1;
    }

    if (dup2(fileno(OUTPUT), STDOUT_FILENO) == -1) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " dup2() failed.\n\n" RESET);
        return 1;
    }


    // Fork a child process to execute the command

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " fork() failed.\n\n" RESET);
        return 2;
    }
    else if (pid == 0) {
        if (!foreground) {
            setpgid(0, 0);
        }
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, RED BOLD "ERROR:" RESET RED " Command not found.\n\n" RESET);
            exit(1);
        }
        exit(EXIT_FAILURE);
    } else {
        process_st* new_process = (process_st*) calloc(1, sizeof(process_st));
        new_process->pid = pid;
        new_process->name = strdup(args[0]);

        if (!foreground) {
            add_process(new_process);

            printf(BLUE BOLD "%d\n" RESET, pid);
            printf(RESET "\n");
        }
        else {
            handle_foreground(new_process);
        }
    }

    current_foreground = 0;
    strcpy(current_foreground_name, "\0");

    // Reset stdout and stdin

    dup2(save_stdin, STDIN_FILENO);
    dup2(save_stdout, STDOUT_FILENO);

    close(save_stdin);
    close(save_stdout);

    printf("\n");
    return 0;
}