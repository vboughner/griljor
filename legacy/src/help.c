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
#include <time.h>
#include "config.h"
#include "def.h"
#include "extern.h"


print_usage_instructions()
{
  if (am_driver) {
    printf("\nUsage: grildriver [-h] [-l] [-n] [-i info_file] ");
    printf(  "[-g game_name] [-m map_file]\n\n");
    printf("\t[-h]           get this usage help\n");
    printf("\t[-l]           get a list of games in progress\n");
    printf("\t[-n]           have driver not quit when there are no players\n");
    printf("\t               in game (this option should be used sparingly!)\n");
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
  static time_t now, last = 0;
  char out[300];
  FILE *fp;

  /* if logging is not active then don't bother */
  if (!GM_ACTIVE) return FALSE;
  now = time(NULL);

  /* open the log file for appending, if possible */
  fp = fopen(GM_FILE, "a");
  if (fp == NULL) return FALSE;

  /* put out a time signature line if it has been more than 10 minutes
     since the last log message went out */
  if (now > last + 600) {
    last = now;
    fprintf(fp, "%s\n", string_time());
  }

  /* make the output line and add to the file */
  fprintf(fp, "%s\n", msg);

  fclose(fp);
  return TRUE;
}
