#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipefd[2];
int status, pid_ch1, pid_ch2, pid, out, in, err;


static void sig_handler(int signo){
    switch(signo){
        case SIGINT:
            //CTRL+C kill fg job
            kill(-pid_ch1,SIGINT);
            break;
        case SIGTSTP:
            //CTRL+Z Stop fg job
            kill(-pid_ch1,SIGTSTP);        
            break;
    }
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
		  	if (signal(SIGINT, sig_handler) == SIG_ERR) printf("signal(SIGINT) error");
		  	if (signal(SIGTSTP, sig_handler) == SIG_ERR) printf("signal(SIGTSTP) error");
		  	close(pipefd[0]); //close the pipe in the parent
		  	close(pipefd[1]);
		  	int count = 0;
			while (count < 2) {
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
		  	execvp(arg2[0], arg2);  // runs word count
		}
	} else {
		  // Child 1
		setsid(); // child 1 creates a new session and a new group and becomes leader group id is same as his pid: pid_ch1
		close(pipefd[0]); // close the read end
		dup2(pipefd[1],STDOUT_FILENO);
		execvp(arg1[0], arg1);  // runs top
	}
}

void pipeRedirection(char** arg1, char** arg2, 
	char* fileFrom, char* fileTo, char* fileErr, int to, int from, int error){
	
	char ch[1]={0};
	
  	if (pipe(pipefd) == -1) {
		  perror("pipe");
	 	 exit(-1);
  	}
  	//create child
  	pid_ch1 = fork();

	//open files
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

	if (pid_ch1 > 0){
	  // Parent
		pid_ch2 = fork();
		if (pid_ch2 > 0){
			if (signal(SIGINT, sig_handler) == SIG_ERR) printf("signal(SIGINT) error");
			if (signal(SIGTSTP, sig_handler) == SIG_ERR) printf("signal(SIGTSTP) error");
			close(pipefd[0]); //close the pipe in the parent
			close(pipefd[1]);
			int count = 0;
		  	while (count < 2) {
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
	  	}else {
		//Child 2
			sleep(1);
			setpgid(0,pid_ch1); //child2 joins the group whose group id is same as child1's pid
		   	close(pipefd[1]); // close the write end
			dup2(pipefd[0],STDIN_FILENO);
			if(to){dup2(out,STDOUT_FILENO);}
			if(error){dup2(err,STDERR_FILENO);}
			execvp(arg2[0], arg2);  // runs word count
	  	}
  	} else {
		// Child 1
	  	setsid(); // child 1 creates a new session and a new group and becomes leader group id is same as his pid: pid_ch1
		close(pipefd[0]); // close the read end
		if(from){dup2(in,STDIN_FILENO);}
		else {dup2(pipefd[0],STDIN_FILENO);} 
	  	dup2(pipefd[1],STDOUT_FILENO);
	  	execvp(arg1[0], arg1);  // runs top
  }
}
