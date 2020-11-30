/***************************************************************************
 * @(#) serial.c 1.5 - last change made 08/07/92
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

/* code to make and maintain a heap of the packets sent with a
   reliability response requested.  The heap is created so that the item
   that is the oldest (longest since it was last sent) is at the top.
   Thus when a search is made for packets that need resending, it will be
   be fairly efficient. */

#include <stdio.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "heap.h"
#include "serial.h"


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
/* adds a packet to the serial reliability tree, marking it with the
   given time.  A copy of the packet and the address is made, so that the
   caller can free his own copies. */
BossPacket *packet;
Sockaddr *address;
long sendtime;
{
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
}



void RemoveSerial(num)
/* tells us that the response to a serial numbered packet has come in
   and we can delete the reference to it in the reliability tree. */
short num;
{
}



void CheckSerials(gametime)
/* gets us to look through the tree for packets that need to be resent,
   this routine need only be called occasionally (say, every 2 seconds).
   It will call for the resending or deletion of older packets in the tree. */
long gametime;
{
}
