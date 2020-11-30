h23736
s 00000/00000/00077
d D 1.2 92/08/07 01:01:41 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00077/00000/00000
d D 1.1 91/02/16 12:54:20 labc-3id 1 0
c date and time created 91/02/16 12:54:20 by labc-3id
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

/* Routines for handling identification numbers */

/* Each person in the game is to have a unique identification number.
   The driver assigns these numbers to players.  Id numbers are often stored
   in short integers, so they cannot exceed about 32,000.  I don't believe
   that will ever become a problem in any single game. */
   

#include "config.h"
#include "def.h"
#include "map.h"
#include "objects.h"
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
E 1
