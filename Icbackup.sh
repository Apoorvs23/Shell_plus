#!/bin/bash

# Seting up the directory where our backups will be stored
backupDirectory="$HOME/backup" # this means it will be stored at /home/apoorvs23/backup

path_to_root="$HOME" # It says from where we need to start our research

no_of_complete_backups=0 # Total no. of complete backups we have done as of now
no_of_incremental_backups=0 # Total no. of incremental backups we have done as of now
no_of_differential_backups=0 # Total no. of differential backups we have done as of now

# Initializing variables to track the last backup times
last_backup=$(date +"%Y-%m-%d %H:%M:%S") # this will help us to track all the backup changes
last_complete_backup=$(date +"%Y-%m-%d %H:%M:%S") # this will only be helpful in complete backup

interval="120" # Seting up the interval between backups - it means 120 seconds or 2 minutes

# Function to get the customized path for a file within the backup directory
getCustomizedPath() {  # this function will give path eg. home/apoorvs/backup/log.txt
    local file="$1" # get name of the first argument
    echo "$backupDirectory/$file" #create the path
}


create_backup_directory() {  # This function will create a backup directory if it doesn't exist
    local getPath="$1"
    mkdir -p "$getPath"  # -p will create all the required directorries and sub-directories
}

# Function to get the formatted date
getFormattedDate() {
    local format="%a %d %B %Y %I:%M:%S %p %Z"  # Function to get the formatted date
    echo $(date +"$format")  # return date in format like Wed 20 Mar 2024 06:16:08 PM EDT
}

# Function to log messages with timestamps
Logger() {
    local entry="$1"  # the message we want to print 
    local log_file=$(getCustomizedPath "backup.log") # path of the log file
    local date_formatted=$(getFormattedDate)    # return date in format like Wed 20 Mar 2024 06:16:08 PM EDT
    printf "%s %s\n\n" "$date_formatted" "$entry" >> "$log_file" # write the msg to the log file
}

# Function to perform a complete backup
complete_backup() {

    local backup_prefix="cbw24"  # prefix for complete backup
    local complete_path="$backupDirectory/$backup_prefix" # it will give /home/apoorvs/backup/cbw24

    # Check if at least one file exists in the root directory
    if ! find "$path_to_root" -mindepth 1 -type f -not -path '*/.*' -print -quit | grep -q .; then # it means atleast one file should exists in the directory - mindepth 1 means start from root and once you find single file quit
        Logger "No files - Complete Backup cannot be created..."
        return
    fi
    # Creating backup directory if it doesn't exist
    mkdir -p "$complete_path"  # -p will create all the required directorries and sub-directories
    ((no_of_complete_backups++))  # since we will do a new complete backup - increase the number by 1
    local tar_name="$complete_path/cbw24-$no_of_complete_backups.tar"  # it will build a complete path for tar - home\apoorvs23\backup\cbw24\cbw24-1.tar
    # Using find command to find all the regular files in the give home directory and we will execlude backup directory and then 
    #-print0 generate a list of file paths separated by null characters (\0).
    find "$path_to_root" -type f ! -path "$backupDirectory/*" ! -path '*/.*' -print0 |
    # Pipe the output to xargs and use tar to create a compressed tar
    # xargs will execute tar commands and will convert all the files that we received from find command. -0 means it will get something that is null terminated 
    # czf - c - new archive, z - it should compress archive using gzip, f - name of tar ($tar_name),
    #--transform "s|^$path_to_root|| will remove $path_to_root prefix from each file path
    # The output will be pushed to dev/null means it wont show any output on the screen
    xargs -0 tar -czf "$tar_name" --transform "s|^$path_to_root||" -C "$path_to_root" &> /dev/null  
    last_backup=$(getFormattedDate)  # it will get updated on every backup
    last_complete_backup=$(getFormattedDate) # only get updated on the complete backup
    local message="cbw24-$no_of_complete_backups.tar was created" # create a local messge cbw24-1.tar was created and pass it to logger function
    Logger "$message"
}

# Function to perform an incremental backup
incremental_backup() {
    local backup_prefix="ibw24" # prefix for incremental backup
    local incremental_path="$backupDirectory/$backup_prefix" # it will give /home/apoorvs/backup/ibw24

    # Find files that modified or created since the last complete backup, newermt will check the date of last_backup, if any file is there thats modified after that it will pick it
    local files_modified=$(find "$path_to_root" -type f ! -path "$backupDirectory/*" ! -path '*/.*' -newermt "$last_backup")

    if [ -n "$files_modified" ]; then # if $files_modfied is not empty
        mkdir -p "$incremental_path"  # -p will create all the required directorries and sub-directories
        ((no_of_incremental_backups++))  # since we will do a new incremental backup - increase the number by 1
        local tar_name="$incremental_path/ibw24-$no_of_incremental_backups.tar"
        tar -czf "$tar_name" $files_modified &> /dev/null  #create the tar and push the output to devnull
        last_backup=$(getFormattedDate) # update the backup date 
        local message="ibw24-$no_of_incremental_backups.tar was created" # ibw24-1.tar was created
        Logger "$message" # log the message with time stamp
    else
        local message="No changes - Incremental backup was not created" #log this message if there are no changes
        Logger "$message"
    fi
}

# Function to perform a differential backup
differential_backup() {
    local backup_prefix="dbw24"  # prefix for differential backup
    local differentialPath="$backupDirectory/$backup_prefix"  # it will give /home/apoorvs/backup/dbw24

    # Find files modified or created since the last complete backup, newermt will check the date of last_complete_backup, if any file is there thats modified after that it will pick it
    local files_modified=$(find "$path_to_root" -type f ! -path "$backupDirectory/*" ! -path '*/.*' -newermt "$last_complete_backup")

    if [ -n "$files_modified" ]; then  # if $files_modfied is not empty
        mkdir -p "$differentialPath" # -p will create all the required directorries and sub-directories
        ((no_of_differential_backups++)) # since we will do a new differential backup - increase the number by 1
        local tar_name="$differentialPath/dbw24-$no_of_differential_backups.tar"
        tar -czf "$tar_name" $files_modified &> /dev/null #create the tar and push the output to devnull
        last_backup=$(getFormattedDate) # update the backup date 
        local message="dbw24-$no_of_differential_backups.tar was created" # dbw24-1.tar was created
        Logger "$message" # log the message with time stamp
    else
        local message="No changes - Differential backup was not created"
        Logger "$message"
    fi
}

# Infinite loop to run backup processes in the background
for (( ; ; )); do
    complete_backup
    sleep $interval
    incremental_backup 
    sleep $interval
    incremental_backup
    sleep $interval
    differential_backup
    sleep $interval
    incremental_backup
    sleep $interval
done