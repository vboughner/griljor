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

/* Main Program File for Monsters */

/* inclusions */

#include "config.h"
#include "def.h"
#include "extern.c"
#include "map.h"
#include "objects.h"
#include "missile.h"
#include "socket.h"
#include "person.h"


/* main program flow */

main(argc, argv)
int	argc;
char	*argv[];
{
  PlayerInfo	player;
  
  printf("working\n");
  what_am_i = MONSTER;

  /* get command line options or defaults */
  set_player_defaults(&player);
  interpret_command_line(&player, argc, argv);

  /* trap signals and set up error avoidance */
  avoid_errors();

  switch (player.play_mode) {
	case MODE_PLAY:		pre_be_monster(&player);
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



/* stuff we must do before passing control to other monster routines */

pre_be_monster(player)
PlayerInfo *player;
{
  /* initialize random number generator */
  initrand();

  /* check for bad or non-existant bitmaps */
  check_player_bitmaps(player);

  /* make sure we can open the player's display, if so then do so */
  if (player->out_to_display) {
    use_display = TRUE;
    check_reverse_mode(player);
    open_player_display();
    welcome_player();
  }

  /* make sure that monster language handler knows about info record */
  pass_info_record(player);

  /* pass control over to other monster routines now */
  be_monster(player->player_name);

  /* make sure that they terminated their game properly */
  if (MQGameGoing()) {
    printf("Warning: game not ended properly with MLeaveGame()");
    MLeaveGame();
  }
}





  

/* ====================== P A C K E T  handlers ======================== */

general_person_message_handler(pnum, pack)
int pnum;
Packet *pack;
{
}
