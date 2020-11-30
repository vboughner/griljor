h53959
s 00000/00000/00086
d D 1.3 92/08/07 01:04:23 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00004/00003/00082
d D 1.2 91/10/16 20:06:42 labc-4lc 2 1
c changed made for DecStations
e
s 00085/00000/00000
d D 1.1 91/03/24 18:45:34 labc-3id 1 0
c date and time created 91/03/24 18:45:34 by labc-3id
e
u
U
f e 0
t
T
I 1
D 2

E 2
#include "X11/cursorfont.h"

#include "obtor.h"


/*****************************************************************/

toggle_try_window()
{
  if (try_win_up)
    {
      try_win_up = False;
      XUnmapWindow(display,trywin);
    }
  else
    {
      try_win_up = True;
      XMapRaised(display,trywin);
    }
}

/*****************************************************************/

draw_try_window()
{
  int i,j;
  
  XCopyArea(display,try_masks,trywin,maskGC,
	    0,0,TRY_SIZE*BITMAP_WIDTH,TRY_SIZE*BITMAP_WIDTH,0,0);
  XCopyArea(display,try_pixmaps,trywin,drawGC,
	    0,0,TRY_SIZE*BITMAP_WIDTH,TRY_SIZE*BITMAP_WIDTH,0,0);
}


/*****************************************************************/

try_drop(group,num,x,y)
{
  x = x/BITMAP_WIDTH;
  y = y/BITMAP_HEIGHT;

  if ((group >= 0)&&(x>=0)&&(y>=0))
    if (IS_VALID_OBJ(group,num))
      {
	if ((x == 0)&&(y == 0))
	  {
	    XSetWindowBackgroundPixmap(display,trywin,item_pixmap[group][num]);
	    XClearArea(display,trywin,0,0,0,0,True);
	  }
	else if ((x < TRY_SIZE)&&(y < TRY_SIZE))
	  {
	    XCopyArea(display,item_pixmap[group][num],try_pixmaps,mainGC,
		      0,0,BITMAP_WIDTH,BITMAP_HEIGHT,
		      x*BITMAP_WIDTH,y*BITMAP_HEIGHT);
	    
	    if (objdef[group][num]->masked)
	      XCopyArea(display,item_mask[group][num],try_masks,mainGC,
			0,0,BITMAP_WIDTH,BITMAP_HEIGHT,
			x*BITMAP_WIDTH,y*BITMAP_HEIGHT);
	    else
	      XFillRectangle(display,try_masks,mainGC,
			     x*BITMAP_WIDTH,y*BITMAP_HEIGHT,
			     BITMAP_WIDTH,BITMAP_HEIGHT);
	  }
	XClearArea(display,trywin,0,0,0,0,True);
      }
}

/*****************************************************************/

init_try_pixmaps()
{
  try_pixmaps = XCreatePixmap(display,trywin,
D 2
			      TRY_SIZE*BITMAP_WIDTH,TRY_SIZE*BITMAP_HEIGHT,1);
E 2
I 2
			      TRY_SIZE*BITMAP_WIDTH,
			      TRY_SIZE*BITMAP_HEIGHT,depth);
E 2
  try_masks = XCreatePixmap(display,trywin,
D 2
			    TRY_SIZE*BITMAP_WIDTH,TRY_SIZE*BITMAP_HEIGHT,1);
E 2
I 2
			    TRY_SIZE*BITMAP_WIDTH,
			    TRY_SIZE*BITMAP_HEIGHT,depth);
E 2
		
  XFillRectangle(display,try_pixmaps,inverseGC,0,0,TRY_SIZE*BITMAP_WIDTH,
		 TRY_SIZE*BITMAP_WIDTH);
  XFillRectangle(display,try_masks,inverseGC,0,0,TRY_SIZE*BITMAP_WIDTH,
		 TRY_SIZE*BITMAP_WIDTH);
}

/*****************************************************************/
E 1
