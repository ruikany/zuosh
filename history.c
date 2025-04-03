#include <stdio.h>
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

// check bounds first, THEN update position if in bounds
char *peek_history(int direction) {
  if (history->count == 0)
    return NULL;

  if (history->current_pos == -1) {
    history->current_pos = history->tail;
    return history->commands[history->current_pos];
  }

  int new_pos = history->current_pos + direction;
  if (history->tail >= history->head) {
    if (new_pos < history->head || new_pos > history->tail)
      return NULL;
  } else {
    if ((direction > 0 && new_pos > history->tail && new_pos < history->head) ||
        (direction < 0 && new_pos < history->head && new_pos > history->tail))
      return NULL;
  }

  if (new_pos < 0)
    new_pos = HISTORY_SIZE - 1;
  if (new_pos >= HISTORY_SIZE)
    new_pos = 0;

  history->current_pos = new_pos;
  return history->commands[history->current_pos];
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
