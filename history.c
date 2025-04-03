#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "history.h"

struct CommandHistory *history = NULL;

void init_history() {
  history = malloc(sizeof(struct CommandHistory));
  history->head = 0;
  history->tail = -1;
  history->count = 0;
  history->current_pos = -1;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    history->commands[i] = NULL;
  }
}

char *peek_history(int direction) { // actually, could just copy to new array
  char *command = NULL;
  if (history->count == 0)
    return command;

  if (history->current_pos == -1) {
    history->current_pos = history->tail;
  } else {
    history->current_pos = (history->current_pos + direction) % HISTORY_SIZE;
  }

  if (direction == -1) {
    if (history->current_pos == history->head) {
      command = history->commands[history->head];
    }
    return history->commands[history->current_pos];
  } else {
    if (history->current_pos == (history->tail + 1) % HISTORY_SIZE) {
      command = NULL;
    }
  }
  return command;
}

void add_to_history(char *command) {
  history->current_pos = -1;
  if (history->count == HISTORY_SIZE) {
    free(history->commands[history->head]);
    history->head = (history->head + 1) % HISTORY_SIZE;
    history->count--;
  }
  history->tail = (history->tail + 1) % HISTORY_SIZE;
  history->commands[history->tail] = strdup(command);
  history->count++;
}

void free_history() {
  for (int i = 0; i < HISTORY_SIZE; i++) {
    if (history->commands[i] != NULL)
      free(history->commands[i]);
  }
  free(history); ////////////////
}
