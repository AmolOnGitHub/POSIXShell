#include "../methods.h"

void update_process_info(pid_t pid);

// Function to add a process to the process list, stored in lexographical order

void add_process(process_st* process) {
    if (processes == NULL) {
        processes = process;
        update_process_info(process->pid);
        return;
    }
    
    process_st* temp = processes;
    while (temp->next && strcmp(temp->next->name, process->name) < 0) {
        temp = temp->next;
    }
    process->next = temp->next;
    temp->next = process;
    process->status = (char*) calloc(4, sizeof(char));
    process->exe_path = (char*) calloc(257, sizeof(char));

    update_process_info(process->pid);
}


// Function to add process information to a process structure

void update_process_info(pid_t pid) {
    
    // Find process in the process list

    process_st* process = processes;
    while (process) {
        if (process->pid == pid) break;
        process = process->next;
    }

    if (process == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Process doesn't exist.\n\n" RESET);
        return;
    }

    usleep(100); // Let the process update its status

    // Read the process information from /proc/pid/stat and /proc/pid/exe

    char* proc_stat_path = (char*) calloc(256, sizeof(char));
    char* proc_exe_path = (char*) calloc(256, sizeof(char));

    snprintf(proc_stat_path, 256, "/proc/%d/stat", process->pid);
    snprintf(proc_exe_path, 256, "/proc/%d/exe", process->pid);

    FILE* stat_file = fopen(proc_stat_path, "r");
    if (stat_file == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Process doesn't exist.\n\n" RESET);
        free(proc_stat_path);
        free(proc_exe_path);
        return;
    }

    char status;
    int group_id;
    unsigned long vm_size;
    char* process_state = (char*) calloc(3, sizeof(char));
    char* exe_path = (char*) calloc(256, sizeof(char));
    
    char* line = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
    fgets(line, DEFAULT_STRING_SIZE, stat_file);
    char* token;
    for (int i = 0; (token = strtok_r(line, " ", &line)) != NULL; i++) {
        if (i == 2) status = token[0];
        else if (i == 5) group_id = atoi(token);
        else if (i == 22) vm_size = atol(token);
    }

    process_state[0] = status;
    if (group_id == tcgetpgrp(STDIN_FILENO))
        process_state[1] = '+';

    ssize_t len = readlink(proc_exe_path, exe_path, 255);
    exe_path[len] = '\0';

    process->status = process_state;
    process->exe_path = exe_path;
    process->group_id = group_id;
    process->vm_size = vm_size;
}


// Function to kill all processes

void kill_all_processes() {
    process_st* trav = processes->next;
    while (trav) {
        if (trav->pid != getpid()) {
            kill(trav->pid, SIGKILL);
        }
        trav = trav->next;
    }
}


// Function to print the process information

int proclore(char** args, FILE* OUTPUT) {

    // If no arguments are given, print information about the current process

    pid_t pid = getpid();
    if (args[1] != NULL) {
        pid = atoi(args[1]);
    }
    
    // Check if the process exists

    process_st* trav = processes;
    while (trav) {
        if (trav->pid == pid) break;
        trav = trav->next;
    }

    if (trav == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Process doesn't exist.\n\n" RESET);
        return 1;
    }

    // Print the process information

    fprintf(OUTPUT, "pid: %d\n", trav->pid);
    fprintf(OUTPUT, "Process State: %s\n", trav->status);
    fprintf(OUTPUT, "Process Group ID: %d\n", trav->group_id);
    fprintf(OUTPUT, "Virtual Memory Size: %lu\n", trav->vm_size);
    fprintf(OUTPUT, "Executable Path: %s\n\n", trav->exe_path);

    return 0;
}


// Function to list all currently running processes

void activities(FILE* OUTPUT) {
    process_st* trav = processes->next;
    if (trav == NULL) {
        printf(RED "No processes currently running.\n\n" RESET);
        return;
    }
    while (trav) {
        fprintf(OUTPUT, "%d : %s - %s\n", trav->pid, trav->name, strcmp(trav->status, "T") == 0 ? "Stopped" : "Running");
        trav = trav->next;
    }
    printf("\n");
}


// Function to handle foreground processes

void handle_foreground(process_st* process) {
    setpgid(process->pid, 0);
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, process->pid);

    // printf("Setting %d as foreground process.\n", process->pid);
    current_foreground = process->pid;
    strcpy(current_foreground_name, process->name);

    int status;
    waitpid(process->pid, &status, WUNTRACED);

    if (WIFSTOPPED(status)) {
        handle_sigtstp(SIGTSTP); 
    }

    tcsetpgrp(STDIN_FILENO, getpgrp());
    signal(SIGTTOU, SIG_DFL);
}


// Function to bring a process to the foreground

int fg(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " No process ID given.\n\n" RESET);
        return 1;
    }

    pid_t pid = atoi(args[1]);
    process_st* trav = processes;
    while (trav) {
        if (trav->pid == pid) break;
        trav = trav->next;
    }

    if (trav == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Process doesn't exist.\n\n" RESET);
        return 1;
    }

    if (kill(pid, SIGCONT) == -1) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Unable to continue process.\n\n" RESET);
        return 1;
    }

    handle_foreground(trav);    
    return 0;
}


// Function to start a stopped background process

int bg(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " No process ID given.\n\n" RESET);
        return 1;
    }

    pid_t pid = atoi(args[1]);
    process_st* trav = processes;
    while (trav) {
        if (trav->pid == pid) break;
        trav = trav->next;
    }

    if (trav == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Process doesn't exist.\n\n" RESET);
        return 1;
    }

    if (kill(pid, SIGCONT) == -1) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Unable to continue process.\n\n" RESET);
        return 1;
    }

    update_process_info(pid);
    return 0;
}
