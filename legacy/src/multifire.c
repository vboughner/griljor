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

/* Routines for firing many missiles at once */

#include <stdio.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gametime.h"
#include "movement.h"


/* Type defininitions for our book-keeping in this file */

typedef struct _multi_shot {
	char x1, y1, x2, y2;	/* start and stop coords for this missile */
	struct _multi_shot *next;
} MultiShot;

/* Globals for this file's use only */

int		preparing = FALSE;	/* TRUE when a packet is started */
MultiPack	multi_pack;		/* the packet in preparation to send */
MultiShot	*multi_list = NULL;	/* the list of missiles for packet */
MultiShot	*multi_end = NULL;	/* last element in the list */

/* To send a multi-missile packet:
	- call start_multi_pack with given info
	- call add_to_multi_pack for each missile that you have
	- call send_multi_pack with a valid sender idnum to send it off */


Location *missile_arc();


/* start the making of a multi missile sendoff by getting straight a few
   constants all these missiles share */

start_multi_pack(idnum, roomnum, include_start, type, hurts)
int idnum, roomnum, include_start, type, hurts;
{
  if (preparing) Gerror("already started a multi-pack");

  preparing = TRUE;
  multi_pack.miss.owner = idnum;
  multi_pack.miss.type = type;
  multi_pack.miss.hurts_owner = hurts;
  multi_pack.miss.room = roomnum;
  multi_pack.include_start = include_start;
}

/* add a missile to this planned packet */

add_to_multi_pack(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  MultiShot *new;

  new = (MultiShot *) malloc(sizeof(MultiShot));
  demand(new, "no memory for multi-shot");

  /* put values into it */
  new->x1 = x1; 
  new->y1 = y1;
  new->x2 = x2;
  new->y2 = y2;
  new->next = NULL;

  /* add it to the end of the list of missiles to far */
  if (multi_end)
    multi_end->next = new;
  else
    multi_list = new;

  multi_end = new;
}



/* free the given list of missile shots, by recursion */

free_shot_list(list)
MultiShot *list;
{
  if (list) {
    free_shot_list(list->next);
    free(list);
  }
}

/* send the multi pack off to everyone, including myself.  If there are
   too many missiles to fit into one packet, then send several until
   all missiles are sent off */

send_multi_pack(senderid)
int senderid;
{
  register int i;
  MultiShot *ptr;

  /* look for an error condition */
  if (!preparing || !multi_list) Gerror("no start and fill of multipack");

  /* address the packet */

  ptr = multi_list;
  do {
    /* fill the packet */
    for (i=0; (i<MAX_MULTI_MISSILES && ptr); i++) {
      multi_pack.x1[i] = ptr->x1;
      multi_pack.y1[i] = ptr->y1;
      multi_pack.x2[i] = ptr->x2;
      multi_pack.y2[i] = ptr->y2;
      ptr = ptr->next;
    }
    multi_pack.number = i;

    prepare_and_send_multimissile_packet(&multi_pack);
    add_multi_missiles(&multi_pack);
  } while (ptr);

  /* free the list of missiles */
  free_shot_list(multi_list);

  /* reset multi packet building status */
  multi_list = NULL;
  multi_end = NULL;
  preparing = FALSE;
}


/* add all of the missiles in a multi missile packet */

add_multi_missiles(mm)
MultiPack *mm;
{
  register int i;
  int length;
  Missile *new;
  Location *list;
  Missile dummy;

  /* go through all the missiles in the packet */
  for (i=0; i < mm->number; i++) {
    /* allocate memory for a new missile record */
    new = (Missile *) malloc(sizeof(Missile));
    if (new == NULL) Gerror("out of memory making missile");
    /* copy information into missile packet */
    new->owner = mm->miss.owner;
    new->type = mm->miss.type;

    /* signal this missile as a 'new' missile */
    new->lastmove = 0;
    new->x = -1;        new->y = -1;
    new->oldx = -1;	new->oldy = -1;

    new->hurts_owner = mm->miss.hurts_owner;
    new->heading =
      MissileHeading(mm->x1[i], mm->y1[i], mm->x2[i], mm->y2[i], 0);
				  
    /* make a movement list for the missile */
    new->moveq =
      diag_movement_list(mm->x1[i], mm->y1[i], mm->x2[i], mm->y2[i], TRUE);

    new->wait = missile_wait(new->type);
    
    remove_move_head(&new->moveq); 

    /* figure out the directional orientation of this missile */
    prepare_missile_direction(&dummy, new->type, mm->x1[i], mm->y1[i],
			      mm->x2[i], mm->y2[i]);

    new->direction = dummy.direction;
    /* place this missile at the beginning of the room missile list */
    new->room = mm->miss.room;
    AddMissileToHeap(new);
  }

  if (mm->include_start) CreateStationaryMissile(mm);
}

/*****************************************************************/

CreateStationaryMissile(mp)
MultiPack *mp;
{
  Location *move;
  Missile *new;
  int x,y;

  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory making missile");

  new->owner = mp->miss.owner;
  new->type = mp->miss.type;
  new->wait = missile_wait(new->type);
    
  
  new->lastmove = 0;
  new->x = -1;        new->y = -1;
  new->oldx = -1;	new->oldy = -1;

  new->hurts_owner = mp->miss.hurts_owner;
  new->heading = 0;

  x = mp->x1[0];
  y = mp->y1[0];
  move = makelocation(x,y,NULL);
  new->moveq = makelocation(x,y,move);

  new->direction = 0;
  /* place this missile at the beginning of the room missile list */
  new->room = mp->miss.room;
  AddMissileToHeap(new);
}

/* ==================== A U T O  missile effects ======================== */

/* This routine will randomly make missiles of the given type appear
   all over the given room, owned by the given person, and sent under
   the authority of the given id number.  Distance specifies how many
   squares above the spots to start the firing. */

random_missile_barrage(pnum, senderid, roomnum, type, distance)
int pnum, senderid, roomnum, type, distance;
{
  int i, x, y;

  start_multi_pack(gameperson[pnum]->id, roomnum, TRUE,
		   (int) ((unsigned char) info[type]->movingobj),FALSE);

  for (i=0; i<70; i++) {
    x = lrand48() % ROOM_WIDTH;
    y = lrand48() % ROOM_HEIGHT;
    add_to_multi_pack(x, y - distance, x, y);
  }

  send_multi_pack(senderid);
}



/* this one uses the hand combat weapon as a random barrage */

operation_big_fist(pnum)
int pnum;
{
  if (info[0]->weapon) {
    random_missile_barrage(pnum, gameperson[pnum]->id, 
			   gameperson[pnum]->room, 0, 3);
  }
}



/* this one makes an arc of the given missile type occur as a multifire
   missile package.  dir is the direction (in degrees) to fire and
   Dtheta is the amount of swing to each side of the given angle. */

operation_arc_fire(ownerid, senderid, include_start, type, roomnum, x, y,
		   radius, dir, Dtheta, hurts, spread)
int ownerid, senderid, include_start, type; 
int roomnum, x, y, radius, dir, Dtheta, hurts, spread;
{
  int i;
  Location *list, *ptr;

  if (radius < 0) Gerror("negative radius not allowed in operation_arc_fire");

  start_multi_pack(ownerid, roomnum, include_start, type, hurts);
  
  /* get a list of the destinations */
  list = missile_arc(x, y, radius, dir, Dtheta, 0,spread);

  for (ptr=list; ptr; ptr = ptr->next) 
    add_to_multi_pack(x, y, (int)ptr->x, (int)ptr->y);

  free_move_queue(&list);
  
  send_multi_pack(senderid);
}



/* cause an explosion at a given square */

explode_at_square(senderid, roomnum, x, y, radius, type,
		  arc, spread, direction)
int senderid, roomnum, x, y, radius, type, arc, spread, direction;
{
  if (arc == 0) 
    arc = 180;

  operation_arc_fire(senderid, senderid, TRUE, type, roomnum,
		     x, y, radius, direction, arc, TRUE, spread);
}
