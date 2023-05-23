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

void executeShell(int timeout);

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
    int status;
    char minishell[] = "penn-sh> ";
    writeToStdout(minishell);

    command = getCommandFromInput();

    

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

/* Writes particular text to standard output */
void writeToStdout(char *text) {
    if (write(STDOUT_FILENO, text, strlen(text)) == -1) {
        perror("Error in write");
        exit(EXIT_FAILURE);
    }
    
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
