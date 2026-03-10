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

/* Time handling routines */

#define TIME_MAIN

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include "config.h"
#include "def.h"

/* Global variables */

long	gametime;	/* clicks that have passed in this game */



/* variables for use in this file only */

struct timeval	initial_time;	/* what time was when we started game */
long		mark_time;	/* time when time mark was set */


/* initialize the game timer */

init_game_time()
{
  /* zero clicks passed so far */
  gametime = 0;

  /* game started at following time */
  gettimeofday(&initial_time, NULL);

  /* set the time mark for current time */
  set_time_mark();
}



/* mark the current time so that at some later time we can ask how much
   time has elapsed since we marked the time */

set_time_mark()
{
  mark_time = time(NULL);
}



/* return how many seconds it has been since we marked the time */

int query_time_marked()
{
  return (int) (time(NULL) - mark_time);
}



/* update the game time counter by comparing current time to start time */

update_game_time()
{
  struct timeval	current_time;
  time_t		seconds;
  long			microsec;

  /* get the current time */
  gettimeofday(&current_time, NULL);

  /* how many seconds have passed since game began */
  seconds = current_time.tv_sec - initial_time.tv_sec;

  /* and how many microseconds */
  microsec = current_time.tv_usec - initial_time.tv_usec;
  if (microsec < 0) {
    seconds--;
    microsec += 1000000;
  }

  /* now figure out just how many clicks that is, being careful not to
     overflow the capacity of a 32-bit integer */
  gametime = ((microsec / 1000) * CLICKS_PER_SEC / 1000) +
             (seconds * CLICKS_PER_SEC);
}




/* print number of clicks passed in game for testing purposes */

print_game_time()
{
  printf("gametime: %d clicks\n", gametime);
}
