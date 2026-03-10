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

/* Routines for handling identification numbers */

/* Each person in the game is to have a unique identification number.
   The driver assigns these numbers to players.  Id numbers are often stored
   in short integers, so they cannot exceed about 32,000.  I don't believe
   that will ever become a problem in any single game. */
   

#include "config.h"
#include "def.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"


/* Global variables inside this file */

int	nextid = SPECIAL_IDS;	/* the next id number that can be used */



/* =================== D R I V E R  use routines ==================== */

/* Figure out the next usable and unique id number, return it, and
   reserve it so that no one else uses it later. */

int alloc_id()
{
  /* increment free id number storage */
  nextid++;
  
  return (nextid - 1);
}



/* free an id number for later use (unreserve it) */

free_id()
{
  /* currently we pay no attention to this request */
}




/* ============================ M I S C ============================== */


/* given an id number, return the person array element number, or return
   -1 when no one was found by that id number */

int person_array(idnum)
int idnum;
{
  register int	i;
  int		found = -1;

  /* start at beginning and look for the id number */
  for (i=0; (i<persons_in_game) && (found == -1); i++)
    if (gameperson[i]->id == idnum) found = i;

  return found;
}
