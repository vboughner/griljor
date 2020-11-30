h13546
s 00000/00000/00200
d D 1.6 92/08/07 21:12:05 vbo 6 5
c fixes for sun port merged in
e
s 00009/00000/00191
d D 1.5 92/03/11 20:52:59 vanb 5 4
c added to structures for containers, fixed more bugs
e
s 00014/00014/00177
d D 1.4 92/03/02 22:07:15 vanb 4 3
c finished getting editmap to work on basics
e
s 00012/00000/00179
d D 1.3 92/02/20 21:58:34 vanb 3 2
c 
e
s 00106/00039/00073
d D 1.2 92/02/15 01:08:14 vanb 2 1
c major changes to map structures
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
I 2
/* Extensively redone, February 1992 */
E 2


D 2
typedef struct _mapinfo {
	char	name[MAP_NAME_LENGTH];
	char	team_name[NUM_OF_TEAMS][TEAM_NAME_LENGTH];
	short	teams_supported;	 /* how many teams are supported */
	char	objfilename[PATH_LENGTH];/* file name where obj file found */
	int	rooms;			 /* how many rooms are defined */
	char	execute_file[PATH_LENGTH];	/* driver runs this one */
	char	startup_file[PATH_LENGTH];	/* random object startup */
	char	placement_file[PATH_LENGTH];	/* random objs during game */
	char	neutrals_allowed;	/* TRUE when neutral human players are
					   allowed in the game */
	char	pad[999];		/* space for later addition */
} MapInfo;
E 2
I 2
#define OL_NOWHERE	0		/* object location is not known */
#define OL_IN_MAP	1		/* object is on the map */
#define OL_ON_PERSON	2		/* object is with a person */
E 2

I 2
#define OBJECT_HASH_TABLE_SIZE	5000	/* number of buckets in the table */
E 2

D 2
/* object records as stored in files */
E 2

D 2
typedef struct _objrecord {
	char		x, y;		/* location of recorded object */
	unsigned char	objtype;	/* type the object is */
E 2
I 2

typedef struct _obj_record {
D 4
/* object records as stored in files */
E 4
E 2
	short		detail;		/* info about this character */
	short		infox, infoy;	/* more info for use by obj */
	short		zinger;		/* even more info storage */
	short		extra[3];	/* extra space for information */
I 2
	char		dirty;		/* TRUE if something here's changed */
E 2
} RecordedObj;


D 2
/* objects records as stored in memory */
E 2

D 2
typedef struct _memobj {
	RecordedObj	obj;	/* info about the recorded object */
	struct _memobj	*next;	/* next object in linked list */
} MemObj;
E 2
I 2
typedef struct _map_location {
	short		type;		/* type of location, eg. OL_IN_MAP */
	short		room;		/* room that the object is in */
D 4
	char		x, y;		/* x and y location in the room */
E 4
I 4
	short		x, y;		/* x and y location in the room */
E 4
} MapLocation;
E 2


I 2
typedef struct _person_location {
	short		type;		/* type of location object is in */
	short		id;		/* id number of person holding it */
	char		knowledge;	/* TRUE if we know where on person */
	char		in_hand;	/* TRUE if in one of the hands */
	char		index;		/* hand number or inventory number */
} PersonLocation;


typedef union _obj_location {
	short		type;		/* type of location object is in */
	MapLocation	maploc;		/* on the map somewhere */
	PersonLocation	personloc;	/* on a person */
} ObjLocation;



I 3
D 4
typedef struct _object_bitmaps {
/* store special bitmaps in the case of a particular instance having
   it own special look, like players, for example. */
	char *bitmap;			/* array of bits in bitmap */
	char *bitmask;			/* array of bits in mask */
	long pixmap;			/* id number of pixmap */
	long pixmask;			/* and pixmap'd mask too */
} ObjBitmaps;



E 4
E 3
typedef struct _object_instance {
I 5
/* add new additions to MapObject as well, for saving in the file */
E 5
	short		type;		/* type number of the object */
	long		id;		/* unique id for object instance */
	long		*space;		/* variable space allocated for obj */
	RecordedObj	*record;	/* parameters for each instance */
	ObjLocation	*location;	/* pointer to location information */
I 3
D 4
	ObjBitmaps	*bitmaps;	/* pointer to instance bitmaps */
E 4
I 4
	ObjInfo		*info;		/* instance's override info */
I 5
	long		contained_id;	/* id of 1st object contained by me */
	long		container_id;	/* id of object containing me */
	long		lsibling_id;	/* id of prior contained with me */
	long		rsibling_id;	/* id of next contained with me */
E 5
E 4
E 3
	struct _object_instance *prev;	/* doubly linked list pointers */
	struct _object_instance *next;
} ObjectInstance;



typedef struct _object_hash {
/* structure used in linked list that forms the buckets in a hash table. */
	ObjectInstance *obj;
	struct _object_hash *next;
} ObjectHash;

typedef ObjectHash* ObjectHashP;


typedef struct _square_rec {
  	char		seen;		/* TRUE if player has seen square */
	char		view_blocked;	/* TRUE if something here blocks */
	short		number;		/* number of objects on this square */
	ObjectInstance	*first;		/* first object in list */
	ObjectInstance	*last;		/* last object in list */
} SquareRec;


E 2
typedef struct _roominfo {
D 2
	char		name[ROOM_NAME_LENGTH];
	unsigned char	floor;		/* predominant floor object */
	unsigned char	team;		/* 0=neutral, 1=team A, 2=team B */
E 2
I 2
	char		*name;		/* name of the room */
	short		floor;		/* predominantly drawn floor object */
	short		team;		/* owner: 0=neutral, 1=A, 2=B... */
	short		object_floor;	/* floor on which objects appear */
	short		people_floor;	/* floor on which people appear */
	char		objects_appear;	/* TRUE when objects may appear here */
	char		people_appear;	/* True when people may appear here */
	char		dark;		/* TRUE when room is dark */
	char		cycles;		/* TRUE if room follows light cycle */
	char		limited_sight;	/* TRUE if you don't see whole room */
E 2
	short		exit[4];	/* North East South West exit rooms */
					/* when -1, cannot go that way */
D 2
	unsigned char	numobjs;	/* how many recorded objs in room */
	unsigned char	spot[ROOM_WIDTH][ROOM_HEIGHT][ROOM_DEPTH];/* squares */
	char		appearance;	/* concerns random item appearances */
	char		dark;		/* is TRUE when room is dark */
	char		pad[50];	/* for future use */
E 2
I 2
	SquareRec	*square[ROOM_WIDTH][ROOM_HEIGHT];   /* squares */
E 2
} RoomInfo;


I 2
typedef struct _mapinfo {
	char	*name;
	char	*objfilename;		/* file name where obj file found */
	char	*execute_file;	 	/* driver runs this one */
	char	*startup_file;		/* random object startup */
	char	*placement_file;       	/* random objs during game */
	char	*team_name[NUM_OF_TEAMS+1];
	char	neutrals_allowed;	/* TRUE if neutral humans allowed */
	short	teams_supported;	/* how many teams are supported */
	short	rooms;			/* how many rooms are defined */
	RoomInfo *room;			/* array of rooms in the map */
	long	next_id;		/* next available object id number */
	short	table_size;		/* size of this hash table */
	ObjectHashP *hash_table;	/* pointer to array of pointers */
} MapInfo;
E 2

I 2


typedef ObjectInstance OI;
I 4



typedef struct _map_object {
/* type definition of object when being saved to file */
	short		type;		/* object type */
	long		id;		/* object id number */
I 5
	long		contained_id;	/* id of 1st object contained by me */
	long		container_id;	/* id of object containing me */
	long		lsibling_id;	/* id of prior contained with me */
	long		rsibling_id;	/* id of next contained with me */
E 5
	RecordedObj	rec;		/* recorded object slots */
	MapLocation	loc;		/* objects always in map in file */
	char		info;		/* in there instance flaggage */
} MapObject;

E 4


E 2
/* external definitions of global variables */
#ifndef MAP_MAIN

extern MapInfo		mapstats;	/* info about loaded map */
extern RoomInfo		*room;		/* pointer to array of rooms in map */
D 2
extern MemObj		*roomrecobj;	/* pointer to room obj lists array */
E 2
extern short		current_room;	/* number of room we are in now */
extern char		mapfile[120], objectfile[120];
extern int		xdir[4];
extern int		ydir[4];

#endif


D 2
/* functions in map.c */
E 2

D 2
RoomInfo *make_room_array();
MemObj *make_room_obj_list_array();
MemObj *allocate_mem_obj();
MemObj *what_recorded_obj_here();
MemObj *nearby_exit_character();
MemObj *takeable_on_square();
unsigned char *whats_on_square();
E 2
I 2
void load_map();
void create_new_map();
void save_map();
void add_new_room();
void free_map();
void put_down_object();
int has_view_obstruction();
int most_prominant_on_square();
int probable_floor_here();
int max_move_on_square();
int may_fire_onto_square();
int may_fire_through_square();
int new_room_for_refire();

E 2


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
