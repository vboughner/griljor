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
/* Extensively redone, February 1992 */


#define OL_NOWHERE	0		/* object location is not known */
#define OL_IN_MAP	1		/* object is on the map */
#define OL_ON_PERSON	2		/* object is with a person */

#define OBJECT_HASH_TABLE_SIZE	5000	/* number of buckets in the table */



typedef struct _obj_record {
	short		detail;		/* info about this character */
	short		infox, infoy;	/* more info for use by obj */
	short		zinger;		/* even more info storage */
	short		extra[3];	/* extra space for information */
	char		dirty;		/* TRUE if something here's changed */
} RecordedObj;



typedef struct _map_location {
	short		type;		/* type of location, eg. OL_IN_MAP */
	short		room;		/* room that the object is in */
	short		x, y;		/* x and y location in the room */
} MapLocation;


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



typedef struct _object_instance {
/* add new additions to MapObject as well, for saving in the file */
	short		type;		/* type number of the object */
	long		id;		/* unique id for object instance */
	long		*space;		/* variable space allocated for obj */
	RecordedObj	*record;	/* parameters for each instance */
	ObjLocation	*location;	/* pointer to location information */
	ObjInfo		*info;		/* instance's override info */
	long		contained_id;	/* id of 1st object contained by me */
	long		container_id;	/* id of object containing me */
	long		lsibling_id;	/* id of prior contained with me */
	long		rsibling_id;	/* id of next contained with me */
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


typedef struct _roominfo {
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
	short		exit[4];	/* North East South West exit rooms */
					/* when -1, cannot go that way */
	SquareRec	*square[ROOM_WIDTH][ROOM_HEIGHT];   /* squares */
} RoomInfo;


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



typedef ObjectInstance OI;



typedef struct _map_object {
/* type definition of object when being saved to file */
	short		type;		/* object type */
	long		id;		/* object id number */
	long		contained_id;	/* id of 1st object contained by me */
	long		container_id;	/* id of object containing me */
	long		lsibling_id;	/* id of prior contained with me */
	long		rsibling_id;	/* id of next contained with me */
	RecordedObj	rec;		/* recorded object slots */
	MapLocation	loc;		/* objects always in map in file */
	char		info;		/* in there instance flaggage */
} MapObject;



/* external definitions of global variables */
#ifndef MAP_MAIN

extern MapInfo		mapstats;	/* info about loaded map */
extern RoomInfo		*room;		/* pointer to array of rooms in map */
extern short		current_room;	/* number of room we are in now */
extern char		mapfile[120], objectfile[120];
extern int		xdir[4];
extern int		ydir[4];

#endif



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
