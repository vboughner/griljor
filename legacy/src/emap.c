/***************************************************************************
 * @(#) emap.c 1.4 - last change made 08/07/92
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

/* routines that mangle the map for editmap program */

#include <string.h>
#include <stdio.h>
#include "config.h"
#include "def.h"
#include "objects.h"
#include "map.h"
#include "mapfunc.h"



void link_one_room_to_another(room1, dir, room2)
/* link two rooms together, you are given the first room, the direction
   from that to the second, and the second room */
int room1, dir, room2;
{
  /* set exit flag for first room */
  room[room1].exit[dir] = room2;

  /* set exit flag for second room */
  room[room2].exit[opposite_direction(dir)] = room1;
}



int opposite_direction(dir)
/* given a direction number (0-north, 1-east, 2-south, 3-west), return
   the opposite direction */
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



void destroy_direction_link(room1, direction, room2)
/* destroy a link found between two adjacent rooms, destroy it from both
   directions if possible */
int room1, direction, room2;
{
  /* destroy link from starting room */
  if (room1 >= 0 && room1 < mapstats.rooms) room[room1].exit[direction] = -1;

  /* if the other room is linked back, then destroy that link too */
  if (room2 >= 0 && room2 < mapstats.rooms) {
    if (room[room2].exit[opposite_direction(direction)] == room1)
      room[room2].exit[opposite_direction(direction)] = -1;
  }
}



void destroy_exit_link(room1, x, y)
/* destroy a link found through an exit character */
int room1, x, y;
{
  if (room1 >= 0 && room1 < mapstats.rooms) {
    ObjectInstance *exit1;

    exit1 = what_exit_obj_here(&mapstats, room1, x, y);
    if (!exit1) return; 	/* no exit char in room1, do nothing */

    /* destroy the first link */
    set_record(exit1, REC_DETAIL, -1);
    set_record(exit1, REC_INFOX, -1);
    set_record(exit1, REC_INFOY, -1);
  }
}



void copy_edge_of_room(room1, direction, room2)
/* copy the map from the edge of one room onto the edge of
   the next room in a certain direction (there must be a room in that
   direction) */
int room1, direction, room2;
{
  int i, j, k;
  int x1 = 0, y1 = 0, x2 = ROOM_WIDTH-1, y2 = ROOM_HEIGHT-1;
  int xfactor = ROOM_WIDTH-1, yfactor = ROOM_HEIGHT-1;
  SquareRec *square1;
  
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
  if ((room1 >= 0 && room1 < mapstats.rooms) &&
      (room2 >= 0 && room2 < mapstats.rooms)) {
    for (i=x1; i<=x2; i++)
      for (j=y1; j<=y2; j++) {
	square1 = get_square(&mapstats, room1, i, j);
	copy_square_overwrite(square1, &mapstats, room2,
			      abs(xfactor-i), abs(yfactor-j));
      }
  }
}



OI *nearby_exit_character(roomnum, x, y, dir)
/* check to see if there are any recorded objects that is an exit nearby,
   return a pointer to found exit object, or NULL if we find none.
   Report the direction we had to look to find it.  */
int roomnum, x, y;
int *dir;
{
  OI *result;
  int i;

  /* go through each direction until we find an exit character */
  for (i=0; i<4; i++) {

    result = what_exit_obj_here(&mapstats, roomnum, x + xdir[i], y + ydir[i]);

    if (result) {
      *dir = i;
      return(result);
    }
  }
   
  return(NULL);
}



int follow_insinuation(roomnum, x, y, destination_room)
/* given a character x and y location in this room, find out what a cursor
   following command should take as the direction:
     -1:  exit character on this square
      0:  north
      1:  east
      2:  south
      3:  west    if there is such a room then assign it to destination_room
		  if no such room. then destination_room = -1  */
int roomnum, x, y;
int *destination_room;
{
  OI *obj;
  int minimum, result;

  /* check for an exit character on this square */
  obj = what_exit_obj_here(&mapstats, roomnum, x, y);
  if (obj) {
    *destination_room = get_record(obj, REC_DETAIL);
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
  if (roomnum < 0 || roomnum >= mapstats.rooms)
    *destination_room = -1;
  else
    *destination_room = room[roomnum].exit[result];

  return(result);
}
