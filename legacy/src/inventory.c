/***************************************************************************
 * @(#) inventory.c 1.7 - last change made 09/05/92
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
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
 **************************************************************************/

/* Routines for handling inventory arrays */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "mapfunc.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "inventory.h"


/* easy access to person inventory arrays */
#define INVOI(num, i)     (person[num]->inventory_object[i])
#define INVOITYPE(num, i) (person[num]->inventory_object[i]->type)
#define FORINV(i)	for (i=0; i<INV_SIZE; i++)

/* same easy access to hand contents */
#define HANDOI(num, i)	    (person[num]->hand_object[i])
#define HANDOITYPE(num, i)  (person[num]->hand_object[i]->type)
#define FORHANDS(i)	for (i=0; i<NUMBER_OF_HANDS; i++)

/* macro procedures for getting info on object types */
#define TAKEABLE(x)	(info[(uc)x]->takeable)
#define WEIGHT(x)	(info[(uc)x]->weight)


/* =============
  Arrays to hold inventory, and two vars to hold what's in hands, are defined
  like this inside the person definitions:

  ObjectInstance	*inventory_object[INV_SIZE];
  ObjectInstance	*hand_object[NUMBER_OF_HANDS];

  Procedures to handle inventory will be given person numbers.
  ============== */


/* =========================== U T I L I T I E S ========================= */

int person_burden(num)
/* figure out the weight of the objects being carried by a certain person */
int num;
{
  int i, result = 0;

  /* add up weight of all things in inventory */
  FORINV(i)
    if (INVOI(num, i))
      result += WEIGHT(INVOITYPE(num, i));

  /* add things that are in players hands as well */
  FORHANDS(i)
    if (HANDOI(num, i))
      result += WEIGHT(HANDOITYPE(num, i));
      
  return result;
}



/* ============================= T A K I N G ============================ */



static int open_inv_slot(num)
/* return the first free spot in inventory array or return -1 if there
   are no more */
int num;	/* person number */
{
  int i, found = -1;

  /* go through all, looking for an empty spot in array */
  FORINV(i)
    if (!INVOI(num, i)) {
      found = i;
      break;
    }

  return found;
}



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



/* add an object to the inventory list, returning the element number where
   it was added, or -1 if there was no space or persons weight requirement
   would be exceeded.  Notify all of change to map and register the move
   with the map structures. */

static int add_to_inventory(roomnum, x, y, num, o)
int roomnum, x, y, num;
OI *o;
{
  int slot;
  
  /* find an empty space */
  slot = open_inv_slot(num);
  if (slot == -1) return(-1);

  /* check weight requirement */
  if (person_burden(num) + WEIGHT(o->type) >
      gameperson[num]->weight_limit) return -1;

  /* set the inventory object pointer in that spot */
  person[num]->inventory_object[slot] = o;

  /* notify all of change to map */
  printf("add_to_inventory: no change to map notify\n");

  /* return which slot was used */
  return slot;
}



/* place an object into one of the hands, by first taking the object into
   inventory, and then swapping with the hand contents, returning the
   slot number exchanged with, or -1 if there was no room for the object
   in inventory. */

static int add_to_hand(roomnum, x, y, num, which_hand, o)
int roomnum, x, y, num, which_hand;
OI *o;
{
  int slot;

  slot = add_to_inventory(roomnum, x, y, num, o);

  if (slot != -1)
    swap_hand_and_inv(num, which_hand, slot);

  return(slot);
}



/* This routine decides whether an object should be taken into the
   hand or into the inventory and it picks it up off the square.
   It requires the location room, x, y, the person picking it
   up.  Returns element of inventory array that was changed, or -1 if the
   take failed.  'intohand' must be either -1, 0, or 1.
   -1 signifies the object is going straight into inventory, unless the
   object includes a force into hand flag. */

int take_object_from_ground(roomnum, x, y, pnum, intohand)
int roomnum, x, y, pnum, intohand;
{
  OI *o;
  int slot;

  if (OUT_OF_BOUNDS(roomnum, x, y)) return(-1);
  if (intohand < -1 || intohand >= NUMBER_OF_HANDS) return(-1);

  /* figure out which object the player should be trying to pick up here */
  o = first_takeable_on_square(&mapstats, roomnum, x, y);
  if (!o) return(-1);

  if (intohand == -1)
    slot = add_to_inventory(roomnum, x, y, pnum, o);
  else
    slot = add_to_hand(roomnum, x, y, pnum, intohand, o);

  return slot;
}




/* ========================= S W A P P I N G ============================= */


/* swap the contents of a hand with an inventory slot */

void swap_hand_and_inv(num, which_hand, slot)
int num, which_hand, slot;
{
  OI *tmp;

  tmp = person[num]->inventory_object[slot];
  person[num]->inventory_object[slot] = person[num]->hand_object[which_hand];
  person[num]->hand_object[which_hand] = tmp;

  printf("swap_hand_and_inv: no map notify yet\n");
}



/* ========================= D R O P P I N G ============================= */

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
   the given person's current square, return TRUE if object was dropped */

int drop_object_from_inv(person_num, slot)
int person_num, slot;
{
  OI *o;
  int result;
  
  if (DEBUG)
    printf("Dropping inv item %d from person %d\n", slot, person_num);

  /* if item to drop is a blank (obj #0) then ignore drop request */
  o = INVOI(person_num, slot);
  if (!o) return(FALSE);

  /* when we can, we attempt to add the object to the immediate square */
  result = add_to_square(gameperson[person_num]->room,
		gameperson[person_num]->x, gameperson[person_num]->y, o);

  /* if object was successfully dropped then remove item from our inventory */
  if (result) person[person_num]->inventory_object[slot] = NULL;

  return(result);
}



/* locate a square near a location that could hold an object, return TRUE if
   we could successfully find one.  */

static int locate_empty_square(roomnum, loc_x, loc_y, ret_room, ret_x, ret_y)
int roomnum, loc_x, loc_y, *ret_room, *ret_x, *ret_y;
{
  int x, y, radius = 1, unsuccessful = 0, again = 0;
  int done = FALSE, tried_obvious = FALSE;

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
        else done = (spot_is_visible(roomnum, loc_x, loc_y, x, y) &&
		     (max_move_on_square(roomnum, x, y) > 0) &&
		     (!exit_char_on_square(roomnum, x, y)));

        if (!done) unsuccessful++;
      } while (!done && unsuccessful < 4);

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
  }

  *ret_room = roomnum;
  *ret_x = x;
  *ret_y = y;

  return(done);
}



/* Given an object instance, find a nearby spot on the map to drop it.
   Returns TRUE if the object dropped, FALSE if it wasn't. */

static int drop_object_nearby(roomnum, x, y, o)
int roomnum, x, y;
OI *o;
{
  int new_roomnum, new_x, new_y, result;

  /* if no item to drop, then ignore drop request */
  if (!o) return(FALSE);

  /* get a nearby empty square to drop something on */
  if (!locate_empty_square(roomnum, x, y, &new_roomnum, &new_x, &new_y))
    return(FALSE);

  result = add_to_square(new_roomnum, new_x, new_y, o);

  return(result);
}



/* drop everything a person has onto the nearest squares */

void drop_everything(pnum)
int pnum;
{
  int i;

  FORHANDS(i)
    if (drop_object_nearby(gameperson[pnum]->room, gameperson[pnum]->x,
			   gameperson[pnum]->y, HANDOI(pnum, i))) {
      person[pnum]->hand_object[i] = NULL;
    }

  for (i=0; i<INV_SIZE; i++)
    if (drop_object_nearby(gameperson[pnum]->room, gameperson[pnum]->x,
			   gameperson[pnum]->y, INVOI(pnum, i))) {
      person[pnum]->inventory_object[i] = NULL;
    }
}



 
/* Given an object type and a location, create the object at that square
   onto a nearby empty square, return TRUE if object was dropped. Used by
   missile.c code */

int drop_type_near_square(type, roomnum, x, y)
int type;
int roomnum, x, y;
{
  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (type == 0) return FALSE;

  printf("drop_type_near_square: not implemented\n");
  return(FALSE);
}



/* lose an object from your hand and reload one from inventory if such
   an object exists there.  Return the slot that was used, or -1 if there
   was nothing to reload with. */

int remove_with_reload(pnum, which_hand)
int pnum, which_hand;
{
  int slot;

  /* look for another copy */
  slot = find_object_in_inventory(pnum, HANDOITYPE(pnum, which_hand));;

  /* erase hand contents */
  person[pnum]->hand_object[which_hand] = NULL;

  /* swap empty hand with inventory copy */
  if (slot != -1) swap_hand_and_inv(pnum, which_hand, slot);

  return(slot);
}



/* drop an object from one of a person's hands and reload another object
   of the same type into the hand if possible.  Return the inventory
   slot number that was changed by the process. */

int drop_out_of_hand(pnum, which_hand)
int pnum, which_hand;
{
  int slot;

  /* look for another copy */
  slot = find_object_in_inventory(pnum, HANDOITYPE(pnum, which_hand));

  /* if there are no other copies of this kind of object, pick an empty slot */
  if (slot < 0) slot = open_inv_slot(pnum);

  /* if there were no empty slots, use slot #0 */
  if (slot < 0) slot = 0;

  /* swap the slot with hand content and drop what was in the hand */
  swap_hand_and_inv(pnum, which_hand, slot);
  drop_object_from_inv(pnum, slot);
  return(slot);
}



/* =========================== U T I L I T I E S ========================= */


/* copy object in someone's hand, unless that object fires a different kind
   of object, in which case, create a copy of the fired object */

OI *copy_fired_object(pnum, which_hand)
int pnum, which_hand;
{
  int type;
  OI *result;

  type = HANDOITYPE(pnum, which_hand);

  if (info[(uc)type]->movingobj == type) {
    result =  duplicate_object(&mapstats, HANDOI(pnum, which_hand));
    printf("copy_fired_object: warning: duplicate_object does not net well\n");
  }
  else 	/* create a copy */
    result = create_object(&mapstats, info[(uc)type]->movingobj, ANY_ID);

  return(result);
}



/* reduce the count on an object's uses left */

void reduce_count(obj, amount)
OI *obj;
int amount;
{
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    if (get_record(obj, REC_DETAIL) > 0) {
      set_record(obj, REC_DETAIL, (get_record(obj, REC_DETAIL) - amount));
      if (get_record(obj, REC_DETAIL) < 0) set_record(obj, REC_DETAIL, 0);
    }
}



/* increase the number of charges an item has */

void increase_count(obj, amount)
OI *obj;
int amount;
{
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded) {
    set_record(obj, REC_DETAIL, (get_record(obj, REC_DETAIL) + amount));
    if ((get_record(obj, REC_DETAIL) > info[(uc)obj->type]->capacity) &&
	info[(uc)obj->type]->capacity)
      set_record(obj, REC_DETAIL, info[(uc)obj->type]->capacity);
  }
}



/* figure out how many charges are left in item.  If this is not a numbered
   item then return zero */

int query_count(obj)
OI *obj;
{
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    return(get_record(obj, REC_DETAIL));
  else
    return(0);
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

int has_uses_left(obj)
OI *obj;
{
  int result = TRUE;

  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    result = (get_record(obj, REC_DETAIL) > 0);

  return(result);
}



/* look for object of given type in a person's inventory, return the slot
   number if you find, or return -1 if you can't find one */

int find_object_in_inventory(pnum, type)
int pnum, type;
{
  int i, found = -1;

  for (i=0; i<INV_SIZE && found == -1; i++)
    if (INVOI(pnum, i))
      if (INVOITYPE(pnum, i) == type) found = i;

  return(found);
}
