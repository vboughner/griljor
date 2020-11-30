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

/* Packet handling routines for players and monsters */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "map.h"
#include "objects.h"
#include "missile.h"
#include "socket.h"
#include "person.h"



/* Given a person array element number, handle the top packet on the
   queue.  Once the packet is handled, remove it from queue head. */

handle_person_packet_queue(pnum)
int pnum;
{
  QPacket *current;
  
  /* check that this is a valid person number */
  if (pnum < 0 || pnum >= num_persons) return;
  
  /* if there are no packets on the queue, do nothing */
  if (person[pnum]->pqueue == NULL) return;

  /* make a pointer to the live packet */
  if (DEBUG)
    printf("Person id %d handling own packet\n", gameperson[pnum]->id);
  current = person[pnum]->pqueue;

  /* pass on the packet according to its type */
  switch (GetType(current->pack)) {
	case MESSAGE:	general_person_message_handler(pnum, current->pack);
			break;
	default:	break;
  }

  /* remove packet off of queue */
  person[pnum]->pqueue = current->next;
  if (person[pnum]->final == current) person[pnum]->final = NULL;

  /* free the memory used for the queue packet */
  free(current->pack);
  free(current);
}
