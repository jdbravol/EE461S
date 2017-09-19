#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int cpid, status, pid;

static void sig_handler(int signo){
    switch(signo){
        case SIGINT:
            //CTRL+C kill fg job
            kill(cpid,SIGINT);
            break;
        case SIGTSTP:
            //CTRL+Z Stop fg job
            kill(cpid,SIGTSTP);        
            break;
    }
}

void singleProcess (char** args){
    // create child
    cpid = fork();
    int count = 0;
    if (cpid > 0){
        //Parent
        while (count < 1){
            if (signal(SIGINT, sig_handler) == SIG_ERR) printf("signal(SIGINT) error");
            if (signal(SIGTSTP, sig_handler) == SIG_ERR) printf("signal(SIGTSTP) error");
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
            if (WIFSTOPPED(status)) {
                printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
                printf("Sending CONT to %d\n", pid);
                sleep(4); //sleep for 4 seconds before sending CONT
                kill(pid,SIGCONT);
          } else if (WIFCONTINUED(status)) {
                printf("Continuing %d\n",pid);
          }
        }
        return;
    } else {
        //Child
        execvp(args[0], args);
    }
}