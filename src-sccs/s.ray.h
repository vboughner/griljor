h61037
s 00000/00000/00043
d D 1.5 92/08/07 01:03:27 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00001/00001/00042
d D 1.4 91/04/14 11:56:17 labc-3id 4 3
c replaced oops.
e
s 00002/00005/00041
d D 1.3 91/04/14 11:42:34 labc-3id 3 2
c Fixed etrnal declarations.
e
s 00017/00006/00029
d D 1.2 91/02/18 21:13:58 labc-3id 2 1
c 
e
s 00035/00000/00000
d D 1.1 91/02/16 13:01:21 labc-3id 1 0
c date and time created 91/02/16 13:01:21 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * February 1991
 **************************************************************************/

I 2
#define HFLAG(room, x, y) 1
#define VFLAG(room, x, y) 1
#define HSET(x, y) 0
#define VSET(x, y) 0
#define UNHSET(x, y) 0
#define UNVSET(x, y) 0


E 2
/* header file for code to handle line of sight vision for players */
/* you gotta love it! */
/* comments you say?  You're lucky even to have this code! */

struct raypack {
  int botego;
  int ritego;
  int botlit;
  int ritlit;
  int drawn;
  int lre;
  int lrl;
  int mre;
  int mrl;
  int hflag;
  int vflag;
};

D 2
#define HFLAG(room, x, y) 1
#define VFLAG(room, x, y) 1
#define HSET(x, y) 0
#define VSET(x, y) 0
#define UNHSET(x, y) 0
#define UNVSET(x, y) 0
E 2
I 2
typedef struct _rulesquare {
	int lr, mr;
} RuleSquare;

D 3
extern struct raypack raypack[ROOM_WIDTH+1][ROOM_HEIGHT+1];
extern RuleSquare rulepack[ROOM_WIDTH][ROOM_HEIGHT];



E 3
I 3
extern struct raypack **raypack;
D 4
extern RuleSquare **rulepack;
E 4
I 4
extern RuleSquare rulepack[ROOM_WIDTH][ROOM_HEIGHT];
E 4
E 3
E 2
E 1
