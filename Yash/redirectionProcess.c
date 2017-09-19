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

//signal
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

void redirect(char** args, char* fileTo, char* fileFrom, char* fileErr, int to, int from, int error){
    // create child
    cpid = fork();
    //open file
    if(to){
        if((out = open(fileTo, O_TRUNC | O_CREAT | O_WRONLY, 0666)) < 0){
        //Send Error
        }
    }
    if(from){
        if((in = open(fileFrom, O_RDONLY)) < 0){
            //send error
        }
    }
    if(error){
        if((err = open(fileErr, O_TRUNC | O_CREAT | O_WRONLY, 0666)) < 0){
            //send error
        }
    }
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
                //Stop process and send to bg
                printf("%d stopped by signal %d\n", pid,WSTOPSIG(status));
                printf("Sending CONT to %d\n", pid);
                sleep(4); //sleep for 4 seconds before sending CONT
                kill(pid,SIGCONT);
            } else if (WIFCONTINUED(status)) {
                //send process to fg
                printf("Continuing %d\n",pid);
            }
        }
        return;
    } else {
        //Child
        if(to){dup2(out, STDOUT_FILENO);}
        if(from){dup2(in,STDIN_FILENO);}
        if(error){dup2(err,STDERR_FILENO);}
        execvp(args[0], args);
    }
}

