#include "helper.h"

bool valid_format(char* input);

// Function to take any given command and execute it

void execute_command(char* input, bool log_override, FILE* OUTPUT, FILE* INPUT) {

    // Check format of input, whether any wrongly placed pipes / semicolons / ampersands

    if (valid_format(input) == false) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Invalid command.\n\n" RESET);
        return;
    }

    // Note down initial time when command starts executing

    struct timeval start, end;
    gettimeofday(&start, NULL);


    // Tokenise input on ; & and |

    char* temp = strdup(input);
    char* delim = "&;|";
    char* saveptr;

    char* command = strtok_r(temp, delim, &saveptr);
    bool log_called = log_override;
    char* command_list = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char)); // For providing info in next terminal prompt

    int* prev_pipe = NULL;

    // Iterate through tokenised commands, executing them as necessary 

    while (command) {
        
        // Determine delimiter used to get command, and use it to figure out piping / foreground or background.

        char used_delim = input[command - temp + strlen(command)];
        bool foreground = used_delim != '&';
        if (foreground) snprintf(command_list, DEFAULT_STRING_SIZE, "%s %s", command_list, command);
        
        int* pipefd = NULL;

        if (used_delim == '|') {
            pipefd = (int*) calloc(2, sizeof(int));
            if (pipe(pipefd) == -1) {
                fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " Pipe failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
            OUTPUT = fdopen(pipefd[1], "w");
        }

        if (prev_pipe != NULL) {
            INPUT = fdopen(prev_pipe[0], "r");
        }

        // Set up input and output redirection if necessary

        if (OUTPUT == NULL) OUTPUT = stdout;
        if (INPUT == NULL) INPUT = stdin;

        char** args = (char**) calloc(256, sizeof(char*));
        char* tok_command = strdup(command);
        char* token;

        int inp_pos = -1, out_pos = -1, min_pos = -1;
        bool append_flag = false;

        for (int i = 0; i < 256 && (token = strtok_r(tok_command, " \t", &tok_command)); i++) {
            args[i] = strdup(token);
            if (strcmp(token, "<") == 0) inp_pos = i;
            if (strcmp(token, ">") == 0) out_pos = i;
            if (strcmp(token, ">>") == 0) {
                out_pos = i;
                append_flag = true;
            }
        }

        if (inp_pos != -1) {
            if (INPUT != stdin) fclose(INPUT);
            INPUT = fopen(args[inp_pos + 1], "r");
            if (INPUT == NULL) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " File does not exist.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
            min_pos = inp_pos;
        }

        if (out_pos != -1) {
            if (OUTPUT != stdout) fclose(OUTPUT);
            if (append_flag)
                OUTPUT = fopen(args[out_pos + 1], "a");
            else
                OUTPUT = fopen(args[out_pos + 1], "w");
            if (min_pos == -1) min_pos = out_pos;
            else min_pos = min_pos > out_pos ? out_pos : min_pos;
        }

        if (min_pos != -1) {
            for (int i = min_pos; args[i] != NULL; i++) {
                free(args[i]);
                args[i] = NULL;
            }
        }


        // Process command

        if (strcmp(args[0], "exit") == 0) {
            free(temp);
            free(cur_cwd);
            free(prev_cwd);
            free(cwd);
            exit(0);
        }

        else if (strcmp(args[0], "hop") == 0) {
            int exit_status = hop(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " hop failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else if (strcmp(args[0], "reveal") == 0) {
            int exit_status = reveal(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " reveal failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else if (strcmp(args[0], "log") == 0) {
            log_called = true;
            int exit_status = log_c(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " log failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else if (strcmp(args[0], "seek") == 0) {
            int exit_status = seek(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " seek failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }
        
        else if (strcmp(args[0], "proclore") == 0) {
            int exit_status = proclore(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " proclore failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else if (strcmp(args[0], "activities") == 0) {
            activities(OUTPUT);
        }

        else if (strcmp(args[0], "ping") == 0) {
            int exit_status = ping(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " ping failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else if (strcmp(args[0], "fg") == 0) {
            int exit_status = fg(args);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " fg failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else if (strcmp(args[0], "bg") == 0) {
            int exit_status = bg(args);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " bg failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }
        
        else if (strcmp(args[0], "neonate") == 0) {
            neonate(args, OUTPUT);
        }

        else if (strcmp(args[0], "iMan") == 0) {
            int exit_status = iman(args, OUTPUT);
            if (exit_status == 1) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " iman failed.\n\n" RESET);
                free(temp);
                free(command_list);
                return;
            }
        }

        else {
            // Check if command is an alias

            bool is_alias = false, is_func = false;
            for (int i = 0; i < alias_count; i++) {
                if (strcmp(args[0], alias_commands[i].name) == 0) {
                    execute_command(alias_commands[i].command, true, OUTPUT, INPUT);
                    is_alias = true;
                    break;
                }
            }


            // check if command is a function

            if (!is_alias) {
                for (int i = 0; i < func_count; i++) {
                    if (strcmp(args[0], func_commands[i].name) == 0) {
                        is_func = true;
                        
                        // Tokenize args, form command 
                        char* temp_command = strdup(func_commands[i].command);
                        for (int i = 1; args[i] != NULL; i++) {
                            if (i > func_commands[i].arg_count) {
                                fprintf(stderr, RED BOLD "ERROR:" RESET RED " Too many arguments for function.\n\n" RESET);
                                return;
                            }
                            
                            char* replace = (char*) calloc(5, sizeof(char));
                            snprintf(replace, 5, "\"$%d\"", i);
                            temp_command = replace_substrings(temp_command, replace, token);
                        }

                        execute_command(temp_command, true, OUTPUT, INPUT);
                        break;
                    }
                }
            }


            // If neither, try running it via execvp

            if (!is_alias && !is_func) {
                general_bash(args, foreground, OUTPUT, INPUT);
            }
        }

        command = strtok_r(NULL, delim, &saveptr);

        for (int i = 0; args[i] != NULL; i++)
            free(args[i]);
        free(args);

        if (OUTPUT != stdout) {
            fclose(OUTPUT);
            OUTPUT = NULL;
        }
        if (INPUT != stdin) {
            fclose(INPUT);
            INPUT = NULL;
        }

        if (pipefd != NULL) {    
            prev_pipe = pipefd;
        }
    }

    // Update log if necessary

    if (log_called == false) {
        update_log(input);
    }

    // Calculate total time taken to execute command, and add it to next terminal output if necessary

    gettimeofday(&end, NULL);

    long seconds = end.tv_sec - start.tv_sec;
    if (seconds > 2) {
        snprintf(terminal_add, DEFAULT_STRING_SIZE, " %s: %lds", command_list, seconds);
    }
    
    free(command_list);
}


// Function to check if input is in the correct format

bool valid_format(char* input) {

    // Tokenise on spaces, see if any invalid tokens are right next to each other

    char* temp = strdup(input);
    char* token;
    char** tokens = (char**) calloc(1024, sizeof(char*));
    int tc = 0;

    for (tc = 0; tc < 1024 && (token = strtok_r(temp, " \t", &temp)); tc++) {
        tokens[tc] = strdup(token);
        if (strcmp(token, "|") == 0) continue;
        if (strcmp(token, ";") == 0) continue;
        if (strcmp(token, "&") == 0) continue;

        else tokens[tc] = strdup("command\0");
    }

    for (int i = 0; i < tc; i++) {
        if (strcmp(tokens[i], "command") == 0) continue;
        if (strcmp(tokens[i], ";") == 0) continue;

        // Command has to exist before &

        if (strcmp(tokens[i], "&") == 0) {
            if (i == 0) return false;
            if (strcmp(tokens[i - 1], "command") != 0) return false;
        }


        // Command has to exist before and after |

        if (strcmp(tokens[i], "|") == 0) {
            if (i == 0 || i == tc - 1) return false;
            if (strcmp(tokens[i - 1], "command") != 0 || strcmp(tokens[i + 1], "command") != 0) return false;
        }
    }

    return true;
}
