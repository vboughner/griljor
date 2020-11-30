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
extern short		game_password;		/* current packet password */

#endif


/* macros that help tell whether the game is ending or is over */
#define GAME_GOING	(-1)
#define RESET_GAME()	(end_of_world = GAME_GOING)
#define IS_GAME_OVER	(IS_GAME_ENDING && (gametime > end_of_world))
#define IS_GAME_ENDING	(end_of_world != GAME_GOING)
#define IS_DRIVER_GAME_OVER	(IS_GAME_ENDING &&	\
				 (gametime > end_of_world + GAME_OVER_LINGER))

/* macros that mark and check the passage of time between io's */
#define IO_RECEIVED()	(last_io = gametime)
#define	NOT_RECEIVING	(gametime > (last_io + NO_IO_CUTOUT_CLICKS))



/* ======= D A T A G R A M  socket packets (inter-player) =============== */

/*
 * MACHINE-SPECIFIC TYPEDEFS
 *
 * These are to insure proper translation of data for network transmission.
 * They will soon be moved to a config.h file or somesimilar.
 */

#define DPRINTF if (DEBUG) printf

#define SUN3

#ifdef SUN3
#	define USUAL
#endif SUN3

#ifdef DECSTATION
#	define USUAL
#endif /* DECSTATION */

#ifdef USUAL
	typedef char		CARD8;
	typedef short		CARD16;
	typedef int		CARD32;
#endif /* USUAL */

#ifdef HAILMARY
	typedef signed : 8		CARD8;
	typedef signed : 16		CARD16;
	typedef signed : 32		CARD32;
#endif /* HAILMARY */

/* types of packets, and the union that holds them all */

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

#define	FLAG_DEITY	1
#define FLAG_LISTED	2

/*
 * The following is the ever-evolving new definition of Packet.
 */

#define MAXPACKETLEN 1024

		/*
		 * MAXPACKETLEN is the maximum number of CARD8s that can be put
		 * in one packet.  Note that an entire PACKETLEN of data will not
		 * be transmitted for each message, unlike before, so making this
		 * number arbitrarily large is only a memory consideration.  I
		 * toyed with the idea of auto-expanding packets; while this
		 * lets us disregard any worry of maximum packet size, it is also
		 * a bit slower than what I have written here.
		 */

#define GetType(p)		((CARD8)((p)->info[TYPE_OFFSET]))
#define GetTo(p)		(ntohs(*((CARD16 *)((p)->info + TO_OFFSET))))

typedef struct _packet {
		CARD16 to;
		unsigned int len;
		CARD8 info[MAXPACKETLEN];
} Packet;

typedef Packet BossPacket;

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

#define TYPE_OFFSET 0
#define PASSWORD_OFFSET 4
#define FROM_OFFSET 8
#define TO_OFFSET 12
#define SERIAL_OFFSET 16
#define RANDOM_DATA_OFFSET 20

/* The following are used in the global Get and Add macros (ick.) */

extern	CARD8	junkCARD8;
extern	CARD16	junkCARD16;
extern	CARD32	junkCARD32;


/* for making a queue-list of such packets */
typedef struct _qpacket {
	Packet	*pack;
	struct _qpacket	*next;
} QPacket;



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
