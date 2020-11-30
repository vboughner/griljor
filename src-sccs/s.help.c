h22900
s 00000/00000/00097
d D 1.3 92/08/07 01:01:34 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00017/00009/00080
d D 1.2 91/04/17 16:55:06 labc-3id 2 1
c made logging of messages a little neater, added 10 minute counter
e
s 00089/00000/00000
d D 1.1 91/02/16 12:54:17 labc-3id 1 0
c date and time created 91/02/16 12:54:17 by labc-3id
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

/* Help Program File, includes log file routines */

#include <stdio.h>
#include <string.h>
I 2
#include <time.h>
E 2
#include "config.h"
#include "def.h"
#include "extern.h"


print_usage_instructions()
{
  if (am_driver) {
D 2
    printf("\nUsage: grildriver [-h] [-l] [-i info_file] [-g game_name]");
    printf(  " [-m map_file]\n\n");
E 2
I 2
    printf("\nUsage: grildriver [-h] [-l] [-n] [-i info_file] ");
    printf(  "[-g game_name] [-m map_file]\n\n");
E 2
    printf("\t[-h]           get this usage help\n");
    printf("\t[-l]           get a list of games in progress\n");
I 2
    printf("\t[-n]           have driver not quit when there are no players\n");
    printf("\t               in game (this option should be used sparingly!)\n");
E 2
    printf("\t[-i info_file] use a game listing file other than default\n");
    printf("\t[-g game_name] specify name of the new game (max 8 letters)\n");
    printf("\t[-m map_file]  specify filename of map you will play on\n\n");
  }
  else {
    printf("\nUsage: griljor [-h] [-l] [-r] [-b bitmap] [-bm mask_bitmap]\n\n");
    printf("\t[-h]           get this usage help\n");
    printf("\t[-l]           get a list of games in progress\n");
    printf("\t[-r]           put game window in reverse video mode\n");
    printf("\t[-b  bitmap]   use this bitmap when playing as my image\n");
    printf("\t[-bm bitmap]   use this bitmap as a mask behind my image\n");
    printf("\t[-g  game]     join the named game in progress\n\n");
  }
}



/* print list of games in progress */


print_game_list(player)
PlayerInfo *player;
{
  printf("\nGames running under %s  Version %s\n", PROGRAM_NAME,
	 PROGRAM_VERSION);
  printf("1990 By Van A. Boughner, Mel Nicholson, ");
  printf("and Albert C. Baker III\n\n");

  print_list_of_games(player);
  printf("\n");
}



/* Enter a line into the log file, add to it information concerning current
   time, game, map, etc.  Any of the parameters (except msg)  may be NULL
   pointers if that information isn't important to you at the time.
   Return TRUE if write to log file was successful. */

int log_message(msg)
char *msg;
{
I 2
  static time_t now, last = 0;
E 2
  char out[300];
  FILE *fp;

  /* if logging is not active then don't bother */
  if (!GM_ACTIVE) return FALSE;
I 2
  now = time(NULL);
E 2

D 2
  /* make the output line */
  sprintf(out, "%s \t%s", string_time(), msg);

  /* open the log file if possible */
E 2
I 2
  /* open the log file for appending, if possible */
E 2
  fp = fopen(GM_FILE, "a");
  if (fp == NULL) return FALSE;

D 2
  /* add line to the file */
  fprintf(fp, "%s\n", out);
E 2
I 2
  /* put out a time signature line if it has been more than 10 minutes
     since the last log message went out */
  if (now > last + 600) {
    last = now;
    fprintf(fp, "%s\n", string_time());
  }
E 2

D 2
  /* close the file and return success flag */
E 2
I 2
  /* make the output line and add to the file */
  fprintf(fp, "%s\n", msg);

E 2
  fclose(fp);
  return TRUE;
}
E 1
