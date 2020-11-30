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
Hit *hit_on_square();
Hit *alloc_hit();
