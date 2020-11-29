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

/* Map header file */


#define NEW(n, type, size) \
	demand(((n) = (type) malloc((size))), "no memory available")

#define NEW_CLEAR(n, type, size) \
	{ \
	  NEW(n, type, size); \
	  bzero(n, size); \
	}



typedef struct _floorlocation {
	short	room;
	short	x;
	short	y;
} FloorLocation;



typedef struct _personlocation {
	short	person_id;	/* unique person id number */
	short	in_hand;	/* TRUE: object in hand, FALSE: in inventory */
	short	hand_or_slot_number;
} PersonLocation;



typedef struct _instancelocation {
/* the location of objects is either at a (room, x, y) or on a person */
	char	on_a_person;	/* is TRUE when object is on a person */
        union _actuallocation {
	  FloorLocation in_room;
	  PersonLocation on_person;
	} specific;
} InstanceLocation;


#define IL_on_a_person(l)		((l)->on_a_person)
#define IL_person_id(l)			((l)->on_person.person_id)
#define IL_in_hand(l)			((l)->on_person.in_hand)
#define IL_hand_or_slot_number(l)	((l)->on_person.hand_or_slot_number)
#define IL_room(l)			((l)->in_room.room)
#define IL_x(l)				((l)->in_room.x)
#define IL_y(l)				((l)->in_room.y)

#define SET_IL_on_a_person(l, v)	((l)->on_a_person = (v))
#define SET_IL_person_id(l, v)		((l)->on_person.person_id = (v))
#define SET_IL_in_hand(l, v)		((l)->on_person.in_hand = (v))
#define SET_IL_hand_or_slot_number(l, v) \
				((l)->on_person.hand_or_slot_number = (v))
#define SET_IL_room(l, v)		((l)->in_room.room = (v))
#define SET_IL_x(l, v)			((l)->in_room.x = (v))
#define SET_IL_y(l, v)			((l)->in_room.y = (v))




typedef struct _objinstance {
/* every instance of any kind of object will have this structure */
	short	type;		/* type of object */
	long	id;		/* unique id number of object instance */
	short	*recorded;	/* variables settable in map editor */
	long	*variables;	/* static variable space for forth code */
	InstanceLocation *loc;	/* location of the object */
	struct _objinstance *next;	/* used for hash table on id numbers */
} ObjInstance;


#define OI_type(i)		((i)->type)
#define OI_id(i)		((i)->id)
#define OI_recorded(i, n)	((i)->recorded[(n)])
#define OI_variables(i, n)	((i)->variables[(n)])
#define OI_loc(i)		((i)->loc)
#define OI_next(i)		((i)->next)
#define OI_on_a_person(i)		((OI_loc(i))->on_a_person)
#define OI_person_id(i)			((OI_loc(i))->on_person.person_id)
#define OI_in_hand(i)		        ((OI_loc(i))->on_person.in_hand)
#define OI_hand_or_slot_number(i)  ((OI_loc(i))->on_person.hand_or_slot_number)
#define OI_room(i)			((OI_loc(i))->in_room.room)
#define OI_x(i)				((OI_loc(i))->in_room.x)
#define OI_y(i)				((OI_loc(i))->in_room.y)

#define FREE_OI(x)			(if (x) free(x))

#define SET_OI_type(i, v)		((i)->type = (v))
#define SET_OI_id(i, v)			((i)->id = (v))
#define SET_OI_recorded(i, n, v)	{ \
					  ((i)->recorded[(n)] = (v)); }
#define SET_OI_variables(i, n, v)	{ \
					  ((i)->variables[(n)] = (v)); }
#define SET_OI_loc(i, v)		{ FREE_OI((i)->loc); \
					  ((i)->loc = (v)); }
#define SET_OI_next(i, v)		((i)->next = (v))
#define SET_OI_on_a_person(i, v)	SET_IL_on_a_person(OI_loc(i), v)
#define SET_OI_person_id(i, v)		SET_IL_person_id(OI_loc(i), v)
#define SET_OI_in_hand(i, v)		SET_IL_in_hand(OI_loc(i), v)
#define SET_OI_hand_or_slot_number(i,v)	SET_IL_hand_or_slot_number(OI_loc(i),v)
#define SET_OI_room(i, v)		SET_IL_room(OI_loc(i), v)
#define SET_OI_x(i, v)			SET_IL_x(OI_loc(i), v)
#define SET_OI_y(i, v)			SET_IL_y(OI_loc(i), v)



typedef struct _squarelink {
/* every square has a linked list of objects on the square */
	ObjInstance *object;	/* pointer to object instance */
	struct _squarelink *next;
} SquareLink;


#define SL_object(s)		((s)->object)
#define SL_next(s)		((s)->next)

#define SET_SL_object(s, v)	((s)->object = (v))
#define SET_SL_next(s, v)	((s)->next = (v))



typedef struct _squareinfo {
/* stores a little info about a square, and linked list of objects there */
	SquareLink *first;	/* first object in square */
	SquareLink *last;	/* last object in linked list */
	short	quantity;	/* how many objects on square */
	/* insert future things about whether square is drawn here */
} SquareInfo;


#define SI_first(s)		((s)->first)
#define SI_last(s)		((s)->last)
#define SI_quantity(s)		((s)->quantity)

#define SET_SI_first(s, v)	((s)->first = (v))
#define SET_SI_last(s, v)	((s)->last = (v))
#define SET_SI_quantity(s, v)	((s)->quantity = (v))
#define INC_SI_quantity(s)	((s)->quantity++)
#define DEC_SI_quantity(s)	((s)->quantity--)



typedef struct _roominfo {
/* every room has one of these structure on info about the room */
	char		*name;
	short	        floor;		/* predominant floor object */
	short    	team;		/* 0=neutral, 1=team A, 2=team B */
	short		exit[4];	/* North East South West exit rooms */
					/* when -1, cannot go that way */
        char            objects_may_appear;
	char            people_may_appear;
        short           where_objects_appear;
        short           where_people_appear;
	char            see_whole_room;
	char		see_all_people;
	char            daylight_will_cycle;
	char            currently_dark  /* is TRUE when room is/starts dark */
	SquareInfo      *square[ROOM_WIDTH][ROOM_HEIGHT];  /* squares */
} RoomInfo;


#define RI_name(r)			((r)->name)
#define RI_floor(r)			((r)->floor)
#define RI_team(r)			((r)->team)
#define RI_exit(r, n)			((r)->exit[(n)])
#define RI_objects_may_appear(r)	((r)->objects_may_appear)
#define RI_people_may_appear(r)		((r)->people_may_appear)
#define RI_where_objects_appear(r)	((r)->where_objects_appear)
#define RI_where_people_appear(r)	((r)->where_people_appear)
#define RI_see_whole_room(r)		((r)->see_whole_room)
#define RI_see_all_people(r)		((r)->see_all_people)
#define RI_daylight_will_cycle(r)	((r)->daylight_will_cycle)
#define RI_currently_dark(r)		((r)->currently_dark)
#define RI_square(r, x, y)		((r)->square[(x)][(y)])

#define FREE_RI(x)			(if (x) free(x))

#define SET_RI_name(r, v)		{ FREE_RI((r)->name); \
					  (r)->name = (v); }
#define SET_RI_floor(r, v)		{ \
					  (r)->floor = (v); }
#define SET_RI_team(r, v)		{ \
					  (r)->team = (v); }
#define SET_RI_exit(r, n, v)		{ \
					  (r)->exit[(n)] = (v); }
#define SET_RI_objects_may_appear(r, v)	{ \
					  (r)->objects_may_appear = (v); }
#define SET_RI_people_may_appear(r, v)	{ \
					  (r)->people_may_appear = (v); }
#define SET_RI_where_objects_appear(r, v) { \
					  (r)->where_objects_appear = (v); }
#define SET_RI_where_people_appear(r, v)  { \
					  (r)->where_people_appear = (v); }
#define SET_RI_see_whole_room(r, v)	{ \
					  (r)->see_whole_room = (v); }
#define SET_RI_see_all_people(r, v)	{ \
					  (r)->see_all_people = (v); }
#define SET_RI_daylight_will_cycle(r, v)  { \
					  (r)->daylight_will_cycle = (v); }
#define SET_RI_currently_dark(r, v)	{ \
					  (r)->currently_dark = (v); }
#define SET_RI_square(r, x, y, v)	{ FREE_RI((r)->square[(x)][(y)]); \
					  (r)->square[(x)][(y)] = (v); }




typedef struct _mapinfo {
/* every map has one of these structures of information */
	char	*name;                   /* name of the map */
	char    *author;                 /* name of author of map */
	short	teams_supported;	 /* how many teams are supported */
	char	*team_name[NUM_OF_TEAMS];
	char	*objfilename;            /* file name where obj file found */
	char	*execute_file;	         /* driver runs this one */
	char	*startup_file;	         /* random object startup */
	char	*placement_file;	 /* random objs during game */
	char    *code_file;              /* code for inside objects */
	char	neutrals_allowed;	 /* TRUE when neutral players are
					    allowed in the game */
	short	rooms;			 /* how many rooms are defined */
        RoomInfo *room[];                /* rooms in the map */
	ObjInstance *hash[];		 /* hash on id numbers of objects */
} MapInfo;


#define MI_name(m)		((m)->name)
#define MI_author(m)		((m)->author)
#define MI_teams_supported(m)	((m)->teams_supported)
#define MI_team_name(m, n)	((m)->team_name[(n)])
#define MI_objfilename(m)	((m)->objfilename)
#define MI_execute_file(m)	((m)->execute_file)
#define MI_startup_file(m)	((m)->startup_file)
#define MI_placement_file(m)	((m)->placement_file)
#define MI_code_file(m)		((m)->code_file)
#define MI_neutrals_allowed(m)	((m)->neutrals_allowed)
#define MI_rooms(m)		((m)->rooms)
#define MI_room(m, n)		((m)->room[(n)])
#define MI_hash(m, n)		((m)->hash[(n)])

#define FREE_MI(x)		(if (x) free(x))

#define SET_MI_name(m, v)		{ FREE_MI((m)->name); \
					  ((m)->name) = (v); }
#define SET_MI_author(m, v)		{ FREE_MI((m)->author); \
					  ((m)->author) = (v); }
#define SET_MI_teams_supported(m, v)	{ \
					  ((m)->teams_supported) = (v); }
#define SET_MI_team_name(m, n, v)	{ FREE_MI((m)->team_name[(n)]); \
					  ((m)->team_name[(n)]) = (v); }
#define SET_MI_objfilename(m, v)	{ FREE_MI((m)->objfilename); \
					  ((m)->objfilename) = (v); }
#define SET_MI_execute_file(m, v)	{ FREE_MI((m)->execute_file); \
					  ((m)->execute_file) = (v); }
#define SET_MI_startup_file(m, v)	{ FREE_MI((m)->startup_file); \
					  ((m)->startup_file) = (v); }
#define SET_MI_placement_file(m, v)	{ FREE_MI((m)->placement_file); \
					  ((m)->placement_file) = (v); }
#define SET_MI_code_file(m, v)		{ FREE_MI((m)->code_file); \
					  ((m)->code_file) = (v); }
#define SET_MI_neutrals_allowed(m, v)	{ \
					   ((m)->neutrals_allowed) = (v); }
#define SET_MI_rooms(m, v)		{ \
					   ((m)->rooms) = (v); }
#define SET_MI_room(m, n, v)		{ FREE_MI((m)->room[(n)]); \
					  ((m)->room[(n)]) = (v); }
#define SET_MI_hash(m, n, v)		{ FREE_MI((m)->hash[(n)]); \
					  ((m)->hash[(n)]) = (v); }

#define HASH_SIZE			713
#define HASH_VALUE(id)			(id % HASH_SIZE)
#define HASH_OBJECT(o)			(HASH_VALUE(OI_id(o)))




/* external definitions of global variables */
#ifndef MAP_MAIN

extern MapInfo		mapstats;	/* info about loaded map */
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
