/***************************************************************************
 * @(#) serial.h 1.5 - last change made 08/07/92
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

/* definition of stuff for storing information about "reliable" packets
   to be sent using the datagram socket stuff in socket.c */

#define NON_RELIABLE	0	/* what to set serial number to for non-
				   reliable packet sendings */
#define FIRST_SERIAL	1	/* first usable serial number */
#define LAST_SERIAL	32000	/* highest permissible serial number to be
				   used before rolling back to the first one */
#define RESEND_ATTEMPTS	5	/* number of resends to be made before
				   giving up on sending the packet */
#define RESEND_INTERVAL	3000	/* number of clicks to wait for a response
				   before resending a packet */


typedef struct _reliable {
	short resends;		/* number of times you have resent it */
	Sockaddr *address;	/* socket address to resend the packet to */
	BossPacket *packet;	/* pointer to a copy of the packet */
} ReliablePacket;
