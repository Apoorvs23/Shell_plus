#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include<limits.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define Max_Permitted_Arguments 5
char* shell; // to name the shell 
int background_process[100]={-1}; //keeping the array of all paused process
int current_index_Bg=0; // current index that we know that the process is background processing
int bg_flag=0; // whether user have executed the case of background processing...

void execute_command(char *args[], int no_of_arguments) {
    // Forking the child process
    int pid = fork(); 

    if (pid > 0) 
    {
        // This is a parent process
        int status;
        // Waiting for child process that and it will continue when child will finish
        waitpid(pid, &status, 0); 
        
    } 
    else if (pid == 0) {
        // we are in child process
        // Executing the command given by args
        execvp(args[0], args); 
        
        // If we are here then it means execvp failed....
        // char* msg = "execvp failed...\n";
        // printf(msg); // pop the msg that it is failed
        
        exit(EXIT_FAILURE);  // Exit the child process with failure
        
    } 
    else {
        // Now Forking is  failed
        char* msg = "fork failed :( ...\n";
        printf(msg); // Again Notify about the failure
    }
}

int execute_command_operator(char *command, char *args[]) 
{
     // Forking the child process
    int pid = fork();

    if (pid >0) 
    {
        // This process is parent
        int status;
         // Waiting for child process that and it will continue when child will finish
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) 
        {
            // Normal exit of the child
            if (WEXITSTATUS(status) == 0) 
            {
                // Command executed successfully
                return 1;
            } 
            else 
            {
                // Command executed but returned an error
                return 0;
            }
        } 
        else 
        {
            // Child process exited abnormally
            // printf("Child process exited abnormally");
            return 0;
        }

       
    } 
    else if (pid == 0) 
    {
       // we are in child process
        // Executing the command given by args
        if (execvp(command, args) == -1) {

               // If we are here then it means execvp failed....
            printf("bash: %s: command not found\n", command);
            exit(EXIT_FAILURE);
        }
    } 
    else 
    {
       // Now Forking is  failed
        char* msg = "fork failed :( ...\n";
        printf(msg); // Again Notify about the failure
    }

    return -1;
}

void abs_path(char* path) {
    // Allocating memory for the result path
    char* result_path = (char*)malloc(PATH_MAX * sizeof(char));
    
    // Checking if memory allocation failed
    if (result_path == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // Initializing pointers for iterating over the input path and the result path
    char* pos = path; // Pointer for iterating over the input path
    char* result_iterator = result_path; // Pointer for iterating over the result path
    
    // Iterating over the input path
    while (*pos != '\0') // while we dont encounter end of the path
    {
        // Checking if the current character is '~' (tilde)
        if (*pos == '~')
        {
            // Get the value of the HOME environment variable
            char* get_home = getenv("HOME");
            // Checking if we are failed to retrieve home
            if (get_home == NULL) 
            {
                char* msg="Getting HOME Variable Failed\n";
                printf(msg);
                free(result_path);
                return;
            }
            // Storing HOME environment variable's value to the result path
            strcpy(result_iterator, get_home);
            // Moving the result_iterator pointer to the end of the copied HOME value by adding length of the home path
            result_iterator += strlen(get_home);
            // Moving to the next character in the input path
            pos++;
        } 
        else 
        {
            // Copying the current character from the input path to the result path
            *result_iterator = *pos;
            // Moving to the next character in both input and result paths
            result_iterator++;
            pos++;
        }
    }
    
    // Null-terminating our result string
    *result_iterator = '\0';
    
    // Copying the result path back to the original path
    strcpy(path, result_path);
    
    // Freeing up the dynamically allocated memory for the result path
    free(result_path);
}



char* my_strdup(const char* str) { // this function is to copy a  string 
    // Geting the length of the input string
    int length = strlen(str);

    // Allocating  memory = string + null character
    char* duplicate = (char*)malloc((length + 1) * sizeof(char));
    if (duplicate == NULL)
    {
        return NULL; // Memory allocation got failed
    }

    // Copying the contents of the original string into the newly created string
    strcpy(duplicate, str);

    return duplicate;
}

char* trimSpaces(char *str)  // this function is to trim leading and trailing spaces
{
    int i = 0, j = strlen(str) - 1;

    // Trim leading spaces

    while (str[i] == ' ') 
    {
        i++;
    }

    // Trim trailing spaces
    while (str[j] == ' ') 
    {
        j--;
    }

    // Shifting characters to the beginning of the string
    for (int r = 0; r <= j - i; r++) 
    {
        str[r] = str[r + i];
    }

    // Terminating the string with null characters
    str[j - i + 1] = '\0';
    return str;
}

//this function is to tokenize the string on the bases of spaces
// This function tokenizes a string based on a given delimiter (e.g., space)
// It stores the tokens in an array and keeps track of the number of tokens
int tokenize(char* command, int *no_of_arguments, char **args, char* delimiter, int max_counter) 
{  
    // Initializing a pointer to store each token
    char *token;
    
    // Geting the first token by  using strtok
    token = strtok(command, delimiter);
   
    
    // Looping through each token and store it in the args array
    while (token != NULL && (*no_of_arguments)< (max_counter + 1)) 
    {
        token = trimSpaces(token); //trimming spaces from front and end
        // Storing the token in the args array
        args[*no_of_arguments] = token;
        
        // Incrementing the argument variable count AFTER printing the token
        (*no_of_arguments)++;
        
        // Get the next token
        token = strtok(NULL, delimiter);
    }

        // If there are more tokens than the maximum allowed arguments, print an error
    if(token != NULL && *no_of_arguments > Max_Permitted_Arguments)
    {
        printf("ERROR -- Invalid no of arguments, Allowed - more than 1 and less than 6\n");
       return 1;
    }

// Return 0 to indicate successful tokenization
    return 0;

}
// to check if the given no of arguments are valid or not
int check_no_of_arguments_valid(int no_of_arguments)
{
    if (no_of_arguments < 1 || no_of_arguments > Max_Permitted_Arguments) 
    {
            return 0;
    }
    return 1;
}

// this code will convert 1 # 2 # 3 to cat 1 2 3
void work_with_hash(char* command) {
    // Making a copy of the command
    char* command_copy = my_strdup(command);
    if (command_copy == NULL) {
        printf("strdup failed");
        return;
    }

    // Tokenizing the copied command based on whitespace and '#' delimiter
    int no_of_arguments = 0;
    char *args[Max_Permitted_Arguments + 2]; // +2 for command name and NULL terminator - storing all
    
    // Initialize the first argument as "cat" 
    args[0] = "cat";
    no_of_arguments++;

    // Tokenizing the command_copy string based on whitespace and '#' delimiter
    int result = tokenize(command_copy, &no_of_arguments, args, " # ", Max_Permitted_Arguments);
    if (result) 
    {
        // If tokenization fails, return
        return;
    }

    // Seting the last argument as NULL and adjusting the argument count
    args[no_of_arguments] = NULL;
    no_of_arguments--;

    // Executing the cat command
    execute_command(args, no_of_arguments);

    // Free the memory allocated for the copy of the command
    free(command_copy);
}

// we have created a new function internal tokenizer since it is different from the above tokenizer function - in this we are not printing extra messages and also we are passig an iterator
void internal_tokenizer(int* argc, char ** args2, char** args, int i) {
    // Initializing a pointer to store each token
    char* token = strtok(args[i], " ");
    
    // Looping through each token and store it in the args2 array
    while (token != NULL && *argc < Max_Permitted_Arguments + 1) 
    {
        // Trimming leading and trailing spaces from the token
        token = trimSpaces(token);

        // Storing the token in the args2 array and increment argc
        args2[(*argc)++] = token;

        // Geting the next token
        token = strtok(NULL, " ");
    }

    // NULL-terminating the args2 array
    args2[*argc] = NULL;
}

// we are waiting for all the childs of the parent
void parent_wait(int childs) // make parent wait for each command to execute
{   int child=0;
    //wait for every process
    while (child < childs)
    {
        wait(NULL); //waiting for any child
        child++;
    }

} 

// to close the pipes
void closePipes(int (*pipes)[2], int no_of_arguments, int read_end, int write_end) 
{
    int i = 0;

    // Loop through each pipe
    while (i < no_of_arguments - 1) 
    {
        // we will close the read end of the pipe now
        close(pipes[i][read_end]);
        
        // we will close the write end of the pipe now
        close(pipes[i][write_end]);
        
        // we will go to the next pipe now
        i++;
    }
}

//dupe_input duplicates the input from the previous command onto the standard input of the current command and closes unnecessary pipe ends

void dupe_input(int (*pipes)[2], int i, int read_end, int write_end) 
{
    // Duplicate the input file descriptor
    dup2(pipes[i - 1][read_end], STDIN_FILENO);
    
    // Read end pipe of the pipe will be closed as it's now been duplicated
    close(pipes[i - 1][read_end]);
    
    // Closing the write end of the pipe as not needed for input
    close(pipes[i - 1][write_end]);
}

//dupe_output duplicates the output of the current command onto the standard output for the next command and closes unnecessary pipe ends.
void dupe_output(int (*pipes)[2], int i, int read_end, int write_end) 
{
    // Duplicating the output file descriptor 
    dup2(pipes[i][write_end], STDOUT_FILENO);
    
    // Closing the read end of the pipe, we not need for the output
    close(pipes[i][read_end]);
    
    // Closing the write end of the pipe as it's been duplicated onto STDOUT_FILENO
    close(pipes[i][write_end]);
}

int check_arguments_valid(int argc) // this function is strictly written for redirections
{
    // Check if the number of arguments is not equal to 2
    if (argc != 2) {
        // If not, return 0 indicating invalid arguments
        return 0;
    }
    // If the number of arguments is 2, return 1 indicating valid arguments
    return 1;
}

/*
It tokenizes a command string containing piped commands, creates pipes for communication between commands, 
forks child processes to execute each command, 
duplicates input and output file descriptors as needed for piping, and waits for all child processes to finish.
*/
void pipe_code(char *command) 
{
    int no_of_arguments = 0;
    int max_pipe_permitted = 6;
    int total_arguments = 6 + 1 + 1; // 8 = 6 pipes will have 7 arguments and 1 function name
    char *args[total_arguments];

    // Tokenizing the command by '|' to get individual commands
    int result = tokenize(command, &no_of_arguments, args, "|", total_arguments);

    // Checking if the number of commands exceeds the maximum allowed (6 pipes)
    if (no_of_arguments > 7)
    {
        char * msg="Only 6 pipes allowed - You have exceeded the total limit permited\n";
        printf(msg);
        return;
    }

    /*
    The code  portion creates a multiple pipes, one for each pair  of adjacent commands in a pipeline. It iterates over the  number of commands
     minus one, creating a pipe for each pair. If the creation of any pipe fails, it prints an  error message and exits the program. 
     These pipes facilitate communication between the processes executing the commands in the pipeline.
    */
    // Creating pipes
    int pipes[no_of_arguments - 1][2];
    for (int i = 0; i < no_of_arguments - 1; i++) 
    {
        if (pipe(pipes[i]) == -1) // it will create the pipe
        {
            printf("piping operation failed\n");
            exit(EXIT_FAILURE);
        }
    }


    int read_end = 0;
    int write_end = 1;

    // Traversing through each command
    for (int i = 0; i < no_of_arguments; i++) 
    {
        char *args2[Max_Permitted_Arguments + 2]; // +2 for command name and NULL terminator
        int argc = 0;

        // Tokenizing the command by ' ' to get individual arguments
        internal_tokenizer(&argc, args2, args, i);

        // Checking if the number of arguments exceeds the maximum allowed
        if (argc > Max_Permitted_Arguments) 
        {
            char* msg = "Total arguments greater than 5 which not permiited/allowed\n";
            printf(msg);
            return;
        }

        // Saving standard input and output file descriptors
        int input = dup(0);
        int output = dup(1);

        // Execute commands
        int pid = fork();
        if (pid == -1) 
        {
            printf("Issue in forking -- Failed\n");

            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) // This is child process
        { 
            // Redirect input if not the first command
            if (i != 0)
            {
                dupe_input(pipes, i, 0, 1);
            }
            // Redirecting output if not the last command
            if (i != no_of_arguments - 1)
            {
                dupe_output(pipes, i, 0, 1);
            }
            // Closinn unnecessary pipe descriptors
            closePipes(pipes, no_of_arguments, 0, 1);

            // Executing the command
            execvp(args2[0], args2);
            // if we are here this means execvp failed...
            printf("execvp failed in Piping Operation\n");
            exit(EXIT_FAILURE);
        }
    }

    // Closing all pipe descriptors in the parent process
    closePipes(pipes, no_of_arguments, read_end, write_end);

    // Waiting for all child processes to finish
    parent_wait(no_of_arguments);
}

int duping(int x, int y) {
    // Duplicating file descriptor x onto file descriptor y
    if (dup2(x, y) == -1) {
        // If dup2() fails, close all the  file descriptors and print an error message
        close(x);
        close(y);
        char* failed = "dup2 failed \n";
        printf(failed);
        return 1; // Return 1 so we can say dup - failed
    }
    return 0; // Returning 0 to indicate success
}

void input_code(char* command) {
    char *args[Max_Permitted_Arguments + 2]; // +2 for command name and NULL terminator
    int argc = 0;

    // Tokenizing the command  on '<' delimiter to separate command and the file name
    tokenize(command, &argc, args, "<", Max_Permitted_Arguments);
    
    // Checking if the arguments are valid for input redirection
    if (!check_arguments_valid(argc)) 
    {
        char* msg= "Incorrect arguments - < should have two arguments in left and right\n";
        printf(msg);
        return;
    }

    // Extracting the file name and trimming all leading/trailing spaces
    char *file_name = trimSpaces(args[1]); //args[1] is file name

    // Saving the current stdin file descriptor
    int std_input_fd = dup(STDIN_FILENO);
    
    // Opening the file so that we can read it
    int input_fd = open(file_name, O_RDONLY);
    if (input_fd == -1) 
    {
        char* msg= "Issue on opening the file\n";
        printf(msg);
        return;
    }
    
    // Redirecting input from the file to stdin
    if (duping(input_fd, STDIN_FILENO)) return;
    
    // Closing the file descriptor for the file
    close(input_fd);

    // Tokenizing the command before redirection
    argc = 0;
    tokenize(args[0], &argc, args, " ", Max_Permitted_Arguments);
    args[argc] = NULL; // NULL-terminate the array

    // Executing the command with input redirected from the file
    execute_command(args, argc);

    // Restoring the original stdin file descriptor
    if (duping(std_input_fd, STDIN_FILENO)) return;
    close(std_input_fd);
}

void output_code(char* command) {
    // Geting the absolute path of the command in case there is ~ so home will be appended in front
    abs_path(command);
    
    // Tokenizing the command based on '>' operator to separate both the given left command and the given file 
    char* delimeter =">";
    char *first_half_cmd = strtok(command, delimeter);

    char *file = strtok(NULL, delimeter);
    
    // Checking if the command or file is NULL
    if (first_half_cmd == NULL || file == NULL) 
    {
        char* msg="Issue with the command syntax given for the parsing...\n";
        printf(msg);
        return;
    }

    // Triming leading and trailing spaces from the file name
    file = trimSpaces(file);

    // Array created to store command and arguments
    char *args[Max_Permitted_Arguments + 2]; // +2 for command name and NULL terminator
    int argc = 0;
    
    // Saving original stdout file descriptor
    int fd_stdout = dup(STDOUT_FILENO);

    // Tokenizing the first part of the command based on space
    tokenize(first_half_cmd, &argc, args, " ", Max_Permitted_Arguments);

    // Checking if the number of arguments exceeds the maximum permiteed
    if (argc > Max_Permitted_Arguments) {
        printf("Resubmit the command: to have arguments betwwen 1 and 5\n");
        return;
    }

    // NULL-terminating the arguments array
    args[argc] = NULL;

    // Open the file in 3 modes - we have to create it if is not present and truncate if it is there.
    int fd = open(file, O_WRONLY | O_TRUNC | O_CREAT  , 0777);
    if (fd == -1)
    {
        printf("File '%s' --> not present\n", file);
        return;
    }

    // pushing stdout to the given file
    if (duping(fd, STDOUT_FILENO)) return;

    // Close our file decripter
    close(fd);

    // Executing CMD with output redirected to the file
    execute_command(args, argc);

    // Restoring standard output
    if (duping(fd_stdout, STDOUT_FILENO)) return;
    close(fd_stdout);
}

void double_arrow(char* command) {
     // Geting the absolute path of the command in case there is ~ so home will be appended in front
    abs_path(command);
    
    // Tokenizing the command based on '>>' to separate both the given left command and the given file 
    char* delimeter=">>";
    char *first_half_cmd = strtok(command, delimeter);

    char *file = strtok(NULL, delimeter);
    
    // Checking if the command or file is NULL
    if (first_half_cmd == NULL || file == NULL) {
        printf("Invalid command syntax\n");
        return;
    }

    // Trim,ing leading and trailing spaces from the file name
    file = trimSpaces(file);

    // Array to store command and arguments
    char *args[Max_Permitted_Arguments + 2]; // +2 for command name and NULL terminator
    int argc = 0;
    
    // Saving original stdout file descriptor
    int fd_stdout = dup(STDOUT_FILENO);

    // Tokenizing the first part of the command
    tokenize(first_half_cmd, &argc, args, " ", Max_Permitted_Arguments);

    // Check if the number of arguments exceeds the maximum allowed
    if (argc > Max_Permitted_Arguments) {
        printf("Resubmit the command: to have arguments betwwen 1 and 5\n");
        return;
    }

    // NULL-terminating the arguments array
    args[argc] = NULL;

    // We will open the file now and only create if doesnot exist
    int fd;
    if (access(file, F_OK) != -1) //if file exists
    {
        // Open file in Append mode
        fd = open(file, O_WRONLY | O_APPEND, 0777); // only Append permission is added rest all is same as we wdid in >
    } 
    else  // if file doesnot exists
    {
        // File is not there or exists - create a new file
        fd = open(file, O_WRONLY |O_CREAT | O_APPEND , 0777);
    }
    if (fd == -1) 
    {
        printf("File '%s' not found\n", file);
        return;
    }

    // pushing stdout to the given file
    if (duping(fd, STDOUT_FILENO)) return;

    // Close our file descripter
    close(fd);

    // Executing the CMD with output is appended to the file
    execute_command(args, argc);

    // Restoring standard output
    if (duping(fd_stdout, STDOUT_FILENO)) return;
    close(fd_stdout);
}

void seq_execute_cmd(char* command) 
{
    char *args[Max_Permitted_Arguments + 1]; // Null terminate will take +1 of the spave
    int argc = 0; // storing no. of arguments

    // Tokenizing the command based on ';'
    tokenize(command, &argc, args, ";", Max_Permitted_Arguments);
    
    // Check if the number of commands exceeds the maximum allowed
    if (argc > Max_Permitted_Arguments)
    {
        printf("Resubmit the command: to have arguments betwwen 1 and 5\n");
        return;
    }
   
    // NULL-terminate the array
    args[argc] = NULL;

    // We will Execute each command sequentially
    for (int i = 0; i < argc; i++) 
    {
        // Tokenizing each command on space and executomg it
        char *args2[Max_Permitted_Arguments + 2]; // +2 for command name and NULL terminator
        int argc2 = 0; // arguments
        
        // Tokenizing the current command
        tokenize(args[i], &argc2, args2, " ", Max_Permitted_Arguments);
        
        // NULL-terminate the array
        args2[argc2] = NULL;
        
        // Executing the command
        if (argc2 >= 1 && argc2 <= Max_Permitted_Arguments)
        {
            execute_command(args2, argc2);
        } 
        else 
        {
            printf("Resubmit the command: to have arguments betwwen 1 and 5\n");
            return;
        }
    }
}

void background(char *args[], int no_of_arguments) // the main point of making a process as background process is to stop parent waiting for it
{
    int pid = fork();

    if (pid > 0) 
    {
         // Parent process
       background_process[current_index_Bg]=pid;
       current_index_Bg++;
      // printf("adding process to background:%d\n",pid);

    } 
    else if (pid == 0) 
    {
        // Child process
        execvp(args[0], args);
        // If we are here - this means execvp failed
        printf("execvp failed in background function... \n");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        // Fork failed
        printf("forking failed in background function... \n");
        return;
    }
}

void modify_string_bg(char* command)
{
      command[strlen(command) - 1] = '\0'; // Remove the '&' character
}

void fg_process() 
{
     int process_id =  background_process[current_index_Bg - 1];

    // Check if there are background processes in the array and bring the last one to the foreground
    if (current_index_Bg > 0 && process_id != -1)
    {
       // printf("current_idx: \n%d", current_index_Bg);
        int status;
    
        // Wait for the last background process
        printf("Process ID: %d will come to foreground now\n",process_id);
        waitpid(process_id, &status, 0); //start making parent wait again

        
        // Reset the background process and update the index
        background_process[current_index_Bg - 1] = -1;
        current_index_Bg--;
    } 
    else // there are 0 background processes
    {
        char* msg="There are 0 background processs so no processes can be bring to foreground\n";
        printf(msg);
        return;
    }
}

int new_terminal(char *command, char* terminal1) 
{
    int pid = fork(); // Fork to detach the terminal process

    if (pid > 0) // we are in the parent
    {
        // parent process is happy to get a child
        return 1;
       
    } 
    else if (pid == 0) 
    {
        // Child process
        // Try different terminal emulators
        if (execlp(terminal1, terminal1, "-e", command, NULL) != -1) // -e means execution of the given program - which is shell24 in our case
        {
            exit(EXIT_SUCCESS); // Success
        }
        exit(EXIT_FAILURE); // we are here means that excelp failed
    } 
    else 
    {
         // Fork failed
        printf("forking failed in creating a new terminal \n");
        return 0; // Failure
    }
}
/*
if operator &, previous command gives 0 - dont execute the next command and skip it.
if operator &, previous command gives 1 -  execute the next command.

if operator ||, previous command gives 0 - execute the next command.
if operator ||, previous command gives 1 - dont execute the next command and skip it.
*/
void special_op(char* command) 
{

    int resultant = 0; // to store the output of the command

    char* temp_command = (char*)malloc(strlen(command) + 1); // Allocating memory for temp_command,  we will store individual commands

    if (temp_command == NULL) 
    {
        printf("Memory allocation failed in special operator function.\n");
        return;
    }

    temp_command[0] = '\0'; // Making temp_command empty


    for (int i = 0; command[i] != '\0'; i++) //iterate till you reach end of the string
    {

        if(temp_command == NULL) 
        {
            temp_command = (char*)malloc(strlen(command) + 1); // assign memory to temp_command
        }

        if(command[i] != '&' && command[i] != '|') 
        {
            strncat(temp_command, &command[i], 1); // puting each character from command to temp_command  till we dont encounter either & or |
        }
        else // we have encountered & or |
        {
               // we will execute the command 
            char* args[Max_Permitted_Arguments + 2];
            int no_of_arguments = 0;
            tokenize(temp_command, &no_of_arguments, args, " ", Max_Permitted_Arguments); //we will tokenize the command based on the spaces
            if(!check_no_of_arguments_valid(no_of_arguments)) // checking if the no of arguments are invalid of each subcommand
            {
                return;
            }
            args[no_of_arguments] = NULL;

            resultant = execute_command_operator(args[0], args); //execute the comand store the resultant

            // make the temp_command NULL as we have already executed 
            temp_command = NULL;

           // dsfd || echo 3 && echo 4
            if(command[i]=='&')
            {
                if (resultant)
                {
                    //skip one more & and now we will execute the command since previous operation was successful
                    i++;
                }
                else 
                {
                     // skip all & till we dont get the first | - reason - because 0 && something will always be 0
                    for(int j=i; command[j] != '\0'; j++) 
                    {
                        if (command[j]=='|') 
                        {
                            break;
                        }
                        if(command[i+1]!='\0')
                        {
                                i++;
                        }
                    }
                    if(command[i]=='|')
                    {
                        i++; //because there will be one more |, - so we can execute now.
                    }
                }
            }
            else // we got ||
            {
                if (!resultant) 
                {
                    // if we are here this means 0 || something, we will execute something
                    i++; // i++ because there are two pipe operators (||)
        
                }
                else 
                {
                    // skip it till we get first end
                    for(int j=i; command[j] != '\0'; j++) 
                    {
                        if (command[j]=='&') 
                        {
                            break; // we got it - skip it so above code can execute the command
                        }
                        if(command[i+1]!='\0')
                        {
                            i++;
                        }
                    }
                    if(command[i]=='&')
                    {
                        i++; // +1 to skip it here
                    }
                }
            }
        }
    }

    if(temp_command != NULL) //if we are at last like a && b, now since we have to execute b and we havent found any operator so thats why we will exuecte b here
    {
        char* args[Max_Permitted_Arguments + 2];
        int no_of_arguments = 0;
        tokenize(temp_command, &no_of_arguments, args, " ", Max_Permitted_Arguments);
        args[no_of_arguments] = NULL;
        resultant = execute_command_operator(args[0], args);
    }
}

int check_operator_valid(char* command) // this function will count total occurence of & and | -> it should be in specified limits
{
    int count=0;
    int length = strlen(command);
    for(int i =0 ; i<length;i++)
    {
        if(command[i]=='&' || command[i]=='|')
        {
            count++;
            i=i+1; // because there are two && and ||
        }
    }
    if(count<1 || count>Max_Permitted_Arguments) 
    {
        return 0; // return false that it is not valid
    }
    else
    {
        return 1; // return 1 or true that it is valid
    }
}
int main() 
{
    umask(0000);
    while (1==1) 
    {
        bg_flag=0;
        char command[256];
        shell="shell24$ ";
        printf(shell);
      
        fflush(stdout);

        // Reading the user input
        if (fgets(command, sizeof(command), stdin) == NULL) // read a line from standard input and put that in command
        {
            char* error_message="ERROR in FGETS\n";
            printf(error_message);
            exit(EXIT_FAILURE);
        }
        if (strcmp(command, "\n") == 0) // if enter is pressed do nothing
        {
            continue;
        }
        // Removing trailing newline character
        char *newline_position = strchr(command, '\n');
        if (newline_position != NULL) 
        {
            *newline_position = '\0';
        }

    // Checking if the command contains a '#' symbol, indicating a hash command, we need to do cat
        if(strstr(command, "#"))
        {
            work_with_hash(command);
            continue;
        }
    //do logical operations
        if(strstr(command, "||") || strstr(command, "&&"))
        {
             // Checking if the number of logical operators is valid
            int valid=check_operator_valid(command);
            if(!valid)
            {
                printf("No. of operators are less than 1 or more than 5 - INVALID \n ");
                continue;;
            }
           special_op(command);
           continue;
        }
     // Checking if the command contains '<', indicating input redirection   
        if(strstr(command, "<"))
        {
            input_code(command);
           continue;
        }
    // Checking if the command contains '>>', indicating append output redirection
        if(strstr(command,">>"))
        {
            double_arrow(command);
            continue;
        }

        // Checking if the command contains '>', indicating output redirection
        if(strstr(command, ">"))
        {
           output_code(command);
           continue;
        }

        // Checking  if the command contains ';', indicating sequential execution
        if(strstr(command, ";"))
        {
            seq_execute_cmd(command);
            continue;
        }
        // Checking if the command contains '&', indicating background execution
        if(strstr(command, "&"))
        {
            int count=0;
            for(int i=0;i<strlen(command);i++)
            {
                if(command[i]=='&') count++;
            }
            if(count==1)
            {
                
                bg_flag=1;
                modify_string_bg(command);
            }
           
        }

        // Checking if the command is 'fg', to bring a background process to the foreground
        if(strcmp(command, "fg") == 0)
        {
        
            fg_process();
            continue;
        }

       // Checking if the command contains '|', indicating piping operation
        if(strstr(command, "|"))
        {
           pipe_code(command);
           continue;
        }
        // if we are this means no above operation is requested

        char *args[Max_Permitted_Arguments + 1]; // +2 for command name and NULL terminator - storing all 
        int no_of_arguments = 0;
       int result= tokenize(command, &no_of_arguments, args, " ", Max_Permitted_Arguments); // tokenizing the command on the basis of the space

        for (int i = 1; i < no_of_arguments; i++) 
        {
            if(strstr((args[i]), "~"))
            {
                abs_path(args[i]); // appending HOME wherever required
            }
        }

        args[no_of_arguments] = NULL; // NULL-terminating the array

         if (strcmp(args[0], "newt") == 0) //if command is newt open a new terminal
         {
            char * welcome_msg="Opening an new shell24 Terminal for you :) \n";
            printf(welcome_msg);

            char* Terminal1 = "xterm"; // Xterm is the terminal in no machine
            // Fork and execute a new shell24 session
           if (!new_terminal("./shell+",Terminal1)) // it will open a new session
           {
                char* error="shell24 failed to open... :( \n)";
                printf(error);
               continue;
           }
        } 

        if(bg_flag) // if user requested a background operation
        {
            if (no_of_arguments >= 1 && no_of_arguments <= Max_Permitted_Arguments) 
            {
               background(args,no_of_arguments); //make the process to background
            } 
           
             continue;
        }

        if (no_of_arguments >= 1 && no_of_arguments <= Max_Permitted_Arguments) 
        {
            execute_command(args, no_of_arguments); // now execute the normal command
        }
        else 
        {
            continue; //just return because msg will automitically printed by tokenization function 
        }     
    }

    return 0;
}