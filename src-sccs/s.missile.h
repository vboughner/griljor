h13635
s 00000/00000/00048
d D 1.8 92/08/07 01:02:28 vbo 8 7
c source copied to a separate tree for work on new map and object format
e
s 00010/00000/00038
d D 1.7 92/01/20 16:30:00 labc-4lc 7 6
c blojo finished changes necesssary for network packets
e
s 00001/00000/00037
d D 1.6 91/12/15 23:07:09 labc-4lc 6 5
c Fixing datagram incompatibilities.   -bloo
e
s 00001/00001/00036
d D 1.5 91/08/26 00:33:38 vanb 5 4
c fixed up procedure defs and other compatibilty problems
e
s 00004/00003/00033
d D 1.4 91/05/26 22:43:44 labc-3id 4 3
c Worked on drawing improvements
e
s 00000/00000/00036
d D 1.3 91/05/20 23:53:45 labc-3id 3 2
c T: fixed all sorts of missle bugs and what not...
e
s 00003/00002/00033
d D 1.2 91/02/23 01:49:25 labc-3id 2 1
c 
e
s 00035/00000/00000
d D 1.1 91/02/16 13:00:58 labc-3id 1 0
c date and time created 91/02/16 13:00:58 by labc-3id
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

/* Missile type definitions */

typedef struct _missile {
	/* these slots provided essential packet sending information */
        short	owner;		/* person who fired the missile */
D 2
	char	*id;		/* missile id, (InvObj *) to object fired */
	char	special;	/* TRUE when this has been specially defined */
E 2
I 2
/*	char	*id;	*/	/* missile id, (InvObj *) to object fired */
	short   heading;        /* the direction in degrees the missile 
				   was traveling.  0 = East */
E 2
	char    hurts_owner;    /* will this missile hurt its owner? */
	short   type;           /* the type of object */
	short	direction;	/* directional orientation (0-3,N,E,S,W) */
	short	wait;		/* speed missile travels, clicks per move */
	/* the following slots are used in memory to keep track of it */
	Location *moveq;	/* movement queue pointer */
D 4
	struct _missile *next;	/* next missile pointer (used in mem use) */
E 4
	long	lastmove;	/* time (gameclicks) of last move */
I 4
	int     room;
E 4
	char	oldx, oldy;	/* last position */
	char	x, y;		/* current position in it's room */
I 6
	int     range;
E 6
} Missile;

I 7
typedef struct _multi_pack {
	Missile miss;
	char    x1[MAX_MULTI_MISSILES];
	char    x2[MAX_MULTI_MISSILES];
	char    y1[MAX_MULTI_MISSILES];
	char    y2[MAX_MULTI_MISSILES];
	int	number;
	char	include_start;
} MultiPack;

E 7
D 4
/* a list of all of the missiles in a room */
extern Missile *room_missiles;
E 4
I 4
Missile *TopOfHeap();
D 5
void AddMissileToHeap(Missile *add);
E 5
I 5
void AddMissileToHeap();
E 5
void RemoveTopOfHeap();
E 4

/* a constant for doing some division of missile speed */
#define BIGPOWER        256   
E 1
