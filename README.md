# Shell+
Summary: An enhanced shell with additional features supporting user commands, piping, redirection, and execution. Enhanced system capabilities include advanced process management, file control, and background processing. 
Data integrity and rapid recovery are ensured by implementing a versatile backup script with various backup modes.


# Project Overview

## Shell24 - An enhanced Shell

### Description
Shell24 is a C program shell that operates in an infinite loop, awaiting user commands. Upon receiving a command, the program assembles and executes it using `fork()`, `exec()`, and other necessary system calls. It adheres to specific rules and conditions outlined below:

### Features
- **Modular Approach:** The program is designed with modularity in mind, ensuring ease of maintenance and extensibility.
- **Rule-Based Command Execution:** Commands are executed according to predefined rules and conditions.
- **Support for Special Characters:** Shell24 handles various special characters such as piping, redirection, conditional execution, background processing, and sequential execution of commands.

### Rules and Conditions
1. **Rule 1:** The `newt` command (`shell24$newt`) creates a new copy of Shell24. There's no upper limit on the number of terminal sessions that can be opened.
2. **Rule 2:** The `argc` (argument count) of any command/program should be between 1 and 5, inclusive.
3. **Rule 3:** The `argc` of individual commands or programs used with special characters must also be between 1 and 5.

### Special Characters
Shell24 supports handling the following special characters:
- `#` for text file concatenation.
- `|` for piping.
- `>`, `<`, `>>` for redirection.
- `&&` and `||` for conditional execution.
- `&` for background processing.
- `;` for sequential execution of commands.

## Inbuilt Process Control (a2prc.c)

### Description
The `a2prc.c` program searches for a process in the process tree rooted at a specified process and prints requested information based on input parameters.

### Synopsis
```
a2prc [process_id] [root_process] [OPTION]
```

### Options
- `-rp`: Kills the process if it belongs to the specified process tree.
- `-pr`: Kills the root process.
- `-xn`: Lists the PIDs of all non-direct descendants of the specified process.
- `-xd`: Lists the PIDs of all immediate descendants of the specified process.
- `-xs`: Lists the PIDs of all sibling processes of the specified process.
- `-xt`: Pauses the specified process.
- `-xc`: Resumes processes paused earlier.
- `-xz`: Lists the PIDs of all defunct descendants of the specified process.
- `-xg`: Lists the PIDs of all grandchildren of the specified process.
- `-zs`: Prints the status of the specified process (Defunct/Not Defunct).

## Inbuilt File Control (fileutil)

### Description
The `fileutil` program performs various file-related operations based on specified arguments and options.

### Synopsis
```
fileutil [root_dir] filename
fileutil [root_dir] [storage_dir] [options] filename
fileutil [root_dir] [storage_dir] extension
```

### Options
- `-cp`: Copies the file.
- `-mv`: Moves the file.

## Backup Script (Icbackup.sh)

### Description
The `Icbackup.sh` bash script performs continuous backup operations for files in a specified directory tree.

### Steps
1. Creates a complete backup of all files in the directory tree.
2. Creates incremental backups of newly created or modified files.
3. Creates a differential backup of newly created or modified files.
4. Continuously loops through steps 1-3.

## Additional Notes
- All programs include appropriate error handling and comments.
- `nftw()` function is utilized for file tree traversal.
- Weekly complete backups and daily incremental backups are recommended in practical scenarios.
