/***************************************************************************
 * @(#) objstore.h 1.2 - last change made 08/07/92
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
