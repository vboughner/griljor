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
#include "queued.h"
#include "message.h"

/*
 * Used in prepare_missile_course to set the path for a new missile.
 * Having them global may be kinda bad, but hey, it's easy.
 *
 *						-blojo
 */

char local_x[MAX_MISSILE_RANGE], local_y[MAX_MISSILE_RANGE];

/* Global variables for use by anybody */

Missile		**all_missiles;		/* points to the missile heap */
int  nummissiles,maxmissiles;


#define TRUNC(c) (int)((unsigned char)c)

/* ========================= F I R I N G =============================== */

/* Given lots of information, this procedure will do everything concerned
   with firing a missile, including telling everyone else about it (everyone
   that needs to know that is)  Using movingobject, and not boombit */

fire_missile(owner, id, roomnum, x1, y1, x2, y2, type)
int	owner;		/* person id who is firing the missile */
InvObj	*id;		/* id pointer of this missile (book-keeping) */
int	roomnum;	/* room it was fired in */
int	x1, y1, x2, y2;	/* start and end coords in room of missile path */
int	type;		/* the type number of the missile */
{
  int dir,dis;

  if (info[type]->arc == 0)
    fire_single_missile(owner,id,roomnum,x1,y1,x2,y2,type);
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


fire_single_missile(owner, id, roomnum, x1, y1, x2, y2, type)
int	owner;		/* person id who is firing the missile */
InvObj	*id;		/* id pointer of this missile (book-keeping) */
int	roomnum;	/* room it was fired in */
int	x1, y1, x2, y2;	/* start and end coords in room of missile path */
int	type;		/* the type number of the missile */
{
  Missile *m;
  /* allocate memory for a new missile record */
  m = (Missile *) malloc(sizeof(Missile));
  if (m == NULL) Gerror("out of memory noticing missile");

  FanMissileCourse(x1,y1,&x2,&x2,(int) info[type]->fan);

  prepare_missile_poop(m, owner, TRUNC(info[type]->movingobj), FALSE);

  /* determine the course the missile should follow */
  prepare_missile_course(m, roomnum, x1, y1, &x2, &y2,
			 info[type]->range, info[type]->stop);

  prepare_missile_direction(m, TRUNC(info[type]->movingobj),
			    x1, y1, x2, y2);

  /* prepare the missile packet and send it */
  prepare_missile_packet(m, local_x, local_y);

  /* tell myself about the missile too */
  add_missile(m);
}


/* prepare the poop (information) about a missile */

prepare_missile_poop(m, owner, type, hurts)
Missile *m;
int owner;
int type, hurts;
{
  m->owner = owner;
  m->type = type;
  m->hurts_owner = hurts;

  /* figure out speed for missile objects */
  m->wait = missile_wait(type);

  /* set a few things to NULL, they will be used later */
  m->moveq = NULL;
  m->lastmove = 0;
  m->oldx = -1;
  m->oldy = -1;
}


/*****************************************************************/

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
{  Missile *new;
   int range;
   Location *move;
   int i;
   int newroom;

  /* allocate memory for a new missile record */
  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory noticing missile");

   move = m->moveq;

   /* place room number into missile packet (yes, I mean single =), if
      there is no legal exit, then abort */

   if ((newroom = new_room_for_refire(move, room)) == -1) return; 

   new->room = newroom;

   /* translate moveq in appropriate direction, also set range */
   if (move->x < 0) translate_x(ROOM_WIDTH, move, &range);
   else if (move->x >= ROOM_WIDTH) translate_x(-ROOM_WIDTH, move, &range);
   else if (move->y < 0) translate_y(ROOM_HEIGHT, move, &range);
   else if (move->y >= ROOM_HEIGHT) translate_y(-ROOM_HEIGHT, move, &range);
   else /* this proceedure has been called in error */ {
     printf("refire_missile_in_new_room called in error\n");
     return;
   }

   /* place the info about missile inside structure */
   prepare_missile_poop(new, m->owner, m->type, m->hurts_owner);

   /* copy the direction orientation */
   new->direction = m->direction;
   new->heading = m->heading;

  /* place the movement queue into struct */
  for (i=0; (move); i++) {
    local_x[i] = move->x;
    local_y[i] = move->y;
    move = move->next;
  }

  new->range = i;

  /* set current location */
  new->x = local_x[0];
  new->y = local_y[0];

  /* prepare the missile packet and send it */
  prepare_missile_packet(new, local_x, local_y);

  /* tell myself about the missile too */
  add_missile(new);
}


/* As a missile leaves a room, initiate a new missile using the old
   missile pack information.  Inform all appropriate players (possibly
   including self) This routine is specifically for missiles that have
   encountered exit characters */

refire_missile_through_exit(m, exit_char)
Missile *m;
MemObj *exit_char;
{  Location *move;
   int i;
   Missile *new;
   int newroom;

  /* allocate memory for a new missile record */
  new = (Missile *) malloc(sizeof(Missile));
  if (new == NULL) Gerror("out of memory noticing missile");

  /* place room number into missile packet (yes, I mean single =), if
     there is no legal exit, then abort */

  if ((newroom = exit_char->obj.detail) == -1) return; 

  new->room = newroom;

  /* make a new movement queue */
  move = makelocation(exit_char->obj.infox, exit_char->obj.infoy, NULL);

  /* place the info about missile inside network packet */
  prepare_missile_poop(new, m->owner, m->type, m->hurts_owner);

  /* copy the direction orientation */
  new->direction = m->direction;

  /* place the movement queue into struct */
  for (i=0;(move); i++) {
    local_x[i] = move->x;
    local_y[i] = move->y;
    move = move->next;
  }
  new->range = i;
  new->owner = m->owner;
  new->heading = m->heading;

  /* place current location into missile definition */
  new->x = local_x[0];
  new->y = local_y[0];

  /* prepare the rest of the missile packet and send it */
  prepare_missile_packet(new, local_x, local_y);

  /* tell myself about the missile too */
  add_missile(new);
}


/* Prepare the course of a missile.
 * We need to be given the range to extend the missile.
 * If x2 and y2 end up being changed, the new values will be returned.
 */

prepare_missile_course(m, roomnum, x1, y1, x2, y2, range,stops)
Missile *m;
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
  if (!stops) extend_range(x1, y1, x2, y2, range);

  m->heading = MissileHeading(x1, y1, *x2, *y2);
  /* figure out the straight line course */
  move = diag_movement_list(x1, y1, *x2, *y2, TRUE);
  /* don't need to move the starting square */
  remove_move_head(&move);

  /* place the movement queue into packet */
  for (i=0; (i<range) && (move); i++) {
    old = move;
    local_x[i] = move->x;
    local_y[i] = move->y;
    move = move->next;
    free(old);
  }

  m->range = i;
  m->room = roomnum;

  /* place current location into missile definition */
  m->x = local_x[0];
  m->y = local_y[0];
}



/* figure out and place missile direction in packet */

#define motion_amount(a, b)		(((a)>=(b)) ? 0 : ((b)-(a)))
#define first_is_max(a, b, c, d)	((a)>=(b) && (a)>=(c) && (a)>=(d))

prepare_missile_direction(m, type, x1, y1, x2, y2)
Missile *m;
int type, x1, y1, x2, y2;
{
  int up, right, down, left, result;

  /* if this is a non-directional kind of missile then 0 is the default */
  if (!(info[type]->directional)) {
    m->direction = 0;
    return;
  }

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
  m->direction = result;
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



FanMissileCourse(x1,y1,x2,y2,fan)
int x1,y1,*x2,*y2,fan;
{
  double dis;
  int dir;

  if (fan == 0)
    return;

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

/* =========================== N O T I C I N G =========================== */

/* given a missile packet, either received or one we created ourselves,
   add the missile to one of our room missile lists, if we have any
   of our persons in the missile's room */

add_missile(new)
Missile *new;
{
  register int i;
  Location *this, *last;

  /* create movement list for the new missile */
  for (i = 0, last = NULL; i < new->range; i++) {
    this = makelocation(local_x[i], local_y[i], NULL);
    if (last) last->next = this;
    else new->moveq = this;
    last = this;
  }

  /* mark this missile as a >new< missile.  It's position will 
     be taken from the first entry on the movement que */
  new->lastmove = 0;
  new->x = -1;  new->y = -1;
  new->oldx = -1; new->oldy = -1;

  /* place missile in the correct room list, at the beginning */
  AddMissileToHeap(new);
}





/* =========================== M O V I N G =============================== */

/* initialize the missile heap array */

initialize_missile_array()
{
  nummissiles = 0;
  maxmissiles = 10000;

  all_missiles = (Missile **) malloc(sizeof(Missile) * maxmissiles);

  if (all_missiles == NULL)
    Gerror("No memory for missiles");
}


free_missile_array()
{
  free(all_missiles);
}

/*****************************************************************/

htime(i)
int i;
{
  Missile *m;

  m = all_missiles[i];
  if (m == NULL)
    return(0);
  
  return(m->lastmove + m->wait);
}

Missile *TopOfHeap()
{
  if (nummissiles)
    return(all_missiles[0]);
  else
    return(NULL);
}

void AddMissileToHeap(add)
Missile *add;
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

BubbleUp(k)
int k;
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

BubbleDown(k)
int k;
{
  int j,t;
  Missile *m;

  m = all_missiles[k];
  j = k*2 + 1;
  t = htime(k);

  while (j < nummissiles)
    {
      if (((j+1) < nummissiles)&&(htime(j+1) < htime(j)))
	j++;
      
      if (htime(j) >= t)
	break;
      
      all_missiles[k] = all_missiles[j];
      k = j;
      j = k*2 + 1;
    }
  
  all_missiles[k] = m;
}

/*****************************************************************/
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

/*****************************************************************/

/* Check for the need to move any of the missiles we know about.  If any of
   the missiles we check want to die, then delete them from their room list. */

update_missiles()
{
  Missile *current;
  int missle_end, refire,count;

  current = TopOfHeap();
  
  while((current) && (gametime >= current->lastmove + current->wait))
    {
      RemoveTopOfHeap();

      refire = FALSE;
      
      /* handle the individual missile here */
      missle_end = update_individual_missile(current, &refire);
      
      /* remove the missile if it is at it's end */
      if (!missle_end) 
	AddMissileToHeap(current);
      else
	{
	  /* remove the rest of this missiles movement que */
	  free_move_queue(&(current->moveq));

	  /* refire will only be true when missile is 
	     going on to another room and we musn't explode 
	     or drop it yet */
	  
	  if ((one_of_me(current->owner) && (!refire)))
	    drop_or_explode_it(current);
	  
	  free(current);
	}

      current = TopOfHeap();
    }
}


/* update an individual missile's position, if it is time to do so.  Return
   TRUE if this missile is now finished and needs to either be deleted,
   cause an explosion or be placed in the room next to dying location.
   Which of the above occurs depends on the missile's type.
   Set keep_ptr to TRUE if we should not free the object copy.  Do
   nothing to it if we don't care what caller does with object copy. */
  
update_individual_missile(m, refire)
Missile *m;
int *refire;
{
  Location *move;
  int lucky_person,roomnum;
  int dont_let_it_get_away,first_shot;
  MemObj *exit_char = NULL;
  
  first_shot = (m->lastmove == 0);

  roomnum = m->room;

  if (!first_shot)
    if (has_display)
      QDraw_priority_request(roomnum,m->x,m->y,EVERYTHING); 
  
  /* let's check if there is any more movement queue */
  if (m->moveq == NULL) return TRUE;	/* missile is finished */
  move = m->moveq;

  /* check for destination out of bounds */
  if (move->x < 0 || move->x >= ROOM_WIDTH || 
      move->y < 0 || move->y >= ROOM_HEIGHT)
    {
      /* call leaving room routine (if it's mine) */
      if (one_of_me(m->owner))
	refire_missile_in_new_room(m, roomnum);
      
      *refire = TRUE;
      return TRUE;
    }
  
  /* check for missile hitting a wall or some other non-giving object,
     before it moves onto that square */

  if (!first_shot)
    if (!can_fire_onto(roomnum,m->type,move->x,move->y))
      return TRUE;
  
  /* everything checks out, let's move the rotten thing */
  m->oldx = m->x;	m->oldy = m->y;
  m->x = move->x;	m->y = move->y;		

  if (first_shot)
    m->lastmove = gametime;
  else
    m->lastmove += m->wait;

  remove_move_head(&(m->moveq));

  /* check if missile is going to try to leave a square that it can't leave.
     this is done here beacause the square may be changed, i.e. destroyed,
     allowing the object through when it should not be. */

  dont_let_it_get_away = ! can_fire_through(roomnum,m->type, m->x, m->y);

  /* if we own this missile and it destroys walls then try to do so */
  if (one_of_me(m->owner) && info[m->type]->destroys)
    destroy_square(roomnum, m->x, m->y, m); 

  /* check for missile hitting a person (other than its owner) */
  
  lucky_person = person_on_square(roomnum, m->x, m->y);
  if (lucky_person != -1  &&
      (lucky_person != person_array(m->owner) || m->hurts_owner)) 
    {
      /* register a hit in the hit array */
      add_new_hit(roomnum, m->x, m->y);
      /* check if it is one of my persons that is hit */
      if (lucky_person < num_persons) take_missile_damage(lucky_person, m);
      return TRUE;
    }


  /* check for a missile hitting an exit square, it should go one space
     through the exit */

  exit_char = exit_char_on_square(roomnum, m->x, m->y);
  if (exit_char) 
    {
      if (one_of_me(m->owner)) 
	refire_missile_through_exit(m, exit_char);
      *refire = TRUE;
      return TRUE;
    }

  if (dont_let_it_get_away)
    return TRUE;

  /* draw the missile */
  
  if (has_display)
      redraw_missile(m,roomnum);

  return FALSE;
}
  

/*****************************************************************/


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



own_player_damage_message(pnum, missile)
/* send ourselves the message about getting damage from a missile */
int pnum;
Missile *missile;
{
  char msg[MSG_LENGTH];
  char *fire_name = NULL, *fireteam_name = NULL;
  int firer = person_array(missile->owner);
  int strength = missile_damage(missile);

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



/* one of our guys is hit, take the damage if we have to, in any case,
   notify the owner of the missile that we've handled it  */

take_missile_damage(pnum, missile)
int pnum;
Missile *missile;
{
  int strength;

  /* nevermind this hit if he is already dead */
  if (person[pnum]->health <= 0) return;

  /* figure out strength of missile */
  strength = missile_damage(missile);
  if (!strength) return;	/* no damage done */
  
  /* UNFINISHED: do checks to see whether we should be vulnerable */

  /* take the damage from health points */
  person[pnum]->health -= strength;

  /* update health point display if we have a display */
  if (has_display && pnum==0) {
    redraw_player_health();
    own_player_damage_message(pnum, missile);
  }

  /* send a damage report to the owner of the missile */
  if (person_array(missile->owner) >= num_persons) {
    send_missile_owner_message(pnum, missile);
  }
  
  AddtoDamageRecord(pnum,strength,person_array(missile->owner));

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
    death_experience(pnum);
  }
}




/* missile's life is at an end, check to see if it should be deposited on
   floor nearby */

missile_dropped(m, roomnum)
Missile *m;
int roomnum;
{
  /* drop object only if does not explode, and the boombit is not 0 */

  if (info[m->type]->explodes == 0)
    if (info[m->type]->boombit)
      drop_type_near_square((int) info[m->type]->boombit,roomnum,m->x,m->y);
}



/* a missile has reached the end of it's path, check to see if it explodes */

drop_or_explode_it(m)
Missile *m;
{
  int explodes, explosion,radius;
  int roomnum;

  roomnum = m->room;
  explodes = info[m->type]->explodes;

  if (explodes == 0)
    missile_dropped(m, roomnum);
  else
    {
      explosion = info[m->type]->boombit;
      radius = explodes - 1;
      explode_at_square(m->owner, roomnum, m->x, 
			m->y, radius,explosion,
			(int) info[m->type]->arc,
			(int) info[m->type]->spread,
			FanDirection(m->heading,(int) info[m->type]->fan));
    }
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

  for (i=0; (i < ROOM_DEPTH + 1); i++) 
    if (whats_here[i])
      {
	if (!info[whats_here[i]]->movement)
	  can_move = FALSE;
	
	if (info[whats_here[i]]->vulnerable && info[type]->destroys) 
	  will_destroy = TRUE;
	
	if (!info[whats_here[i]]->permeable)
	  permeable = FALSE;
      }

  return (permeable || will_destroy || can_move || (info[type]->ignoring));
}



/* destroy the vulnerable objects on a given square, and notify all other
   players about the destruction.  If any objects on the square are
   flamable, then cause then to explode.  Objects that are not vulnerable
   cannot be flamable (they might continue exploding indefinitely) */

destroy_square(roomnum, x, y, m)
int roomnum, x, y;
Missile *m;
{
  int i;
  unsigned char *here;
  int explode_set[ROOM_DEPTH+1], num_explodes = 0;

  /* if location is out of bounds, forget trying this */
  if (OUT_OF_BOUNDS(roomnum, x, y)) return;

  /* find out what is on the square */
  here = whats_on_square(roomnum, x, y);

  /* go through each object, removing it if it is vulnerable,
     setting it for explosion and removing it if it is flamable. */
  for (i=0; i<ROOM_DEPTH+1; i++) {
    if (info[here[i]]->vulnerable) {
      if (info[here[i]]->flammable) {
        explode_set[num_explodes] = here[i];
        num_explodes++;
      }
      if (info[here[i]]->recorded) {
	MemObj *first, *ptr;
	/* get a pointer to the object info here previously */
	first = what_recorded_obj_here(roomnum, x, y);
	/* make a copy of that object info */
	ptr = allocate_mem_obj();
	if (first) bcopy(first, ptr, sizeof(MemObj));
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
    }
  }

  for (i=0; i<num_explodes; i++) {
    int radius = info[explode_set[i]]->flammable - 1;
    explode_at_square(m->owner, roomnum, x, y, radius,
		      info[explode_set[i]]->boombit,
		      (int)info[explode_set[i]]->arc,
		      (int)info[explode_set[i]]->spread,
		      FanDirection(m->heading, 
				   (int)info[explode_set[i]]->fan));
  }
}

/*****************************************************************/

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
}
