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

/* Routines for moving around */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "gametime.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "movement.h"
#include "level.h"
#include "queued.h"

/* ====================== M O V E M E N T  list maker =================== */



/* absolute value of a float */
#define labs(x)		(((x)>0) ? (x) : -(x))

/* maximum of two values */
#define max(x, y)	(((x)>(y)) ? (x) : (y))



/* allocate memory for location record */

Location *makelocation(x, y, next)
char x, y;
Location *next;
{

  Location *result  = (Location *)malloc(sizeof(Location));

  if (!result) {
    printf("Malloc failed in makelocation\n");
    exit(1);
  }
  result->x = x;
  result->y = y;
  result->next = next;
  return result;
}
    



/* given two xy locations in a room, give us a linked list containing
   the integral positions in between the two locations.  The initial
   and final positions are included in the list.  This provides movement
   limited to vertical and horizontal movements only */

Location *block_movement_list(x1, y1, x2, y2)
char x1, y1, x2, y2;
{
  int xdir = (x2 == x1) ? 0 : (x2-x1) / abs(x2-x1);
  int ydir = (y2 == y1) ? 0 : (y2-y1) / abs(y2-y1);
  int x = x1;
  int y = y1;
  float i,xf,yf;
  
  Location *result = makelocation(x1,y1,NULL);
  Location *end = result;  
  Location *temp;


  for (i = 0.0 ; i <= 1.0 ; i += .02) {
    xf = x1 + ((float)x2 - (float)x1) * i;
    yf = y1 + ((float)y2 - (float)y1) * i;
    if (labs(xf-x) > 0.9) x += xdir;
    else if (labs(yf-y) > 0.9) y += ydir;
    if ((x == end->x) && (y == end->y)) continue;
 
    temp = makelocation(x,y,NULL);
    end->next = temp;
    end = temp;
  }
  if (end->x != x2 || end->y != y2) {
    temp = makelocation(x2,y2,NULL);
    end->next = temp;
  }

  return result;
}




/* given two xy locations in a room, give us a linked list containing
   the integral positions in between the two locations.  The initial
   and final positions are included in the list.  This provides movement
   which includes all 8 directions, ie. diagonal moves are included */

Location *diag_movement_list(x1, y1, x2, y2, overlap)
char x1, y1, x2, y2;
int overlap;
{
  int xdir = (x2 == x1) ? 0 : (x2-x1) / abs(x2-x1);
  int ydir = (y2 == y1) ? 0 : (y2-y1) / abs(y2-y1);
  int xdist = abs(x2-x1);
  int ydist = abs(y2-y1);
  int x = x1;
  int y = y1;
  float i,xf,yf;

  Location *result = makelocation(x1,y1,NULL);
  Location *end = result;  
  Location *temp;

  for (i = 0.0 ; i <= 1.0 ; i += 1 / (float) ((xdist > ydist) ? xdist : ydist))
    {
      xf = x1 + ((float)x2 - (float)x1) * i;
      yf = y1 + ((float)y2 - (float)y1) * i;
      if (labs(xf-x) > 0.5) x += xdir;
      if (labs(yf-y) > 0.5) y += ydir;
      if ((!overlap) && (x == end->x) && (y == end->y)) continue;
      if ((overlap) && (x == result->x) && (y == result->y)) continue;

      temp = makelocation(x,y,NULL);
      end->next = temp;
      end = temp;
    }
  if (end->x != x2 || end->y != y2) {
    temp = makelocation(x2,y2,NULL);
    end->next = temp;
  }
  return result;
}



/* calls the default current movement list maker */

Location *movement_list(x1, y1, x2, y2)
char x1, y1, x2, y2;
{
  Location *result;

  result = diag_movement_list(x1, y1, x2, y2, FALSE);
  remove_move_head(&result);
  
  return result;
}



/* remove the head of the movement queue, freeing the now unused record,
   making the new head of the list be the following record */

remove_move_head(qptr)
Location **qptr;
{
  Location *old;

  /* if there is no list then forget it */
  if (!(*qptr)) return;

  /* make 'old' point to the old queue head */
  old = *qptr;

  /* make the element after queue head into the new queue head */
  *qptr = old->next;

  /* free the now unused old queue head record */
  if (old) free(old);
}



/* free the entire move queue so that this movement is stopped */

free_move_queue(qptr)
Location **qptr;
{
  Location *previous = NULL, *current;

  /* free each thing in the queue */
  for (current = *qptr; current; current = current->next) {
    if (previous) free(previous);
    previous = current;
  }

  /* nullify head pointer to the queue */
  *qptr = NULL;
}




/* ========================= G E N E R A L ============================== */


/* Handle a request to move just one square in a certain direction, these
   are the directional movement keys: qwe, asd, and zxc (numbered 0 - 8)
   The middle key (s) will be used for a 'random' movement. */

directional_move(pnum, dir)
int pnum, dir;
{
  /* ignore erroneous input */
  if (pnum >= num_persons) return;
  if (dir < 0 || dir > 8) return;

  /* do the requested movement */
  switch (dir) {
	case 0:	request_movement(pnum, gameperson[pnum]->x - 1,
				 gameperson[pnum]->y - 1, FALSE);
		break;
	case 1:	request_movement(pnum, gameperson[pnum]->x,
				 gameperson[pnum]->y - 1, FALSE);
		break;
	case 2:	request_movement(pnum, gameperson[pnum]->x + 1,
				 gameperson[pnum]->y - 1, FALSE);
		break;
	case 3:	request_movement(pnum, gameperson[pnum]->x - 1,
				 gameperson[pnum]->y, FALSE);
		break;
	case 4:	request_movement(pnum, (lrand48() % (ROOM_WIDTH + 2) - 1),
				 (lrand48() % (ROOM_HEIGHT + 2) - 1), FALSE);
		break;
	case 5:	request_movement(pnum, gameperson[pnum]->x + 1,
				 gameperson[pnum]->y, FALSE);
		break;
	case 6:	request_movement(pnum, gameperson[pnum]->x - 1,
				 gameperson[pnum]->y + 1, FALSE);
		break;
	case 7:	request_movement(pnum, gameperson[pnum]->x,
				 gameperson[pnum]->y + 1, FALSE);
		break;
	case 8:	request_movement(pnum, gameperson[pnum]->x + 1,
				 gameperson[pnum]->y + 1, FALSE);
		break;
  }
}



/* Handle a request to move a person towards a given x and y location.
   Do this by creating a movement queue for them, extend the range if
   the extend variable is passed as TRUE */

request_movement(pnum, x, y, extend)
int pnum, x, y, extend;
{
  Location *queue;

  /* if there is an old move queue, free it */
  if (gameperson[pnum]->moveq) free_move_queue(&(gameperson[pnum]->moveq));

  /* extend the range if requested */
  if (extend) extend_range(gameperson[pnum]->x, gameperson[pnum]->y,
			   &x, &y, max(ROOM_WIDTH, ROOM_HEIGHT));

  /* request a new move queue that moves us toward new location */
  gameperson[pnum]->moveq =  movement_list(gameperson[pnum]->x,
					   gameperson[pnum]->y, x, y);
 
  /* update the lastmove variable if last movement was long ago */
  if (gameperson[pnum]->lastmove < gametime - gameperson[pnum]->movewait)
    gameperson[pnum]->lastmove = gametime - gameperson[pnum]->movewait;
}



/* check to see if any of the persons in the game
   need to be moved.  They will need to be moved when their movement
   countdown reaches zero and they have a movement queue */

move_persons()
{
  register int i;
  
  /* go through each person in the game, looking for those we move queues */

  for (i=0; i<persons_in_game; i++)
    if ((gameperson[i]->lastmove+gameperson[i]->movewait) < gametime)
      {
	if (gameperson[i]->moveq)
	  execute_next_move(i);
	else if (i<num_persons)
	  check_for_exit_popoff(i);
      }
}

/* given a person number, execute his next move.  This will only be
   called at times when he is allowed to make that move, so we don't
   need to test that */

execute_next_move(num)
int num;
{
  int possible, dest_room;
  MemObj *exit_char;

  /* check to make sure there is a move queue */
  if (gameperson[num]->moveq) 
    {
    /* check to see if he is leaving the room */
    if (gameperson[num]->moveq->x < 0 || gameperson[num]->moveq->y < 0 ||
	gameperson[num]->moveq->x >= ROOM_WIDTH ||
	gameperson[num]->moveq->y >= ROOM_HEIGHT) 
      {
	/* this person has moved off edge of room */
	move_off_room_edge(num);
      }
    else 
      {	
	/* if he is moving onto an exit character, then send him off */
	exit_char = exit_char_on_square(gameperson[num]->room,
					gameperson[num]->moveq->x,
					gameperson[num]->moveq->y);
	if (exit_char) {
	  person_follow_exit_char(num, exit_char);
	  return;
	}


	/* no exit char, he is just moving to a new spot in same room */
	possible = move_person(num, gameperson[num]->room,
			       gameperson[num]->moveq->x,
			       gameperson[num]->moveq->y, FALSE);
	
	/* handle movement queue according to whether move was made or not */
	if (possible)
	  {
	    remove_move_head(&(gameperson[num]->moveq));
	    gameperson[num]->lastmove += gameperson[num]->movewait;
	  }
	else
	  free_move_queue(&(gameperson[num]->moveq));
      }
  }
}




/* move a gameperson to a new location, return 0 if no movement was made,
   the anywhere variable is set to TRUE if movement should be allowed to
   locations off of the map */

int move_person(num, rooom, x, y, anywhere)
int num, rooom, x, y, anywhere;
{
  int speed;
  
  /* check for legal person number */
  if (num < 0  ||  num >= persons_in_game) return FALSE;
 
  /* check to make sure this is a legal location, if not then don't
     do the data structure dependent checking below */

  if (rooom >= 0 && rooom < mapstats.rooms && x >= 0 && y >= 0 &&
      x < ROOM_WIDTH && y < ROOM_HEIGHT) {

      /* check out the destination square for movement speed allowed on it */
      speed = max_move_on_square(rooom, x, y);

      /* if he is hitting a wall, then stop this movement */
      if (speed==0 && !gameperson[num]->ignore_walls)  return FALSE;
    
      /* if this is one of my own persons then do a collision check with
         ther persons in the room and stop the movement if we are
	 going to hit another person */
      if (num < num_persons)
        if (will_be_collision(num, rooom, x, y)) return FALSE;

      /* update his new movement speed according to what he has moved onto,
	 if he is one of our people, then adjust for damage he has taken */
      gameperson[num]->movewait = wait_clicks_for_speed(speed);
      if (num < num_persons) {
	gameperson[num]->movewait = gameperson[num]->movewait *
				    max_person_health(num) /
				    max(person[num]->health,
					max_person_health(num) / 4);
      }

#ifdef RAYCODE
      if (num == 0) {
        if(gameperson[num]->room != rooom) {
          InitRaypack(gameperson[num]->room);
          redraw_exit_window();
        }
      }
#endif
  }

  /* only do the rest of this stuff if the anywhere variable is TRUE or if
     the location chosen was a legal one */
     
  if ((rooom >= 0 && rooom < mapstats.rooms && x >= 0 && y >= 0 &&
       x < ROOM_WIDTH && y < ROOM_HEIGHT) || (anywhere)) {
  
    /* update variable values */
    if (DEBUG) printf("Moving person (id %d) to room %d loc %d %d\n",
		      gameperson[num]->id, rooom, x, y);

    gameperson[num]->oldroom = gameperson[num]->room;
    gameperson[num]->oldx = gameperson[num]->x;
    gameperson[num]->oldy = gameperson[num]->y;
    gameperson[num]->room = rooom;
    gameperson[num]->x = x;
    gameperson[num]->y = y;

    /* if this is on a display then call the player redraw routine */
    if (has_display)
      if (num < num_persons) {
#ifdef RAYCODE
          if ((rooom >= 0 && rooom < mapstats.rooms && x >= 0 && y >= 0 &&
            x < ROOM_WIDTH && y < ROOM_HEIGHT)) {
          current_room = gameperson[0]->room;
	if (ROOM_IS_GLOBAL(rooom)) {
#endif
          erase_old_game_person(num);
	  flag_all_visibilities();
  	  redraw_newly_visible_persons();
#ifdef RAYCODE
	}
	else if (ROOM_IS_LIT(rooom)) {
	  QDraw_priority_request(gameperson[num]->oldroom,
				 gameperson[num]->oldx, gameperson[num]->oldy,
				 UNCONCEALED);
	  DrawEgoRaysWithoutLight(rooom, x, y);
	}
	else if (ROOM_IS_DARK(rooom)) {
	  EraseOwnSquare(gameperson[num]->oldroom, gameperson[num]->oldx,
			 gameperson[num]->oldy);
	  DrawEgoRaysWithLight(rooom, x, y, OWN_LIGHT(num));
	}
	else 
	  printf("Whoa! Room %d is not lit, global, or dark!\n", rooom);
	  }
#endif
      }
      else {
        erase_old_game_person(num);
	flag_visibility(num);
        redraw_game_person(num);
        /* redraw the possible persons left underneath them */
        redraw_persons_here(gameperson[num]->oldroom, gameperson[num]->oldx,
			    gameperson[num]->oldy);
      }

    /* tell everyone in room where I am if this is one of my persons,
       unless I've changed rooms, in which case tell everyone my location */
    if (num < num_persons) {
      if (gameperson[num]->room == gameperson[num]->oldroom)
        notify_room_of_location(num);
      else {
	/* Tell all of room change, if I'm changing rooms, wait for redraws
	   to finish up before notifying all of new location */
	/* if (has_display) wait_for_graphics(); */
        notify_all_of_location(num);
      }
    }

    /* check for the need to update the room person array */
    person_has_changed_rooms(num);

    /* check if this is not one of my persons and he is changing rooms,
       if so, be alert that we might need to tell him where we are */
    if (num >= num_persons  &&
        gameperson[num]->room != gameperson[num]->oldroom)
		notify_incoming_of_locations(num);

    return TRUE;
  }
  else return FALSE;
}



/* check for persons on the specified square, return FALSE if we won't
   collide, or return the person's id number of who we would collide with */

int will_be_collision(pnum, roomnum, x, y)
int pnum, roomnum, x, y;
{
  PersonList	*ptr;
  int		collide = FALSE;

  /* if we are changing rooms then there will be no collision */
  if (gameperson[pnum]->room != roomnum) return FALSE;

  /* look through room for people on the square we want */
  for (ptr = room_persons[roomnum].next; (ptr && !collide); ptr = ptr->next)
    if ((ptr->person->id != gameperson[pnum]->id) &&
	(ptr->person->x == x && ptr->person->y == y) &&
	(ptr->person->appearance != APPEAR_DYING))
		collide = ptr->person->id;

  if (DEBUG) printf("Collision check returns %d\n", collide);
  return collide;
}



/* move up and down in place, ie. notify everyone again of my location
   so that my appearance can be redrawn */

move_in_place(pnum)
int pnum;
{
  move_person(pnum, gameperson[pnum]->room, gameperson[pnum]->x,
	      gameperson[pnum]->y, FALSE);
}



/* ======================= L E A V I N G  the room ====================== */

/* find out if there is an exit character on a certain square, if so,
   then return the MemObj record containing destination, if not then
   return NULL */

MemObj *exit_char_on_square(roomnum, x, y)
int roomnum, x, y;
{
  MemObj *recorded_obj;

  /* get recorded object that is here */
  recorded_obj = what_recorded_obj_here(roomnum, x, y);

  /* if there is no recorded object then return NULL */
  if (recorded_obj == NULL) return NULL;

  /* if it is not an exit character return NULL */
  if (!info[(uc)recorded_obj->obj.objtype]->exit) return NULL;

  /* otherwise return a pointer to the object */
  return recorded_obj;
}




/* person moved of edge of room, or has at least tried to.  Check it, and
   move him to the appropriate room */

move_off_room_edge(num)
int num;
{
  int direction=NORTH, dest_room, xchange=0, ychange=0;
  
  /* figure out where he was going */
  if (!gameperson[num]->moveq) return;
  xchange = ((gameperson[num]->moveq->x < 0) ? -1 : 0);
  xchange = ((gameperson[num]->moveq->x >= ROOM_WIDTH) ? 1 : xchange);
  ychange = ((gameperson[num]->moveq->y < 0) ? -1 : 0);
  ychange = ((gameperson[num]->moveq->y >= ROOM_HEIGHT) ? 1 : ychange);
  if (xchange && ychange) return;
  if (xchange) direction = ((xchange == 1) ? EAST : WEST);
  else direction = ((ychange == 1) ? SOUTH : NORTH);
  dest_room = room[gameperson[num]->room].exit[direction];

  /* abort any future movements left on queue */
  free_move_queue(&(gameperson[num]->moveq));

  /* if the destination room isn't assigned then -1 was returned */
  if (dest_room < 0 || dest_room >= mapstats.rooms) return;

  /* move us into new room */
  move_person(num, dest_room, gameperson[num]->x +
	      (xdir[direction] * (-ROOM_WIDTH +1)), gameperson[num]->y +
	      (ydir[direction] * (-ROOM_HEIGHT +1)), FALSE);
}




/* a person has moved onto an exit character, given the recorded info
   for the character and the person's number, move him to new location */

person_follow_exit_char(num, exit)
int num;
MemObj *exit;
{
  /* move the person to new location */
  move_person(num, exit->obj.detail, exit->obj.infox, exit->obj.infoy, FALSE);

  /* clear the movement queue */
  free_move_queue(&(gameperson[num]->moveq));

  /* set time for next move */
  gameperson[num]->lastmove += gameperson[num]->movewait;
}



/* check to see if one of our persons is standing on an exit character,
   if so send him off to the target location */

check_for_exit_popoff(num)
int num;
{
  MemObj *exit_char;

  exit_char = exit_char_on_square(gameperson[num]->room, gameperson[num]->x,
				  gameperson[num]->y);
  if (exit_char) person_follow_exit_char(num, exit_char);
}


/* ============================ M I S C ================================ */


/* given a speed number (0 - 9, slow - fast), return the number of clicks
   we should wait between moves */

int wait_clicks_for_speed(speed)
int speed;
{
  /* 5 is the usual speed */
  if (speed == 0)
    {
      printf("The object is defined with a 0 speed!\n");
      speed = 1;
    }

  return CLICKS_PER_MOVE * 5 / speed;
}
