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

/* Header file for game listing routines */

/* the maximum number of players described for any one game */
#define NUM_GAMERS_LISTED	20


/* the signifier in a player->gamelist_file string that means that
   the name given is a host name, not a filename. */
#define HOST_SIGNIFIER		"host:"


/* Records in the game list file are sequential GameInfo structures */

typedef struct _gamer {	/* for storing info about each player */
	char name[NAME_LENGTH];
	char rank[RANK_LENGTH];
	char bitmap[BITMAP_ARRAY_SIZE];
	char mask[BITMAP_ARRAY_SIZE];
	int  games;
	char deity;
	char host[HOST_LENGTH];
	char login[LOGIN_LENGTH];
	char team;
} Gamer;


typedef struct _gameinfo {
	char	name[GAME_NAME_LENGTH];	/* name of the game */
	char	host[PATH_LENGTH];	/* machine where found */
	char	map[MAP_NAME_LENGTH];	/* name of play map */
	int	rooms;		/* number of rooms in the map */
	int	teams;		/* number of teams supported in map */
	char	neutrals_allowed;	/* TRUE when neutral players allowed */
	char	team_name[NUM_OF_TEAMS][TEAM_NAME_LENGTH];
	Gamer	player[NUM_GAMERS_LISTED];	/* player descriptions */
	int	size;		/* # of players (real) in game */
	char	time[60];	/* when the game started */
	Sockaddr	address;	/* internet address */
	struct _gameinfo  *next;	/* pointer used when in memory */
} GameInfo;


/* Procedures that need header info */

GameInfo *get_game_by_name(), *allocate_game_record();
char **return_list_of_games();
