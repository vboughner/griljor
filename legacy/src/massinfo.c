/***************************************************************************
 * @(#) massinfo.c 1.12 - last change made 08/07/92
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

#include "obtor.h"
int InfoEditHook();

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
  DialogList *AllocObjectProperties();
  DialogList *ObjectData = AllocObjectProperties();
  int newx, newy;	
  int winnum,which;
  Dialog *info_rec;

  ConvertToMassEdit(ObjectData,EditItem);
  XDefineCursor(display,obtorwin,WatchCursor);
  info_rec = make_dialog(rootwin,newx,newy,MASS_FONT,ObjectData);
  XSetWindowBackgroundPixmap(display,info_rec->win,info_pix);
  XStoreName(display,info_rec->win,"Obtor Mass Edit");
  StartInfoEdit(info_rec);
  SetDialogHook(info_rec,MassInfoHook);
  SetDialogKeyHook(info_rec,InfoEditHook);
  SetDialogWindowList(info_rec, objwin);
  SetDialogHandCursor(info_rec, HandCursor);
  SetDialogWatchCursor(info_rec, WatchCursor);
  DialogProcess(info_rec);
  XDefineCursor(display,obtorwin,ArrowCursor);
  StartInfoEdit(NULL);
  free(ObjectData);
}
  
/*****************************************************************/
  
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

ConvertToMassEdit(DialogList *form,int (*proc)())
{
  int j;

  for (j = 0;form[j].name;j++)
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
	case HIDSTR:
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
	  break;
	}
    }

  j = FindNextData(form,-1,TITLEBOX);
  form[j].type = TITLEBOX;
  form[j].name = "MassEdit";

  j = FindNextData(form,-1,LSTRBOX);
  form[j].type = TITLEBOX;
  form[j].name = "Click on property to edit.  All changes are perminant!";
  
  j = FindNextData(form,-1,QUITBOX);
  form[j].type = QUITBOX;
  form[j].name = "Done";
  form[j].call = NULL;
  
  j = FindNextData(form,-1,MARKSHO);
  form[j].type = MARKSHO;
  form[j].name = "Group";

}

/*****************************************************************/
