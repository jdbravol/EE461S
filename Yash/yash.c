#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pipeProcess.h"
#include "singleProcess.h"
#include "redirectionProcess.h"

typedef enum { false, true} bool;

//function declarations
void activateSignals(void);
void restartVariables(void);
void scanInput(char[] , size_t);
char* getProgram(char[], int, int);
char** organizeArguments(char* );
void freeArgs(char**);

//variable to determine number of tokens
int tokens = 0;
//Variables to determine where process are
int redirection[3] = {INT_MAX, INT_MAX, INT_MAX};
int piping = INT_MAX;
bool jobControl = false;

int main(int argc, char** argv){
	char input[2000];
	//activate signals
	activateSignals();

	while (1){
		//restart variables
		restartVariables();

		//console I/O
		printf("# ");
        if (fgets(input, 2000, stdin) == NULL){
			exit(0);
		};
        //trim input from \n
        size_t inputLength = strlen(input);
        if(input[inputLength-1] == '\n'){
            input[inputLength-1] = '\0';
        }

		//Scan input
		scanInput(input, inputLength);

        // job control
        if (jobControl){

		}
		// do both file piping first and then redirection
		else if (piping != INT_MAX && redirection[0] != INT_MAX) {
			//get first token
			int pipe = 0, token = 0, in = 0, out = 0, err = 0;
			int x = 0;
			char* fileTo = NULL;
			char* fileFrom = NULL;
			char* fileErr = NULL;
			char** prog1 = NULL;
			char** prog2 = NULL;
			if (piping < redirection[0]){
				token = piping;
				piping = INT_MAX;
				x = 0;
			} else {
				token = redirection[0];
				x = 1;
			}
			// first arg will always be a program
			char* program1 = getProgram(input, 0, token - 1);
			prog1 = organizeArguments(program1);
			for (x = x; x < 3; x++){
				// get second token use prev token as first value for next arg
				int start = token;
				token = inputLength - 1;
				if (piping < redirection[x + 1]){
					token = piping; 
					piping = INT_MAX;
					x--;
				} else if (redirection[x] != INT_MAX){
					token = redirection[x];
				}
				if(input[start] == '|'){
					char* program2 = getProgram(input, start + 1, token - 1);
					prog2 = organizeArguments(program2);
				}
				// get next argument
				else if(input[start] == '<' && redirection[x] <= start){
					//file is a secong argument
					fileFrom = getProgram(input, start + 1, token);
					//flag on
					out = 1;
					pipe = 1;
				}
				else if (input[start] == '>'){
					//file is second
					fileTo = getProgram(input, start + 1, token);
					//flag on
					in = 1;					
				}
				else if (input[start] == '2' && input[start+1] == '>'){
					//program is first if only 1 redirection
					fileErr = getProgram(input, start + 2, token);
					err = 1;
				}

			}
			if (pipe){
				pipeRedirection(prog1, prog2, fileFrom, fileTo, fileErr, in, out, err);
				free(prog1);
				free(prog2);
			}
			else{
				redirect(prog1, fileTo, fileFrom, fileErr, in, out, err);
				free(prog1);
			}
			if (in){free(fileTo);}
			if (out){free(fileFrom);}
			if (err){free(fileErr);}
		}


        // will only do a file redirection
        else if (redirection[0] != INT_MAX){
			int in = 0 , out = 0, err = 0;
			int index;
			int start = 0;			
			int end = (int) inputLength - 1;
			char* fileTo = NULL;
			char* fileFrom = NULL;
			char* fileErr = NULL; 
			char** arguments = NULL;
			//check for entire command
			for(int x = 0; x < 3; x++){
				if(redirection[x] != INT_MAX){
					index = redirection[x];
					//change end
					end = (int) inputLength - 1;
					if (redirection[x+1] != INT_MAX){end = redirection[x+1] - 1; }
					if(input[index] == '<'){
						//program is first argument
						char* program = getProgram(input, start, index);
						arguments = organizeArguments(program);
						//file is a secong argument
						fileFrom = getProgram(input, index + 1, end);
						//flag on
						out = 1;
					}
					else if (input[index] == '>'){
						//program is first
						char* program = getProgram(input, start, index);
						arguments = organizeArguments(program);
						//file is second
						fileTo = getProgram(input, index + 1, end);
						//flag on
						in = 1;					
					}
					else if (input[index] == '2' && input[index+1] == '>'){
						//program is first if only 1 redirection
						if (x == 0){
							char* program = getProgram(input, start, index);
							arguments = organizeArguments(program);
						}
						fileErr = getProgram(input, index + 2, end);
						err = 1;
					}
				}
			}

			//Redirect after grabing whole code
			redirect( arguments, fileTo, fileFrom, fileErr, in, out, err);

			//free heap
			freeArgs(arguments);
			if (in){free(fileTo);}
			if (out){free(fileFrom);}
			if (err){free(fileErr);}

		}
		
        //piping only
		else if (piping != INT_MAX){
            char* program1 = getProgram(input, 0, piping);
            char* program2 = getProgram(input,piping+1, (int) inputLength);
            //arguments
            char** argument1 = organizeArguments(program1);
            char** argument2 = organizeArguments(program2);
			pipeProcess(argument1, argument2);
			
            freeArgs(argument1);
        	freeArgs(argument2);
        }
		//only one process
		else{
			char** argument = organizeArguments(input);
			singleProcess(argument);
			free(argument);
		}
		//job control if last value is &
		if (input[inputLength - 1] == '&'){
			
		}
	}
}

void activateSignals(void){
}

void restartVariables(void){
	redirection[0] = INT_MAX;
	redirection[1] = INT_MAX;
	redirection[2] = INT_MAX;
	piping = INT_MAX;
	jobControl = false;
}

void scanInput(char arguments[], size_t size){
	for(int i = 0; i < size; i++){
		//search for 2> redirection
		if (arguments[i] == '2' && arguments[i+1] == '>'){
			if(redirection[0] == INT_MAX){
				redirection[0]=i;
			}else if (redirection[1] == INT_MAX){
				redirection[1]=i;
			}else{
				redirection[2]=i;
			}
			//make sure to skip one character
			i++;
		}
		//search for single redirections
		else if (arguments[i] == '>' || arguments[i] == '<'){
			if(redirection[0] == INT_MAX){
				redirection[0]=i;
			}else if (redirection[1] == INT_MAX){
				redirection[1]=i;
			}else {
				redirection[2]=i;
			}
		}
        //search for pipe
        else if(arguments[i] == '|'){
			tokens ++;
			piping = i;
		}
        //search for job control
        else if (arguments[i] == 'j' && size-i > 3){
            if (arguments[i++] == 'o' && arguments[i+2] =='b' && arguments[i+3] == 's'){
                jobControl = true;
                break;
            }
        } else if (arguments[i] == 'b' && size - i > 1){
            if (arguments[i++] == 'g'){
				jobControl = true;
				break;
			}
        } else if (arguments[i] == 'f' && size - i > 1){
			if (arguments[i++] == 'g'){
				jobControl = true;
				break;
			}
		}
	}
}

char* getProgram(char input[], int start, int end){
    size_t prgLen = (size_t) (end - start);
    char* program = (char *) malloc(prgLen);
    for (int x = 0; x < prgLen; x++){
        program[x] = input[x + start];
    }
    // trim from initial and final space
    if (program[0] == ' '){
        for (int x = 0; x < prgLen; x++){
            program[x] = program[x+1];
        }
        program[prgLen-1] = '\0';
        prgLen --;
    }
    if (program[prgLen-1] == ' '){
        program[prgLen-1] = '\0';
    }
    return program;
}

char** organizeArguments(char* wholeLine){
    int argsLen = (int) strlen(wholeLine);
    int spaces = 0;
    for (int x = 0; x < argsLen; x++){
        if (wholeLine[x] == ' '){
            spaces++;
        }
    }
    char ** myargs = (char **) malloc((size_t) (spaces + 2));
    char* argument = strtok(wholeLine, " ");
    int argIndex = 0;
    while (argument != NULL){
        myargs[argIndex] = argument;
        argument = strtok(NULL, " ");
        argIndex ++;
    }
    myargs[argIndex] = NULL;
    return myargs;
}

void freeArgs(char** myArgs){
	free(myArgs[0]);
    free(myArgs);
}