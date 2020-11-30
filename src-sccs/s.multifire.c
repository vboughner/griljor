h49391
s 00000/00000/00310
d D 1.10 92/08/07 01:02:40 vbo 10 9
c source copied to a separate tree for work on new map and object format
e
s 00036/00051/00274
d D 1.9 92/01/20 16:30:03 labc-4lc 9 8
c blojo finished changes necesssary for network packets
e
s 00002/00002/00323
d D 1.8 91/09/06 00:35:42 labb-3li 8 7
c added reliability flag to all procedure called send_to_...()
e
s 00019/00011/00306
d D 1.7 91/08/29 01:40:42 vanb 7 6
c fixed up more compatibility problems
e
s 00006/00006/00311
d D 1.6 91/06/30 16:57:32 labc-3id 6 5
c removed all traces of old explosion 'special' types
e
s 00008/00006/00309
d D 1.5 91/05/26 22:45:09 labc-3id 5 4
c 
e
s 00057/00014/00258
d D 1.4 91/05/20 23:53:41 labc-3id 4 3
c T: fixed all sorts of missle bugs and what not...
e
s 00016/00008/00256
d D 1.3 91/02/23 01:48:52 labc-3id 3 2
c 
e
s 00001/00000/00263
d D 1.2 91/02/18 16:44:54 labc-3id 2 1
c Fixed scpecials bug for explosions
e
s 00263/00000/00000
d D 1.1 91/02/16 12:55:07 labc-3id 1 0
c date and time created 91/02/16 12:55:07 by labc-3id
e
u
U
f e 0
t
T
I 1
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
D 3
#include "special.h"
E 3
#include "gametime.h"
#include "movement.h"


/* Type defininitions for our book-keeping in this file */

typedef struct _multi_shot {
	char x1, y1, x2, y2;	/* start and stop coords for this missile */
	struct _multi_shot *next;
} MultiShot;

D 9

E 9
/* Globals for this file's use only */

int		preparing = FALSE;	/* TRUE when a packet is started */
D 9
Packet		multi_pack;		/* the packet in preparation to send */
E 9
I 9
MultiPack	multi_pack;		/* the packet in preparation to send */
E 9
MultiShot	*multi_list = NULL;	/* the list of missiles for packet */
MultiShot	*multi_end = NULL;	/* last element in the list */

D 9

E 9
/* To send a multi-missile packet:
	- call start_multi_pack with given info
	- call add_to_multi_pack for each missile that you have
	- call send_multi_pack with a valid sender idnum to send it off */


Location *missile_arc();


/* start the making of a multi missile sendoff by getting straight a few
   constants all these missiles share */

D 4
start_multi_pack(int idnum, int roomnum, int special, int type,int hurts)
E 4
I 4
D 7
start_multi_pack(int idnum, int roomnum, int include_start, int type,int hurts)
E 7
I 7
start_multi_pack(idnum, roomnum, include_start, type, hurts)
int idnum, roomnum, include_start, type, hurts;
E 7
E 4
{
  if (preparing) Gerror("already started a multi-pack");

  preparing = TRUE;
D 9
  multi_pack.info.mult.owner = idnum;
  multi_pack.info.mult.type = type;
  multi_pack.info.mult.hurts_owner = hurts;
  multi_pack.info.mult.roomnum = roomnum;
I 4
  multi_pack.info.mult.include_start = include_start;
E 9
I 9
  multi_pack.miss.owner = idnum;
  multi_pack.miss.type = type;
  multi_pack.miss.hurts_owner = hurts;
  multi_pack.miss.room = roomnum;
  multi_pack.include_start = include_start;
E 9
E 4
}

/* add a missile to this planned packet */

D 7
add_to_multi_pack(int x1, int y1, int x2, int y2)
E 7
I 7
add_to_multi_pack(x1, y1, x2, y2)
int x1, y1, x2, y2;
E 7
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

D 7
free_shot_list(MultiShot *list)
E 7
I 7
free_shot_list(list)
MultiShot *list;
E 7
{
  if (list) {
    free_shot_list(list->next);
    free(list);
  }
}

D 9


E 9
/* send the multi pack off to everyone, including myself.  If there are
   too many missiles to fit into one packet, then send several until
   all missiles are sent off */

D 7
send_multi_pack(int senderid)
E 7
I 7
send_multi_pack(senderid)
int senderid;
E 7
{
  register int i;
  MultiShot *ptr;

  /* look for an error condition */
  if (!preparing || !multi_list) Gerror("no start and fill of multipack");

  /* address the packet */
D 9
  multi_pack.type = MULTI_FIRE;  
  address_packet(&multi_pack, senderid, TO_ALL);
E 9

  ptr = multi_list;
  do {
    /* fill the packet */
    for (i=0; (i<MAX_MULTI_MISSILES && ptr); i++) {
D 9
      multi_pack.info.mult.x1[i] = ptr->x1;
      multi_pack.info.mult.y1[i] = ptr->y1;
      multi_pack.info.mult.x2[i] = ptr->x2;
      multi_pack.info.mult.y2[i] = ptr->y2;
E 9
I 9
      multi_pack.x1[i] = ptr->x1;
      multi_pack.y1[i] = ptr->y1;
      multi_pack.x2[i] = ptr->x2;
      multi_pack.y2[i] = ptr->y2;
E 9
      ptr = ptr->next;
    }
D 9
    multi_pack.info.mult.number = i;
E 9
I 9
    multi_pack.number = i;
E 9

D 9
    /* notify myself of the packet by putting it on mail queue,
       then notify others */
D 8
    send_to_id(gameperson[0]->id, &multi_pack);
    send_to_room(multi_pack.info.mult.roomnum, &multi_pack);
E 8
I 8
    send_to_id(gameperson[0]->id, &multi_pack, FALSE);
    send_to_room(multi_pack.info.mult.roomnum, &multi_pack, FALSE);
E 9
I 9
    prepare_and_send_multimissile_packet(&multi_pack);
    add_multi_missiles(&multi_pack);
E 9
E 8
  } while (ptr);

  /* free the list of missiles */
  free_shot_list(multi_list);

  /* reset multi packet building status */
  multi_list = NULL;
  multi_end = NULL;
  preparing = FALSE;
}


/* add all of the missiles in a multi missile packet */

D 9
add_multi_missiles(pack)
Packet *pack;
E 9
I 9
add_multi_missiles(mm)
MultiPack *mm;
E 9
{
  register int i;
I 4
  int length;
E 4
  Missile *new;
  Location *list;
D 9
  Packet dummy;
E 9
I 9
  Missile dummy;
E 9

  /* go through all the missiles in the packet */
D 9
  for (i=0; i<pack->info.mult.number; i++) {
E 9
I 9
  for (i=0; i < mm->number; i++) {
E 9
    /* allocate memory for a new missile record */
    new = (Missile *) malloc(sizeof(Missile));
    if (new == NULL) Gerror("out of memory making missile");
    /* copy information into missile packet */
D 9
    new->owner = pack->info.mult.owner;
D 3
    new->id = NULL;
I 2
    new->special = 0;
E 3
E 2
    new->type = pack->info.mult.type;
E 9
I 9
    new->owner = mm->miss.owner;
    new->type = mm->miss.type;
E 9
D 4
    new->wait = missile_wait(new->type);
    new->lastmove = gametime - new->wait;
E 4
I 4

    /* signal this missile as a 'new' missile */
    new->lastmove = 0;
    new->x = -1;        new->y = -1;
E 4
    new->oldx = -1;	new->oldy = -1;
D 4
    new->x = pack->info.mult.x1[i];
    new->y = pack->info.mult.y1[i];
E 4
I 4

E 4
D 9
    new->hurts_owner = pack->info.mult.hurts_owner;
I 3
    new->heading = MissileHeading(pack->info.mult.x1[i],
				  pack->info.mult.y1[i],
				  pack->info.mult.x2[i],
				  pack->info.mult.y2[i],
				  0);
E 9
I 9
    new->hurts_owner = mm->miss.hurts_owner;
    new->heading =
      MissileHeading(mm->x1[i], mm->y1[i], mm->x2[i], mm->y2[i], 0);
E 9
				  
E 3
    /* make a movement list for the missile */
    new->moveq =
D 9
      diag_movement_list(pack->info.mult.x1[i], pack->info.mult.y1[i],
D 5
			 pack->info.mult.x2[i], pack->info.mult.y2[i]);
E 5
I 5
			 pack->info.mult.x2[i], pack->info.mult.y2[i],
			 TRUE);
E 9
I 9
      diag_movement_list(mm->x1[i], mm->y1[i], mm->x2[i], mm->y2[i], TRUE);
E 9
E 5
I 3
D 4
    /* remove the starting square from the missile que */
    remove_move_head(&new->moveq);
E 4
I 4

    new->wait = missile_wait(new->type);
D 5
			
E 5
I 5
    
E 5
    remove_move_head(&new->moveq); 

E 4
E 3
    /* figure out the directional orientation of this missile */
D 6
    prepare_missile_direction(&dummy, FALSE, new->type,
E 6
I 6
D 9
    prepare_missile_direction(&dummy, new->type,
E 6
			      pack->info.mult.x1[i], pack->info.mult.y1[i],
			      pack->info.mult.x2[i], pack->info.mult.y2[i]);
E 9
I 9
    prepare_missile_direction(&dummy, new->type, mm->x1[i], mm->y1[i],
			      mm->x2[i], mm->y2[i]);
E 9
I 5

E 5
D 9
    new->direction = dummy.info.fire.poop.direction;
E 9
I 9
    new->direction = dummy.direction;
E 9
    /* place this missile at the beginning of the room missile list */
D 5
    new->next = room_missiles[pack->info.mult.roomnum].next;
    room_missiles[pack->info.mult.roomnum].next = new;
E 5
I 5
D 9
    new->room = pack->info.mult.roomnum;
E 9
I 9
    new->room = mm->miss.room;
E 9
    AddMissileToHeap(new);
E 5
  }
I 4

D 9
  if (pack->info.mult.include_start)
    CreateStationaryMissile(pack);
E 9
I 9
  if (mm->include_start) CreateStationaryMissile(mm);
E 9
E 4
}

I 4
/*****************************************************************/

D 7
CreateStationaryMissile(Packet *pack)
E 7
I 7
D 9
CreateStationaryMissile(pack)
Packet *pack;
E 9
I 9
CreateStationaryMissile(mp)
MultiPack *mp;
E 9
E 7
{
  Location *move;
  Missile *new;
  int x,y;

  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory making missile");

D 9
  new->owner = pack->info.mult.owner;
  new->type = pack->info.mult.type;
E 9
I 9
  new->owner = mp->miss.owner;
  new->type = mp->miss.type;
E 9
  new->wait = missile_wait(new->type);
    
  
  new->lastmove = 0;
  new->x = -1;        new->y = -1;
  new->oldx = -1;	new->oldy = -1;

D 9
  new->hurts_owner = pack->info.mult.hurts_owner;
E 9
I 9
  new->hurts_owner = mp->miss.hurts_owner;
E 9
  new->heading = 0;

D 9
  x = pack->info.mult.x1[0];
  y = pack->info.mult.y1[0];
E 9
I 9
  x = mp->x1[0];
  y = mp->y1[0];
E 9
  move = makelocation(x,y,NULL);
  new->moveq = makelocation(x,y,move);

  new->direction = 0;
  /* place this missile at the beginning of the room missile list */
D 5
  new->next = room_missiles[pack->info.mult.roomnum].next;
  room_missiles[pack->info.mult.roomnum].next = new;
E 5
I 5
D 9
  new->room = pack->info.mult.roomnum;
E 9
I 9
  new->room = mp->miss.room;
E 9
  AddMissileToHeap(new);
E 5
}

E 4
/* ==================== A U T O  missile effects ======================== */

/* This routine will randomly make missiles of the given type appear
   all over the given room, owned by the given person, and sent under
   the authority of the given id number.  Distance specifies how many
   squares above the spots to start the firing. */

D 6
random_missile_barrage(pnum, senderid, roomnum, special, type, distance)
int pnum, senderid, roomnum, special, type, distance;
E 6
I 6
random_missile_barrage(pnum, senderid, roomnum, type, distance)
int pnum, senderid, roomnum, type, distance;
E 6
{
  int i, x, y;

D 4
  start_multi_pack(gameperson[pnum]->id, roomnum, special,
E 4
I 4
  start_multi_pack(gameperson[pnum]->id, roomnum, TRUE,
E 4
		   (int) ((unsigned char) info[type]->movingobj),FALSE);

  for (i=0; i<70; i++) {
    x = lrand48() % ROOM_WIDTH;
    y = lrand48() % ROOM_HEIGHT;
    add_to_multi_pack(x, y - distance, x, y);
  }

  send_multi_pack(senderid);
}



/* this one uses the hand combat weapon as a random barrage */

D 7
operation_big_fist(int pnum)
E 7
I 7
operation_big_fist(pnum)
int pnum;
E 7
{
  if (info[0]->weapon) {
    random_missile_barrage(pnum, gameperson[pnum]->id, 
D 6
			   gameperson[pnum]->room, FALSE, 0, 3);
E 6
I 6
			   gameperson[pnum]->room, 0, 3);
E 6
  }
}



/* this one makes an arc of the given missile type occur as a multifire
   missile package.  dir is the direction (in degrees) to fire and
   Dtheta is the amount of swing to each side of the given angle. */

D 4
operation_arc_fire(int ownerid, int senderid, int special, int type, 
E 4
I 4
D 7
operation_arc_fire(int ownerid, int senderid, int include_start, int type, 
E 4
		   int roomnum, int x, int y, int radius, 
D 3
		   int dir, int Dtheta, int hurts)
E 3
I 3
		   int dir, int Dtheta, int hurts, int spread)
E 7
I 7
operation_arc_fire(ownerid, senderid, include_start, type, roomnum, x, y,
		   radius, dir, Dtheta, hurts, spread)
int ownerid, senderid, include_start, type; 
int roomnum, x, y, radius, dir, Dtheta, hurts, spread;
E 7
E 3
{
  int i;
  Location *list, *ptr;

  if (radius < 0) Gerror("negative radius not allowed in operation_arc_fire");

D 4
  start_multi_pack(ownerid, roomnum, special, type, hurts);

E 4
I 4
  start_multi_pack(ownerid, roomnum, include_start, type, hurts);
  
E 4
  /* get a list of the destinations */
D 3
  list = missile_arc(x, y, radius, dir, Dtheta);
E 3
I 3
  list = missile_arc(x, y, radius, dir, Dtheta, 0,spread);
E 3

D 4
  for (ptr=list; ptr; ptr = ptr->next) {
E 4
I 4
  for (ptr=list; ptr; ptr = ptr->next) 
E 4
    add_to_multi_pack(x, y, (int)ptr->x, (int)ptr->y);
D 4
  }
E 4

  free_move_queue(&list);
I 4
  
E 4
  send_multi_pack(senderid);
}



/* cause an explosion at a given square */

D 3
explode_at_square(int senderid, int roomnum,
		  int x, int y, int radius, int type)
E 3
I 3
D 7
explode_at_square(int senderid, int roomnum, int x, int y, int radius,
D 6
		  int type, int arc, int spread,int direction)
E 6
I 6
		  int type, int arc, int spread, int direction)
E 7
I 7
explode_at_square(senderid, roomnum, x, y, radius, type,
		  arc, spread, direction)
int senderid, roomnum, x, y, radius, type, arc, spread, direction;
E 7
E 6
E 3
{
I 3
  if (arc == 0) 
    arc = 180;

E 3
D 4
  operation_arc_fire(senderid, senderid, FALSE, type, roomnum,
E 4
I 4
  operation_arc_fire(senderid, senderid, TRUE, type, roomnum,
E 4
D 3
		     x, y, radius,(lrand48() % 360), 180, TRUE);
E 3
I 3
D 6
		     x, y, radius,direction, arc, TRUE,spread);
E 6
I 6
		     x, y, radius, direction, arc, TRUE, spread);
E 6
E 3
}
E 1
