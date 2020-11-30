h09629
s 00000/00000/00399
d D 1.18 92/08/07 01:03:20 vbo 18 17
c source copied to a separate tree for work on new map and object format
e
s 00002/00002/00397
d D 1.17 92/05/27 21:45:45 vbo 17 16
c made minimal changes required for compile on Sparc2 SVR4
e
s 00001/00008/00398
d D 1.16 92/01/20 16:30:08 labc-4lc 16 15
c blojo finished changes necesssary for network packets
e
s 00000/00000/00406
d D 1.15 91/12/17 19:37:16 labc-4lc 15 14
c 
e
s 00008/00001/00398
d D 1.14 91/12/15 18:37:59 labc-4lc 14 13
c Put in packet macros.  -blojo
e
s 00000/00001/00399
d D 1.13 91/12/07 17:38:03 labc-4lc 13 12
c made error message window bigger and scrollable
e
s 00002/00002/00398
d D 1.12 91/09/06 00:35:47 labb-3li 12 11
c added reliability flag to all procedure called send_to_...()
e
s 00002/00002/00398
d D 1.11 91/08/27 23:50:50 vanb 11 10
c 
e
s 00002/00002/00398
d D 1.10 91/08/03 23:20:47 labc-3id 10 9
c changed redrawing speed
e
s 00003/00003/00397
d D 1.9 91/06/23 17:25:42 labc-3id 9 8
c fixed a minor thing about making masks
e
s 00002/00000/00398
d D 1.8 91/05/17 02:06:41 labc-3id 8 7
c 
e
s 00006/00002/00392
d D 1.7 91/05/10 04:46:19 labc-3id 7 6
c 
e
s 00001/00001/00393
d D 1.6 91/04/27 21:27:07 labc-3id 6 5
c fixed screen undate rate
e
s 00003/00002/00391
d D 1.5 91/04/14 23:07:32 labc-3id 5 4
c 
e
s 00000/00001/00393
d D 1.4 91/02/19 19:19:36 labc-3id 4 3
c 
e
s 00002/00000/00392
d D 1.3 91/02/18 22:02:56 labc-3id 3 2
c 
e
s 00002/00000/00390
d D 1.2 91/02/18 21:14:02 labc-3id 2 1
c 
e
s 00390/00000/00000
d D 1.1 91/02/16 12:55:38 labc-3id 1 0
c date and time created 91/02/16 12:55:38 by labc-3id
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

/* Main Program File */
/* To contain procedures used only to interface with human players */

/* inclusions */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.c"
#include "lib.h"
#include "map.h"
#include "objects.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gamelist.h"
#include "gametime.h"

D 7

E 7
D 5
int	qdraw_wait = 20;	/* clicks between draw orderly draw requests */
E 5
I 5
D 10
int	qdraw_wait = 40;	/* clicks between draw orderly draw requests */
E 10
I 10
D 17
int	qdraw_wait = 20;	/* clicks between orderly draw requests */
E 17
I 17
int	qdraw_wait = 10;	/* clicks between orderly draw requests */
E 17
E 10
E 5
int	qdraw_last = 0;		/* last time drawing was done */
D 5
int	qdraw_quant = 5;	/* number of draws to do each time */
E 5
I 5
D 6
int	qdraw_quant = 25;	/* number of draws to do each time */
E 6
I 6
D 10
int	qdraw_quant = 8;	/* number of draws to do each time */
E 10
I 10
D 17
int	qdraw_quant = 28;	/* number of draws to do each time */
E 17
I 17
int	qdraw_quant = 64;	/* number of draws to do each time */
E 17
E 10
E 6
E 5


main(argc, argv)
int	argc;
char	*argv[];
{
  PlayerInfo	player;
  
  what_am_i = PLAYER;

  /* get the values of several important game parameters */
  set_player_defaults(&player);
  interpret_command_line(&player, argc, argv);

  /* trap signals and set up error avoidance */
  avoid_errors();

  switch (player.play_mode) {
	case MODE_PLAY:		play_game(&player);
				break;
	case MODE_HELP:		print_usage_instructions();
				break;
	case MODE_STATS:	print_game_list(&player);
				break;
	default:		Gerror("unknown play mode");
				break;
  }

  exit(0);
}



/* main game loop for human players */

play_game(player)
PlayerInfo *player;
{
  /* initialize random number generator */
  initrand();
I 7
D 11
  #ifdef RAYPACK
E 11
I 11
#ifdef RAYPACK
E 11
E 7
I 5
  SetupRaypack();
I 7
D 11
  #endif
E 11
I 11
#endif
E 11
E 7
E 5

  /* welcome player to the game */
  /* this is no longer used, as an X window is used for welcoming now */
  /* welcome_player(); */
 
  /* check for bad or non-existant bitmaps */
  check_player_bitmaps(player);

  /* if player has selected reverse screen mode, then set that mode */
  check_reverse_mode(player);

  /* make sure we can open the player's display, if so then do so */
  open_player_display();

  /* fill in the default information for players */
  fill_in_player_info(player);

  /* start up the person record and game person records for this player */
  startup_player_persons(1);

  /* load player info and bitmap into person #0 */
  load_player_into_person(player, 0);
  load_player_bitmaps(player);

D 4
  initialize_specials();
E 4
  player_main_window_setup();


  /* ask player to select a game, and keep running games until he quits */

  while (player_select_game(player)) {

    /* initialize several of the important game arrays */
    joiner_message("Initializing dynamic memory storage...");
    initialize_room_person_array();
    assign_persons_to_rooms();
    initialize_missile_array();
    initialize_hit_list(mapstats.rooms);
    initialize_important_object_list();
    QDraw_initialize();
I 8
    RoomDrawInitialize();
E 8

    /* set up windows and close the window started for game entry data */
    if (DEBUG) printf("Start up player windows\n");
    joiner_message("Creating play windows and defining pixmaps...");
    start_player_windows();
    close_joiner();
  
    /* set up the neccessary window and map variables for the game */
    if (DEBUG) printf("Start up player variables\n");
    setup_player_variables();
  
    /* get latest news */
    if (DEBUG) printf("Get the latest news\n");
    get_game_news();
  
    /* initialize game time click counter and game over flag */
    init_game_time();
    RESET_GAME();
  
    /* do game play loop */
    player_loop(player);
  
    /* say goodbye to other players in game and free things up for a restart */
    free_for_restart(1);
    comply_with_termination();

  }
  

  /* we are closing up, termination requests would be inconvenient */
  ignore_termination();

  /* close up the socket we were using */
  endBoss();

  /* say goodbye */
  /* printf("Thank you for playing.\n"); */
}



/* main player run-time input and command execution loop */

player_loop(player)
PlayerInfo* player;
{
  int done = FALSE, i, counter = 0;

I 3
#ifdef RAYCODE
E 3
I 2
  InitRaypack(0);
I 3
#endif
E 3

E 2
  /* select a location for my persons */
  select_starting_places();

  /* set up game mode (so players later can watch themselves die when 
     what_mode is set to DYING) */
  what_mode = NORMAL;

  do {
    /* update the time counters */
    if (counter > 11) counter = 0;
    update_game_time();

    /* get input from player and execute commands if there are any this time,
       we now ignore direct quit requests, we'll wait for timer to run out,
       so we don't keep the value player_get_input returns here. */
    player_get_input();
D 7

E 7
I 7
    
E 7
    /* handle the movement all the persons controlled by this player */
    move_persons();

    /* handle drawing requests in the queue */
    if ((gametime - qdraw_last) > qdraw_wait) {
      QDraw_cycle(qdraw_quant);
      qdraw_last = gametime;
    }

    if ((counter % 3) == 0) {
      /* remove old hits from display */
      remove_old_hits();
      /* check for object use commands from the past */
      for (i=0; i<num_persons; i++) use_if_ready(i);
    }

    if ((counter % 3) == 1) {
      /* handle the movement of all missiles in the room */
      update_missiles();
      /* have persons check to see if they are dead, if there is a death
         of the player then what_mode will be set to DYING */
      check_player_deaths();
    }

    if ((counter % 3) == 2) {
      /* load any network packet mail that has come in recently */
      BossIO();
      /* have each person handle their own mail */
      for (i=0; i<num_persons; i++) handle_person_packet_queue(i);
      /* check for futurized person events */
      check_person_events();
    }

    if (!counter) {
      /* check for possibility of returning health points */
      if (what_mode == NORMAL) revive_persons();
      /* check for expiration of the quit countdown timer */
      if ((what_mode == QUITTING) && (time_left <= gametime)) {
        player_quitting_ritual();
        done = TRUE;
      }
      /* check for game over countdown expiration */
      if (IS_GAME_OVER || NOT_RECEIVING) done = TRUE;
    }

    counter++;
  } while (!done);
}





/* fill in possibly undefined information about a player with some defaults */

fill_in_player_info(player)
PlayerInfo *player;
{
  if (!strcmp(player->player_name, UNKNOWN)) 
    strcpy(player->player_name, "guest");

  if (!strcmp(player->password, UNKNOWN))
    strcpy(player->password, "");
}



/* load the player's bitmap and mask into person 0.  If player requested 
   a bitmap but not a mask, then we will make him one */

load_player_bitmaps(player)
PlayerInfo *player;
{
D 9
  short *new;
  unsigned short *make_mask();
E 9
I 9
  char *new;
  char *make_mask();
E 9
  
  /* load the bitmap and mask specified in player info rec */
  load_person_bitmap_and_mask(0, player->personal_bitmap, player->bitmap_mask);

  /* if that mask we loaded was default mask, but bitmap wasn't, then
     figure out a correct mask for him */
  if (strcmp(player->personal_bitmap, DFLT_BITMAP) &&
      !strcmp(player->bitmap_mask, DFLT_MASK)) {
D 9
	  new = (short *) make_mask((unsigned short *)gameperson[0]->bitmap);
E 9
I 9
	  new = make_mask(gameperson[0]->bitmap);
E 9
	  bitmapcpy(gameperson[0]->mask, new);
	  free(new);
  }
}




/* have the player select a team to be on, and then tell everybody about it */

select_a_team()
{
  int team;
  Packet pack;
  char s[80], *team_name;

  /* if there is only one player team then the choice is made */
  if (mapstats.teams_supported == 0) team = 0;
  else if (mapstats.teams_supported == 1) team = 1;
  else {
    print_teams(0);
    do {
      team = atoi(prompt_for_input("Choose a team number: "));
    } while (team < 0 || team > mapstats.teams_supported);
  }

  /* let everyone know what team we've chosen */
  gameperson[0]->team = team;
  prepare_my_info(0, &pack);
  address_packet(&pack, gameperson[0]->id, TO_ALL);
D 12
  send_to_important(&pack);
E 12
I 12
  send_to_important(&pack, FALSE);
E 12
  if (mapstats.teams_supported != 1) {
    if (team) team_name = mapstats.team_name[team-1];
    else team_name = "as a neutral";
    sprintf(s, "%s->ALL %s (%d) is joining %s", GM_NAME, gameperson[0]->name,
	    gameperson[0]->id, team_name);
    prepare_message(&pack, s);
    address_packet(&pack, gameperson[0]->id, TO_ALL);
D 12
    send_to_players(&pack);
E 12
I 12
    send_to_players(&pack, FALSE);
E 12
  }
}




/* check for any of the player's person's dying off, if person zero dies
   then set dying mode.  If dying mode was set long ago then check for
   an end to the death wait period */

check_player_deaths()
{
  register int i;

  for (i=0; i<num_persons; i++)
    if (person[i]->health <= 0) {
      if (person[i]->death_timer < 0) {
	/* drop all his possessions on ground nearby */
	disperse_belongings(i);
	/* change appearance to tombstone */
        show_person_dying(i);
	/* if this is main person then set dying mode */
        if (i==0) set_quit_mode(DYING);
      }
      else if (person[i]->death_timer <= gametime) {
        /* remove the person from the game area and restore appearance */
	dead_person_ritual(i);

	/* start person in a new place */
	select_person_place(i);

	/* if that was person zero, then nullify DYING mode and do redraws */
	if (i==0) {
	  what_mode = NORMAL;
	  if (has_display) {
	    redraw_player_stat_window();
D 13
	    redraw_error_window(" ");
E 13
	  }
	}
      }
    }
}


/* free the various arrays and prepare to allow everything to be
   re-initialized for joining another game.  'num' is the number
   of persons this player is using, usually the same number that
   was given to startup_player_persons() near the top of this file. */

free_for_restart(num)
int num;
{
  register int i;
  
  /* notify other players and driver of leaving persons */
  ignore_termination();
  if (!am_driver) for (i=0; i<num_persons; i++)  leaveGame(i);
  qdraw_last = 0;
I 7

  /* reset various flags and counters */
  reset_revivals();
E 7

  /* free memory up for re-use */
  trim_person_arrays(num);
  free_missile_array();
  free_hit_list();
  free_important_objects();
  free_object_pixmaps();
  free_person_pixmaps();
  free_map();
  free_objects();
I 8
  free_draw_pixmap();
E 8
  close_player_windows();
}



/* ====================== P A C K E T  handlers ======================== */

general_person_message_handler(pnum, pack)
int pnum;
Packet *pack;
{
  /* For live players, we want to print every message
     for the currently displayed person (including TO_ALL messages),
     but only print personal messages that are for the other persons
     (if we have any), meaning ignore TO_ALL messages to other than the
     current person.  TO_ALL message will therefore be printed only once. */

D 16
  if ((pack->to != TO_ALL) || (pnum == 0))
E 16
I 16
  if ((GetTo(pack) != TO_ALL) || (pnum == 0))
E 16
D 14
    player_out_message(pack->info.msg);
E 14
I 14
    player_out_message(GetString(pack));
D 16

/*
 * The above GetString is really iffy-- note it only works if we
 * haven't done a GetString before.  Haven't checked yet.
 *
 *						-blojo
 */
E 16
E 14
}
E 1
