#include "../helper.h"


void toggle_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void reset_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, 0);
}


pid_t get_latest_pid() {
    FILE* file = fopen("/proc/loadavg", "r");
    if (file == NULL) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " /proc/loadavg doesn't exist.\n\n" RESET);
        return -1;
    }

    char* line = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
    fgets(line, DEFAULT_STRING_SIZE, file);
    fclose(file);
    char* token;
    for (int i = 0; (token = strtok_r(line, " ", &line)) != NULL; i++) {
        if (i == 4) return atoi(token);
    }
}


// Function to print new process every x seconds

void neonate(char** args, FILE* OUTPUT) {
    int time = 5;
    if (args[1] != NULL) {
        if (strcmp(args[1], "-n") == 0) {
            if (args[2] == NULL) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " No time interval given.\n\n" RESET);
                return;
            }
            time = atoi(args[2]);
        }
        else {
            fprintf(stderr, RED BOLD "ERROR:" RESET RED " Invalid flag.\n\n" RESET);
            return;
        }
    } 

    toggle_raw_mode();

    struct timeval start, end;
    gettimeofday(&start, NULL);

    while (true) {
        int pid = get_latest_pid();
        gettimeofday(&end, NULL);
        if (pid != -1 && end.tv_sec - start.tv_sec >= time) {
            fprintf(OUTPUT, "%d\n", pid);
            gettimeofday(&start, NULL);
        }

        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            if (ch == 'x') {
                reset_raw_mode();
                printf("Terminating neonate..\n");
                break;
            }
        }
    }
    
}