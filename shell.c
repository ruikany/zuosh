#define MAX_USER_INPUT 1000
#define MAX_ARGS 10

#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int interpret(int argc, char *argv[]) {
  char *command = argv[0];

  // command selection
  if (strcmp(command, "zcat") == 0) { // command is on argv[1]
    if (argc != 2) {
      printf("Usage: zcat <filename>\n");
      return EXIT_FAILURE;
    }
    zcat(argv[1]);
  } else if (strcmp(command, "zgrep") == 0) {
    if (argc < 3) {
      printf("Usage: zgrep <search_term> <filename(s)>\n");
      return 1;
    }
    zgrep(argv[1], &argv[2], argc - 2);
  } else if (strcmp(command, "zzip") == 0) { // does RLE
    if (argc < 2) {
      printf("Usage: zzip <filename(s)>\n");
      return 1;
    }
    zzip(&argv[1], argc - 1);
  } else if (strcmp(command, "zunzip") == 0) {
    if (argc < 2) {
      printf("Usage: zunzip <filename(s)>\n");
      return 1;
    }
    zunzip(&argv[1], argc - 1);
  } else if (strcmp(command, "zsort") == 0) {
    if (argc != 2) {
      printf("Usage: zsort <filename>\n");
      return 1;
    }
    zsort(argv[1]);
  } else if (strcmp(command, "zrev") == 0) {
    if (argc != 2) {
      printf("Usage: zrev <filename>\n");
      return 1;
    }
    zrev(argv[1]);
  } else if (strcmp(command, "exit") == 0) {
    quit();
  } else {
    fprintf(stderr, "COMMAND DOES NOT EXIST: %s\n", command);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int parseInput(char *input) {
  char *tokens[MAX_ARGS];
  int num_of_tokens = 0;

  input[strcspn(input, "\n")] = '\0';
  char *token = strtok(input, " ");
  while (token != NULL && num_of_tokens <= MAX_ARGS) {
    tokens[num_of_tokens++] = strdup(token);
    token = strtok(NULL, " ");
  }

  int errorCode = interpret(num_of_tokens, tokens);
  for (int i = 0; i < num_of_tokens; i++) {
    free(tokens[i]);
  }
  return errorCode;
}

int main() {
  printf("zuosh\n");
  char *userInput = NULL;
  size_t len = 0;
  ssize_t read;
  int errorCode = 0;

  while (1) {
    if (isatty(STDIN_FILENO)) {
      printf(">> ");
      fflush(stdout);
    }

    read = getline(&userInput, &len, stdin);
    if (read == -1) {
      if (feof(stdin)) {
        break;
      } else {
        perror("getline failed");
        exit(1);
      }
    }
    errorCode = parseInput(userInput);
  }
  free(userInput);
  return errorCode;
}
