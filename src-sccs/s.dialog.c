h61698
s 00009/00002/00939
d D 1.17 92/08/27 20:08:31 vbo 17 16
c fixed a discrepancy between obtor and editmap use of dialogs
e
s 00012/00003/00929
d D 1.16 92/08/14 18:37:07 vbo 16 15
c added a check for NULL string pointer in several places
e
s 00000/00000/00932
d D 1.15 92/08/07 21:33:09 vbo 15 14
c fixes for sun port merged in
e
s 00024/00010/00908
d D 1.14 92/03/02 22:06:33 vanb 14 13
c finished getting editmap to work on basics
e
s 00005/00002/00913
d D 1.13 92/02/15 01:09:25 vanb 13 12
c 
e
s 00003/00003/00912
d D 1.12 91/12/13 23:21:31 labc-4lc 12 11
c added and deleted a lot of object properties
e
s 00004/00000/00911
d D 1.11 91/12/13 19:33:34 labc-4lc 11 10
c fixed bug that made a completely blank object unreadable and made it possible to move object properties into an old/outdated state
e
s 00018/00002/00893
d D 1.10 91/11/28 00:10:12 labc-4lc 10 9
c nearly finished hidden text items in dialogs
e
s 00020/00002/00875
d D 1.9 91/11/27 15:07:00 labc-4lc 9 8
c changed message properties to hidden strings
e
s 00017/00003/00860
d D 1.8 91/11/27 13:41:03 labc-4lc 8 7
c fixed string in objects so space is malloced, not static
e
s 00005/00005/00858
d D 1.7 91/10/16 20:05:21 labc-4lc 7 6
c changed made for DecStations
e
s 00005/00028/00858
d D 1.6 91/09/15 23:33:16 labc-4lc 6 5
c made mapinfo dialog editing window work
e
s 00149/00014/00737
d D 1.5 91/09/15 20:53:04 labc-4lc 5 4
c made dialog.c a little more generic, changed obtor to deal with it
e
s 00001/00000/00750
d D 1.4 91/09/15 16:14:14 labc-4lc 4 3
c 
e
s 00015/00000/00735
d D 1.3 91/05/17 02:01:25 labc-3id 3 2
c Added code for showing object numbers using '#'
e
s 00017/00000/00718
d D 1.2 91/05/03 23:10:33 labc-3id 2 1
c 
e
s 00718/00000/00000
d D 1.1 91/03/24 18:44:32 labc-3id 1 0
c date and time created 91/03/24 18:44:32 by labc-3id
e
u
U
f e 0
t
T
I 1
D 5
#include "obtor.h"
E 5
I 5
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
E 5

I 5
/* this file is used by both the obtor and editmap programs for displaying
   and editing the properties of objects, maps, and rooms.  The various
   different fields in the structure that defines an object or map or room
   are referred to by their offsets.  These offsets, and the kinds of
   variables that they represent are defined in objprops.c 
   The routines in this file use the info about the types of the variables to
   create a window full of little 'boxes' that allow you to edit the value
   in one of the structure fields. */
E 5
I 4

E 4
D 5
/*****************************************************************/
  
E 5
I 5
#include "windowsX11.h"
#include <X11/cursorfont.h>
#include "def.h"
#include "dialog.h"
E 5
D 10
char *get_string_hook();
E 10
I 10
char *get_string_hook(), *display_edit_string();
E 10
I 6
int linkpic_hook();
E 6

I 5
D 6

E 6
E 5
/*****************************************************************/

Dialog *make_dialog(basewin,x,y,fi,data)
I 5
/* creates a dialog window which will contain various subfields and areas
   for use in editing the properties of whatever it is that we are editing. */
E 5
Window basewin;
int x,y;
XFontStruct *fi;
DialogList data[];
{
  Window diagwin;
  Dialog *diag;
  int count;

  for (count = 0;data[count].name != NULL;count++)
    ;  /* count the number of entries */

  diag = (Dialog *) malloc(sizeof(Dialog));
  if (diag == NULL) exit(25);
I 14
  diag->readonly = False;
E 14
  diag->changed = False;
  diag->list = data;
  diag->count = count;
  diag->fi = fi;
  diag->hook = NULL;
I 3
  diag->key = NULL;
I 5
  diag->pixlist = NULL;
  diag->pixlistsize = 0;
  diag->handcursor = NULL;
  diag->watchcursor = NULL;
  diag->windowlist = NULL;
E 5
E 3
  diag->ent = (DialogEntry *) malloc(sizeof(DialogEntry)*count);
  if (diag->ent == NULL) exit(26);

  ParseDialog(diag);

  diag->win = diagwin = XCreateSimpleWindow(display, basewin,x,y,
					    diag->width,diag->height,
D 7
					    1,fgcolor,bgcolor);
E 7
I 7
				            1,fgcolor,bgcolor);
E 7

D 5
  XDefineCursor(display,diagwin,HandCursor);
E 5
  XSelectInput(display, diagwin,KeyPressMask | ButtonPressMask | ExposureMask);

  XMapRaised(display,diag->win);
  return(diag);
}

/*****************************************************************/

ParseDialog(diag)
I 5
/* takes a dialoglist (a list of object/map/room properties) and forms
   a number of fields in the window for editing it */
E 5
Dialog *diag;
{
  int i;
  DialogList *list;
  DialogEntry *ent;
  int x,y,w,h,maxy;
  int rowheight,colwidth,onewidth;

  onewidth = diag->fi->max_bounds.rbearing - diag->fi->min_bounds.lbearing;
  colwidth = COLWIDTH*onewidth;
  rowheight = diag->fi->ascent + diag->fi->descent;
  diag->width = 0;
  diag->height = 0;

  x = COLPAD;
  y = COLPAD;
  maxy = 0;
  for (i = 0;i < diag->count;i++)
    {
      list = &(diag->list[i]);
      ent = &(diag->ent[i]);
      ent->value = NULL;  /* default */

      switch(list->type)
	{
	case NEWCOL:
	  x += colwidth+COLPAD;
	  y = COLPAD;
D 12
	  w = 0;
E 12
I 12
	  w = 1;
E 12
	  h = 0;
	  break;
	case BLANKBOX:
	case BOLBOX:
	case INTBOX:
	case TITLEBOX:
	case STRBOX:
I 9
        case HIDSTR:
E 9
	case QUITBOX:
	case MARKSHO:
	  w = ((strlen(list->name)+4)*onewidth + colwidth - 1)/colwidth;
	  h = 1;
	  break;
	case LSTRBOX:
	  w = 2;
	  h = 1;
	  break;
	case MARKHID:
D 12
	  w = 0;
E 12
I 12
	  w = 1;
E 12
	  h = 0;
	  break;
	case LINKPIC:
	  w = (BITMAP_WIDTH + BITPAD*2 + colwidth -1 )/colwidth;
	  h = (BITMAP_HEIGHT + rowheight - 1 + BITPAD*3)/rowheight + 1;
	  break;
	case ICONPIC:
	  w = ((strlen(list->name)+2)*onewidth +
	       BITMAP_WIDTH + colwidth - 1+BITPAD*2)/colwidth;
	  h = (BITMAP_HEIGHT + rowheight - 1 + BITPAD *2)/rowheight;
	  break;
I 11
	default:
D 12
	  w = 0;
E 12
I 12
	  w = 1;
E 12
	  h = 0;
	  break;
E 11
	}

      if (h)
	{
	  ent->x1 = x;
	  ent->y1 = y;
	  ent->x2 = x + colwidth*w + COLPAD*(w-1);
	  y += rowheight*h;
	  ent->y2 = y; 
	  maxy = (y > maxy ? y : maxy);
	}
      else
	{
	  ent->x1 = 0;
	  ent->y1 = 0;
	  ent->x2 = 0;
	  ent->y2 = 0;
	}
    }
  diag->width = x + colwidth*w + COLPAD;
  diag->height = maxy + COLPAD;
}

/*****************************************************************/

SetDialogData(diag,data)
Dialog *diag;
char *data;
{
  int i;
 
  for (i = 0; i < diag->count;i++)
    CopyDiagData(diag,data,i);
}

/*****************************************************************/

CopyDiagData(diag,data,i)
int i;
Dialog *diag;
char *data;
{
  DialogList *list;
  DialogEntry *ent;
  
  list = &(diag->list[i]);
  ent = &(diag->ent[i]);
  
  switch(list->type)
    {
    case BOLBOX:
      ent->value = *((char *) (data + list->offset));
I 2
      if (list->size != sizeof(char))
	printf("PANIC!  Size mismatch for field %d\n",i);
E 2
      break;
    case INTBOX:
      ent->value = *((short *) (data + list->offset));
I 2
      if (list->size != sizeof(short))
	printf("PANIC!  Size mismatch for field %d\n",i);
E 2
      break; 
    case LINKPIC:
      ent->value = *((short *) (data + list->offset));
I 2
      if (list->size != sizeof(short))
	printf("PANIC!  Size mismatch for field %d\n",i);
E 2
      break; 
    case LSTRBOX:
    case STRBOX:
I 9
    case HIDSTR:
E 9
D 8
      ent->value = malloc(STRMAXSIZE);
D 7
      strcpy(ent->value,(data + list->offset));
E 7
I 7
      strcpy((char *)ent->value,(data + list->offset));
E 8
I 8
      if (*((char **) (data + list->offset))) {
	char **tmp = (char **) (data + list->offset);
        demand((ent->value = malloc(strlen(*tmp) + 1)),
	       "no memory for string in dialog copy");
        strcpy((char *)ent->value, *tmp);
      }
      else ent->value = 0;
E 8
E 7
      break;
    case ICONPIC:
      if (ent->value)
	XFreePixmap(display,ent->value);

      ent->value =
	XCreatePixmapFromBitmapData(display,diag->win,
				    (data + list->offset),
				    BITMAP_WIDTH,BITMAP_HEIGHT,
D 7
				    fgcolor,bgcolor,1);
E 7
I 7
				    fgcolor,bgcolor,depth);
E 7
      break;
    }
}

/*****************************************************************/

GetDialogData(diag,data)
Dialog *diag;
char *data;
{
  DialogList *list;
  DialogEntry *ent;
  int i;
 
  for (i = 0; i < diag->count;i++)
    {
      list = &(diag->list[i]);
      ent = &(diag->ent[i]);
      
      switch(list->type)
	{
	case BOLBOX:
	  *((char *) (data + list->offset)) = (char) ent->value;
	  break;
	case INTBOX:
	  *((short *) (data + list->offset)) = (short) ent->value;
	  break; 
	case LINKPIC:
	  *((short *) (data + list->offset)) = (short) ent->value;
	  break; 
	case LSTRBOX:
	case STRBOX:
I 9
	case HIDSTR:
E 9
D 7
	  strcpy((data + list->offset),ent->value);
E 7
I 7
D 8
	  strcpy((data + list->offset),(char *)ent->value);
E 8
I 8
          if (ent->value) {
	    char **tmp = (char **) (data + list->offset);
	    if (*tmp) free(*tmp);
            demand(*tmp = (char *)malloc(strlen((char *)ent->value)+1),
		   "no memory to copy string back");
            strcpy(*tmp, (char *) ent->value);
	  }
E 8
E 7
	  break;
	case ICONPIC:
	  TransferPixmapToData(ent->value,data+list->offset);
	  break;
	}
    }
}

/*****************************************************************/

SetDialogMark(diag,which,mark)
Dialog *diag;
int mark,which;
{
  DialogList *list;
  DialogEntry *ent;
  int i;

  for (i = 0;i < diag->count;i++)
    {
      list = &(diag->list[i]);
      ent = &(diag->ent[i]);

      if (((list->type == MARKSHO)||(list->type == MARKHID))&&
	  (list->offset == which))
	ent->value = mark;
    }
}

/*****************************************************************/

GetDialogMark(diag,which)
Dialog *diag;
int which;
{
  DialogList *list;
  DialogEntry *ent;
  int i;

  for (i = 0;i < diag->count;i++)
    {
      list = &(diag->list[i]);
      ent = &(diag->ent[i]);

      if (((list->type == MARKSHO)||(list->type == MARKHID))&&
	  (list->offset == which))
	return(ent->value);
    }
  return(-1);
}

/*****************************************************************/

I 5
SetDialogPixList(diag, list, size)
/* sets a pointer in the dialog structure to insure that whenever
   a particular object's pixmap is needed, it can be found */
Dialog *diag;
Pixmap *list;
int size;
{
  demand(diag, "SetDialogPixList: NULL diag pointer passed");
  if (!list && size)
    printf("Warning: SetDialogPixList: list param is NULL but size is non-zero");
  diag->pixlist = list;
  diag->pixlistsize = size;
}

/*****************************************************************/

I 14
MakeDialogReadOnly(diag)
Dialog *diag;
{
  diag->readonly = True;
}

/*****************************************************************/

E 14
Pixmap GetDialogPixmap(diag, num)
/* returns the id number of a pixmap in the dialog pixmap list,
   or returns NULL, if the requested pixmap number is out of bounds */
Dialog *diag;
int num;
{
  demand(diag, "GetDialogPixmap: NULL diag pointer passed");
  if (num < diag->pixlistsize && num >= 0)
    return diag->pixlist[num];
  else
    return NULL;
}

/*****************************************************************/

SetDialogHandCursor(diag, cursor)
Dialog *diag;
Cursor cursor;
{
  diag->handcursor = cursor;
}

Cursor GetDialogHandCursor(diag)
Dialog *diag;
{
  return diag->handcursor;
}

/*****************************************************************/

SetDialogWatchCursor(diag, cursor)
Dialog *diag;
Cursor cursor;
{
  diag->watchcursor = cursor;
}

Cursor GetDialogWatchCursor(diag)
Dialog *diag;
{
  return diag->watchcursor;
}

/*****************************************************************/

SetDialogWindowList(diag, list)
/* sets up the array of windows that are in the obtor editor.
   Must be called after make_dialog and before DrawDialogItem gets called. */
Dialog *diag;
Window *list;
{
  diag->windowlist = list;
}

I 17
int HasDialogWindowList(diag)
Dialog *diag;
{
  return(diag->windowlist != NULL);
}

E 17
Window GetDialogWindowListItem(diag, num)
Dialog *diag;
int num;
{
  if (diag->windowlist)
    return diag->windowlist[num];
  else
    return NULL;
}

/*****************************************************************/

E 5
DrawDialogItem(diag,item)
I 5
/* determines from the type of the variable, what kind of editing box to
D 14
   create for it. */
E 14
I 14
   create for it.  In readonly dialogs, command boxes are not drawn. */
E 14
E 5
Dialog *diag;
int item;
{
  DialogList *list;
  DialogEntry *ent;
  char string[1000];
  int size,group,num,x,y;
  Pixmap pix;

  list = &(diag->list[item]);
  ent = &(diag->ent[item]);

  switch(list->type)
    {
    case BOLBOX:
      size = (diag->fi->ascent + diag->fi->descent);
      sprintf(string,"%s ",list->name);
      text_write(diag->win,diag->fi, ent->x1+size, ent->y1,0,0,string);
      XFillRectangle(display,diag->win,inverseGC,ent->x1,ent->y1,size,size);
      XDrawRectangle(display,diag->win,mainGC,
		     ent->x1 + size*2/10,ent->y1 + size*2/10,
		     size * 6/10,size*6/10);
      if (ent->value)
	{
	  XDrawLine(display,diag->win,mainGC,
		    ent->x1 + size*2/10,ent->y1 + size*2/10,
		    ent->x1+size*8/10,ent->y1 + size*8/10);
	  XDrawLine(display,diag->win,mainGC,
		    ent->x1 + size*8/10,ent->y1 + size*2/10,
		    ent->x1+size*2/10,ent->y1 + size*8/10);
	}
      break;
    case MARKSHO:
    case INTBOX:
      sprintf(string,"%s:%d ",list->name,ent->value);
      text_write(diag->win,diag->fi, ent->x1, ent->y1,0,0,string);
      break;
    case TITLEBOX:
      sprintf(string,"%s ",list->name);
      text_write(diag->win,diag->fi, ent->x1, ent->y1,0,0,string);
      XDrawLine(display,diag->win,mainGC,ent->x1,ent->y2-1,ent->x2,ent->y2-1);
      break;
    case LSTRBOX:
    case STRBOX:
D 16
      sprintf(string,"%s:%s ",list->name,ent->value);
E 16
I 16
      if (ent->value)
	sprintf(string, "%s:%s ", list->name, ent->value);
      else
	sprintf(string, "%s: ", list->name);
E 16
      text_write(diag->win,diag->fi, ent->x1, ent->y1,0,0,string);
      break;
I 9
    case HIDSTR:
      if (!ent->value) {
        sprintf(string,"%s[0] ", list->name);
      }
      else {
        char *ptr;
        int lines = 0;
        for (ptr=(char *)ent->value; *ptr; ptr++)
          if (*ptr == '\n') lines++;
	if (!lines) lines = 1;
        sprintf(string,"%s[%d] ", list->name, lines);
      }
      text_write(diag->win,diag->fi, ent->x1, ent->y1,0,0,string);
      break;
E 9
    case QUITBOX:
D 14
      XFillRectangle(display,diag->win,inverseGC,
		     ent->x1,ent->y1,ent->x2-ent->x1,ent->y2-ent->y1);
      text_center(diag->win,diag->fi, 
		  ent->x1,ent->x2,ent->y1,ent->y2-1,list->name);
      XDrawRectangle(display,diag->win,mainGC,
		     ent->x1,ent->y1-1,ent->x2-ent->x1-1,ent->y2-ent->y1);
E 14
I 14
      if (!diag->readonly) {
	XFillRectangle(display,diag->win,inverseGC,
		       ent->x1,ent->y1,ent->x2-ent->x1,ent->y2-ent->y1);
	text_center(diag->win,diag->fi, 
		    ent->x1,ent->x2,ent->y1,ent->y2-1,list->name);
	XDrawRectangle(display,diag->win,mainGC,
		       ent->x1,ent->y1-1,ent->x2-ent->x1-1,ent->y2-ent->y1);
      }
E 14
      break;
    case LINKPIC:
      sprintf(string,"%s:%d ",list->name,ent->value);
      text_write(diag->win,diag->fi, ent->x1, ent->y1,0,0,string);

      x = ent->x1 + (ent->x2 - ent->x1 - BITMAP_WIDTH-BITPAD*2)/2;
      y = ent->y1 + text_height(diag->fi) + BITPAD;
      group = GetDialogMark(diag,0);
      num = ent->value;
      
D 5
      if (group >= 0)
	if (IS_VALID_OBJ(group,num))
	    {
	      pix = item_pixmap[group][num];
E 5
I 5
      pix = GetDialogPixmap(diag, num);
      if (pix) {
E 5
	      XFillRectangle(display,diag->win,inverseGC,x,y,
			     BITMAP_WIDTH + BITPAD*2,BITMAP_WIDTH+BITPAD*2);
	      XDrawRectangle(display,diag->win,mainGC,x,y,
			     BITMAP_WIDTH+BITPAD*2-1,BITMAP_WIDTH+BITPAD*2-1);
	      XCopyArea(display, pix,diag->win, mainGC,0, 0, 
			BITMAP_WIDTH, BITMAP_HEIGHT,x + BITPAD,y+BITPAD);
D 5
	    }
E 5
I 5
      }
E 5
      break;
    case ICONPIC:
      sprintf(string,"%s: ",list->name);
      text_center(diag->win,diag->fi, 
		  ent->x1,ent->x1,ent->y1,ent->y2,string);
      x = ent->x1 + text_width(diag->fi,string);
      XFillRectangle(display,diag->win,inverseGC,x,ent->y1,
		     BITMAP_WIDTH + BITPAD*2,BITMAP_WIDTH + BITPAD*2);
      XDrawRectangle(display,diag->win,mainGC,x,ent->y1,
		     BITMAP_WIDTH + BITPAD*2 -1 ,BITMAP_WIDTH + BITPAD*2-1);
      XCopyArea(display, ent->value,diag->win, mainGC,0, 0, 
		BITMAP_WIDTH, BITMAP_HEIGHT, 
		x + BITPAD,ent->y1+BITPAD);
      break;
    }
}

/*****************************************************************/

RedrawDialog(diag,evt)
Dialog *diag;
XEvent *evt;
{
  DialogList *list;
  DialogEntry *ent;
  int i,nodraw;
  int x1,y1,x2,y2;

I 5
  if (GetDialogHandCursor(diag))
    XDefineCursor(display,diag->win,GetDialogHandCursor(diag));

E 5
  for (i = 0;i < diag->count;i++)
    {
      list = &(diag->list[i]);
      ent = &(diag->ent[i]);

      x1 = evt->xexpose.x;
      y1 = evt->xexpose.y;
      x2 = evt->xexpose.x + evt->xexpose.width;
      y2 = evt->xexpose.y + evt->xexpose.height;

      nodraw = (((ent->x1 < x1)&&(ent->x2 < x1)) ||
		((ent->x1 > x2)&&(ent->x2 > x2)) ||
		((ent->y1 < y1)&&(ent->y2 < y1)) ||
		((ent->y1 > y2)&&(ent->y2 > y2)));
      
      if (!nodraw)
	DrawDialogItem(diag,i);
    }
}

/*****************************************************************/

kill_dialog(diag)
Dialog *diag;
{
  DialogList *list;
  DialogEntry *ent;
  int i;

  for (i = 0;i < diag->count;i++)
    {
      list = &(diag->list[i]);
      ent = &(diag->ent[i]);
      switch(list->type)
	{
	case LSTRBOX:
	case STRBOX:
I 9
	case HIDSTR:
E 9
	  free(ent->value);
	  break;
	case ICONPIC:
	  XFreePixmap(display,ent->value);
	  break;
	}
    }

  XDestroyWindow(display,diag->win);
  free(diag->ent);
  free(diag);
}

/*****************************************************************/

CheckProcInhibit(diag,item,event)
Dialog *diag;
XEvent *event;
int item;
{
  DialogList *list;

  list = &(diag->list[item]);
  if (list->call)
    return( (*list->call)(diag,item,event));
  
  return(False);
}

/*****************************************************************/

D 6
linkpic_hook(diag,event,str)
XEvent *event;
char *str;
Dialog *diag;
{
  int group,num;

  if ((diag->win == event->xany.window)||(event->type != ButtonPress))
    return(False);
  
  group = GetDialogMark(diag,0);
  if (group != FindObtorWindow(event->xany.window))
    return(False);
  
  num = obtor_obj_in_grab_win(event->xbutton.x,event->xbutton.y,group);
D 5
  if (IS_VALID_OBJ(group,num))
E 5
I 5
  if (GetDialogPixmap(diag, num))
E 5
    {
      sprintf(str,"%d",num);
      return(True);
    }
  
  return(False);
}

/*****************************************************************/

E 6
ItemClick(diag,item,event)
I 5
/* determines from the type of the variable in the area where you clicked
   the mouse button what to do and how to get further input and change
D 6
   the value of the variable in the structure. */
E 6
I 6
   the value of the variable in the structure.  Returns TRUE if we should
   quit the dialog window. */
E 6
E 5
Dialog *diag;
XEvent *event;
int item;
{
  char *inp,prompt[1025];
  DialogList *list;
  DialogEntry *ent;
  int newint,group,num,offset,i;

  list = &(diag->list[item]);
  ent = &(diag->ent[item]);

  if (CheckProcInhibit(diag,item,event))
    return(False);
    
  switch(list->type)
    {
    case BOLBOX:
      ent->value = !ent->value;
      diag->changed = True;
      break;
    case INTBOX:
      sprintf(prompt,"%s(%d):",list->name,ent->value);
      inp = get_string_shift(diag->win, ent->x1, ent->y1, prompt, diag->width);
      if (sscanf(inp,"%d",&newint) == 1)
	{
	  ent->value = newint;
	  diag->changed = True;
	}
      break;
    case LINKPIC:
      group = GetDialogMark(diag,0);
D 5
      XDefineCursor(display,objwin[group],HandCursor);
E 5
I 5
D 17
      if (GetDialogHandCursor(diag))
E 17
I 17
      if (GetDialogHandCursor(diag) && HasDialogWindowList(diag))
E 17
        XDefineCursor(display,GetDialogWindowListItem(diag, group),
		      GetDialogHandCursor(diag));
E 5
      sprintf(prompt,"%s(%d):",list->name,ent->value);
      inp = get_string_hook(diag->win,ent->x1,ent->y1, prompt,diag->width
			    ,linkpic_hook,(long) diag);
D 5
      XDefineCursor(display,objwin[group],None);
E 5
I 5
D 17
      XDefineCursor(display,GetDialogWindowListItem(diag,group),None);
E 17
I 17
      if (HasDialogWindowList(diag))
	XDefineCursor(display,GetDialogWindowListItem(diag,group),None);
E 17
E 5
      if (sscanf(inp,"%d",&newint) == 1)
	{
	  ent->value = newint;
	  diag->changed = True;
	}
      break;
I 9
    case HIDSTR:
D 10
      printf("edit HIDSTR\n");
E 10
I 10
      sprintf(prompt,"%s: ",list->name);
      inp = display_edit_string(diag->win, FALSE, ent->x1, ent->y1, prompt,
				((char *) ent->value), TRUE, regfont, 80);
      if (inp && (strlen(inp) > 0))
	{
	  if (ent->value) free(ent->value);
	  demand((ent->value = malloc(strlen(inp) + 1)),
		 "no memory for copy of input");
	  strcpy((char *)ent->value,inp);
	  diag->changed = True;
	}
      else if (inp) {
	if (ent->value) free(ent->value);
	ent->value = NULL;
	diag->changed = True;
      }
      break;
E 10
E 9
    case LSTRBOX:
    case STRBOX:
D 16
      sprintf(prompt,"(%s:%s) ",list->name,ent->value);
E 16
I 16
      if (ent->value)
	sprintf(prompt,"(%s:%s) ",list->name,ent->value);
      else
	sprintf(prompt,"(%s:) ",list->name);
E 16
      text_write(diag->win,diag->fi,ent->x1,ent->y1,0,1,prompt);
      text_erase(diag->win,diag->fi,ent->x1,ent->y1,0,0,strlen(prompt));
      sprintf(prompt,"%s:",list->name);
      inp = get_string_shift(diag->win, ent->x1, ent->y1, prompt, diag->width);
D 16
      sprintf(prompt,"(%s:%s) ",list->name,ent->value);
E 16
I 16
      if (ent->value)
	sprintf(prompt,"(%s:%s) ",list->name,ent->value);
      else
	sprintf(prompt,"(%s:) ",list->name);
E 16
      text_clear(diag->win,diag->fi,ent->x1,ent->y1,0,1,strlen(prompt));
D 9
      
E 9
      if (strlen(inp) > 0)
	{
I 8
	  if (ent->value) free(ent->value);
	  demand((ent->value = malloc(strlen(inp) + 1)),
		 "no memory for copy of input");
E 8
D 7
	  strcpy(ent->value,inp);
E 7
I 7
	  strcpy((char *)ent->value,inp);
E 7
	  diag->changed = True;
	}
      break;
I 5
D 9

E 9
E 5
    case ICONPIC:
D 5
      XDefineCursor(display,diag->win,WatchCursor);
E 5
I 5
      if (GetDialogWatchCursor(diag))
        XDefineCursor(display,diag->win,GetDialogWatchCursor(diag));
E 5
      edit_pixmap(&(ent->value));
D 5
      XDefineCursor(display,diag->win,HandCursor);
E 5
I 5
      if (GetDialogHandCursor(diag))
        XDefineCursor(display,diag->win,GetDialogHandCursor(diag));
E 5
      diag->changed = True;
      break;
    case QUITBOX:
D 14
      kill_dialog(diag);
E 14
      return(True);
      break;
    }

  DrawDialogItem(diag,item);

  return(False);
}

/*****************************************************************/

DialogButton(diag,event)
I 6
/* this is called when a button is pressed, it will return TRUE
   if we should quit the dialog */
E 6
Dialog *diag;
XEvent *event;
{
  int x,y;
  int i;
  DialogEntry *ent;

  x = event->xbutton.x;
  y = event->xbutton.y;
  
  if (event->xany.window != diag->win)
    return(False);

  for (i = 0;i < diag->count;i++)
    {
      ent = &(diag->ent[i]);
      if ((x >= ent->x1)&&(x < ent->x2)&&
	  (y >= ent->y1)&&(y < ent->y2))
	return(ItemClick(diag,i,event));
    }
  return(False);
I 2
}

/*****************************************************************/

D 13
FindNextData(DialogList *form,int j,int what)
E 13
I 13
FindNextData(form, j, what)
DialogList *form;
int j;
int what;
E 13
{
  for (j++;form[j].name;j++)
    if (form[j].data == what)
      return(j);
  
  return(-1);
E 2
}

/*****************************************************************/

I 3
SetDialogKeyHook(diag,hook)
Dialog *diag;
int (*hook)();
{
  diag->key = hook;
}

/*****************************************************************/

E 3
SetDialogHook(diag,hook)
Dialog *diag;
int (*hook)();
{
  diag->hook = hook;
}

/*****************************************************************/

DialogProcess(diag)
I 5
/* this is the loop you call once you have completely set up a dialog
   box.  It will cycle through reading events and changing variables
   as the user directs, until the diag hook procedure returns TRUE
   on one of the events. */
E 5
Dialog *diag;
{
  XEvent event;
  int done = False;
    
  while(!done)
    {
      XNextEvent(display, &event);
      if (diag->hook) 
	if ((*(diag->hook))(diag,&event))
	  done = True;
      switch(event.type)
	{
	case ButtonPress:
D 14
	  if (DialogButton(diag,&event))
E 14
I 14
	  if (diag->readonly || DialogButton(diag,&event))
E 14
	    done = True;
I 3
	  break;
	case KeyPress:
D 14
	  if (diag->key)
E 14
I 14
	  if (diag->readonly)
	    done = True;
	  else if (diag->key) {
E 14
	    if ((*(diag->key))(diag,&event))
	      done = True;
I 14
	  }
E 14
E 3
	  break;
	case GraphicsExpose:
	case Expose:
	  if (event.xany.window == diag->win)
	    RedrawDialog(diag,&event);
	  else
	    {
	      /* here so the rest of obtor works */
	      bcopy(&event, &last_event, sizeof(XEvent));
	      RedrawGlobaly(&event);
	    }
	  break;
	}
    }
I 14
  kill_dialog(diag);
E 14
}

/*****************************************************************/

char *get_string_hook(what_win, xstart, y, prompt, width,hook,data)
Window  what_win;
int	xstart, y;
char    *prompt;
int width;
int (*hook)();
long data;
{
  XFontStruct *fi;
  int x, promptlen, detail, i = 0,offset;
  int type, mousex, mousey, done = 0;
  int maxx,rightside;
  char c, s[2];
  static char line[TEXTINPLEN];
  Window *happenwin;

  fi = regfont;
  maxx = -1;
  line[0] = '\0';
  promptlen = text_width(fi, prompt);
  x = promptlen + xstart;
  while (x > width)
    {
      offset = text_width(fi,"12345");
      xstart -= offset;
      x -= offset;
    }
  text_write(what_win, fi, xstart, y, 0, 0, prompt);
  text_write(what_win, fi, x, y, 0, 0, "_");
  maxx = (x + text_width(fi,line) + text_width(fi,"_   "));

  do
    {
      /* Look for input until some is found */
      do
	{
	  type =
	    get_input(what_win, &c, &mousex, &mousey, &detail, &happenwin);
D 13
	  done = (*hook)(data,&last_event,&line);
E 13
I 13
	  done = (*hook)(data, &last_event, line);
E 13
	} while ((type==NOTHING)&&(!done));

    /* Now act according to the input received */
      switch (type)
	{
	case KEYBOARD:
	  if (c=='\n' || c=='\r') 
	    {
	      /* Erase underline char and quit on return key */
	      done = 1;
	      text_erase(what_win, fi, x, y, i, 0, 1);
	    }
	  else if (c=='\b' || c==127 || c==8) 
	    {
	      /* Erase chars when backspacing */
	      text_erase(what_win, fi, x, y, i + 1, 0, 1);
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      if (i>0) i--;
	      line[i] = '\0';
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      text_write(what_win, fi, x, y, i, 0, "_");
	    }
	  else 
	    {
	      /* Add and write chars when they are received */
	      if (i < TEXTINPLEN-2) 
		{
		  s[0] = c;
		  s[1] = '\0';

		  if ((text_width(fi,line)+x+text_width(fi,"_ ")) > width)
		    {
		      text_erase(what_win,fi,x,y,i-5,0,5);
		      offset = text_width(fi,"12345");
		      xstart -= offset;
		      x -= offset;
		      text_write(what_win,fi,xstart,y,0,0,prompt);
		      text_write(what_win,fi,x,y,0,0,line);
		    }

		  text_erase(what_win, fi, x, y, i, 0, 1);
		  text_write(what_win, fi, x, y, i, 0, s);
		  line[i++] = c;
		  line[i] = '\0';
		  text_write(what_win, fi, x, y, i, 0, "_");

		  rightside = (x + text_width(fi,line) + text_width(fi,"_ "));
		  if (rightside > maxx)
		    maxx= rightside;
		}
	    }
	  break;
	case EXPOSED:
	  text_write(what_win, fi, xstart, y, 0, 0, prompt);
	  text_write(what_win, fi, x, y, 0, 0, line);
	  text_write(what_win, fi, x, y, i, 0, "_");
	  break;
	default:
	  break;
	}
    } while (!done);
  
  XClearArea(display,what_win,xstart,y,maxx-xstart,text_height(fi),True);
  return line;
}

/*****************************************************************/

E 1
