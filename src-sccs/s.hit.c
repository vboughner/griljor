h30141
s 00000/00000/00165
d D 1.3 92/08/07 01:01:38 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00008/00004/00157
d D 1.2 91/08/29 01:39:56 vanb 2 1
c fixed up more compatibility problems
e
s 00161/00000/00000
d D 1.1 91/02/16 12:54:18 labc-3id 1 0
c date and time created 91/02/16 12:54:18 by labc-3id
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

/* Routines for registering, updating, and removing hits */

#include <stdio.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "gametime.h"
#include "queued.h"
#include "hit.h"


/* global variables for inside this file only */
int	hit_rooms;	/* number of rooms we keep track of hits for */
Hit	*hit_list;	/* array of linked lists, one for each room */


/* Initialize hit note-keeping.  We must be given the number of rooms we
   are to be responsible for. */

D 2
initialize_hit_list(int rooms)
E 2
I 2
initialize_hit_list(rooms)
int rooms;
E 2
{
  register int i;

  /* allocate memory for the array of rooms */
  hit_list = (Hit *) malloc(rooms * sizeof(Hit));
  if (hit_list == NULL) Gerror("no memory to allocate hit list");

  /* set all the dummy records so that rooms are empty of hits */
  for (i=0; i<rooms; i++) hit_list[i].next = NULL;

  /* set the global variable that remembers how many rooms we have */
  hit_rooms = rooms;
}


free_hit_list()
{
  free(hit_list);
  hit_list = NULL;
}



/* Add new hit to database.  If a hit register already exists on given
   square, then just prolong the expiration timer instead of duplicating
   hit registration. */

D 2
add_new_hit(int roomnum, int x, int y)
E 2
I 2
add_new_hit(roomnum, x, y)
int roomnum, x, y;
E 2
{
  Hit *ptr;

  /* check for out of bounds attempt */
  if (x <  0  ||  x >= ROOM_WIDTH ||
      y <  0  ||  y >= ROOM_HEIGHT ||
      roomnum < 0 || roomnum >= hit_rooms)  return;

  /* find us the hit currently on this square */
  ptr = hit_on_square(roomnum, x, y);

  /* no hit here currently, create a new one */
  if (ptr == NULL) {
    ptr = alloc_hit();
    ptr->x = x;
    ptr->y = y;
    /* put it at front of room's hit list */
    ptr->next = hit_list[roomnum].next;
    hit_list[roomnum].next = ptr;
    /* draw it for the first time */
    if (has_display) draw_hit(roomnum, x, y);
  }

  /* set timer for the future */
  ptr->timer = gametime + HIT_LINGER_CLICKS;
}



/* Remove all the old hits that have expired, this routine should be called
   every 40 clicks or so */

remove_old_hits()
{
  register int i;
  Hit *erase, *ptr, *last;

  /* go through all rooms */
  for (i=0; i<hit_rooms; i++)
    /* go through all hit registrations */
    for ((ptr = hit_list[i].next),(last = &(hit_list[i])); ptr;) {
      /* determine if this one should be erased */
      if (ptr->timer <= gametime) {
	/* remove the hit from the linked list */
	erase = ptr;
        ptr = erase->next;
	last->next = erase->next;
	/* erase the registered hit from display */
	if (has_display)
	  QDraw_priority_request(i, erase->x, erase->y, CONCEALED);
	free(erase);
      }
      else {
	/* if not erasing this one, then just go on to next one */
	last = ptr;
        ptr = ptr->next;
      }
    }
}



/* Looks for a hit on the given square, returns TRUE when there is one
   currently, returns FALSE when there is not */

D 2
is_hit_on_square(int roomnum, int x, int y)
E 2
I 2
is_hit_on_square(roomnum, x, y)
int roomnum, x, y;
E 2
{
  return (hit_on_square(roomnum, x, y) != NULL);
}



/* Looks for a hit on given square, returns pointer to the hit if there is
   one there, or return NULL when there isn't one at the given location */

D 2
Hit *hit_on_square(int roomnum, int x, int y)
E 2
I 2
Hit *hit_on_square(roomnum, x, y)
int roomnum, x, y;
E 2
{
  Hit *ptr, *result = NULL;

  /* check for out of bounds request */
  if (roomnum < 0 || roomnum >= hit_rooms) return result;

  /* look for hit on square xy in this room */
  for (ptr = hit_list[roomnum].next; ptr && !result; ptr = ptr->next)
    if (ptr->x == x && ptr->y == y) result = ptr;

  return result;
}



/* allocate a hit record in memory */

Hit *alloc_hit()
{
  Hit *new;

  new = (Hit *) malloc(sizeof(Hit));
  if (new == NULL) Gerror("no memory for a hit registration");

  return new;
}
E 1
