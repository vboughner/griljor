h38403
s 00000/00000/00014
d D 1.2 92/08/07 01:00:10 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00014/00000/00000
d D 1.1 91/05/17 04:16:17 labc-3id 1 0
c date and time created 91/05/17 04:16:17 by labc-3id
e
u
U
f e 0
t
T
I 1
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
E 1
