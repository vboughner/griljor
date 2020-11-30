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

/* Map record manipulation routines */

#define MAP_MAIN

#include <string.h>
#include <stdio.h>
#include "config.h"
#include "def.h"
#include "map.h"
#include "objects.h"


/* Global variables and those concerning the current map */

MapInfo		mapstats;	/* info about loaded map */
RoomInfo	*room;		/* pointer to array of rooms in map */
MemObj		*roomrecobj;	/* pointer to array of room obj lists */
short		current_room;	/* number of room we are in now */
char		mapfile[120], objectfile[120];
int		xdir[4] = { 0, 1, 0, -1 };
int		ydir[4] = { -1, 0, 1, 0 };

/* Global variables which concern the stored copy of the original map
   (these variables are used only by players and the driver) */

RoomInfo	*origroom;	/* pointer to array of rooms in map */
MemObj		*origroomrecobj;/* pointer to array of room obj lists */



/* load the map from a file - the driver and map editor make use of this */

load_map()
{
  int i, j;
  FILE *fp;
  RecordedObj o;
  char *libbed_filename();
  
  fp = fopen(mapfile, "r");
  if (fp == NULL) {
    fp = fopen(libbed_filename(MAP_LIB_DIR, mapfile), "r");
    if (!fp) {
      printf("Griljor: was searchingg for map file %s\n", mapfile);
      Gerror("unable to read map file");
    }
  }

  fread((char *)&mapstats, sizeof(mapstats), 1, fp);

  if (DEBUG) printf("Loading %s... %d rooms\n", mapstats.name, mapstats.rooms);

  /* allocate room array and fill it from the file */
  room = make_room_array(mapstats.rooms);

  /* allocate the array of lists of recorded objs in the rooms and fill it */
  roomrecobj = make_room_obj_list_array(mapstats.rooms);
  initialize_room_obj_list_array(roomrecobj, mapstats.rooms);

  /* read rooms */
  for (i=0; i<mapstats.rooms; i++) {
    fread((char *)&(room[i]), sizeof(RoomInfo), 1, fp);
    /* read recorded objects in this room */
    for (j=0; j<room[i].numobjs; j++) {
      fread((char *)&o, sizeof(RecordedObj), 1, fp);
      add_file_record_to_mem(i, &o);
    }
  }
  fclose(fp);

  /* load the object file also, allowing for the possibility of an overriding
     object file selection if the 'objectfile' variable is anything other
     than UNKNOWN */
  if (strcmp(objectfile, UNKNOWN)) strcpy(mapstats.objfilename, objectfile);
  load_obj_file(mapstats.objfilename);
}



create_new_map(objfile)
char *objfile;
{
  register int i;
  
  /* initialize map information record */
  bzero(&mapstats, sizeof(MapInfo));
  strcpy(mapstats.name, "no name");
  for (i=0; i<NUM_OF_TEAMS; i++)
    sprintf(mapstats.team_name[i], "Team %d", i+1);
  mapstats.teams_supported = 2;
  mapstats.rooms = 1;
  strcpy(mapstats.execute_file, NO_FILE);
  strcpy(mapstats.startup_file, NO_FILE);
  strcpy(mapstats.placement_file, NO_FILE);
  strcpy(mapstats.objfilename, objfile);

  printf("Creating a new map...\n");

  /* allocate an array of one room */
  room = make_room_array(1);
  set_room_defaults(room);

  /* allocate recorded object list array of size 1 */
  roomrecobj = make_room_obj_list_array(1);
  initialize_room_obj_list_array(roomrecobj, 1);
  
  /* load the object file also */
  load_obj_file(mapstats.objfilename);
}



/* save the current map into a file, rooms and all */

save_map()
{
  int i, j;
  FILE *fp;
  MemObj *ptr;
  
  fp = fopen(mapfile, "w");
  if (fp == NULL) {
    printf("unable to open map file for writing - not saved\n");
    return;
  }

  /* save the room records in the mapfile */
  fwrite((char *)&mapstats, sizeof(mapstats), 1, fp);

  for (i=0; i<mapstats.rooms; i++) {
    /* figure out number of recorded objects */
    room[i].numobjs = number_of_objs_here(i);
    /* write room info record */
    fwrite((char *)&(room[i]), sizeof(RoomInfo), 1, fp);

    /* write recorded objects in this room */
    for (j=0,ptr=roomrecobj[i].next; j<room[i].numobjs; j++,ptr = ptr->next)
      fwrite((char *)&(ptr->obj), sizeof(RecordedObj), 1, fp);
  }

  fclose(fp);
}



/* free the map.  This frees all the rooms and the recorded objects in
   them as well. */

free_map()
{
  int i;
  MemObj *m, *last;

  /* go through each room in the room array and free the various pieces */
  for (i=0; i<mapstats.rooms; i++) {
    /* free recorded object lists */
    for (last=NULL,m=roomrecobj[i].next; m; last=m,m = m->next)
      if (last) free(last);
    if (last) free(last);
  }

  /* free room pointer arrays */
  free(roomrecobj);
  free(room);
}



/* free the objects */

free_objects()
{
  int i;

  for (i=0; i<objects; i++) free(info[(uc)i]);
  objects = 0;
}



/* return the number of recorded objects in this room */

int number_of_objs_here(roomnum)
int roomnum;
{
  MemObj *ptr;
  int result;
  
  /* count how many objects there are */
  for (result=0,ptr=roomrecobj[roomnum].next; ptr; result++,ptr = ptr->next);

  return result;
}


/* allocate space for an array of rooms */

RoomInfo *make_room_array(size)
int size;
{
  RoomInfo *result;
  
  result = (RoomInfo *) malloc(sizeof(RoomInfo) * size);
  if (result == NULL) Gerror("not enough memory for room array");

  return result;
}




/* set defaults for a new room */

set_room_defaults(room)
RoomInfo *room;
{
  register int i, j, k;
  
  bzero(room, sizeof(RoomInfo));
  strcpy(room->name, "no name");
  room->team = 0;
  room->floor = 0;
  room->numobjs = 0;

  /* set exits to go to nowhere */
  for (i=0; i<4; i++) room->exit[i] = -1;

  /* set mapped squares to be empty */
  for (i=0; i<ROOM_HEIGHT; i++)
    for (j=0; j<ROOM_WIDTH; j++)
      for (k=0; k<ROOM_DEPTH; k++)
        room->spot[i][j][k] = 0;

  /* init the various flags */
  room->appearance = PLACE_ANYWHERE;
  room->dark = FALSE;
}



/* add a room to the room array, this will mean allocating a larger array
   space and then copying all of the previous array into it, all the
   appropriate global variables should be updated */

add_new_room()
{
  register int i;
  RoomInfo *newarray;
  MemObj *newlist;

  /* allocate new roominfo array */
  newarray = make_room_array(mapstats.rooms + 1);

  /* copy all of the old info into new array */
  for (i=0; i<mapstats.rooms; i++) newarray[i] = room[i];

  /* set defaults for new room */
  set_room_defaults(newarray + mapstats.rooms);

  /* allocate a new room recorded obj list array */
  newlist = make_room_obj_list_array(mapstats.rooms + 1);

  /* copy the old info into new array */
  initialize_room_obj_list_array(newlist, mapstats.rooms + 1);
  for (i=0; i<mapstats.rooms; i++) newlist[i] = roomrecobj[i];

  /* free the old arrays */
  free(room);
  free(roomrecobj);
  
  /* reset global variables */
  room = newarray;
  roomrecobj = newlist;
  mapstats.rooms++;
}



/* link two rooms together, you are given the first room, the direction
   from that to the second, and the second room */

link_one_room_to_another(room1, dir, room2)
int room1, dir, room2;
{
  /* set exit flag for first room */
  room[room1].exit[dir] = room2;

  /* set exit flag for second room */
  room[room2].exit[opposite_direction(dir)] = room1;
}



/* given a direction number (0-north, 1-east, 2-south, 3-west), return
   the opposite direction */

int opposite_direction(dir)
int dir;
{
  switch (dir) {
    case NORTH:	return SOUTH;
    case EAST:	return WEST;
    case SOUTH:	return NORTH;
    case WEST:	return EAST;
    default: return dir;
  }
}



/* destroy a link found between two adjacent rooms, destroy it from both
   directions if possible */

destroy_direction_link(room1, direction, room2)
int room1, direction, room2;
{
  /* destroy link from starting room */
  room[room1].exit[direction] = -1;

  /* if the other room is linked back, then destroy that link too */
  if (room[room2].exit[opposite_direction(direction)] == room1)
    room[room2].exit[opposite_direction(direction)] = -1;
}



/* destroy a link found through an exit character on a certain spot in
   a certain room, and destroy the way back as well, if there is one.
   Error checks are made to ignore destroy commands when no exit char
   is present in room1.  */

destroy_exit_link(room1, x, y)
int room1, x, y;
{
  MemObj	*first, *second;
  int		room2, destx, desty;
  
  /* find out what x and y this link goes to */
  first = what_recorded_obj_here(room1, x, y);
  if (first == NULL) return;	/* no exit char in room1, do nothing */
  if (!info[(uc)first->obj.objtype]->exit) return;	/* not an exit char */
  room2 = first->obj.detail;
  destx = first->obj.infox;
  desty = first->obj.infoy;

  /* destroy the first link */
  first->obj.detail = -1;

  /* if there is an exit char at destination that points back to here... */
  if (room2 == -1) return;	/* no destination pointed to earlier */
  second = what_recorded_obj_here(room2, destx, desty);
  if (second == NULL) return;	/* no exit char in room2 */
  if (!info[(uc)second->obj.objtype]->exit) return;	/* not an exit char */
  
  /* there is one, check if it points back to room 1 */
  if (second->obj.detail == room1 &&
      second->obj.infox == x  &&  second->obj.infoy == y)
		second->obj.detail = -1;
}



/* copy the mapped characters from the edge of one room onto the edge of
   the next room in a certain direction (there must be a room in that
   direction) */

copy_edge_of_room(room1, direction, room2)
int room1, direction, room2;
{
  register int i, j, k;
  int x1 = 0, y1 = 0, x2 = ROOM_WIDTH-1, y2 = ROOM_HEIGHT-1;
  int xfactor = ROOM_WIDTH-1, yfactor = ROOM_HEIGHT-1;
  
  switch (direction) {
    case 0:	y2 = y1;
		xfactor = 0;
		break;
    case 1:	x1 = x2;
		yfactor = 0;
		break;
    case 2:	y1 = y2;
		xfactor = 0;
		break;
    case 3:	x2 = x1;
		yfactor = 0;
		break;
  }

  /* now do the copying, given the x1, y1, x2, y2 constraints formed above */
  for (i=x1; i<=x2; i++)
    for (j=y1; j<=y2; j++)
      for (k=0; k<ROOM_DEPTH; k++)
        room[room2].spot[abs(xfactor-i)][abs(yfactor-j)][k] =
						room[room1].spot[i][j][k];
}



/* allocate space for an array of head pointers to lists of recorded objects
   in rooms */

MemObj *make_room_obj_list_array(size)
int size;
{
  MemObj *result;

  result = (MemObj *) malloc(sizeof(MemObj) * size);
  if (result == NULL) Gerror("not enough memory for recorded obj array");

  return result;
}



/* Initializes room object list header next pointers to NULL */

initialize_room_obj_list_array(arrayptr, size)
MemObj *arrayptr;
int size;
{
  register int i;

  for (i=0; i<size; i++)
    arrayptr[i].next = (MemObj *) NULL;
}



/* add a recorded object record from the file to the memory obj list */

add_file_record_to_mem(roomnum, record)
int roomnum;
RecordedObj *record;
{
  MemObj *newmemobj;

  newmemobj = allocate_mem_obj();

  newmemobj->obj = *record;
  newmemobj->next = (MemObj *) NULL;

  add_obj_to_room(roomnum, newmemobj);
}



/* add a memory object to a room's list of recorded objects */

add_obj_to_room(roomnum, objrecord)
int roomnum;
MemObj *objrecord;
{
  MemObj *ptr;
  
  /* go to the end of the list */
  for (ptr=(roomrecobj+roomnum); ptr->next !=(MemObj *)NULL; ptr = ptr->next);

  /* add on the new object */
  ptr->next = objrecord;
  objrecord->next = (MemObj *) NULL;
}



/* allocate a memory recorded object record */

MemObj *allocate_mem_obj()
{
  MemObj *result;
  register int i;
  
  result = (MemObj *) malloc(sizeof(MemObj));
  if (result == NULL) Gerror("not enough memory for a memory object");

  /* initialize the variables inside */
  result->obj.x = -1;		result->obj.y = -1;
  result->obj.objtype = 0;	result->obj.detail = -1;
  result->obj.infox = -1;	result->obj.infoy = -1;
  result->obj.zinger = -1;	result->next = NULL;
  for (i=0; i<3; i++)	result->obj.extra[i] = -1;

  return result;
}



/* this procedure places an object into a square.  It attempts to be
   intelligent about it, replacing first any 0 object it finds on the spot
   and then, if there are none, it replaces the first obj of the same type
   (masked or non-masked) it can find in the list.  If it can find its
   own type on the list already, then it does nothing.  */

put_down_mapped_obj(x, y, n)
int x, y;
unsigned char n;
{
  register int i;
  int found;
  
  /* check to see if this kind of char is already here */
  for (i=0,found = -1; i<ROOM_DEPTH && found == -1; i++)
    if (room[current_room].spot[x][y][i] == n) found = i;

  /* if it is already there then leave without doing anything */
  /* if its object #0 then it doesn't matter if it's already there */
  if (found != -1 && n != 0) return;

  /* look for obj of same masking type */
  for (i=0,found = -1; i<ROOM_DEPTH && found == -1; i++)
      if (info[(unsigned char)room[current_room].spot[x][y][i]]->masked ==
	  info[(unsigned char)n]->masked) found = i;

  /* if no same type found, look for empty obj spot */
  if (found == -1 )
    for (i=0; i<ROOM_DEPTH && found == -1; i++)
      if (room[current_room].spot[x][y][i] == 0)
        found = i;

  /* if still nothing found then just replace the first one */
  if (found == -1) found = 0;
  /* replace the object */

  room[current_room].spot[x][y][found] = n;
}



/* this procedure puts a recorded object into a room, ie. an exit object
   or some other special characteristic saving object.  If there are special
   default values defined for the object then use those values instead
   of the normal -1 initialization. */

put_down_recorded_obj(x, y, n)
int x, y;
unsigned char n;
{
  MemObj *oldobj, *newobj;
 
  /* if there is a recorded object here store value temporarily and remove */
  oldobj = what_recorded_obj_here(current_room, x, y);
  remove_recorded_obj(current_room, x, y);

  /* get an object record and place the location info in it */
  newobj = allocate_mem_obj();
  newobj->obj.x = x;
  newobj->obj.y = y;
  newobj->obj.objtype = n;

  /* set details to the "no set info" code or special default */
  newobj->obj.detail = rec_obj_default(n, 1);
  newobj->obj.infox =  rec_obj_default(n, 2);
  newobj->obj.infoy =  rec_obj_default(n, 4);
  newobj->obj.zinger = rec_obj_default(n, 8);
  newobj->obj.extra[0] = rec_obj_default(n, 16);
  newobj->obj.extra[1] = rec_obj_default(n, 32);
  newobj->obj.extra[2] = rec_obj_default(n, 64);
  newobj->next = NULL;

  /* add the new object to the room list */
  add_obj_to_room(current_room, newobj);
}




/* figure out the recorded object default value for a certain type of
   object given which variable we should be filling.  When there is 
   no set default, -1 if used. */

int rec_obj_default(type, varnum)
int type, varnum;
{
  int i, j, result = -1;

  /* figure out which default array element the varnum refers to */
  for (j=0,i=1; i<varnum; j++,i*=2);

  /* if there is a default then look at it */
  if (info[(unsigned char)type]->set[j]) result = (info[(unsigned char)type]->def[j]);

  /* return default value that is appropriate */
  return result;
}



/* test to see if there is any recorded object on a certain square in a
   certain room (only one recorded object is allowed in each square),
   if there is such an object, return a pointer to its record, otherwise
   return a NULL pointer.  If there is an illegal object here, then
   return NULL as well. */

MemObj *what_recorded_obj_here(roomnum, x, y)
int roomnum, x, y;
{
  MemObj *ptr;

  /* check for illegal x and y coordinates */
  if (x<0 || y<0 || x>=ROOM_WIDTH || y>=ROOM_HEIGHT) return (MemObj *) NULL;

  /* go through all objects in list (except first dummy object) and see
     if it is in the requested location, if so, return it */

  for (ptr=roomrecobj[roomnum].next; ptr != NULL; ptr = ptr->next)
    if (x == ptr->obj.x && y == ptr->obj.y) {
	/* check for validity of the number of the object */
	if (ptr->obj.objtype >= 0 && ptr->obj.objtype < objects) return ptr;
	else return NULL;
    }

  /* nothing found, so return NULL */
  return (MemObj *) NULL;
}



/* return plain TRUE or FALSE in response to whether there is a
   recorded object already at a specific x and y in a room */

int is_recorded_obj_here(roomnum, x, y)
int roomnum, x, y;
{
  MemObj *tmp;

  tmp = what_recorded_obj_here(roomnum, x, y);

  return (tmp != NULL);
}



/* check to see if there are any recorded objects that is an exit nearby,
   return a pointer to found recorded obj, or NULL if we find none.
   Report the direction we had to look to find it.  */

MemObj *nearby_exit_character(roomnum, x, y, dir)
int roomnum, x, y;
int *dir;
{
  int		i, done = FALSE;
  MemObj	*result;

  /* go through each direction until we find an exit character */
  for (i=0; i<4 && !done; i++) {

    result = what_recorded_obj_here(roomnum, x + xdir[i], y + ydir[i]);

    /* if this is an exit character, then hurray, we can stop looking */
    if (result != NULL) {
      if (info[(uc)result->obj.objtype]->exit) done = TRUE;
      *dir = i;
    }
  }

  return result;
}



/* will remove a recorded object from certain x and y in a room if such
   an object exists, if there is none then it will simply do nothing */

remove_recorded_obj(roomnum, x, y)
int roomnum, x, y;
{
  MemObj *ptr, *previous;

  /* go through all the recorded objects in a room, if we find any at the
     x and y we are looking for then delete it from the list */

  previous = (roomrecobj + roomnum);
  for (ptr=previous->next; ptr != NULL; previous=ptr,ptr=ptr->next)
    if (x == ptr->obj.x  &&  y == ptr->obj.y) {
      previous->next = ptr->next;
      free(ptr);
    }
}



/* the following procedure returns a pointer to an unsigned char array of
   length ROOM_DEPTH + 1.  It contains a complete list of what is on the
   requested square, including a recorded object if there is one.
   If there are any illegal objects/out-of-bounds object numbers in the
   square, they are returned as is, which might cause errors in some
   of the more naive callers. */

unsigned char *whats_really_on_square(roomnum, x, y)
int roomnum, x, y;
{
  register int i;
  static unsigned char result[ROOM_DEPTH + 1];
  MemObj *ptr;

  /* get mapped objects */
  for (i=0; i<ROOM_DEPTH; i++)
    result[i] = room[roomnum].spot[x][y][i];

  /* now look for a recorded object on the square */
  ptr = what_recorded_obj_here(roomnum, x, y);

  if (ptr == NULL)
    result[i] = 0;
  else
    result[i] = ptr->obj.objtype;

  return result;
}



/* the following procedure returns a pointer to an unsigned char array of
   length ROOM_DEPTH + 1.  It contains a complete list of what is on the
   requested square, including a recorded object if there is one.
   If there are any illegal objects/out-of-bounds object numbers in the
   square, they are returned as zeros. */

unsigned char *whats_on_square(roomnum, x, y)
int roomnum, x, y;
{
  int i;
  unsigned char *result;

  result = whats_really_on_square(roomnum, x, y);
  for (i=0; i<(ROOM_DEPTH+1); i++)
    if (result[i] < 0 || result[i] >= objects) result[i] = 0;

  return result;
}



/* look through the objects on a square and determine if there is anything
   takeable.  Return pointer to first takeable object or return NULL
   when there is nothing to take here */

MemObj *takeable_on_square(roomnum, x, y)
int roomnum, x, y;
{
  static MemObj result;
  int i, otype;
  MemObj *ptr;

  /* look at the mapped objects first, if we find one thats takeable return */
  for (i=0; i<ROOM_DEPTH; i++) {
    otype = room[roomnum].spot[x][y][i];
    if (info[(uc)otype]->takeable) {
      result.obj.objtype = otype;
      return &result;
    }
  }

  /* if no takeable mapped chars, take a look at the recorded object */
  ptr = what_recorded_obj_here(roomnum, x, y);

  /* if there's nothing then we've struck out */
  if (ptr == NULL) return NULL;

  /* check to see if it is takeable */
  if (info[(uc)ptr->obj.objtype]->takeable) return ptr;
  else return NULL;
}



/* look for an empty slot on a map square, returning the number 0, 1, or 2
   or the empty slot or -1 if no slot was found.  If recorded is passed as
   TRUE then check the recorded slot instead of the map spots, returning 2
   (ROOM_DEPTH) if recorded slot is open or -1 if it is not */

int find_empty_slot(roomnum, x, y, recorded)
int roomnum, x, y, recorded;
{
  unsigned char *item_list;
  int i, found = -1;
  
  /* check for reasonable room number and xy location */
  if (roomnum < 0  ||  roomnum >= mapstats.rooms  || x<0 || y<0 ||
      x >= ROOM_WIDTH  ||  y >= ROOM_HEIGHT) return -1;

  /* get a list of what is on the square */
  item_list = whats_on_square(roomnum, x, y);

  /* if recorded slot is to be checked, check it and return */
  if (recorded) {
    if (!item_list[ROOM_DEPTH]) return ROOM_DEPTH;
    else return -1;
  }

  /* otherwise check mapped squares for an empty slot */
  for (i=0; (i<ROOM_DEPTH  && found == -1); i++)
    if (item_list[i] == 0) found = i;

  return found;
}




/* check to see whether there are any visual obstructions on a given
   square, if so return TRUE */

int has_view_obstruction(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *list;
  int i, obstructed = FALSE;

  /* find out what's here */
  list = whats_on_square(roomnum, x, y);

  /* return TRUE if any view obstructions are found */
  for (i=0; i<=ROOM_DEPTH && !obstructed; i++)
    if (!info[(uc)list[i]]->transparent) obstructed = TRUE;
    
  return obstructed;
}



/* remove a given object from a square, if there is more than one object
   here of the same type, then remove the first one, return 1 if something
   is removed, 0 if it wasn't there. */

int remove_obj_from_square(roomnum, x, y, objnum)
int roomnum, x, y, objnum;
{
  int i, found = -1;
  unsigned char *stuff_here;

  /* find out what is on this square */
  stuff_here = whats_on_square(roomnum, x, y);

  /* get number of first object of same type on square */
  for (i=0; i<ROOM_DEPTH + 1; i++)
    if (stuff_here[i] == objnum) {
      found = i;
      break;
    }

  /* if it wasn't there then don't bother removing it */
  if (found == -1) return 0;

  /* if it was then remove it */
  if (found < ROOM_DEPTH) room[roomnum].spot[x][y][found] = 0;
  else remove_recorded_obj(roomnum, x, y);

  /* return 1 to show that something was removed */
  return 1;
}



/* return the most prominant object type on any particular square, ie.
   a wall character, if there is one, or the floor underneath it there
   is nothing on it. */

int most_prominant_on_square(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int i, found = -1;

  /* get everything on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look for first masked type object */
  for (i=0; (i<ROOM_DEPTH + 1) && (found == -1); i++)
    if (info[(uc)whats_here[i]]->masked) found = i;

  /* we found nothing then get the first thing there */
  if (found == -1) found = 0;

  /* return the thing we've found */
  return ((int) whats_here[found]);
}



/* find out what the probable floor character is on this square */

int probable_floor_here(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int i, found = -1;

  /* get everything on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look for first non-masked type object */
  for (i=0; (i<ROOM_DEPTH + 1) && (found == -1); i++)
    if (!info[(uc)whats_here[i]]->masked) found = i;

  /* we found nothing then get the first thing there */
  if (found == -1) found = 0;

  /* return the thing we've found */
  return ((int) whats_here[found]);
}


  

/* find out whether we can walk on the following square, if so, return
   the speed number value, if not, return 0 */

int max_move_on_square(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int		i, minimum = 9999;

  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for the lowest possible speed number */
  for (i=0; i < ROOM_DEPTH + 1; i++)
      if (info[(uc)whats_here[i]]->movement < minimum)
	minimum = info[(uc)whats_here[i]]->movement;

  return minimum;
}



/* find out whether we can fire onto a given square, if so return TRUE.
   We may not fire onto squares that are non-permeable and don't allow
   movement on them. */

int may_fire_onto_square(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int		i, move = TRUE, perm = TRUE;

  /* if out of bounds then sure, we can fire there */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return TRUE;

  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for a missile stopping object */
  for (i=0; (i < ROOM_DEPTH + 1); i++) {
    if (!info[(uc)whats_here[i]]->permeable) perm = FALSE;
    if (!info[(uc)whats_here[i]]->movement) move = FALSE;
  }

  return (perm || move);
}




/* find out whether we can fire over the given square, if so return TRUE.
   We may not fire through squares that are non-permeable. */

int may_fire_through_square(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int           i, okay = TRUE;

  /* if out of bounds then sure, we can fire there */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return TRUE;

  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for a missile stopping object */
  for (i=0; (i < ROOM_DEPTH + 1) && okay; i++)
    if (!info[(uc)whats_here[i]]->permeable) okay = FALSE;

  return okay;
}




/* given a character x and y location in this room, find out what a cursor
   following command should take as the direction:
     -1:  exit character on this square
      0:  north
      1:  east
      2:  south
      3:  west    if there is such a room then assign it to destination_room
		  if no such room. then destination_room = -1  */

int follow_insinuation(roomnum, x, y, destination_room)
int roomnum, x, y;
int *destination_room;
{
  int minimum, result;
  MemObj *recorded_obj;

  /* check for an exit character on this square */
  recorded_obj = what_recorded_obj_here(roomnum, x, y);
  if (recorded_obj != NULL)
    if (info[(uc)recorded_obj->obj.objtype]->exit) {
      *destination_room = recorded_obj->obj.detail;
      return -1;
    }
    
  /* find out what direction is most signified by the cursor position */
  result = 0;   minimum = y;
  if (x<minimum) {
    minimum = x;
    result = 3;
  }
  if (ROOM_HEIGHT - y - 1 < minimum) {
    minimum = ROOM_HEIGHT - y - 1;
    result = 2;
  }
  if (ROOM_WIDTH - x - 1 < minimum) {
    minimum = ROOM_WIDTH - y - 1;
    result = 1;
  }

  /* set the destination_room variable according to exit found there */
  *destination_room = room[roomnum].exit[result];

  return result;
}



/* given a room and an illegal location to go to, return the new room
   that the illegal movement goes into, or -1 if there is no such
   room.  Direct diagonals out of the corner of the room will produce
   a -1 result */

new_room_for_refire(location, roomnum)
Location *location;
int roomnum;
{
  int dir = -1;

  /* check for bad room number */
  if (roomnum < 0 || roomnum >= mapstats.rooms) return -1;

  /* figure out the direction from room we are going in, if we are
     leaving room diagonally then revert back to -1 as direction */
  if (location->y < 0) dir = 0;
  if (location->x >= ROOM_WIDTH) dir = (dir < 0) ? 1 : -1;
  if (location->y >= ROOM_HEIGHT) dir = (dir < 0) ? 2 : -1;
  if (location->x < 0) dir = (dir < 0) ? 3 : -1;

  /* if no legal direction found then barf up */
  if (dir < 0) return -1;

  /* return the room we will be going to */
  return room[roomnum].exit[dir];
}
