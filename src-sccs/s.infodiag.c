h29119
s 00000/00000/00264
d D 1.10 92/08/07 01:01:49 vbo 10 9
c source copied to a separate tree for work on new map and object format
e
s 00066/00000/00198
d D 1.9 91/12/05 12:09:21 labc-4lc 9 8
c fixed objheader stuff to compile with editmap
e
s 00030/00011/00168
d D 1.8 91/09/15 23:33:33 labc-4lc 8 7
c made mapinfo dialog editing window work
e
s 00028/00000/00151
d D 1.7 91/09/15 20:53:13 labc-4lc 7 6
c made dialog.c a little more generic, changed obtor to deal with it
e
s 00002/00003/00149
d D 1.6 91/09/05 21:28:37 labb-3li 6 5
c made obtor use the new objprops.c code for dialog lists
e
s 00002/00002/00150
d D 1.5 91/07/14 14:01:51 labc-3id 5 4
c Added 'ObjDiag.t' code.
e
s 00001/00001/00151
d D 1.4 91/06/30 20:27:45 labc-3id 4 3
c fixed up editing order of properties for obtor
e
s 00022/00000/00130
d D 1.3 91/05/17 02:01:38 labc-3id 3 2
c Added code for showing object numbers using '#'
e
s 00016/00011/00114
d D 1.2 91/05/03 23:10:52 labc-3id 2 1
c 
e
s 00125/00000/00000
d D 1.1 91/03/24 18:45:04 labc-3id 1 0
c date and time created 91/03/24 18:45:04 by labc-3id
e
u
U
f e 0
t
T
I 7
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

/* this file contains the code that calls up a dialog window for editing
   an object with the object editing program. */

E 7
I 1
#include "obtor.h"

/*****************************************************************/

I 3
InfoEditHook(Dialog *diag,XEvent *event)
{
  char buf[20];
  KeySym	keysym;
  int nchar;
  
  nchar = XLookupString((XKeyEvent *) event, buf, 19, &keysym, 
			(XComposeStatus *) NULL);
  
  if (nchar == 1)
    switch(buf[0])
      {
      case '#':
	SetShowNumbers(!show_numbers);
      }

  return(False);
}

/*****************************************************************/

E 3
D 8
/* ============================ I N F O requests ========================= */
E 8
I 8
int linkpic_hook(diag,event,str)
Dialog *diag;
XEvent *event;
char *str;
{
  int group,num;
E 8

D 8
/* handle a request for information on a square, results depend on which
   window it is called from.  If there is already an info window up, this
   command puts it away. */
E 8
I 8
  if ((diag->win == event->xany.window)||(event->type != ButtonPress))
    return(False);
  
  group = GetDialogMark(diag,0);
  if (group != FindObtorWindow(event->xany.window))
    return(False);
  
  num = obtor_obj_in_grab_win(event->xbutton.x,event->xbutton.y,group);
  if (IS_VALID_OBJ(group, num))
    {
      sprintf(str,"%d",num);
      return(True);
    }
  
  return(False);
}
E 8

D 8
handle_obtor_info_request(win, x, y)
     Window win;
     int x, y;
E 8
I 8
/*****************************************************************/

handle_obtor_info_request()
/* handle a request for editing the properties of an object */
E 8
{
D 5
#include "ObjDiag.h"   /* special type of include file! */
E 5
I 5
D 6
#include "ObjDiag.t"   /* special type of include file! */
E 6
I 6
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 6
E 5
  int newx, newy;	
  int winnum,which;
  Dialog *info_rec;
  
  winnum = FindObjToEdit(&which);
  if (winnum < 0)
    return;
  
  if (IS_VALID_OBJ(winnum,which))
    {
      newx = 10;
      newy = 10;
      XDefineCursor(display,obtorwin,WatchCursor);
D 4
      info_rec = make_dialog(rootwin,newx,newy,HELP_FONT,ObjectData);
E 4
I 4
      info_rec = make_dialog(rootwin,newx,newy,DIAG_FONT,ObjectData);
E 4
      XSetWindowBackgroundPixmap(display,info_rec->win,info_pix);
      XStoreName(display,info_rec->win,"Obtor Info Edit");
      SetDialogMark(info_rec,0,winnum);
      SetDialogMark(info_rec,1,which);
      SetDialogData(info_rec,objdef[winnum][which]);
I 3
      SetDialogKeyHook(info_rec,InfoEditHook);
I 7
      SetDialogPixList(info_rec, item_pixmap[winnum], num_objects[winnum]);
      SetDialogWindowList(info_rec, objwin);
      SetDialogHandCursor(info_rec, HandCursor);
      SetDialogWatchCursor(info_rec, WatchCursor);
E 7
E 3
      DialogProcess(info_rec);
      XDefineCursor(display,obtorwin,ArrowCursor);
    }
}

/*****************************************************************/
/* hook procedure for the object editor */

SaveObject(rec,item,event)
D 8
int item;
E 8
Dialog *rec;
I 8
int item;
E 8
XEvent *event;
{
  int winnum,which;

  winnum = GetDialogMark(rec,0);
  which = GetDialogMark(rec,1);

  GetDialogData(rec,objdef[winnum][which]);

  if (rec->changed)
    {
      FileChanged(winnum,TRUE);
      certain_object_pixmap(winnum,which);
      redraw_grab_square(winnum,which,False);
    }

  return(FALSE);
}

/*****************************************************************/
/* hook procedure for the object editor */

QuitObject(rec,item,event)
D 8
int item;
E 8
Dialog *rec;
I 8
int item;
E 8
XEvent *event;
{
  int ret;

  if (!rec->changed) 
    return(FALSE);

  ret = placed_letter_query(rec->win, 0, 0, rec->width, rec->height
			    ,"Save Changes?\n(y/n/c)", "ync");

  if (ret == 'y')
D 8
      return(SaveObject(rec,event));
E 8
I 8
      return(SaveObject(rec, item, event));
E 8

  return(ret == 'c');
}

/*****************************************************************/

AutoMask(rec,item,event)
D 8
int item;
E 8
Dialog *rec;
I 8
int item;
E 8
XEvent *event;
{
D 5
#include "ObjDiag.h"
E 5
I 5
D 6
#include "ObjDiag.t"
E 5

E 6
D 2
  int group,num;
E 2
I 2
  int group,num,field;
E 2
  char data[BITMAP_ARRAY_SIZE];
  char new[BITMAP_ARRAY_SIZE];

I 2

E 2
  group = GetDialogMark(rec,0);
  num = GetDialogMark(rec,1);

D 2
  TransferPixmapToData((Pixmap) rec->ent[BITMAPFIELD].value,
E 2
I 2
  field = FindNextData(rec->list,-1,BITMAPFIELD);
  TransferPixmapToData((Pixmap) rec->ent[field].value,
E 2
		       data);
  
  make_mask(data,new);

D 2
  if (rec->ent[MASKFIELD].value)
    XFreePixmap(display,rec->ent[MASKFIELD].value);
E 2
I 2
  field = FindNextData(rec->list,-1,MASKFIELD);
  if (rec->ent[field].value)
    XFreePixmap(display,rec->ent[field].value);
E 2
  
D 2
  rec->ent[MASKFIELD].value = 
    XCreatePixmapFromBitmapData(display, rootwin,
				new, BITMAP_WIDTH,
E 2
I 2

  rec->ent[field].value = 
    XCreatePixmapFromBitmapData(display, rootwin,new, BITMAP_WIDTH,
E 2
				BITMAP_HEIGHT, fgcolor, bgcolor, 1);
I 2
  DrawDialogItem(rec,field);
E 2
  
D 2
  rec->ent[MASKEDFIELD].value = 1;
  rec->changed = True;
  DrawDialogItem(rec,MASKEDFIELD);
  DrawDialogItem(rec,MASKFIELD);
E 2

I 2
  field = FindNextData(rec->list,-1,MASKEDFIELD);
  rec->ent[field].value = 1;
  DrawDialogItem(rec,field);

  rec->changed = True;
E 2
  return(True);
}

/*****************************************************************/

I 9
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
E 9
E 1
