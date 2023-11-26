#define DEBUGPRINTF

enum DisplayCommands
{
  clear,
  position,
  scroll_left,
  scroll_right,
  none,
  ERROR
};

typedef enum DisplayCommands tDisplayCommand;

int myatoi(char* Num);
void ParseUntilDel(char* c, char* target, int from, int max, char del);
tDisplayCommand ParseCommand(char* c, int* i, int* line, int* col);
