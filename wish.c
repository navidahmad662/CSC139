#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<ctype.h>

//for new commit for new
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
int preProcess(char *buffer){
        int stdout_copy=0;
        int rc;
        if(strstr(buffer,">")!=NULL){ //REDIRECT
                        int a=0;
                        
                        char* multiRedirect[sizeof(char)*512];
                        multiRedirect[0]= strtok(strdup(buffer)," \n\t>");
                        while(multiRedirect[a]!=NULL){
                                a++;
                                multiRedirect[a]=strtok(NULL," \n\t>");
                        }
                        if(a==1){ //no output file
                            write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE)); 
                            exit(0);    
                        }
                        int i=0;
                        char* myargs[sizeof(buffer)];
                        myargs[0]= strtok(buffer,"\n\t>");
                        while(myargs[i]!=NULL){
                                i++;
                                myargs[i]=strtok(NULL," \n\t>"); 
                        }
                        if(i>2){ //no output file
                            write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE)); 
                            exit(0);    
                        }
                        int x=0;
                        char* tokenize[sizeof(myargs[1])];
                        tokenize[0]= strtok(myargs[1]," \n\t");
                        while(tokenize[x]!=NULL){
                                x++;
                                tokenize[x]=strtok(NULL," \n\t"); 
                        }
                        
                        char *fout=strdup(tokenize[0]);
                        stdout_copy=dup(1);
                        int out=open(fout,O_WRONLY|O_CREAT|O_TRUNC,0666);
                        int error=open(fout,O_WRONLY|O_CREAT|O_TRUNC,0666);
                        fflush(stdout);
                        dup2(out,STDOUT_FILENO);
                        dup2(out,STDERR_FILENO);
                        close(out);
                        CLOSED=1;
                        if(out==-1 || error==-1 || x>1 || i>2){
                                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                                exit(0);
                        }
                        myargs[i+1]=NULL;
                        tokenize[x+1]=NULL;
                        strcpy(buffer,myargs[0]);
                        
                        
                }
                
                if(buffer[0] != '\0' && buffer[0] != '\n') {
                        char *command[sizeof(buffer)];
                        command[0] = strtok(buffer, " \t\n");
                        int p=0;
                        while(command[p]!=NULL){
                                p++;
                                command[p]=strtok(NULL, " \n\t");
                                
                        }
                        command[p+1]=NULL;
		        if(strcmp(command[0],"cd") == 0){//cd
                                if(p==2){
                                        if(chdir(command[1])!=0){
                                                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                                               
                                        }
                                 }
                                 else{ //0 Arguments or more than 2 arguments?
                                        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                                        
                                 }
                                                        
                        }  
                        else if(strcmp(command[0],"path") == 0){
                                pathChanged=1;
                                if(p==2){
                                        pathEmpty=0;
                                        path=strdup(command[1]);
                                         if(path[strlen(path)-1]!='/'){
                                                strcat(path,"/");
                                        }      
                                }
                                else if(p==1){
                                       
                                        pathEmpty=1;
                                }
                                else{ 
                                        pathEmpty=0;
                                        for(int i=1;i<p;i++){
                                                char *temp=strdup(command[i]);
                                                if(temp[strlen(temp)-1]!='/')
                                                        strcat(temp,"/");
                                                strcpy(multiPath[i-1],temp);
                                                numberMultiPath++;
                                        }
                                        
                                        //printf("%d\n",numberMultiPath);
                                        //for(int i=0;i<numberMultiPath;i++)
                                                //printf("%s\n",multiPath[i]);
                        
                                }
                                
                                       
			}
                        else if(strcmp(command[0],"exit") == 0) {
			    if(p==1){
                                        exit(0);
                                }
                                else{ //0 Arguments or more than 2 arguments?
                                        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                                        
                                }
                        }    
                        else{
                                if(pathEmpty==1)
                                        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                                else
                                        rc=newProcess(command);
                        }

                }
                if(CLOSED==1){
                        dup2(stdout_copy,1);
                        close(stdout_copy);
                        
                }
               return rc;
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
