h19816
s 00000/00000/00226
d D 1.3 92/08/07 01:04:52 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00125/00001/00101
d D 1.2 91/09/16 22:20:32 labc-4lc 2 1
c made room and recobj dialog edit windows work for editmap
e
s 00102/00000/00000
d D 1.1 91/09/15 23:36:11 labc-4lc 1 0
c date and time created 91/09/15 23:36:11 by labc-4lc
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

/* this file contains the routines called by editmap for editing the
   properties of a map, room, or recorded object instance. */

#include "windowsX11.h"
#include "def.h"
#include "dialog.h"
extern Window grabwin;

I 2
static RecordedObj *DataPtr = NULL;



/* ==================== E D I T  M A P  I N F O ========================= */

E 2
void DialogEditMapInfo()
/* start up a dialog window containing the things in the MapInfo structure,
   and allow the user to edit it until satisfied. */
{
  DialogList *StaticMapProperties();
  DialogList *MapData = StaticMapProperties();
  int newx = 10, newy = 10;
  Dialog *info_rec;

  info_rec =
    make_dialog(RootWindow(display, screen), newx, newy, regfont, MapData);
  XStoreName(display, info_rec->win, "Editmap MapInfo Edit");
  SetDialogData(info_rec, &mapstats);
  SetDialogPixList(info_rec, obj_pixmap, objects);
  DialogProcess(info_rec);
}



int EditMapInfoSave(rec, item, event)
/* hook procedure which is called when exiting the dialog edit mode,
   saves the changes into the mapinfo structure */
Dialog *rec;
int item;
XEvent *event;
{
  GetDialogData(rec, &mapstats);
  if (rec->changed) {
    redraw_stat_window();
  }

  return FALSE;
}



int EditMapInfoQuit(rec, item, event)
/* hook procedure called when exciting the dialog edit mode, will
   query user about whether to save changes */
Dialog *rec;
int item;
XEvent *event;
{
  int ret;

  if (!rec->changed) return FALSE;

  ret = placed_letter_query(rec->win, 0, 0, rec->width, rec->height,
D 2
			    "Save Changes?\n(y/n/c)", "ync");
E 2
I 2
			    "Save Changes? (y/n/c)", "ync");
E 2

  if (ret == 'y') return(EditMapInfoSave(rec, item, event));
  else return (ret == 'c');
}



int linkpic_hook(diag,event,str)
Dialog *diag;
XEvent *event;
char *str;
{
  int num;

  if ((event->xany.window != grabwin) || (event->type != ButtonPress))
    return FALSE;

  num = obj_in_grab_win(event->xbutton.x, event->xbutton.y);  
  if (num >= 0 && num < objects) {
    sprintf(str, "%d", num);
    return TRUE;
  }
  else return FALSE;
I 2
}



/* ================== E D I T  R O O M  I N F O ======================== */

void DialogEditRoomInfo()
/* calls up a window for editing the room info for the current room. */
{
  DialogList *StaticRoomProperties();
  DialogList *RoomData = StaticRoomProperties();
  int newx = 10, newy = 10;
  Dialog *info_rec;

  info_rec =
    make_dialog(RootWindow(display, screen), newx, newy, regfont, RoomData);
  XStoreName(display, info_rec->win, "Editmap RoomInfo Edit");
  SetDialogData(info_rec, &(room[current_room]));
  SetDialogPixList(info_rec, obj_pixmap, objects);
  DialogProcess(info_rec);
}



int EditRoomInfoSave(rec, item, event)
/* hook procedure which is called when exiting the dialog edit mode,
   saves the changes into the roominfo structure */
Dialog *rec;
int item;
XEvent *event;
{
  GetDialogData(rec, &(room[current_room]));
  if (rec->changed) {
    redraw_stat_window();
  }

  return FALSE;
}



int EditRoomInfoQuit(rec, item, event)
/* hook procedure called when exciting the dialog edit mode, will
   query user about whether to save changes */
Dialog *rec;
int item;
XEvent *event;
{
  int ret;

  if (!rec->changed) return FALSE;

  ret = placed_letter_query(rec->win, 0, 0, rec->width, rec->height,
			    "Save Changes? (y/n/c)", "ync");

  if (ret == 'y') return(EditRoomInfoSave(rec, item, event));
  else return (ret == 'c');
}



/* ============== E D I T  R E C O R D E D  O B J E C T S ============ */

void DialogEditRecObj(data)
/* calls up a window for editing the info in a recorded object */
RecordedObj *data;
{
  DialogList *StaticRecObjProperties();
  DialogList *RecObjData = StaticRecObjProperties();
  int newx = 10, newy = 10;
  Dialog *info_rec;

  DataPtr = data;
  info_rec =
    make_dialog(RootWindow(display, screen), newx, newy, regfont, RecObjData);
  XStoreName(display, info_rec->win, "RecObjInfo Edit");
  SetDialogData(info_rec, DataPtr);
  SetDialogPixList(info_rec, obj_pixmap, objects);
  DialogProcess(info_rec);
  DataPtr = NULL;
}



int EditRecObjSave(rec, item, event)
/* hook procedure which is called when exiting the dialog edit mode,
   saves the changes into the recorded object structure */
Dialog *rec;
int item;
XEvent *event;
{
  if (DataPtr) {
    GetDialogData(rec, DataPtr);
    if (rec->changed) {
      redraw_stat_window();
    }
  }
  return FALSE;
}



int EditRecObjQuit(rec, item, event)
/* hook procedure called when exciting the dialog edit mode, will
   query user about whether to save changes */
Dialog *rec;
int item;
XEvent *event;
{
  int ret;

  if (!rec->changed) return FALSE;

  ret = placed_letter_query(rec->win, 0, 0, rec->width, rec->height,
			    "Save Changes? (y/n/c)", "ync");

  if (ret == 'y') return(EditRecObjSave(rec, item, event));
  else return (ret == 'c');
E 2
}
E 1
