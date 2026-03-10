/***************************************************************************
 * @(#) map.c 1.10 - last change made 08/07/92
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

/* Map record manipulation routines */

#define MAP_MAIN

#include <string.h>
#include <stdio.h>
#include "config.h"
#include "def.h"
#include "objects.h"
#include "map.h"
#include "mapstore.h"
#include "mapfunc.h"


/* Global variables and those concerning the current map */
/* These are very similar to the original map globals, and so don't
   conform very naturally to the new way of dealing with maps.  It made
   fewer changes to the code neccessary during the upgrade, however. */

MapInfo		mapstats;	/* info about loaded map */
RoomInfo	*room;		/* pointer to array of rooms in map */
short		current_room;	/* number of room we are in now */

char		mapfile[120];
char		objectfile[120];

const int	xdir[4] = { 0, 1, 0, -1 };
const int	ydir[4] = { -1, 0, 1, 0 };

MapInfo		*origmap;	/* pointer to copy of original unchanged map */



/* ================= F O R E  F U N C T I O N S ===================== */
/* These functions provide a front to the more generic mapfunc.c routines */



void load_map()
/* load the map from a file - the driver and map editor make use of this.
   Object set gets loaded during map file load. */
{
  FILE *fp;
  MapInfo *map = NULL;

  fp = fopen(mapfile, "r");
  if (fp) {
    /* read and set up the map information into the way we use it */
    map = lib_read_map_header_from_fd(fileno(fp));
    if (!map) Gerror("header is wrong in map file");
    bcopy(map, &mapstats, sizeof(MapInfo));
    room = mapstats.room;
    free(map);

    /* load the object file before loading object instances */
    if (strcmp(objectfile, UNKNOWN)) strcpy(mapstats.objfilename, objectfile);
    load_obj_file(mapstats.objfilename);

    /* now load the object instances in the map */
    while (lib_read_mapobj_from_fd(&mapstats, fileno(fp)));
    fclose(fp);
  }
  else {
    Gerror("unable to read map file");
  }
}



void create_new_map(objfile)
char *objfile;
{
  MapInfo *map = lib_create_new_map(objfile);
  if (!map) {
    Gerror("unable to read map file");
  }
  else {
    /* set up the map information into the way we use it */
    bcopy(map, &mapstats, sizeof(MapInfo));
    room = mapstats.room;
    free(map);

    /* load the object file also */
    load_obj_file(mapstats.objfilename);
  }
}



void save_map()
{
  if (!lib_save_map(&mapstats, mapfile)) {
    fprintf(stderr, "Warning: could not write to file %s\n", mapfile);
    /**** COMPLETE: somewhere, user should get another crack at naming */
  }
}



void add_new_room()
/* add a room to the map, this will mean allocating a larger array
   space and then copying all of the previous array into it, all the
   appropriate global variables should be updated */
{
  lib_add_new_room(&mapstats);
  room = mapstats.room;
}



void free_map()
{
  MapInfo *map = (MapInfo *) malloc(sizeof(MapInfo));
  demand(map, "not enough room for temporary map info copy");
  bcopy(&mapstats, map, sizeof(MapInfo));
  lib_free_map(map);
}



void put_down_object(x, y, n)
/* this procedure places an object into a square. */
int x, y;
unsigned char n;
{
  if (!is_valid_square(&mapstats, current_room, x, y))
    Gerror("bad square in put_down_object");

  add_object_to_square(&mapstats, current_room, x, y,
		       create_object(&mapstats, n, ANY_ID));
}



int has_view_obstruction(roomnum, x, y)
/* check to see whether there are any visual obstructions on a given
   square, if so return TRUE */
int roomnum, x, y;
{
  SquareRec *square = get_square(&mapstats, roomnum, x, y);
  if (square) return(view_obstructed_by_square(square));
  else return(FALSE);
}



int most_prominant_on_square(roomnum, x, y)
/* return the most prominant object type on any particular square, ie.
   a wall character, if there is one, or the floor underneath it if there
   is nothing else on it. */
int roomnum, x, y;
{
  OI *obj = last_obj_here(roomnum, x, y);
  if (!obj) return(0);
  else return(obj->type);
}



int probable_floor_here(roomnum, x, y)
/* find out what the probable floor character is on this square */
int roomnum, x, y;
{
  OI *obj;
  SquareRec *square = get_square(&mapstats, roomnum, x, y);
  if (!square) return(0);
  obj = square_last_masked(square, FALSE);
  if (!obj) return(0);
  else return(obj->type);
}

  

int max_move_on_square(roomnum, x, y)
/* find out whether we can walk on the following square, if so, return
   the speed number value, if not, return 0 */
int roomnum, x, y;
{
  return(movement_on_square(get_square(&mapstats, roomnum,x, y)));
}



int may_fire_onto_square(roomnum, x, y)
/* find out whether we can fire onto a given square, if so return TRUE.
   We may not fire onto squares that are non-permeable and don't allow
   movement on them. */
int roomnum, x, y;
{
  return(firing_onto_square(get_square(&mapstats, roomnum, x, y)));
}



int may_fire_through_square(roomnum, x, y)
/* find out whether we can fire over the given square, if so return TRUE.
   We may not fire through squares that are non-permeable. */
int roomnum, x, y;
{
  return(firing_through_square(get_square(&mapstats, roomnum, x, y)));
}



int new_room_for_refire(location, roomnum)
/* given a room and an illegal location to go to, return the new room
   that the illegal movement goes into, or -1 if there is no such
   room.  Direct diagonals out of the corner of the room will produce
   a -1 result */
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
