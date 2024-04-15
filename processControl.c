#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
int parentID=-1; //parent ID of the process_id
int found=0; //this flag will be used check whther there exists any child/grandchild/sibling for any perticular request
int no_of_digits(int n) //it will give no. of digits in the process id, n is a process id
 {
    int count_digits=0;
    while(n) 
    {
        count_digits++;
        n/=10;
    }
    return count_digits;
}

char* create_custom_command(char* command,int process_id) //general function to create commands
 {
    int process_id_digits = no_of_digits(process_id); //got current process id digits
    int command_size = strlen(command) + 2* process_id_digits; //giving double the buffer
    char* custom_command = (char*)malloc((command_size + 2) * sizeof(char)); 
    if (custom_command == NULL) 
    {
        printf("Heap memory Allocation failed\n");
        return NULL; 
    }

    strcpy(custom_command, command);
    char process_id_str[process_id_digits]; 
    sprintf(process_id_str, "%d", process_id);
    strcat(custom_command, process_id_str);

    return custom_command; //created custom command for the given process_id
}

int search_process_in_root_process(int process_id, int root_process_id)
 {
    if(process_id < root_process_id) //because process_id should always be greater than root process id because its descendent
    {
        return 0;
    }  
    if(process_id==root_process_id)
    {
        return 1; // since process_id and root_price id will are having same process tree
    }

    char* command_get_parent = create_custom_command("ps -o ppid= -p ",process_id); //it will parent id of the current process_id
    if(command_get_parent == NULL) 
    {
        return 0; // issue in creating the command
    }
    
    FILE* file_pointer = popen(command_get_parent, "r");// popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
    if (file_pointer == NULL) 
    {
        printf("issue in getting parentID\n");
        return 0;
    }

    char parent_id_string[100]; // keeping max process id size 100

    //fgets will reads character from filepointer and store parent_id_string and total no. of characters stored will be sizeof(parent_id_string)
    if (fgets(parent_id_string, sizeof(parent_id_string), file_pointer) == NULL)  
    {
        // Error or end of file
        printf("fgets failed\n");
        pclose(file_pointer); // Close the file pointer
        return 0;
    }

    pclose(file_pointer); // Closing the file pointer
    
    // converting parent_id to int
    int parent_id = atoi(parent_id_string);
    
    if(parentID==-1) //this means this the first time we are executing this recursive call, so storing the parent of process_id in a global variable
    {
            parentID = parent_id;

    }
    if(parent_id == root_process_id) //we got our process_id in the process tree, we can return true
    {
        return 1;
    }
    //if atoi gives 0 to parse or the process is init - we cant get any root for it which will have init in its path so returning 0
    else if(!parent_id || parent_id == 1) 
    {
        return 0;
    }
    else 
    {
        
        return search_process_in_root_process(parent_id, root_process_id); //recursively calling the function by replacing the process_id with parent_id
    }
}
int get_parent(int process_id) //will return the parent of the given process_id
{
    char* command_get_parent = create_custom_command("ps -o ppid= -p ",process_id); //dynamically creating the command
    if(command_get_parent == NULL) 
    {
        return 0; // returning 0 because there is issue in creating the command
    }
    
    FILE* file_pointer = popen(command_get_parent, "r");  // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
    free(command_get_parent); // freeing up the memory
    if (file_pointer == NULL) 
    {
        printf("issue in getting parentID\n");
        return 0;
    }

    
    char parent_id_string[100]; // keeping max process id size 100

    //fgets will read character from filepointer and store parent_id_string and total no. of characters stored will be sizeof(parent_id_string)
    if (fgets(parent_id_string, sizeof(parent_id_string), file_pointer) == NULL) 
    {
        
        printf("fgets failed in get_parent function()\n");
        pclose(file_pointer); // closing the pointer
        return 0;
    }

    pclose(file_pointer); // closing the pointer
    
    // Extract the PPID from the output
    int parent_id = atoi(parent_id_string);
    return parent_id;
}
int check_defunc(int process_id) 
{
    char* defunc_command = create_custom_command("ps -o state= -p ", process_id); //it will give status of the process if status is z (this means zombie). T means it is stopped, S means it is running
    if (defunc_command == NULL) 
    {
        return 0; 
    }

    FILE* file_pointer = popen(defunc_command, "r"); // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
    if (file_pointer == NULL) 
    {
        printf("Issue in executing defunc_command\n");
        free(defunc_command); // freeing up the memory
        return 0;
    }

    char status[3]; // for "Z\n" and null terminator

    //fgets will reads character from filepointer and store status of process_id
    if (fgets(status, sizeof(status), file_pointer) == NULL)
    {
        // Error or end of file
        printf("fgets failed in check_defunc\n");
        pclose(file_pointer); // Close the file pointer
        free(defunc_command); // freeing up the memory
        return 0;
    }

    pclose(file_pointer); //closing up the file 
    free(defunc_command); // freeing up the memory
    return strcmp(status, "Z\n") == 0; // it will return 1 if we the status is Z which means zombie
}

void imd_desc_xd(int process_id)
{
    char* command_get_child = create_custom_command("pgrep -P ", process_id);  // this will give us all the child of the prcoess ids
    if(command_get_child == NULL) 
    {
        return; // issue in creating the command
    }
    
    FILE* file_pointer = popen(command_get_child, "r"); // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
  
    if (file_pointer == NULL) 
    {
        printf("issue in executing the command in imd_desc_xd Function()\n");
        return;
    }

    char child_id_string[100]; //
    
    //fgets will reads character from filepointer and store child_id_string and total no. of characters stored will be sizeof(child_id_string)
    while (fgets(child_id_string, sizeof(child_id_string), file_pointer) != NULL) 
    {
        if(!found) found=1;
        printf("Child Processes ID:%s\n", child_id_string);
    }
    printf("\n"); // breaking the line after all child processes are printed
    free(command_get_child); // freeing up the memory
    pclose(file_pointer); // Close the file pointer
}
void sbl_process_xs(int process_id) //listing sibling process ids
{
    int parent= get_parent(process_id);
    char* command_get_parent = create_custom_command("pgrep -P ", parent); //get all children of the parent
    if(command_get_parent == NULL) 
    {
        return; // Failed to create command, return false
    }
    
    FILE* file_pointer = popen(command_get_parent, "r"); // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
  
    if (file_pointer == NULL) 
    {
        printf("issue in getting parentID\n");
        return;
    }
    //fgets will reads character from filepointer and store child_id_string and total no. of characters stored will be sizeof(child_id_string)
    char child_id_string[100]; // keeping max process id size 100


    //fgets will reads character from filepointer and store child_id_string and total no. of characters stored will be sizeof(child_id_string)
    while (fgets(child_id_string, sizeof(child_id_string), file_pointer) != NULL) 
    {
        if(atoi(child_id_string) != process_id)
        {
            if(!found) found=1;
            printf("Sibling Processes ID:%s\n", child_id_string);
        }
    }
    free(command_get_parent); // freeing up the memory
    pclose(file_pointer); // Close the file pointer

}
void stop_process_xt(int process_id) //to stop the process using SIGSTOP and storing the stopped process ids in stopped_process_ids.txt
{
    int process_id_digits = no_of_digits(process_id); //gettign digits of the process id
    if (kill(process_id, SIGSTOP) == -1) 
    {
        printf("SIGSTOP signal sending caused an error.\n");
        return;
    }
    printf("Process ID: %d is paused.\n", process_id);
    int process_file = open("stopped_process_ids.txt", O_CREAT | O_RDWR | O_APPEND, 0744); // opening the file to start writing the paused process_id to the file, // 7 - rwx permission to user, 4 - readonly to group, 4 -readonly to others
    if (process_file == -1) 
    {
        printf("Error opening file in stop_process_xt function \n");
        return;
    }
    char reader[process_id_digits + 2]; //+2 to for \n and \0
    sprintf(reader, "%d\n", process_id);
    if (write(process_file, reader, strlen(reader)) == -1) //start writiting the process_id of the stopped process to the file
    {
        printf("Error writing to file in stop_process_xt function()\n");
        close(process_file);
        return;
    }
    close(process_file);
}

void  cont_process_xc(int process_id,int root_process_id) // to send sigcont to the processes paused by -xt
{
    int process_file_fd = open("stopped_process_ids.txt", O_RDONLY); //this file will have list of all the process ids paused by -xt
    if (process_file_fd == -1) 
    {
        printf("No processes are currently stopped by -xt\n");
        return;
    }
    
    char reader;
    char process_id_str[100]; // keeping max process id size 100
    int process_id_str_index = 0; //index 

    // code will read line by line from the fill and will send SIGCONT to the process ids.
    while (read(process_file_fd, &reader, 1) > 0) 
    {
        if (reader == '\n' || reader == '\0') //if the line is ended or file is completed read 
        {
            int process_id = atoi(process_id_str); //converting string to int

            if (kill(process_id, SIGCONT) == -1)
            {
                printf("Process ID:%d cant be killed (either process doesnot exists)\n",process_id);
            }
            else
            {
                printf("Sent SIGCONT signal to process with PID %d\n", process_id);
            }
            // Reset the process_id_str_index for next process ID
            process_id_str_index = 0;
        } 
        else 
        {
            process_id_str[process_id_str_index++] = reader;
        }
    }
    
    close(process_file_fd);
    
    // Delete the file
    if (remove("stopped_process_ids.txt") != 0) //deleting the file as all the process are continued so we dont require list of stopped process ids.
    {
        printf("Error in deleting stopped_process_ids.txt file\n");
        return;
    }
}
    
    
void non_direct_process_xn(int process_id, int current_level) //listing all the non-direct descendants of the given process id
{
    char* command_get_child = create_custom_command("pgrep -P ", process_id);
    if(command_get_child == NULL) 
    {
        return; // Failed to create command, return false
    }
    
    FILE* file_pointer = popen(command_get_child, "r"); // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
  
    if (file_pointer == NULL) 
    {
        printf("issue in getting childID\n");
        return;
    }

    char child_id_string[100]; // keeping max process id size 100

    //fgets will reads character from filepointer and store child_id_string and total no. of characters stored will be sizeof(child_id_string)
    while (fgets(child_id_string, sizeof(child_id_string), file_pointer) != NULL) 
    {
        if(current_level) //this means we are not on level 0 - 0 is direct desendants
        {
            if(!found) found=1;
            printf("Processes IDs of Non-Descendent: %s\n",child_id_string); //printing id of non-direct descendents 
        }
        int child_id = atoi(child_id_string); //converting it to int
        non_direct_process_xn(child_id,1+current_level); //recursively calling the function and increasing the current_level+1
        
    }
    free(command_get_child); // freeing up the memory of our command
    pclose(file_pointer); // Closing the file pointer
}
void grand_process_xg(int process_id, int current_level) //function to print the grandchildren of current process id
{
    char* command_get_child = create_custom_command("pgrep -P ", process_id); //it will give child of the current process id
    if(command_get_child == NULL) 
    {
        return; // issue in creating the command
    }
    
    FILE* file_pointer = popen(command_get_child, "r"); // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 
  
    if (file_pointer == NULL) 
    {
        printf("issue in getting childID\n");
        return;
    }

    char child_id_string[100]; // keeping max process id size 100

 //fgets will reads character from filepointer and store child_id_string and total no. of characters stored will be sizeof(child_id_string)
    while (fgets(child_id_string, sizeof(child_id_string), file_pointer) != NULL) 
    {
        if(current_level==1) //the level after direct children
        {
            if(!found) found=1;
            printf("Processes IDs of Grand-Childerns are: %s\n",child_id_string); //printing ids of grand_children
        }
        int child_id = atoi(child_id_string);
        if(current_level<2) grand_process_xg(child_id,1+current_level); //only call the function if we didnt reached grand_children level - so function will be called for level=0 and level=1
        else return;
        
    }
    free(command_get_child); // freeing up the memory
    pclose(file_pointer); // Closing the file pointer
}

void kill_process(int process_id,char* msg) //will kill the process, msg means "Process_id" or Root_process_id
{
    if(check_defunc(process_id))
    {
          printf("%s with PID %d is defunc, kill operation wont be called on\n", msg, process_id);
          return;
    }
    else if (kill(process_id, SIGKILL) == 0)
    {
        printf("%s with PID %d killed successfully.\n", msg, process_id);
    } 
    else
    {
        printf("Error killing process\n");
        return;
    }
}


void defunc(int process_id) //defunc will recursively call itself and will use check_defunc
{
    char* command_defunc = create_custom_command("pgrep -P ", process_id); //it will give child of process_id
    if (command_defunc == NULL)
     {
        return; 
    }

    FILE* file_pointer = popen(command_defunc, "r"); // popen spawn a new process and execute this command and r means pipe is opened for reading and at last it returns file 

    if (file_pointer == NULL) 
    {
        printf("Issue in executing the command in defunc() function\n");
        free(command_defunc); // freeing up the memory
        return;
    }

    char child_id_string[100]; // keeping max process id size 100
    //fgets will reads character from filepointer and store child_id_string and total no. of characters stored will be sizeof(child_id_string)
    while (fgets(child_id_string, sizeof(child_id_string), file_pointer) != NULL)
    {

        int child_id = atoi(child_id_string);
        if (check_defunc(child_id)) //will check for each child_id  whether it is defunc or not
        {
            if(!found) found=1;
            printf("Process ID of Defunct Process: %d\n", child_id);
        }
        defunc(child_id); //recursively calling the function for the child

    }
    free(command_defunc); // freeing up the memory
    pclose(file_pointer); // Close the file pointer
}

void function(int process_id,int root_process_id, char* options)
{
    if(options==NULL) return;
    else if(strcmp(options, "-xd") == 0 ) 
    {
        imd_desc_xd(process_id); // printing immediate descendants of process_id
        if(!found) printf("No immediate desendants of the given process ID: %d \n",process_id);
    }
    else if(strcmp(options, "-xs") == 0 )
    {
        sbl_process_xs(process_id); // printing PIDs of all the sibling processes of process_id
        if(!found) printf("No siblings of the given process ID: %d \n",process_id);
    }
    else if(strcmp(options, "-xt") == 0 )
    {
        stop_process_xt(process_id); // stopping given process_id
    }
    else if(strcmp(options, "-xc") == 0 )
    {
        cont_process_xc(process_id,root_process_id); // making all process_id continue which were stopped by xt
    }
    else if(strcmp(options, "-xn") == 0 )
    {
        int current_level=0; 
        non_direct_process_xn(process_id, current_level); // printing all non direct descendants of given process_id
        if(!found) printf("No non-direct descendants of the given process ID: %d\n",process_id);
    }
    else if(strcmp(options, "-xg") == 0 )
    {
        int current_level=0;
        grand_process_xg(process_id, current_level); // printing all the grand_children of the curren process_id
        if(!found) printf("No grand children of the given process ID: %d\n",process_id);
    }
    else if(strcmp(options, "-rp") == 0 )
    {
        kill_process(process_id,"process_id"); // killing the process
    }
    else if(strcmp(options, "-pr") == 0 )
    {
        kill_process(root_process_id,"root_process_id"); //killing the root process 
    }
    else if(strcmp(options, "-xz") == 0 )
    {
        defunc(process_id); // printing all defunc pids which are decendants of given process_id
        if(!found) printf("No defunc decendants of the given process ID: %d\n",process_id);
    }
    else if(strcmp(options, "-zs") == 0)
    {
        if(check_defunc(process_id)) //checking whether gven process_id is defunc or not
        {
            printf("The process is Defunc\n");
        }
        else
        {
            printf("The process is not Defunc\n");
        }
    }
    else
    {
        printf("Incorrect option!\n");
    }

}

int main(int no_of_arguments, char *args[]) 
{
    if(no_of_arguments == 3 || no_of_arguments == 4) //3 means that options are not provided
    {
        // getting command line arguments
        int process_id = atoi(args[1]); //atoi will convert string to int
        int root_process_id = atoi(args[2]);
        char* options = args[3];
        if(kill(process_id,0)!=0) //to check whether process_id exists or not
        {
            printf("The process_id: %d is invalid\n",process_id);
            return 1;
        }
        if(kill(root_process_id,0)!=0) //to check whether root_process_id exists or not
        {
            printf("The root_process_id: %d is invalid\n",root_process_id);
            return 1;
        }

        // Checking if process_id exists in process tree of root_process
        if (search_process_in_root_process(process_id, root_process_id))
        {
            // Printing PID and PPID of the process_id if it exists in the same process tree
            printf("PID: %d\nPPID: %d\n", process_id, parentID);
            if(no_of_arguments == 4) function(process_id,root_process_id,options); //once it exists, we calling this function which will take options and execute different function based on different arguments
        } 
        else 
        {
            printf("Does not belong to the process tree\n");
        }

        return 0;
    }
    else 
    {
        printf("There should be 4 arguments in the function  %s: function name, process_id, root_process_id, option\n", args[0]);
        return 1;
    }
}
