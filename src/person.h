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


/* data structure for holding an object as an inventory item */

typedef struct _invobj {
	char	type;		/* type number of the object */
	MemObj	*record;	/* recorded object information */
} InvObj;


typedef struct _damobj 
{
  double damage[MAX_GAME_SIZE];
  int when;
} DamObj;

/* data structure to hold information on one person object under a player */

typedef struct _person {
	short	health;			/* health of person */
	short	power;			/* magic power person has */
	long	experience;		/* number of experience pts he has */
	DamObj  damage;

	/* array to hold inventory, and two vars to hold what's in hands */
	InvObj	inventory[INV_SIZE];
	InvObj	hand[2];

	/* array to hold error and status messages recently received */
	char *errmsg[MAX_MESSAGES];

	/* array to hold the most recent messages received by this person */
 	char	*msg[MAX_MESSAGES];

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
