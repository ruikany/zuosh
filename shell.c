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

  // command selection
  if (strcmp(command, "help") == 0) {
    printf("Available commands:\n");
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
      printf("Usage: zcat <filename>\n");
      return EXIT_FAILURE;
    }
    zcat(argv[1]);
  } else if (strcmp(command, "zgrep") == 0) {
    if (argc < 3) {
      printf("Usage: zgrep <search_term> <filename(s)>\n");
      return EXIT_FAILURE;
    }
    zgrep(argv[1], &argv[2], argc - 2);
  } else if (strcmp(command, "zzip") == 0) { // does RLE
    if (argc < 2) {
      printf("Usage: zzip <filename(s)>\n");
      return EXIT_FAILURE;
    }
    zzip(&argv[1], argc - 1);
  } else if (strcmp(command, "zunzip") == 0) {
    if (argc < 2) {
      printf("Usage: zunzip <filename(s)>\n");
      return EXIT_FAILURE;
    }
    zunzip(&argv[1], argc - 1);
  } else if (strcmp(command, "zsort") == 0) {
    if (argc != 2) {
      printf("Usage: zsort <filename>\n");
      return EXIT_FAILURE;
    }
    zsort(argv[1]);
  } else if (strcmp(command, "zrev") == 0) {
    if (argc != 2) {
      printf("Usage: zrev <filename>\n");
      return EXIT_FAILURE;
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
  enable_rmode();
  init_history();

  char *userInput = NULL;
  int errorCode = 0;
  char inputBuffer[MAX_USER_INPUT] = {0};
  int inputPos = 0;

  while (1) {
    if (isatty(STDIN_FILENO)) {
      printf("\r>> %s", inputBuffer);
      fflush(stdout);
    }

    unsigned char c;
    ssize_t bytes_read = read(STDIN_FILENO, &c, 1);
    if (bytes_read == -1) {
      perror("read error");
      break;
    }

    if (c == '\x1B') {
      unsigned char seq[2];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;

      if (seq[0] == '[') {
        char *historyCmd = NULL;
        switch (seq[1]) {
        case 'A': // Up arrow
          historyCmd = peek_history(-1);
          if (historyCmd) {
            strncpy(inputBuffer, historyCmd, MAX_USER_INPUT);
            inputPos = strlen(inputBuffer);
          }
          continue;
        case 'B': // Down arrow
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
    // Handle Enter key
    else if (c == '\n') {
      if (inputPos > 0) {
        // Add to history and process
        add_to_history(inputBuffer);
        errorCode = parseInput(inputBuffer);

        // Clear input buffer
        inputBuffer[0] = '\0';
        inputPos = 0;
      }
      printf("\n");
      continue;
    }
    // Handle backspace
    else if (c == 127 || c == '\b') { // Backspace/Delete
      if (inputPos > 0) {
        inputBuffer[--inputPos] = '\0';
      }
      continue;
    }
    // Regular character input
    else if (isprint(c) && inputPos < MAX_USER_INPUT - 1) {
      inputBuffer[inputPos++] = c;
      inputBuffer[inputPos] = '\0';
    }
  }

  free(userInput);
  return errorCode;
}

// int main() {
//   printf("zuosh\n");
//   enable_rmode();
//   init_history();
//
//   char *userInput = NULL;
//   size_t len = 0;
//   ssize_t rread;
//   int errorCode = 0;
//
//   while (1) {
//     if (isatty(STDIN_FILENO)) {
//       printf(">> ");
//       fflush(stdout);
//     }
//
//     rread = getline(&userInput, &len, stdin);
//     if (rread == -1) {
//       if (feof(stdin)) {
//         break;
//       } else {
//         perror("getline failed");
//         exit(1);
//       }
//     }
//     unsigned char c;
//     read(STDIN_FILENO, &c, 1);
//
//     if (c == '\x1B') { // Escape sequence
//       unsigned char seq[2];
//       if (read(STDIN_FILENO, &seq[0], 1) != 1)
//         continue;
//       if (read(STDIN_FILENO, &seq[1], 1) != 1)
//         continue;
//
//       if (seq[0] == '[') {
//         switch (seq[1]) {
//         case 'A':
//           peek_history(-1);
//           continue;
//         case 'B':
//           peek_history(1);
//           continue;
//         }
//       }
//     }
//
//     if (rread > 1) {
//       add_to_history(userInput);
//       errorCode = parseInput(userInput);
//     }
//   }
//   free(userInput);
//   return errorCode;
// }
