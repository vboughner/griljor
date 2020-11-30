h53916
s 00000/00000/00267
d D 1.12 92/08/07 01:02:16 vbo 12 11
c source copied to a separate tree for work on new map and object format
e
s 00000/00001/00267
d D 1.11 91/12/13 21:15:39 labc-4lc 11 10
c took out uncovered type warning
e
s 00000/00000/00268
d D 1.10 91/12/13 19:33:42 labc-4lc 10 9
c fixed bug that made a completely blank object unreadable and made it possible to move object properties into an old/outdated state
e
s 00001/00000/00267
d D 1.9 91/11/27 15:07:07 labc-4lc 9 8
c changed message properties to hidden strings
e
s 00024/00001/00243
d D 1.8 91/09/15 20:53:16 labc-4lc 8 7
c made dialog.c a little more generic, changed obtor to deal with it
e
s 00003/00001/00241
d D 1.7 91/09/05 21:28:40 labb-3li 7 6
c made obtor use the new objprops.c code for dialog lists
e
s 00001/00001/00241
d D 1.6 91/07/14 14:01:55 labc-3id 6 5
c Added 'ObjDiag.t' code.
e
s 00001/00001/00241
d D 1.5 91/06/30 20:27:48 labc-3id 5 4
c fixed up editing order of properties for obtor
e
s 00003/00000/00239
d D 1.4 91/05/17 02:01:40 labc-3id 4 3
c Added code for showing object numbers using '#'
e
s 00030/00029/00209
d D 1.3 91/05/03 23:36:01 labc-3id 3 2
c 
e
s 00068/00016/00170
d D 1.2 91/05/03 23:10:57 labc-3id 2 1
c 
e
s 00186/00000/00000
d D 1.1 91/03/24 18:45:12 labc-3id 1 0
c date and time created 91/03/24 18:45:12 by labc-3id
e
u
U
f e 0
t
T
I 1
D 8
#include "obtor.h"
E 8
I 8
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
E 8

I 8
#include "obtor.h"
E 8
I 4
int InfoEditHook();

E 4
/*****************************************************************/

MassInfoHook(diag,event)
Dialog *diag;
XEvent *event;
{
  int group,num;
  char *vptr;

  if (event->type != ButtonPress)
    return(False);
  group = FindObtorWindow(event->xany.window);
  if (group < 0)
    return(False);
    
  num = obtor_obj_in_grab_win(event->xbutton.x,event->xbutton.y,group);
  
  if (!IS_VALID_OBJ(group, num))
    return(False);

  vptr = (((char *) objdef[group][num]) + diag->list[mass_field].offset);

  switch(diag->list[mass_field].data)
    {
    case BOLBOX:
      *vptr = !(*vptr);
      redraw_mass_info(group,num);
      break;
    case LINKPIC:
    case INTBOX:
      *((short *) vptr) = MassChangeNum(group,num,(int) *((short *) vptr));
      break;
    }

  FileChanged(group,True);
  return(False);
}

/*****************************************************************/

I 2
EditItem(rec,item,event)
int item;
Dialog *rec;
XEvent *event;
{
  XDefineCursor(display,obtorwin,ArrowCursor);
  SetInfoEdit(rec,item);
  mass_changed = True;
  return(True);
}

/*****************************************************************/

E 2
MassChangeNum(group,num,def)
int group,num,def;
{
  char *inp;
  int x,y,new;

  calc_obj_xy(group,num,&x,&y);
  inp = get_string_shift(objwin[group],x, y, ">",0);
  if (sscanf(inp,"%d",&new) == 1)
    def = new;

  return(def);
}

/*****************************************************************/
  
handle_mass_info(win, x, y)
     Window win;
     int x, y;
{
D 2
#include "MassEdit.h"   /* special type of include file! */
E 2
I 2
D 6
  #include "ObjDiag.h"
E 6
I 6
D 7
  #include "ObjDiag.t"
E 7
I 7
  DialogList *AllocObjectProperties();
  DialogList *ObjectData = AllocObjectProperties();
E 7
E 6
E 2
  int newx, newy;	
  int winnum,which;
  Dialog *info_rec;
D 2
  
E 2
I 2

  ConvertToMassEdit(ObjectData,EditItem);
E 2
  XDefineCursor(display,obtorwin,WatchCursor);
D 2
  info_rec = make_dialog(rootwin,newx,newy,HELP_FONT,MassEdit);
E 2
I 2
D 5
  info_rec = make_dialog(rootwin,newx,newy,HELP_FONT,ObjectData);
E 5
I 5
  info_rec = make_dialog(rootwin,newx,newy,MASS_FONT,ObjectData);
E 5
E 2
  XSetWindowBackgroundPixmap(display,info_rec->win,info_pix);
  XStoreName(display,info_rec->win,"Obtor Mass Edit");
  StartInfoEdit(info_rec);
  SetDialogHook(info_rec,MassInfoHook);
I 4
  SetDialogKeyHook(info_rec,InfoEditHook);
I 8
  SetDialogWindowList(info_rec, objwin);
  SetDialogHandCursor(info_rec, HandCursor);
  SetDialogWatchCursor(info_rec, WatchCursor);
E 8
E 4
  DialogProcess(info_rec);
  XDefineCursor(display,obtorwin,ArrowCursor);
  StartInfoEdit(NULL);
I 7
  free(ObjectData);
E 7
}
  
/*****************************************************************/
  
D 2
EditItem(rec,item,event)
int item;
Dialog *rec;
XEvent *event;
{
  XDefineCursor(display,obtorwin,ArrowCursor);
  SetInfoEdit(rec,item);
  mass_changed = True;
  return(True);
}

/*****************************************************************/

E 2
SetInfoEdit(rec,item)
Dialog *rec;
int item;
{
  int i;

  mass_field = item;

  for (i = 0;i < MAX_BUFFERS;i++)
    redraw_info_items(i);

  return(False);
}

/*****************************************************************/

StartInfoEdit(rec)
Dialog *rec;
{
  int i;

  if (rec)
    {
      mass_rec = rec;
      mass_field = -1;
      mass_changed = False;
    }
  else
    {
      mass_rec = NULL;
      if (mass_changed)
	for (i = 0;i < MAX_BUFFERS;i++)
	  XClearArea(display,objwin[i],0,0,0,0,True);
    }
}

/*****************************************************************/

redraw_info_items(win)
int win;
{
  int i;

  for (i = 0;i < num_objects[win];i++)
    redraw_mass_info(win,i);
}

/*****************************************************************/

redraw_mass_info(win,i)
int win,i;
{
  int x,y,w,h;
  char string[100],*vptr;
  long value;

  if (!IS_VALID_OBJ(win,i))
    return;
  if ((mass_rec == NULL)||(mass_field < 0))
    return;

  calc_obj_xy(win,i,&x,&y);
  x += BITMAP_HEIGHT - MASSPAD - char_width(tinyfont)*3 - 1;
  y += BITMAP_HEIGHT - MASSPAD - text_height(tinyfont) - 1;
  w = char_width(tinyfont) * 3 + 2;
  h = text_height(tinyfont)+2;
  XClearArea(display,objwin[win],x,y,w,h,False);
  vptr = (((char *) objdef[win][i]) + mass_rec->list[mass_field].offset);
  
  switch(mass_rec->list[mass_field].data)
    {
    case BOLBOX:
      if (*((char *) vptr))
	{
	  XDrawLine(display,objwin[win],mainGC,x,y,x+w,y+h);
	  XDrawLine(display,objwin[win],mainGC,x+w,y,x,y+h);
	}
/*      sprintf(string,"%s",(*((char *) vptr) > 0 ? " X " : "   ")); */
      break;
    case LINKPIC:
    case INTBOX:
      sprintf(string,"%3d",(int) (* ((short *) vptr) ));
      text_write(objwin[win],tinyfont,x+1,y+1,0,0,string);
      break;
    default:
      return;
    }
  
  XDrawRectangle(display,objwin[win],mainGC,x,y,w,h);
}

/*****************************************************************/

I 2
ConvertToMassEdit(DialogList *form,int (*proc)())
{
  int j;

  for (j = 0;form[j].name;j++)
D 3
    switch(form[j].type)
      {
      case NEWCOL:
      case BLANKBOX:
	form[j].data = BLANKBOX;
	break;
      case TITLEBOX:
      case STRBOX:
      case LSTRBOX:
      case QUITBOX:
      case MARKSHO:
      case ICONPIC:
      case LINKPIC:
	form[j].data = form[j].type;
	form[j].type = BLANKBOX;
	break;
      case BOLBOX:
      case INTBOX:
	form[j].data = form[j].type;
	form[j].type = QUITBOX;
	form[j].call = proc;
	break;
      default:
	printf("Warning... uncovered type.\n");
	break;
      }
E 3
I 3
    {
      form[j].data = form[j].type;
      switch(form[j].type)
	{
	case NEWCOL:
	case BLANKBOX:
	case TITLEBOX:
	  break;
	case STRBOX:
	case LSTRBOX:
I 9
	case HIDSTR:
E 9
	case QUITBOX:
	case MARKSHO:
	case ICONPIC:
	case LINKPIC:
	  form[j].type = BLANKBOX;
	  break;
	case BOLBOX:
	case INTBOX:
	  form[j].type = QUITBOX;
	  form[j].call = proc;
	  break;
	default:
D 11
	  printf("Warning... uncovered type.\n");
E 11
	  break;
	}
    }
E 3

  j = FindNextData(form,-1,TITLEBOX);
  form[j].type = TITLEBOX;
  form[j].name = "MassEdit";
I 3

  j = FindNextData(form,-1,LSTRBOX);
  form[j].type = TITLEBOX;
  form[j].name = "Click on property to edit.  All changes are perminant!";
E 3
  
  j = FindNextData(form,-1,QUITBOX);
  form[j].type = QUITBOX;
  form[j].name = "Done";
  form[j].call = NULL;
  
  j = FindNextData(form,-1,MARKSHO);
  form[j].type = MARKSHO;
  form[j].name = "Group";

D 3
  j = FindNextData(form,-1,LSTRBOX);
  form[j].type = LSTRBOX
  form[j].name = "Click on property to edit.  All changes are perminant!";
E 3
}

/*****************************************************************/
E 2
E 1
