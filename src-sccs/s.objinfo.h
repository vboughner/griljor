h42242
s 00000/00000/00033
d D 1.2 92/08/07 01:04:55 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00033/00000/00000
d D 1.1 91/12/03 17:32:36 labc-4lc 1 0
c date and time created 91/12/03 17:32:36 by labc-4lc
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

/* Header for object file header information */


typedef struct _objheader {
	char *name;	/* name of the object definition set */
	char *author;	/* name of the author of the object set */
	char *codefile;	/* name of the file that code for objects is in */
} ObjHeader;


void free_objheader(), insert_header_into_messages();
ObjHeader *new_objheader(), *extract_header_from_messages();
E 1
