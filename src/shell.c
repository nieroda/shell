/*
 * shell.c
 * The core of the shell application.
 */

/*
  MIT License
  Copyright (c) 2018 Tyler Gearing, Nathan Kamm, Ezio Ballarin
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/* 
 * If using the -std-c99 option, then features.h (which is implicitly included
 * by unistd.h) will NOT default to setting _BSD_SOURCE in such a way that the
 * prototype for gethostname() gets included. You force it with your own 
 * feature macro definition (as below). 
 */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include "constants.h"
#include "parsetools.h"

// Function Prototypes
void shellPrompt();
void syserror(const char*);
void runProcess(char*, int, int, int (*)[2], int);
int parseRedirection(char*, int*, int*);

// Globals for shell prompt
char* user; 
char host[_SC_HOST_NAME_MAX];

int main() {
	
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

	shellPrompt();

	while(fgets(line, MAX_LINE_CHARS, stdin)) {

		// Hacker Lvl 3000, not sophisticated but it works
		for(int i = 0; i < strlen(line); ++i) {
			if(line[i] == '\"' || line[i] == '\'')
				line[i] = ' ';
			}
		
		int num_words = split_cmd_line(line, line_words, "|");
        int pfd[num_words - 1][2];
        for (int i = 0; i < num_words - 1; i++) {
			if (pipe(pfd[i]) == -1) {printf("Error creating pipe\n");};
        }

        for (int i = 0; i < num_words; i++) {
			int pipeIn = i == 0 ? - 1 : i - 1;
			int pipeOut = i == num_words - 1 ? -1 : i;
			runProcess(line_words[i], pipeIn, pipeOut, pfd, num_words - 1);
        }
		for (int i = 0; i < num_words - 1; i++) {
			if(close(pfd[i][0]) == -1 || close(pfd[i][1]) == -1)
				syserror("Could not close file descriptors");
		}
	}
	return 0;
}

void shellPrompt() {
    user = getlogin();
    gethostname(host, _SC_HOST_NAME_MAX);
    fprintf(stdout, "[%s@%s] ", user, host);
}

void syserror(const char *s) {
	extern int errno;
	fprintf(stderr, "%s\n", s);
	fprintf(stderr, " (%s)\n", strerror(errno));
	exit(1);
}

int parseRedirection(char *line, int *pipeIn, int *pipeOut) {

	char *linePtr = NULL;
	char *value = NULL;
	int fd = 0;

#ifdef EXTRACREDIT
	char *fileName = NULL;
	int input_fd = -1;
	int output_fd = -1;
#endif

	char *lineCopy = malloc(strlen(line) + 1);
	strcpy(lineCopy, line);

	if ((linePtr = strstr(lineCopy, "<")) != NULL) {

		value = strtok(linePtr + 1, " \t\n");

		if ((fd = open(value, O_RDONLY)) < 0) {
			printf("Could not open file... ERR: %d\n", fd);
			free(lineCopy);
			return -1;
		}
		dup2(fd, 0);
		if(close(fd) == -1)
			syserror("Could not close file descriptor");
		*pipeIn = -1;
	}

	strcpy(lineCopy, line);
	if ((linePtr = strstr(lineCopy, ">")) != NULL) {
		char *append = NULL;
		if ((append = strstr(lineCopy, ">>")) != NULL) {
			linePtr = append + 1;
		}
#ifdef EXTRACREDIT
		else {
			input_fd = (int) *(linePtr - 1) - '0';
			printf("input_fd: %d\n", input_fd);
			if ((linePtr + 1) != '&') {
				fileName = strtok(linePtr + 1, " \t\n");
			}
			else {
				output_fd = (int) *(linePtr + 2) - '0';
				printf("output_fd: %d\n", output_fd);
			}
		}
		
		if (output_fd != -1 && input_fd != -1) {
			goto done;
		}
		
		value = fileName != NULL ? fileName : strtok(linePtr + 1, " \t\n");
		printf("filename: %s\n", value);
#else
		value = strtok(linePtr + 1, " \t\n");
#endif
		if ((fd = open(value,
					   O_CREAT | O_WRONLY | (append == NULL ? 0 : O_APPEND),
					   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
			{
				printf("Could not open file... ERR: %d\n", fd);
				free(lineCopy);
				return -1;
			}
#ifdef EXTRACREDIT
		dup2(fd, input_fd == -1 ? 1 : input_fd);
#else
		dup2(fd, 1);
#endif
		if(close(fd) == -1)
			syserror("Could not close file descriptor");
		*pipeOut = -1;
	}
	free(lineCopy);

	return fd != 0 ? 1 : 0; 
#ifdef EXTRACREDIT
 done:
	free(lineCopy);
	dup2(output_fd, input_fd);
	//close(input_fd);
	if(close(input_fd) == -1)
		syserror("Could not close file descriptor");
	return 2;
#endif
}

void runProcess(char *line, int pipeIn, int pipeOut, int pfd[][2], int len) {

	if (fork() == 0) {

		int redirect = parseRedirection(line, &pipeIn, &pipeOut);
		if (redirect == -1) return;
		if (redirect == 1)
			line = strtok(line, "<>");
		else if (redirect == 2)
			line = strtok(line, "<>0123456789");
	
		//max 100 words
		char* line_words[100];
		split_cmd_line(line, line_words, " \t\n");

		if (pipeIn != -1) {
			dup2(pfd[pipeIn][0], 0);
			if(close(pfd[pipeIn][1]) == -1)
				syserror("Could not close file descriptor");
		}

		if (pipeOut != -1) {
			dup2(pfd[pipeOut][1], 1);
			if(close(pfd[pipeOut][0]) == -1)
				syserror("Could not close file descriptor");
		}

		for (int i = 0; i < len; i++) {
			if (i == pipeIn || i == pipeOut) continue;
			if(close(pfd[i][0]) == -1 || close(pfd[i][1]) == -1)
				syserror("Could not close file descriptors");
		}
		execvp(*line_words, line_words);
		syserror( "Could not exec command line" );
	}
}
