h32888
s 00000/00000/00055
d D 1.4 92/08/07 01:02:10 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00018/00004/00037
d D 1.3 91/05/16 00:34:44 labc-3id 3 2
c added more teleporter types
e
s 00012/00003/00029
d D 1.2 91/05/15 05:06:26 labc-3id 2 1
c added a few more definitions
e
s 00032/00000/00000
d D 1.1 91/02/16 13:00:52 labc-3id 1 0
c date and time created 91/02/16 13:00:52 by labc-3id
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

I 3
/* Code that means teleporting should be to same room player is already in */
#define SAME_ROOM	-100

E 3
/* Codes for magical powers */
#define MA_UNDEFINED	(-1)	/* magic property wasn't defined */
#define	MA_NOTHING	0	/* no magical property */
D 2
#define	MA_HEALTH	1	/* changes your health point total */
#define MA_MAGIC	2	/* changes your magic point total */
#define	MA_RANDPORT	3	/* teleports you to random locale */
E 2
I 2
#define	MA_HEALTH	1	/* changes your health point total by (x),
				   if negative, you must have enough points */
#define MA_MAGIC	2	/* changes your magic point total by (x),
				   if negative, you must have enough points */
#define	MA_OWNPORT	3	/* teleports you to random own-team locale */
D 3
#define MA_TEAMPORT     4       /* teleports you to team number (extra1) */
E 3
I 3
#define MA_TEAMPORT     4       /* teleports you to team number (zinger) */
E 3
#define MA_ANYPORT      5       /* teleports you to possibly any locale */
D 3
#define MA_ROOMPORT     6       /* teleports you to room given in (extra1) */
E 3
I 3
#define MA_ROOMPORT     6       /* teleports you to room given in (zinger) */
E 3
#define MA_POINTPORT    7       /* teleports you to a specific point in
				   room (extra1): x is (extra2), y (extra3) */
D 3
#define MA_FISTS        8       /* raining fists of death spell, object to
				   use as rain given in (zinger) */
E 3
I 3
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
E 3
E 2
E 1
