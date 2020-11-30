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
#include "queued.h"
#include "windowsX11.h"
#include "missile.h"
#include "roomdraw.h"

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


/* globals in this file for handling the ordered queue */

static NonQueuedDraws;          /* number of draws not using this package */
static QDraw	*qone = NULL;	/* low priority */
static QDraw    *tone = NULL;
static QDraw	*qtwo = NULL;	/* high priority */
static QDraw    *ttwo = NULL;

/* other variables */

/* initialize the two-dimensional array to be empty */
QDraw_initialize()
{
  int i, j;

  /* free previously created requests if any */
  while (qone)
    remove_head(&qone);
  while (qtwo)
    remove_head(&qtwo);

  NonQueuedDraws = 0;
}


/* enter a new draw request into the queue */
QDraw_request(room, x, y, way)
int room, x, y;	/* location of square to draw */
int way;	/* draw method: BLACKED, CONCEALED, UNCONCEALED, EVERYTHING */
{
  QDraw *ptr;

  /* look for out of bounds request */
  if (x<0 || y<0 || x>=ROOM_WIDTH || y>=ROOM_HEIGHT) return;

  ptr = create_QDraw(room, x, y, way);

  if (tone)
    {
      tone->next = ptr;
      tone = ptr;
    }
  else
    {
      tone = ptr;
      qone = ptr;
    }
}


/* enter a new draw request with high priority.  It will be placed near the
   head of the queue, after the other previously entered priority requests */
QDraw_priority_request(room, x, y, way)
int room, x, y;	/* location of square to draw */
int way;	/* draw method: BLACKED, CONCEALED, UNCONCEALED, EVERYTHING */
{
  QDraw *ptr;

  /* look for out of bounds request */
  if (x<0 || y<0 || x>=ROOM_WIDTH || y>=ROOM_HEIGHT) return;

  ptr = create_QDraw(room, x, y, way);

  if (ttwo)
    {
      ttwo->next = ptr;
      ttwo = ptr;
    }
  else
    {
      ttwo = ptr;
      qtwo = ptr;
    }
}

/*****************************************************************/

QDraw_notify()
{
  NonQueuedDraws++;
}

/* grab the next few things in the queue and draw them.  This routine should
   be called once in a while to insure that things are eventually drawn. */
QDraw_cycle(num)
int num;	/* how many things to call from the queue, a negative
		   number signifies getting everything currently in it */
{
  int i, cease;
  QDraw **head;

  /* check to see how much non-queued drawing has been done */
  if (NonQueuedDraws > num)
    NonQueuedDraws = num;

  num -= NonQueuedDraws;
  NonQueuedDraws = 0;

  for (i=0; (i<num || num<0) ; i++) 
    {
      if (qtwo)
	head = &qtwo;
      else
	head = &qone;
      if ((*head) == NULL)
	return;

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
    }
}


/* =================== locally used routines follow ====================== */
/* delete a request from the queue and from the array */

remove_head(ptr)
QDraw **ptr;
{
  QDraw *tmp;

  tmp = (*ptr)->next;
  free(*ptr);
  if (*ptr == ttwo)
    ttwo = NULL;
  if (*ptr == tone)
    tone = NULL;
  *ptr = tmp;
}

/* malloc a QDraw structure and put values into it */
QDraw *create_QDraw(room, x, y, way)
int room, x, y, way;
{
  QDraw *result;

  result = (QDraw *) malloc(sizeof(QDraw));
  demand(result, "not enough memory for QDraw structure");
  result->room = room;
  result->x = x;
  result->y = y;
  result->way = way;
  result->next = NULL;

  return result;
}

