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

/* Routines for handling inventory arrays */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"


/* easy access to person inventory arrays */
#define INV(num, i)	(person[num]->inventory[i].type)
#define INVREC(num, i)	(person[num]->inventory[i].record)
#define FORINV(i)	for (i=0; i<INV_SIZE; i++)

/* same easy access to hand contents */
#define HAND(num, i)	(person[num]->hand[i].type)
#define HANDREC(num, i)	(person[num]->hand[i].record)
#define FORHANDS(i)	for (i=0; i<2; i++)

/* macro procedures for getting info on object types */
#define TAKEABLE(x)	(info[(uc)x]->takeable)
#define WEIGHT(x)	(info[(uc)x]->weight)


/* =============
   This is what an item in an inventory looks like:

  typedef struct _invobj {
	char	type;	
	MemObj	*record;
  } InvObj;

  Array to hold inventory, and two vars to hold what's in hands, are defined
  like this inside the person definitions:

  InvObj	inventory[INV_SIZE];
  InvObj	hand[2];

  Procedures to handle inventory will be given person numbers
  ============== */


/* procedure declarations */
InvObj *copy_hand_object();
InvObj *copy_fired_object();
InvObj *create_object_copy();



/* ============================= T A K I N G ============================ */

/* figure out the weight of the objects being carried by a certain person */

int person_burden(num)
int num;
{
  int i, result = 0;

  /* add up weight of all things in inventory */
  FORINV(i)
    if (INV(num, i))
      result += WEIGHT(INV(num, i));

  /* add things that are in players hands as well */
  FORHANDS(i)
    if (HAND(num, i))
      result += WEIGHT(HAND(num, i));
      
  return result;
}



/* return the first free spot in inventory array or return -1 if there
   are no more */

int open_inv_slot(num)
int num;	/* person number */
{
  int i, found = -1;

  /* go through all, looking for an empty spot in array */
  FORINV(i)
    if (!INV(num, i)) {
      found = i;
      break;
    }

  return found;
}



/* add an object to the inventory list, returning the element number where
   it was added, or -1 if there was no space or persons weight requirement
   would be exceeded */

int add_to_inventory(num, o)
int num;
InvObj *o;
{
  int slot;
  
  /* find an empty space */
  slot = open_inv_slot(num);
  if (slot == -1) return -1;

  /* check weight requirement */
  if (person_burden(num) + WEIGHT(o->type) >
      gameperson[num]->weight_limit) return -1;

  /* copy the inventory object into that spot */
  bcopy(o, &(person[num]->inventory[slot]), sizeof(InvObj));

  /* return which slot was used */
  return slot;
}




/* place an object into one of the hands, wiping out what was previously
   in that hand */

add_to_hand(num, which_hand, o)
int num, which_hand;
InvObj *o;
{
  /* copy the object into the hand */
  bcopy(o, &(person[num]->hand[which_hand]), sizeof(InvObj));
}




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
}



/* take the object in a square in the given slot number, and return
   the inventory array element number that was filled or -1 if take failed.
   If space_slot passed is negative, instead of 0, 1, or 2, then the take
   will be of whatever on that square happens to be the most takeable. */

int take_object_into_inventory(roomnum, x, y, pnum, space_slot)
int roomnum, x, y, pnum, space_slot;
{
  MemObj *take_obj;
  InvObj inv_obj;
  int    otype, slot;	/* inventory slot number */
  
  if (space_slot < 0 || space_slot > ROOM_DEPTH) {
    /* find out what is here to take */
    take_obj = takeable_on_square(roomnum, x, y);

    /* if there is nothing there to take then return quietly */
    if (take_obj == NULL) return -1;

    /* now we add a copy of it to our inventory */
    inv_obj.type = take_obj->obj.objtype;
    if (info[(uc)inv_obj.type]->recorded) {
      inv_obj.record = allocate_mem_obj();
      bcopy(take_obj, inv_obj.record, sizeof(MemObj));
    }
    else inv_obj.record = NULL;
    slot = add_to_inventory(pnum, &inv_obj);
    otype = take_obj->obj.objtype;
  }
  else {
    /* find out what type of object we are taking */
    unsigned char *whats_here = whats_on_square(roomnum, x, y);
    otype = whats_here[space_slot];
    if (!info[otype]->takeable) return -1;

    /* add it to our inventory */
    inv_obj.type = otype;
    if (info[(uc)inv_obj.type]->recorded) {
      MemObj *ptr = what_recorded_obj_here(roomnum, x, y);
      if (!ptr) return -1;
      inv_obj.record = allocate_mem_obj();
      bcopy(ptr, inv_obj.record, sizeof(MemObj));
    }
    else inv_obj.record = NULL;

    slot = add_to_inventory(pnum, &inv_obj);
  }

  /* if we could add it to inventory then remove it from square */
  if (slot != -1) remove_from_square(roomnum, x, y , otype);

  /* return which slot in inventory was updated, if any */
  return slot;
}



/* take an object in the square into one of the hands, putting the previous
   contents of the hands into the inventory, return element of inventory  
   array that was changed or -1 if take failed */

int take_object_into_hand(roomnum, x, y, pnum, space_slot, which_hand)
int roomnum, x, y, pnum, space_slot, which_hand;
{
  int slot;
  
  /* take the object into inventory first */
  slot = take_object_into_inventory(roomnum, x, y, pnum, space_slot);

  /* now swap contents of hand with the inventory slot */
  if (slot != -1) swap_hand_and_inv(pnum, which_hand, slot);

  return slot;
}



/* This routine decides whether an object should be taken into the
   hand or into the inventory and it picks it up off the square.
   It requires the location room, x, y, the person picking it
   up, and the slot number on the square that is to be picked up.
   Returns element of inventory array that was changed, or -1 if the
   take failed.  Pass negative slot number instead of 0, 1, or 2, if
   you want the object from square to pick up to be selected by the
   algorithms in this file.  'intohand' must be either -1, 0, or 1.
   -1 signifies the object is going straight into inventory, unless the
   object includes a force into hand flag. */

int take_object_from_ground(roomnum, x, y, pnum, space_slot, intohand)
int roomnum, x, y, pnum, space_slot, intohand;
{
  int *whats_here, otype;
  int slot;

  if (OUT_OF_BOUNDS(roomnum, x, y)) return -1;
  if (intohand < -1 || intohand > 1) return -1;

  /* the ideas from get_object_from_ground need to be incorporated
     right here ======================= */
  /* check to see if there is a specific slot mentioned, if so, and
     space_slot is specified (not -1) then check the property of the
     object to be picked up for intohandedness */

  if (intohand == -1)
    slot = take_object_into_inventory(roomnum, x, y, pnum, space_slot);
  else
    slot = take_object_into_hand(roomnum, x, y, pnum, space_slot, intohand);

  return slot;
}



/* ========================= S W A P P I N G ============================= */


/* swap the contents of a hand with an inventory slot */

swap_hand_and_inv(num, which_hand, slot)
int num, which_hand, slot;
{
  InvObj temp;

  /* load old contents of hand into temp */
  if (DEBUG) printf("Swap hand %d and inventory %d\n", which_hand, slot);
  bcopy(&(person[num]->hand[which_hand]), &temp, sizeof(InvObj));

  /* copy contents of inventory slot into hand */
  add_to_hand(num, which_hand, &(person[num]->inventory[slot]));

  /* copy temp into inventory slot */
  bcopy(&temp, &(person[num]->inventory[slot]), sizeof(InvObj));
}



/* ========================= D R O P P I N G ============================= */

/* Given the inventory square number, attempt to drop the object there onto
   the given person's current square, return TRUE if object was dropped */

int drop_object_from_inv(person_num, inv_num)
int person_num, inv_num;
{
  MemObj *takeable;
  int    result;
  
  if (DEBUG)
    printf("Dropping inv item %d from person %d\n", inv_num, person_num);

  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (INV(person_num, inv_num) == 0) return FALSE;

  /* make sure that square is empty of takeable objects */
  takeable = takeable_on_square(gameperson[person_num]->room,
	     gameperson[person_num]->x,	gameperson[person_num]->y);

  /* when we can, we attempt to add the object to the immediate square */
  if (!takeable)
    result = add_to_square(gameperson[person_num]->room,
		gameperson[person_num]->x, gameperson[person_num]->y,
		&(person[person_num]->inventory[inv_num]));

  if (takeable || (!takeable && !result)) {
    /* if there is something already there then we try to drop nearby */
    result = drop_object_nearby(person_num, 
				&(person[person_num]->inventory[inv_num]));
  }

  /* if object was successfully dropped then remove item from our inventory */
  if (result) remove_from_inv(person_num, inv_num, FALSE);

  return result;
}




/* add an inventory object to a map square, return TRUE if successful */

int add_to_square(roomnum, x, y, inv_obj)
int roomnum, x, y;
InvObj *inv_obj;
{
  int recorded, empty_slot;

  /* see if the object is a recorded one */
  recorded = info[(uc)inv_obj->type]->recorded;

  /* look for an empty slot on the square (recorded if neccessary) */
  empty_slot = find_empty_slot(roomnum, x, y, recorded);

  /* if no empty slot then abort this placement attempt */
  if (empty_slot == -1) return FALSE;

  /* change x and y in recorded object record */
  if (recorded) {
    inv_obj->record->obj.x = x;
    inv_obj->record->obj.y = y;
  }

  /* place object into empty slot */
  notify_all_of_map_change(roomnum, x, y, empty_slot, 0,
			   inv_obj->type, inv_obj->record);

  return TRUE;
}


/* add an inventory object to a map square, return TRUE if successful */

int add_type_to_square(roomnum, x, y, type)
int roomnum, x, y;
int type;
{
  int empty_slot;

  /* look for an empty slot on the square (recorded if neccessary) */
  empty_slot = find_empty_slot(roomnum, x, y, FALSE);

  /* if no empty slot then abort this placement attempt */
  if (empty_slot == -1) return FALSE;

  /* place object into empty slot */
  notify_all_of_map_change(roomnum, x, y, empty_slot, 0,
			   type, FALSE);

  return TRUE;
}


/* remove an object from person's inventory.  If you are given TRUE in the
   to_free variable then free the inventory object pointed to after
   removing it from inventory */

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



/* drop everything a person has onto the nearest squares */

drop_everything(pnum)
int pnum;
{
  register int i;

  FORHANDS(i)
    if (drop_object_nearby(pnum, &(person[pnum]->hand[i]))) {
      if (info[(uc)HAND(pnum, i)]->recorded) free(HANDREC(pnum, i));
      HAND(pnum, i) = 0;
      HANDREC(pnum, i) = NULL;
    }

  for (i=0; i<INV_SIZE; i++)
    if (drop_object_nearby(pnum, &(person[pnum]->inventory[i])))
      remove_from_inv(pnum, i, FALSE);
}




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
    tries++;
  } while (!result && tries < 10);
  
  return result;
}



/* Given an object type and a location, create the object at that square
   onto a nearby empty square, return TRUE if object was dropped */

int drop_type_near_square(type, roomnum, x, y)
int type;
int roomnum, x, y;
{
  int    drop_room, drop_x, drop_y, result, tries = 0;
  
  /* if item to drop is a blank (obj #0) then ignore drop request */
  if (type == 0) return FALSE;

  do {
    /* get a nearby empty square to drop something on */
    if (locate_empty_square(roomnum, x, y,
			    &drop_room, &drop_x, &drop_y, FALSE) == FALSE)
	return FALSE;

    /* now we attempt to add the object to the square */
    result = add_type_to_square(drop_room, drop_x, drop_y, type);
    tries++;
  } while (!result && tries < 10);
  
  return result;
}



/* lose an object from your hand and reload one from inventory if such
   an object exists there.  Return the slot that was used, or -1 if there
   was nothing to reload with. */

remove_with_reload(pnum, which_hand)
int pnum, which_hand;
{
  int slot;

  /* look for another copy */
  slot = find_object_in_inventory(pnum, person[pnum]->hand[which_hand].type);

  /* erase hand contents */
  HAND(pnum, which_hand) = 0;
  if (HANDREC(pnum, which_hand)) free(HANDREC(pnum, which_hand));
  HANDREC(pnum, which_hand) = NULL;

  /* swap empty hand with inventory copy */
  if (slot != -1) swap_hand_and_inv(pnum, which_hand, slot);

  return slot;
}



/* drop an object from one of a person's hands and reload another object
   of the same type into the hand if possible.  Return the inventory
   slot number that was changed by the process. */

int drop_out_of_hand(pnum, which_hand)
int pnum, which_hand;
{
  int slot;

  /* look for another copy */
  slot = find_object_in_inventory(pnum, person[pnum]->hand[which_hand].type);

  /* if there are no other copies of this kind of object, pick an empty slot */
  if (slot < 0) slot = open_inv_slot(pnum);

  /* if there were no empty slots, use slot #0 */
  if (slot < 0) slot = 0;

  /* swap the slot with hand content and drop what was in the hand */
  swap_hand_and_inv(pnum, which_hand, slot);
  drop_object_from_inv(pnum, slot);
  return slot;
}


/* ===================== E M P T Y  square checks ======================= */

/* locate a square near a location that could hold an object, return TRUE if
   we could successfully find one.  The recorded flag tells us whether we
   are looking for a space free of other recorded objects or not */

int locate_empty_square(roomnum, loc_x, loc_y,
			ret_room, ret_x, ret_y, recorded)
int roomnum, loc_x, loc_y, *ret_room, *ret_x, *ret_y, recorded;
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



/* copy object in someone's hand, unless that object fires a different kind
   of object, in which case, create a copy of the fired object */

InvObj *copy_fired_object(pnum, which_hand)
int pnum, which_hand;
{
  int type;
  InvObj *result;

  type = HAND(pnum, which_hand);

  if (info[(uc)type]->movingobj == type)
    result =  copy_hand_object(pnum, which_hand);
  else 	/* create a copy */
    result =  create_object_copy(info[(uc)type]->movingobj);

  return result;
}



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



/* reduce the count on an object's uses left */

reduce_count(obj, amount)
InvObj *obj;
int amount;
{
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record)
    if (obj->record->obj.detail > 0) {
      obj->record->obj.detail -= amount;
      if (obj->record->obj.detail < 0) obj->record->obj.detail = 0;
    }
}



/* increase the number of charges an item has */

increase_count(obj, amount)
InvObj *obj;
int amount;
{
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record) {
    obj->record->obj.detail += amount;
    if (obj->record->obj.detail > info[(uc)obj->type]->capacity &&
	info[(uc)obj->type]->capacity)
		obj->record->obj.detail = info[(uc)obj->type]->capacity;
  }
}



/* figure out how many charges are left in item.  If this is not a numbered
   item then return zero */

int query_count(obj)
InvObj *obj;
{
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record)
    return (obj->record->obj.detail);
  else
    return 0;
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

has_uses_left(obj)
InvObj *obj;
{
  int result = TRUE;

  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded && obj->record)
    result = (obj->record->obj.detail > 0);

  return result;
}



/* look for object of given type in a person's inventory, return the slot
   number if you find, or return -1 if you can't find one */

int find_object_in_inventory(pnum, type)
int pnum, type;
{
  register int i, found = -1;

  for (i=0; i<INV_SIZE && found == -1; i++)
    if (person[pnum]->inventory[i].type == type) found = i;

  return found;
}
