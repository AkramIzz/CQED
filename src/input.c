#ifdef _WIN32
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// TODO handle failure and empty lines
char* readline(char *prompt) {
	fputs(prompt, stdout);
	char *buffer = malloc(2048);
	fgets(buffer, 2048, stdin);
	buffer[strlen(buffer)-1] = '\0'; // remove newline character
	return buffer;
}

void add_history(char *line) {
	// not supported in windows
}
#endif