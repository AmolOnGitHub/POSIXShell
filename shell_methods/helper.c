#include "helper.h"

// Function to chop off the first chop_length characters from a string

void chop(char* str, int chop_length) {
    int str_length = strlen(str);
    if (str_length < chop_length) return;
    memmove(str, str + chop_length, str_length - chop_length + 1);
}


// Function to strip leading and trailing whitespaces from a string

void strip(char* str) {
    int frontcut = 0;
    for (frontcut = 0; isspace(*(str + frontcut)); frontcut++);
    chop(str, frontcut);
    
    if (*str == '\0') return;

    char* end;
    for (end = str + strlen(str) - 1; end > str && isspace(*end); end--);

    memset(end + 1, '\0', strlen(end + 1));
}


// Function to check if a path exists

int path_exists(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path doesn't exist.\n\n" RESET);
        return 0;
    }

    return 1;
}


// Function to check if a directory exists

int dir_exists(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path doesn't exist.\n\n" RESET);
        return 0;
    }

    if (!S_ISDIR(sb.st_mode)) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path is not a directory.\n\n" RESET);
        return 0;
    }

    return 1;
}


// Function to check if a file exists

int file_exists(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path doesn't exist.\n\n" RESET);
        return 0;
    }

    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path is not a file.\n\n" RESET);
        return 0;
    }

    return 1;
}


// Function to check if a path is a directory

int is_dir(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path doesn't exist.\n\n" RESET);
        return 0;
    }

    return S_ISDIR(sb.st_mode);
}


// Function to check if a path is a file

int is_file(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        fprintf(stderr, RED BOLD "ERROR:" RESET RED " Provided path doesn't exist.\n\n" RESET);
        return 0;
    }

    return S_ISREG(sb.st_mode);
}


// Function to process a relative path, and return an absolute path

char* process_relative_path(char* relative) {
    char* temp_path = strdup(cur_cwd);
    char* dir;

    while ((dir = strtok_r(relative, "/", &relative))) {
        if (strcmp(dir, ".") == 0) continue;
        else if (strcmp(dir, "..") == 0) {
            char* end;
            for (end = temp_path + strlen(temp_path) - 1; end > temp_path && *end != '/'; end--);
            memset(end, '\0', strlen(end));
        }
        else if (strcmp(dir, "~") == 0) {
            temp_path = strcpy(temp_path, cwd);
        }
        else if (strcmp(dir, "-") == 0) {
            if (strcmp(prev_cwd, "") == 0) {
                fprintf(stderr, RED BOLD "ERROR:" RESET RED " No previous directory exists.\n\n" RESET);
                return NULL;
            }
            temp_path = strcpy(temp_path, prev_cwd);
        }
        else {
            char *temp_dircheck_path = (char*) calloc(PATH_MAX, sizeof(char));
            snprintf(temp_dircheck_path, PATH_MAX, "%s/%s", temp_path, dir);
            
            if (!path_exists(temp_dircheck_path)) {
                free(temp_dircheck_path);
                return NULL;
            }

            temp_path = strcpy(temp_path, temp_dircheck_path);
            free(temp_dircheck_path);
        }
    }
    
    return temp_path;
}


// Function to replace all occurrences of a substring in a string with another substring

char* replace_substrings(char* str, char* substr, char* replacement) {
    char* result = (char*) calloc(DEFAULT_STRING_SIZE, sizeof(char));
    char* temp = strdup(str);
    char* token;
    
    while ((token = strtok_r(temp, substr, &temp)) != NULL) {
        snprintf(result, DEFAULT_STRING_SIZE, "%s%s%s", result, token, replacement);
    }
    
    return result;
}