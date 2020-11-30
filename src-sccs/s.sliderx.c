h00792
s 00000/00000/00122
d D 1.3 92/08/07 01:04:07 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00005/00004/00117
d D 1.2 92/01/20 16:25:03 labc-4lc 2 1
c changed references to bitmaps and fonts so they'd be in config.h
e
s 00121/00000/00000
d D 1.1 91/04/14 19:35:41 labc-3id 1 0
c date and time created 91/04/14 19:35:41 by labc-3id
e
u
U
f e 0
t
T
I 1
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "standardx.h"
#include "lib.h"
#include "def.h"
#include "password.h"
#include "sliderx.h"
I 2
#include "config.h"
E 2
  
  void load_all_bitmaps (disp)
disp_fields	disp;
{
  unsigned int dummy1, dummy2;
  int dummy3,dummy4;
  
  if (XReadBitmapFile (disp->display,
		       disp->win,
		       ICON_FILE_NAME, 
		       &dummy1, 
		       &dummy2,
		       &(disp->icon),
		       &dummy3,
		       &dummy4) != BitmapSuccess)
    handle_error (FATAL, "Could not load bitmap for the icon");
  
  if (XReadBitmapFile (disp->display,
		       disp->slider_win->win,
		       SLIDER_FILE_NAME, 
		       &dummy1, 
		       &dummy2,
		       &(disp->slider_bitmap),
		       &dummy3,
		       &dummy4) != BitmapSuccess)
    handle_error (FATAL, "Could not load bitmap for the scrolling buttons.");
  
  if (XReadBitmapFile (disp->display,
		       disp->bitmap_win->win,
D 2
		       "bit/disc_icon.bit",
E 2
I 2
		       DISC_FILE_NAME,
E 2
		       &dummy1, 
		       &dummy2,
		       &(disp->disc_icon),
		       &dummy3,
		       &dummy4) != BitmapSuccess)
    handle_error (FATAL, "Could not load bitmap for the disc icon.");
  
  if (XReadBitmapFile (disp->display,
		       disp->bitmap_win->win,
D 2
		       "bit/grey.bit",
E 2
I 2
		       GREY_FILE_NAME,
E 2
		       &dummy1, 
		       &dummy2,
		       &(disp->grey),
		       &dummy3,
		       &dummy4) != BitmapSuccess)
    handle_error (FATAL, "Could not load bitmap for the background");
  
  if (XReadBitmapFile (disp->display,
		       disp->win,
D 2
		       "bit/pict.bit",
E 2
I 2
		       PICT_FILE_NAME,
E 2
		       &dummy1, 
		       &dummy2,
		       &(disp->pict),
		       &dummy3,
		       &dummy4) != BitmapSuccess)
    handle_error (FATAL, "Could not load bitmap for the bitmap pict");
  
  if (XReadBitmapFile (disp->display,
		       disp->win,
D 2
		       "bit/title.bit",
E 2
I 2
		       TITLE_FILE_NAME,
E 2
		       &dummy1, 
		       &dummy2,
		       &(disp->title),
		       &dummy3,
		       &dummy4) != BitmapSuccess)
    handle_error (FATAL, "Could not load bitmap for the title pict");
}

void draw_slider_bits (disp)
     disp_fields	disp;
{
  XCopyArea (disp->display, disp->slider_bitmap, disp->slider_win->win,
	     disp->regular_gc, 0,0, HALF_WAY_X_2, FULL_LENGHT,
	     0,0);
}

void display_slider (disp)
     disp_fields disp;
{
  draw_slider_bits (disp);
}

int area_slider (disp, x,y)
     disp_fields	disp;
     int x,y;
{
  if (y < UPPER_QUARTER_Y) return SCROLL_UP;
  else if (y < HALF_WAY_Y) return SCROLL_DOWN;
  else if (y < LOWER_QUARTER_Y) return SCROLL_UP_PAGE;
  else return SCROLL_DOWN_PAGE;
}

void flash_slider_area (disp,mouse_x,mouse_y)
     disp_fields	disp;
{
  int x = 1,y = 1,dummy,area;
  
  area = area_slider (disp, mouse_x,mouse_y);
  if (area == SCROLL_DOWN) y+= UPPER_QUARTER_Y;
  else if (area == SCROLL_UP_PAGE) y+= HALF_WAY_Y;
  else if (area == SCROLL_DOWN_PAGE) y+= LOWER_QUARTER_Y;
  
  XCopyArea (disp->display, disp->slider_win->win,disp->slider_win->win,
	     disp->inversion_gc, x,y,
	     SLIDER_BUTTON_WIDTH,SLIDER_BUTTON_HEIGHT,
	     x,y);
}

redraw_slid_win (disp)
     disp_fields disp;
{
  draw_slider_bits (disp);
}
E 1
