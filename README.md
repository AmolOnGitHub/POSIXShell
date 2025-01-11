[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Qiz9msrr)
# Mini Project 1

- ### [Structure](#structure-1):
    - [shell.c](#shellc)
    - [helper.h](#helperh)
    - [methods.h](#methodsh)
    - [helper.c](#helperc)
    - [execute_command.c](#execute_commandc)
    - [myshrc_handler.c](#myshrc_handlerc)
    - [shell_based.c](#shell_basedc)
    - [file_based.c](#file_basedc)
    - [log.c](#logc)
    - [process_based.c](#process_basedc)
    - [signal_processing.c](#signal_processingc)
    - [neonate.c](#neonatec)
    - [networking.c](#networkingc)

- ### [Assumptions](#assumptions-1)

## Execution:

```
make run
```

## Structure:

### shell.c

- `main()`: Sets up signal handlers, initialises global variables, creates log file, gets CWD information, gets username and system name, parses and initialises aliases and functions, and has the main shell loop where input is taken and executed.

- `term_print()`: Prints the terminal prompt with username, system name, path, and any extra information if necessary.

### helper.h

- Contains all necessary include headers, externs for global variables, macros, structs, and some basic helper functions.

### methods.h

- Contains `.h` definition for all functions required globally, but not in `helper.h`.

### helper.c

- Contains some basic helper functions, such as `chop()`, `strip()`, `path_exists()`, `dir_exists()`, `file_exists()`, `is_dir()`, `is_file()`, `process_relative_path()`, `replace_substrings()`.

### execute_command.c

- `execute_command()`: Takes a given input command, and performs the necessary steps to execute that command, such as input redirection, piping, backgrounding, etc. 

- `valid_format()`: Takes a given input command and checks whether the command is in a valid form, in regards to `&` and `|`.

### myshrc_handler.c

- `get_alias_commands()`: Parses through the `.myshrc` file and identifies and saves all aliases present.

- `get_func_commands()`: Parses through the `.myshrc` file and identifies all functions, and concatenates the commands in the function into one. Also stores how many arguments are required for each function, etc.

- `find_max_arg_count()`: Looks through the concatenated command for each function and finds the number of arguments required for that function.

### shell_based.c

- `hop()`: Performs the hop command as per project specifications.

- `general_bash()`: Takes any bash command, and tries executing it with given arguements. This is where background/foreground processes are run.

### file_based.c

- `reveal()`: Performs the reveal command as per project specifications.

- `seek()`: Performs the seek command as per project specifications.

- `get_directory_data()`: Uses `dirent` to get the necessary files/directories in a directory, and store them in a `data_table`, consisting of `data_row`s.

- `get_data_of_file()`: Gets the data of a file, given the `stat` of the file. Stores it in a `data_row`.

- `lexographic_compare_row()`: Helper function for qsort-ing `data_rows` based off filename.

- `formatted_file_name_print()`: Takes a `data_row` and prints the necessary information, formatted in the necessary way.

- `allocate_column_datarow()`: Allocates memory for a `data_row`.

- `free_data_row()`: Frees memory for a `data_row`.

### log.c

- `log_c()`: Performs the log command as per project specifications.

- `update_log()`: Updates the log file with the given command as per specifications.

- `get_nth_recent_command()`: Fetches the n-th most recent command from the log file.

- `get_num_lines()`: Gets the number of lines from the file, given the path to the file. 

### process_based.c

- `proclore()`: Performs the proclore command as per project specifications. 

- `activities()`: Performs the activities command as per project specifications. 

- `fg()`: Performs the fg command as per project specifications. 

- `bg()`: Performs the bg command as per project specifications. 

- `add_process()`: Given a `process_st`, adds that process to the process list and stores the process' info in the struct. 

- `update_process_info()`: Given the pid of a process, reads `proc` information about the process and updates its info in the list.

- `kill_all_processes()`: Iterates through process list, sending a `SIGKILL` to all processes.

- `handle_foreground()`: Given a process - hands over terminal control to the process, makes it the current foreground process, and handles the termination of the process.

### signal_processing.c

- `ping()`: Performs the ping command as per project specifications.

- `setup_signal_handlers()`: Sets up signal handlers for process termination, interrupt signal, and stop signal.

- `remove_process_pid()`: Removes the given pid from the process list.

- `is_process_pid()`: Checks if the given pid is in the process list.

- `handle_sigchild()`: Signal handler for process termination, prints info on how the process exited, and removes it from the process list post termination.

- `handle_sigint()`: Signal handler for interrupt signal (SIGINT / Ctrl-C).

- `handle_sigtstp()`: Signal handler for stop signal (SIGTSTP / Ctrl-Z). Sends a `SIGSTOP` signal to the current foreground process, and adds the process into the process list.

### neonate.c

- `neonate()`: Performs the neonate function as per project specifications.

- `toggle_raw_mode()`: Enables RAW mode, where keyboard inputs are non-canonical. 

- `reset_raw_mode()`: Disables RAW mode.

- `get_latest_pid()`: Looks in `proc/loadavg/` to find the latest process id.

### networking.c

- `iman()`: Performs the iman function as per project specifications.

- `extract_pre_content`: Takes the response given by the page, and tries extracting the content between the two `<PRE>` and `</PRE>` tag.

- `remove_headers()`: Removes HTTP headers from the response.

- `strcasestr()`: `strstr()`, but case-insensitive.

## Assumptions

### Specification 1

- Assumed maximum arguments to a command to be 256.

### Specification 2

- Assumed that background process outputs should be displayed immediately, as well as its exit status.
- Took maximum input length as 4096.

### Specification 3

- Assumed that all absolute paths dont end with a `/`, but relative paths can.
- Assumed max path length as 4096.
- When executing `hop . ; hop -`, assumed `hop .` sets the previous directory to `.`.
- Assumed no paths have whitespace characters.

### Specification 4

- Assumed maximum of 4096 entries in each directory.
- Assumed files with user execute permissions to be treated as an executable.

### Specification 5

- Log file is created in `~/`.
- Assumed commands such as `reveal -a <space> -l` to be different from `reveal -a <space><space> -l`.

### Specification 6

- Background process outputs are displayed even during interactive processes, such as `vim`.
- Any command shows the total run time of all **foreground processes** in the next terminal line, if the run time is greater than 2.
- Assumed maximum limit of background processes to be 4096.

### Specification 7

- Virtual memory is shown in bytes.

### Specification 8

- Assumed maximum depth of directory tree to be 4096.

### Specification 9

- Assumed maximum of 256 alias commands.
- Assumed aliases would be of the form `alias<space><name><space>=<space>command`
- Assumed functions would be of the form:
```
func<space>function_name()
{
    <command>
    <command>
    ...
}
```

### Specification 10

- Assumed format of commands would only be either `command with args < input > output` or `command with args > output < input`.
- Assumed `echo "Hello World" > newfile.txt` would result in `newfile.txt` containing `"Hello World"`

### Specification 11

- Assumed a `&` at the end of a pipe would imply a background task for the last command of the pipe, rather than the whole pipe.
- Assumed it was fine for ANSI codes to cause a problem in the output file for `reveal -la > newfile.txt`.

### Specification 13

- Assumed that including command args wasn't necessary when printing the command name.

### Specification 16

- Assumed default time (when given no -n flag) to be 5 seconds.

## AI Resouces:

Entire used chat history: [ChatGPT](https://chatgpt.com/share/66e6a9df-8894-8012-ade4-32474898b169)