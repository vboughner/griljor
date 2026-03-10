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

/* Special game person event queue routines */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "level.h"
#include "gametime.h"
#include "event.h"

/* Global variables for this file only */

PersonEvent *peq = NULL;/* person event queue front of the list pointer */


/* ===================== A D D I N G  events =========================== */


/* make an event record, and put in the most important values */

PersonEvent *make_event(id, cmd, time, first)
int id, cmd;
long time;
int first;
{
  PersonEvent *new;

  new = (PersonEvent *) malloc(sizeof(PersonEvent));
  if (!new) Gerror("no memory for future person event");

  bzero(new, sizeof(PersonEvent));
  new->id = id;
  new->cmd = cmd;
  new->time = time;
  new->z[0] = first;
  new->next = NULL;

  return new;
}



/* set the values of the integer array in the event */

set_event_option(event, index, value)
PersonEvent *event;
int index, value;
{
  if (index < 0 || index > 39) Gerror("event option index out of bounds");
  else event->z[index] = value;
}



/* set one of the two strings in the event */

set_event_string(event, which, msg)
PersonEvent *event;
int which;
char *msg;
{
  if (which < 0 || which > 1) Gerror("event string index out of bounds");
  else strcpy(event->s[which], msg);
}


/* set the erase on person's death flag to TRUE or FALSE */

set_event_death(event, value)
PersonEvent *event;
int value;
{
  event->die = value;
}



/* add an event to the queue, in a location according to it's execution
   time.  Events that occur later are to found in the back of the list. */

add_event_to_queue(event)
PersonEvent *event;
{
  PersonEvent *ptr, *last = NULL;

  for (ptr = peq; ptr; ptr = ptr->next) {
    if (ptr->time > event->time) break;
    last = ptr;
  }

  if (ptr == peq) {
    peq = event;
    event->next = ptr;
  }
  else {
    last->next = event;
    event->next = ptr;
  }
}



/* ================== C H E C K I N G  for events ====================== */

/* checks an event for errors, if there are any it returns NULL instead
   of the original event pointer */

PersonEvent *check_event_for_errors(event)
PersonEvent *event;
{
  if ((person_array(event->id) < 0) ||
      (event->cmd < 0 || event->cmd > PEQ_HIGHESTCMD)) {
	event = NULL;
	printf("Warning: person event queue error caught\n");
  }

  return event;
}



/* check to see if the front event in the queue should be pulled,
   returns a pointer to the event and removes it from the queue if so,
   otherwise it returns NULL */

PersonEvent *next_person_event()
{
  PersonEvent *result = NULL;

  if (peq) {
    if (peq->time <= gametime) {
      result = peq;
      peq = result->next;
      result = check_event_for_errors(result);
    }
  }

  return result;
}


/* do the periodic check of the event queue */

check_person_events()
{
  PersonEvent *new;

  new = next_person_event();
  while (new) {
    execute_event(new);
    free(new);
    new = next_person_event();
  }
}


/* ================ E X E C U T I N G  events =========================== */

/* given an event, try to execute it according to it's command number */

execute_event(event)
PersonEvent *event;
{
  int pnum = person_array(event->id);
  if (pnum < 0) return;

  /* commands that can be used on/by any person */
    switch (event->cmd) {
	case PEQ_NULL:
		break;
	case PEQ_APPEAR:
		gameperson[pnum]->appearance = event->z[0];
		move_in_place(pnum);
		break;
	case PEQ_EXPLOSION:
		explode_at_square(event->id, event->z[0], event->z[1],
				  event->z[2], event->z[3], event->z[4]);
		break;
	default:break;
    }
    
  /* commands that can be used on/by one's own persons only */
  if (one_of_me(event->id))
    switch (event->cmd) {
	case PEQ_INPUT:
		break;
	case PEQ_DEATH:
		dead_person_ritual(pnum);
		break;
	case PEQ_QUIT:
		set_quit_mode(QUITTING);
		break;
	case PEQ_VISIBLE:
	case PEQ_ALLSEE:
	default:break;
    }
}


/* =============== E A S Y  handlers for outside world =================== */

/* sets up a tombstone appearance style death for the person mentioned.
   They will appear as a tombstone for a while, and then come back to life */

setup_death(pnum)
int pnum;
{
  add_event_to_queue(make_event(gameperson[pnum]->id, PEQ_DEATH,
				gametime + CLICKS_TO_DIE, 0));
  add_event_to_queue(make_event(gameperson[pnum]->id, PEQ_APPEAR,
				gametime, APPEAR_DYING));
}


/* sets up an explosion to occur at a given gametime at a given location:
	pnum:		array element number of the person doing this
	roomnum, x, y:	location of the explosion
	radius:		radius of the explosion (0-20 or so)
			note: a large radius will signifigantly reduce game
			      performance while explosion is drawn
	type:		the type of explosion (0-2 right now) 
	time:		gameclick number when this will occur */
setup_explosion(pnum, roomnum, x, y, radius, type, time)
int pnum, roomnum, x, y, radius, type;
long time;
{
  PersonEvent *evt;

  evt = make_event(gameperson[pnum]->id, PEQ_EXPLOSION, time, 0);
  evt->z[0] = roomnum;
  evt->z[1] = x;
  evt->z[2] = y;
  evt->z[3] = radius;
  evt->z[4] = type;
  add_event_to_queue(evt);
}
