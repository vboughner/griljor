h43443
s 00000/00000/00205
d D 1.9 92/08/07 01:03:38 vbo 9 8
c source copied to a separate tree for work on new map and object format
e
s 00075/00230/00130
d D 1.8 91/09/03 21:39:20 labb-3li 8 7
c Trevor's misc changes - fixed missile heap
e
s 00000/00000/00360
d D 1.7 91/08/03 16:27:31 labc-3id 7 6
c fixed a small problem with large n in obj_place_pixmap()
e
s 00047/00009/00313
d D 1.6 91/05/27 14:35:34 labc-3id 6 5
c Made priority requests replace each other
e
s 00019/00001/00303
d D 1.5 91/05/26 22:43:25 labc-3id 5 4
c Worked on drawing improvements
e
s 00003/00037/00301
d D 1.4 91/05/17 02:04:53 labc-3id 4 3
c Intermediate work on offscreen pixmap
e
s 00036/00003/00302
d D 1.3 91/05/10 04:49:52 labc-3id 3 2
c exclusive oring of missiles
e
s 00002/00000/00303
d D 1.2 91/04/14 19:52:35 labc-3id 2 1
c 
e
s 00303/00000/00000
d D 1.1 91/02/16 12:55:45 labc-3id 1 0
c date and time created 91/02/16 12:55:45 by labc-3id
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
 * February 1991
 **************************************************************************/

#include <stdio.h>
#include "config.h"
#include "def.h"
D 8
/* #include "map.h" */
E 8
#include "queued.h"
#include "windowsX11.h"
I 3
#include "missile.h"
I 4
#include "roomdraw.h"
E 4
E 3

/* =================== Q U E U E D   D R A W I N G ===================== */

/* The idea of the queued drawing system is to have individual square
   redraw requests placed in a queue instead of being drawn directly to
   the screen.  The sending of these draw requests to the X server could
   then take place at a reasonable rate, allowing interspersion of input
   events and movement.  Thus a person who knew what a room looked like
   could move before it was completely drawn.  There will be regular checking
   of the queue for squares to redraw, and if the current room has changed
   since the request was made, the redraw will be ignored.  When a square is
   requested twice before getting redrawn, it will only be redrawn once. */


D 8

E 8
/* globals in this file for handling the ordered queue */

I 5
static NonQueuedDraws;          /* number of draws not using this package */
E 5
D 8
static QDraw	*head = NULL;	/* head of queue, next to be drawn */
static QDraw	*last = NULL;	/* last priority event in the queue */
static QDraw	*tail = NULL;	/* tail of queue, last thing requested */
E 8
I 8
static QDraw	*qone = NULL;	/* low priority */
static QDraw    *tone = NULL;
static QDraw	*qtwo = NULL;	/* high priority */
static QDraw    *ttwo = NULL;
E 8

D 8
/* global for an two-dimensional array of squares in a room.  Each time
   a draw request comes in, a check will be made for duplicate earlier
   requests for that square (in the same room).  The array will contain
   a linked list of pointers to QDraw structs that reside in the queue
   created through the pointers above. */

static QPoint	*req[ROOM_WIDTH][ROOM_HEIGHT];
I 3
D 4
static int      xored[ROOM_WIDTH][ROOM_HEIGHT];
E 4
E 3

E 8
D 3

E 3
/* other variables */

D 8


E 8
D 3

E 3
/* initialize the two-dimensional array to be empty */
QDraw_initialize()
{
  int i, j;

  /* free previously created requests if any */
D 8
  while (head) remove_request(head);
  head = NULL;
  last = NULL;
  tail = NULL;
E 8
I 8
  while (qone)
    remove_head(&qone);
  while (qtwo)
    remove_head(&qtwo);
E 8

D 8
  /* make sure array is emptied out */
  for (i=0; i<ROOM_WIDTH; i++)
    for (j=0; j<ROOM_HEIGHT; j++)
D 3
      req[i][j] = (QPoint *) NULL;
E 3
I 3
D 4
      {
	req[i][j] = (QPoint *) NULL;
	xored[i][j] = 0;
      }
E 4
I 4
      req[i][j] = (QPoint *) NULL;
I 5

E 8
  NonQueuedDraws = 0;
E 5
E 4
E 3
}


D 8

E 8
/* enter a new draw request into the queue */
QDraw_request(room, x, y, way)
int room, x, y;	/* location of square to draw */
int way;	/* draw method: BLACKED, CONCEALED, UNCONCEALED, EVERYTHING */
{
  QDraw *ptr;

  /* look for out of bounds request */
  if (x<0 || y<0 || x>=ROOM_WIDTH || y>=ROOM_HEIGHT) return;

D 8
  /* look for a prior request on the same square */
  ptr = prior_request_in_array(room, x, y);
E 8
I 8
  ptr = create_QDraw(room, x, y, way);
E 8

D 8
  /* if there is a prior one, replace it, otherwise add a new one */
  if (ptr) replace_request(ptr, way);
  else {
D 6
    ptr = create_QDraw(room, x, y, way);
E 6
I 6
    ptr = create_QDraw(room, x, y, way, 0);
E 6
I 5

E 5
    add_request_to_array(ptr);
    if (tail) {
      ptr->prev = tail;
      tail->next = ptr;
      tail = ptr;
      ptr->next = NULL;
E 8
I 8
  if (tone)
    {
      tone->next = ptr;
      tone = ptr;
E 8
    }
D 8
    else {
      ptr->prev = NULL;
      head = ptr;
      tail = ptr;
      ptr->next = NULL;
E 8
I 8
  else
    {
      tone = ptr;
      qone = ptr;
E 8
    }
D 8
  }
E 8
}


D 8

E 8
/* enter a new draw request with high priority.  It will be placed near the
   head of the queue, after the other previously entered priority requests */
QDraw_priority_request(room, x, y, way)
int room, x, y;	/* location of square to draw */
int way;	/* draw method: BLACKED, CONCEALED, UNCONCEALED, EVERYTHING */
{
  QDraw *ptr;

  /* look for out of bounds request */
  if (x<0 || y<0 || x>=ROOM_WIDTH || y>=ROOM_HEIGHT) return;

D 8
  /* look for a prior request on this square */
  ptr = prior_request_in_array(room, x, y);
E 8
I 8
  ptr = create_QDraw(room, x, y, way);
E 8

D 6
  /* if there is one, delete it.  We will be replacing it with priority */
D 5
  if (ptr) remove_request(ptr);
E 5
I 5
  if (ptr) 
    remove_request(ptr);
E 6
I 6
D 8
  /* if there is one, extract it.  We will be replacing it in a priority
   * position; if it already priority, replace it */
  if (ptr) {
    if(ptr->prio) {
      replace_request(ptr, way);
      return;
E 8
I 8
  if (ttwo)
    {
      ttwo->next = ptr;
      ttwo = ptr;
E 8
    }
D 8
    else {
      ptr = extract_request(ptr);
      ptr->way  = way;
      ptr->room = room;
E 8
I 8
  else
    {
      ttwo = ptr;
      qtwo = ptr;
E 8
    }
D 8
  }
  else ptr = create_QDraw(room, x, y, way, 1);
E 6
E 5

  /* add the request at the head of queue, after other priority requests */
D 6
  ptr = create_QDraw(room, x, y, way);
E 6
  add_request_to_array(ptr);
  if (last) {
    /* there are other priority requests to be placed after */
    ptr->prev = last;
    ptr->next = last->next;
    last->next = ptr;
    last = ptr;
    if (ptr->next) ptr->next->prev = ptr;
    else tail = ptr;
  }
  else {
    if (head) {
      /* this will be the only priority request in the queue, put at head */
      ptr->prev = NULL;
      ptr->next = head;
      head->prev = ptr;
      head = ptr;
      last = ptr;
    }
    else {
      /* there is no queue at all, this will be it */
      ptr->prev = NULL;
      ptr->next = NULL;
      head = ptr;
      last = ptr;
      tail = ptr;
    }
  }
E 8
}

I 5
/*****************************************************************/
E 5

I 5
QDraw_notify()
{
  NonQueuedDraws++;
}
E 5

I 5
D 8

E 8
E 5
/* grab the next few things in the queue and draw them.  This routine should
   be called once in a while to insure that things are eventually drawn. */
QDraw_cycle(num)
int num;	/* how many things to call from the queue, a negative
		   number signifies getting everything currently in it */
{
  int i, cease;
I 8
  QDraw **head;
E 8
I 5

  /* check to see how much non-queued drawing has been done */
  if (NonQueuedDraws > num)
    NonQueuedDraws = num;

  num -= NonQueuedDraws;
  NonQueuedDraws = 0;
E 5

D 8
  for (i=0; ((i<num || num<0) && head); i++) {
E 8
I 8
  for (i=0; (i<num || num<0) ; i++) 
    {
      if (qtwo)
	head = &qtwo;
      else
	head = &qone;
      if ((*head) == NULL)
	return;
E 8

D 8
    /* delete squares that have the wrong room number, without counting them */
    if (head->room != current_room) {
      remove_request(head);
      i--;
E 8
I 8
      /* delete squares that have the wrong room number, 
	 without counting them */

      if ((*head)->room != current_room) 
	{
	  remove_head(head);
	  i--;
	}
      else
	{
	  /* draw the square requested */
	  switch ((*head)->way) 
	    {
	    case BLACKED:
	      DrawOnSquare(NULL,NULL,(*head)->x,(*head)->y,ESQUARE);
	      break;
	    case CONCEALED:
	    case UNCONCEALED:
	    case EVERYTHING:
	      redraw_player_room_square((*head)->room,(*head)->x,(*head)->y);
	      break;
	    default:
	      Gerror("invalid way given for a draw request");
	      break;
	    }
	  /* remove the request from the queue and array */
	  remove_head(head);
	}
E 8
    }
D 8
    else {
I 3
D 4
      xored[head->x][head->y] = 0;
E 4
E 3
      /* draw the square requested */
      switch (head->way) {
      case BLACKED:
I 2
D 4
	erase_room_square(head->x, head->y);
E 4
I 4
	DrawOnSquare(NULL,NULL,head->x,head->y,ESQUARE);
E 4
	break;
E 2
      case CONCEALED:
      case UNCONCEALED:
      case EVERYTHING:
	redraw_player_room_square(head->room, head->x, head->y);
	break;
      default:
	Gerror("invalid way given for a draw request");
	break;
      }
      /* remove the request from the queue and array */
      remove_request(head);
    }
  }
E 8
}

D 4

I 3
QErase_missile(Missile *m,int roomnum)
{
  /* printf("<- %d  %dx%d\n",xored[m->x][m->y],m->x,m->y); */
  if (xored[m->x][m->y] > 0)
    {
      xored[m->x][m->y]--;
      xor_missile(m,roomnum);
    }
  else
    QDraw_priority_request(roomnum, m->x,m->y, CONCEALED);
}

QDraw_missile(Missile *m,int room)
{
  if (room != current_room)
    return;

  /* printf("-> %d  %dx%d\n",xored[m->x][m->y],m->x,m->y); */
  if (info[(uc) m->type]->armor)
    {
      xored[m->x][m->y]++;
      xor_missile(m,room);
    }
  else
    {
      xored[m->x][m->y] = 0;
      redraw_missile(m,room);
    }
}
E 4
E 3

/* =================== locally used routines follow ====================== */
D 6

E 6
/* delete a request from the queue and from the array */
D 8
remove_request(ptr)
QDraw *ptr;
{
  int found = FALSE;
  QPoint *p, *prior = NULL;
E 8
D 6
  
E 6
I 6

E 6
D 8
  /* remove the reference to the request from the array */
  for (p=req[ptr->x][ptr->y]; (p && !found); p = p->next) {
    if (ptr == p->ptr) {
      found = TRUE;
      if (prior) prior->next = p->next;
      else req[ptr->x][ptr->y] = p->next;
      free(p);
      break;
    }
    prior = p;
  }

  /* now remove the entry in the queue */
  if (last == ptr) last = ptr->prev;
  if (ptr->prev) ptr->prev->next = ptr->next;
  else head = ptr->next;
  if (ptr->next) ptr->next->prev = ptr->prev;
  else tail = ptr->prev;
  free(ptr);
}

I 6
/* extract a request from the queue and from the array */
QDraw *extract_request(ptr)
QDraw *ptr;
E 8
I 8
remove_head(ptr)
QDraw **ptr;
E 8
{
D 8
  int found = FALSE;
  QPoint *p, *prior = NULL;
  
  /* remove the reference to the request from the array */
  for (p=req[ptr->x][ptr->y]; (p && !found); p = p->next) {
    if (ptr == p->ptr) {
      found = TRUE;
      if (prior) prior->next = p->next;
      else req[ptr->x][ptr->y] = p->next;
      free(p);
      break;
    }
    prior = p;
  }
E 8
I 8
  QDraw *tmp;
E 8
E 6

I 6
D 8
  /* now remove the entry in the queue */
  if (last == ptr) last = ptr->prev;
  if (ptr->prev) ptr->prev->next = ptr->next;
  else head = ptr->next;
  if (ptr->next) ptr->next->prev = ptr->prev;
  else tail = ptr->prev;
  return (ptr);
E 8
I 8
  tmp = (*ptr)->next;
  free(*ptr);
  if (*ptr == ttwo)
    ttwo = NULL;
  if (*ptr == tone)
    tone = NULL;
  *ptr = tmp;
E 8
}
E 6

I 6
D 8


E 6
/* look for a request in the array at a given spot, return pointer to one
   found or NULL if there is none like that being searched for */
QDraw *prior_request_in_array(room, x, y)
int room, x, y;	/* square the request will be for */
{
  QDraw *result = NULL;
  QPoint *p;

  for (p = req[x][y]; (p && !result); p = p->next)
    if ((p->ptr->room == room) && (p->ptr->x == x) && (p->ptr->y == y))
      result = p->ptr;

  return result;
}



/* replace the way a request will be drawn */
replace_request(ptr, way)
QDraw *ptr;
int way;	/* draw method: BLACKED, CONCEALED, UNCONCEALED, EVERYTHING */
{
  if (!ptr) Gerror("replace_request called with NULL pointer");
  else ptr->way = way;
}



/* add the pointer to a request into the array */
add_request_to_array(ptr)
QDraw *ptr;
{
  QPoint *new, *p, *prior = NULL;

  if (!ptr) Gerror("NULL pointer given to add_request_to_array");
  
  /* add to end of linked list */
  for (p = req[ptr->x][ptr->y]; p; p = p->next) prior = p;
  new = create_QPoint(ptr);
  if (prior) prior->next = new;
  else req[ptr->x][ptr->y] = new;
}



E 8
/* malloc a QDraw structure and put values into it */
D 6
QDraw *create_QDraw(room, x, y, way)
int room, x, y, way;
E 6
I 6
D 8
QDraw *create_QDraw(room, x, y, way, prio)
int room, x, y, way, prio;
E 8
I 8
QDraw *create_QDraw(room, x, y, way)
int room, x, y, way;
E 8
E 6
{
  QDraw *result;

  result = (QDraw *) malloc(sizeof(QDraw));
  demand(result, "not enough memory for QDraw structure");
  result->room = room;
  result->x = x;
  result->y = y;
  result->way = way;
I 6
D 8
  result->prio = prio;
E 6
  result->prev = NULL;
E 8
  result->next = NULL;

  return result;
}

D 8


/* malloc a QPoint structure and put values into it */
QPoint *create_QPoint(ptr)
QDraw *ptr;
{
  QPoint *result;

  result = (QPoint *) malloc(sizeof(QPoint));
  demand(result, "no memory for QPoint structure");
  result->ptr = ptr;
  result->next = NULL;

  return result;
}
E 8
E 1
