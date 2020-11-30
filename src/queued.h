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

/* Header file for queued drawing system */

/* various ways a square may be called for drawing */
#define BLACKED		0	/* erase the square completely */
#define CONCEALED	1	/* draw objects there you may see from afar */
#define UNCONCEALED	2	/* draw what you would see on an adjacent */
#define EVERYTHING	3	/* draw what you would see on your own */


/* queue data structure for holding draw requests in order they arrive */
typedef struct _qdraw {
	int	room; /* the room the square to draw is in */
	int	x;    /* x location of square in room */
	int	y;    /* y location of square in room */
	int	way;  /* one of BLACKED, CONCEALED, UNCONCEALED, EVERYTHING */
	struct _qdraw *next;
	struct _qdraw *plink;
} QDraw;

QDraw *create_QDraw();

