h31007
s 00005/00001/00133
d D 1.7 92/08/28 14:28:04 vbo 7 6
c replaced a constant with a macro
e
s 00003/00010/00131
d D 1.6 92/08/27 20:56:36 vbo 6 5
c removed old ObjInv and replaced it with ObjectInstance
e
s 00000/00000/00141
d D 1.5 92/08/07 01:03:12 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00004/00001/00137
d D 1.4 91/12/07 17:37:54 labc-4lc 4 3
c made error message window bigger and scrollable
e
s 00006/00000/00132
d D 1.3 91/09/03 21:39:17 labb-3li 3 2
c Trevor's misc changes - fixed missile heap
e
s 00000/00000/00132
d D 1.2 91/08/26 00:34:31 vanb 2 1
c fixed up procedure defs and other compatibilty problems
e
s 00132/00000/00000
d D 1.1 91/02/16 13:01:06 labc-3id 1 0
c date and time created 91/02/16 13:01:06 by labc-3id
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

/* Structure Definition File */


I 7
/* number of hands a person has */
#define NUMBER_OF_HANDS 2


E 7
D 6
/* data structure for holding an object as an inventory item */

typedef struct _invobj {
	char	type;		/* type number of the object */
	MemObj	*record;	/* recorded object information */
} InvObj;


E 6
I 3
typedef struct _damobj 
{
  double damage[MAX_GAME_SIZE];
  int when;
} DamObj;
E 3

I 6

E 6
/* data structure to hold information on one person object under a player */

typedef struct _person {
	short	health;			/* health of person */
	short	power;			/* magic power person has */
	long	experience;		/* number of experience pts he has */
I 3
	DamObj  damage;
E 3

	/* array to hold inventory, and two vars to hold what's in hands */
D 6
	InvObj	inventory[INV_SIZE];
	InvObj	hand[2];
E 6
I 6
	ObjectInstance	*inventory_object[INV_SIZE];
D 7
	ObjectInstance  *hand_object[2];
E 7
I 7
	ObjectInstance  *hand_object[NUMBER_OF_HANDS];
E 7
E 6

I 4
	/* array to hold error and status messages recently received */
	char *errmsg[MAX_MESSAGES];

E 4
	/* array to hold the most recent messages received by this person */
D 4
	char	*msg[MAX_MESSAGES];
E 4
I 4
 	char	*msg[MAX_MESSAGES];
E 4

	/* a pointer to a queue of unread packet mail for this person */
	QPacket	*pqueue;	/* front of the queue */
	QPacket	*final;		/* the last thing in the queue */

	/* timer for counting down the pause for death to occur */
	long	death_timer;

	/* the next game click when he can fire again */
	long	fire_again;

	/* his next requested firing/using information, future_hand
           will be -1 when there is no request */
	int	future_hand, future_room, future_x, future_y;
} Person;



/* definition for info on all persons in game */

typedef struct _gameperson {
	short	id;			/* official number in game */
	short	parent;			/* game id of his master person,
					   ie, who to send mail to for him */
	char	name[NAME_LENGTH];	/* name of person */
	char	login[LOGIN_LENGTH];	/* player's login name */
	char	host[HOST_LENGTH];	/* host that player is playing from */

	short	room;			/* room he is in */
	short	x, y;			/* x and y location in room if known */
	short	oldroom;		/* last room he was in */
	short	oldx, oldy;		/* last location he was in */

	short	ignore_walls;		/* is 1 if he walks through walls */
	short	team;			/* number (0-4) of team he is on */
	short	listed;			/* is 1 if he appears in game list */
	short	level;			/* his experience level */
	char	rank[RANK_LENGTH];	/* his ranking */
	int	deity;			/* TRUE if person is a deity */
	short	kills, losses;		/* his luck in battle */
	short	games;			/* how many games he has won in life */
	short	weight_limit;		/* how much weight he can carry */

	short	appearance;			/* appearance type flag */
	char	bitmap[BITMAP_ARRAY_SIZE];	/* person's bitmap */
	char	mask[BITMAP_ARRAY_SIZE];	/* mask used underneath */

	Location *moveq;		/* movement queue */
	long	movewait;		/* clicks to wait between moves */
	long	lastmove;		/* game time at last movement */
	Sockaddr address;		/* where to send his mail */
} GamePerson;



/* type for storing a linked list of persons in a room */

typedef struct _person_list {	  /* info on person in the room */
	GamePerson	*person;  /* pointer to gameperson array record */
	short		important;/* TRUE when he represents player in room */
	struct _person_list *next;/* next record in linked list */
} PersonList;



/* Procedures */

Person		*allocate_person();
GamePerson	*allocate_game_person();
char		*allocate_message_string();
short		*dup_bitmap_array();


/* macro for figuring out other hand */
#define OTHER(hand)	((hand) ? 0 : 1)

/* macro for determining what kind of light a player has (radius) */
#define OWN_LIGHT(pnum)		6


/* Global variables */

#ifndef PERSON_MAIN

/* Persons defined under this player */
extern int	num_persons;		 /* how many persons in use */
extern Person	*person[MAX_PLAYER_SIZE];/* storage of persons' information */

/* array of persons in the game, by game id number */
extern int persons_in_game;		/* how many persons in game */
extern GamePerson	*gameperson[MAX_GAME_SIZE];	/* everyone */

/* array of lists of persons in certain rooms */
extern PersonList	*room_persons;	/* an array with list for each room */

#endif
E 1
