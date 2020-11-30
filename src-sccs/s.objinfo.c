h45903
s 00000/00000/00086
d D 1.3 92/08/07 01:04:54 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00004/00074/00082
d D 1.2 91/12/05 12:09:30 labc-4lc 2 1
c fixed objheader stuff to compile with editmap
e
s 00156/00000/00000
d D 1.1 91/12/03 17:32:33 labc-4lc 1 0
c date and time created 91/12/03 17:32:33 by labc-4lc
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

/* File containing the routines that allow setting of the name, author, and
   code filenames for object sets in obtor. */

#include <stdio.h>
#include <string.h>
D 2
#include "externX11.h"
#include "obtor.h"
E 2
I 2
#include "vline.h"
#include "objinfo.h"
#include "objheader.h"
E 2


I 2

E 2
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
D 2



/* ================ D I A L O G  S T U F F ============================ */

void handle_header_edit_request()
/* handle a request to edit the header information for a particular object
   set, by calling up a top level window and allowing user to
   edit the fields of an object set's header components. */
{
  DialogList *StaticHeaderProperties();
  DialogList *HeaderData = StaticHeaderProperties();
  Dialog *info_rec;
  int newx = 10, newy = 10, winnum, which;

  winnum = FindObjToEdit(&which);
  if (winnum < 0) return;

  XDefineCursor(display,obtorwin,WatchCursor);
  info_rec = make_dialog(rootwin,newx,newy,DIAG_FONT,HeaderData);
  XSetWindowBackgroundPixmap(display,info_rec->win,info_pix);
  XStoreName(display,info_rec->win,"Object Set Header");
  SetDialogMark(info_rec,0,winnum);
  SetDialogData(info_rec,objheader[winnum]);
  SetDialogHandCursor(info_rec, HandCursor);
  SetDialogWatchCursor(info_rec, WatchCursor);
  DialogProcess(info_rec);
  XDefineCursor(display,obtorwin,ArrowCursor);
}



EditHeaderSave(rec,item,event)
/* is called when used presses 'save changes' button after editing fields */
Dialog *rec;
int item;
XEvent *event;
{
  int winnum;

  winnum = GetDialogMark(rec,0);
  GetDialogData(rec, objheader[winnum]);

  if (rec->changed) {
     FileChanged(winnum,TRUE);
  }

  return(FALSE);
}



EditHeaderQuit(rec,item,event)
/* is called when used presses 'quit' button after editing fields */
Dialog *rec;
int item;
XEvent *event;
{
  int ret;

  if (!rec->changed) 
    return(FALSE);

  ret = placed_letter_query(rec->win, 0, 0, rec->width, rec->height
			    ,"Save Changes?\n(y/n/c)", "ync");

  if (ret == 'y')
      return(EditHeaderSave(rec, item, event));

  return(ret == 'c');
}

E 2
E 1
