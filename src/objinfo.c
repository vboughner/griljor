/***************************************************************************
 * @(#) objinfo.c 1.3 - last change made 08/07/92
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

/* File containing the routines that allow setting of the name, author, and
   code filenames for object sets in obtor. */

#include <stdio.h>
#include <string.h>
#include "vline.h"
#include "objinfo.h"
#include "objheader.h"



ObjHeader *new_objheader()
{
  ObjHeader *new = (ObjHeader *) malloc(sizeof(ObjHeader));
  demand(new, "no memory for new object header");

  bzero(new, sizeof(ObjHeader));
  return new;
}



void free_objheader(header)
ObjHeader *header;
{
  if (header) {
    if (header->name) free(header->name);
    if (header->author) free(header->author);
    if (header->codefile) free(header->codefile);
    free(header);
  }
}


/* ================ H E A D E R  C O N V E R S I O N ================= */

ObjHeader *extract_header_from_messages(msgs)
/* looks through the messages at the beginning of an object set file
   for header items and places copies of them in a newly formed header
   structure.  A pointer to the newly allocated structure is returned
   (whether or not there where actually any header items in msgs.) */
VLine *msgs;
{
  ObjHeader *result = new_objheader();
  result->name = os_extract_name(msgs);
  result->author = os_extract_author(msgs);
  result->codefile = os_extract_codefilename(msgs);
  return result;
}



void insert_header_into_messages(header, msgs)
/* puts non-null header items into the message list for later storage
   in the object definition file.  Will create msgs if there aren't
   any at the moment. */
ObjHeader *header;
VLine **msgs;
{
  if (header) {
    *msgs = os_set_name(*msgs, header->name);
    *msgs = os_set_author(*msgs, header->author);
    *msgs = os_set_codefilename(*msgs, header->codefile);
  }
}
