#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include<limits.h>
//#include<sys/wait.h>

//these variables are declred global so we can use them in each function
int found=-1; // tracking whether the file is found or not
char *get_file_name=NULL; // name of the file
char *get_root=NULL; // root directory
char *get_directory=NULL; //storage directory
const char *get_options=NULL; // options -mv or -cp
char *get_extension=NULL; // any extension that starts with . and it will help us in making .tar 
char * temp_destination=NULL; // temperorary variable for storage directory

char *get_full_destination_path(const char *destination, const char *filename) // this function will append destination and filename together
{
    int destination_size = strlen(destination) + strlen(filename) + 2; // +1 for the '/' and +1 for null character
    char *destination_with_filename_Local = (char *)malloc(destination_size);
    if(destination_with_filename_Local==NULL)
    {
        printf("Error in allocating memory in setting up destination path\n");
        exit(0);
    }
    strcpy(destination_with_filename_Local, destination);
    strcat(destination_with_filename_Local, "/");
    strcat(destination_with_filename_Local, filename);
    return destination_with_filename_Local;
}
int check_options_invalid(const char* options) //check whether given options are valid or not
{
     if(strcmp(options, "-mv")==0 || strcmp(options, "-cp")==0) //we have only two valid options
     {
        return 1;
     }
     return 0;
}
int check_directory_exists(const char* path) //check whether given directory is valid or not
{
    return access(path,F_OK); // a library function in <unistd.h> to check whether the file exists or not
}
char* abs_path( char* path) //if the path is relative it will give the absolute path, if it is already an absolute path, it will return as it is.
{
    char* result_path = NULL;
    char c = path[0];

    if (c != '/') 
    {
        char* get_home = getenv("HOME");
        if (get_home == NULL) 
        {
            printf("There is an issue with the HOME variable\n");
            return NULL;
        }
        result_path = (char*)malloc(PATH_MAX * sizeof(char));
        if (result_path == NULL)
        {
            printf("Memory allocation failed\n");
            return NULL;
        }
        strcpy(result_path, get_home);
        strcat(result_path, "/");
        strcat(result_path, path);
    } 
    else // if its already absolute
    {
        return path; 
    }

    return result_path;
}

char* create_Directory(const char *directory_path, const char *folder_Name) //this function is for .tar - if there is not storage directory, create one
 {
    int command_size = strlen("mkdir -p ") + strlen(directory_path) + (folder_Name ? strlen(folder_Name) : 0) + 2; // +2 for space and null character
    char *command_Create_Directory = (char *)malloc(command_size * sizeof(char));
    if (command_Create_Directory == NULL)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }
/* mkdir -p is a system call which will create directories, 
-p will create parent directories if needed. 
If folder already exists - nothing will happen
*/
    strcpy(command_Create_Directory, "mkdir -p "); 
    strcat(command_Create_Directory, directory_path);
    if (folder_Name)
    {
        strcat(command_Create_Directory, "/");
        strcat(command_Create_Directory, folder_Name);
    }

    int status = system(command_Create_Directory); //executing the command
    if (status == -1) 
    {
        printf("Error in making directory");
        return NULL;
    }

    free(command_Create_Directory);

   // now we the directory is already created, we have to return it to the user
    int result_size = strlen(directory_path) + (folder_Name ? strlen(folder_Name) + 1 : 0) + 1; // +1 for null character, +1 if folderName exists for '/'
    char *result = (char *)malloc(result_size * sizeof(char));
    if (result == NULL) 
    {
        printf("Memory allocation failed\n");
        return NULL;
    }

    strcpy(result, directory_path);
    if (folder_Name)
     {
        strcat(result, "/");
        strcat(result, folder_Name);
    }

    return result;
}

void remove_temp_directory( char *path) //this funtion is created to be used in .tar -> once the files are transfered to temporary directory - we will remove it
 {
    int command_size = strlen("rm -rf ") + strlen(path) + 1; //+1 for / if we want to add

    // Allocating memory for our command_remove
    char *command_remove = (char *)malloc(command_size * sizeof(char));

    // Check if memory allocation was successful
    if (command_remove == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

  //creating command
    strcpy(command_remove, "rm -rf "); //rm will remove, r will recursive and f will force the error messages. - rm is a  command-line utilit
    strcat(command_remove, path);

    int status = system(command_remove); //executing the command

    // Checking whether the command_remove executed successfully or not
    if (status != 0) 
    {
         printf("Failed to delete directory %s.\n", path);
         return;
    } 

    // Freeing up dynamically allocated memory
    free(command_remove);
}
void create_tar() 
{

   int command_size = strlen("tar -cf ") + strlen(temp_destination) + strlen(get_directory) + strlen("/a1.tar -C ") + strlen(" --transform 's/^.//' .") + 1;
    char* command_tar = (char*)malloc((command_size) * sizeof(char));
    if (command_tar == NULL) 
    {
        printf("Failed to allocate memory.\n");
        return; 
    }
    strcpy(command_tar, "tar -cf ");
    strcat(command_tar, get_directory);
    strcat(command_tar, "/a1.tar -C ");
    strcat(command_tar, temp_destination);
    strcat(command_tar," --transform 's/^.//' .");

    int result = system(command_tar);     // Executing the tar command
    if (result == 0) 
    {
        printf("Tar archive created successfully in location: %s.\n",get_directory);
    } 
    else 
    {
        printf("Error creating tar archive.\n");
    }

    
    free(command_tar); // Freeing up the allocated memory
    return;
}

void move_file(const char *source, const char *destination) //this function to move the files 
{
   if(rename(source,destination)==0) // it is a system call - rename will move the file and change the name of the file if its asked to do so
   {
       printf("The file was moved from %s to %s successfully\n",source,destination);
   }
   else
   {
       printf("Move Failed\n");
   }

}
void copy_file(const char *source, const char *destination_with_filename) //this function to copy the files 
{

    int source_fd = open(source, O_RDONLY); // open the source file in readonly mode
    if (source_fd == -1) // error in opening the source file.
    {
        printf("Error opening source file\n");
        return;
    }

    int destination_fd = open(destination_with_filename, O_CREAT | O_RDWR | O_TRUNC, 0744);// 7 - rwx permission to user, 4 - readonly to group, 4 -readonly to others
    if (destination_fd == -1 && get_extension==NULL)
    {
        printf("Error creating/opening destination file\n");
        return;
    }
    char reader; //reader will read characters from source file
    // now we will read source file - character by character and write to destination file.
    while (read(source_fd, &reader, 1) > 0)
    {
        write(destination_fd, &reader, 1);
    }
    //closing file descripters
    close(source_fd);
    close(destination_fd);
    if(get_extension==NULL) //dont print message if the function is called for .tar case
    {
         printf("The file copied from: %s to %s successfully...\n", source, destination_with_filename);
    }
    else //executing this case for .tar
    {
        printf("%s\n",source);
    }
   
}
char* get_extension_from_file(const char *filename) // to get extension from the file
{
    char *extension = strrchr(filename, '.'); //return null if could not find - strrchar will get the last ouccurence of . else return null
    return extension;
}

/*
This function searches files. 
This function will use all three conditions t- search, move/copy or if we want to make tar.
fpath - pointer to string that contains path of the file that is being searched. Eg.  fpath = "/home/user/file.txt";
sb - pointer to a struct that contains data about the file -  eg. size, permissions and timestamps.
type_flag - integer that will tell us what is the type of the file - whether it is file or symbolic links
ftwbuf - it is a pointer to a struct which is used by nftw function() to store the file tree, it contains depth of the traverals and base directories
*/
int search_file(const char *fpath, const struct stat *sb, int type_flag, struct FTW *ftwbuf)
{

    if (type_flag == FTW_F) //it is a regular file
    {
      const char *filename = basename((char *)fpath); //basename function will extract the filename from the given path
      char *destination_with_filename;
        if (get_extension!=NULL || strcmp(filename, get_file_name) == 0)
        {
            //abspath is to provide absolute path(relative to home directly only)
            char * temp_fpath = abs_path((char*)fpath); // converting constant pointer to char pointer then checking abs path
            if(get_extension==NULL ) // this means we dont want to print this message and turn flag=1 when we are executing search_file to create the .tar file.
            {
                 printf("File found: %s\n", temp_fpath);
                 found = 1;
            } 
            if (get_options!=NULL && strcmp(get_options, "-cp") == 0 && get_directory!=NULL) //case 5 triggerd - compare if that options contain -cp
            {
                if(check_directory_exists(get_directory)==-1) //check if the storage directory exists or not
                {
                    printf("storage directory is invalid\n");
                    exit(0);
                }
               destination_with_filename = get_full_destination_path(get_directory, filename); //get the full path of the file with filename
               copy_file(temp_fpath, destination_with_filename); // copy the file
            }
            else if (get_options!=NULL &&  strcmp(get_options, "-mv") == 0 && get_directory!=NULL) //case 5 triggerd -   //compare if that options contain -mv
            {
                if(check_directory_exists(get_directory)==-1) //check if the storage directory exists or not
                {
                    printf("storage directory is invalid\n");
                    exit(0);
                }
                  destination_with_filename = get_full_destination_path(get_directory, filename); //get the full path of the file with filename
                  move_file(temp_fpath, destination_with_filename);  // move the file
            }
            /*
            to make tar, the function will search the file get the files and copy them to a temporary directory called Temp_Folder
            Before copying, I have used access command to check whether the file exists or not - if it does not exists then only we 
            want to copy the file ( since we want first occurance of our search).
            Then I will call create_tar() function from main which will create .tar from all the files in Temp_Folder.
            Then I have called remove_temporary_directory function to remove Temp_Folder.
            */
            else if(get_extension!=NULL) // case 4 triggered
            {
                char * file_extension=NULL;
                destination_with_filename = get_full_destination_path(temp_destination, filename);  //get the full path of the file with filename
                file_extension =  get_extension_from_file(filename); //get the file extension
                if(file_extension!=NULL  && strcmp(file_extension, get_extension) == 0)
                {
                    if(access(destination_with_filename,F_OK)==-1) //this means file is not present
                    {
                        found=1; //turn the flag on since we only want to make the tar incase there is a single file
                        copy_file(temp_fpath, destination_with_filename);
                    }          
               
                }
            }
            if(get_extension==NULL) // if search is completed and we are not doing search for .tar case - stop the search as we want only the first occurance
            {
                exit(0);
            }
        } 
        
    }
    return 0;
}

void nftw_util() //utility function to call search_file using nftw
{
    found = 0; //reintilizing the found flag=0
    if (nftw(get_root, search_file, 10, FTW_PHYS) == 0)
    {
        if (found)
        {
            return;    // we can safely return as all the processing is completed in search_file function.
        }
        else if(get_extension==NULL) //print only when we are not running this for 4th case that is .tar file one
        {
            printf("Search Unsuccessful\n");
        }
    }
    else
    {
        printf("Issue in traversing the directories - NFTW failed..."); 
        return;
    }
}

int main(int no_of_arguments, char *arguments[])
{
    umask(0000); //directly give the permissions user asked for
    if (no_of_arguments == 3 || no_of_arguments == 4 || no_of_arguments == 5)
    {
        get_root = arguments[1]; //root directory
        get_root=abs_path(get_root); //get absolute path in relative to home directory
        if(check_directory_exists(get_root)==-1) //checking if root exists
        {
            printf("root directory is invalid \n");
            return 1;
        }
        switch (no_of_arguments)
        {
        case 3: // fileutil [ root_dir] filename
                // the first one is function name
            get_file_name = arguments[2]; //filename
            nftw_util();
            break;
        case 4: //fileutil [root_dir] [storage_dir] extension
            get_directory = arguments[2]; //storage directory
            get_directory=abs_path(get_directory); //get absolute path (relative to home directory) for storage direcotry
            get_extension=arguments[3]; //get extension from the argument provided in command line
            if(get_extension[0]!='.') //invaild extension
            {
                printf("Please provide only extension starting with . \n");
                exit(1); //1 means general failure
            }
            get_file_name=get_extension; //copying file name to get_file_name that will help us in searching the files with that extension
            temp_destination=create_Directory(get_directory,"Temp_Folder"); //creating Temp_Folder in storage directory 
            nftw_util(); //all files with that extension get copied to the directory
            if(found==1) //atleast one file is there so we can make the tar
            {
                create_tar();
                remove_temp_directory(temp_destination); //remove the Temp_Folder that is used to make the tar
            }
            else
            {
                printf(".tar file cannot be created because no file found for the given extension. \n"); // In case of any error 
            }
            break;
        case 5: // fileutil [root_dir] [storage_dir] [options] filename
            get_directory = arguments[2]; //storage directory
            get_options = arguments[3]; // options either -mv or -cp
            if(!check_options_invalid(get_options)) //check whether options are valid or not
            {
                printf("The given options are invalid. Choose from -mv or -cp.\n");
                exit(1); //1 means general failure
            }
            get_file_name = arguments[4]; //get the filename from the command line arguments
            nftw_util();
           if(check_directory_exists(get_directory)==-1 && found==0) //if file could not be found and directory doesnot exists 
            {
                printf("storage directory is invalid\n");
                exit(1);
            }
            break;
        default: //user have provied too few options or too much
            printf("No operations can be perfomed successfully...\n"); 
            break;
        }
    }
    else
    {
        printf("The no. of arguments are wrong. Chose from 3 to 5.\n"); 
        return 1;
    }

    return 0;
}