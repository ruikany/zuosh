#define MAX_USER_INPUT 1000
#define MAX_ARGS 10

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "history.h"
#include "utilities.h"

struct termios orig_termios;

void disable_rmode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enable_rmode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_rmode);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int interpret(int argc, char *argv[]) {
  char *command = argv[0];

  if (strcmp(command, "help") == 0) {
    printf("\nAvailable commands:\n");
    printf("  zcat <filename>                      - display file contents\n");
    printf(
        "  zgrep <search_term> <filename(s)>    - search for text in files\n");
    printf(
        "  zzip <filename(s)>                   - compress files using RLE\n");
    printf("  zunzip <filename(s)>                 - decompress RLE-compressed "
           "files\n");
    printf("  zsort <filename>                     - sort file contents\n");
    printf("  zrev <filename>                      - reverse file contents\n");
    printf("  exit                                 - exit the program\n");
    printf("  help                                 - show this help message\n");
    return EXIT_SUCCESS;
  }

  if (strcmp(command, "zcat") == 0) { // command is on argv[1]
    if (argc != 2) {
      printf("\nUsage: zcat <filename>");
      return EXIT_FAILURE;
    }
    zcat(argv[1]);
  } else if (strcmp(command, "zgrep") == 0) {
    if (argc < 3) {
      printf("\nUsage: zgrep <search_term> <filename(s)>");
      return EXIT_FAILURE;
    }
    zgrep(argv[1], &argv[2], argc - 2);
  } else if (strcmp(command, "zzip") == 0) { // does RLE
    if (argc < 2) {
      printf("\nUsage: zzip <filename(s)>");
      return EXIT_FAILURE;
    }
    zzip(&argv[1], argc - 1);
  } else if (strcmp(command, "zunzip") == 0) {
    if (argc < 2) {
      printf("\nUsage: zunzip <filename(s)>");
      return EXIT_FAILURE;
    }
    zunzip(&argv[1], argc - 1);
  } else if (strcmp(command, "zsort") == 0) {
    if (argc != 2) {
      printf("\nUsage: zsort <filename>");
      return EXIT_FAILURE;
    }
    zsort(argv[1]);
  } else if (strcmp(command, "zrev") == 0) {
    if (argc != 2) {
      printf("\nUsage: zrev <filename>");
      return EXIT_FAILURE;
    }
    zrev(argv[1]);
  } else if (strcmp(command, "exit") == 0) {
    quit();
  } else {
    fprintf(stderr, "\nCOMMAND DOES NOT EXIST: %s", command);
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

  // should_exit flag so that the above tokens[] can be cleaned up. Instead of
  // exiting in interpret() and never reaching clean up
  int should_exit = 0;
  if (num_of_tokens > 0 && strcmp(tokens[0], "exit") == 0) {
    should_exit = 1;
  }
  interpret(num_of_tokens, tokens);
  for (int i = 0; i < num_of_tokens; i++) {
    free(tokens[i]);
  }

  if (should_exit)
    exit(0);
  return 0;
}

int main() {
  printf("zuosh123\n");
  enable_rmode();
  init_history();

  char *userInput = NULL;
  int errorCode = 0;
  char inputBuffer[MAX_USER_INPUT] = {0};
  int inputPos = 0;

  while (1) {
    if (isatty(STDIN_FILENO)) {
      printf("\r\033[K> %s", inputBuffer);
      fflush(stdout);
    }

    unsigned char c;
    ssize_t bytes_read = read(STDIN_FILENO, &c, 1);
    if (bytes_read == -1) {
      perror("read error");
      break;
    }

    if (c == '\x1B') {
      unsigned char sequence[2];
      if (read(STDIN_FILENO, &sequence[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &sequence[1], 1) != 1)
        continue;

      // arrow keys: A for up, B for down
      if (sequence[0] == '[') {
        char *historyCmd = NULL;
        switch (sequence[1]) {
        case 'A':
          historyCmd = peek_history(-1);
          if (historyCmd) {
            strncpy(inputBuffer, historyCmd, MAX_USER_INPUT);
            inputPos = strlen(inputBuffer);
          }
          continue;
        case 'B':
          historyCmd = peek_history(1);
          if (historyCmd) {
            strncpy(inputBuffer, historyCmd, MAX_USER_INPUT);
            inputPos = strlen(inputBuffer);
          } else {
            inputBuffer[0] = '\0';
            inputPos = 0;
          }
          continue;
        }
      }
    }
    // enter key
    else if (c == '\n') {
      if (inputPos > 0) {
        add_to_history(inputBuffer);
        errorCode = parseInput(inputBuffer);

        inputBuffer[0] = '\0';
        inputPos = 0;
      }
      printf("\n");
      continue;
    }
    // backspace
    else if (c == 127 || c == '\b') {
      if (inputPos > 0) {
        inputBuffer[--inputPos] = '\0';
      }
      continue;
    }
    // regular input
    else if (isprint(c) && inputPos < MAX_USER_INPUT - 1) {
      inputBuffer[inputPos++] = c;
      inputBuffer[inputPos] = '\0';
    }
  }

  free(userInput);
  return errorCode;
}
