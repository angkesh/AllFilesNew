#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_SIZE 1024

pid_t childPid = 0;

void executeShell(int timeout);

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

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
        alarm(0);
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
    char *command;
    int status;
    char minishell[] = "penn-shredder# ";
    writeToStdout(minishell);

    command = getCommandFromInput();
    alarm(timeout);

    if (command != NULL) {

        childPid = fork();

        if (childPid < 0) {
            free(command);
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }
        
        if (childPid == 0) {
            
            char *const envVariables[] = {NULL};
            char *const args[] = {command, NULL};

            //Ignore SIGCALLS for child process
            signal(SIGINT, SIG_IGN);
            if (execve(command, args, envVariables) == -1) {
                free(command);
                perror("Error in execve");
                exit(EXIT_FAILURE);
            }
            
        } else {
            do {
               
                if (wait(&status) == -1) {
                    free(command);
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
                
                childPid = 0;
                
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    free(command);
}

/* Writes particular text to standard output */
void writeToStdout(char *text) {
    if (write(STDOUT_FILENO, text, strlen(text)) == -1) {
        perror("Error in write");
        exit(EXIT_FAILURE);
    }
    
}

/* Reads input from standard input till it reaches a new line character.
 * Checks if EOF (Ctrl + D) is being read and exits penn-shredder if that is the case
 * Otherwise, it checks for a valid input and adds the characters to an input buffer.
 *
 * From this input buffer, the first 1023 characters (if more than 1023) or the whole
 * buffer are assigned to command and returned. An \0 is appended to the command so
 * that it is null terminated */
char *getCommandFromInput() {

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
            char* command = (char*) calloc(INPUT_SIZE, sizeof(char));

            for (int i = 0; i < INPUT_SIZE; i++){
                if(userInput[i] == '\n'){
                    break;
                }
                else if(userInput[i] == ' '){
                    continue;
                }
                command[commandLen] = userInput[i];
                commandLen++;
            }

            if (commandLen == 0){
                free(command);
                command = NULL;
            }
            else{
                command[commandLen+1] = '\0';
            }
 
            free(userInput);
            return command; 
        }
       
    }
        
}
