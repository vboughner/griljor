h22313
s 00000/00000/00060
d D 1.3 92/08/07 01:01:32 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00001/00001/00059
d D 1.2 91/12/15 23:07:12 labc-4lc 2 1
c Fixing datagram incompatibilities.   -bloo
e
s 00060/00000/00000
d D 1.1 91/02/16 12:54:14 labc-3id 1 0
c date and time created 91/02/16 12:54:14 by labc-3id
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
D 2
  switch (current->pack->type) {
E 2
I 2
  switch (GetType(current->pack)) {
E 2
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
E 1
