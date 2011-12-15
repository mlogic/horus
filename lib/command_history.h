

#define HISTORY_SIZE 128
#define HISTORY_PREV(x) ((x) == 0 ? HISTORY_SIZE - 1 : (x) - 1)
#define HISTORY_NEXT(x) ((x) + 1 == HISTORY_SIZE ? 0 : (x) + 1)
struct command_history
{
  char *array[HISTORY_SIZE];
  int last;
  int current;
};

void command_history_add (char *command_line,
       struct command_history *history, struct shell *shell);

//EXTERN_COMMAND (show_history);
