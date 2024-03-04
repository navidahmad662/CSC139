#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<ctype.h>

#define BSIZE 512
#define ERROR_MESSAGE "An error has occurred\n"

int batch = 0;
int pathChanged = 0;
char *path;
int CLOSED = 0;
int pathEmpty = 0;
char multiPath[512][512];
int numberMultiPath = 0;

// Function to check if the buffer contains only spaces
int checkOnlySpace(char* buffer) {
    for(int i = 0; i < strlen(buffer); i++) {
        if(!isspace(buffer[i])) {
            return 1; // Not all spaces
        }
    }
    return 0; // All spaces
}       

// Function to print error message and exit
void printError() {
    write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
    exit(1);  
}

// Function to print the shell prompt
void printPrompt() {
    write(STDOUT_FILENO, "wish> ", strlen("wish> "));                
}

// Function to create a new process for executing commands
int newProcess(char *myargs[]) {
     int rc = fork();
    if (rc < 0) { // Fork Error
        perror("Fork Error");
        exit(EXIT_FAILURE);
    } else if (rc == 0) { // Child process
        char* path = NULL;
        // Check if the path is provided and not empty
        if (pathChanged == 0) {
            path = "/bin/";
        } else {
            path = strdup(path);
        }
        // Try to execute the command in the default or custom path
        if (path != NULL) {
            path = strcat(path, myargs[0]);
            if (access(path, X_OK) != 0) {
                // Command not found in the path
                if (pathChanged == 0) {
                    // Try /usr/bin/ path
                    path = "/usr/bin/";
                    path = strcat(path, myargs[0]);
                    if (access(path, X_OK) != 0) {
                        // Command not found in /usr/bin/ path
                        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // Custom path specified but command not found
                    write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                    exit(EXIT_FAILURE);
                }
            }
        }
        // Execute the command
        if (execv(path, myargs) == -1) {
            // Execution failed
            perror("Execution Error");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int returnStatus;
        waitpid(rc, &returnStatus, 0);
    }
    return rc;
}

// Function to preprocess the input command
int preProcess(char *buffer) {
    // Implementation for preprocessing input command
}

int main(int argc, char* argv[]) {
    FILE *file = NULL;
    path = (char*) malloc(BSIZE);
    char buffer[BSIZE];
        
    // Check the number of arguments to determine the mode
    if(argc == 1) { // Interactive mode
        file = stdin;
        printPrompt();
    } else if(argc == 2) { // Batch mode
        char *bFile = strdup(argv[1]);
        file = fopen(bFile, "r");
        if (file == NULL) {
            printError();
        }
        batch = 1;
    } else {
        printError();
    }

    while(fgets(buffer, BSIZE, file)) {
        CLOSED = 0;
        // Skip if the buffer contains only spaces
        if(checkOnlySpace(buffer) == 0) {
            continue;
        }
        // Check for concurrency
        if(strstr(buffer, "&") != NULL) {
            int j = 0;
            char *myargs[sizeof(buffer)];
            myargs[0] = strtok(buffer, "\n\t&");
            while(myargs[j] != NULL) {
                j++;
                myargs[j] = strtok(NULL, "\n\t&");
            }
            myargs[j+1] = NULL;
            int pid[j];
            // Preprocess and execute each command in parallel
            for(int i = 0; i < j; i++) {
                pid[i] = preProcess(myargs[i]);
            }
            // Wait for all child processes to finish
            for(int x = 0; x < j; x++) {
                int returnStatus = 0;
                waitpid(pid[x], &returnStatus, 0);                        
                if (returnStatus == 1) {
                    printError();    
                }
            }
        } else {
            preProcess(buffer);
        }
        // Print prompt in interactive mode
        if(argc == 1) {
            printPrompt();
        }
    }
}
