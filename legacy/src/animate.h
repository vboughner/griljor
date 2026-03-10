typedef struct player_struct {
  char *name;
  Pixmap bits;
  Pixmap mask;
  Pixmap restore;
  int curx, cury;
  int locx, locy;
  int changeme;
  int inc;
  int direction;
  int dead;
  int drawdead;
  struct player_struct *next;
} Player;
