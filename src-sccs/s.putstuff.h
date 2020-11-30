h14669
s 00000/00000/00024
d D 1.4 92/08/07 01:03:36 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00001/00001/00023
d D 1.3 91/12/09 00:29:33 labc-4lc 3 2
c added alteration messages
e
s 00002/00002/00022
d D 1.2 91/08/26 00:34:46 vanb 2 1
c fixed up procedure defs and other compatibilty problems
e
s 00024/00000/00000
d D 1.1 91/02/16 13:01:17 labc-3id 1 0
c date and time created 91/02/16 13:01:17 by labc-3id
e
u
U
f e 0
t
T
I 1

#define MAXSTRLEN 80
D 3
#define MAXLINES  100
E 3
I 3
#define MAXLINES  1000
E 3
#define DEFAULT_HOW_OFTEN 30

typedef int boolean;

typedef struct line {
  char command;
  int objnum;
  int quantity;
  int where;
} Line;

typedef struct DS {
  int NumLines;
  int HowOften;		/* number of seconds between placements */
  int LastPlacement;	/* time (second count) of last placement */
  Line file[MAXLINES];
} DropShipment;


D 2
boolean PutStuffInGame(DropShipment *data);
DropShipment *ReadStuffFile(char *filename);
E 2
I 2
boolean PutStuffInGame();
DropShipment *ReadStuffFile();
E 2
E 1
