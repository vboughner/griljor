h35617
s 00000/00000/00113
d D 1.3 92/08/07 01:02:13 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00002/00001/00111
d D 1.2 92/05/27 21:45:40 vbo 2 1
c made minimal changes required for compile on Sparc2 SVR4
e
s 00112/00000/00000
d D 1.1 91/02/16 13:00:54 labc-3id 1 0
c date and time created 91/02/16 13:00:54 by labc-3id
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

/* Map routine header file */


typedef struct _mapinfo {
	char	name[MAP_NAME_LENGTH];
	char	team_name[NUM_OF_TEAMS][TEAM_NAME_LENGTH];
	short	teams_supported;	 /* how many teams are supported */
	char	objfilename[PATH_LENGTH];/* file name where obj file found */
D 2
	int	rooms;			 /* how many rooms are defined */
E 2
I 2
	char    dummy;
	short	rooms;			 /* how many rooms are defined */
E 2
	char	execute_file[PATH_LENGTH];	/* driver runs this one */
	char	startup_file[PATH_LENGTH];	/* random object startup */
	char	placement_file[PATH_LENGTH];	/* random objs during game */
	char	neutrals_allowed;	/* TRUE when neutral human players are
					   allowed in the game */
	char	pad[999];		/* space for later addition */
} MapInfo;


/* object records as stored in files */

typedef struct _objrecord {
	char		x, y;		/* location of recorded object */
	unsigned char	objtype;	/* type the object is */
	short		detail;		/* info about this character */
	short		infox, infoy;	/* more info for use by obj */
	short		zinger;		/* even more info storage */
	short		extra[3];	/* extra space for information */
} RecordedObj;


/* objects records as stored in memory */

typedef struct _memobj {
	RecordedObj	obj;	/* info about the recorded object */
	struct _memobj	*next;	/* next object in linked list */
} MemObj;


typedef struct _roominfo {
	char		name[ROOM_NAME_LENGTH];
	unsigned char	floor;		/* predominant floor object */
	unsigned char	team;		/* 0=neutral, 1=team A, 2=team B */
	short		exit[4];	/* North East South West exit rooms */
					/* when -1, cannot go that way */
	unsigned char	numobjs;	/* how many recorded objs in room */
	unsigned char	spot[ROOM_WIDTH][ROOM_HEIGHT][ROOM_DEPTH];/* squares */
	char		appearance;	/* concerns random item appearances */
	char		dark;		/* is TRUE when room is dark */
	char		pad[50];	/* for future use */
} RoomInfo;



/* external definitions of global variables */
#ifndef MAP_MAIN

extern MapInfo		mapstats;	/* info about loaded map */
extern RoomInfo		*room;		/* pointer to array of rooms in map */
extern MemObj		*roomrecobj;	/* pointer to room obj lists array */
extern short		current_room;	/* number of room we are in now */
extern char		mapfile[120], objectfile[120];
extern int		xdir[4];
extern int		ydir[4];

#endif


/* functions in map.c */

RoomInfo *make_room_array();
MemObj *make_room_obj_list_array();
MemObj *allocate_mem_obj();
MemObj *what_recorded_obj_here();
MemObj *nearby_exit_character();
MemObj *takeable_on_square();
unsigned char *whats_on_square();


/* Directions */

#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3


/* room lighting vales */
#define DARK	0
#define DAYLIT	1
#define LIT	2
#define GLOBAL	3

/* global means that you may see all of the room, but only the people in line
   of missile firing */
#define ROOM_IS_GLOBAL(r)	(room[r].dark == GLOBAL)

/* When a room is lit it is because it is either LIT all the time,
   or it is DAYLIT and now is daytime */
#define ROOM_IS_LIT(r)		(room[r].dark == LIT || room[r].dark == DAYLIT)

#define ROOM_IS_DARK(r)		(!ROOM_IS_LIT(r))
E 1
