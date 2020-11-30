h44846
s 00000/00000/00105
d D 1.3 92/08/07 01:01:28 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00013/00012/00092
d D 1.2 92/05/27 21:45:37 vbo 2 1
c made minimal changes required for compile on Sparc2 SVR4
e
s 00104/00000/00000
d D 1.1 91/02/16 12:54:12 labc-3id 1 0
c date and time created 91/02/16 12:54:12 by labc-3id
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

/* Time handling routines */

#define TIME_MAIN

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
D 2
#include <sys/timeb.h>
E 2
I 2
#include <sys/time.h>
E 2
#include "config.h"
#include "def.h"

/* Global variables */

long	gametime;	/* clicks that have passed in this game */



/* variables for use in this file only */

D 2
struct timeb	initial_time;	/* what time was when we started game */
E 2
I 2
struct timeval	initial_time;	/* what time was when we started game */
E 2
long		mark_time;	/* time when time mark was set */


/* initialize the game timer */

init_game_time()
{
  /* zero clicks passed so far */
  gametime = 0;

  /* game started at following time */
D 2
  ftime(&initial_time);
E 2
I 2
  gettimeofday(&initial_time);
E 2

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
D 2
  struct timeb		current_time;
E 2
I 2
  struct timeval	current_time;
E 2
  time_t		seconds;
D 2
  short			millisec;
E 2
I 2
  long			microsec;
E 2

  /* get the current time */
D 2
  ftime(&current_time);
E 2
I 2
  gettimeofday(&current_time);
E 2

  /* how many seconds have passed since game began */
D 2
  seconds = current_time.time - initial_time.time;
E 2
I 2
  seconds = current_time.tv_sec - initial_time.tv_sec;
E 2

D 2
  /* and how many milliseconds? */
  millisec = current_time.millitm - initial_time.millitm;
  if (millisec < 0) {
E 2
I 2
  /* and how many microseconds */
  microsec = current_time.tv_usec - initial_time.tv_usec;
  if (microsec < 0) {
E 2
    seconds--;
D 2
    millisec += 1000;
E 2
I 2
    microsec += 1000000;
E 2
  }

  /* now figure out just how many clicks that is, being careful not to
     overflow the capacity of a 32-bit integer */
D 2
  gametime = (millisec * CLICKS_PER_SEC / 1000) + (seconds * CLICKS_PER_SEC);
E 2
I 2
  gametime = ((microsec / 1000) * CLICKS_PER_SEC / 1000) +
             (seconds * CLICKS_PER_SEC);
E 2
}




/* print number of clicks passed in game for testing purposes */

print_game_time()
{
  printf("gametime: %d clicks\n", gametime);
}
E 1
