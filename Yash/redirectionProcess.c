#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//declare variables
int status, cpid, pid;

int out, in, err;

void redirectToFile(char** args, char* file){
    
    // create child
    cpid = fork();
    //open file
    if((out = open(file, O_TRUNC | O_CREAT | O_WRONLY, 0666)) < 0){
        //send error
    }
    int count = 0;
    if (cpid > 0){
        //Parent
        while (count < 1){
            pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
            if (pid == -1) {
                return;
                  //perror("waitpid");
                  //exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                count++;
            } else if (WIFSIGNALED(status)) {
                count++;
            }
        }
    } else {
        //Child
        dup2(out, STDOUT_FILENO);
        execvp(args[0], args);
    }
}

void redirectToProcess(char** args, char* file){
     // create child
     cpid = fork();
     //open file
     if((in = open(file, O_RDONLY)) < 0){
         //send error
     }
     int count = 0;
     if (cpid > 0){
         //Parent
         while (count < 1){
             pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
             if (pid == -1) {
                 return;
                   //perror("waitpid");
                   //exit(EXIT_FAILURE);
             }
             if (WIFEXITED(status)) {
                 count++;
             } else if (WIFSIGNALED(status)) {
                 count++;
             }
         }
     } else {
         //Child
         dup2(in, STDIN_FILENO);
         execvp(args[0], args);
     }
}
void redirectError(char**args, char* file){
     // create child
     cpid = fork();
     //open file
     if((err = open(file, O_TRUNC | O_CREAT | O_WRONLY, 0666)) < 0){
         //send error
     }
     int count = 0;
     if (cpid > 0){
         //Parent
         while (count < 1){
             pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
             if (pid == -1) {
                 return;
                   //perror("waitpid");
                   //exit(EXIT_FAILURE);
             }
             if (WIFEXITED(status)) {
                 count++;
             } else if (WIFSIGNALED(status)) {
                 count++;
             }
         }
     } else {
         //Child
         dup2(err, STDERR_FILENO);
         execvp(args[0], args);
     }
}