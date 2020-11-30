h22972
s 00000/00000/00032
d D 1.6 92/08/07 01:03:40 vbo 6 5
c source copied to a separate tree for work on new map and object format
e
s 00003/00013/00029
d D 1.5 91/09/03 21:39:24 labb-3li 5 4
c Trevor's misc changes - fixed missile heap
e
s 00002/00000/00040
d D 1.4 91/05/27 14:35:41 labc-3id 4 3
c Made priority requests replace each other
e
s 00000/00000/00040
d D 1.3 91/05/26 22:43:49 labc-3id 3 2
c Worked on drawing improvements
e
s 00000/00000/00040
d D 1.2 91/05/10 04:50:03 labc-3id 2 1
c 
e
s 00040/00000/00000
d D 1.1 91/02/16 13:01:19 labc-3id 1 0
c date and time created 91/02/16 13:01:19 by labc-3id
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
I 4
D 5
	int     prio; /* 0= low pirority, 1= high priority */
E 4
	struct _qdraw *prev, *next;	/* doubly linked */
E 5
I 5
	struct _qdraw *next;
	struct _qdraw *plink;
E 5
} QDraw;

D 5

/* data structure for making linked lists of pointers to QDraw structures */
typedef struct _qpoint {
	QDraw	*ptr;
	struct _qpoint *next;
} QPoint;


QDraw *prior_request_in_array();
E 5
QDraw *create_QDraw();
D 5
QPoint *create_QPoint();
I 4
QDraw *extract_request();
E 5
I 5

E 5
E 4
E 1
