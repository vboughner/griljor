
#include "X11/cursorfont.h"

#include "obtor.h"


/*****************************************************************/
  
drag_object(win,x,y)
int win,x,y;
{
  int num,result,newwin,newnum;
  Cursor old;
  Window root,child;
  int rx,ry,hasused,cx,cy;
  unsigned int keys;
  ObjInfo *temp;
  XEvent throwaway;
  
  num = obtor_obj_in_grab_win(x, y,win);

  if (!IS_VALID_OBJ(win, num))
    return(False);

  XQueryPointer(display,objwin[win],&root,&child,&rx,&ry,&cx,&cy,&keys);
  TempUnmapArrowWindows(win);
  XDefineCursor(display,obtorwin,DragCursor);
  XGrabServer(display); 
  box_grab_square(win,num,mainGC);
  hasused = drag_bitmap(win,num,cx,cy);
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
		  adj_switch_all(newwin,num,newnum);
		}
	      else
		copy_obj_to_group(objdef[win][num],newwin,newnum);
	    }
	  else if (newwin != win)
	    {
	      insert_obj_in_group(copy_obj_definition(objdef[win][num]),
				  newwin,newnum);
	      adj_clear(newwin,newnum);
	    }
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

drag_bitmap(win,num,clickx,clicky)
int win,num,clickx,clicky;
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
  dx = x - clickx;
  dy = y - clicky;

  obj = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,depth);
  mask = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,depth);
  backg = XCreatePixmap(display,rootwin,BITMAP_WIDTH,BITMAP_HEIGHT,depth);

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
