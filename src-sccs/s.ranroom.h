h57002
s 00000/00000/00019
d D 1.5 92/08/07 01:03:43 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00002/00002/00017
d D 1.4 91/05/10 04:52:08 labc-3id 4 3
c 
e
s 00000/00000/00019
d D 1.3 91/05/03 23:11:10 labc-3id 3 2
c 
e
s 00000/00000/00019
d D 1.2 91/05/02 16:51:00 labc-3id 2 1
c Added by Trevor Pering
e
s 00019/00000/00000
d D 1.1 91/05/02 16:43:54 labc-3id 1 0
c date and time created 91/05/02 16:43:54 by labc-3id
e
u
U
f e 0
t
T
I 1
D 4
#define NEW_X_RAN  5
#define NEW_Y_RAN  5
E 4
I 4
#define NEW_X_RAN 5 
#define NEW_Y_RAN 2 
E 4

typedef struct _sqr
{
  int x1,x2,y1,y2,gx,gy;
  double nw,ne,sw,se;
} SquareDef;

typedef struct _prop
{
  double dismult,diffmult,ranadd;
} PropDef;

typedef struct _stat
{
  int (*treemark)[20],*terrmark;
  int range;
} StatDef;
E 1
