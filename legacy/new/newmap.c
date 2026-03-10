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

/* Routines for accessing the map and room data abstractions */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "newmap.h"



static InstanceLocation *NewInstanceLocation()
{
  InstanceLocation *new;
  NEW_CLEARED(new, (InstanceLocation *), sizeof(InstanceLocation));
  return(new);
}



static ObjInstance *NewObjInstance(type, id)
short type;
long id;
{
  ObjInstance *new;
  NEW_CLEARED(new, (ObjInstance *), sizeof(ObjInstance));
  SET_OI_type(new, type);
  SET_OI_id(new, id);
  if (info[type]->recorded)
    NEW_CLEARED(new->recorded,(short *),(sizeof(short) * RECORDED_SIZE));
  if (info[type]->varsize)
    NEW_CLEARED(new->variables,(long *),(sizeof(long) * info[type]->varsize));
  SET_OI_loc(new, NewInstanceLocation());
  return(new);
}



static SquareLink *NewSquareLink(object, next)
ObjInstance *object;
SquareLink *next;
{
  SquareLink *new;
  NEW(new, (SquareLink *), sizeof(SquareLink));
  SET_SL_object(new, object);
  SET_SL_next(new, next);
  return(new);
}



static SquareInfo *NewSquareInfo()
{
  SquareInfo *new;
  NEW_CLEARED(new, (SquareInfo *), sizeof(SquareInfo));
  return(new);
}



static RoomInfo *NewRoomInfo()
{
  int i, j;
  RoomInfo *new;
  NEW_CLEARED(new, (RoomInfo *), sizeof(RoomInfo));
  for (i=0; i<ROOM_WIDTH; i++)
    for (j=0; j<ROOM_HEIGHT; j++)
      SET_RI_square(new, i, j, NewSquareInfo());
  return(new);
}



static MapInfo *NewMapInfo()
{
  MapInfo *new;
  NEW_CLEARED(new, (MapInfo *), sizeof(MapInfo));
  NEW_CLEARED(new->hash, (ObjInstance **), (sizeof(ObjInstance *) *HASH_SIZE));
  return(new);
}
