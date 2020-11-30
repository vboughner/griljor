h63624
s 00004/00003/00566
d D 1.7 92/09/05 01:26:17 vbo 7 6
c fixed problem with NULL object pointers not being recognized
e
s 00002/00001/00567
d D 1.6 92/09/03 16:55:44 vbo 6 5
c must check for NULL pointer before referencing
e
s 00239/00546/00329
d D 1.5 92/09/02 14:43:21 vbo 5 4
c major changes made to convert from use of InvObj to ObjectInstance
e
s 00000/00000/00875
d D 1.4 92/08/07 01:01:54 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00081/00026/00794
d D 1.3 91/04/28 23:22:51 labc-3id 3 2
c Van and Mels' auto take stuff
e
s 00049/00000/00771
d D 1.2 91/02/23 01:48:23 labc-3id 2 1
c 
e
s 00771/00000/00000
d D 1.1 91/02/16 12:54:28 labc-3id 1 0
c date and time created 91/02/16 12:54:28 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
D 5
 * The War of Griljor
E 5
I 5
 * %Z% %M% %I% - last change made %G%
E 5
 *
D 5
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
E 5
I 5
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
E 5
 *
D 5
 * Students of the University of California at Berkeley
 * October 1989
E 5
I 5
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
E 5
 **************************************************************************/

/* Routines for handling inventory arrays */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
I 5
#include "mapfunc.h"
E 5
#include "missile.h"
#include "socket.h"
#include "person.h"
I 5
#include "inventory.h"
E 5


/* easy access to person inventory arrays */
D 5
#define INV(num, i)	(person[num]->inventory[i].type)
#define INVREC(num, i)	(person[num]->inventory[i].record)
E 5
I 5
#define INVOI(num, i)     (person[num]->inventory_object[i])
#define INVOITYPE(num, i) (person[num]->inventory_object[i]->type)
E 5
#define FORINV(i)	for (i=0; i<INV_SIZE; i++)

/* same easy access to hand contents */
D 5
#define HAND(num, i)	(person[num]->hand[i].type)
#define HANDREC(num, i)	(person[num]->hand[i].record)
#define FORHANDS(i)	for (i=0; i<2; i++)
E 5
I 5
#define HANDOI(num, i)	    (person[num]->hand_object[i])
#define HANDOITYPE(num, i)  (person[num]->hand_object[i]->type)
#define FORHANDS(i)	for (i=0; i<NUMBER_OF_HANDS; i++)
E 5

/* macro procedures for getting info on object types */
#define TAKEABLE(x)	(info[(uc)x]->takeable)
#define WEIGHT(x)	(info[(uc)x]->weight)


/* =============
D 5
   This is what an item in an inventory looks like:

  typedef struct _invobj {
	char	type;	
	MemObj	*record;
  } InvObj;

  Array to hold inventory, and two vars to hold what's in hands, are defined
E 5
I 5
  Arrays to hold inventory, and two vars to hold what's in hands, are defined
E 5
  like this inside the person definitions:

D 5
  InvObj	inventory[INV_SIZE];
  InvObj	hand[2];
E 5
I 5
  ObjectInstance	*inventory_object[INV_SIZE];
  ObjectInstance	*hand_object[NUMBER_OF_HANDS];
E 5

D 5
  Procedures to handle inventory will be given person numbers
E 5
I 5
  Procedures to handle inventory will be given person numbers.
E 5
  ============== */


D 5
/* procedure declarations */
InvObj *copy_hand_object();
InvObj *copy_fired_object();
InvObj *create_object_copy();
E 5
I 5
/* =========================== U T I L I T I E S ========================= */
E 5

D 5


/* ============================= T A K I N G ============================ */

/* figure out the weight of the objects being carried by a certain person */

E 5
int person_burden(num)
I 5
/* figure out the weight of the objects being carried by a certain person */
E 5
int num;
{
  int i, result = 0;

  /* add up weight of all things in inventory */
  FORINV(i)
D 5
    if (INV(num, i))
      result += WEIGHT(INV(num, i));
E 5
I 5
    if (INVOI(num, i))
      result += WEIGHT(INVOITYPE(num, i));
E 5

  /* add things that are in players hands as well */
  FORHANDS(i)
D 5
    if (HAND(num, i))
      result += WEIGHT(HAND(num, i));
E 5
I 5
    if (HANDOI(num, i))
      result += WEIGHT(HANDOITYPE(num, i));
E 5
      
  return result;
}



I 5
/* ============================= T A K I N G ============================ */



static int open_inv_slot(num)
E 5
/* return the first free spot in inventory array or return -1 if there
   are no more */
D 5

int open_inv_slot(num)
E 5
int num;	/* person number */
{
  int i, found = -1;

  /* go through all, looking for an empty spot in array */
  FORINV(i)
D 5
    if (!INV(num, i)) {
E 5
I 5
    if (!INVOI(num, i)) {
E 5
      found = i;
      break;
    }

  return found;
}



I 5
static OI *first_takeable_on_square(map, roomnum, x, y)
MapInfo *map;
int roomnum, x, y;
{
  SquareRec *square;
  OI *o;

  square = get_square(&mapstats, roomnum, x, y);
  if (!square) return(NULL);

  for (o = square->first; o; o = o->next)
    if (TAKEABLE(o->type)) return(o);

  return(NULL);
}



E 5
/* add an object to the inventory list, returning the element number where
   it was added, or -1 if there was no space or persons weight requirement
D 5
   would be exceeded */
E 5
I 5
   would be exceeded.  Notify all of change to map and register the move
   with the map structures. */
E 5

D 5
int add_to_inventory(num, o)
int num;
InvObj *o;
E 5
I 5
static int add_to_inventory(roomnum, x, y, num, o)
int roomnum, x, y, num;
OI *o;
E 5
{
  int slot;
  
  /* find an empty space */
  slot = open_inv_slot(num);
D 5
  if (slot == -1) return -1;
E 5
I 5
  if (slot == -1) return(-1);
E 5

  /* check weight requirement */
  if (person_burden(num) + WEIGHT(o->type) >
      gameperson[num]->weight_limit) return -1;

D 5
  /* copy the inventory object into that spot */
  bcopy(o, &(person[num]->inventory[slot]), sizeof(InvObj));
E 5
I 5
  /* set the inventory object pointer in that spot */
  person[num]->inventory_object[slot] = o;
E 5

I 5
  /* notify all of change to map */
  printf("add_to_inventory: no change to map notify\n");

E 5
  /* return which slot was used */
  return slot;
}



I 5
/* place an object into one of the hands, by first taking the object into
   inventory, and then swapping with the hand contents, returning the
   slot number exchanged with, or -1 if there was no room for the object
   in inventory. */
E 5

D 5
/* place an object into one of the hands, wiping out what was previously
   in that hand */

add_to_hand(num, which_hand, o)
int num, which_hand;
InvObj *o;
E 5
I 5
static int add_to_hand(roomnum, x, y, num, which_hand, o)
int roomnum, x, y, num, which_hand;
OI *o;
E 5
{
D 5
  /* copy the object into the hand */
  bcopy(o, &(person[num]->hand[which_hand]), sizeof(InvObj));
}
E 5
I 5
  int slot;
E 5

I 5
  slot = add_to_inventory(roomnum, x, y, num, o);
E 5

I 5
  if (slot != -1)
    swap_hand_and_inv(num, which_hand, slot);
E 5

D 5

/* remove an object from a certain square */

remove_from_square(roomnum, x, y, rem_obj)
int roomnum, x, y, rem_obj;
{
  int i, found = -1;
  unsigned char *stuff_here;

  /* find out what is on this square */
  if (DEBUG) printf("Removing obj %d from square %d %d\n", rem_obj, x, y);
  stuff_here = whats_on_square(roomnum, x, y);

  /* get number of first object of same type on square */
  for (i=0; i<ROOM_DEPTH + 1; i++)
    if (stuff_here[i] == rem_obj) {
      found = i;
      break;
    }

  /* if not found then return */
  if (found == -1) return;

  /* notify all about the change and change it for myself as well */
  notify_all_of_map_change(roomnum, x, y, found, rem_obj, 0, NULL);
E 5
I 5
  return(slot);
E 5
}



D 3
/* take the object in the square where a certain person is standing, return
   the inventory array element number that was filled or -1 if take failed */
E 3
I 3
D 5
/* take the object in a square in the given slot number, and return
   the inventory array element number that was filled or -1 if take failed.
   If space_slot passed is negative, instead of 0, 1, or 2, then the take
   will be of whatever on that square happens to be the most takeable. */
E 3

D 3
int take_object_into_inventory(num)
int num;
E 3
I 3
int take_object_into_inventory(roomnum, x, y, pnum, space_slot)
int roomnum, x, y, pnum, space_slot;
E 3
{
  MemObj *take_obj;
  InvObj inv_obj;
D 3
  int    slot;
E 3
I 3
  int    otype, slot;	/* inventory slot number */
E 3
  
D 3
  /* find out what is here to take */
  take_obj = takeable_on_square(gameperson[num]->room, gameperson[num]->x,
				gameperson[num]->y);
E 3
I 3
  if (space_slot < 0 || space_slot > ROOM_DEPTH) {
    /* find out what is here to take */
    take_obj = takeable_on_square(roomnum, x, y);
E 3

D 3
  /* if there is nothing there to take then return quietly */
  if (take_obj == NULL) return -1;
E 3
I 3
    /* if there is nothing there to take then return quietly */
    if (take_obj == NULL) return -1;
E 3

D 3
  /* now we add a copy of it to our inventory */
  inv_obj.type = take_obj->obj.objtype;
  if (info[(uc)inv_obj.type]->recorded) {
    inv_obj.record = allocate_mem_obj();
    bcopy(take_obj, inv_obj.record, sizeof(MemObj));
E 3
I 3
    /* now we add a copy of it to our inventory */
    inv_obj.type = take_obj->obj.objtype;
    if (info[(uc)inv_obj.type]->recorded) {
      inv_obj.record = allocate_mem_obj();
      bcopy(take_obj, inv_obj.record, sizeof(MemObj));
    }
    else inv_obj.record = NULL;
    slot = add_to_inventory(pnum, &inv_obj);
    otype = take_obj->obj.objtype;
E 3
  }
D 3
  else inv_obj.record = NULL;
  slot = add_to_inventory(num, &inv_obj);
E 3
I 3
  else {
    /* find out what type of object we are taking */
    unsigned char *whats_here = whats_on_square(roomnum, x, y);
    otype = whats_here[space_slot];
    if (!info[otype]->takeable) return -1;
E 3

D 3
  /* if we can add it to inventory then remove it from square */
  if (slot != -1)
    remove_from_square(gameperson[num]->room, gameperson[num]->x,
		       gameperson[num]->y, take_obj->obj.objtype);
E 3
I 3
    /* add it to our inventory */
    inv_obj.type = otype;
    if (info[(uc)inv_obj.type]->recorded) {
      MemObj *ptr = what_recorded_obj_here(roomnum, x, y);
      if (!ptr) return -1;
      inv_obj.record = allocate_mem_obj();
      bcopy(ptr, inv_obj.record, sizeof(MemObj));
    }
    else inv_obj.record = NULL;
E 3

D 3
  /* return which slot in inventory was updated */
E 3
I 3
    slot = add_to_inventory(pnum, &inv_obj);
  }

  /* if we could add it to inventory then remove it from square */
  if (slot != -1) remove_from_square(roomnum, x, y , otype);

  /* return which slot in inventory was updated, if any */
E 3
  return slot;
}



/* take an object in the square into one of the hands, putting the previous
   contents of the hands into the inventory, return element of inventory  
   array that was changed or -1 if take failed */

D 3
int take_object_into_hand(num, which_hand)
int num, which_hand;
E 3
I 3
int take_object_into_hand(roomnum, x, y, pnum, space_slot, which_hand)
int roomnum, x, y, pnum, space_slot, which_hand;
E 3
{
  int slot;
  
  /* take the object into inventory first */
D 3
  slot = take_object_into_inventory(num);
E 3
I 3
  slot = take_object_into_inventory(roomnum, x, y, pnum, space_slot);
E 3

  /* now swap contents of hand with the inventory slot */
D 3
  if (slot != -1) swap_hand_and_inv(num, which_hand, slot);
E 3
I 3
  if (slot != -1) swap_hand_and_inv(pnum, which_hand, slot);
E 3

  return slot;
}

I 3


E 5
/* This routine decides whether an object should be taken into the
   hand or into the inventory and it picks it up off the square.
   It requires the location room, x, y, the person picking it
D 5
   up, and the slot number on the square that is to be picked up.
   Returns element of inventory array that was changed, or -1 if the
   take failed.  Pass negative slot number instead of 0, 1, or 2, if
   you want the object from square to pick up to be selected by the
   algorithms in this file.  'intohand' must be either -1, 0, or 1.
E 5
I 5
   up.  Returns element of inventory array that was changed, or -1 if the
   take failed.  'intohand' must be either -1, 0, or 1.
E 5
   -1 signifies the object is going straight into inventory, unless the
   object includes a force into hand flag. */

D 5
int take_object_from_ground(roomnum, x, y, pnum, space_slot, intohand)
int roomnum, x, y, pnum, space_slot, intohand;
E 5
I 5
int take_object_from_ground(roomnum, x, y, pnum, intohand)
int roomnum, x, y, pnum, intohand;
E 5
{
D 5
  int *whats_here, otype;
E 5
I 5
  OI *o;
E 5
  int slot;

D 5
  if (OUT_OF_BOUNDS(roomnum, x, y)) return -1;
  if (intohand < -1 || intohand > 1) return -1;
E 5
I 5
  if (OUT_OF_BOUNDS(roomnum, x, y)) return(-1);
  if (intohand < -1 || intohand >= NUMBER_OF_HANDS) return(-1);
E 5

D 5
  /* the ideas from get_object_from_ground need to be incorporated
     right here ======================= */
  /* check to see if there is a specific slot mentioned, if so, and
     space_slot is specified (not -1) then check the property of the
     object to be picked up for intohandedness */
E 5
I 5
  /* figure out which object the player should be trying to pick up here */
  o = first_takeable_on_square(&mapstats, roomnum, x, y);
  if (!o) return(-1);
E 5

  if (intohand == -1)
D 5
    slot = take_object_into_inventory(roomnum, x, y, pnum, space_slot);
E 5
I 5
    slot = add_to_inventory(roomnum, x, y, pnum, o);
E 5
  else
D 5
    slot = take_object_into_hand(roomnum, x, y, pnum, space_slot, intohand);
E 5
I 5
    slot = add_to_hand(roomnum, x, y, pnum, intohand, o);
E 5

  return slot;
}
E 3



I 5

E 5
/* ========================= S W A P P I N G ============================= */


/* swap the contents of a hand with an inventory slot */

D 5
swap_hand_and_inv(num, which_hand, slot)
E 5
I 5
void swap_hand_and_inv(num, which_hand, slot)
E 5
int num, which_hand, slot;
{
D 5
  InvObj temp;
E 5
I 5
  OI *tmp;
E 5

D 5
  /* load old contents of hand into temp */
  if (DEBUG) printf("Swap hand %d and inventory %d\n", which_hand, slot);
  bcopy(&(person[num]->hand[which_hand]), &temp, sizeof(InvObj));
E 5
I 5
  tmp = person[num]->inventory_object[slot];
  person[num]->inventory_object[slot] = person[num]->hand_object[which_hand];
  person[num]->hand_object[which_hand] = tmp;
E 5

D 5
  /* copy contents of inventory slot into hand */
  add_to_hand(num, which_hand, &(person[num]->inventory[slot]));

  /* copy temp into inventory slot */
  bcopy(&temp, &(person[num]->inventory[slot]), sizeof(InvObj));
E 5
I 5
  printf("swap_hand_and_inv: no map notify yet\n");
E 5
}



/* ========================= D R O P P I N G ============================= */

D 5
/* Given the inventory square number, attempt to drop the object there onto
E 5
I 5
/* add an inventory object to a map square, return TRUE if successful */

static int add_to_square(roomnum, x, y, o)
int roomnum, x, y;
OI *o;
{
  /* call mapfunc routine to change object's location? */

  /* place object into empty slot */
  /* notify_all_of_map_change(roomnum, x, y, empty_slot, 0,
			   inv_obj->type, inv_obj->record); */

  return TRUE;
}



/* Given the inventory slot number, attempt to drop the object there onto
E 5
   the given person's current square, return TRUE if object was dropped */

D 5
int drop_object_from_inv(person_num, inv_num)
int person_num, inv_num;
E 5
I 5
int drop_object_from_inv(person_num, slot)
int person_num, slot;
E 5
{
D 5
  MemObj *takeable;
  int    result;
E 5
I 5
  OI *o;
  int result;
E 5
  
  if (DEBUG)
D 5
    printf("Dropping inv item %d from person %d\n", inv_num, person_num);
E 5
I 5
    printf("Dropping inv item %d from person %d\n", slot, person_num);
E 5

  /* if item to drop is a blank (obj #0) then ignore drop request */
D 5
  if (INV(person_num, inv_num) == 0) return FALSE;
E 5
I 5
  o = INVOI(person_num, slot);
  if (!o) return(FALSE);
E 5

D 5
  /* make sure that square is empty of takeable objects */
  takeable = takeable_on_square(gameperson[person_num]->room,
	     gameperson[person_num]->x,	gameperson[person_num]->y);

E 5
  /* when we can, we attempt to add the object to the immediate square */
D 5
  if (!takeable)
    result = add_to_square(gameperson[person_num]->room,
		gameperson[person_num]->x, gameperson[person_num]->y,
		&(person[person_num]->inventory[inv_num]));
E 5
I 5
  result = add_to_square(gameperson[person_num]->room,
		gameperson[person_num]->x, gameperson[person_num]->y, o);
E 5

D 5
  if (takeable || (!takeable && !result)) {
    /* if there is something already there then we try to drop nearby */
    result = drop_object_nearby(person_num, 
				&(person[person_num]->inventory[inv_num]));
  }

E 5
  /* if object was successfully dropped then remove item from our inventory */
D 5
  if (result) remove_from_inv(person_num, inv_num, FALSE);
E 5
I 5
  if (result) person[person_num]->inventory_object[slot] = NULL;
E 5

D 5
  return result;
E 5
I 5
  return(result);
E 5
}



I 5
/* locate a square near a location that could hold an object, return TRUE if
   we could successfully find one.  */
E 5

D 5
/* add an inventory object to a map square, return TRUE if successful */

int add_to_square(roomnum, x, y, inv_obj)
int roomnum, x, y;
InvObj *inv_obj;
E 5
I 5
static int locate_empty_square(roomnum, loc_x, loc_y, ret_room, ret_x, ret_y)
int roomnum, loc_x, loc_y, *ret_room, *ret_x, *ret_y;
E 5
{
D 5
  int recorded, empty_slot;
E 5
I 5
  int x, y, radius = 1, unsuccessful = 0, again = 0;
  int done = FALSE, tried_obvious = FALSE;
E 5

D 5
  /* see if the object is a recorded one */
  recorded = info[(uc)inv_obj->type]->recorded;
E 5
I 5
  /* look for a blank square in the given radius until we find one
     or we are unsuccessful enough to want to increase the search radius */
  do {
    do {
      do {
        /* get a random square */
	if (!tried_obvious) {
	  x = loc_x;
	  y = loc_y;
	  tried_obvious = TRUE;
	}
	else {
 	  x = loc_x + (lrand48() % (radius * 2 + 1)) - radius;
          y = loc_y + (lrand48() % (radius * 2 + 1)) - radius;
	}
E 5

D 5
  /* look for an empty slot on the square (recorded if neccessary) */
  empty_slot = find_empty_slot(roomnum, x, y, recorded);
E 5
I 5
        /* if it is a good random square then we are done */
	if (OUT_OF_BOUNDS(roomnum, x, y)) done = FALSE;
        else done = (spot_is_visible(roomnum, loc_x, loc_y, x, y) &&
		     (max_move_on_square(roomnum, x, y) > 0) &&
		     (!exit_char_on_square(roomnum, x, y)));
E 5

D 5
  /* if no empty slot then abort this placement attempt */
  if (empty_slot == -1) return FALSE;
E 5
I 5
        if (!done) unsuccessful++;
      } while (!done && unsuccessful < 4);
E 5

D 5
  /* change x and y in recorded object record */
  if (recorded) {
    inv_obj->record->obj.x = x;
    inv_obj->record->obj.y = y;
E 5
I 5
      radius += 2;
      unsuccessful = 0;
    } while (!done && radius < 18);

    radius = 2;
    again ++;
  } while (!done && again < 2);

  /* if we couldn't find a nearby square, use the one given as center */
  if (!done) {
    x = loc_x;
    y = loc_y;
    done = TRUE;
E 5
  }

D 5
  /* place object into empty slot */
  notify_all_of_map_change(roomnum, x, y, empty_slot, 0,
			   inv_obj->type, inv_obj->record);
E 5
I 5
  *ret_room = roomnum;
  *ret_x = x;
  *ret_y = y;
E 5

D 5
  return TRUE;
E 5
I 5
  return(done);
E 5
}


I 2
D 5
/* add an inventory object to a map square, return TRUE if successful */
E 5
E 2

I 2
D 5
int add_type_to_square(roomnum, x, y, type)
E 5
I 5
D 7
/* Given an object instance, find a nearby spot on the map to drop it */
E 7
I 7
/* Given an object instance, find a nearby spot on the map to drop it.
   Returns TRUE if the object dropped, FALSE if it wasn't. */
E 7

static int drop_object_nearby(roomnum, x, y, o)
E 5
int roomnum, x, y;
D 5
int type;
E 5
I 5
OI *o;
E 5
{
D 5
  int empty_slot;
E 5
I 5
  int new_roomnum, new_x, new_y, result;
E 5

D 5
  /* look for an empty slot on the square (recorded if neccessary) */
  empty_slot = find_empty_slot(roomnum, x, y, FALSE);
E 5
I 5
D 7
  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (o->type == 0) return FALSE;
E 7
I 7
  /* if no item to drop, then ignore drop request */
  if (!o) return(FALSE);
E 7
E 5

D 5
  /* if no empty slot then abort this placement attempt */
  if (empty_slot == -1) return FALSE;
E 5
I 5
  /* get a nearby empty square to drop something on */
  if (!locate_empty_square(roomnum, x, y, &new_roomnum, &new_x, &new_y))
    return(FALSE);
E 5

D 5
  /* place object into empty slot */
  notify_all_of_map_change(roomnum, x, y, empty_slot, 0,
			   type, FALSE);
E 5
I 5
  result = add_to_square(new_roomnum, new_x, new_y, o);
E 5

D 5
  return TRUE;
E 5
I 5
  return(result);
E 5
}


E 2
D 5
/* remove an object from person's inventory.  If you are given TRUE in the
   to_free variable then free the inventory object pointed to after
   removing it from inventory */
E 5

D 5
remove_from_inv(person_num, inv_num, to_free)
int person_num, inv_num, to_free;
{
  /* see if inv_num inventory spot contains something */
  if (INV(person_num, inv_num) == 0) return;

  /* free the inventory object if requested and object was a recorded obj */
  if (to_free && info[(uc)INV(person_num, inv_num)]->recorded)
    free(INVREC(person_num, inv_num));
    
  /* delete the object by making it into object zero */
  INV(person_num, inv_num) = 0;
  INVREC(person_num, inv_num) = NULL;
}



E 5
/* drop everything a person has onto the nearest squares */

D 5
drop_everything(pnum)
E 5
I 5
void drop_everything(pnum)
E 5
int pnum;
{
D 5
  register int i;
E 5
I 5
  int i;
E 5

  FORHANDS(i)
D 5
    if (drop_object_nearby(pnum, &(person[pnum]->hand[i]))) {
      if (info[(uc)HAND(pnum, i)]->recorded) free(HANDREC(pnum, i));
      HAND(pnum, i) = 0;
      HANDREC(pnum, i) = NULL;
E 5
I 5
    if (drop_object_nearby(gameperson[pnum]->room, gameperson[pnum]->x,
			   gameperson[pnum]->y, HANDOI(pnum, i))) {
      person[pnum]->hand_object[i] = NULL;
E 5
    }

  for (i=0; i<INV_SIZE; i++)
D 5
    if (drop_object_nearby(pnum, &(person[pnum]->inventory[i])))
      remove_from_inv(pnum, i, FALSE);
E 5
I 5
    if (drop_object_nearby(gameperson[pnum]->room, gameperson[pnum]->x,
			   gameperson[pnum]->y, INVOI(pnum, i))) {
      person[pnum]->inventory_object[i] = NULL;
    }
E 5
}



D 5

/* Given a person and a pointer to an inventory record, drop the object
   onto a nearby empty square, return TRUE if object was dropped */

int drop_object_nearby(person_num, item)
int person_num;
InvObj *item;
{
  MemObj *takeable;
  int    roomnum, x, y, result, rec;
  
  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (item->type == 0) return FALSE;

  /* figure out whether this is a recorded object or not */
  rec = info[(uc)item->type]->recorded;

  do {
    /* get a nearby empty square to drop something on */
    if (locate_empty_square(gameperson[person_num]->room,
        gameperson[person_num]->x, gameperson[person_num]->y,
	&roomnum, &x, &y, rec) == FALSE) return FALSE;

    /* now we attempt to add the object to the square */
    result = add_to_square(roomnum, x, y, item);
  } while (!result);
  
  return result;
}



/* Given a pointer to an inventory record, and a location, drop the object
   onto a nearby empty square, return TRUE if object was dropped */

int drop_object_near_square(item, roomnum, x, y)
InvObj *item;
int roomnum, x, y;
{
  MemObj *takeable;
  int    drop_room, drop_x, drop_y, result, rec, tries = 0;
  
  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (item->type == 0) return FALSE;

  /* figure out whether this is a recorded object or not */
  rec = info[(uc)item->type]->recorded;

  do {
    /* get a nearby empty square to drop something on */
    if (locate_empty_square(roomnum, x, y,
			    &drop_room, &drop_x, &drop_y, rec) == FALSE)
	return FALSE;

    /* now we attempt to add the object to the square */
    result = add_to_square(drop_room, drop_x, drop_y, item);
I 2
    tries++;
  } while (!result && tries < 10);
  
  return result;
}



E 5
I 5
 
E 5
/* Given an object type and a location, create the object at that square
D 5
   onto a nearby empty square, return TRUE if object was dropped */
E 5
I 5
   onto a nearby empty square, return TRUE if object was dropped. Used by
   missile.c code */
E 5

int drop_type_near_square(type, roomnum, x, y)
int type;
int roomnum, x, y;
{
D 5
  int    drop_room, drop_x, drop_y, result, tries = 0;
  
E 5
  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (type == 0) return FALSE;

D 5
  do {
    /* get a nearby empty square to drop something on */
    if (locate_empty_square(roomnum, x, y,
			    &drop_room, &drop_x, &drop_y, FALSE) == FALSE)
	return FALSE;

    /* now we attempt to add the object to the square */
    result = add_type_to_square(drop_room, drop_x, drop_y, type);
E 2
    tries++;
  } while (!result && tries < 10);
  
  return result;
E 5
I 5
  printf("drop_type_near_square: not implemented\n");
  return(FALSE);
E 5
}



/* lose an object from your hand and reload one from inventory if such
   an object exists there.  Return the slot that was used, or -1 if there
   was nothing to reload with. */

D 5
remove_with_reload(pnum, which_hand)
E 5
I 5
int remove_with_reload(pnum, which_hand)
E 5
int pnum, which_hand;
{
  int slot;

  /* look for another copy */
D 5
  slot = find_object_in_inventory(pnum, person[pnum]->hand[which_hand].type);
E 5
I 5
  slot = find_object_in_inventory(pnum, HANDOITYPE(pnum, which_hand));;
E 5

  /* erase hand contents */
D 5
  HAND(pnum, which_hand) = 0;
  if (HANDREC(pnum, which_hand)) free(HANDREC(pnum, which_hand));
  HANDREC(pnum, which_hand) = NULL;
E 5
I 5
  person[pnum]->hand_object[which_hand] = NULL;
E 5

  /* swap empty hand with inventory copy */
  if (slot != -1) swap_hand_and_inv(pnum, which_hand, slot);

D 5
  return slot;
E 5
I 5
  return(slot);
E 5
}



/* drop an object from one of a person's hands and reload another object
   of the same type into the hand if possible.  Return the inventory
   slot number that was changed by the process. */

int drop_out_of_hand(pnum, which_hand)
int pnum, which_hand;
{
  int slot;

  /* look for another copy */
D 5
  slot = find_object_in_inventory(pnum, person[pnum]->hand[which_hand].type);
E 5
I 5
  slot = find_object_in_inventory(pnum, HANDOITYPE(pnum, which_hand));
E 5

  /* if there are no other copies of this kind of object, pick an empty slot */
  if (slot < 0) slot = open_inv_slot(pnum);

  /* if there were no empty slots, use slot #0 */
  if (slot < 0) slot = 0;

  /* swap the slot with hand content and drop what was in the hand */
  swap_hand_and_inv(pnum, which_hand, slot);
  drop_object_from_inv(pnum, slot);
D 5
  return slot;
E 5
I 5
  return(slot);
E 5
}


D 5
/* ===================== E M P T Y  square checks ======================= */
E 5

D 5
/* locate a square near a location that could hold an object, return TRUE if
   we could successfully find one.  The recorded flag tells us whether we
   are looking for a space free of other recorded objects or not */
E 5
I 5
/* =========================== U T I L I T I E S ========================= */
E 5

D 5
int locate_empty_square(roomnum, loc_x, loc_y,
			ret_room, ret_x, ret_y, recorded)
int roomnum, loc_x, loc_y, *ret_room, *ret_x, *ret_y, recorded;
{
  int x, y, radius = 1, unsuccessful = 0, again = 0;
  int done = FALSE, tried_obvious = FALSE;
E 5

D 5
  /* look for a blank square in the given radius until we find one
     or we are unsuccessful enough to want to increase the search radius */
  do {
    do {
      do {
        /* get a random square */
	if (!tried_obvious) {
	  x = loc_x;
	  y = loc_y;
	  tried_obvious = TRUE;
	}
	else {
 	  x = loc_x + (lrand48() % (radius * 2 + 1)) - radius;
          y = loc_y + (lrand48() % (radius * 2 + 1)) - radius;
	}

        /* if it is a good random square then we are done */
	if (OUT_OF_BOUNDS(roomnum, x, y)) done = FALSE;
        else done = (square_is_empty(roomnum, x, y, recorded) &&
		     spot_is_visible(roomnum, loc_x, loc_y, x, y) &&
		     max_move_on_square(roomnum, x, y) > 0);

        if (!done) unsuccessful++;
      } while (!done && unsuccessful < 4);

      radius += 2;
      unsuccessful = 0;
    } while (!done && radius < 18);

    radius = 2;
    again ++;
  } while (!done && again < 2);

  /* if we couldn't find a nearby square, try one in the room somewhere */
  if (!done)
    done = random_square_in_room(roomnum, &x, &y, recorded, PLACE_OVERRIDE);

  /* if still haven't found one, look around the map */
  if (!done)
    done = random_square_in_map(ret_room, ret_x, ret_y, recorded);
  else {
    *ret_room = roomnum;
    *ret_x = x;
    *ret_y = y;
  }

  return done;
}



/* check that a square can be moved onto, is not an exit character, and has
   an empty space on it.  Return TRUE if all of the above conditions are
   met.  The recorded flag tells us whether we are looking for space for a
   recorded object or a mapped one. */

int square_is_empty(roomnum, x, y, recorded)
int roomnum, x, y, recorded;
{
  /* check that square is legal */
  if (roomnum < 0 || roomnum >= mapstats.rooms || x < 0 || y< 0 ||
      x >= ROOM_WIDTH || y >= ROOM_WIDTH) return FALSE;

  /* check that square can be moved onto */
  if (max_move_on_square(roomnum, x, y) == 0) return FALSE;

  /* check that square is free of takeable objects (**no longer important**) */
  /* if (takeable_on_square(roomnum, x, y)) return FALSE; */

  /* check for exit character on square */
  if (exit_char_on_square(roomnum, x, y)) return FALSE;

  /* check for an open slot on the square */
  if (find_empty_slot(roomnum, x, y, recorded) == -1) return FALSE;

  /* all tests were passed, there is space on this square */
  return TRUE;
}



/* ===================== B O O K - K E E P I N G ========================= */

/* copy the data record concerning an object in someone's hand and 
   return a pointer to the newly allocated record */

InvObj *copy_hand_object(pnum, which_hand)
int pnum, which_hand;
{
  InvObj *new;

  new = (InvObj *) malloc(sizeof(InvObj));
  if (new == NULL) Gerror("no memory for copy of hand object");

  bcopy(&(person[pnum]->hand[which_hand]), new, sizeof(InvObj));

  if (new->record != NULL) {
    new->record = allocate_mem_obj();
    bcopy(person[pnum]->hand[which_hand].record, new->record, sizeof(MemObj));
  }

  return new;
}



E 5
/* copy object in someone's hand, unless that object fires a different kind
   of object, in which case, create a copy of the fired object */

D 5
InvObj *copy_fired_object(pnum, which_hand)
E 5
I 5
OI *copy_fired_object(pnum, which_hand)
E 5
int pnum, which_hand;
{
  int type;
D 5
  InvObj *result;
E 5
I 5
  OI *result;
E 5

D 5
  type = HAND(pnum, which_hand);
E 5
I 5
  type = HANDOITYPE(pnum, which_hand);
E 5

D 5
  if (info[(uc)type]->movingobj == type)
    result =  copy_hand_object(pnum, which_hand);
E 5
I 5
  if (info[(uc)type]->movingobj == type) {
    result =  duplicate_object(&mapstats, HANDOI(pnum, which_hand));
    printf("copy_fired_object: warning: duplicate_object does not net well\n");
  }
E 5
  else 	/* create a copy */
D 5
    result =  create_object_copy(info[(uc)type]->movingobj);
E 5
I 5
    result = create_object(&mapstats, info[(uc)type]->movingobj, ANY_ID);
E 5

D 5
  return result;
E 5
I 5
  return(result);
E 5
}



D 5
/* create an inventory record copy of a certain kind of object */

InvObj *create_object_copy(type)
int type;
{
  InvObj *new;
  MemObj *newobj;

  new = (InvObj *) malloc(sizeof(InvObj));
  if (new == NULL) Gerror("no memory for created object copy");

  new->type = type;

  if (!info[(uc)type]->recorded)
    /* not a recorded object, so no extra data record is allocated */
    new->record = NULL;
  else {
    /* get an memory object record and initialize it */
    newobj = allocate_mem_obj();
    newobj->obj.x = -1;
    newobj->obj.y = -1;
    newobj->obj.objtype = type;
    newobj->obj.detail = -1;
    newobj->obj.infox = -1;
    newobj->obj.infoy = -1;
    newobj->obj.zinger = -1;
    new->record = newobj;
  }

  return new;
}



/* free an inventory object from memory */

free_inv_object(obj)
InvObj *obj;
{
  if (obj->record) free(obj->record);
  free(obj);
}



E 5
/* reduce the count on an object's uses left */

D 5
reduce_count(obj, amount)
InvObj *obj;
E 5
I 5
void reduce_count(obj, amount)
OI *obj;
E 5
int amount;
{
D 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record)
    if (obj->record->obj.detail > 0) {
      obj->record->obj.detail -= amount;
      if (obj->record->obj.detail < 0) obj->record->obj.detail = 0;
E 5
I 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    if (get_record(obj, REC_DETAIL) > 0) {
      set_record(obj, REC_DETAIL, (get_record(obj, REC_DETAIL) - amount));
      if (get_record(obj, REC_DETAIL) < 0) set_record(obj, REC_DETAIL, 0);
E 5
    }
}



/* increase the number of charges an item has */

D 5
increase_count(obj, amount)
InvObj *obj;
E 5
I 5
void increase_count(obj, amount)
OI *obj;
E 5
int amount;
{
D 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record) {
    obj->record->obj.detail += amount;
    if (obj->record->obj.detail > info[(uc)obj->type]->capacity &&
E 5
I 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded) {
    set_record(obj, REC_DETAIL, (get_record(obj, REC_DETAIL) + amount));
    if ((get_record(obj, REC_DETAIL) > info[(uc)obj->type]->capacity) &&
E 5
	info[(uc)obj->type]->capacity)
D 5
		obj->record->obj.detail = info[(uc)obj->type]->capacity;
E 5
I 5
      set_record(obj, REC_DETAIL, info[(uc)obj->type]->capacity);
E 5
  }
}



/* figure out how many charges are left in item.  If this is not a numbered
   item then return zero */

int query_count(obj)
D 5
InvObj *obj;
E 5
I 5
OI *obj;
E 5
{
D 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record)
    return (obj->record->obj.detail);
E 5
I 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    return(get_record(obj, REC_DETAIL));
E 5
  else
D 5
    return 0;
E 5
I 5
    return(0);
E 5
}




/* return TRUE if these two objects are of compatible types */

int types_match(type1, type2)
int type1, type2;
{
  return (type1 & type2);
}



/* return TRUE if these two id numbers are compatible */

int ids_compatible(id1, id2)
int id1, id2;
{
  return (id1 == id2 || !id1 || !id2 || id1 == ((id2 / ID_MATCH) * ID_MATCH) ||
	  id2 == ((id1 / ID_MATCH) * ID_MATCH));
}



/* return TRUE if there is still some uses left in given object.  FALSE
   will only be returned when it is numbered and has zero left. */

D 5
has_uses_left(obj)
InvObj *obj;
E 5
I 5
int has_uses_left(obj)
OI *obj;
E 5
{
  int result = TRUE;

D 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record)
    result = (obj->record->obj.detail > 0);
E 5
I 5
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    result = (get_record(obj, REC_DETAIL) > 0);
E 5

D 5
  return result;
E 5
I 5
  return(result);
E 5
}



/* look for object of given type in a person's inventory, return the slot
   number if you find, or return -1 if you can't find one */

int find_object_in_inventory(pnum, type)
int pnum, type;
{
D 5
  register int i, found = -1;
E 5
I 5
  int i, found = -1;
E 5

  for (i=0; i<INV_SIZE && found == -1; i++)
D 5
    if (person[pnum]->inventory[i].type == type) found = i;
E 5
I 5
D 6
    if (INVOITYPE(pnum, i) == type) found = i;
E 6
I 6
    if (INVOI(pnum, i))
      if (INVOITYPE(pnum, i) == type) found = i;
E 6
E 5

D 5
  return found;
E 5
I 5
  return(found);
E 5
}
E 1
