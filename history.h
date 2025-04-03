#define HISTORY_SIZE 10

struct CommandHistory {
  char *commands[HISTORY_SIZE];
  int head;
  int tail;
  int count;
  int current_pos;
};

extern struct CommandHistory *history;
void init_history();
void add_to_history(char *command);
void free_history();
char *peek_history(int direction);
