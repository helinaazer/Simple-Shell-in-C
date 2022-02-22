/*
* CSS 430
* Project 1: UNIX Shell
* This program is a shell interface that accepts user commands and then executes each command in a separate process.
* It supports input and output redirection as well as pipes
* This program uses the UNIX fork(), exec(), wait(), dup2(), and pipe() system calls.
* Some of the basic commands this shell supports include: 
* 'ls', 'ps', 'pwd', 'cat', '&', '!!', '<', '>'. '|', 'clear', 'exit'
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80 //80 characters per command which is the maximum length of the command.

//Function that takes in user input and breaks it apart to find the command before the ampersand sign (&)
void amp(char *in, int *wait) {
    char *a, *z;
    *wait = 0;
    a = in;
    z = in;
    while (*a != '\0') {
        *a = *z;
        if (*z != '&') z++;
        else *wait = 0;
        *a++;
    }
    *z = '\0';
}

//Function that takes in user input and breaks it apart to find the command before the redirection (<, >)
void redirection(char *in, int *par) {
    char *a, *z;
    *par = 0;
    a = in;
    z = in;
    while (*a != '\0') {
        *a = *z;
        if (*z != '>' && *z != '<' && *par == 0) z++;
        else *par = 1;
        a++;
    }
    *z = '\0';
}

//Function that takes in user input and redirects output to a file or redirect output from a file
int redirectOutput(char* args[], int *filedescriptor) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            *filedescriptor = open(args[i + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(*filedescriptor, 1);
            close(*filedescriptor);
        } else if (strcmp(args[i], "<") == 0) {
            *filedescriptor = open(args[i + 1], O_RDONLY);
            dup2(*filedescriptor, 0);
            close (*filedescriptor);
        }
        i++;
    }
}

//Function that replaces whitespace with zeros until there is no whitespace detected. 
//This marks the beginning of the argument and goes until there is whitespace again and saves it into args.
char parsecommand(char *in, char* args[]) { 
    char *co = (char*) malloc(MAX_LINE);
    memcpy(co, in, MAX_LINE);
    while (*co != '\0') {
        while (*co == ' ' || *co == '\n') {
            *co = '\0';
            *co++;
        }
        if (*co) {
            *args = co;
            *args++;
        }
        while (*co != '\0' && *co != ' ' && *co != '\n') {
            co++;
        }
    }
    *args = '\0';
}

//Function that executes the shell commands
int executeCommands(char *in, char* args[], int wfc) {
    pid_t pid = fork();
    int status;
    
    //child process
    if (pid == 0) {
        if (wfc == 1) {
            setpgid(0,0);
        }
        //pipe command (|)
        if (strchr(in, '|') != NULL) {
            char *input[MAX_LINE / 2+1], *output[MAX_LINE / 2+1], *pipeinput = strdup(in), *pipeoutput;
            int p[2], pid2; 
            pipeinput = strtok(in, "|");
            pipeoutput = strtok(NULL, "|");
            //check if pipe worked
            if (pipe(p) < 0) {
                perror("Pipe failed.\n");
                exit(0);
            }
            pid2 = fork(); //fork child process
            //pipe input
            if (pid2 == 0) {
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                parsecommand(pipeinput, input);
                status = execvp(input[0], input);
                if (status < 0) {
                    perror("Pipe input command not valid.\n");
                    exit(0);
                }
            }
            pid2 = fork();
            //pipe output
            if (pid2 == 0) {
                close(0);
                dup(p[0]);
                close(p[1]);
                close(p[0]);
                parsecommand(pipeoutput, output);
                status = execvp(output[0], output);
                if (status < 0) {
                    perror("Pipe output command not valid.\n");
                    exit(0);
                }
            }
            close(p[0]);
            close(p[1]);
            wait(NULL);
            wait(NULL);
        //check if command entered by user is valid
        } else {
            fflush(stdout);
            status = execvp(args[0], args);
            if (status < 0) {
                printf("Invalid Command.\n");
                exit(0);
            }
        }
    //parent process
    } else if (pid > 0) {
        int *pidptr;
        if (wfc == 0) waitpid(pid, pidptr, 0);
    //if pid < 0 which means there is an error
    } else {
        perror("Forking Child Process Failed.\n");
        exit(1);
    }
    return 0;
}

int main(void) {
    char *args[MAX_LINE/ 2+1], *argHistory[MAX_LINE/ 2+1], *history = NULL, *in;
    int should_run = 1, wfc = 0, par, fd;
            
    while (should_run){
        int input = dup(0), output = dup(1);
        fflush(stdout);
        fflush(stdin);
        printf("osh> ");
        char *inputarg = NULL;
        ssize_t buffer = 0;
        getline(&inputarg, &buffer, stdin);
        in = inputarg;
      
        //history feature using !!
        if (strcmp(in, "!!\n") == 0) {
            if (history == NULL){
                printf("No commands in history.\n");
                continue;
            } else {
                printf("%s", history);
                in = history;
            }
        } else {
            if (sizeof(args) > 0 && sizeof(in) > 0) {
                history = (char*) malloc(MAX_LINE);
                memcpy(argHistory, args, MAX_LINE);
                memcpy(history, in, MAX_LINE);
            }
        }
        //ampersand
        if (strchr(in,'&') != NULL) amp(in, &wfc);
        else wfc = 0;
        //exit command      
        if (strcmp(in, "exit\n") == 0) {
            should_run = 0;
            kill(0, SIGINT);
            exit(0);
        }
        //redirect input and output
        if (strchr(in, '>') != NULL || strchr(in, '<') != NULL) {
            parsecommand(in, args);          
            redirectOutput(args, &fd);
            redirection(in, &par);
            parsecommand(in, args);
            executeCommands(in, args, wfc);
        } else {
            parsecommand(in, args);
            executeCommands(in, args, wfc);
        }
        
        fflush(stdout);
        fflush(stdin);
        dup2(output, 1);
        dup2(input, 0);
        close(output);
        close(input);
    }
    return 0;
}



