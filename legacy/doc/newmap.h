/***************************************************************************
 * %Z% %M% %G% - last change made %I%
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

/* Structures as stored in memory and the map files */

typedef struct _mapinfo {
	char	name[MAP_NAME_LENGTH];
	char	team_name[NUM_OF_TEAMS][TEAM_NAME_LENGTH];
	short	teams_supported;	/* how many teams are supported */
	char	objfilename[PATH_LENGTH];/* file name where obj file found */
	int	rooms;			 /* how many rooms are defined */
	char	execute_file[PATH_LENGTH];	/* driver runs this one,
						   it lists the monsters etc.
						   that are in the map. */
	char	startup_file[PATH_LENGTH];	/* random object startup */
	char	placement_file[PATH_LENGTH];	/* random objs during game */
	char	neutrals_allowed;	/* TRUE when neutral human players are
					   allowed in the game */

	/* -------- new entries (not yet subtracted from pad space ------ */
	char	only_one_allowed;	/* TRUE if only one instance of this
					   map is allowed to be running at
					   any particular time */
	char	msg_file[PATH_LENGTH];	/* message database file */
	char	save_file[PATH_LENGTH];	/* file where on ongoing version of
					   this map is stored */
	short	classes_supported;	/* number of classes this map has */
	short	class_names;		/* msg number in message file
					   where names of classes are
					   enumerated, along with pointers
					   to help message for each class */
	short	theme_message;		/* msg number in message file
					   where map's theme is stored
					   (zero means no message at all) */
	short	minimum_level;		/* minimum level player must be
					   to enter (or zero) */
	short	maximum_level;		/* maximum level player may be upon
					   entering map (or zero) */
	char	levels_loaded;		/* TRUE if levels gained/lost in this
					   map count permenantly, and levels
					   loaded upon entry count for
					   anything. */
	/* -------------------------------------------------------------- */

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
