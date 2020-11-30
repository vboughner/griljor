h30684
s 00000/00000/00245
d D 1.12 92/08/07 01:04:13 vbo 12 11
c source copied to a separate tree for work on new map and object format
e
s 00009/00006/00236
d D 1.11 92/01/20 16:30:52 labc-4lc 11 10
c blojo finished changes necesssary for network packets
e
s 00010/00000/00232
d D 1.10 91/12/15 23:07:05 labc-4lc 10 9
c Fixing datagram incompatibilities.   -bloo
e
s 00077/00132/00155
d D 1.9 91/12/15 15:09:38 labc-4lc 9 8
c blojo's datagram mungification, attempt #1
e
s 00003/00002/00284
d D 1.8 91/12/07 21:40:56 labc-4lc 8 7
c split socket.c into three files and change DAMAGE_REPORT to REPORT
e
s 00000/00000/00286
d D 1.7 91/09/10 13:31:44 labc-4lc 7 6
c 
e
s 00001/00005/00285
d D 1.6 91/09/06 00:36:17 labb-3li 6 5
c added reliability flag to all procedure called send_to_...()
e
s 00000/00000/00290
d D 1.5 91/08/26 00:35:01 vanb 5 4
c fixed up procedure defs and other compatibilty problems
e
s 00001/00000/00289
d D 1.4 91/05/20 23:53:48 labc-3id 4 3
c T: fixed all sorts of missle bugs and what not...
e
s 00002/00002/00287
d D 1.3 91/04/18 02:11:26 labc-3id 3 2
c 
e
s 00000/00000/00289
d D 1.2 91/02/23 01:49:34 labc-3id 2 1
c 
e
s 00289/00000/00000
d D 1.1 91/02/16 13:01:26 labc-3id 1 0
c date and time created 91/02/16 13:01:26 by labc-3id
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

/* Socket Boss definitions */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* the type of a socket address */
typedef struct sockaddr_in	Sockaddr;


/* more useful constants */
#define TO_ALL	(-1)	/* signifies that this packet is to many persons
			   the game, not just to one particular person */
#define TO_GM	(-2)	/* signifies a packet for GM to be logged in file,
			   only the driver should get packets like this */
#define FROM_GM	(-2)	/* signifies a packet was from the GM */


/* external variable definitions for globals that calling program should
   know about, the Boss package will define these for itself, so for it we
   skip these. */

#ifndef BOSS_PACKAGE

extern int		Iamboss;	/* TRUE if I am the boss */
extern Sockaddr		ownaddr;	/* my own network address */
extern int		thesocket;	/* my own main socket */
extern Sockaddr		recaddr;	/* address for receiving new players */
extern int		recsocket;	/* socket where receiving is done */
extern Sockaddr		driveraddr;	/* address of drivers dgram socket */
extern long		end_of_world;	/* countdown to game end */
extern long		last_io;	/* last gameclick io was received on */
extern char		dmessage[MSG_LENGTH];	/* last message from driver */
I 8
extern short		game_password;		/* current packet password */
E 8

#endif


D 3
/* a way to tell is the game is ending or is over */
E 3
I 3
/* macros that help tell whether the game is ending or is over */
E 3
#define GAME_GOING	(-1)
#define RESET_GAME()	(end_of_world = GAME_GOING)
#define IS_GAME_OVER	(IS_GAME_ENDING && (gametime > end_of_world))
#define IS_GAME_ENDING	(end_of_world != GAME_GOING)
#define IS_DRIVER_GAME_OVER	(IS_GAME_ENDING &&	\
				 (gametime > end_of_world + GAME_OVER_LINGER))

D 3
/* io time marking macros */
E 3
I 3
/* macros that mark and check the passage of time between io's */
E 3
#define IO_RECEIVED()	(last_io = gametime)
#define	NOT_RECEIVING	(gametime > (last_io + NO_IO_CUTOUT_CLICKS))



/* ======= D A T A G R A M  socket packets (inter-player) =============== */

D 9
/* types of packets, and the union that holds them all */
E 9
I 9
/*
 * MACHINE-SPECIFIC TYPEDEFS
 *
 * These are to insure proper translation of data for network transmission.
 * They will soon be moved to a config.h file or somesimilar.
 */
E 9

D 9
#define	MY_INFO		1	/* person to all: initial info about me */
#define MY_BITMAP	2	/* person to all: this is my bitmap */
#define	MY_MASK		3	/* person to all: this is my mask */
#define MY_LOCATION	4	/* person to any: this is where I am */
#define GAME_OVER	5	/* person to all: game end requirements done */
#define WEAPON_FIRED	6	/* person to any: weapon fired near you */
D 8
#define DAMAGE_REPORT	7	/* person to person: you hit me */
E 8
I 8
#define REPORT		7	/* person to person: report message */
E 8
#define MESSAGE		8	/* person to person: text message */
#define ADD_EXPERIENCE	9	/* person to person: take this many exp. pts */
#define LEAVING_GAME	10	/* person to all: I'm leaving the game */
#define CHANGE_MAP	11	/* person to all: change the map as follows */
#define EXECUTE_COMMAND	12	/* person to person: override your control */
#define	REPEAT_INFO	13	/* person to person: give me your info again */
#define	SAVE_STATS	14	/* person to driver: save my stats please */
#define YOU_KILLED_ME	15	/* person to person: give yourself a kill */
#define	MULTI_FIRE	16	/* person to room: many missiles fired here */
E 9
I 9
#define DPRINTF if (DEBUG) printf
E 9

D 9
/* This info structure is for MY_INFO packets which are sent so that
   persons will know each others name etc.   It is the responsibility of
   the new player to inform each of the other players in the game of his
   existence. */
typedef struct _my_info {	/* person's game information */
	short	parent;		/* parent person id number */
	char	name[NAME_LENGTH];	/* person's name */
	char	login[LOGIN_LENGTH];	/* player's last login name */
	char	host[HOST_LENGTH];	/* host that player last played from */
	char	rank[RANK_LENGTH];	/* person's rank */
	short	level;		/* person's level number */
	int	deity;		/* TRUE if person is a deity */
	short	team;		/* number (0-4) of team person is on */
	short	listed;		/* TRUE if this is listed human person */
} My_Info;
E 9
I 9
#define SUN3
E 9

I 9
#ifdef SUN3
#	define USUAL
#endif SUN3
E 9

D 9
/* used as the info structure for the following type of packet: MY_LOCATION */
typedef struct _my_location {	/* personal address info */
	short		room;	/* room he is in */
	char		x, y;	/* location in the room */
	short		appear;	/* this person's current appearance type */
} My_Location;	
E 9
I 9
#ifdef DECSTATION
#	define USUAL
#endif /* DECSTATION */
E 9

I 9
#ifdef USUAL
D 11
	typedef unsigned char		CARD8;
	typedef unsigned short		CARD16;
	typedef unsigned int		CARD32;
E 11
I 11
	typedef char		CARD8;
	typedef short		CARD16;
	typedef int		CARD32;
E 11
#endif /* USUAL */
E 9

D 9
/* used when the requirements for ending the game are fulfilled. The player
   that finshes the last requirement sends this message to all and to the
   driver so that everyone can simultaneously perform game end rituals */
typedef struct _game_over {	/* info on which tema won etc. */
	short		winner;	/* id of person responsible for ending game */
	short		team;	/* team that wins the game */
} Game_Over;
E 9
I 9
#ifdef HAILMARY
D 11
	typedef unsigned : 8		CARD8;
	typedef unsigned : 16		CARD16;
	typedef unsigned : 32		CARD32;
E 11
I 11
	typedef signed : 8		CARD8;
	typedef signed : 16		CARD16;
	typedef signed : 32		CARD32;
E 11
#endif /* HAILMARY */
E 9

I 9
/* types of packets, and the union that holds them all */
E 9

D 9
/* this one is for WEAPON_FIRED (firing normal weapons in game) */
typedef struct _weapon_fired {	/* info about a weapon discharge or use */
	Missile		poop;	/* the poop (info) about the missile */
	short		room;	/* which room it is in */
	short		range;	/* how far it will go (highest array element
				   in use; 0 is for starting location) */
	char		x[MAX_MISSILE_RANGE];	/* locations in room it is */
	char		y[MAX_MISSILE_RANGE];	/* flying/going through */
} Weapon_Fired;
E 9
I 9
enum {
	MY_INFO,		/* person to all: initial info about me */
	MY_BITMAP,		/* person to all: this is my bitmap */
	MY_MASK,		/* person to all: this is my mask */
	MY_LOCATION,		/* person to any: this is where I am */
	GAME_OVER,		/* person to all: game end requirements done */
	WEAPON_FIRED,		/* person to any: weapon fired near you */
	REPORT,			/* person to person: report message */
	MESSAGE,		/* person to person: text message */
	ADD_EXPERIENCE,		/* person to person: take this many exp. pts */
	LEAVING_GAME,		/* person to all: I'm leaving the game */
	CHANGE_MAP,		/* person to all: change the map as follows */
	EXECUTE_COMMAND,	/* person to person: override your control */
	REPEAT_INFO,		/* person to person: give me your info again */
	SAVE_STATS,		/* person to driver: save my stats please */
	YOU_KILLED_ME,		/* person to person: give yourself a kill */
	MULTI_FIRE		/* person to room: many missiles fired here */
};
E 9

I 10
#define	FLAG_DEITY	1
#define FLAG_LISTED	2

E 10
I 9
/*
 * The following is the ever-evolving new definition of Packet.
 */
E 9

D 9
/* this one is for ADD_EXPERIENCE */
typedef struct _add_experience {/* what kind of experience pts you get */
	short	id;		/* person to get experience */
	int	amount;		/* number of points you should add */
	char	msg[MSG_LENGTH];/* a message concerning the addition */
} Add_Experience;
E 9
I 9
#define MAXPACKETLEN 1024
E 9

I 9
		/*
		 * MAXPACKETLEN is the maximum number of CARD8s that can be put
		 * in one packet.  Note that an entire PACKETLEN of data will not
		 * be transmitted for each message, unlike before, so making this
		 * number arbitrarily large is only a memory consideration.  I
		 * toyed with the idea of auto-expanding packets; while this
		 * lets us disregard any worry of maximum packet size, it is also
		 * a bit slower than what I have written here.
		 */
I 11

#define GetType(p)		((CARD8)((p)->info[TYPE_OFFSET]))
#define GetTo(p)		(ntohs(*((CARD16 *)((p)->info + TO_OFFSET))))
E 11
E 9

D 9
/* this one is for CHANGE_MAP */
typedef struct _change_map {	/* info on what to change on the map */
	short		room;	/* room where change takes place */
	short		x, y;	/* square in the room */
	short		which;	/* which spot to change (between 0 and
				   ROOM_DEPTH, max value refers to rec obj) */
	short		old;	/* the old type that was there */
	short		new;	/* the new type that is going there */
	RecordedObj	recobj;	/* possibly used for rec obj record */
} Change_Map;


/* this one is for EXECUTE_COMMAND */
typedef struct _exec_command {	/* a direct command to a player process */
	short		force;	/* TRUE when he must absolutely obey */
	short		type;	/* which command type */
} Execute_Command;


/* this one is for SAVE_STATS */
typedef struct _save_stats {	/* driver saves this info in password file */
	short		id;	/* person id number in game */
	int		ExpPts;	/* new experience point total */
	char		rank[RANK_LENGTH];
	short		level;
	int		kills, losses, games;	/* other stats */
} Save_Stats;



/* this one is for MULTI_FIRE */
typedef	struct _multi_fire {	/* many missiles of same type fired at once */
	short	owner;		/* id of person responsible for this */
	char    hurts_owner;    /* if these missiles should hurt owners */
	short	type;		/* object type of the parent */
	short	roomnum;	/* which room this is happening in */
	char	number;		/* number of missiles in this packet */
I 4
	char    include_start;  /* include the start of the missiles? */
E 4
	char	x1[MAX_MULTI_MISSILES];	/* starting coords for each missile */
	char	y1[MAX_MULTI_MISSILES];
	char	x2[MAX_MULTI_MISSILES]; /* ending coords for each missile */
	char	y2[MAX_MULTI_MISSILES];
} Multi_Fire;



/* finally, all the options for info in a packet lumped as a union */
typedef union _packinfo {
	My_Info		data;			/* MY_INFO		   */
	My_Location	loc;			/* MY_LOCATION		   */
	char		bit[BITMAP_ARRAY_SIZE];	/* MY_BITMAP | MY_MASK	   */
	Game_Over	game;			/* GAME_OVER		   */
	Weapon_Fired	fire;			/* WEAPON_FIRED		   */
D 8
	char		msg[MSG_LENGTH];	/* MESSAGE | DAMAGE_REPORT */
E 8
I 8
	char		msg[MSG_LENGTH];	/* MESSAGE | REPORT        */
E 8
	Add_Experience	add;			/* ADD_EXPERIENCE	   */
	Change_Map	map;			/* CHANGE_MAP		   */
	Execute_Command cmd;			/* EXECUTE_COMMAND	   */
	Save_Stats	stat;			/* SAVE_STATS		   */
	Multi_Fire	mult;			/* MULTI_FIRE		   */
} PackInfo;
	

/* the final definition of a packet, using the union and all */
E 9
typedef struct _packet {
D 9
	short		type;		/* the type of packet sent */
	short		password;	/* identifier of Boss version */
	short		from;		/* id number of sending person */
	short		to;		/* id number of intended receiver */
I 6
	short		serial;		/* serial number of the packet */
E 6
	PackInfo	info;		/* the pertinent information */
} BossPacket;
E 9
I 9
		CARD16 to;
		unsigned int len;
		CARD8 info[MAXPACKETLEN];
} Packet;
E 9

I 9
typedef Packet BossPacket;
E 9

D 9
/* other uses for the packet structure */
typedef BossPacket Packet;
E 9
I 9
/*
 * Every info[] array in a Packet begins with the following information,
 * and then contains (afterward) packet-specific information:
 *
 * CARD8				type
 * CARD16				password
 * CARD16				from
 * CARD16				to
 * CARD16				serial
 */
E 9

I 9
#define TYPE_OFFSET 0
#define PASSWORD_OFFSET 4
#define FROM_OFFSET 8
#define TO_OFFSET 12
#define SERIAL_OFFSET 16
#define RANDOM_DATA_OFFSET 20
I 10

/* The following are used in the global Get and Add macros (ick.) */

extern	CARD8	junkCARD8;
extern	CARD16	junkCARD16;
extern	CARD32	junkCARD32;

E 10
E 9

/* for making a queue-list of such packets */
typedef struct _qpacket {
	Packet	*pack;
	struct _qpacket	*next;
} QPacket;
D 6





E 6



/* ========== S T R E A M  socket packets (new player) ================== */

/* a structure for storing information about a player */

typedef struct _mini_player {
	char	player_name[NAME_LENGTH];
	char	password[PASSWORD_LENGTH];
	char	login[LOGIN_LENGTH];	/* player's last login name */
	char	host[HOST_LENGTH];	/* host that player last played from */
	char	our_game[NAME_LENGTH];
	int	num_persons;		/* how many persons I use */
	int	what_am_i;		/* am I monster or player? */
	Sockaddr    address;		/* my network address */
} MiniPlayer;


/* a record to store important person information */

typedef struct _mini_person {
	short	id;
	short	parent;
	char	name[NAME_LENGTH];
	char	bitmap[BITMAP_ARRAY_SIZE];
	char	mask[BITMAP_ARRAY_SIZE];
	long	experience;
	char	rank[RANK_LENGTH];
	int	team;
	int	deity;			/* TRUE if person is a deity */
	short	kills, losses, games;	/* person statistics */
	short	savebits;	/* TRUE if he has image driver should save */
	short	loadbits;	/* TRUE if player should read in bitmap */
	short	listed;		/* TRUE if person should appear in game list */
	short	checksave;	/* TRUE if person might have saved record */
	short	loaded;		/* TRUE when driver found a save record */
} MiniPerson;


/* the record that is to contain all information a player should tell to
   the driver when he requests entry to a game.  Driver will add to this
   and send it back. */

typedef struct _driver_pack {
	MiniPlayer  info;	/* contains player's name, password, etc. */
	MiniPerson  newperson[MAX_PLAYER_SIZE];
} DriverPack;



/* The record that driver sends back to player to let him know whether he
   has been accepted into the game or not */

typedef struct _acceptance_pack {
	short	accepted;		/* TRUE or FALSE */
	char	msg[MSG_LENGTH];	/* message concerning entry */
	Sockaddr	address;	/* driver's datagram address */
	short		password;	/* this game's packet password */
} AcceptancePack;




/* header information for use of routines inside of socket.c */
Sockaddr	*specifyBossHost();
QPacket		*duplicate_packet();
E 1
