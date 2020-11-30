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

/* Configuration definitions */

#define PROGRAM_NAME	"Griljor"
#define	PROGRAM_VERSION	"1.0"
#define DRIVER_RUN_FILE	"/net/rootbeer.Eng/export/home/vbo/games/bin/grildriver"

/* obtor editor callup strings */
#define XTERM_COMMAND	"xterm -T 'Obtor Text Edit' -e %s"
#define DEFAULT_EDITOR	"emacs %s"

/* default filenames for maps and object definition files */
#define DEFAULT_MAP_PATH \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/map/default.map"
#define DEFAULT_OBJ_PATH "main.obj"
#define MAP_LIST_FILE	"maplist"

/* define the library directories for maps and objects and other things */
#define OBJ_LIB_DIR \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/obj"
#define MAP_LIB_DIR \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/map"
#define HELP_LIB_DIR \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib"
#define MAINT_LIB_DIR \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib"
#define MONSTER_LIB_DIR \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib"


/* locations of help files for text in help windows */
#define EDITOR_HELP_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/editor.help"
#define PLAYER_HELP_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/player.help"
#define GAME_HELP_FILE	 \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/game.help"
#define OBTOR_HELP_FILE  \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/obtor.help"
#define HOST_HELP_FILE	\
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/host.help"

/* location of current news file for game startup */
#define NEWS_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/news"

/* location of the time resitrictions file, if it exists */
#define TIME_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/times"

/* default map file for editor and driver */
#define DFLT_MAP_FILE	DEFAULT_MAP_PATH

/* default password file drivers will look into */
#define DFLT_PASS_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/players"

/* default game list file that players will search to find games to join */
#define DFLT_GAME_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/games"

/* default monster variable file, for specifying monster info */
#define DFLT_VAR_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/variables"

/* default bitmap and mask that players will assume when joining game */
#define DFLT_BITMAP \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/man.bit"
#define DFLT_MASK \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/man.mask"

/* location of bitmaps used by the editpass program */
#define	SLIDER_FILE_NAME \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/bit/slider.bit"
#define	ICON_FILE_NAME \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/bit/icon.bit"
#define DISC_FILE_NAME \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/bit/disc_icon.bit"
#define GREY_FILE_NAME \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/bit/grey.bit"
#define PICT_FILE_NAME \
       "/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/bit/pict.bit"
#define TITLE_FILE_NAME \
       "/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/bit/title.bit"


/* active flag and location of logfile for messages to game master */
#define GM_NAME		"GM"	/* mail name for GM from inside game */
#define	GM_ACTIVE	1	/* is 1 if messages are logged */
#define GM_FILE \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/messages"
#define	GM_PERSONALITY \
	"/net/rootbeer.Eng/export/home/vbo/games/src/griljor/1.0/lib/gm.text"

/* path names of a few good fonts on the system */
#define BIG_FONT_PATH	"12x24"  /* "12x21bas" */   /* "12x24" */
#define REG_FONT_PATH	"9x15"                      /* "9x15"  */
#define TINY_FONT_PATH	"5x8"    /* "6x10"     */   /* "5x8"   */


/* player and game size limitations */
#define MAX_PLAYER_SIZE	5	/* max # of persons player process may run */
#define	MAX_GAME_SIZE	20	/* max # of persons in game */

/* game speed (all items are measured in clicks) */
#define	CLICKS_PER_SEC	1000	/* clicks that pass per second of real time
				   this is set to 1000 for one click per ms */
#define	CLICKS_TO_QUIT	4000	/* time wait when quitting */
#define CLICKS_TO_DIE	5000	/* time wait when dying */
#define USUAL_EXPLOSION_CLICKS	800	/* normal explosion duration */
#define HIT_LINGER_CLICKS	200	/* duration hit register seen */
#define FIRING_WAIT_CLICKS	850	/* usual wait after firing missile */
#define	CLICKS_PER_MOVE	500	/* usual wait between player moves, this is
				   the one to change to adjust player speed */
#define	GAME_OVER_WAIT		12000	/* usual wait for end of game */
#define	GAME_OVER_LINGER	5000	/* how long the driver will linger
					   beyond the GAME_OVER_WAIT time */
#define	NO_IO_CUTOUT_CLICKS	1800000	/* (ms) time till quit when is no io */

/* Factor by which missiles are faster than people */
#define MISSILE_SPEED_FACTOR	2.2

/* socket constants (port numbers should stay same for playing across net) */
#define BOSS_PORT	1137	/* port number we try to get */
#define BOSS_REC_PORT	3323	/* port number for driver receiving players */
#define BOSS_PASSWORD	(game_password)	/* packet password for current game */
#define BOSS_DEBUG	DEBUG	/* TRUE when you want to trace use of boss */
#define DRIVER_SLEEP	1	/* seconds the driver will pause for when
				   checking for new players, until he again
				   checks his current game mail socket */
#define STARTUP_WAIT	60000	/* game clicks a new player will send his
				   MY_INFO packet to other new players when
				   they join the game near time he does */

/* miscellaneous things */
#define DEBUG		0	/* 1 if we should be in debug mode */
#define SPECIAL_IDS	10	/* number of reserved person id numbers */
#define MAX_MISSILE_RANGE	50	/* maximum distance missiles may go */
#define MAX_MULTI_MISSILES	25	/* max number of missiles in packet */
#define MAX_MESSAGES	28	/* number of recent msgs stored for player */
#define MAX_ERROR_MESSAGES	12	/* number of error msgs stored */
#define PLAYER_MESSAGE_COLUMNS 70	/* width of message windows */
/* see "level.h" for configuration of experience levels, hit points, etc. */

/* map definition particulars (changing these makes old maps unusable) */
#define MAP_NAME_LENGTH		80	/* string size reserved */
#define ROOM_NAME_LENGTH	80	/* ditto */
#define ROOM_WIDTH		20	/* object slots (squares) in width */
#define ROOM_HEIGHT		20	/* object slots (squares) in height */
#define ROOM_DEPTH		2	/* number of objs per square */
#define	NUM_OF_TEAMS		4	/* number of teams game handles */
#define	TEAM_NAME_LENGTH	80	/* string space for team names */

/* object particulars (not a good idea to change these) */
#define OBJ_NAME_LENGTH		80	/* string size reserved */
#define BITMAP_WIDTH		32	/* pixels in width */
#define BITMAP_HEIGHT		32	/* pixels in height */
#define BITMAP_ARRAY_SIZE	128	/* number of chars required */
#define MAX_OBJECTS		256	/* maximum types of objects allowed */

/* inventory limitations (changing might cause player window to look funny) */
#define INV_WIDTH	7	/* width of inventory box */
#define INV_HEIGHT	5	/* height of same */
#define INV_SIZE	(INV_WIDTH * INV_HEIGHT)
#define ID_MATCH	10	/* the base number for id matching */

/* other misc things (overall these needn't/shouldn't be changed) */
#define PATH_LENGTH	180	/* length of strings for disk access paths */
#define NAME_LENGTH	25	/* one more than maximum player name length */
#define RANK_LENGTH	50	/* string size for rank names */
#define MSG_LENGTH	80	/* longest message sendable player to player */
#define LOGIN_LENGTH	15	/* length of storage for login name */
#define HOST_LENGTH	40	/* length of storage for host name */
#define PASSWORD_LENGTH	 9	/* one more than maximum password length */
#define GAME_NAME_LENGTH 9	/* one more than maximum game name length */
#define UNKNOWN		"#??#"	/* string signifying an unknown variable */
