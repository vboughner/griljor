h58287
s 00000/00000/00021
d D 1.3 92/08/07 01:03:59 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00000/00000/00021
d D 1.2 91/02/18 16:45:32 labc-3id 2 1
c 
e
s 00021/00000/00000
d D 1.1 91/02/16 13:01:23 labc-3id 1 0
c date and time created 91/02/16 13:01:23 by labc-3id
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

/* Rules for use by ray.c */

typedef struct _rulesquare {
	int lr, mr;
} RuleSquare;

extern struct raypack raypack[ROOM_WIDTH+1][ROOM_HEIGHT+1];
extern RuleSquare rulepack[ROOM_WIDTH][ROOM_HEIGHT];


E 1
