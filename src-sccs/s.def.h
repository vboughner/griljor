h54915
s 00000/00000/00129
d D 1.9 92/08/07 01:00:41 vbo 9 8
c source copied to a separate tree for work on new map and object format
e
s 00004/00000/00125
d D 1.8 92/05/27 22:05:10 vbo 8 7
c put back MAX and MIN which shouldn't have been taken out
e
s 00000/00021/00125
d D 1.7 92/05/27 21:45:33 vbo 7 6
c made minimal changes required for compile on Sparc2 SVR4
e
s 00004/00000/00142
d D 1.6 91/12/07 14:18:14 labc-4lc 6 5
c moved a macro constant from objstore.c to def.h
e
s 00000/00000/00142
d D 1.5 91/11/29 16:09:05 labc-4lc 5 4
c finished making entry of hidden text properties in obtor
e
s 00004/00007/00138
d D 1.4 91/04/17 16:56:54 labc-3id 4 3
c added driver_forever flag to player info structure
e
s 00001/00001/00144
d D 1.3 91/04/14 23:07:17 labc-3id 3 2
c 
e
s 00005/00000/00140
d D 1.2 91/04/14 11:43:29 labc-3id 2 1
c integrated raycode
e
s 00140/00000/00000
d D 1.1 91/02/16 13:00:22 labc-3id 1 0
c date and time created 91/02/16 13:00:22 by labc-3id
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


#include "config.h"

D 4
/* Global Definition File */
E 4
I 4
/* Global Definition File
   This file is included by almost every C file in griljordom */
E 4

D 4
/* data types */

E 4
typedef	char	Boolean;

D 4

/* definitions */

E 4
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

/* play modes */
#define MODE_PLAY	1
#define MODE_HELP	2
#define MODE_STATS	3

/* type of program running */
#define DRIVER			0
#define PLAYER			1
#define MONSTER			2
#define am_driver		(what_am_i == DRIVER)
#define am_player		(what_am_i == PLAYER)
#define am_monster		(what_am_i == MONSTER)
#define	has_display		(am_player || use_display)

/* mode of display and input (of use to players only) */
#define QUITTING		0	/* person is quitting */
#define DYING			1	/* person has died, looks dead too */
#define NORMAL			2


D 4
/* appearance of a person (what they look like), in playerX10.c this
E 4
I 4
/* appearance of a person (what they look like), in playerX11.c this
E 4
   number defines which pixmap to use for drawing the persons new look */
#define NUM_APPEARANCES		5	/* count of special appearances */
#define APPEAR_QUITTING		0	/* you see "pop" quitting bitmap */
#define APPEAR_DYING		1	/* you see tombstone death bitmap */
#define APPEAR_INVISIBLE	2	/* you see nothing */
#define	APPEAR_LOST		3	/* you see loser of the game */
#define APPEAR_WON		4	/* you see winner of the game */
#define APPEAR_NORMAL		5	/* you see their normal bitmap,
					   this constant must be highest num */

/* constants concerning when and where you can place "random" items and
   persons after they die and need a place to start */
#define PLACE_OVERRIDE		(-2)	/* for placing in PLACE_NEVER rooms */
#define PLACE_DEFAULT		(-1)	/* do the right thing for this room */
#define PLACE_NEVER		0	/* cannot drop people in this room */
#define PLACE_FLOOR_ONLY	1	/* may drop them on designated floor */
#define PLACE_ANYWHERE		2	/* put them anywhere in this room */

/* constant that signifies that this kind of file is not needed */
#define NO_FILE			"none"


/* data structures */

typedef struct _player_info {
	short	play_mode;	/* MODE_PLAY or MODE_HELP or MODE_STATS */
	char	player_name[NAME_LENGTH];
	char	password[PASSWORD_LENGTH];
	char	login[LOGIN_LENGTH];		/* player's login name */
	char	host[HOST_LENGTH];		/* host that player is from */
	char	our_game[NAME_LENGTH];		/* the game we're joining */
	char	map_file[PATH_LENGTH];		/* path to main map def file */
	char	password_file[PATH_LENGTH];	/* path to password file */
	char	gamelist_file[PATH_LENGTH];	/* path to gamelist file */
	char	variable_file[PATH_LENGTH];	/* path to a monster file */
	char	personal_bitmap[PATH_LENGTH];	/* filename of bitmap */
	char	bitmap_mask[PATH_LENGTH];	/* filename of bitmap mask */
	int	out_to_display;			/* TRUE when you use display */
						/* used for monsters only */
	int	reversed;			/* TRUE for display reversed */
	int	team;				/* which team player is on */
I 4
	int	driver_forever;			/* TRUE for no-quit driver */
E 4
} PlayerInfo;


/* linked-list movement queue structure*/

typedef struct _location {
  char			x, y;	/* position in room */
  struct _location	*next;	/* net record in list */
} Location;


/* procedure headers */
char *query_personality();
char *get_team_name();


/* demand macro for testing memory allocations */
#define demand(test,errmsg)						\
{									\
  if (!(test)) {							\
    printf("%s\n",errmsg);						\
    exit(1);								\
  }									\
}


I 8
#define MAX(a,b)  ( (a > b) ? (a) : (b) )
#define MIN(a,b)  ( (a < b) ? (a) : (b) )


E 8
/* macro return TRUE when map location given is out of bounds */
#define OUT_OF_BOUNDS(roomnum, x, y)					\
	((roomnum) < 0 || (roomnum) >= mapstats.rooms ||		\
	 (x) < 0 || (y) < 0 || (x) >= ROOM_WIDTH || (y) >= ROOM_HEIGHT)

/* additional type definition */
typedef unsigned char	uc;


D 7
/*****************************************************************/

#define RESIZE(x,s,m) (((x) \
			? ((x) = (typeof(x)) realloc((x),(s)*sizeof(*(x)))) \
			: ((x) = (typeof(x)) malloc((s)*sizeof(*(x)))) ) ? \
		       (x) : ((typeof(x)) Gerror(m)))

#define INIT(x,m) ( ((int) ((x) = (typeof((x))) malloc(sizeof(*(x))))) ? \
		    (x) : ((typeof(x)) Gerror(m)) )

I 2
#define AINIT(x, i) { \
D 3
  (x) = (typeof(x))malloc(i * sizeof(typeof(*(x)))); \
E 3
I 3
  (x) = (typeof(x))malloc((i) * sizeof(typeof(*(x)))); \
E 3
  if (!(x)) demand(FALSE, "Memory Allocation Failed");\
}

E 2
#define MAX(a,b)  ( (a > b) ? (a) : (b) )
#define MIN(a,b)  ( (a < b) ? (a) : (b) )

/*****************************************************************/

I 6

E 7
/* buffer size of maximum size string that can be written out or read in
   for a textual property in an object */
#define STRING_BUFFER_SIZE	20000
E 6
E 1
