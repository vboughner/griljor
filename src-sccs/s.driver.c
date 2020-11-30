h51222
s 00002/00003/00373
d D 1.5 92/09/03 15:05:40 vbo 5 4
c removed use of NO_FILE as the no file indicator, now uses NULL
e
s 00000/00000/00376
d D 1.4 92/08/07 01:00:50 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00046/00018/00330
d D 1.3 91/04/17 16:55:40 labc-3id 3 2
c added permanent game names, and ability to be permanent
e
s 00002/00000/00346
d D 1.2 91/03/31 00:07:49 labc-3id 2 1
c 
e
s 00346/00000/00000
d D 1.1 91/02/16 12:53:48 labc-3id 1 0
c date and time created 91/02/16 12:53:48 by labc-3id
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

/* Main Program File for Driver */

/* inclusions */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.c"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "gamelist.h"
#include "person.h"
#include "lib.h"
#include "gametime.h"
#include "putstuff.h"

#define	DRIVER_QUIT_TIME	300	/* how long driver will wait (sec) */

/* globals in this file only */
char		ownhost[PATH_LENGTH];	/* name of machine driver is on */
int		gamesize = 0;		/* number of real players in game */
char		start_time[80];		/* time when game started */
DropShipment	*placement_data = NULL;	/* random object placement stuff */
D 3
#define		GAME_NAMES	120	/* number of default name choices */
char		*gamename[GAME_NAMES] = {
E 3
I 3
/* possible randomly-chosen names for games.  Last element of this
   list must be a NULL (or you'll get a major segmentation fault!) */
char		*gamename[] = {
E 3
		"todo", "oz", "slime", "arch", "gore",
		"big", "salt", "pop", "blister", "zero",
		"bull",	"cad", "grape", "albert", "mel",
		"eric", "stefan", "boyd", "ghost", "trev",
		"peter", "ollie", "blood", "guts", "maroon",
		"fido", "amber", "frap", "neutron", "nuke",
		"atom", "zap", "rat", "mega", "terra",
		"giga", "tera", "jove",	"emacs", "mouse",
		"cat", "yacc", "lambda", "pi", "delta",
		"gobears", "bomb", "stop", "go", "burt",
		"random", "wheel", "bleys", "caine", "omni",
		"soda", "ocf", "web", "bh", "silver",
		"gold", "fish", "cray", "crumb", "nomacs",
		"nomad", "lithium", "radon", "crypt", "krypton",
		"zed", "plato", "sock", "biff", "load",
		"grill", "brat", "tongue", "lip", "vortex",
D 3
		"time", "new", "sun", "moon", "space",
E 3
I 3
		"trap", "new", "sun", "moon", "space",
E 3
		"bum", "peep", "blip", "cord", "cork",
D 3
		"thrat", "thrift", "nasty", "beetle", "finger",
E 3
I 3
		"thrash", "thrift", "nasty", "beetle", "finger",
E 3
		"watch", "ftp",	"open", "csh", "jobs",
		"pad", "frac", "marvin", "simpson", "juice",
		"zaphod", "dent", "blake", "drwho", "ford",
		"finland", "holland", "china", "money", "cash",
D 3
		"gray",	"color", "glass", "quest", "drool" };
E 3
I 3
		"gray",	"color", "glass", "quest", "drool",
		"van", "doug", "will", "berk", "cal", NULL};
E 3

I 3
/* possible not-randomly-chosen names for games set up in non-dying mode.
   Last element of this list must be a NULL. */
char		*permname[] = {
		"default", "tourney1", "tourney2", "tourney3",
		"tourney4", NULL};
E 3

I 3

E 3
/* procedure declarations */
GameInfo	*make_game_info_record();
char		*figure_default_game_name();



/* main program flow */

main(argc, argv)
int	argc;
char	*argv[];
{
  PlayerInfo	driver;
  
  what_am_i = DRIVER;

  /* get command line options or defaults */
  set_player_defaults(&driver);
  interpret_command_line(&driver, argc, argv);

  /* trap signals and set up error avoidance */
  avoid_errors();

  switch (driver.play_mode) {
	case MODE_PLAY:		be_driver(&driver);
				break;
	case MODE_HELP:		print_usage_instructions();
				break;
	case MODE_STATS:	print_game_list(&driver);
				break;
	default:		Gerror("unknown play mode");
				break;
  }

  exit(0);
}



be_driver(driver)
PlayerInfo *driver;
{
  int do_again = TRUE;

  /* check for times during the week when play is not allowed */
  exit_upon_time_restriction();
  
  /* initialize random number generator */
  initrand();
  
  /* let the password file routines know what the password file is */
  set_password_file(driver->password_file);

  /* figure out what machine I'm on */
  gethostname(ownhost, sizeof(ownhost));

  /* load game master's personality file */
I 2
  if (DEBUG) printf("Loading GM personality...\n");
E 2
  set_personality(0, GM_PERSONALITY);

  /* keep recycling the game until it has been empty for some time */
  do {

    /* figure out start time of the game */
I 2
    if (DEBUG) printf("Starting a new game up...\n");
E 2
    strcpy(start_time, string_time());

    /* load the map we are using */
    strcpy(mapfile, driver->map_file);
    strcpy(objectfile, UNKNOWN);
    load_map();

    /* put stuff onto the map if that is called for in the map file */
D 5
    if (strcmp(mapstats.startup_file, NO_FILE))
      PutStuff(mapstats.startup_file);
E 5
I 5
    if (mapstats.startup_file) PutStuff(mapstats.startup_file);
E 5
  
    /* load the in-game random object placement information file */
D 5
    if (strcmp(mapstats.placement_file, NO_FILE))
E 5
I 5
    if (mapstats.placement_file)
E 5
      placement_data = ReadStuffFile(mapstats.placement_file);

    /* figure out how many flags each team will need to win the game */
    initialize_important_object_list();
    count_all_game_flags();

    /* ignore ctrl-c from this point */
    ignore_termination();

    /* setup my sockets */
    initSelfBoss();

    /* get myself listed in the game list */
    make_my_game_listed(driver);

    /* allow ctrl-c or other termination (with a catch) */
    safety_termination(driver);

    /* initialize game person list */
    driver_person_init();

    /* accept players and update the map until current game is over,
       if FALSE is returned then this was the last game we'll play. */
    do_again = driver_game_loop(driver);

    /* remove my game listing */
    ignore_termination();
    remove_game(driver);
    comply_with_termination();

    /* free all of the allocated arrays so the driver can do it again */
    free_driver_for_restart();

  } while (do_again);

  /* ignore termination requests from now till we finish wrapping it up */
  ignore_termination();
  endBoss();
}




/* add myself to the gamelist file, putting my internet address and other
   useful information into the list of avaliable games */

make_my_game_listed(driver)
PlayerInfo *driver;
{
  /* search for a default game name and use it instead if no name was given */
  if (!strcmp(driver->our_game, UNKNOWN))
    strcpy(driver->our_game, figure_default_game_name(driver));

  /* add my game to the game list file */
  if (!add_game(driver, make_game_info_record(driver)))
    Gerror("sorry game by this name already exists");
}



D 3
/* figure out a suitable game name that is not already in use */
E 3
I 3
/* figure out a suitable game name that is not already in use, either
   the next permanent game name, if available, or a random game name.
   If all the game names seem to have been used up, we will allow repeats. */
E 3

char *figure_default_game_name(driver)
PlayerInfo *driver;
{
D 3
  int chosen, okay;
E 3
I 3
  char *name;
  int num_games, num_perms, perm = 0;
  int chosen, okay, tries = 0;
E 3

I 3
  for(num_games=0; gamename[num_games]; num_games++);
  for(num_perms=0; permname[num_perms]; num_perms++);

E 3
  do {
D 3
    /* choose a name from the random list of default names */
    chosen = lrand48() % GAME_NAMES;
E 3
I 3
    /* look for a permanent game name if this is a permanent game */
    if (driver->driver_forever && perm < num_perms) {
      name = permname[perm];
      perm++;
    }
    else {
      /* otherwise choose a name from the random list of default names */
      chosen = lrand48() % num_games;
      name = gamename[chosen];
    }
E 3

    /* see if the game exists already */
D 3
    okay = (game_exists(driver, gamename[chosen])) ? FALSE : TRUE;
  } while (!okay);
E 3
I 3
    okay = (game_exists(driver, name)) ? FALSE : TRUE;
E 3

D 3
  return (gamename[chosen]);
E 3
I 3
  } while (!okay && num_games > tries++);

  return (name);
E 3
}



/* return a GameInfo record that contains all current info on our game
   Warning: this routine makes heavy use of global variables, but only
   in an information gathering capacity. */

GameInfo *make_game_info_record(driver)
PlayerInfo *driver;
{
  int i, j;
  static GameInfo record;

  /* place information about me as driver into gamelist record,
     also include a great deal of info about the current game */

  strncpy(record.name, driver->our_game, GAME_NAME_LENGTH-1);
  record.name[GAME_NAME_LENGTH-1]='\0';	/* game name */
  strcpy(record.host, ownhost);		/* machine name */
  strcpy(record.map, mapstats.name);	/* map name */
  strcpy(record.time, start_time);	/* when game started */
  record.rooms = mapstats.rooms;		/* # of rooms in the map */
  record.teams = mapstats.teams_supported;	/* # of teams in this game */
  record.neutrals_allowed = mapstats.neutrals_allowed;
  for (i=0; i<mapstats.teams_supported; i++)	/* names of the teams */
    strcpy(record.team_name[i], mapstats.team_name[i]);

  /* place information about each of the listed players in the game */
  for (i=0,j=0; i<persons_in_game; i++)
    if (gameperson[i]->listed) {
	strcpy(record.player[j].name, gameperson[i]->name);
	strcpy(record.player[j].rank, gameperson[i]->rank);
	bitmapcpy(record.player[j].bitmap, gameperson[i]->bitmap);
	bitmapcpy(record.player[j].mask, gameperson[i]->mask);
	strcpy(record.player[j].host, gameperson[i]->host);
	strcpy(record.player[j].login, gameperson[i]->login);
	record.player[j].deity = gameperson[i]->deity;
	record.player[j].team = gameperson[i]->team;
	j++;
    }
  record.size = j;	/* # of listed players in game */


  /* a copy of the socket address is neccessary for new players to join */
  bcopy(&recaddr, &(record.address), sizeof(Sockaddr));

  return &record;
}



/* Initialize game person list and set number of persons in game so far
   to be zero. */

driver_person_init()
{
  persons_in_game = 0;
  initialize_game_person_array();
}



/* accept new players and update the map until game is over, return TRUE
   if we should play this map again, FALSE if we should not */

driver_game_loop(driver)
PlayerInfo *driver;
{
  int listed_persons, done = 0, play_again = TRUE;
  
  /* set the time mark and the game as continual */
  init_game_time();
  set_time_mark();
  RESET_GAME();

  /* while there are still players in the game or we haven't waited long
     enough, we should continue to look for new players */

  do {
    /* update the time counter */
    update_game_time();

    /* check and accept a new player (as long as game isn't ending) */
    /* when new player joins, reset time till we quit */
    if (checkNewPlayer()) set_time_mark();	

    /* check for mail from players already in the game */
    BossIO();

    /* look for a chance to put things in the game */
D 3
    if (gamesize > 0)
      CheckForPutStuffIn(placement_data, (gametime / CLICKS_PER_SEC));
E 3
I 3
    CheckForPutStuffIn(placement_data, 
		       (gametime / CLICKS_PER_SEC), gamesize);
E 3

    /* check for a change in number of listed persons in the game */
    if ((listed_persons = number_of_listed_persons()) != gamesize) {
      /* update the gamelist file */
      gamesize =  listed_persons;
      update_game(driver, make_game_info_record(driver));
      set_time_mark();
    }

D 3
    /* quit if there are no players anymore and timer has run out */
    done = (!persons_in_game && (query_time_marked() >= DRIVER_QUIT_TIME));
E 3
I 3
    /* quit if there are no players anymore and timer has run out,
       but only if the "driver_forever" command line option wasn't set */
    done = (!persons_in_game &&
	    (query_time_marked() >= DRIVER_QUIT_TIME) &&
	    (!driver->driver_forever));
E 3
    if (done) play_again = FALSE;

    /* check that game hasn't been won by someone */
    if (IS_DRIVER_GAME_OVER) done = TRUE;

D 3
    /* loop will also terminate when no io has been received for long time */
    if (NOT_RECEIVING) done = TRUE;
E 3
I 3
    /* loop will also terminate when no io has been received for long time,
       again only if the command line "driver_forever" option wasn't set. */
    if (NOT_RECEIVING && !driver->driver_forever) done = TRUE;
E 3

  } while (!done);
  return play_again;
}



/* free the game arrays and other things in preparation for the driver
   restarting the game and loading everything fresh again.  Refer to
   the free_for_restart() routine for players. */

free_driver_for_restart()
{
  free_all_game_persons();
  free_important_objects();
  free_map();
  free_objects();
}
E 1
