h15578
s 00000/00000/00120
d D 1.5 92/08/07 01:04:47 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00000/00000/00120
d D 1.4 91/09/10 21:41:19 labc-4lc 4 3
c 
e
s 00058/00004/00062
d D 1.3 91/09/10 13:31:56 labc-4lc 3 2
c 
e
s 00005/00002/00061
d D 1.2 91/09/06 22:42:45 labb-3li 2 1
c 
e
s 00063/00000/00000
d D 1.1 91/09/05 22:46:37 labb-3li 1 0
c date and time created 91/09/05 22:46:37 by labb-3li
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

D 2
/* code to make and maintain a binary tree of the packets sent with a
   reliability response requested. */
E 2
I 2
/* code to make and maintain a heap of the packets sent with a
   reliability response requested.  The heap is created so that the item
   that is the oldest (longest since it was last sent) is at the top.
   Thus when a search is made for packets that need resending, it will be
   be fairly efficient. */
E 2

#include <stdio.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
I 3
#include "heap.h"
E 3
#include "serial.h"


D 3
void AddSerial(packet, address, sendtime)
E 3
I 3
/* the heap pointer we'll use to refer to heap that stores reliable packets */
static HeapInfo *serialheap = NULL;
#define INITIAL_SERIAL_HEAP_SIZE 200


/* the keeper of the current serial number */
static int next_serial = FIRST_SERIAL;



int NextSerialNumber()
/* returns a unique serial number useable by the caller for installation into
   the BossPacket record before sending the packet off */
{
  int result = next_serial;
  next_serial++;
  if (next_serial > LAST_SERIAL) next_serial = FIRST_SERIAL;
  return result;
}



void FreeSerial(serial)
/* frees the things inside a serial record and the serial itself */
ReliablePacket *serial;
{
  if (serial) {
    if (serial->address) free(serial->address);
    if (serial->packet) free(serial->packet);
    free(serial);
  }
}



void AddSerial(packet, sendtime)
E 3
/* adds a packet to the serial reliability tree, marking it with the
D 3
   given time.  A copy of the packet is made, so that the caller can
   free his own copy. */
E 3
I 3
   given time.  A copy of the packet and the address is made, so that the
   caller can free his own copies. */
E 3
BossPacket *packet;
Sockaddr *address;
long sendtime;
{
I 3
  ReliablePacket *relrec;
  BossPacket *pktcopy;
  Sockaddr *addrcopy;

  if (!serialheap)
    serialheap = InitializeHeap(INITIAL_SERIAL_HEAP_SIZE, FALSE, FreeSerial);

  relrec = (ReliablePacket *) malloc(sizeof(ReliablePacket));
  demand(relrec, "not enough memory for reliable packet");
  pktcopy = (BossPacket *) malloc(sizeof(BossPacket));
  demand(pktcopy, "not enough memory for packet copy");
  addrcopy = (Sockaddr *) malloc(sizeof(Sockaddr));
  demand(addrcopy, "not enough memory for address copy");

  relrec->resends = 0;
  relrec->address = addrcopy;
  relrec->packet = pktcopy;
  AddToHeap(serialheap, sendtime, (void *)relrec);
E 3
}



void RemoveSerial(num)
/* tells us that the response to a serial numbered packet has come in
   and we can delete the reference to it in the reliability tree. */
short num;
{
}



void CheckSerials(gametime)
/* gets us to look through the tree for packets that need to be resent,
D 3
   this routine should be called only occasionally (say, every 2 seconds).
E 3
I 3
   this routine need only be called occasionally (say, every 2 seconds).
E 3
   It will call for the resending or deletion of older packets in the tree. */
long gametime;
{
}
E 1
