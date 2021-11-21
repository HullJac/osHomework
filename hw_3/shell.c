#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <wait.h>

/* 0 = stdin
 * 1 = stdout
 * 2 = stderr
 */

// Variables for redirecting input and output
char inRed[128];
char outRed[128];
int in = 0;
int out = 0;


// Counts all the args in argList
int countArgs(char* argList[]) {
	int count = 0;
	while (argList[count] != NULL) {
		count++;
	}
	return count;
}


// Puts pointers to the strings into argList
void makeArgList(char* string, char* argList[]) {
	int index = 0;
	char* curr = string;
	while(*curr != '\0') {
		while(*curr == ' ') {
			*curr = '\0';
			curr++;	
		}
		if (*curr != '\0') {
			// If we find input redirection
			if (*curr == '<') {
				in = 1;
				*curr = '\0';
				curr++;
				// Check for the spaces before the file
				while(*curr == ' ') {
					*curr = '\0';
					curr++;
				}
				// Capture the filename
				int i = 0;
				while(*curr != ' ' && *curr != '\0' && *curr != '>') {
					inRed[i] = *curr;
					*curr = '\0';
					curr++;
					i++;
				}
				inRed[i] = '\0';
			}
			// If there is 1 > make out=1 if there are 2 > make out=2
			else if (*curr == '>') {
				// Remember to check for another >
				*curr = '\0';
				curr++;
				if (*curr == '>') {
					*curr = '\0';
					curr++;
					out = 2;
				}
				else {
					out = 1;
				}
				// Check for the spaces before the file
				while(*curr == ' ') {
					*curr = '\0';
					curr++;
				}
				// Capture the filename
				int i = 0;
				while(*curr != ' ' && *curr != '\0' && *curr != '<') {
					outRed[i] = *curr;
					*curr = '\0';
					curr++;
					i++;
				}
				outRed[i] = '\0';
				
			}
			// Otherwise do everything we did before
			else {
				argList[index] = curr;
				index++;
				curr++;
				while(*curr != ' ' && *curr != '\0' && *curr != '<' && *curr != '>') {
					curr++;
				}
			}
		}
	}
	argList[index] = NULL;
}


// Prints out all the arguments in a fancy way
void debugargs(char* argList[]) {
	// Extra stuff to use to fancy print
	char* cmd = "cmd: [";
	char* arg = "arg: [";
	char rbracket = ']';
	char* null = "(null)";
	char nl = '\n';
	
	// Find the number of args in the list
	int numArgs = countArgs(argList);

	// Print the first command as cmd
	write(1, cmd, 6);
	write(1, argList[0], strlen(argList[0]));
	write(1, &rbracket, 1);
	write(1, &nl, 1);

	for (int i = 0; i < numArgs; i++) {
		write(1, arg, 6);
		write(1, argList[i], strlen(argList[i]));
		write(1, &rbracket, 1);
		write(1, &nl, 1);
	}

	// Print the last null
	write(1, arg, 6);
	write(1, null, 6);
	write(1, &rbracket, 1);
	write(1, &nl, 1);
}


int main() {
	// Variables to hold stuff to print and input
	char* cd = "cd";
	char* pwd = "pwd";
	char* ext = "exit";
	char* debug = "debugargs";
	char nl = '\n';
	char cwd[256];
	char inp[256];
	char* args[256];
	char* err = "Not a valid command. Try again.";
	char dollar = '$';
	char space = ' ';
	while (1) {
		// Print the cwd and dollar sign and a space
		getcwd(cwd, 256);
		int pathLen = strlen(cwd);
		write(1, &cwd, pathLen);
		write(1, &dollar, 1);
		write(1, &space, 1);

		// Take input
		int inputSize = read(0, inp, 256);

		// Get rid of the newline at the end
		inp[inputSize - 1] = '\0';
	
		// Puts all the arguments into a list contained in args	
		makeArgList(inp, args);
	
		//Processs commands below here
		// exit
		if (strcmp(args[0], ext) == 0) {
			break;
		}
		// cd
		else if (strcmp(args[0], cd) == 0) {
			if (chdir(args[1]) != 0) {
				char* error = strerror(errno);
				write(1, error, strlen(error));
				write(1, &nl, 1);
			}
		}
		// pwd
		else if (strcmp(args[0], pwd) == 0) {
			getcwd(cwd, 256);
			int pathLen = strlen(cwd);
			write(1, &cwd, pathLen);
			write(1, &nl, 1);
		}
		// debugargs
		else if (strcmp(args[0], debug) == 0) {
			debugargs(args);
		}
		// If arg is not one of the available args
		else {
			int childPid;
			if ((childPid = fork()) == 0) {
				// Change file descriptor stuff
				// stdin file descriptor
				if (in == 1) {
					if (close(0) < 0) {
						char* error = strerror(errno);
						write(1, error, strlen(error));
						write(1, &nl, 1);
					}
					else {
						if (open(inRed, O_RDONLY) < 0) {
							char* error = strerror(errno);
							write(1, error, strlen(error));
							write(1, &nl, 1);
						}
					}
				}
				// stdout file descriptor
				if (out > 0) {
					if (close(1) < 0) {
						char* error = strerror(errno);
						write(1, error, strlen(error));
						write(1, &nl, 1);
					}
					else {
						if (out == 1) {
							if (open(outRed, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU) < 0) {
								char* error = strerror(errno);
								write(1, error, strlen(error));
								write(1, &nl, 1);
							}
						}
						else {
							if (open(outRed, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU) < 0) {
								char* error = strerror(errno);
								write(1, error, strlen(error));
								write(1, &nl, 1);
							}
						}
					}
				}
				// Actually start the new program to be ran
				if (execvp(args[0],args) < 0) {
					char* error = strerror(errno);
					write(1, error, strlen(error));
					write(1, &nl, 1);
				}
				exit(1);
			}
			else {
				while(wait(0) != childPid)
					;
			}
		}
		// Reset the boolean values for redirecting input and output
		in = 0;
		out = 0;
	}
	return 0;
}
