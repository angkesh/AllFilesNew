#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tokenizer.h"

#define INPUT_SIZE 1024

pid_t childPid = 0;

pid_t childPid1 = 0;

pid_t childPid2 = 0;

void executeShell(int timeout);

void redirectionOnlyProcess(char** command);

int isPipe(char** command);

char** createArrayOfTokensBeforePipe(char** command);

char** createArrayOfTokensAfterPipe(char** command);

void pipeProcess(char** commandArray1, char** commandArray2, int* fd);

char** redirectionsPipeWriterProcess(char** commandArray2);

char** redirectionsPipeReaderProcess(char** commandArray2);

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char **getCommandFromInput();

void freeCommand(char** command);

void registerSignalHandlers();

void killChildProcess();

int main(int argc, char **argv) {
    registerSignalHandlers();

    int timeout = 0;
    if (argc == 2) {
        timeout = atoi(argv[1]);
    }

    if (timeout < 0) {
        writeToStdout("Invalid input detected. Ignoring timeout value.\n");
        timeout = 0;
    }

    while (1) {
        executeShell(timeout);
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    if (kill(childPid, SIGKILL) == -1) {
        perror("Error in kill");
        exit(EXIT_FAILURE);
    }
}

/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig) {
    if (sig == SIGALRM){
        if (childPid != 0){
            killChildProcess();
            writeToStdout("Bwahaha ... tonight I dine on turtle soup\n");
        }
    }
}

/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig) {
    if (childPid != 0) {      
        killChildProcess();
    }
}


/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers() {
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("Error in signal");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGALRM, alarmHandler) == SIG_ERR){
        perror("Error in alarm signal");
        exit(EXIT_FAILURE);
    }
}

/* Prints the shell prompt and waits for input from user.
 * Takes timeout as an argument and starts an alarm of that timeout period
 * if there is a valid command. It then creates a child process which
 * executes the command with its arguments.
 *
 * The parent process waits for the child. On unsuccessful completion,
 * it exits the shell. */
void executeShell(int timeout) {
    char **command;
    char minishell[] = "penn-sh> ";
    writeToStdout(minishell);

    command = getCommandFromInput();
    
    if (isPipe(command) == 0){
        redirectionOnlyProcess(command);
    }else{

        char** commandArray1 = createArrayOfTokensBeforePipe(command);
        char** commandArray2 = createArrayOfTokensAfterPipe(command);
        int fd[2];

        if (pipe(fd) < 0) {
            freeCommand(commandArray1);
            freeCommand(commandArray2);
            freeCommand(command);
            perror("Error in piping");
            exit(EXIT_FAILURE);
        }

        pipeProcess(commandArray1, commandArray2, fd);
        freeCommand(command);
    }
 
}

/*Executes the input if there is a pipe process*/
void pipeProcess(char** commandArray1, char** commandArray2, int* fd){
    int status;
    if (commandArray1[0] != NULL){

        //child1 process
        childPid1 = fork();
        if(childPid1 < 0){
            freeCommand(commandArray1);
            freeCommand(commandArray2);
            perror("Error in creating child process.");
            exit(EXIT_FAILURE);
        }

        if (childPid1 == 0){
            printf("\nchild1\n");
            char** args1 = (char**) calloc(INPUT_SIZE, sizeof(char));
            args1 = redirectionsPipeWriterProcess(commandArray1);
            close(fd[0]); //close the read end
            dup2(fd[1], STDOUT_FILENO);
            
            //Ignore SIGCALLS for child process
            signal(SIGINT, SIG_IGN);

            if (execvp(args1[0], args1) == -1) {
                freeCommand(commandArray1);
                freeCommand(commandArray2);
                freeCommand(args1);
                perror("Error in execvp of child1 process");
                exit(EXIT_FAILURE);
            }
            freeCommand(args1);
        }
        
    }

    if (commandArray2[0] != NULL){
        //child2 process
        childPid2 = fork();
        if(childPid2 < 0){
            freeCommand(commandArray1);
            freeCommand(commandArray2);
            perror("Error in creating child process.");
            exit(EXIT_FAILURE);
        }

        if (childPid2 == 0){
            char** args2 = (char**) calloc(INPUT_SIZE, sizeof(char));
            args2 = redirectionsPipeReaderProcess(commandArray2);
            close(fd[1]); //close the write end
            dup2(fd[0], STDIN_FILENO);
            printf("\nchild2\n");
            //Ignore SIGCALLS for child process
            signal(SIGINT, SIG_IGN);

            if (execvp(args2[0], args2) == -1) {
                freeCommand(commandArray1);
                freeCommand(commandArray2);
                freeCommand(args2);
                perror("Error in execvp of child2 process");
                exit(EXIT_FAILURE);
            }
            freeCommand(args2);
        }
        
    }

    
    
    /**Parent Process Below*/
    close(fd[0]); //close the read process of the parent
    close(fd[1]); //close the write process of the parent

    pid_t pid[] = {childPid1, childPid2};
        
    int i = 0;
   
    while (i < 2){
       
        do {
            
            if (waitpid(-1, &status, 0) == -1) {
                freeCommand(commandArray1);
                freeCommand(commandArray2);
                perror("Error in child process termination");
                exit(EXIT_FAILURE);
            }
                
            pid[i] = 0;
            printf("\nParent\n");

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        i++;
    }
    
    free(commandArray1);
    free(commandArray2);
}







//redirections writer process
char** redirectionsPipeWriterProcess(char** commandArray1){

    char** args1 = (char**) calloc(INPUT_SIZE, sizeof(char));
    int i = 0;
    int m = 0;
    int inputRedirectCount = 0;
    char *tok;
    int new_stdIn;

    while((tok = commandArray1[m]) != NULL){

        if(strcmp(tok, ">") == 0){
            freeCommand(commandArray1);
            freeCommand(args1);
            perror("Invalid: Multiple standart out redirects.");
            exit(EXIT_FAILURE);
        }
        else if(strcmp(tok, "<") == 0){
                    
            inputRedirectCount++;

            if(inputRedirectCount > 1){
                        
                perror("Invalid: Multiple standard input redirects or redirect in invalid location");
                freeCommand(commandArray1);
                freeCommand(args1);
                exit(EXIT_FAILURE);
            }
            else{
                if((new_stdIn = open(commandArray1[++m], O_RDONLY)) != -1){

                    dup2(new_stdIn , STDIN_FILENO);

                }else{
                            
                    perror("Invalid standard input redirect: No such file or directory");
                    freeCommand(commandArray1);
                    freeCommand(args1);
                    exit(EXIT_FAILURE);
                }
                        
            }

        }
        else{
            args1[i] = tok;
            i++;
        }
        m++;
    }

    return args1;

}






//Redirections Reader Process
char** redirectionsPipeReaderProcess(char** commandArray2){

    char** args2 = (char**) calloc(INPUT_SIZE, sizeof(char));
    int i = 0;
    int m = 0;
    int outputRedirectCount = 0;
    char *tok;
    int new_stdout;

    while((tok = commandArray2[m]) != NULL){

        if(strcmp(tok, ">") == 0){
            outputRedirectCount++;

            if(outputRedirectCount > 1){
                        
                perror("Invalid: Multiple standard output redirects.");

                freeCommand(commandArray2);
                freeCommand(args2);

                exit(EXIT_FAILURE);
            }
            else{
                if((new_stdout = open(commandArray2[++m], O_WRONLY | O_TRUNC | O_CREAT, 0644)) != -1){
                    dup2(new_stdout, STDOUT_FILENO);
                }else{
                            
                    perror("Invalid: Error in openning a file.");
                    freeCommand(commandArray2);
                    freeCommand(args2);
                    exit(EXIT_FAILURE);
                }
                        
            }
        }
        else if(strcmp(tok, "<") == 0){
                    
            freeCommand(commandArray2);
            freeCommand(args2);
            perror("Invalid: Multiple standard input redirects.");
            exit(EXIT_FAILURE);

        }
        else{
            args2[i] = tok;
            i++;
        }
        m++;
    }

    return args2;

}






/* Writes particular text to standard output */
void writeToStdout(char *text) {
    if (write(STDOUT_FILENO, text, strlen(text)) == -1) {
        perror("Error in write");
        exit(EXIT_FAILURE);
    }
    
}


/*Create array of tokens before pipe*/
char** createArrayOfTokensBeforePipe(char** command){
    char** commandArray1 = (char**) calloc(INPUT_SIZE, sizeof(char));
    char* tok;
    int m = 0;
    int i = 0;
    while(strcmp((tok = command[m]), "|") != 0){
        commandArray1[i] = tok;
        m++;
        i++;
    }
    return commandArray1;
}

/*Create array of tokens after pipe*/
char** createArrayOfTokensAfterPipe(char** command){
    char** commandArray2 = (char**) calloc(INPUT_SIZE, sizeof(char));
    char* tok;
    int m = 0;
    int i = 0;
    int pipe = 0;
    while((tok = command[m]) != NULL){
        if (strcmp(tok, "|") == 0){
            pipe = 1;
        }
        else if(pipe == 0){
            m++;
            continue;
        }
        else{
            commandArray2[i] = tok;
            i++;
        }
        m++;
    }
    return commandArray2;
}



/*Executes redirections without pipes (project2a)*/
void redirectionOnlyProcess(char** command){
    int status;
    if (command[0] != NULL) {

    childPid = fork();

    if (childPid < 0) {
        free(command);
        perror("Error in creating child process.");
        exit(EXIT_FAILURE);
    }
        
    if (childPid == 0) {
            
            
        char** argv = (char**) calloc(INPUT_SIZE, sizeof(char));
        int i = 0;
        int m = 0;
        int inputRedirectCount = 0;
        int outputRedirectCount = 0;
        char *tok;
        int new_stdout;
        int new_stdIn;

        while((tok = command[m]) != NULL){

            if(strcmp(tok, ">") == 0){

                outputRedirectCount++;

                if(outputRedirectCount > 1){
                        
                    perror("Invalid: Multiple standard output redirects.");

                    freeCommand(command);

                    exit(EXIT_FAILURE);
                }
                else{
                    if((new_stdout = open(command[++m], O_WRONLY | O_TRUNC | O_CREAT, 0644)) != -1){
                        dup2(new_stdout, STDOUT_FILENO);
                    }else{
                            
                        perror("Invalid: Error in openning a file.");
                        freeCommand(command);
                        exit(EXIT_FAILURE);
                    }
                        
                }

            }
            else if(strcmp(tok, "<") == 0){
                    
                inputRedirectCount++;

                if(inputRedirectCount > 1){
                        
                    perror("Invalid: Multiple standard input redirects or redirect in invalid location");
                    freeCommand(command);
                    exit(EXIT_FAILURE);
                }
                else{
                    if((new_stdIn = open(command[++m], O_RDONLY)) != -1){

                        dup2(new_stdIn , STDIN_FILENO);

                    }else{
                            
                        perror("Invalid standard input redirect: No such file or directory");
                        freeCommand(command);
                        exit(EXIT_FAILURE);
                    }
                        
                }

            }
            else{
                argv[i] = tok;
                i++;
            }
            m++;
        }



        //Ignore SIGCALLS for child process
        signal(SIGINT, SIG_IGN);

        if (execvp(command[0], argv) == -1) {
            freeCommand(command);
            perror("Error in execvp");
            exit(EXIT_FAILURE);
        }
            
            
        } else {
            do {
               
                if (wait(&status) == -1) {
                    freeCommand(command);
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
                
                childPid = 0;

            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    freeCommand(command);
}













int isPipe(char** command){
    char* tok;
    int m = 0;
    while((tok = command[m]) != NULL){
        if (strcmp(tok, "|") == 0){
            return 1;
        }
        m++;
    }
    return 0;
}

void freeCommand(char** command){

    int m = 0;
    while(command[m] != NULL){
        free(command[m]);
        m++;
    }
    free(command);
}
/* Reads input from standard input till it reaches a new line character.
 * Checks if EOF (Ctrl + D) is being read and exits penn-shredder if that is the case
 * Otherwise, it checks for a valid input and adds the characters to an input buffer.
 *
 * From this input buffer, the first 1023 characters (if more than 1023) or the whole
 * buffer are assigned to command and returned. An \0 is appended to the command so
 * that it is null terminated */
char **getCommandFromInput() {
    
    //userInput will hold the input of the user
    char* userInput = (char*) calloc(INPUT_SIZE, sizeof(char));;

    //read in the user input
    int returnValue = read(STDIN_FILENO, userInput, INPUT_SIZE-1);

    //commandLen will count the number of characters in command
    int commandLen = 0;

    if (returnValue < 0){
        free(userInput);
        exit(2);

    }else{

        if ((returnValue == 0 && strlen(userInput) == 0)){
            free(userInput);
            exit(2);
          
        }else{
            //command will be pure input of the user and returning value of this function 
            char** command = (char**) calloc(INPUT_SIZE, sizeof(char));

            char *tok;

            TOKENIZER *tokenizer;

            tokenizer = init_tokenizer(userInput);

           
            
            while((tok = get_next_token( tokenizer )) != NULL){
                
                command[commandLen] = tok;
                commandLen++;
            }
            
            free(userInput);
            free_tokenizer(tokenizer);
            return command; 
        }
       
    }
        
}
