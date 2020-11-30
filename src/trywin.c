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
			      TRY_SIZE*BITMAP_WIDTH,
			      TRY_SIZE*BITMAP_HEIGHT,depth);
  try_masks = XCreatePixmap(display,trywin,
			    TRY_SIZE*BITMAP_WIDTH,
			    TRY_SIZE*BITMAP_HEIGHT,depth);
		
  XFillRectangle(display,try_pixmaps,inverseGC,0,0,TRY_SIZE*BITMAP_WIDTH,
		 TRY_SIZE*BITMAP_WIDTH);
  XFillRectangle(display,try_masks,inverseGC,0,0,TRY_SIZE*BITMAP_WIDTH,
		 TRY_SIZE*BITMAP_WIDTH);
}

/*****************************************************************/
