#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "constants.h"
#include "parsetools.h"


int parseRedirection(char *line, int *pipeIn, int *pipeOut) {

  char *linePtr = NULL; char *value; int fd = 0;
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
    value = strtok(linePtr + 1, " \t\n");
    if ((fd = open(value,
      O_CREAT | O_WRONLY | (append == NULL ? 0 : O_APPEND),
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0)
    {
      printf("Could not open file... ERR: %d\n", fd);
      free(lineCopy);
      return -1;
    }
    dup2(fd, 1);
    close(fd);
    *pipeOut = -1;
  }

  free(lineCopy);
  if (fd != 0) {
    return 1;
  }
  return 0;
}

void runProcess(char *line, int pipeIn, int pipeOut, int pfd[][2], int len) {

  if (fork() == 0) {

    int redirect = parseRedirection(line, &pipeIn, &pipeOut);
    if (redirect == -1) return;
    if (redirect == 1) {
      line = strtok(line, "<>");
    }
    //max 10 words ¯\_(ツ)_/¯
    char* line_words[10];
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

    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    // holds separated words based on whitespace
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
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
