#include "../helper.h"

void remove_process_pid(pid_t pid);
process_st* is_process_pid(pid_t pid);

void handle_sigchild(int sig);
void handle_sigint(int sig);
void handle_sigtstp(int sig);


// Function to setup signal handlers

void setup_signal_handlers() {
    struct sigaction sa_chld, sa_int, sa_tstp, sa_quit;
    
    // Process termination signal

    sa_chld.sa_handler = &handle_sigchild;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);


    // Interrupt signal

    sa_int.sa_handler = &handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);


    // Stop signal

    sa_tstp.sa_handler = &handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}


// Function to send signals to processes

int ping(char** args, FILE* OUTPUT) {
    if (args[1] == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " No PID provided.\n\n" RESET);
        return 1;
    }
    if (args[2] == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " No signal number provided.\n\n" RESET);
        return 1;
    }

    pid_t pid = atoi(args[1]);
    int sig = atoi(args[2]) % 32;

    if (is_process_pid(pid) == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Process doesn't exist.\n\n" RESET);
        return 1;
    }

    if (kill(pid, sig) == -1) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " kill() failed.\n\n" RESET);
        return 1;
    }

    update_process_info(pid);
    fprintf(OUTPUT, "Sent signal %d to PID %d.\n\n", sig, pid);
}


// Functions to handle necessary signals, and print appropriate messages

void handle_sigchild(int sig) {
    int status;
    pid_t pid;
    process_st* process;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        if ((process = is_process_pid(pid)) != NULL) {
            if (WIFEXITED(status)) {
                printf(GREEN BOLD "PID %d:" RESET " %s exited normally.\n", process->pid, process->name);
            } else {
                printf(RED BOLD "PID %d:" RESET " %s exited abnormally.\n", process->pid, process->name);
            }
        }
        else {
            current_foreground = -1;
        }
        remove_process_pid(pid);
    }
}

void handle_sigint(int sig) {
    // printf("%d\n", current_foreground);
    if (current_foreground > 0) {
        kill(current_foreground, SIGINT);
        current_foreground = 0;
    }
}

void handle_sigtstp(int sig) {
    // printf("%d\n", current_foreground); fflush(stdout);
    if (current_foreground > 0) {
        kill(current_foreground, SIGSTOP);
        process_st *process = (process_st*) calloc(1, sizeof(process_st));
        process->pid = current_foreground;
        process->name = strdup(current_foreground_name);
        add_process(process);
        current_foreground = 0;
    } 
}

void remove_process_pid(pid_t pid) {
    process_st* trav = processes;
    while (trav->next) {
        if (trav->next->pid == pid) {
            process_st* temp = trav->next;
            trav->next = trav->next->next;
            free(temp);
            break;
        }
        trav = trav->next;
    }
}

process_st* is_process_pid(pid_t pid) {
    process_st* trav = processes;
    while (trav->next) {
        if (trav->next->pid == pid) return trav->next;
        trav = trav->next;
    }
    return NULL;
}