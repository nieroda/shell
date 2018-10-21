#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include "constants.h"
#include "parsetools.h"

// Checking commit

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
    close(fd);
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
      if (isalpha(*(linePtr + 1))/* && (*(linePtr + 1) != '&')*/)
		  fileName = strtok(linePtr + 1, " \t\n");
	  else
		  output_fd = (int) *(linePtr + 2) - '0';
    }
	
    if (output_fd != -1 && input_fd != -1) {
      goto done;
    }

    value = fileName != NULL ? fileName : strtok(linePtr + 1, " \t\n");
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
    close(fd);
    *pipeOut = -1;
  }
  free(lineCopy);

  return fd != 0 ? 1 : 0;
#ifdef EXTRACREDIT
done:
  free(lineCopy);
  dup2(input_fd, output_fd);
  //close(input_fd);
  return 2;
#endif
}

void runProcess(char *line, int pipeIn, int pipeOut, int pfd[][2], int len) {

  if (fork() == 0) {

    int redirect = parseRedirection(line, &pipeIn, &pipeOut);
    if (redirect == -1) return;
    if (redirect == 1) {
      line = strtok(line, "<>");
    } else if (redirect == 2) {
      line = strtok(line, "<>0123456789");
    }
    //max 100 words ¯\_(ツ)_/¯...wtf
    char* line_words[100];
    split_cmd_line(line, line_words, " \t\n");

    if (pipeIn != -1) {
      dup2(pfd[pipeIn][0], 0);
      close(pfd[pipeIn][1]);
    }

    if (pipeOut != -1) {
      dup2(pfd[pipeOut][1], 1);
      close(pfd[pipeOut][0]);
    }

    for (int i = 0; i < len; i++) {
      if (i == pipeIn || i == pipeOut) continue;
      close(pfd[i][0]);
      close(pfd[i][1]);
    }
    execvp(*line_words, line_words);
  }
}

int main() {

    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
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
          close(pfd[i][0]);
          close(pfd[i][1]);
        }
    }

    return 0;
}
