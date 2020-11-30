h52717
s 00000/00000/00040
d D 1.5 92/08/07 01:04:45 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00000/00000/00040
d D 1.4 91/09/10 21:41:01 labc-4lc 4 3
c 
e
s 00001/00004/00039
d D 1.3 91/09/10 13:31:49 labc-4lc 3 2
c 
e
s 00002/00001/00041
d D 1.2 91/09/06 22:42:42 labb-3li 2 1
c 
e
s 00042/00000/00000
d D 1.1 91/09/05 22:46:30 labb-3li 1 0
c date and time created 91/09/05 22:46:30 by labb-3li
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
D 2
	short serial;		/* serial number of packet when you sent it */
E 2
D 3
	long lastsent;		/* game time (in clicks) it was last sent */
E 3
	short resends;		/* number of times you have resent it */
I 2
D 3
	short serial;		/* serial number of packet when you sent it */
E 2
	BossPacket *packet;	/* pointer to a copy of the packet */
E 3
	Sockaddr *address;	/* socket address to resend the packet to */
I 2
D 3
	struct _reliable *parent, *left, *right;  /* heap helpers */
E 3
I 3
	BossPacket *packet;	/* pointer to a copy of the packet */
E 3
E 2
} ReliablePacket;
E 1
