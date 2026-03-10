/***************************************************************************
 * @(#) infodiag.c 1.10 - last change made 08/07/92
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

#include "obtor.h"

/*****************************************************************/

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

int linkpic_hook(diag,event,str)
Dialog *diag;
XEvent *event;
char *str;
{
  int group,num;

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

/*****************************************************************/

handle_obtor_info_request()
/* handle a request for editing the properties of an object */
{
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
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
      info_rec = make_dialog(rootwin,newx,newy,DIAG_FONT,ObjectData);
      XSetWindowBackgroundPixmap(display,info_rec->win,info_pix);
      XStoreName(display,info_rec->win,"Obtor Info Edit");
      SetDialogMark(info_rec,0,winnum);
      SetDialogMark(info_rec,1,which);
      SetDialogData(info_rec,objdef[winnum][which]);
      SetDialogKeyHook(info_rec,InfoEditHook);
      SetDialogPixList(info_rec, item_pixmap[winnum], num_objects[winnum]);
      SetDialogWindowList(info_rec, objwin);
      SetDialogHandCursor(info_rec, HandCursor);
      SetDialogWatchCursor(info_rec, WatchCursor);
      DialogProcess(info_rec);
      XDefineCursor(display,obtorwin,ArrowCursor);
    }
}

/*****************************************************************/
/* hook procedure for the object editor */

SaveObject(rec,item,event)
Dialog *rec;
int item;
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
      return(SaveObject(rec, item, event));

  return(ret == 'c');
}

/*****************************************************************/

AutoMask(rec,item,event)
Dialog *rec;
int item;
XEvent *event;
{
  int group,num,field;
  char data[BITMAP_ARRAY_SIZE];
  char new[BITMAP_ARRAY_SIZE];


  group = GetDialogMark(rec,0);
  num = GetDialogMark(rec,1);

  field = FindNextData(rec->list,-1,BITMAPFIELD);
  TransferPixmapToData((Pixmap) rec->ent[field].value,
		       data);
  
  make_mask(data,new);

  field = FindNextData(rec->list,-1,MASKFIELD);
  if (rec->ent[field].value)
    XFreePixmap(display,rec->ent[field].value);
  

  rec->ent[field].value = 
    XCreatePixmapFromBitmapData(display, rootwin,new, BITMAP_WIDTH,
				BITMAP_HEIGHT, fgcolor, bgcolor, 1);
  DrawDialogItem(rec,field);
  

  field = FindNextData(rec->list,-1,MASKEDFIELD);
  rec->ent[field].value = 1;
  DrawDialogItem(rec,field);

  rec->changed = True;
  return(True);
}

/*****************************************************************/

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
