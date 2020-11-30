h13596
s 00000/00000/00025
d D 1.3 92/08/07 01:01:39 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00001/00001/00024
d D 1.2 91/08/29 01:40:01 vanb 2 1
c fixed up more compatibility problems
e
s 00025/00000/00000
d D 1.1 91/02/16 13:00:40 labc-3id 1 0
c date and time created 91/02/16 13:00:40 by labc-3id
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
 * October 1989
 **************************************************************************/

/* Definitions for hits */

/* structure to hold information about a hit registered inside a room */

typedef struct _hit {
	char		x, y;	/* where it happened */
	long		timer;	/* game click when hit should be erased */
	struct _hit	*next;	/* next hit in list of this room's hits */
} Hit;


/* Procedure declarations */
D 2
Hit *hit_on_square(int roomnum, int x, int y);
E 2
I 2
Hit *hit_on_square();
E 2
Hit *alloc_hit();
E 1
