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
/*	char	*id;	*/	/* missile id, (InvObj *) to object fired */
	short   heading;        /* the direction in degrees the missile 
				   was traveling.  0 = East */
	char    hurts_owner;    /* will this missile hurt its owner? */
	short   type;           /* the type of object */
	short	direction;	/* directional orientation (0-3,N,E,S,W) */
	short	wait;		/* speed missile travels, clicks per move */
	/* the following slots are used in memory to keep track of it */
	Location *moveq;	/* movement queue pointer */
	long	lastmove;	/* time (gameclicks) of last move */
	int     room;
	char	oldx, oldy;	/* last position */
	char	x, y;		/* current position in it's room */
	int     range;
} Missile;

typedef struct _multi_pack {
	Missile miss;
	char    x1[MAX_MULTI_MISSILES];
	char    x2[MAX_MULTI_MISSILES];
	char    y1[MAX_MULTI_MISSILES];
	char    y2[MAX_MULTI_MISSILES];
	int	number;
	char	include_start;
} MultiPack;

Missile *TopOfHeap();
void AddMissileToHeap();
void RemoveTopOfHeap();

/* a constant for doing some division of missile speed */
#define BIGPOWER        256   
