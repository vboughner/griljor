h09126
s 00000/00000/01197
d D 1.27 92/08/07 01:02:25 vbo 27 26
c source copied to a separate tree for work on new map and object format
e
s 00014/00053/01183
d D 1.26 92/01/20 16:29:51 labc-4lc 26 25
c blojo finished changes necesssary for network packets
e
s 00000/00139/01236
d D 1.25 91/12/16 16:23:24 labc-4lc 25 24
c removed altering object messages and auto-actor stuff
e
s 00146/00095/01229
d D 1.24 91/12/15 22:38:06 labc-4lc 24 23
c Just ripped this one to shreds.   -blojo
e
s 00046/00004/01278
d D 1.23 91/12/08 21:10:22 labc-4lc 23 22
c added messages from objects for attacker
e
s 00050/00010/01232
d D 1.22 91/12/08 18:41:09 labc-4lc 22 21
c added messages for victims of hits
e
s 00001/00001/01241
d D 1.21 91/12/07 21:40:22 labc-4lc 21 20
c split socket.c into three files and change DAMAGE_REPORT to REPORT
e
s 00016/00007/01226
d D 1.20 91/12/07 17:37:29 labc-4lc 20 19
c made error message window bigger and scrollable
e
s 00001/00002/01232
d D 1.19 91/09/27 12:15:37 labc-4lc 19 18
c 
e
s 00001/00001/01233
d D 1.18 91/09/06 22:42:48 labb-3li 18 17
c 
e
s 00003/00003/01231
d D 1.17 91/09/06 00:35:29 labb-3li 17 16
c added reliability flag to all procedure called send_to_...()
e
s 00029/00006/01205
d D 1.16 91/09/03 21:38:58 labb-3li 16 15
c Trevor's misc changes - fixed missile heap
e
s 00008/00004/01203
d D 1.15 91/08/29 01:40:25 vanb 15 14
c fixed up more compatibility problems
e
s 00011/00012/01196
d D 1.14 91/07/07 20:55:47 labc-3id 14 13
c renamed the old property structure references
e
s 00015/00025/01193
d D 1.13 91/06/30 16:57:22 labc-3id 13 12
c removed all traces of old explosion 'special' types
e
s 00004/00007/01214
d D 1.12 91/06/08 14:18:05 labc-3id 12 11
c Fixed freeing of movement queue
e
s 00146/00128/01075
d D 1.11 91/05/26 22:43:04 labc-3id 11 10
c Worked on drawing improvements
e
s 00097/00099/01106
d D 1.10 91/05/20 23:53:29 labc-3id 10 9
c T: fixed all sorts of missle bugs and what not...
e
s 00020/00019/01185
d D 1.9 91/05/18 15:45:02 labc-3id 9 8
c T: working on missiles again... what happens when.  Not done yet.
e
s 00027/00022/01177
d D 1.8 91/05/17 02:04:24 labc-3id 8 7
c Intermediate work on offscreen pixmap
e
s 00004/00003/01195
d D 1.7 91/05/10 04:50:52 labc-3id 7 6
c 
e
s 00003/00000/01195
d D 1.6 91/05/02 16:51:50 labc-3id 6 5
c 
e
s 00118/00000/01077
d D 1.5 91/04/28 23:23:11 labc-3id 5 4
c Van and Mels' auto take stuff
e
s 00010/00007/01067
d D 1.4 91/03/21 17:25:56 labc-3id 4 3
c 
e
s 00023/00007/01051
d D 1.3 91/03/20 23:48:16 labc-3id 3 2
c 
e
s 00115/00067/00943
d D 1.2 91/02/23 01:48:31 labc-3id 2 1
c 
e
s 01010/00000/00000
d D 1.1 91/02/16 12:54:56 labc-3id 1 0
c date and time created 91/02/16 12:54:56 by labc-3id
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

/* Missile handling routines for players and monsters */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "gametime.h"
#include "lib.h"
#include "map.h"
#include "objects.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "movement.h"
D 13
#include "special.h"
E 13
#include "queued.h"
I 22
#include "message.h"
E 22

I 24
/*
 * Used in prepare_missile_course to set the path for a new missile.
 * Having them global may be kinda bad, but hey, it's easy.
 *
 *						-blojo
 */
E 24

I 24
char local_x[MAX_MISSILE_RANGE], local_y[MAX_MISSILE_RANGE];

E 24
/* Global variables for use by anybody */

D 11
Missile		*room_missiles;		/* points to array of missile lists.
					   It is an array of the rooms in the
					   map, each element is a dummy
					   header of a linked list of missiles
					   in that room */
E 11
I 11
Missile		**all_missiles;		/* points to the missile heap */
int  nummissiles,maxmissiles;
E 11

I 13

I 24
#define TRUNC(c) (int)((unsigned char)c)
E 24

E 13
D 11

E 11
/* ========================= F I R I N G =============================== */

D 14
/* Given lots of inforamtion, this procedure will do everything concerned
E 14
I 14
/* Given lots of information, this procedure will do everything concerned
E 14
   with firing a missile, including telling everyone else about it (everyone
   that needs to know that is)  Using movingobject, and not boombit */

D 13
fire_missile(owner, id, roomnum, x1, y1, x2, y2, special, type)
E 13
I 13
fire_missile(owner, id, roomnum, x1, y1, x2, y2, type)
E 13
int	owner;		/* person id who is firing the missile */
InvObj	*id;		/* id pointer of this missile (book-keeping) */
int	roomnum;	/* room it was fired in */
int	x1, y1, x2, y2;	/* start and end coords in room of missile path */
D 13
int	special;	/* TRUE if this is a special pre-defined missile */
E 13
int	type;		/* the type number of the missile */
{
I 2
  int dir,dis;

  if (info[type]->arc == 0)
D 13
    fire_single_missile(owner,id,roomnum,x1,y1,x2,y2,special,type);
E 13
I 13
    fire_single_missile(owner,id,roomnum,x1,y1,x2,y2,type);
E 13
  else
    {
      dir = MissileHeading(x1,y1,x2,y2);
      dis = MissileDistance(x1,y1,x2,y2);
      dir = FanDirection(dir,info[type]->fan);

      if ((dis > info[type]->range)||(! info[type]->stop))
	dis = info[type]->range;
      
      operation_arc_fire(owner,owner,FALSE,(int) info[type]->movingobj,
			 roomnum,x1,y1,dis,dir,
			 (int) info[type]->arc,FALSE,(int) info[type]->spread);
    }
}

/*****************************************************************/

I 24
D 26
/*
 * WEAPON_FIRED
 *
 * CARD16	owner
 * CARD16	type
 * CARD8	hurts
 * CARD16	wait
 *   ^^ (prepare_missile_poop)
 * CARD16	room
 * CARD16	heading
 * CARD16	range
 * Arbitrary8	x[1],x[2],...x[range]
 * Arbitrary8	y[1],y[2],...y[range]
 *   ^^ (prepare_missile_course)
 * CARD8	direction
 *   ^^ (prepare_missile_direction)
 */
E 26

D 26
prepare_missile_packet(m)
Missile *m;
{
  Packet pack;
  PacketInit(&pack, WEAPON_FIRED);

  Add16(&pack, m->owner);
  Add16(&pack, m->type);
  Add8(&pack, m->hurts_owner);
  Add16(&pack, m->wait);

  Add16(&pack, m->room);
  Add16(&pack, m->heading);
  Add16(&pack, m->range);
  AddArbitrary(&pack, local_x, m->range);
  AddArbitrary(&pack, local_y, m->range);

  Add8(&pack, m->direction);
  address_packet(&pack, m->owner, TO_ALL);
  send_to_room(m->room, &pack, FALSE);
}

E 26
E 24
D 13
fire_single_missile(owner, id, roomnum, x1, y1, x2, y2, special, type)
E 13
I 13
fire_single_missile(owner, id, roomnum, x1, y1, x2, y2, type)
E 13
int	owner;		/* person id who is firing the missile */
InvObj	*id;		/* id pointer of this missile (book-keeping) */
int	roomnum;	/* room it was fired in */
int	x1, y1, x2, y2;	/* start and end coords in room of missile path */
D 13
int	special;	/* TRUE if this is a special pre-defined missile */
E 13
int	type;		/* the type number of the missile */
{
E 2
D 24
  Packet misspack;
E 24
I 24
  Missile *m;
  /* allocate memory for a new missile record */
  m = (Missile *) malloc(sizeof(Missile));
  if (m == NULL) Gerror("out of memory noticing missile");
E 24

D 24
  /* place the info about missile inside network packet */
D 2
  prepare_missile_poop(&misspack, owner, id, special, 
E 2
I 2
D 13
  prepare_missile_poop(&misspack, owner, special, 
E 13
I 13
  prepare_missile_poop(&misspack, owner, 
E 13
E 2
		       (int) ((unsigned char) info[type]->movingobj), FALSE);

  /* determine the course the missile should follow and place in packet */
E 24
I 2
  FanMissileCourse(x1,y1,&x2,&x2,(int) info[type]->fan);
E 2
D 24
  prepare_missile_course(&misspack, roomnum, x1, y1, &x2, &y2,
			 info[type]->range,info[type]->stop);
E 24

D 24
  /* figure out the direction orientation */
D 13
  prepare_missile_direction(&misspack, special, 
E 13
I 13
D 14
  prepare_missile_direction(&misspack,(load ".jtemp")

E 14
I 14
  prepare_missile_direction(&misspack,
E 14
E 13
			    (int) ((unsigned char) info[type]->movingobj),
E 24
I 24
  prepare_missile_poop(m, owner, TRUNC(info[type]->movingobj), FALSE);

  /* determine the course the missile should follow */
  prepare_missile_course(m, roomnum, x1, y1, &x2, &y2,
			 info[type]->range, info[type]->stop);

  prepare_missile_direction(m, TRUNC(info[type]->movingobj),
E 24
			    x1, y1, x2, y2);

D 24
  /* prepare the rest of the missile packet and send it */
  misspack.type = WEAPON_FIRED;
  address_packet(&misspack, owner, TO_ALL);
D 17
  send_to_room(roomnum, &misspack);
E 17
I 17
  send_to_room(roomnum, &misspack, FALSE);
E 24
I 24
  /* prepare the missile packet and send it */
D 26
  prepare_missile_packet(m);
E 26
I 26
  prepare_missile_packet(m, local_x, local_y);
E 26
E 24
E 17

  /* tell myself about the missile too */
D 24
  add_missile(&misspack);
E 24
I 24
  add_missile(m);
E 24
}


I 24
/* prepare the poop (information) about a missile */
E 24

D 24

/* prepare the poop (information) about a missile and place it into
   the missile's packet. */

D 2
prepare_missile_poop(pack, owner, id, special, type, hurts)
E 2
I 2
D 13
prepare_missile_poop(pack, owner, special, type, hurts)
E 13
I 13
prepare_missile_poop(pack, owner, type, hurts)
E 13
E 2
Packet *pack;
E 24
I 24
prepare_missile_poop(m, owner, type, hurts)
Missile *m;
E 24
int owner;
D 2
InvObj *id;
E 2
D 13
int special, type, hurts;
E 13
I 13
int type, hurts;
E 13
{
D 24
  /* place a few info items into missile packet */
  pack->info.fire.poop.owner = owner;
D 2
  pack->info.fire.poop.id = (char *) id;
  pack->info.fire.poop.special = special;
E 2
  pack->info.fire.poop.type = type;
  pack->info.fire.poop.hurts_owner = hurts;
E 24
I 24
  m->owner = owner;
  m->type = type;
  m->hurts_owner = hurts;
E 24

D 13
  /* figure out speed for special or regular objects */
E 13
I 13
  /* figure out speed for missile objects */
E 13
D 2
  if (special)
    pack->info.fire.poop.wait = missile_wait(type);
  else
    pack->info.fire.poop.wait = missile_wait(type);
E 2
I 2
D 24
  pack->info.fire.poop.wait = missile_wait(type);
E 24
I 24
  m->wait = missile_wait(type);
E 24
E 2

  /* set a few things to NULL, they will be used later */
D 24
  pack->info.fire.poop.moveq = NULL;
D 11
  pack->info.fire.poop.next = NULL;
E 11
  pack->info.fire.poop.lastmove = 0;
  pack->info.fire.poop.oldx = -1;
  pack->info.fire.poop.oldy = -1;
E 24
I 24
  m->moveq = NULL;
  m->lastmove = 0;
  m->oldx = -1;
  m->oldy = -1;
E 24
}

I 10

/*****************************************************************/

D 26
 
E 26
E 10
/* calculate the wait time for the missile */

missile_wait(type)
int type;
{
  return(((BIGPOWER * wait_clicks_for_speed(info[type]->speed)) /
	  MISSILE_SPEED_FACTOR)/BIGPOWER);
}


/* As a missile leaves a room, initiate a new missile using the old
   missile pack information.  Inform all appropriate players (possibly
   including self) */

refire_missile_in_new_room(m, room)
Missile *m;
int room;
D 24
{  Packet p;
E 24
I 24
{  Missile *new;
E 24
   int range;
   Location *move;
   int i;
I 24
   int newroom;
E 24

I 24
  /* allocate memory for a new missile record */
  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory noticing missile");

E 24
   move = m->moveq;

   /* place room number into missile packet (yes, I mean single =), if
      there is no legal exit, then abort */

D 24
   if ((p.info.fire.room = new_room_for_refire(move, room)) == -1)
E 24
I 24
D 26
   if ((newroom = new_room_for_refire(move, room)) == -1)
E 24
     return; 
E 26
I 26
   if ((newroom = new_room_for_refire(move, room)) == -1) return; 
E 26

I 26
   new->room = newroom;

E 26
   /* translate moveq in appropriate direction, also set range */
   if (move->x < 0) translate_x(ROOM_WIDTH, move, &range);
   else if (move->x >= ROOM_WIDTH) translate_x(-ROOM_WIDTH, move, &range);
   else if (move->y < 0) translate_y(ROOM_HEIGHT, move, &range);
   else if (move->y >= ROOM_HEIGHT) translate_y(-ROOM_HEIGHT, move, &range);
   else /* this proceedure has been called in error */ {
     printf("refire_missile_in_new_room called in error\n");
     return;
   }

D 24
   /* place the info about missile inside network packet */
D 2
   prepare_missile_poop(&p, m->owner, m->id, m->special,
E 2
I 2
D 13
   prepare_missile_poop(&p, m->owner, FALSE,
E 2
			m->type, m->hurts_owner);
E 13
I 13
   prepare_missile_poop(&p, m->owner, m->type, m->hurts_owner);
E 24
I 24
   /* place the info about missile inside structure */
   prepare_missile_poop(new, m->owner, m->type, m->hurts_owner);
E 24
E 13

   /* copy the direction orientation */
D 24
   p.info.fire.poop.direction = m->direction;
E 24
I 24
   new->direction = m->direction;
I 26
   new->heading = m->heading;
E 26
E 24

D 24
  /* place the movement queue into packet */
E 24
I 24
  /* place the movement queue into struct */
E 24
  for (i=0; (move); i++) {
D 24
    p.info.fire.x[i] = move->x;
    p.info.fire.y[i] = move->y;
E 24
I 24
    local_x[i] = move->x;
    local_y[i] = move->y;
E 24
    move = move->next;
  }
D 24
  p.info.fire.range = i;
E 24

I 24
  new->range = i;

E 24
D 26
  /* place current location into missile definition */
E 26
I 26
  /* set current location */
E 26
D 24
  p.info.fire.poop.x = p.info.fire.x[0];
  p.info.fire.poop.y = p.info.fire.y[0];
E 24
I 24
  new->x = local_x[0];
  new->y = local_y[0];
E 24

D 26
  /* prepare the rest of the missile packet and send it */
D 24
  p.type = WEAPON_FIRED;
  address_packet(&p, m->owner, TO_ALL);
D 17
  send_to_room(p.info.fire.room, &p);
E 17
I 17
  send_to_room(p.info.fire.room, &p, FALSE);
E 24
I 24
  prepare_missile_packet(new, TO_ALL, FALSE);
E 26
I 26
  /* prepare the missile packet and send it */
  prepare_missile_packet(new, local_x, local_y);
E 26
E 24
E 17

  /* tell myself about the missile too */
D 24
  add_missile(&p);
E 24
I 24
  add_missile(new);
E 24
}


/* As a missile leaves a room, initiate a new missile using the old
   missile pack information.  Inform all appropriate players (possibly
   including self) This routine is specifically for missiles that have
   encountered exit characters */

refire_missile_through_exit(m, exit_char)
Missile *m;
MemObj *exit_char;
D 24
{  Packet p;
   Location *move;
E 24
I 24
{  Location *move;
E 24
   int i;
I 24
   Missile *new;
   int newroom;
E 24

I 24
  /* allocate memory for a new missile record */
  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory noticing missile");

E 24
  /* place room number into missile packet (yes, I mean single =), if
     there is no legal exit, then abort */

D 24
  if ((p.info.fire.room = exit_char->obj.detail) == -1) return; 
E 24
I 24
  if ((newroom = exit_char->obj.detail) == -1) return; 
E 24

I 24
  new->room = newroom;

E 24
D 12
  /* free old movement queue and make a new one */
  move = m->moveq;
  free_move_queue(&move);
E 12
I 12
  /* make a new movement queue */
E 12
  move = makelocation(exit_char->obj.infox, exit_char->obj.infoy, NULL);

  /* place the info about missile inside network packet */
D 2
  prepare_missile_poop(&p, m->owner, m->id, m->special,
E 2
I 2
D 13
  prepare_missile_poop(&p, m->owner, FALSE,
E 2
		       m->type, m->hurts_owner);
E 13
I 13
D 24
  prepare_missile_poop(&p, m->owner, m->type, m->hurts_owner);
E 24
I 24
  prepare_missile_poop(new, m->owner, m->type, m->hurts_owner);
E 24
E 13

  /* copy the direction orientation */
D 24
  p.info.fire.poop.direction = m->direction;
E 24
I 24
  new->direction = m->direction;
E 24

D 24
  /* place the movement queue into packet */
E 24
I 24
  /* place the movement queue into struct */
E 24
  for (i=0;(move); i++) {
D 24
    p.info.fire.x[i] = move->x;
    p.info.fire.y[i] = move->y;
E 24
I 24
    local_x[i] = move->x;
    local_y[i] = move->y;
E 24
    move = move->next;
  }
D 24
  p.info.fire.range = i;
E 24
I 24
D 26
  new->range = m->range;
E 26
I 26
  new->range = i;
E 26
  new->owner = m->owner;
I 26
  new->heading = m->heading;
E 26
E 24

  /* place current location into missile definition */
D 24
  p.info.fire.poop.x =  p.info.fire.x[0];
  p.info.fire.poop.y =  p.info.fire.y[0];
E 24
I 24
  new->x = local_x[0];
  new->y = local_y[0];
E 24

  /* prepare the rest of the missile packet and send it */
D 24
  p.type = WEAPON_FIRED;
  address_packet(&p, m->owner, TO_ALL);
D 17
  send_to_room(p.info.fire.room, &p);
E 17
I 17
  send_to_room(p.info.fire.room, &p, FALSE);
E 24
I 24
D 26
  prepare_missile_packet(new);
E 26
I 26
  prepare_missile_packet(new, local_x, local_y);
E 26
E 24
E 17

  /* tell myself about the missile too */
D 24
  add_missile(&p);
E 24
I 24
  add_missile(new);
E 24
}


D 24
/* Prepare the course of a missile and place it into missile packet.
   We need to be given the range to extend the missile.   If x2 and y2
   end up being changed, the new values will be returned */
E 24
I 24
/* Prepare the course of a missile.
 * We need to be given the range to extend the missile.
 * If x2 and y2 end up being changed, the new values will be returned.
 */
E 24

D 24
prepare_missile_course(pack, roomnum, x1, y1, x2, y2, range,stops)
Packet *pack;
E 24
I 24
prepare_missile_course(m, roomnum, x1, y1, x2, y2, range,stops)
Missile *m;
E 24
int roomnum, x1, y1, *x2, *y2, range;
int stops;
{
  Location *move, *old;
  register int i;

  /* check that range requested will fit in missile packet */
  if (range > MAX_MISSILE_RANGE) range = MAX_MISSILE_RANGE;

  /* if range requested is less than one then give it at least one */
  if (range < 1) range = 1;

  /* extend the line to the end of the range (by changing x2 and y2)
     but only if this is a missile that shouldn't stop at target */
D 24
  if (!stops)
    extend_range(x1, y1, x2, y2, range);
  
I 2
  pack->info.fire.poop.heading = MissileHeading(x1,y1,*x2,*y2);
E 24
I 24
  if (!stops) extend_range(x1, y1, x2, y2, range);
E 24

I 24
  m->heading = MissileHeading(x1, y1, *x2, *y2);
E 24
E 2
  /* figure out the straight line course */
D 11
  move = diag_movement_list(x1, y1, *x2, *y2);
E 11
I 11
  move = diag_movement_list(x1, y1, *x2, *y2, TRUE);
E 11
I 2
  /* don't need to move the starting square */
E 2
  remove_move_head(&move);

  /* place the movement queue into packet */
  for (i=0; (i<range) && (move); i++) {
    old = move;
D 24
    pack->info.fire.x[i] = move->x;
    pack->info.fire.y[i] = move->y;
E 24
I 24
    local_x[i] = move->x;
    local_y[i] = move->y;
E 24
    move = move->next;
    free(old);
  }
D 24
  pack->info.fire.range = i;
E 24

D 24
  /* place current location into missile definition */
  pack->info.fire.poop.x = pack->info.fire.x[0];
  pack->info.fire.poop.y = pack->info.fire.y[0];
E 24
I 24
  m->range = i;
  m->room = roomnum;
E 24

D 24
  /* place room number into missile packet */
  pack->info.fire.room = roomnum;
E 24
I 24
  /* place current location into missile definition */
  m->x = local_x[0];
  m->y = local_y[0];
E 24
}



/* figure out and place missile direction in packet */

#define motion_amount(a, b)		(((a)>=(b)) ? 0 : ((b)-(a)))
#define first_is_max(a, b, c, d)	((a)>=(b) && (a)>=(c) && (a)>=(d))

D 13
prepare_missile_direction(pack, special, type, x1, y1, x2, y2)
E 13
I 13
D 24
prepare_missile_direction(pack, type, x1, y1, x2, y2)
E 13
Packet *pack;
E 24
I 24
prepare_missile_direction(m, type, x1, y1, x2, y2)
Missile *m;
E 24
D 13
int special, type, x1, y1, x2, y2;
E 13
I 13
int type, x1, y1, x2, y2;
E 13
{
  int up, right, down, left, result;

  /* if this is a non-directional kind of missile then 0 is the default */
D 2
  if (special) {
    if (!smissile[type]->directional) {
      pack->info.fire.poop.direction = 0;
      return;
    }
E 2
I 2
  if (!(info[type]->directional)) {
D 24
    pack->info.fire.poop.direction = 0;
E 24
I 24
    m->direction = 0;
E 24
    return;
E 2
  }
D 2
  else {
    if (!(info[type]->directional)) {
      pack->info.fire.poop.direction = 0;
      return;
    }
  }
E 2

  /* figure out the amount we are going in each direction */  
  up = motion_amount(y2, y1);
  right = motion_amount(x1, x2);
  down = motion_amount(y1, y2);
  left = motion_amount(x2, x1);

  /* the result is the maximum of all of those */
  if (first_is_max(up, right, down, left)) result = 0;
  else if (first_is_max(right, down, left, up)) result = 1;
  else if (first_is_max(down, left, up, right)) result = 2;
  else if (first_is_max(left, up, right, down)) result = 3;

  /* place result into directional flag variable */
D 24
  pack->info.fire.poop.direction = result;
E 24
I 24
  m->direction = result;
E 24
}



/* given missile starting and destination points, extend the destination
   x and y according to the full range of the missile, change the destination
   x and y and return */

extend_range(x1, y1, x2, y2, range)
int x1, y1, *x2, *y2, range;
{
  double initial_dist, new_x, new_y, fraction;

  /* figure out the initially requested range */
  initial_dist =
    sqrt((double) ((*x2 - x1) * (*x2 - x1) + (*y2 - y1) * (*y2 - y1)));

  /* if the initial range is already large enough then leave */
  if (initial_dist > ((double) range) - 0.5) return;

  /* figure out what fraction of the range is complete so far */
  fraction = (initial_dist) / ((double) range);

  /* figure out new x and y by extending them using the fraction */
  new_x = ((double) *x2) + ((double) (*x2 - x1)) / fraction;
  new_y = ((double) *y2) + ((double) (*y2 - y1)) / fraction;

  /* assign the newly computed results to x2 and y2 as the answer */
  *x2 = (int) (new_x + 0.5);
  *y2 = (int) (new_y + 0.5);
}


D 13

/* return TRUE if this kind of missile stops at target point */

int missile_stops(special, type)
int special, type;
{
D 2
  if (special)
    return smissile[type]->stop;
  else
    return info[type]->stop;
E 2
I 2
  return info[type]->stop;
E 2
}
E 13

I 2
FanMissileCourse(x1,y1,x2,y2,fan)
int x1,y1,*x2,*y2,fan;
{
  double dis;
  int dir;
E 2

I 2
  if (fan == 0)
    return;
E 2

I 2
  dis = MissileDistance(x1,y1,*x2,*y2) + 0.5;
  dir = MissileHeading(x1,y1,*x2,*y2);
  dir = FanDirection(dir,fan);
  *x2 = x1 + floor(dis*cos( (double) dir * M_PI / 180.0 )+ 0.5);
  *y2 = y1 + floor(dis*sin( (double) dir * M_PI / 180.0 )+ 0.5);
}

MissileDistance(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
  double dis;
  x1 = x1-x2;
  y1 = y1-y2;
  dis = sqrt((double) x1*x1 + (double) y1*y1);
  return((int) dis);
}

MissileHeading(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
  double dx,dy,dir;
  int heading;

  dx = x2-x1;
  dy = y2-y1;

  if ((dx == 0.0)&&(dy == 0.0))
    return(0);

  dir = atan2(dy,dx);
  heading = dir * 180.0 / M_PI;
  return(heading);
}

FanDirection(dir,fan)
int dir,fan;
{
  if (fan)
    dir += (lrand48() % (fan*10))/5 - fan;
  
  return(dir);
}

E 2
/* =========================== N O T I C I N G =========================== */

/* given a missile packet, either received or one we created ourselves,
   add the missile to one of our room missile lists, if we have any
   of our persons in the missile's room */

D 24
add_missile(pack)
Packet *pack;
E 24
I 24
D 26
add_missile(new, range, coursex, coursey)
E 26
I 26
add_missile(new)
E 26
Missile *new;
D 26
int range;
char coursex[], coursey[];
E 26
E 24
{
  register int i;
D 24
  Missile *new;
E 24
  Location *this, *last;

D 24
  /* allocate memory for a new missile record */
  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory noticing missile");

  /* copy missile from packet into new memory area */
  bcopy(&(pack->info.fire.poop), new, sizeof(Missile));

E 24
  /* create movement list for the new missile */
D 24
  for (i=0,last=NULL; (i < pack->info.fire.range); i++) {
    this = makelocation(pack->info.fire.x[i], pack->info.fire.y[i], NULL);
E 24
I 24
D 26
  for (i = 0, last = NULL; i < range; i++) {
    this = makelocation(coursex[i], coursey[i], NULL);
E 26
I 26
  for (i = 0, last = NULL; i < new->range; i++) {
    this = makelocation(local_x[i], local_y[i], NULL);
E 26
E 24
    if (last) last->next = this;
    else new->moveq = this;
    last = this;
  }

D 10
  /* set the gametime marker for this missile */
  new->lastmove = gametime - new->wait;
E 10
I 10
  /* mark this missile as a >new< missile.  It's position will 
     be taken from the first entry on the movement que */
  new->lastmove = 0;
  new->x = -1;  new->y = -1;
  new->oldx = -1; new->oldy = -1;
E 10

  /* place missile in the correct room list, at the beginning */
D 11
  new->next = room_missiles[pack->info.fire.room].next;
  room_missiles[pack->info.fire.room].next = new;
E 11
I 11
D 24
  new->room = pack->info.fire.room;
E 24
  AddMissileToHeap(new);
E 11
}





/* =========================== M O V I N G =============================== */

D 18
/* initialize the missile array now that we now how many rooms there are */
E 18
I 18
/* initialize the missile heap array */
E 18

initialize_missile_array()
{
D 11
  register int i;
  
  /* allocate space in memory for this stuff */
  room_missiles = (Missile *) malloc(sizeof(Missile) * mapstats.rooms);
  if (room_missiles == NULL) Gerror("no memory left for missile array");
E 11
I 11
  nummissiles = 0;
  maxmissiles = 10000;
E 11

D 11
  /* clear all lists dummy headers to point to NULL lists */
  for (i=0; i<mapstats.rooms; i++) room_missiles[i].next = NULL;
E 11
I 11
  all_missiles = (Missile **) malloc(sizeof(Missile) * maxmissiles);

  if (all_missiles == NULL)
    Gerror("No memory for missiles");
E 11
}


free_missile_array()
{
D 11
  free(room_missiles);
  room_missiles = NULL;
E 11
I 11
  free(all_missiles);
E 11
}

I 10
/*****************************************************************/
E 10

I 11
D 15
htime(int i)
E 15
I 15
htime(i)
int i;
E 15
{
  Missile *m;

  m = all_missiles[i];
  if (m == NULL)
    return(0);
I 16
  
E 16
  return(m->lastmove + m->wait);
}

Missile *TopOfHeap()
{
  if (nummissiles)
    return(all_missiles[0]);
  else
    return(NULL);
}

D 15
void AddMissileToHeap(Missile *add)
E 15
I 15
void AddMissileToHeap(add)
Missile *add;
E 15
{
  all_missiles[nummissiles] = add;
  BubbleUp(nummissiles);
  nummissiles++;
  if (nummissiles >= maxmissiles)
    Gerror("Too many missiles!");
}

void RemoveTopOfHeap()
{
  if (nummissiles == 0)
    return;
  
  nummissiles--;

  all_missiles[0] = all_missiles[nummissiles];
  all_missiles[nummissiles] = NULL;
  
  BubbleDown(0);
}

D 15
BubbleUp(int k)
E 15
I 15
BubbleUp(k)
int k;
E 15
{
  int j,t;
  Missile *m;
  
  m = all_missiles[k];
  t = htime(k);

  while(k > 0)
    {
      j = (k-1)/2;
      
      if (htime(j) <= t)
	break;

      all_missiles[k] = all_missiles[j];
      k = j;
    }

  all_missiles[k] = m;
}

D 15
BubbleDown(int k)
E 15
I 15
BubbleDown(k)
int k;
E 15
{
  int j,t;
  Missile *m;

  m = all_missiles[k];
  j = k*2 + 1;
  t = htime(k);

  while (j < nummissiles)
    {
D 16
      if (htime(j) >= t)
E 16
I 16
      if (((j+1) < nummissiles)&&(htime(j+1) < htime(j)))
E 16
	j++;
D 16

      if (j >= nummissiles)
	break;

E 16
I 16
      
E 16
      if (htime(j) >= t)
	break;
      
      all_missiles[k] = all_missiles[j];
      k = j;
      j = k*2 + 1;
    }
  
  all_missiles[k] = m;
}

/*****************************************************************/
I 16
/* used to check the consistancy of the missile heap - for debugging
  only.  Call with CheckHeap(0) */
  
CheckHeap(w)
int w;
{
  int j;
  
  j = w*2+1;
  if (j >= nummissiles)
    return;
  if (htime(j) < htime(w))
    printf("Bad heap\n");
  CheckHeap(j);
  j++;
  if (j >= nummissiles)
    return;
  if (htime(j) < htime(w))
    printf("Bad heap\n");
  CheckHeap(j);
}
E 16

I 16
/*****************************************************************/

E 16
E 11
D 10

E 10
/* Check for the need to move any of the missiles we know about.  If any of
   the missiles we check want to die, then delete them from their room list. */

update_missiles()
{
D 11
  register int i;
D 10
  Missile *current,*prevous;
  int missle_end, refire = FALSE;
E 10
I 10
  Missile *current,*previous,*next;
E 11
I 11
  Missile *current;
E 11
  int missle_end, refire,count;
E 10

D 11
  /* check each room for missiles */
  for (i=0; i<mapstats.rooms; i++)
    if (room_missiles[i].next != NULL) 
      {
D 10
	prevous = &(room_missiles[i]);
	current = prevous->next;
	for (; current;)
E 10
I 10
	previous = &(room_missiles[i]);
	
	for (current = previous->next; current;previous=current,current=next)
E 10
	  {
I 10
	    next = current->next;
	    
E 10
	    /* check that the time is right for this missile to move */
D 10
	    if (gametime < current->lastmove + current->wait)
E 10
I 10
	    if (gametime >= current->lastmove + current->wait)
E 10
	      {
D 10
		prevous = current;
		current = current->next;
	      }
	    else
	      {
	      /* handle the individual missile here */
E 10
I 10
		refire = FALSE;
E 11
I 11
  current = TopOfHeap();
  
  while((current) && (gametime >= current->lastmove + current->wait))
    {
      RemoveTopOfHeap();
E 11

D 11
		/* handle the individual missile here */
E 10
		missle_end = update_individual_missile(current, i, &refire);
E 11
I 11
      refire = FALSE;
      
      /* handle the individual missile here */
      missle_end = update_individual_missile(current, &refire);
      
      /* remove the missile if it is at it's end */
      if (!missle_end) 
	AddMissileToHeap(current);
      else
	{
I 12
	  /* remove the rest of this missiles movement que */
	  free_move_queue(&(current->moveq));

E 12
	  /* refire will only be true when missile is 
	     going on to another room and we musn't explode 
	     or drop it yet */
	  
	  if ((one_of_me(current->owner) && (!refire)))
	    drop_or_explode_it(current);
D 12
	  
	  /* remove the rest of this missiles movement que */
	  while(current->moveq)
	    remove_move_head(&(current->moveq));
E 12
	  
	  free(current);
	}
E 11

D 10
	      /* remove the missile if it is at it's end */
	      if (missle_end) 
		{
		  if ((one_of_me(current->owner) && (!refire)))
		    {
		      /* refire will only be true when missile is 
			 going on to another room and we musn't explode 
			 or drop it yet */
		      
D 2
		      try_to_explode_it(current, i);
		      missile_dropped(current, i);
E 2
I 2
		      drop_or_explode_it(current, i);
E 2
		    }
E 10
I 10
D 11
		/* remove the missile if it is at it's end */
		if (missle_end) 
		  {
		    /* refire will only be true when missile is 
		       going on to another room and we musn't explode 
		       or drop it yet */
E 10

I 8
D 10
		  /* remove the movement que */
		  /* needs to be done */

E 8
		  /* remove this object from the linked list */
		  current = current->next;

D 2

		  /* if this was my missile,
		     then I can free it and object copy */
		  if (one_of_me(prevous->next->owner)) 
		    {
		      if (prevous->next->id && !refire)
			free_inv_object((InvObj *)prevous->next->id);
		      free(prevous->next);
		    }

E 2
I 2
		  free(prevous->next);
E 2
		  prevous->next = current;
		}
	      else  /* not the end of the missiles life */
		{
		  prevous = current;
		  current = current->next;
		}
	    }
	}
E 10
I 10
		    if ((one_of_me(current->owner) && (!refire)))
		      drop_or_explode_it(current, i);
		    
		    /* remove the rest of this missiles movement que */
		    while(current->moveq)
		      remove_move_head(&(current->moveq));
		    
		    free(current);
		    previous->next = next;
		    current = previous;
		  }
	      }
	  }
E 10
      }
E 11
I 11
      current = TopOfHeap();
    }
E 11
}


D 10

E 10
/* update an individual missile's position, if it is time to do so.  Return
   TRUE if this missile is now finished and needs to either be deleted,
   cause an explosion or be placed in the room next to dying location.
   Which of the above occurs depends on the missile's type.
   Set keep_ptr to TRUE if we should not free the object copy.  Do
   nothing to it if we don't care what caller does with object copy. */
  
D 11
update_individual_missile(m, roomnum, refire)
E 11
I 11
update_individual_missile(m, refire)
E 11
Missile *m;
D 11
int roomnum, *refire;
E 11
I 11
int *refire;
E 11
{
  Location *move;
D 10
  int lucky_person,can_continue = TRUE;
E 10
I 10
D 11
  int lucky_person;
E 11
I 11
  int lucky_person,roomnum;
E 11
  int dont_let_it_get_away,first_shot;
E 10
  MemObj *exit_char = NULL;
I 10
  
  first_shot = (m->lastmove == 0);
E 10
D 9
  
  /* erase the missile, because it does not exist any more */
D 2
  if (has_display)
    if (!another_erasure_occurred(m, room_missiles[roomnum].next))
      QDraw_priority_request(roomnum, m->x,m->y, CONCEALED);
E 2
I 2
  if (m->oldx != -1)
    if (has_display)
D 8
      if (!another_erasure_occurred(m, room_missiles[roomnum].next))
D 7
	QDraw_priority_request(roomnum, m->x,m->y, CONCEALED);
E 7
I 7
	QErase_missile(m,roomnum);
E 8
I 8
      if (!another_erasure_occurred(m, room_missiles[roomnum].next)) 
	QDraw_priority_request(roomnum,m->x,m->y,EVERYTHING); 
E 9
E 8
E 7
E 2

I 11
  roomnum = m->room;

E 11
I 8
D 9
  /* if we own this missile and it destroys walls then try to do so */
  if (one_of_me(m->owner) && info[m->type]->destroys)
    destroy_square(roomnum, m->x, m->y, m);
E 9
I 9
D 10
  if (m->oldx != -1) /* only do this if the missile actually exists */
    {
      if (has_display)
	if (!another_erasure_occurred(m, room_missiles[roomnum].next)) 
	  QDraw_priority_request(roomnum,m->x,m->y,EVERYTHING); 
E 9

D 9
  /* check for missile hitting a person (other than its owner) */
  lucky_person = person_on_square(roomnum, m->x, m->y);
  if (lucky_person != -1  &&
      (lucky_person != person_array(m->owner) || m->hurts_owner)) {
    /* register a hit in the hit array */
    add_new_hit(roomnum, m->x, m->y);
    /* check if it is one of my persons that is hit */
    if (lucky_person < num_persons) take_missile_damage(lucky_person, m);
    return TRUE;
  }
E 9
I 9
      /* if we own this missile and it destroys walls then try to do so */
      if (one_of_me(m->owner) && info[m->type]->destroys)
	destroy_square(roomnum, m->x, m->y, m);

      /* check for missile hitting a person (other than its owner) */
      lucky_person = person_on_square(roomnum, m->x, m->y);
      if (lucky_person != -1  &&
	  (lucky_person != person_array(m->owner) || m->hurts_owner)) {
	/* register a hit in the hit array */
	add_new_hit(roomnum, m->x, m->y);
	/* check if it is one of my persons that is hit */
	if (lucky_person < num_persons) take_missile_damage(lucky_person, m);
	return TRUE;
      }
    }
E 9

E 10
I 10
  if (!first_shot)
    if (has_display)
D 11
      if (!another_erasure_occurred(m, room_missiles[roomnum].next)) 
	QDraw_priority_request(roomnum,m->x,m->y,EVERYTHING); 
E 11
I 11
      QDraw_priority_request(roomnum,m->x,m->y,EVERYTHING); 
E 11
  
E 10
E 8
  /* let's check if there is any more movement queue */
  if (m->moveq == NULL) return TRUE;	/* missile is finished */
I 8
  move = m->moveq;
E 8

D 10
  /* time and movement check out, check for destination out of bounds */
E 10
I 10
  /* check for destination out of bounds */
E 10
D 8
  move = m->moveq;
E 8
  if (move->x < 0 || move->x >= ROOM_WIDTH || 
      move->y < 0 || move->y >= ROOM_HEIGHT)
    {
      /* call leaving room routine (if it's mine) */
      if (one_of_me(m->owner))
D 10
	{
	  refire_missile_in_new_room(m, roomnum);
	  *refire = TRUE;
	}
E 10
I 10
	refire_missile_in_new_room(m, roomnum);
      
      *refire = TRUE;
E 10
      return TRUE;
    }
D 10

E 10
I 10
  
E 10
  /* check for missile hitting a wall or some other non-giving object,
D 10
     but not when missile is first starting out, end it now, before it
     gets on the square */
E 10
I 10
     before it moves onto that square */
E 10

D 10
  if (m->oldx != -1)
E 10
I 10
  if (!first_shot)
E 10
    if (!can_fire_onto(roomnum,m->type,move->x,move->y))
      return TRUE;
  
  /* everything checks out, let's move the rotten thing */
  m->oldx = m->x;	m->oldy = m->y;
D 10
  m->x = move->x;	m->y = move->y;		m->lastmove += m->wait;
E 10
I 10
  m->x = move->x;	m->y = move->y;		

  if (first_shot)
    m->lastmove = gametime;
  else
    m->lastmove += m->wait;

E 10
  remove_move_head(&(m->moveq));

D 10
  /* draw the missile, even if it is about to die, so it 'flickers' */
  
  if (has_display)
D 8
    if (!another_missile_drawn(m,room_missiles[roomnum].next))
D 7
      redraw_missile(m,roomnum);
E 7
I 7
      QDraw_missile(m,roomnum);
E 8
I 8
    if (!another_missile_drawn(m,room_missiles[roomnum].next)) 
      redraw_missile(m,roomnum);
E 8
E 7
  
  /* check if missile is going to try to leave a square that it cant leave.
E 10
I 10
  /* check if missile is going to try to leave a square that it can't leave.
E 10
     this is done here beacause the square may be changed, i.e. destroyed,
     allowing the object through when it should not be. */

D 10
  if (!can_fire_through(roomnum,m->type, m->x, m->y))
    can_continue = FALSE;
E 10
I 10
  dont_let_it_get_away = ! can_fire_through(roomnum,m->type, m->x, m->y);
E 10

I 10
  /* if we own this missile and it destroys walls then try to do so */
  if (one_of_me(m->owner) && info[m->type]->destroys)
    destroy_square(roomnum, m->x, m->y, m); 

  /* check for missile hitting a person (other than its owner) */
  
  lucky_person = person_on_square(roomnum, m->x, m->y);
D 11
  if (lucky_person != -1  && (lucky_person != person_array(m->owner) || m->hurts_owner)) 
E 11
I 11
  if (lucky_person != -1  &&
      (lucky_person != person_array(m->owner) || m->hurts_owner)) 
E 11
    {
      /* register a hit in the hit array */
      add_new_hit(roomnum, m->x, m->y);
      /* check if it is one of my persons that is hit */
      if (lucky_person < num_persons) take_missile_damage(lucky_person, m);
      return TRUE;
    }


E 10
D 8
  /* check for missile hitting a person (other than its owner) */
  lucky_person = person_on_square(roomnum, move->x, move->y);
  if (lucky_person != -1  &&
      (lucky_person != person_array(m->owner) || m->hurts_owner)) {
    /* register a hit in the hit array */
    add_new_hit(roomnum, move->x, move->y);
    /* check if it is one of my persons that is hit */
    if (lucky_person < num_persons) take_missile_damage(lucky_person, m);
    can_continue = FALSE;
  }

E 8
  /* check for a missile hitting an exit square, it should go one space
     through the exit */
D 10
  exit_char = exit_char_on_square(roomnum, m->x, m->y);
  if (exit_char) {
    if (one_of_me(m->owner)) {
      refire_missile_through_exit(m, exit_char);
      *refire = TRUE;
    }
    can_continue = FALSE;
  }
E 10

D 8
  /* if we own this missile and it destroys walls then try to do so */
  if (one_of_me(m->owner) && info[m->type]->destroys)
D 3
    destroy_square(roomnum, move->x, move->y);
E 3
I 3
    destroy_square(roomnum, move->x, move->y, m);
E 3

E 8
D 10
  /* somewhere above, we sait that the missile is not allowed to continue */
  if (!can_continue)
E 10
I 10
  exit_char = exit_char_on_square(roomnum, m->x, m->y);
  if (exit_char) 
E 10
    {
D 10
      /* erase the dead missile */
      if (has_display)
	if (!another_erasure_occurred(m,room_missiles[roomnum].next))
D 7
	  QDraw_priority_request(roomnum, m->x,m->y, CONCEALED);
E 7
I 7
D 8
	  QErase_missile(m,roomnum);
E 8
I 8
	  QDraw_priority_request(roomnum,m->x,m->y,EVERYTHING);
E 10
I 10
      if (one_of_me(m->owner)) 
	refire_missile_through_exit(m, exit_char);
      *refire = TRUE;
E 10
E 8
E 7
      return TRUE;
    }

I 10
  if (dont_let_it_get_away)
    return TRUE;
E 10

I 10
  /* draw the missile */
  
  if (has_display)
D 11
    if (!another_missile_drawn(m,room_missiles[roomnum].next)) 
E 11
      redraw_missile(m,roomnum);

E 10
  return FALSE;
}
I 10
  
E 10

I 8
/*****************************************************************/
E 8


/* find out what person is on a given square, return their array element
   number, or -1 if no person was found */

int person_on_square(roomnum, x, y)
int roomnum, x, y;
{
  PersonList	*ptr;
  int		collide = -1;

  /* look through room for people on the square we want (not dead people) */
  for (ptr = room_persons[roomnum].next;
       (ptr && collide == -1); ptr = ptr->next)
	  if (ptr->person->x == x && ptr->person->y == y &&
	      ptr->person->appearance != APPEAR_DYING)
		collide = ptr->person->id;

  return person_array(collide);
}



I 22
own_player_damage_message(pnum, missile)
/* send ourselves the message about getting damage from a missile */
int pnum;
Missile *missile;
{
  char msg[MSG_LENGTH];
  char *fire_name = NULL, *fireteam_name = NULL;
  int firer = person_array(missile->owner);
  int strength = missile_damage(missile);
E 22

I 22
  if ((firer >= 0) && (gameperson[firer]->listed)) {
    fire_name = gameperson[firer]->name;
    fireteam_name = mapstats.team_name[gameperson[firer]->team];
  }

  if (info[missile->type]->victimmsg) {
    char *newmsg;
    MessageInfo *msginfo = create_message_info();
    MI_DAMAGE_INFO(msginfo, info[missile->type]->name, fire_name,
		   missile->owner, fireteam_name, gameperson[pnum]->name,
		   gameperson[pnum]->id,
		   mapstats.team_name[gameperson[pnum]->team],
		   room[gameperson[pnum]->room].name, strength);
    newmsg = create_finished_message(info[missile->type]->victimmsg,
				     msginfo, PLAYER_MESSAGE_COLUMNS);
    player_error_out_message(newmsg);
    destroy_message(newmsg);
    destroy_message_info(msginfo);
  }
  else {
    /* no special victim message provided */
    if (fire_name) {
      if (info[missile->type]->name) 
        sprintf(msg, "You suffered %d point hit with %s by %s (%d)", strength,
	        info[missile->type]->name, fire_name, missile->owner);
      else
        sprintf(msg, "You suffered %d point hit from %s (%d)", strength,
	        fire_name, missile->owner);
    }
    else {
      sprintf(msg, "You suffered %d point hit", strength);
    }
    player_error_out_message(msg);
  }
}



I 23
send_missile_owner_message(pnum, missile)
/* send a message to the person that hit or 'pnum' person */
int pnum;
Missile *missile;
{
  char msg[MSG_LENGTH];
  char *fire_name = NULL, *fireteam_name = NULL;
  int strength = missile_damage(missile);
  int firer = person_array(missile->owner);
  if (missile->owner < 0) return;

  if ((firer >= 0) && (gameperson[firer]->listed)) {
    fire_name = gameperson[firer]->name;
    fireteam_name = mapstats.team_name[gameperson[firer]->team];
  }

  if (info[missile->type]->hitmsg) {
    char *newmsg;
    MessageInfo *msginfo = create_message_info();
    MI_DAMAGE_INFO(msginfo, info[missile->type]->name, fire_name,
		   missile->owner, fireteam_name, gameperson[pnum]->name,
		   gameperson[pnum]->id,
		   mapstats.team_name[gameperson[pnum]->team],
		   room[gameperson[pnum]->room].name, strength);
    newmsg = create_finished_message(info[missile->type]->hitmsg,
				     msginfo, PLAYER_MESSAGE_COLUMNS);
    notify_of_report(gameperson[pnum]->id, missile->owner, newmsg);
    destroy_message(newmsg);
    destroy_message_info(msginfo);
  }
  else {
    /* no special message provided */
    if (info[missile->type]->name)
      sprintf(msg, "You hit %s (%d) with %s for %d points damage",
	      gameperson[pnum]->name, gameperson[pnum]->id,
	      info[missile->type]->name, strength);
    else
      sprintf(msg, "You hit %s (%d) for %d points damage",
	      gameperson[pnum]->name, gameperson[pnum]->id, strength);
    notify_of_report(gameperson[pnum]->id, missile->owner, msg);
  }
}



E 23
E 22
/* one of our guys is hit, take the damage if we have to, in any case,
   notify the owner of the missile that we've handled it  */

take_missile_damage(pnum, missile)
int pnum;
Missile *missile;
{
D 23
  char msg[MSG_LENGTH];
E 23
D 20
  int  strength;
E 20
I 20
D 22
  int  firer, strength;
E 22
I 22
  int strength;
E 22
E 20

  /* nevermind this hit if he is already dead */
  if (person[pnum]->health <= 0) return;

  /* figure out strength of missile */
  strength = missile_damage(missile);
  if (!strength) return;	/* no damage done */
  
  /* UNFINISHED: do checks to see whether we should be vulnerable */

  /* take the damage from health points */
  person[pnum]->health -= strength;

  /* update health point display if we have a display */
D 20
  if (has_display && pnum==0) {	/* current player is zero, MAY CHANGE */
E 20
I 20
  if (has_display && pnum==0) {
E 20
    redraw_player_health();
D 20
    sprintf(msg, "You suffered %d point hit", strength);
    redraw_error_window(msg);
E 20
I 20
D 22
    if ( ((firer = (person_array(missile->owner))) >= 0) &&
	 (gameperson[firer]->listed) ) {
      sprintf(msg, "You suffered %d point hit by %s (%d)", strength,
	      gameperson[firer]->name, missile->owner);
    }
    else {
      sprintf(msg, "You suffered %d point hit", strength);
    }
    player_error_out_message(msg);
E 22
I 22
    own_player_damage_message(pnum, missile);
E 22
E 20
  }

  /* send a damage report to the owner of the missile */
D 20
  sprintf(msg, "You hit %s (%d) for %d points damage", gameperson[pnum]->name,
	  gameperson[pnum]->id, strength);
  notify_of_damage(gameperson[pnum]->id, missile->owner, msg);
E 20
I 20
  if (person_array(missile->owner) >= num_persons) {
D 23
    sprintf(msg, "You hit %s (%d) for %d points damage",
	    gameperson[pnum]->name, gameperson[pnum]->id, strength);
D 21
    notify_of_damage(gameperson[pnum]->id, missile->owner, msg);
E 21
I 21
    notify_of_report(gameperson[pnum]->id, missile->owner, msg);
E 23
I 23
    send_missile_owner_message(pnum, missile);
E 23
E 21
  }
E 20
I 16
  
  AddtoDamageRecord(pnum,strength,person_array(missile->owner));
E 16

  /* if person is now dead then send death message to all */
  if_did_him_in(pnum, person_array(missile->owner));
}



/* figure out how much damage given missile causes */

int missile_damage(missile)
Missile *missile;
{
  return info[missile->type]->damage;
}



/* if this person is now dead, then send death message to all blaming the
   given killer */

if_did_him_in(pnum, knum)
int pnum, knum;		/* person array indexes */
{
  if (person[pnum]->health <= 0 && knum >=0) {
    char msg[MSG_LENGTH];
    sprintf(msg, "%s->ALL %s (%d) was killed by %s (%d)", GM_NAME,
	    gameperson[pnum]->name, gameperson[pnum]->id,
	    gameperson[knum]->name, gameperson[knum]->id);
    notify_of_message(FROM_GM, "all", msg);
    add_loss(pnum);
    notify_of_kill(gameperson[pnum]->id, gameperson[knum]->id);

    /* if we have a display, then we should know about it too */
    if (has_display) player_out_message(msg);

    /* see that experience points are rewarded appropriately */
D 16
    death_experience(pnum, knum);
E 16
I 16
    death_experience(pnum);
E 16
  }
}




/* missile's life is at an end, check to see if it should be deposited on
   floor nearby */

missile_dropped(m, roomnum)
Missile *m;
int roomnum;
{
D 2
  InvObj *obj;
E 2
I 2
  /* drop object only if does not explode, and the boombit is not 0 */
E 2

D 2
  /* special missiles can never be dropped */
  if (m->special) return;

  /* get a pointer to the missile's real object definition */
  obj = (InvObj *) m->id;
  
  /* drop object only if it's "thrown" flag is TRUE, and there is an object */
  if (obj)
    if (info[(uc)obj->type]->thrown)
      drop_object_near_square(obj, roomnum, m->x, m->y);
E 2
I 2
  if (info[m->type]->explodes == 0)
    if (info[m->type]->boombit)
      drop_type_near_square((int) info[m->type]->boombit,roomnum,m->x,m->y);
E 2
}



/* a missile has reached the end of it's path, check to see if it explodes */

D 2
try_to_explode_it(m, roomnum)
E 2
I 2
D 11
drop_or_explode_it(m, roomnum)
E 11
I 11
drop_or_explode_it(m)
E 11
E 2
Missile *m;
D 11
int roomnum;
E 11
{
  int explodes, explosion,radius;
I 11
  int roomnum;
E 11

I 11
  roomnum = m->room;
E 11
  explodes = info[m->type]->explodes;
D 2
  explosion = info[m->type]->boombit;
E 2

D 2
  if (explodes > 0) {
    radius = explodes - 1;
    if ((m->x < 0 || m->x >= ROOM_WIDTH || m->y < 0 || m->y >= ROOM_HEIGHT)
	&& m->moveq)
      {
	printf("The case that won't happen happened, try_to_explode\n");
/*      explode_at_square(m->owner, roomnum, m->moveq->x, m->moveq->y,
			radius, explosion, m->parent);
*/
      }
    else
E 2
I 2
  if (explodes == 0)
    missile_dropped(m, roomnum);
  else
    {
      explosion = info[m->type]->boombit;
      radius = explodes - 1;
E 2
      explode_at_square(m->owner, roomnum, m->x, 
D 2
			m->y, radius,explosion);
  }
E 2
I 2
			m->y, radius,explosion,
			(int) info[m->type]->arc,
			(int) info[m->type]->spread,
			FanDirection(m->heading,(int) info[m->type]->fan));
    }
E 2
}


/* ========================== M I S C ================================ */

/* fix the movement queue by adding x to each x-coord.  Also count the number
   of locations on the queue, store this in *r. */

translate_x(x, m, r)
int x;
Location *m;
int *r;

{
  for(*r = 0; m; (*r)++) {
    m->x += x;
    m = m->next;
  }
}

    
/* just like t_x, but y */   

translate_y(y, m, r)
int y;
Location *m;
int *r;

{
  for(*r = 0; m; (*r)++) {
    m->y += y;
    m = m->next;
  }
}



/* figure out, given the object type, how many clicks person must wait
   before he should be allowed to fire again.  If object was defined
   for refire speeds -5 then the wait will be twice as long as usual.
   Refire speed +5 will mean no wait at all. */

int firing_wait(type)
int type;
{
  int x, modifier;

  /* figure out what modification to normal wait the object is defined for */
  x = (info[type]->refire < -5) ? -5 : info[type]->refire;
  x = (x > 5) ? 5 : x;
  modifier = ((double) (-x) / (double) 5.0 *
	      (double) FIRING_WAIT_CLICKS);

  return (FIRING_WAIT_CLICKS + modifier);
}


/* return TRUE if this type of missile can go through this square */

int can_fire_through(roomnum, type, x, y)
int roomnum, x, y, type;
{
  unsigned char *whats_here;
  int           i;
  int		permeable = TRUE;
  
  /* if out of bounds then sure, we can fire there */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return TRUE;

  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for non-permeable or both non-permeable
     non-vulnerable squares */
  for (i=0; (i < ROOM_DEPTH + 1); i++)
    if (whats_here[i])
      {
	if (!info[whats_here[i]]->permeable)
	  permeable = FALSE;
      }

  return (permeable || (info[type]->ignoring));
}



/* return TRUE if this type of missile should be allowed onto this square */

int can_fire_onto(roomnum, type, x, y)
int roomnum, x, y, type;
{
  unsigned char *whats_here;
  int           i;
  int     can_move = TRUE;
  int    will_destroy = FALSE;
  int     permeable = TRUE;

  /* if out of bounds then sure, we can fire there */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return TRUE;

  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for non-permeable or both non-permeable
     non-vulnerable squares */
I 10

E 10
  for (i=0; (i < ROOM_DEPTH + 1); i++) 
    if (whats_here[i])
      {
D 14
	if (!info[whats_here[i]]->move)
E 14
I 14
	if (!info[whats_here[i]]->movement)
E 14
	  can_move = FALSE;
	
	if (info[whats_here[i]]->vulnerable && info[type]->destroys) 
	  will_destroy = TRUE;
	
	if (!info[whats_here[i]]->permeable)
	  permeable = FALSE;
      }

  return (permeable || will_destroy || can_move || (info[type]->ignoring));
}



/* destroy the vulnerable objects on a given square, and notify all other
D 3
   players about the destruction */
E 3
I 3
   players about the destruction.  If any objects on the square are
D 4
   flamable, then cause then to explode. */
E 4
I 4
   flamable, then cause then to explode.  Objects that are not vulnerable
   cannot be flamable (they might continue exploding indefinitely) */
E 4
E 3

D 3
destroy_square(roomnum, x, y)
E 3
I 3
destroy_square(roomnum, x, y, m)
E 3
int roomnum, x, y;
I 3
Missile *m;
E 3
{
I 3
  int i;
E 3
  unsigned char *here;
D 3
  register int i;
E 3
I 3
  int explode_set[ROOM_DEPTH+1], num_explodes = 0;
E 3

  /* if location is out of bounds, forget trying this */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return;

  /* find out what is on the square */
  here = whats_on_square(roomnum, x, y);

D 4
  /* go through each object, removing it if it is vulnerable */
E 4
I 4
  /* go through each object, removing it if it is vulnerable,
     setting it for explosion and removing it if it is flamable. */
E 4
D 3
  for (i=0; i<ROOM_DEPTH+1; i++)
E 3
I 3
  for (i=0; i<ROOM_DEPTH+1; i++) {
D 4
    if (info[here[i]]->flamable) {
      explode_set[num_explodes] = here[i];
      num_explodes++;
    }
E 3
    if (info[here[i]]->vulnerable)
E 4
I 4
    if (info[here[i]]->vulnerable) {
D 14
      if (info[here[i]]->flamable) {
E 14
I 14
      if (info[here[i]]->flammable) {
E 14
        explode_set[num_explodes] = here[i];
        num_explodes++;
      }
E 4
      if (info[here[i]]->recorded) {
	MemObj *first, *ptr;
	/* get a pointer to the object info here previously */
	first = what_recorded_obj_here(roomnum, x, y);
D 19
	if (first == NULL) Gerror("bogus recorded object in room");
E 19
	/* make a copy of that object info */
D 3
	ptr = (MemObj *) malloc(sizeof(MemObj));
	if (ptr == NULL) Gerror("not enough memory for object copy");
E 3
I 3
	ptr = allocate_mem_obj();
E 3
D 19
	bcopy(first, ptr, sizeof(MemObj));
E 19
I 19
	if (first) bcopy(first, ptr, sizeof(MemObj));
E 19
	/* change the part that says what kind of object it is */
	ptr->obj.objtype = info[here[i]]->destroyed;
	/* tell everyone about the change to the map */
        notify_all_of_map_change(roomnum, x, y, i, here[i],
				 (info[here[i]]->destroyed),
				 &(ptr->obj));
	/* free the copy, we no longer need it */
	free(ptr);
      }
      else {
	/* when the object is not recorded it's much less trouble */
        notify_all_of_map_change(roomnum, x, y, i, here[i],
				 (info[here[i]]->destroyed), NULL);
      }
I 4
    }
E 4
I 3
  }

  for (i=0; i<num_explodes; i++) {
D 14
    int radius = info[explode_set[i]]->flamable - 1;
E 14
I 14
    int radius = info[explode_set[i]]->flammable - 1;
E 14
    explode_at_square(m->owner, roomnum, x, y, radius,
		      info[explode_set[i]]->boombit,
		      (int)info[explode_set[i]]->arc,
		      (int)info[explode_set[i]]->spread,
D 8
		      FanDirection(m->heading, (int)info[explode_set[i]]->fan));
E 8
I 8
		      FanDirection(m->heading, 
				   (int)info[explode_set[i]]->fan));
E 8
  }
E 3
}

D 11


/* return TRUE if there is a missile destined for the given square within the
   given search range.  Ranges of less than 1 or an error. */

int targeted_square(roomnum, x, y, range)
int roomnum, x, y, range;
{
  Missile *ptr;
  int i, targeted = FALSE;
  Location *loc;

  /* look through missiles in this room */
  for (ptr=room_missiles[roomnum].next; (ptr && !targeted); ptr=ptr->next) {
    for (loc=ptr->moveq,i=0; (loc && !targeted && i<range); loc=loc->next,i++)
      if (loc->x == x && loc->y == y) targeted = TRUE;
  }

  return targeted;
}



/* the following function returns FALSE if the given missile is not on the
   same square as any other missile found previously on the list.  It returns
   TRUE in the event that there is such a missile.  This provides a way to
   insure that redraws of missiles do not occur if another missile has
   been drawn on the square already. */

int another_missile_drawn(m, list)
Missile *m, *list;
{
  Missile *ptr;
  int found = FALSE;

  for(ptr=list; (ptr && ptr!=m && !found); ptr=ptr->next)
    if (ptr->x == m->x && ptr->y == m->y) found = TRUE;

  return found;
}



/* the following procedure looks for missiles in the list, previous to the
   given missile, that have the same oldx and oldy location and that
   did an erasure of that square after the last placement of the given
   missile.  It returns TRUE if it finds such a missile. */

int another_erasure_occurred(m ,list)
Missile *m, *list;
{
  Missile *ptr;
  int found = FALSE;

  for(ptr=list; (ptr && ptr!=m && !found); ptr=ptr->next)
    if (ptr->oldx == m->oldx && ptr->oldy == m->oldy)
      if ((m->lastmove - m->wait) > (ptr->lastmove - ptr->wait))
	  found = TRUE;

  return found;
}
E 11
D 25


E 25
/*****************************************************************/
D 25
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
E 25

int IsTransparent(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int           i;
  int           transparent = TRUE;

  /* if out of bounds then sure, we can fire there */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return FALSE;

  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for non-permeable or both non-permeable
     non-vulnerable squares */
  for (i=0; (i < ROOM_DEPTH + 1); i++) 
    {
      if (whats_here[i] && !info[whats_here[i]]->transparent)
	transparent = FALSE;
    }

  
  return transparent;
D 25
}

/*****************************************************************/
I 2

I 5
/* this procedure checks to see whether an auto-activate object is on the
 * square */

int is_mine_here(roomnum, x, y)
int roomnum, x, y;
{
  unsigned char *whats_here;
  int           i;
  int           restorable = TRUE;

  if (OUT_OF_BOUNDS(roomnum, x, y)) return FALSE;

I 24
#ifdef PLUGH
E 24
  /* look at what is on the square */
  whats_here = whats_on_square(roomnum, x, y);

  /* look at each item, look for non-permeable or both non-permeable
     non-vulnerable squares */
  for (i=0; (i < ROOM_DEPTH + 1); i++) {
D 14
      if (whats_here[i] && !info[whats_here[i]]->restorable)
E 14
I 14
      if (whats_here[i] && !info[whats_here[i]]->autoactor)
E 14
	restorable = FALSE;
  }
  return restorable;
I 24
#endif /* PLUGH */

  return FALSE;
E 24
}

/*********************************************************************/


#ifdef REDUNDANTCODE
/* Mel: This code wouldn't compile correctly and does some of the
   same things procedures in inventory.c do... it's better to alter
   them slightly to make them correct.  I took best parts of this
   function to do so. */

/* gets an object off the ground and puts it into either the named person's
   hand or inventory.  pnum is person number, i is map object slot number,
   and rem_obj is the object type number */

void get_object_from_ground(roomnum, x, y, pnum, i, rem_obj)
int roomnum, x, y, pnum, i, rem_obj;
{
  int slot;

D 14
  if(info[rem_obj]->thrown) { /*intohand*/
E 14
I 14
  if(info[rem_obj]->intohand) { /*intohand*/
E 14
    /* make sure this is unwieldable */
D 14
    if(info[rem_obj]->pushable) return; /*unswitchable*/
E 14
I 14
    if(info[rem_obj]->unswitchable) return; /*unswitchable*/
E 14

    /* find an empty slot */
    slot = open_inv_slot(pnum);
    if (slot < 0) return;

    /* delete from hand, if possible */
    swap_hand_and_inv(pnum, 0, slot);
    notify_all_of_map_change(roomnum, x, y, i, rem_obj, 0, NULL);
  }
  else {
    if (info[(uc)inv_obj.type]->recorded) {
      inv_obj.record = allocate_mem_obj();
      bcopy(take_obj, inv_obj.record, sizeof(MemObj));
    }
    else inv_obj.record = NULL;
    notify_all_of_map_change(roomnum, x, y, i, rem_obj, 0, NULL);
  }
}
#endif



/* This is the code to auto-activate an object which has been stepped on */

void detonate_mine(roomnum, x, y, pnum)
int roomnum, x, y, pnum;
{
  unsigned char *whats_here;
  int           i, j;
  char go = FALSE;

  if (OUT_OF_BOUNDS(roomnum, x, y)) return;

I 24
#ifdef PLUGH

E 24
  for (i=0; (i < ROOM_DEPTH + 1); i++) {
    /* look at what is on the square here */
    whats_here = whats_on_square(roomnum, x, y);
    j = whats_here[i];

D 14
    if (j && !info[j]->restorable) {
E 14
I 14
    if (j && !info[j]->autoactor) {
E 14
      /* code to do what is to be done */
      if(info[j]->recorded) {
        /* code for recorded stuff here */
      }
      else {
        /* light code here --- wait for RAYCODE to be implemented */
        /* takeable --- high prio */
D 14
        if(info[j]->armor) { /* taken */
E 14
I 14
        if(info[j]->autotaken) { /* taken */
E 14
          /* take item */
          take_object_from_ground(roomnum, x, y, pnum, i, j, -1);
	  continue;
	}

        /* weapon */
I 6
        if(info[i]->weapon) {
	  /*fire missile*/
	  
E 6
          /*lost*/
	  /* decrement ammo*/

        /* explodes */
          /*lost*/
          /*decrement ammo */

        /* swings */

        /* charges */
      }
      
      /* takeable --- low priority */
D 14
      if(!info[j]->armor) {
E 14
I 14
      if(!info[j]->autotaken) {
E 14
        /* take item */
        take_object_from_ground(roomnum, x, y, pnum, i, j, -1);
      }
    }
I 7
  }
E 7
  }
I 24
#endif /* PLUGH */
E 25
E 24
}
E 5
E 2
E 1
