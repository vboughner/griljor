h30078
s 00000/00000/00029
d D 1.2 92/08/07 01:02:54 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00029/00000/00000
d D 1.1 91/07/04 14:02:11 labc-3id 1 0
c date and time created 91/07/04 14:02:11 by labc-3id
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

/* Prototypes for object storage routines */

void write_messages_to_fd();
void write_objects_to_fd();
int write_objects_to_file();
VLine *read_messages_from_fd();
ObjInfo **read_objects_from_fd();
ObjInfo **read_objects_from_file();
E 1
