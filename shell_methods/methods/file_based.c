#include "../methods.h"

int lexographic_compare_row(const void* a, const void* b);
void formatted_filename_print(char** data_row, FILE* OUTPUT);
char** allocate_column_datarow(char** data_row);
char** get_data_of_file(char** data_row, struct stat* fs, char* filename);
char*** get_directory_data(char* path, int* n, int* total_blocks);
void free_data_row(char** data_row);


// Function to reveal the details of a file or directory

int reveal(char** args, FILE* OUTPUT) {

    // Check for flags and path

    bool l_flag = false, a_flag = false;
    char* path = strdup(cur_cwd);

    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-' && strlen(args[i]) > 1) {
            for (int j = 1; args[i][j] != '\0'; j++) {
                if (args[i][j] == 'l') l_flag = true;
                else if (args[i][j] == 'a') a_flag = true;
                else {
                    fprintf(stderr, RED BOLD "ERROR:" RESET RED " Invalid flag.\n\n" RESET);
                    return 1;
                }
            }
        }
        else path = strdup(args[i]);
    }


    // Path processing necessary for relative paths

    if (path[0] != '/') { 
        char* temp_path = process_relative_path(path);
        free(path);
        path = strdup(temp_path);
        free(temp_path);
    }

    if (!path_exists(path)) return 1;
    

    // When it is just one file, we handle it seperately as we want just the details of that single file

    if (is_file(path)) {
        struct stat file_stat;
        if (stat(path, &file_stat) != 0) {
            fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " Path doesn't exist.\n\n" RESET);
            return 1;
        }

        char** data_row = (char**) calloc(7, sizeof(char*));
        allocate_column_datarow(data_row);
        char* filename = strrchr(path, '/') + 1;

        get_data_of_file(data_row, &file_stat, filename);

        if (l_flag)
            fprintf(OUTPUT, "%10s  %2s  %-10s %-8s %9s   %12s   ", data_row[0], data_row[1], data_row[2], data_row[3], data_row[4], data_row[5]);
        formatted_filename_print(data_row, OUTPUT);

        fprintf(OUTPUT, "\n");
        free(data_row);
        free(path);
        return 0;
    }
    
    
    // Get data for the directory we want

    int n = 0, total_blocks;
    char*** data_table = get_directory_data(path, &n, &total_blocks);
    if (data_table == NULL) {
        free(path);
        return 1;
    }


    // Output based on flags & ordering

    qsort(data_table, n, sizeof(char**), lexographic_compare_row);

    if (l_flag) fprintf(OUTPUT, "Total %d\n", total_blocks / 2);

    for (int i = 0; i < n; i++) {
        if (!a_flag && data_table[i][6][0] == '.') continue;

        if (l_flag)
            fprintf(OUTPUT, "%10s  %2s  %-10s %-8s %9s   %12s   ", data_table[i][0], data_table[i][1], data_table[i][2], data_table[i][3], data_table[i][4], data_table[i][5]);
        formatted_filename_print(data_table[i], OUTPUT);
    }

    fprintf(OUTPUT, "\n");
    free(path);

    for (int i = 0; i < 1000; i++)
        free_data_row(data_table[i]);
    free(data_table);

    return 0;
}


// Function to seek through a directory

int seek(char** args, FILE* OUTPUT) {

    // Check for flags and path

    bool d_flag = false, f_flag = false, e_flag = false;
    char* path = strdup(cur_cwd);
    char* search = NULL;
    
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-' && strlen(args[i]) > 1) {
            for (int j = 1; args[i][j] != '\0'; j++) {
                if (args[i][j] == 'd') d_flag = true;
                else if (args[i][j] == 'f') f_flag = true;
                else if (args[i][j] == 'e') e_flag = true;
                else {
                    fprintf(stderr, RED BOLD "ERROR:" RESET RED " Invalid flag.\n\n" RESET);
                    return 1;
                }
            }
        }
        else {
            if (search == NULL) search = strdup(args[i]);
            else path = strdup(args[i]);
        }
    }


    // Check for conflicting flags

    if (d_flag && f_flag) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Cannot use both -d and -f flags.\n\n" RESET);
        return 1;
    }


    // Path processing necessary for relative paths

    if (path[0] != '/') {
        char* temp_path = process_relative_path(path);
        free(path);
        if (temp_path == NULL) return 1;
        path = strdup(temp_path);
        free(temp_path);
    }

    if (!dir_exists(path)) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path doesn't exist.\n\n" RESET);
        return 1;
    }


    // Seek through the directory, similar to BFS

    char** path_queue = (char**) calloc(4096, sizeof(char*));
    char* temp_path = (char*) calloc(PATH_MAX, sizeof(char));
    char* e_path = (char*) calloc(PATH_MAX, sizeof(char)); // Path for -e flag, when only one item is found
    char* e_perms = (char*) calloc(11, sizeof(char));      // Perms for -e flag

    int front = 0, queue_size = 0, found = 0;
    path_queue[queue_size++] = strdup(path);

    while (front < queue_size) {
        int n = 0;
        char*** data_table = get_directory_data(path_queue[front], &n, NULL);
        if (data_table == NULL) {
            fprintf(stderr, RED BOLD "ERROR:" RESET RED " Directory data retrieval failed.\n\n" RESET);
            free(path);
            free(path_queue);
            free(temp_path);
            return 1;
        }

        // Get front of queue to process
        char* dir_path = path_queue[front];       

        for (int i = 0; i < n; i++) {
            char** data_row = data_table[i];
            if (strcmp(data_row[6], ".") == 0 || strcmp(data_row[6], "..") == 0) continue;
            
            found++;
            snprintf(temp_path, PATH_MAX, "%s/%s", dir_path, data_row[6]);

            if (data_row[0][0] == 'd') {
                path_queue[queue_size++] = strdup(temp_path);
            }

            if (strncmp(data_row[6], search, strlen(search)) != 0) continue;
            if (data_row[0][0] == 'd' && f_flag) continue;
            if (data_row[0][0] == '-' && d_flag) continue;

            fprintf(OUTPUT, ".%s\n\n", temp_path + strlen(path));
            strcpy(e_path, temp_path);
            strcpy(e_perms, data_row[0]);
        }

        front++;
    }

    if (found == 0)
        fprintf(stderr, RED "No files found.\n\n" RESET);

    
    // Handle -e flag

    if (found == 1 && e_flag) {
        if (e_perms[0] == 'd') {
            strcpy(cur_cwd, e_path);
        }
        else if (e_perms[3] == 'x') {
            pid_t pid = fork();

            if (pid < 0) {
                fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " fork() failed.\n\n" RESET);
                return 2;
            }
            else if (pid == 0) {
                char* exec_args[] = {e_path, NULL};
                if (execv(exec_args[0], exec_args) == -1) {
                    fprintf(stderr, RED BOLD "ERROR:" RESET RED " Executable not found.\n\n" RESET);
                    exit(1);
                }
                exit(EXIT_FAILURE);
            } else {
                int status;
                waitpid(pid, &status, WUNTRACED);
                printf("\n");
            }
        }
        else {
            FILE* file = fopen(e_path, "r");
            if (file == NULL) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " File not found.\n\n" RESET);
                return 1;
            }
            char* buf = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
            while (fgets(buf, DEFAULT_STRING_SIZE, file) != NULL) {
                fprintf(OUTPUT, "%s", buf);
            }
            
            fprintf(OUTPUT, "\n");
            printf("\n");
            fclose(file);
            free(buf);
        }
        
    }
    
    free(path);
    free(path_queue);
    free(temp_path);
    free(e_path);
    free(e_perms);

    return 0;
}


// Function to sort the data_table based on the filename

int lexographic_compare_row(const void* a, const void* b) {
    char **row_a = *(char***)a;
    char **row_b = *(char***)b;

    return strcmp(row_a[6], row_b[6]);
}


// Function to print the filename in a formatted way 

void formatted_filename_print(char** data_row, FILE* OUTPUT) {
    if (data_row[0][0] == 'd') fprintf(OUTPUT, BLUE BOLD "%s\n" RESET, data_row[6]);
    else if (data_row[0][3] == 'x') fprintf(OUTPUT, GREEN BOLD "%s\n" RESET, data_row[6]);
    else fprintf(OUTPUT, "%s\n", data_row[6]);
}


// Function to allocate memory for the data_row

char** allocate_column_datarow(char** data_row) {
    data_row[0] = (char*) calloc(11, sizeof(char));
    data_row[1] = (char*) calloc(5, sizeof(char));
    data_row[2] = (char*) calloc(128, sizeof(char));
    data_row[3] = (char*) calloc(128, sizeof(char));
    data_row[4] = (char*) calloc(11, sizeof(char));
    data_row[5] = (char*) calloc(13, sizeof(char));
    data_row[6] = (char*) calloc(128, sizeof(char));

    return data_row;
}


// Function to get the data of a file

char** get_data_of_file(char** data_row, struct stat* fs, char* filename) {
    snprintf(data_row[0], 11, "%s%s%s%s%s%s%s%s%s%s",
        (S_ISDIR(fs->st_mode)) ? "d" : "-",
        (fs->st_mode & S_IRUSR) ? "r" : "-",
        (fs->st_mode & S_IWUSR) ? "w" : "-",
        (fs->st_mode & S_IXUSR) ? "x" : "-",
        (fs->st_mode & S_IRGRP) ? "r" : "-",
        (fs->st_mode & S_IWGRP) ? "w" : "-",
        (fs->st_mode & S_IXGRP) ? "x" : "-",
        (fs->st_mode & S_IROTH) ? "r" : "-",
        (fs->st_mode & S_IWOTH) ? "w" : "-",
        (fs->st_mode & S_IXOTH) ? "x" : "-"
    ); // Permissions


    snprintf(data_row[1], 5, "%2lu", fs->st_nlink); // Links 
    snprintf(data_row[2], 128, "%s", getpwuid(fs->st_uid)->pw_name); // Owner
    snprintf(data_row[3], 128, "%s", getgrgid(fs->st_gid)->gr_name); // Group
    snprintf(data_row[4], 11, "%ld", fs->st_size); // Size
    strftime(data_row[5], 13, "%b %d %H:%M", localtime(&fs->st_mtime)); // Date
    snprintf(data_row[6], 128, "%s", filename); // Name

    return data_row;
}


// Function to get the data of a directory

char*** get_directory_data(char* path, int* n, int* total_blocks) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " opendir() failed.\n\n" RESET);
        return NULL;
    }

    struct dirent *entry;
    struct stat file_stat;
    char* full_path = (char*) calloc(PATH_MAX, sizeof(char));
    long tb = 0;

    char*** data_table = (char***) calloc(4096, sizeof(char**));
    for (int i = 0; i < 4096; i++) {
        // We have permissions, links, owner, group, size, date, name
        data_table[i] = (char**) calloc(7, sizeof(char*));
        allocate_column_datarow(data_table[i]);
    }

    int i = 0;
    for (i = 0; (entry = readdir(dir)) != NULL; i++) {
        snprintf(full_path, PATH_MAX, "%s/%s", path, entry->d_name);

        if (stat(full_path, &file_stat) != 0) {
            fprintf(stderr, YELLOW BOLD "ERROR:" RESET YELLOW " Path doesn't exist.\n\n" RESET);
            return NULL;
        }

        if (S_ISREG(file_stat.st_mode) || S_ISDIR(file_stat.st_mode)) {
            struct stat *fs = &file_stat;
            tb += fs->st_blocks;
            get_data_of_file(data_table[i], fs, entry->d_name);
        }
    }

    free(entry);
    free(full_path);
    closedir(dir);

    *n = i;
    if (total_blocks != NULL) *total_blocks = tb;
    return data_table;
}


// Function to free the data_row

void free_data_row(char** data_row) {
    free(data_row[0]);
    free(data_row[1]);
    free(data_row[2]);
    free(data_row[3]);
    free(data_row[4]);
    free(data_row[5]);
    free(data_row[6]);
    free(data_row);
}