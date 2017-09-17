#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipefd[2];
int status, pid_ch1, pid_ch2, pid;

static void sig_int(int signo) {
    printf("Sending signals to group:%d\n",pid_ch1); // group id is pid of first in pipeline
    kill(-pid_ch1,SIGINT);
}
static void sig_tstp(int signo) {
    printf("Sending SIGTSTP to group:%d\n",pid_ch1); // group id is pid of first in pipeline
    kill(-pid_ch1,SIGTSTP);
}

void pipeProcess(char** arg1, char** arg2) {
	
	char ch[1]={0};
	  
	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(-1);
	}
	
	pid_ch1 = fork();
	if (pid_ch1 > 0){
		// Parent
		pid_ch2 = fork();
		if (pid_ch2 > 0){
		  	if (signal(SIGINT, sig_int) == SIG_ERR) printf("signal(SIGINT) error");
		  	if (signal(SIGTSTP, sig_tstp) == SIG_ERR) printf("signal(SIGTSTP) error");
		  	close(pipefd[0]); //close the pipe in the parent
		  	close(pipefd[1]);
		  	int count = 0;
			while (count < 2) {
			// Parent's wait processing is based on the sig_ex4.c
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
		}else {
		  //Child 2
		  	sleep(1);
		  	setpgid(0,pid_ch1); //child2 joins the group whose group id is same as child1's pid
		 	close(pipefd[1]); // close the write end
		 	dup2(pipefd[0],STDIN_FILENO);
		 	printf("child2 blah");
		  	execvp(arg2[0], arg2);  // runs word count
		}
	} else {
		  // Child 1
		setsid(); // child 1 creates a new session and a new group and becomes leader -
				  //   group id is same as his pid: pid_ch1
		close(pipefd[0]); // close the read end
		dup2(pipefd[1],STDOUT_FILENO);
		printf("child1 blah");
		execvp(arg1[0], arg1);  // runs top
	}
}