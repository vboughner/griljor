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

int	qdraw_wait = 10;	/* clicks between orderly draw requests */
int	qdraw_last = 0;		/* last time drawing was done */
int	qdraw_quant = 64;	/* number of draws to do each time */


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
#ifdef RAYPACK
  SetupRaypack();
#endif

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
    RoomDrawInitialize();

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

#ifdef RAYCODE
  InitRaypack(0);
#endif

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
  char *new;
  char *make_mask();
  
  /* load the bitmap and mask specified in player info rec */
  load_person_bitmap_and_mask(0, player->personal_bitmap, player->bitmap_mask);

  /* if that mask we loaded was default mask, but bitmap wasn't, then
     figure out a correct mask for him */
  if (strcmp(player->personal_bitmap, DFLT_BITMAP) &&
      !strcmp(player->bitmap_mask, DFLT_MASK)) {
	  new = make_mask(gameperson[0]->bitmap);
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
  send_to_important(&pack, FALSE);
  if (mapstats.teams_supported != 1) {
    if (team) team_name = mapstats.team_name[team-1];
    else team_name = "as a neutral";
    sprintf(s, "%s->ALL %s (%d) is joining %s", GM_NAME, gameperson[0]->name,
	    gameperson[0]->id, team_name);
    prepare_message(&pack, s);
    address_packet(&pack, gameperson[0]->id, TO_ALL);
    send_to_players(&pack, FALSE);
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

  /* reset various flags and counters */
  reset_revivals();

  /* free memory up for re-use */
  trim_person_arrays(num);
  free_missile_array();
  free_hit_list();
  free_important_objects();
  free_object_pixmaps();
  free_person_pixmaps();
  free_map();
  free_objects();
  free_draw_pixmap();
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

  if ((GetTo(pack) != TO_ALL) || (pnum == 0))
    player_out_message(GetString(pack));
}
