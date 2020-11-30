h12832
s 00000/00000/00307
d D 1.3 92/08/07 01:00:12 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00010/00006/00297
d D 1.2 91/08/26 00:32:56 vanb 2 1
c fixed up procedure defs and other compatibilty problems
e
s 00303/00000/00000
d D 1.1 91/02/16 12:53:38 labc-3id 1 0
c date and time created 91/02/16 12:53:38 by labc-3id
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

/* Routines for finding random appearance spots */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"



/* data abstraction for this file */

typedef struct _loc {
	int roomnum, x, y;
	struct _loc *next;
} Loc;



/* make one of these location things */

Loc *makeloc(roomnum, x, y, next)
int roomnum, x, y;
Loc *next;
{
  Loc *result;
  
  demand(result = (Loc *) malloc(sizeof(Loc)), "out of memory");
  result->roomnum = roomnum;
  result->x = x;
  result->y = y;
  result->next = next;

  return result;
}



/* given a list of valid locations, pick one randomly from it and return
   a ptr to it.  If you get a NULL list, return NULL.  Before returning
   free the given list. */

D 2
Loc *pickloc(Loc *l)
E 2
I 2
Loc *pickloc(l)
Loc *l;
E 2
{
  static Loc result;
  int len, pick, i;
  Loc *ptr;

  /* figure out how many we have to choose from */
  for(ptr=l,len=0; ptr; ptr=ptr->next,len++);

  /* if there are none to choose from, return NULL */
  if (!len) {
    freelocs(l);
    return NULL;
  }

  /* pick one at random */
  pick = lrand48() % len;
  for(ptr=l,i=0; (ptr && i<pick); ptr=ptr->next,i++);
  bcopy(ptr, &result, sizeof(Loc));

  freelocs(l);
  return &result;
}



/* given a list of locations, free all of the elements */

D 2
freelocs(Loc *l)
E 2
I 2
freelocs(l)
Loc *l;
E 2
{
  if (l) {
    freelocs(l->next);
    free(l);
  }
}



/* given a team number, return a room in which a person or object
   could appear, team number of -1 signifies ability to pick any team's
   room as long as the appearance code for the room allows it.  A return
   number of -1 signifies that no room could be found */

D 2
int random_team_room(int team)
E 2
I 2
int random_team_room(team)
int team;
E 2
{
  register int i;
  Loc *l = NULL, *new, *final;

  /* make a list of all the rooms we could use */
  for (i=0; i<mapstats.rooms; i++)
    if (team == -1  ||  room[i].team == team) {
      new = makeloc(i, 0, 0, l);
      l = new;
    }
      
  /* pick a random one from the list */
  final = pickloc(l);

  if (final) return (final->roomnum);
  else return -1;
}



/* given a room number, return a valid space in the room (by reference).
   The return value of the function is TRUE if it succeeds, of FALSE if
   the room won't allow appearances.  We are supposed to guarentee a
   square that does not have walls on it.  It's okay if there's something
   takeable there already.  The recorded flag should be TRUE if we are
   looking for a spot for a recorded object.  Override will be either
   PLACE_DEFAULT if you want room appearance mode to be treated as per
   normal, or it will be one of the other values if you want to override
   the room's normal appearance mode.  A value of PLACE_OVERRIDE will usually
   NOT do the right thing, for example, it will cause rooms which should
   never allow appearances to have them (on the official floor spaces). */

D 2
int random_square_in_room(int roomnum, int *gx, int *gy,
			  int recorded, int override)
E 2
I 2
int random_square_in_room(roomnum, gx, gy, recorded, override)
int roomnum, *gx, *gy, recorded, override;
E 2
{
  register int x, y, i, found;
  Loc *l = NULL, *new, *final;
  int appearance;

  if (OUT_OF_BOUNDS(roomnum, 0, 0)) return FALSE;

  /* figure out the appearance mode for this use of random_square_in_room */
  if (override == PLACE_DEFAULT) {
    appearance = room[roomnum].appearance;
  }
  else if (override == PLACE_OVERRIDE) {
    if (room[roomnum].appearance == PLACE_NEVER)
      appearance = PLACE_FLOOR_ONLY;
    else
      appearance = room[roomnum].appearance;
  }
  else {
    appearance = override;
  }

  /* if room doesn't allow appearances then return FALSE */
  if (appearance == PLACE_NEVER) return FALSE;

  /* pick a few spots blindly to see if spot picking is easy here, if we
     get a good spot, fine.  If we cannot find one right away we will
     resort to making a list to choose from (more time consuming) */
  for (i=0,found=FALSE; (i<10 && !found); i++) {
    x = lrand48() % ROOM_WIDTH;
    y = lrand48() % ROOM_HEIGHT;
    found = (((appearance == PLACE_ANYWHERE) ||
	      (appearance == PLACE_FLOOR_ONLY &&
	       (probable_floor_here(roomnum, x, y) ==
		(uc)room[roomnum].floor))) &&
	     (max_move_on_square(roomnum, x, y)) &&
	     (find_empty_slot(roomnum, x, y, recorded) >= 0));
  }
  if (found) {
    /* blind picking worked, let's return the spot found */
    *gx = x;	*gy = y;
    return TRUE;
  }
  

  /* now that the blind location picking didn't work, we're going to
     make an exahstive list of the available sqaures and then pick one
     square from it at random.  First: look for valid locations
     that we can use and make a list of them */
  for (x=0; x<ROOM_WIDTH; x++)
    for (y=0; y<ROOM_HEIGHT; y++)
      if (((appearance == PLACE_ANYWHERE) ||
 	   (appearance == PLACE_FLOOR_ONLY &&
	   (probable_floor_here(roomnum, x, y)==(uc)room[roomnum].floor))) &&
	  (max_move_on_square(roomnum, x, y)) &&
	  (find_empty_slot(roomnum, x, y, recorded) >= 0)) {
		new = makeloc(roomnum, x, y, l);
		l = new;
      }

  /* now, pick a random one from the list */
  final = pickloc(l);

  if (final) {
    *gx = final->x;
    *gy = final->y;
    return TRUE;
  }
  else return FALSE;
}



/* find a random spot on the map somewhere, return TRUE if one could be 
   found, return FALSE if not */

D 2
int random_square_in_map(int *roomnum, int *gx, int *gy, int recorded)
E 2
I 2
int random_square_in_map(roomnum, gx, gy, recorded)
int *roomnum, *gx, *gy, recorded;
E 2
{
  int tries = 0, found = FALSE;

  while (!found && tries++ < 16) {
    *roomnum = random_team_room(-1);
    found = random_square_in_room(*roomnum, gx, gy, recorded, PLACE_DEFAULT);
  }

  return found;
}



/* given a room number and an object, place object somewhere in the room,
   this routine can not be called in the middle of an active game,
   only before it starts or while editing the map (because it
   does not notify the players of the changes).  Return FALSE if
   they request an erroneous object number, or room where things may
   not be placed, otherwise return TRUE. */

initial_random_placement(objnum, roomnum)
int objnum, roomnum;
{
  int x, y, recorded;

  if (objnum >= objects) return FALSE;

  /* get a good random location */
  recorded = (info[(unsigned char)objnum]->recorded);
  if (!random_square_in_room(roomnum, &x, &y, recorded, PLACE_DEFAULT))
    return FALSE;

  /* put a copy of the object there */
  current_room = roomnum;
  if (recorded)
    put_down_recorded_obj(x, y, objnum);
  else
    put_down_mapped_obj(x, y, objnum);

  /*printf("object %d placed in room %d at %d %d\n", objnum, roomnum, x, y);*/
  return TRUE;
}



/* do an in-game random placement, like the procedure above does, except
   this one tells all the players about the changes.  Returns TRUE when
   the placement was successful. */

int random_placement(objnum, roomnum)
int objnum, roomnum;
{
  int x, y, recorded, empty_slot;
  MemObj *ptr = NULL;
  RecordedObj *rec = NULL;

  if (objnum >= objects) return FALSE;

  /* get a good random location */
  recorded = (info[(unsigned char)objnum]->recorded);
  if (!random_square_in_room(roomnum, &x, &y, recorded, PLACE_DEFAULT))
    return FALSE;

  /* put a copy of the object there */
  /* look for an empty slot on the square (recorded if neccessary) */
  empty_slot = find_empty_slot(roomnum, x, y, recorded);

  /* if no empty slot then abort this placement attempt */
  if (empty_slot == -1) return FALSE;

  /* fix up the MemObj structure with the right values */
  /* set details to the special default values */
  ptr = allocate_mem_obj();
  ptr->next = NULL;
  if (recorded) {
    ptr->obj.x = x;
    ptr->obj.y = y;
    ptr->obj.objtype = objnum;
    ptr->obj.detail = rec_obj_default(objnum, 1);
    ptr->obj.infox =  rec_obj_default(objnum, 2);
    ptr->obj.infoy =  rec_obj_default(objnum, 4);
    ptr->obj.zinger = rec_obj_default(objnum, 8);
    ptr->obj.extra[0] = rec_obj_default(objnum, 16);
    ptr->obj.extra[1] = rec_obj_default(objnum, 32);
    ptr->obj.extra[2] = rec_obj_default(objnum, 64);
    rec = &(ptr->obj);
  }

  /* place object into empty slot, telling everyone of the change */
  notify_all_of_map_change(roomnum, x, y, empty_slot, 0, objnum, rec);

  /*printf("in-game object %d placed in room %d at %d %d\n", objnum, roomnum, x, y);*/
  return TRUE;
}
E 1
