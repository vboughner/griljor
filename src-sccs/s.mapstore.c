h34186
s 00027/00005/00529
d D 1.7 92/09/03 16:13:12 vbo 7 6
c broke map writing up into two functions
e
s 00001/00001/00533
d D 1.6 92/09/03 15:49:40 vbo 6 5
c fixed a stupid for loop bug
e
s 00000/00000/00534
d D 1.5 92/08/07 21:00:35 vbo 5 4
c fixes for sun port merged in
e
s 00249/00012/00285
d D 1.4 92/03/11 20:53:12 vanb 4 3
c added to structures for containers, fixed more bugs
e
s 00031/00014/00266
d D 1.3 92/02/20 22:04:36 vanb 3 2
c 
e
s 00255/00006/00025
d D 1.2 92/02/15 01:19:41 vanb 2 1
c 
e
s 00031/00000/00000
d D 1.1 91/12/17 22:47:32 labc-4lc 1 0
c date and time created 91/12/17 22:47:32 by labc-4lc
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
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

D 2
/* Map storage and transfer routines.  These routines comprise the only
   method by which a map should be written or read from a file device. */
E 2
I 2
/* Map file storage, transfer, and retrieval */
E 2

I 2
#include <string.h>
E 2
#include <stdio.h>
D 2
#include <errno.h>
E 2
I 2
#include "config.h"
E 2
#include "def.h"
D 2
#include "objects.h"
#include "dialog.h"
#include "vline.h"
E 2
I 2
#include "lib.h"
I 4
#include "objects.h"
E 4
E 2
#include "map.h"
I 2
#include "mapstore.h"
#include "mapfunc.h"
D 4
#include "objects.h"
E 4
I 4
#define NOT_OBTOR
#include "dialog.h"
E 4



I 4
/* ================ M A P  H A S H  T A B L E ========================== */



E 4
static void install_object_hash_table(map)
/* allocates the array for storing pointers to linked lists (buckets) of the
   hash table, and returns a pointer to the newly allocated array.
   Size of the table is OBJECT_HASH_TABLE_SIZE. */
MapInfo *map;
{
  ObjectHashP *new = (ObjectHashP *) malloc(sizeof(ObjectHashP) *
					    OBJECT_HASH_TABLE_SIZE);
  demand(new, "no memory for object hash table");
I 4
  bzero(new, sizeof(ObjectHashP) * OBJECT_HASH_TABLE_SIZE);
E 4
  map->table_size = OBJECT_HASH_TABLE_SIZE;
  map->hash_table = new;
}



ObjectHash *find_hash_by_id(map, id)
/* looks through map for the hash table bucket element that matches the
   id, or if it doesn't exist, the one prior to the spot where it *should*
   go is returned.  If the bucket is empty, where it belongs, or it
   should be the first in the bucket, NULL is returned. */
MapInfo *map;
long id;
{
  ObjectHash *last = NULL, *ptr = map->hash_table[(id % map->table_size)];
  if (!ptr) return(NULL);
  for (; ptr; ptr = ptr->next) {
    if (ptr->obj->id == id) return(ptr);
    if (ptr->obj->id > id) return(last);
    last = ptr;
  }
  return(last);
}



void add_object_to_hash(map, obj)
/* uses object id number to add object to the map's hash table */
MapInfo *map;
OI *obj;
{
  ObjectHash *ptr, *new = (ObjectHash *) malloc(sizeof(ObjectHash));
  demand(new, "no memory for object hash");

  new->obj = obj;
  ptr = find_hash_by_id(map, obj->id);
  if (ptr) {
    new->next = ptr->next;
    ptr->next = new;
  }
  else {
    new->next = map->hash_table[(obj->id % map->table_size)];
    map->hash_table[(obj->id % map->table_size)] = new;
  }
}



void remove_object_from_hash(map, obj)
/* removes the entry for an object in the hash table, shouldn't be done
   for objects that aren't about to be immediately destroyed. */
MapInfo *map;
OI *obj;
{
  ObjectHash *last = NULL, *ptr = map->hash_table[(obj->id % map->table_size)];
  demand(ptr, "attempt to delete id that doesn't exist!");

  for (; ptr; ptr = ptr->next)
    if (ptr->obj->id == obj->id) {
      if (last) last->next = ptr->next;
      else map->hash_table[(obj->id % map->table_size)] = ptr->next;
      free(ptr);
      return;
    }
  Gerror("id to delete not found in bucket");
}



I 4
/* ================== C R E A T I O N =============================== */



E 4
SquareRec *create_square()
/* allocates memory for a square structure, fills it with default values,
   and returns a pointer to the newly created square. */
{
  SquareRec *new = (SquareRec *) malloc(sizeof(SquareRec));
  demand(new, "no memory for new square");
  bzero(new, sizeof(SquareRec));
  return(new);
}



D 3
MapInfo *lib_create_new_map(objfile)
char *objfile;
{
  MapInfo *new = (MapInfo *) malloc(sizeof(MapInfo));
  demand(new, "not enough memory for map info structure");
  bzero(new, sizeof(MapInfo));
  if (objfile) new->objfilename = create_string(objfile);
  install_object_hash_table(new);

  return(new);
}



E 3
D 4
void set_room_defaults(room)
E 4
I 4
static void set_room_defaults(room)
E 4
/* set defaults for a new room, absolutely bare minimum settings */
RoomInfo *room;
{
  int i;
D 4
  bzero(room, sizeof(RoomInfo));
E 4
I 3
  room->name = create_string("No Room Name");
  room->people_appear = TRUE;
  room->objects_appear = TRUE;
E 3

  /* set exits to go to nowhere */
  for (i=0; i<4; i++) room->exit[i] = -1;
}



D 4
void lib_add_new_room(map)
E 4
I 4
static void lib_add_new_room_no_defaults(map)
E 4
/* add a room to the map, this will mean allocating a larger array
   space and then copying all of the previous array into it, all the
D 4
   appropriate global variables should be updated */
E 4
I 4
   appropriate global variables should be updated. */
E 4
MapInfo *map;
{
  int i, j;

  map->room =
    (RoomInfo *) realloc(map->room, sizeof(RoomInfo) * (map->rooms + 1));
  demand(map->room, "not enough room to add a room to the map");
D 4
  set_room_defaults(&(map->room[map->rooms]));
E 4
I 4
  bzero(&(map->room[map->rooms]), sizeof(RoomInfo));
E 4

  for (i=0; i<ROOM_WIDTH; i++)
    for (j=0; j<ROOM_HEIGHT; j++)
      map->room[map->rooms].square[i][j] = create_square();

  map->rooms++;
I 3
}



I 4
void lib_add_new_room(map)
/* add a room to the map, this will mean allocating a larger array
   space and then copying all of the previous array into it, all the
   appropriate global variables should be updated */
MapInfo *map;
{
  lib_add_new_room_no_defaults(map);
  set_room_defaults(&(map->room[map->rooms - 1]));
}



static void lib_remove_last_room(map)
/* removes the last room in the map */
MapInfo *map;
{
  int j, k, roomnum;
  void free_object_instances();

  if (map->rooms) {
    roomnum = map->rooms - 1;
    FREE(map->room[roomnum].name);
    for (j=0; j<ROOM_WIDTH; j++)
      for (k=0; k<ROOM_HEIGHT; k++) {
	free_object_instances(map->room[roomnum].square[j][k]);
	FREE(map->room[roomnum].square[j][k]);
      }
    map->room =
      (RoomInfo *) realloc(map->room, sizeof(RoomInfo) * (map->rooms - 1));
    map->rooms--;
  }
}



E 4
MapInfo *lib_create_new_map(objfile)
/* creates a map with all the default values in the map property fields.
   Also creates the first room.  A map with no rooms, causes errors to occur
   in various places in editmap and possibly the other programs too, we've
   got to be able to assume at least a few valid locations are available in
D 4
   any given map. */
E 4
I 4
   any given map.  Major changes to this routine should also be reflected
   in lib_read_map_header_from_fd(), which does its own map creation. */
E 4
char *objfile;
{
  MapInfo *new = (MapInfo *) malloc(sizeof(MapInfo));
  demand(new, "not enough memory for map info structure");
  bzero(new, sizeof(MapInfo));
  new->name = create_string("No Map Name");
  new->team_name[0] = create_string("Neutral");
  new->team_name[1] = create_string("Team 1");
  new->team_name[2] = create_string("Team 2");
  new->team_name[3] = create_string("Team 3");
  new->team_name[4] = create_string("Team 4");
  if (objfile) new->objfilename = create_string(objfile);
  install_object_hash_table(new);
  new->neutrals_allowed = TRUE;
  new->teams_supported = 2;
I 4
  new->next_id = 1;
E 4
  lib_add_new_room(new);

  return(new);
E 3
}



D 4
MapInfo *lib_load_map(filename)
char *filename;
E 4
I 4
/* ======================== R E A D I N G ============================= */



static int lib_read_room_from_fd(map, fd)
/* reads from a device the character stream that represents a room
   description.  Returns TRUE if something was read, returns FALSE
   if the NULL that signifies the end of rooms is read. */
MapInfo *map;
int fd;
E 4
{
D 4
  MapInfo *result = NULL;
E 4
I 4
  RoomInfo *new;
  static DialogList *StaticRoomProperties(), *ObjectData = NULL;
  if (!ObjectData) ObjectData = StaticRoomProperties();
E 4

D 4
  return(result);
E 4
I 4
  lib_add_new_room_no_defaults(map);
  new = &(map->room[map->rooms - 1]);
  if (read_diag_from_fd(ObjectData, new, fd)) {
    return(TRUE);
  }
  else {
    lib_remove_last_room(map);
    return(FALSE);
  }
E 4
}



D 4
void lib_save_map(map, filename)
E 4
I 4
static void add_mapobj_to_map(map, obj)
/* take the descriptive structure for an object in a file, and adds that
   object to the map given.  If the location of the object is out of bounds,
   print a warning message. */
E 4
MapInfo *map;
I 4
MapObject *obj;
{
  OI *new;

  if ((obj->type < 0) || (obj->type >= objects)) {
    fprintf(stderr, "Warning: bad object type (%d) during map read\n",
	    obj->type);
    return;
  }

  if ((obj->loc.room < 0) || (obj->loc.room >= map->rooms)) {
    fprintf(stderr, "Warning: bad object room (%d) during map read\n",
	    obj->loc.room);
    return;
  }

  if (((obj->loc.x < 0) || (obj->loc.x >= ROOM_WIDTH)) ||
      ((obj->loc.y < 0) || (obj->loc.y >= ROOM_HEIGHT))) {
    fprintf(stderr, "Warning: bad object location (%d %d) during map read\n",
	    obj->loc.x, obj->loc.y);
    return;
  }

  /* create the object and add it to the map */
  new = create_object_from_mapobj(map, obj);
  add_object_to_square(map, obj->loc.room, obj->loc.x, obj->loc.y, new);
  if (obj->info) {
    /**** COMPLETE: add read of possible obj instance info here ****/
  }
  /**** COMPLETE: add read of possible obj variable space here ****/
}



int lib_read_mapobj_from_fd(map, fd)
/* reads from a device the character stream that represents an object
   description.  Returns TRUE if something was read, returns FALSE
   if the NULL that signifies the end of rooms is read. */
MapInfo *map;
int fd;
{
  MapObject *new = (MapObject *) malloc(sizeof(MapObject));
  static DialogList *StaticMapObjectProperties(), *ObjectData = NULL;
  if (!ObjectData) ObjectData = StaticMapObjectProperties();
  demand(new, "lib_read_mapobj_from_fd: out of memory");
  bzero(new, sizeof(MapObject));

  if (read_diag_from_fd(ObjectData, new, fd)) {
    add_mapobj_to_map(map, new);
    free(new);
    return(TRUE);
  }
  else {
    free(new);
    return(FALSE);
  }
}



MapInfo *lib_read_map_header_from_fd(fd)
/* reads in a map header from a file descriptor, works for either disk files
   or socket stream connections.  Creates all neccesary map structures
   in memory, and returns pointer to newly allocated MapInfo structure,
   or NULL if there is an error.  Caller must hereafter read in the
   object instances with something like:
       while (lib_read_mapobj_from_fd(&mapstats, fileno(fp)));
*/
int fd;
{
  int roomcount = 0;
  MapInfo *new;
  new = (MapInfo *) malloc(sizeof(MapInfo));
  demand(new, "lib_read_map_from_fd: out of memory");
  bzero(new, sizeof(MapInfo));
  install_object_hash_table(new);

  if (!read_diag_from_fd(StaticMapProperties(), new, fd)) {
    /* there was no mapinfo structure */
    free(new);
    return(NULL);
  }

  /* read rooms */
  while (lib_read_room_from_fd(new, fd)) roomcount++;

  /* if no rooms, exit */
  if (!roomcount) {
    free(new);
    return(NULL);
  }

  return(new);
}



/* ======================= W R I T I N G ============================== */



void lib_write_instance_to_fd(map, obj, fd)
/* converts an object to a MapObject description and writes it to a file
   descriptor.  Uses diag description to store the thing. */
MapInfo *map;
OI *obj;
int fd;
{
  static MapObject mo;

  bzero(&mo, sizeof(MapObject));
  mo.type = obj->type;
  mo.id = obj->id;
  mo.contained_id = obj->contained_id;
  mo.container_id = obj->container_id;
  mo.lsibling_id = obj->lsibling_id;
  mo.rsibling_id = obj->rsibling_id;
  if (obj->record) bcopy(obj->record, &(mo.rec), sizeof(RecordedObj));
  bcopy(obj->location, &(mo.loc), sizeof(MapLocation));

  write_diag_to_fd(StaticMapObjectProperties(), &(mo), fd);

  /**** COMPLETE: add write of possible obj instance info here ****/
  /**** COMPLETE: add write of possible obj variable space here ****/
}



D 7
void lib_write_map_to_fd(map, fd)
/* writes an entire map to a file, including object id numbers and variable
   space for objects.  This is, in effect, the image of a stored game. */
E 7
I 7
void lib_write_map_header_to_fd(map, fd)
/* writes the map header information (map info and room info) to file des. */
E 7
MapInfo *map;
int fd;
{
D 7
  int i, x, y;
  OI *ptr;
E 7
I 7
  int i;
E 7

  /* write MapInfo structure */
  write_diag_to_fd(StaticMapProperties(), map, fd);

  /* write rooms */
  for (i=0; i<map->rooms; i++)
    write_diag_to_fd(StaticRoomProperties(), &(map->room[i]), fd);
  write_end_of_section(fd);
I 7
}
E 7

I 7


void lib_write_map_objects_to_fd(map, fd)
/* write the object instances of the map (map header must have been written
   out already) */
MapInfo *map;
int fd;
{
  int i, x, y;
  OI *ptr;

E 7
  /* write objects */
  for (i=0; i<map->rooms; i++)
    for (x=0; x<ROOM_WIDTH; x++)
      for (y=0; y<ROOM_HEIGHT; y++) {
	ptr = first_obj_here(map, i, x, y);
	for (; ptr; ptr=ptr->next) lib_write_instance_to_fd(map, ptr, fd);
      }
  write_end_of_section(fd);
I 7
}



void lib_write_map_to_fd(map, fd)
/* writes an entire map to a file, including object id numbers and variable
   space for objects.  This is, in effect, the image of a stored game. */
MapInfo *map;
int fd;
{
  lib_write_map_header_to_fd(map, fd);
  lib_write_map_objects_to_fd(map, fd);
E 7
}



int lib_save_map(map, filename)
/* returns non-zero if file saved successfully */
MapInfo *map;
E 4
char *filename;
{
I 4
  FILE *fp;

  /* open the file for writing */
  fp = fopen(filename, "w");
  if (!fp) return(FALSE);

  lib_write_map_to_fd(map, fileno(fp));
  fclose(fp);
  return(TRUE);
E 4
}



/* ===================== F R E E   M E M O R Y ======================= */


void free_instance(obj)
/* free an object instantiation */
ObjectInstance *obj;
{
  if (obj) {
    if (obj->space) free(obj->space);
    if (obj->record) free(obj->record);
    if (obj->location) free(obj->location);
    free(obj);
  }
}



static void free_object_instances(square)
/* frees all the objects found on the given square */
SquareRec *square;
{
  if (square) {
    OI *ptr = square->first, *last = NULL;
    for (; ptr; ptr=ptr->next) {
      if (last) free_instance(last);
      last = ptr;
    }
    if (last) free_instance(last);
  }
}



static void free_object_hash_table(table)
/* frees all the linked lists that make up the buckets in the hash table,
   but not the objects that they refer to. */
ObjectHashP *table;
{
  if (table) {
    int i;
    ObjectHashP ptr, last;
    for (i=0; i<OBJECT_HASH_TABLE_SIZE; i++) {
      if (ptr = table[i]) {
	last = NULL;
	for (; ptr; ptr=ptr->next) {
	  if (last) free(last);
	  last = ptr;
	}
	if (last) free(last);
      }
    }
    free(table);
  }
}



void lib_free_map(map)
/* frees all the things that could have been mallocced for the benefit of
   a map info structure, the map itself, and all the object instances in the
   map. */
MapInfo *map;
{
  if (map) {
    int i, j, k;
    if (map->room) {
      for (i=0; i<map->rooms; i++) {
	/* free contents of rooms and squares (but not objects) */
	if (map->room[i].name) free(map->room[i].name);
	for (j=0; j<ROOM_WIDTH; j++)
D 6
	  for (k=0; j<ROOM_HEIGHT; k++) {
E 6
I 6
	  for (k=0; k<ROOM_HEIGHT; k++) {
E 6
	    free_object_instances(map->room[i].square[j][k]);
	    if (map->room[i].square[j][k]) free(map->room[i].square[j][k]);
	  }
      }
      free(map->room);
    }
    if (map->name) free(map->name);
    if (map->objfilename) free(map->objfilename);
    if (map->execute_file) free(map->execute_file);
    if (map->startup_file) free(map->startup_file);
    if (map->placement_file) free(map->placement_file);
    if (map->hash_table) free_object_hash_table(map->hash_table);
    for (i=0; i<(NUM_OF_TEAMS+1); i++)
      if (map->team_name[i]) free(map->team_name[i]);
    free(map);
  }
}
E 2
E 1
