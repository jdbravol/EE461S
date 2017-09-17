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
void restartVariables(void);
void scanInput(char[] , size_t);
char* getProgram(char[], int, int);
char** organizeArguments(char* );
void freeArgs(char**);

//Variables to determine where process are
int redirection[3] = {INT_MAX, INT_MAX, INT_MAX};
int piping = INT_MAX;
bool jobControl = false;

int main(int argc, char** argv){
	char input[2000];
	//activate signals
	while (1){
		//restart variables
		restartVariables();

		//console I/O
		printf("# ");
        fgets(input, 2000, stdin);
        
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
        // will only do a file redirection
        else if (redirection[0] < piping){
			int index = redirection[0];
			int end = (int) inputLength;
			int start = 0;
			char * file; 
			char ** arguments;
			// run first file redirection
			if (redirection[1] != INT_MAX){
				end = redirection[1];
			} 
			if (input[index] == '>'){
				if (input[index+1] == '>'){
					// STDERROR
				} else{
					file = getProgram(input, index, end);
					char* program = getProgram(input, start, index);
					arguments = organizeArguments(program);
					redirectToFile(arguments, file);
				}
			} else if (input[index] == '<'){
				file = getProgram(input, start, index);
				char* program = getProgram(input, index, end);
				arguments = organizeArguments(program);
			}

			//run second file redirection
			if (redirection[1] != INT_MAX){
				index = redirection[1];
				start = redirection[0];
				if (redirection[2] != INT_MAX){
					end = redirection[2];
				}
				if (input[index] == '>'){
					if (input[index+1] == '>'){
						// STDERROR
					}
					else{
						file = getProgram(input, index, end);
						char* program = getProgram(input, start, index);
						arguments = organizeArguments(program);
						redirectToFile(arguments, file);
					}

				} else if (input[index] == '<'){
					file = getProgram(input, start, index);
					char* program = getProgram(input, index, end);
					arguments = organizeArguments(program);
				}
			}

			//run third file redirection
			if (redirection[2] != INT_MAX){
				index = redirection[2];
				start = redirection[1];
				if (input[index] == '>'){
					if (input[index+1] == '>'){
						// STDERROR
					}
					else{
						file = getProgram(input, index, end);
						char* program = getProgram(input, start, index);
						arguments = organizeArguments(program);
						redirectToFile(arguments, file);
					}

				} else if (input[index] == '<'){
					file = getProgram(input, start, index);
					char* program = getProgram(input, index, end);
					arguments = organizeArguments(program);
				}
			}
		}
		// do both file piping first and then redirection
		else if (piping < redirection[0] && redirection[0] != INT_MAX) {

		}
        //piping only
		else if (piping != INT_MAX){
            char* program1 = getProgram(input, 0, piping);
            char* program2 = getProgram(input,piping+1, (int) inputLength);
            //arguments
            char** argument1 = organizeArguments(program1);
            char** argument2 = organizeArguments(program2);
            pipeProcess(argument1, argument2);
            free(program1);
        	free(program2);
        }
		//only one process
		else{
			char** argument = organizeArguments(input);
			singleProcess(argument);
			freeArgs(argument);
		}
	}
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
        //search for redirection
		if (arguments[i] == '>' || arguments[i] == '<'){
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
    for(int x = 0; x < strlen((const char *) myArgs); x++){
        free(myArgs[x]);
    }
    free(myArgs);
}