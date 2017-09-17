#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int cpid, status, pid;
void singleProcess (char** args){
    // create child
    cpid = fork();
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
        execvp(args[0], args);
    }
}