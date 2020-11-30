h03591
s 00000/00000/00222
d D 1.3 92/08/07 01:01:52 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00006/00000/00216
d D 1.2 91/04/17 16:55:57 labc-3id 2 1
c added -n option for driver
e
s 00216/00000/00000
d D 1.1 91/02/16 12:54:26 labc-3id 1 0
c date and time created 91/02/16 12:54:26 by labc-3id
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

/* Command Line Interpretation and Default Setting File */

#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include "config.h"
#include "def.h"
#include "extern.h"


/* sets the default values of game startup information */

set_player_defaults(player)
PlayerInfo *player;
{
  struct passwd *pw_ent;
  
  player->play_mode = MODE_PLAY;
  strcpy(player->player_name, UNKNOWN);
  strcpy(player->password, UNKNOWN);
  strcpy(player->our_game, UNKNOWN);
  strcpy(player->gamelist_file, DFLT_GAME_FILE);
  strcpy(player->variable_file, DFLT_VAR_FILE);
  strcpy(player->personal_bitmap, DFLT_BITMAP);
  strcpy(player->bitmap_mask, DFLT_MASK);
  player->out_to_display = FALSE;
  player->reversed = FALSE;
  player->team = 0;

  if (am_driver) {
I 2
    player->driver_forever = FALSE;
E 2
    strcpy(player->map_file, DFLT_MAP_FILE);
    strcpy(player->password_file, DFLT_PASS_FILE);
  }
  else {
    strcpy(player->map_file, UNKNOWN);
    strcpy(player->password_file, UNKNOWN);
  }

  /* get login and host name data on this player */
  pw_ent = getpwuid(getuid());
  if (pw_ent) strcpy(player->login, pw_ent->pw_name);
  else strcpy(player->login, "unknown");
  gethostname(player->host, HOST_LENGTH);
  player->host[HOST_LENGTH-1] = '\0';	/* make sure it is NULL terminated */
}



/* looks at the command line and finds any useful information, which it then
   places into the startup information record.  If a save file has been
   requested, it will be searched before looking at other options */
   
interpret_command_line(player, argc, argv)
PlayerInfo	*player;
int		argc;
char		*argv[];
{
  int	i, ok;
  char  argused[40];
  char	*get_nth_arg();

  player->play_mode = MODE_PLAY;

  /* if there is more than fourty args this guy needs help */
  if (argc >= 40) {
    player->play_mode = MODE_HELP;
    return;
  }

  /* clear the argument checking array */
  for (i=1; i<argc; i++) argused[i] = FALSE;

  /* if there is -h or -l options then set play_mode appropriately */
  if (i=option_exists("-h", argc, argv)) {
    player->play_mode = MODE_HELP;
    argused[i] = TRUE;
  }
  else {
    if (i=option_exists("-l", argc, argv)) player->play_mode = MODE_STATS;
    argused[i] = TRUE;
  }

  /* check for other options */
    if (i=option_exists("-n", argc, argv)) {
      strcpy(player->player_name, get_nth_arg(i+1, argc, argv));
      argused[i] = TRUE;  argused[i+1] = TRUE;
    }

    if (i=option_exists("-b", argc, argv)) {
      strcpy(player->personal_bitmap, get_nth_arg(i+1, argc, argv));
      argused[i] = TRUE;  argused[i+1] = TRUE;
    }

    if (i=option_exists("-bm", argc, argv)) {
      strcpy(player->bitmap_mask, get_nth_arg(i+1, argc, argv));
      argused[i] = TRUE;  argused[i+1] = TRUE;
    }

    if (i=option_exists("-i", argc, argv))  {
      strcpy(player->gamelist_file, get_nth_arg(i+1, argc, argv));
      argused[i] = TRUE;  argused[i+1] = TRUE;
    }

    if (i=option_exists("-g", argc, argv))  {
      strcpy(player->our_game, get_nth_arg(i+1, argc, argv));
      argused[i] = TRUE;  argused[i+1] = TRUE;
    }

    if (i=option_exists("-f", argc, argv))  {
      strcpy(player->variable_file, get_nth_arg(i+1, argc, argv));
      argused[i] = TRUE;  argused[i+1] = TRUE;
    }

    if (i=option_exists("-v", argc, argv))  {
      player->out_to_display = TRUE;
      argused[i] = TRUE;
    }

    if (i=option_exists("-r", argc, argv))  {
      player->reversed = TRUE;
      argused[i] = TRUE;
    }
      
    /* the following information flags are useful/legal only for the driver */
    if (am_driver) {
      if (i=option_exists("-m", argc, argv))  {
        strcpy(player->map_file, get_nth_arg(i+1, argc, argv));
        argused[i] = TRUE;  argused[i+1] = TRUE;
      }

      if (i=option_exists("-p", argc, argv))  {
        strcpy(player->password_file, get_nth_arg(i+1, argc, argv));
        argused[i] = TRUE;  argused[i+1] = TRUE;
I 2
      }

      if (i=option_exists("-n", argc, argv))  {
        player->driver_forever = TRUE;
        argused[i] = TRUE;
E 2
      }
    }

    /* This code handles options legal only by monsters */
    if(am_monster) {
      if(i=option_exists("-Mf", argc, argv)) {
	printf("ATTENTION: -Mf option disabled due to lack of typedef\n");
        /* strcpy(player->monster->control_file, get_nth_arg(i+1, argc, argv)); */
        argused[i] = TRUE;  argused[i+1] = TRUE;
      }
    }

    /* now check that all arguments meant something, if not, he needs help */
    for (i=1,ok=TRUE; i<argc; i++) if (!argused[i]) ok=FALSE;
    if (!ok) player->play_mode = MODE_HELP;
 
}




/* given an argument and the list to look through, this procedure returns
   FALSE is the argument is not found, otherwise it returns the number
   of the element in the argument list where the argument was found */

int option_exists(arg, argc, argv)
char *arg;
int  argc;
char *argv[];
{
  register int	i;
  Boolean	found = FALSE;

  for (i=1; i<argc && !found; i++)
    found = !strcmp(arg, argv[i]);

  if (found) return i-1;
  else return 0;
}



/* This procedure returns a string pointer to the nth argument of an argument
   list.  If there is no nth argument then a pointer to a string containing
   the unknown flag is returned.  In either case this string should be copied,
   not used or altered by the calling procedure */

char *get_nth_arg(n, argc, argv)
int  n, argc;
char *argv[];
{
  if (n >= argc) return UNKNOWN;
  else return argv[n];
}




/* for debugging purposes, to get a look at contents of startup info record */

print_player_info(player)
PlayerInfo *player;
{
  printf("Player Name:      %s\n", player->player_name);
  printf("Password:         %s\n", player->password);
  printf("Game:             %s\n", player->our_game);
  printf("Map File:         %s\n", player->map_file);
  printf("Password File:    %s\n", player->password_file);
  printf("Gamelist File:    %s\n", player->gamelist_file);
  printf("Personal Bitmap:  %s\n", player->personal_bitmap);
  printf("Bitmap Mask:      %s\n", player->bitmap_mask);
}
E 1
