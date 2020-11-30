h33378
s 00000/00000/00225
d D 1.4 92/08/07 01:00:46 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00003/00003/00222
d D 1.3 91/10/16 20:05:27 labc-4lc 3 2
c changed made for DecStations
e
s 00014/00010/00211
d D 1.2 91/05/03 23:10:46 labc-3id 2 1
c 
e
s 00221/00000/00000
d D 1.1 91/03/24 18:44:49 labc-3id 1 0
c date and time created 91/03/24 18:44:49 by labc-3id
e
u
U
f e 0
t
T
I 1

#include "X11/cursorfont.h"

#include "obtor.h"


/*****************************************************************/
  
drag_object(win,x,y)
int win,x,y;
{
  int num,result,newwin,newnum;
  Cursor old;
  Window root,child;
D 2
  int rx,ry,hasused;
E 2
I 2
  int rx,ry,hasused,cx,cy;
E 2
  unsigned int keys;
  ObjInfo *temp;
  XEvent throwaway;
D 2

E 2
I 2
  
E 2
  num = obtor_obj_in_grab_win(x, y,win);

  if (!IS_VALID_OBJ(win, num))
    return(False);

I 2
  XQueryPointer(display,objwin[win],&root,&child,&rx,&ry,&cx,&cy,&keys);
E 2
  TempUnmapArrowWindows(win);
  XDefineCursor(display,obtorwin,DragCursor);
  XGrabServer(display); 
  box_grab_square(win,num,mainGC);
D 2
  hasused = drag_bitmap(win,num);
E 2
I 2
  hasused = drag_bitmap(win,num,cx,cy);
E 2
  XUngrabServer(display);
  XDefineCursor(display,obtorwin,ArrowCursor);
  box_grab_square(win,num,inverseGC);
  AdjArrowWindows(win);
  
  if (!hasused)
    {
      XQueryPointer(display,obtorwin,&root,&child,&rx,&ry,&x,&y,&keys);
      if ((newwin = FindObtorWindow(child)) >= 0)
	{
	  XQueryPointer(display,objwin[newwin],
			&root,&child,&rx,&ry,&x,&y,&keys);
	  newnum = obtor_obj_in_grab_win(x, y,newwin);
	  
	  if ((newwin == win)&&(newnum != num))
	    {
	      if (IS_VALID_OBJ(newwin,newnum))
		{
		  temp = copy_obj_definition(objdef[win][num]);
		  copy_obj_to_group(objdef[newwin][newnum], win,num);
		  copy_obj_to_group(temp, newwin,newnum);
		  free(temp);
I 2
		  adj_switch_all(newwin,num,newnum);
E 2
		}
	      else
		copy_obj_to_group(objdef[win][num],newwin,newnum);
	    }
	  else if (newwin != win)
D 2
	    insert_obj_in_group(copy_obj_definition(objdef[win][num]),
				newwin,newnum);
E 2
I 2
	    {
	      insert_obj_in_group(copy_obj_definition(objdef[win][num]),
				  newwin,newnum);
	      adj_clear(newwin,newnum);
	    }
E 2
	}
      else
	{
	  /* this should check to see what window it is in, but I 
	     don't know how to do that! */
	  XQueryPointer(display,trywin,&root,&child,&rx,&ry,&x,&y,&keys);
	  try_drop(win,num,x,y);
	}
    }

  while(XCheckMaskEvent(display,ButtonPressMask,&throwaway))
    ;
  /* throw away any button-2 or button-3 events that were used
     while draging */
}

/*****************************************************************/

D 2
drag_bitmap(win,num)
int win,num;
E 2
I 2
drag_bitmap(win,num,clickx,clicky)
int win,num,clickx,clicky;
E 2
{
  Pixmap backg,obj,mask;
  int x,y,lx,ly,lbg,lbn,retvalue,oldset,force_redraw;
  int wx,wy,rx,ry,masked,group,newnum;
  int dx,dy;
  GC nmaskGC,nmainGC,ndrawGC;
  unsigned int keys,lastmask;
  Window root,child;
  unsigned long maskvalues;
  XGCValues values;

  calc_obj_xy(win,num,&x,&y); /* where  to hold the bitmap by */
D 2
  XQueryPointer(display,objwin[win],&root,&child,&rx,&ry,&wx,&wy,&keys);
  dx = x - wx;
  dy = y - wy;
E 2
I 2
  dx = x - clickx;
  dy = y - clicky;
E 2

D 3
  obj = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,1);
  mask = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,1);
  backg = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,1);
E 3
I 3
  obj = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,depth);
  mask = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,depth);
  backg = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,depth);
E 3

  XCopyArea(display,item_pixmap[win][num],obj,mainGC,0,0,
	    BITMAP_WIDTH,BITMAP_HEIGHT,0,0);

  if (objdef[win][num] -> masked)
    XCopyArea(display,item_mask[win][num],mask,mainGC,0,0,
	      BITMAP_WIDTH,BITMAP_HEIGHT,0,0);
  else
    XFillRectangle(display,mask,mainGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);

  values.subwindow_mode = IncludeInferiors;
  maskvalues = 0; 
  nmaskGC = XCreateGC(display,rootwin,maskvalues,&values);
  ndrawGC = XCreateGC(display,rootwin,maskvalues,&values);
  nmainGC = XCreateGC(display,rootwin,maskvalues,&values);
  maskvalues = ~maskvalues;
  XCopyGC(display,maskGC,maskvalues,nmaskGC);
  XCopyGC(display,drawGC,maskvalues,ndrawGC);
  XCopyGC(display,mainGC,maskvalues,nmainGC);
  XChangeGC(display,nmaskGC,GCSubwindowMode,&values);
  XChangeGC(display,ndrawGC,GCSubwindowMode,&values);
  XChangeGC(display,nmainGC,GCSubwindowMode,&values);

  oldset = False;
  retvalue = 0;
  lastmask = 0;
  force_redraw = False;
  do
    {
      XQueryPointer(display,obtorwin,&root,&child,&x,&y,&wx,&wy,&keys);
      y += dy;
      x += dx;
      if ((x != lx)||(y != ly))
	{
	  if ((group = FindObtorWindow(child)) >= 0)
	    {
	      XQueryPointer(display,objwin[group],&root,&child,
			    &rx,&ry,&wx,&wy,&keys);
	      newnum = obtor_obj_in_grab_win(wx,wy,group);
	    }

	  if (oldset)
	    XCopyArea(display,backg,rootwin,nmainGC,
		      0,0,BITMAP_WIDTH,BITMAP_HEIGHT,lx,ly);
	  
	  if (((lbg != group)||(lbn != newnum))&&(oldset)&&(lbg >= 0))
	    box_grab_square(lbg,lbn,xorGC);

	}

      if (((keys & ~lastmask) & (Button2Mask | Button3Mask))&&(group >= 0))
	{
	  retvalue = True;
	  force_redraw = True;
	}

      if ( (x == lx)&&(y == ly) && (force_redraw))
	{
	  if (group >= 0)
	    {
	      XQueryPointer(display,objwin[group],&root,&child,
			    &rx,&ry,&wx,&wy,&keys);
	      newnum = obtor_obj_in_grab_win(wx,wy,group);
	    }
	  if (((lbg != group)||(lbn != newnum))&&(oldset)&&(lbg >= 0))
	    box_grab_square(lbg,lbn,xorGC);

	  if (oldset)
	    XCopyArea(display,backg,rootwin,nmainGC,
		      0,0,BITMAP_WIDTH,BITMAP_HEIGHT,lx,ly);

	}

      if (((keys & ~lastmask) & Button2Mask)&&(group >= 0))
	{
	  if (IS_VALID_OBJ(group,newnum))
	    put_obj_in_cut(objdef[group][newnum]);
	  copy_obj_to_group(objdef[win][num],group,newnum);
	}

      if (((keys & ~lastmask) & Button3Mask)&&(group >= 0))
	copy_obj_to_group(objdef[win][num],group,num_objects[group]);

      if ((x != lx)||(y != ly)||(force_redraw))
	{
	  if (((lbg != group)||(lbn != newnum))&&(group >= 0))
	    box_grab_square(group,newnum,xorGC);

	  XCopyArea(display,rootwin,backg,nmainGC,
		    x,y,BITMAP_WIDTH,BITMAP_HEIGHT,0,0);
	  XCopyArea(display,mask,rootwin,nmaskGC,
		    0,0,BITMAP_WIDTH,BITMAP_HEIGHT,x,y);
	  XCopyArea(display,obj,rootwin,ndrawGC,
		    0,0,BITMAP_WIDTH,BITMAP_HEIGHT,x,y);
	}

      lx = x;
      ly = y;
      lbg = group;
      lbn = newnum;
      oldset = True;
      lastmask =  keys;
      force_redraw = False;
    }
  while(keys & Button1Mask);

  XCopyArea(display,backg,rootwin,nmainGC,
	    0,0,BITMAP_WIDTH,BITMAP_HEIGHT,x,y);

  if (group >= 0)
    box_grab_square(group,newnum,xorGC);

  XFreeGC(display,nmaskGC);
  XFreeGC(display,ndrawGC);
  XFreeGC(display,nmainGC);

  XFreePixmap(display,obj);
  XFreePixmap(display,mask);
  XFreePixmap(display,backg);

  return(retvalue);
}

/*****************************************************************/
E 1
