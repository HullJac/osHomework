#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* 0 = stdin
 * 1 = stdout
 * 2 = stderr
 */

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
			argList[index] = curr;
			index++;
			curr++;
			while(*curr != ' ' && *curr != '\0') {
				curr++;
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
	char* exit = "exit";
	char* debug = "debugargs";
	char nl = '\n';
	char cwd[256];
	char inp[256];
	char* args[256];
	char* error = "Not a valid command. Try again.";
	while (1) {
		// Print the cwd and dollar sign and a space
		getcwd(cwd, 256);
		int pathLen = strlen(cwd);
		write(1, &cwd, pathLen);
		char dollar = '$';
		write(1, &dollar, 1);
		char space = ' ';
		write(1, &space, 1);

		// Take input
		int inputSize = read(0, inp, 256);

		// Get rid of the newline at the end
		inp[inputSize - 1] = '\0';
	
		// Puts all the arguments into a list contained in args	
		makeArgList(inp, args);
	
		//Processs commands below here
		// exit
		if (strcmp(args[0], exit) == 0) {
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
			write(1, error, strlen(error));
			write(1, &nl, 1);
		}
	}
	return 0;
}
