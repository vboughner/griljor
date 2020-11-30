/***************************************************************************
 * @(#) objinfo.h 1.2 - last change made 08/07/92
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
