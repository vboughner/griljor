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

/* Routines to deal with who you can see (all objects in the room are seen
   regardless of where you are, but other people are only seen when there
   is a direct line or near direct line of sight, perhaps missiles shouldn't
   be seen either, until you round the corner... ) */

/* The following are examples of when people should be able to see each
   other:
                      |          D                                    |
                      |                                              |
   ========================== C  ======================================
       A    B                                                     F
   ====================================   =================  
			|			          |
			|	G			  |   E
			|				  |
			|				  ============

   A, B, and F should all be able to see each other.
   C cannot see A, B, or F.
   C and D can see each other.
   F and E can see each other.
   E does not see anyone but F.
   G sees no one else.

   Generally, people cannot see in positions where they could peek around
   corners except by moving out into the hallway/around corner. */



  /* You can find out whether a square has a wall or other obstruction on
     it by calling this procedure:
     
		int has_view_obstruction(int roomnum, int x, int y)

     which returns TRUE if there is an obstruction and FALSE if not. */


#include <stdio.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "movement.h"


/* This is the routine that the main program calls to find out if one
   location is visible from another.  Return TRUE if it is, and FALSE
   if it is not. x1 y1 is the looker location, and x2 y2 is the looked
   upon spot. */

int spot_is_visible(roomnum, x1, y1, x2, y2)
int roomnum;		/* room they are in */
int x1, y1;             /* location of person 1 */
int x2, y2;           	/* location of person 2 */
{
  Location *list, *ptr;
  int visible = TRUE;

  /* find out which squares are between the two players using the normal
   * movement algorithym diagonals allowed version, but do not
   * pay any attention to square first player is on.
   * (this allows people to see out of the forest, etc.) */
  list = diag_movement_list(x1, y1, x2, y2, FALSE);
  remove_move_head(&list);

  /* Something is always visible when just one square away from you */
  if (list)
    if (list->next)  {
      /* check each spot for a visual obstruction.  Return FALSE if an 
         obstruction is found, or TRUE if the list becomes empty. */

      for (ptr = list; ptr && visible; ptr = ptr->next)
        if (has_view_obstruction(roomnum, ptr->x, ptr->y)) visible = FALSE;
    }

  /* free the movement queue from memory */
  free_move_queue(&list);

  return visible;
}




/* ================== V I S I B I L I T Y  arrays ========================= */

/* figure out the visibility of a person with respect to given person
   and place the result into given visibility array */

update_person_visibility(looker, looked_upon, array)
int looker, looked_upon;
char *array;
{
  if (gameperson[looker]->room < 0 ||
      gameperson[looker]->room >= mapstats.rooms ||
      gameperson[looker]->room != gameperson[looked_upon]->room ||
      gameperson[looked_upon]->room < 0 ||
      gameperson[looked_upon]->room >= mapstats.rooms)
		array[looked_upon] = FALSE;
  else
    array[looked_upon] = spot_is_visible(gameperson[looker]->room,
		gameperson[looker]->x, gameperson[looker]->y,
		gameperson[looked_upon]->x, gameperson[looked_upon]->y);
}



/* figure out visibilties of all persons with regard to given looker */

update_vision(looker, array)
int looker;
char *array;
{
  register int i;

  for (i=num_persons; i<persons_in_game; i++)
    update_person_visibility(looker, i, array);
}
