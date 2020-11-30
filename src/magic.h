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

/* Header concerning magical items and powers */


/* count of how many storage variables there are for magical properties */
#define MAGICAL_SLOTS	7

/* codes for the slots */
#define SL_DETAIL	1
#define SL_X		2
#define SL_Y		4
#define SL_ZINGER	8
#define SL_EXTRA1	16
#define SL_EXTRA2	32
#define SL_EXTRA3	64

/* Code that means teleporting should be to same room player is already in */
#define SAME_ROOM	-100

/* Codes for magical powers */
#define MA_UNDEFINED	(-1)	/* magic property wasn't defined */
#define	MA_NOTHING	0	/* no magical property */
#define	MA_HEALTH	1	/* changes your health point total by (x),
				   if negative, you must have enough points */
#define MA_MAGIC	2	/* changes your magic point total by (x),
				   if negative, you must have enough points */
#define	MA_OWNPORT	3	/* teleports you to random own-team locale */
#define MA_TEAMPORT     4       /* teleports you to team number (zinger) */
#define MA_ANYPORT      5       /* teleports you to possibly any locale */
#define MA_ROOMPORT     6       /* teleports you to room given in (zinger) */
#define MA_POINTPORT    7       /* teleports you to a specific point in
				   room (extra1): x is (extra2), y (extra3) */
#define MA_SAMEPORT	8	/* teleports you to a random location in the
				   same room */
#define MA_OFFPORT	9	/* teleports you the same room, to a location
				   offset from the current one by
				   x (extra1), and y (extra2) */
#define MA_SPOTPORT	10	/* teleports you to spot specified by the
				   cursor in current room */
#define MA_RADIUSPORT   11	/* teleports you to a spot in current room
				   within a certain radius (zinger) */

#define MA_FISTS        15       /* raining fists of death spell, amplifies
				    the normal weapon effect of the object
				    used, or does fists if not a weapon */
