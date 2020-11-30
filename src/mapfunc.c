/***************************************************************************
 * @(#) mapfunc.c 1.5 - last change made 08/07/92
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

/* useful map utility functions */

#include <string.h>
#include <stdio.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "mapstore.h"
#include "mapfunc.h"



/* ================ O B J E C T  I N S T A N C E S ====================== */


static RecordedObj *create_object_record()
{
  RecordedObj *new = (RecordedObj *) malloc(sizeof(RecordedObj));
  demand(new, "out of memory");
  bzero(new, sizeof(RecordedObj));
  return(new);
}



void set_record(obj, which, value)
OI *obj;
int which, value;
{
  if (!obj->record) obj->record = create_object_record();
  obj->record->dirty = TRUE;

  switch (which) {
  case REC_DETAIL:
    obj->record->detail = value;
    break;
  case REC_INFOX:
    obj->record->infox = value;
    break;
  case REC_INFOY:
    obj->record->infoy = value;
    break;
  case REC_ZINGER:
    obj->record->zinger = value;
    break;
  case REC_EXTRA1:
    obj->record->extra[0] = value;
    break;
  case REC_EXTRA2:
    obj->record->extra[1] = value;
    break;
  case REC_EXTRA3:
    obj->record->extra[2] = value;
    break;
  default:
    Gerror("bad set record slot selected");
    break;
  }
}



void set_record_default(obj, which)
/* figures out what to set a recorded slot to, whether a set value, or
   zero if there isn't one set. */
OI *obj;
int which;
{
  int value;
  if (!obj->record) obj->record = create_object_record();
  obj->record->dirty = TRUE;

  if (info[obj->type]->set[which])
    value = info[obj->type]->def[which];
  else
    value = 0;

  switch (which) {
  case REC_DETAIL:
    obj->record->detail = value;
    break;
  case REC_INFOX:
    obj->record->infox = value;
    break;
  case REC_INFOY:
    obj->record->infoy = value;
    break;
  case REC_ZINGER:
    obj->record->zinger = value;
    break;
  case REC_EXTRA1:
    obj->record->extra[0] = value;
    break;
  case REC_EXTRA2:
    obj->record->extra[1] = value;
    break;
  case REC_EXTRA3:
    obj->record->extra[2] = value;
    break;
  default:
    Gerror("bad set record slot selected");
    break;
  }
}



int get_record(obj, which)
OI *obj;
int which;
{
  if (!obj->record) return(0);

  switch (which) {
  case REC_DETAIL:
    return(obj->record->detail);
    break;
  case REC_INFOX:
    return(obj->record->infox);
    break;
  case REC_INFOY:
    return(obj->record->infoy);
    break;
  case REC_ZINGER:
    return(obj->record->zinger);
    break;
  case REC_EXTRA1:
    return(obj->record->extra[0]);
    break;
  case REC_EXTRA2:
    return(obj->record->extra[1]);
    break;
  case REC_EXTRA3:
    return(obj->record->extra[2]);
    break;
  default:
    Gerror("bad get record slot selected");
    break;
  }
}



static ObjLocation *create_blank_location()
{
  ObjLocation *new = (ObjLocation *) malloc(sizeof(ObjLocation));
  demand(new, "no memory for blank object location");
  new->type = OL_NOWHERE;
  return(new);
}



OI *create_object(map, type, id)
/* creates a new object with a new id number for use in the map given,
   at the same time, makes the object's entry in the id hash table. 
   Be sure to assign an initial value in all object fields here.
   If id param is not ANY_ID, then use the id provided for the object,
   giving a warning if it is a duplicate id number. */
MapInfo *map;
int type;
long id;
{
  OI *new;
  demand((type >= 0 && type < objects), "bad type number to create_object");

  new = (OI *) malloc(sizeof(OI));
  demand(new, "no memory for object instance");
  bzero(new, sizeof(ObjectInstance));

  new->type = type;
  if (id == ANY_ID) {
    new->id = map->next_id;
    map->next_id++;
  }
  else {
    new->id = id;
    map->next_id = id + 1;
  }

  if (am_driver && info[type]->varspace) {
    new->space = (long *) malloc(sizeof(long) * info[type]->varspace);
    demand(new->space, "no memory for variable space");
    bzero(new->space, sizeof(long) * info[type]->varspace);
  }
  else new->space = NULL;

  if (info[type]->recorded) {
    int i;
    new->record = create_object_record();
    for (i=0; i<REC_NUM_FIELDS; i++) {
      if (info[type]->set[i])
	set_record(new, i, info[type]->def[i]);
    }
  }
  else new->record = NULL;

  new->location = create_blank_location();
  new->info = NULL;
  new->prev = NULL;
  new->next = NULL;
  add_object_to_hash(map, new);
  return(new);
}



OI *create_object_from_mapobj(map, mapobj)
/* creates an object with the qualities provided by mapobj structure.
   Copies over only the recorded object info, if neccessary.  The location
   and possible instance info structure are not copied.  The id number is
   preserved, however. */
MapInfo *map;
MapObject *mapobj;
{
  OI *new = create_object(map, mapobj->type, mapobj->id);
  new->contained_id = mapobj->contained_id;
  new->container_id = mapobj->container_id;
  new->lsibling_id = mapobj->lsibling_id;
  new->rsibling_id = mapobj->rsibling_id;
  if (new->record) bcopy(&(mapobj->rec), new->record, sizeof(RecordedObj));
  return(new);
}



void change_object_type(obj, newtype)
/* change an object's type, preserving recorded info if neccessary/possible.
   The same id number and location are also preserved. */
OI *obj;
int newtype;
{
  if (!obj) return;

  if (am_driver) {
    obj->space = (long *) realloc(obj->space,
				  sizeof(long) * info[newtype]->varspace);
  }

  if (info[newtype]->recorded && !obj->record) {
    int i;
    obj->record = create_object_record();
    for (i=0; i<REC_NUM_FIELDS; i++) {
      if (info[newtype]->set[i])
	set_record(obj, i, info[newtype]->def[i]);
    }
  }
  else if (!info[newtype]->recorded && obj->record) free(obj->record);

  obj->type = newtype;
}



OI *duplicate_object(map, obj)
/* create a new object just like the given one, the one given, need not
   be from the same map.  Map given as argument should be the one the new
   duplicate will be inserted into.  Location is NOT duplicated. */
MapInfo *map;
OI *obj;
{
  if (obj) {
    OI *new = create_object(map, obj->type, ANY_ID);
    if (new->space && obj->space)
      bcopy(obj->space, new->space, sizeof(long) * info[obj->type]->varspace);
    if (new->record && obj->record)
      bcopy(obj->record, new->record, sizeof(RecordedObj));
    return(new);
  }
  else return(NULL);
}



/* ============= S Q U A R E  M A N I P U L A T I O N ================= */



int is_valid_square(map, room, x, y)
/* returns non-zero only if the location given is valid in the map */
MapInfo *map;
int room, x, y;
{
  if (!map) return(FALSE);
  if (room < 0 || room >= map->rooms) return(FALSE);
  if (x < 0 || x >= ROOM_WIDTH) return(FALSE);
  if (y < 0 || y >= ROOM_HEIGHT) return(FALSE);
  return(TRUE);
}



SquareRec *get_square(map, room, x, y)
/* returns a pointer to a particular square's information.  NULL will be
   returned whenever there is no info on the square because of
   an out-of-bounds location. */
MapInfo *map;
int room, x, y;
{
  if (!is_valid_square(map, room, x, y)) return(NULL);
  return(map->room[room].square[x][y]);
}



OI *square_first_masked(square, masked)
/* looks on the square for the first object with the same kind of mask flag
   as the one given as an arguement here.  Returns NULL if there isn't one. */
SquareRec *square;
int masked;
{
  OI *ptr = square->first;
  for (; ptr; ptr=ptr->next)
    if ((info[ptr->type]->masked && masked) ||
	(!info[ptr->type]->masked && !masked)) return(ptr);

  return(NULL);
}



OI *square_last_masked(square, masked)
/* returns a pointer to the last object on the square, which has the
   same mask type as the one given.  Returns NULL if there is no object
   on the square with this masking type.  Assumes that objects are ordered
   on the square, with unmasked objects first. */
SquareRec *square;
int masked;
{
  OI *ptr;

  if (masked) {
    ptr = square->last;
    for (; ptr; ptr=ptr->prev)
      if (info[ptr->type]->masked) return(ptr);
  }
  else {
    ptr = square->first;
    for (; ptr; ptr=ptr->next)
      if (!info[ptr->type]->masked &&
	  (!ptr->next || info[ptr->next->type]->masked)) return(ptr);
  }

  return(NULL);
}



static void readjust_square_flags(square)
/* used after deletion or removal of an object to be sure flags are correct */
SquareRec *square;
{
  OI *ptr;
  square->view_blocked = FALSE;

  for (ptr=square->first; (ptr && !square->view_blocked); ptr=ptr->next) {
    if (!info[ptr->type]->transparent) square->view_blocked = TRUE;
  }
}



int view_obstructed_by_square(square)
/* returns TRUE if there is an object on the square that obstructs viewing */
SquareRec *square;
{
  if (square) return(square->view_blocked);
  else return(FALSE);
}



int movement_on_square(square)
/* returns the lowest movement number of any objects on the square */
SquareRec *square;
{
  OI *ptr;
  int minimum = MAX_MOVEMENT_NUMBER;
  if (!square) return(0);

  for (ptr=square->first; ptr; ptr=ptr->next) {
    /* look at each item, look for the lowest possible speed number */
    if (info[ptr->type]->movement < minimum)
      minimum = info[ptr->type]->movement;
  }

  return minimum;
}



int firing_onto_square(square)
/* find out whether we can fire onto a given square, if so return TRUE.
   We may not fire onto squares that are non-permeable and don't allow
   movement on them. */
SquareRec *square;
{
  OI *ptr;
  int move = TRUE, perm = TRUE;

  /* if out of bounds then sure, we can fire there */
  if (!square) return(TRUE);

  for (ptr=square->first; (ptr && (perm || move)); ptr=ptr->next) {
    if (!info[ptr->type]->permeable) perm = FALSE;
    if (!info[ptr->type]->movement) move = FALSE;
  }

  return (perm || move);
}



int firing_through_square(square)
/* find out whether we can fire over the given square, if so return TRUE.
   We may not fire through squares that are non-permeable. */
SquareRec *square;
{
  OI *ptr;
  int okay = TRUE;

  /* if out of bounds then sure, we can fire there */
  if (!square) return(TRUE);

  for (ptr=square->first; (ptr && okay); ptr=ptr->next) {
    if (!info[ptr->type]->permeable) okay = FALSE;
  }

  return okay;
}



void copy_square_add(src, map, roomnum, x, y)
/* copies the contents of src square into destination.  Previous contents
   of dest square remain there as well.  src need not be in same map
   as the destination. */
SquareRec *src;
MapInfo *map;
int roomnum, x, y;
{
  OI *ptr;

  if (src && is_valid_square(map, roomnum, x, y)) {
    for (ptr=src->first; ptr; ptr=ptr->next)
      add_duplicate_to_square(map, roomnum, x, y, ptr);
  }
}



void copy_square_overwrite(src, map, roomnum, x, y)
/* copies the contents of the src square onto the destination square,
   obliterating whatever was there, and replacing with fresh new objects
   with they're own unique id numbers.  src need not be from same
   map as the destination. */
SquareRec *src;
MapInfo *map;
int roomnum, x, y;
{
  OI *ptr = first_obj_here(map, roomnum, x, y);
  if (ptr) destroy_objects_in_list(map, ptr);
  copy_square_add(src, map, roomnum, x, y);
}



int number_of_objects_on_square(square)
SquareRec *square;
{
  if (square)
    return(square->number);
  else
    return(0);
}



OI *object_i_on_square(square, i)
/* goes through objects on square, looking for ith.  Count starts at zero. */
SquareRec *square;
int i;
{
  if (square) {
    int j;
    OI *ptr;
    for (j=0,ptr=square->first; (ptr && j<i); ptr=ptr->next,j++);
    return(ptr);
  }
  else return(NULL);
}



/* ================== M A P  F U N C T I O N S ========================= */



OI *first_obj_here(map, room, x, y)
MapInfo *map;
int room, x, y;
{
  SquareRec *square = get_square(map, room, x, y);
  if (!square) return(NULL);
  else return(square->first);
} 



OI *last_obj_here(map, room, x, y)
MapInfo *map;
int room, x, y;
{
  SquareRec *square = get_square(map, room, x, y);
  if (!square) return(NULL);
  else return(square->last);
} 



OI *what_exit_obj_here(map, room, x, y)
/* if there is an exit object on this square, a pointer to it will be
   returned, otherwise NULL is returned */
MapInfo *map;
int room, x, y;
{
  OI *ptr = last_obj_here(map, room, x, y);

  for (; ptr; ptr = ptr->prev)
    if (info[ptr->type]->exit) return(ptr);

  return(NULL);
}



void object_in_map(map, obj, room, x, y)
/* sets the location information for an object in the map */
MapInfo *map;
OI *obj;
int room, x, y;
{
  obj->location->type = OL_IN_MAP;
  obj->location->maploc.room = room;
  obj->location->maploc.x = x;
  obj->location->maploc.y = y;
}



void add_object_to_square(map, room, x, y, obj)
/* adds an object to a square in the room.  Unmasked objects always go
   before objects with masks.  In any case, the added object goes at the
   end of the group of similarly masked objects. */
MapInfo *map;
int room, x, y;
OI *obj;
{
  OI *ptr;
  SquareRec *square;
  square = get_square(map, room, x, y);
  if (!map || !square || !obj) return;

  if (info[obj->type]->masked) {
    /* add object as last one on the square */
    if (square->last) {
      square->last->next = obj;
      obj->prev = square->last;
    }
    else {
      square->first = obj;
      obj->prev = NULL;
    }
    obj->next = NULL;
    square->last = obj;
  }
  else {
    /* add object as last of the initial unmasked group */
    ptr = square_last_masked(square, FALSE);
    if (ptr) {
      /* there's another unmasked object here to put this one after */
      obj->prev = ptr;
      obj->next = ptr->next;
      if (obj->next) obj->next->prev = obj;
      else square->last = obj;
      ptr->next = obj;
    }
    else {
      /* this will be the one and only unmasked object on square */
      obj->prev = NULL;
      obj->next = square->first;
      if (obj->next) obj->next->prev = obj;
      else square->last = obj;
      square->first = obj;
    }
  }

  square->number++;
  if (!info[obj->type]->transparent) square->view_blocked = TRUE;
  object_in_map(map, obj, room, x, y);
}



void add_duplicate_to_square(map, room, x, y, obj)
/* duplicate the given object and add the new copy into the map */
MapInfo *map;
int room, x, y;
OI *obj;
{
  OI *new = duplicate_object(map, obj);
  add_object_to_square(map, room, x, y, new);
}



int is_object_type_here(map, roomnum, x, y, type)
/* looks through objects on square, trying to determine whether any of the
   objects here are of the given type.  Returns TRUE if one is found. */
MapInfo *map;
int roomnum, x, y, type;
{
  OI *ptr;
  int result = FALSE;

  ptr = first_obj_here(map, roomnum, x, y);
  for (; (ptr && !result); ptr=ptr->next)
    if ((ptr->type && type) || (!ptr->type && !type)) result = TRUE;

  return(result);
}



void add_object_replacing_same_mask(map, roomnum, x, y, type)
/* looks for the first object on the square with the same mask type as
   the new type given, and replaces the object with the type given.
   If there is no such object, than an object of the given new type
   is added to the square. */
MapInfo *map;
int roomnum, x, y, type;
{
  OI *ptr;
  SquareRec *square = get_square(map, roomnum, x, y);
  if (!square) return;
  
  ptr = square_first_masked(square, info[type]->masked);
  if (ptr) {
    if (ptr->type != type) change_object_type(ptr, type);
  }
  else {
    ptr = create_object(map, type, ANY_ID);
    add_object_to_square(map, roomnum, x, y, ptr);
  }
}



/* ============= R E M O V A L / D E S T R U C T I O N ================= */



static void remove_object_from_square_list(square, id)
/* should only be done during deletion of objects, doesn't free object. */
SquareRec *square;
long id;
{
  OI *last = NULL, *ptr = square->first;

  for (; ptr; ptr=ptr->next)
    if (ptr->id == id) {
      if (ptr->prev) ptr->prev->next = ptr->next;
      else square->first = ptr->next;
      if (ptr->next) ptr->next->prev = ptr->prev;
      else square->last = ptr->prev;
      readjust_square_flags(square);
      return;
    }
  Gerror("object not found for removal from square");
}



void remove_object_from_map(map, obj)
/* if object's location is in the map, then remove all ties, so that the
   object is in nowhereland. */
MapInfo *map;
OI *obj;
{
  if (obj->location->type == OL_IN_MAP) {
    SquareRec *square = get_square(map, obj->location->maploc.room,
				   obj->location->maploc.x,
				   obj->location->maploc.y);
    if (square) remove_object_from_square_list(square, obj->id);
  }
}



void destroy_object(map, obj)
/* destroys an object by removing it from whatever location it's in
   and freeing it and it's id table entry. */
MapInfo *map;
OI *obj;
{
  remove_object_from_hash(map, obj);
  /* COMPLETE: make provision for removing objects from people */
  remove_object_from_map(map, obj);
  free_instance(obj);
}



void destroy_objects_on_square_masked(map, roomnum, x, y, masked)
/* destroys all objects on a square whose type has a masking flag equal
   to the argument given (TRUE or FALSE) */
MapInfo *map;
int roomnum, x, y, masked;
{
  OI *ptr;
  SquareRec *square = get_square(map, roomnum, x, y);
  if (!square) return;

  do {
    ptr = square_first_masked(square, masked);
    if (ptr) destroy_object(map, ptr);
  } while (ptr);
}



void destroy_objects_in_list(map, list)
/* destroys every object found in linked list.  Deletion of the objects
   from map is taken care of. */
MapInfo *map;
OI *list;
{
  OI *last = NULL, *ptr = list;

  for (; ptr; ptr=ptr->next) {
    if (last) destroy_object(map, last);
    last = ptr;
  }
  if (last) destroy_object(map, last);
}



void destroy_all_objects_on_square(map, roomnum, x, y)
/* destroys every object found on the square */
MapInfo *map;
int roomnum, x, y;
{
  OI *ptr = first_obj_here(map, roomnum, x, y);
  if (ptr) destroy_objects_in_list(map, ptr);  
}



void destroy_all_objects_in_room(map, roomnum)
/* goes through entire room, cleaning every square */
MapInfo *map;
int roomnum;
{
  int i, j;

  for (i=0; i<ROOM_WIDTH; i++)
    for (j=0; j<ROOM_HEIGHT; j++)
      destroy_all_objects_on_square(map, roomnum, i, j);
}
