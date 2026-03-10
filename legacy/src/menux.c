#include <stdio.h>
#include <X11/Xlib.h>
#include <string.h>
#include "standardx.h"
#include "lib.h"
#include "def.h"
#include "password.h"
#include "xx.h"
#include "menux.h"
  
  void display_menu (disp)
disp_fields	disp;
{
  int x,y,counter;
  x = 0;
  y = 0;
  XClearWindow (disp->display, disp->menu_win->win);
  /* Draw horizontal line(s) */
  counter = Y_DIVISIONS;
  while (--counter)
    {
      y += Y_SEP;
      XDrawLine (disp->display, disp->menu_win->win, disp->regular_gc,x, y, 
		 x + MENU_WINDOW_WIDTH,y); 
    }
  /* Draw vertical line(s) */
  counter = X_DIVISIONS;
  y = 0;
  while (--counter)
    {
      x += X_SEP;
      XDrawLine (disp->display, disp->menu_win->win, disp->regular_gc,x,y,
		 x, y + MENU_WINDOW_HEIGHT);
    }
  XSetFont (disp->display, disp->regular_gc, disp->regfont->fid);
  XDrawString (disp->display, disp->menu_win->win, disp->regular_gc,
	       5, (int) (.8 * Y_SEP), "Load Bitmap File",
	       strlen ("Load Bitmap File"));
  XDrawString (disp->display, disp->menu_win->win, disp->regular_gc,
	       5 + X_SEP, (int) (.8 * Y_SEP),
	       "No", strlen ("No"));
  XDrawString (disp->display, disp->menu_win->win, disp->regular_gc,
	       5 + 2 * X_SEP, (int) (.8 * Y_SEP),
	       "Save Bitmap File", strlen ("Save Bitmap File"));
  XDrawString (disp->display, disp->menu_win->win, disp->regular_gc,
	       5, (int) (1.8 * Y_SEP), "Load Mask File", 
	       strlen ("Load Mask File"));
  XDrawString (disp->display, disp->menu_win->win, disp->regular_gc,
	       5 + X_SEP, (int) (1.8 * Y_SEP), "Yes", 
	       strlen ("Yes"));
  XDrawString (disp->display, disp->menu_win->win, disp->regular_gc,
	       5 + 2 * X_SEP, (int) (1.8 * Y_SEP), "Save Mask File", 
	       strlen ("Save Mask File"));
}

int area (x,y,disp)
     int x,y;
     disp_fields disp;
{
  int area,counter = 1;
  area = disp->menu_win->num_selections;
  if (y <= Y_SEP) area = area - X_DIVISIONS;
  while (x <= (MENU_WINDOW_WIDTH - (counter * X_SEP))) {counter++; area--;}
  return area;
}

int handle_menu_area_cursor (x,y,disp,value)
     int x,y;
     disp_fields disp;
     int value;
{
  static int last_area,last_x1,last_x2,last_y1,last_y2;
  int new_area;
  int x1,y1;
  
  new_area = area(x,y,disp);
  
  if ((new_area == last_area) || (value == RETURN_AREA)) return new_area;
  
  x1 = 0; y1 = 0;
  if (new_area > X_DIVISIONS) {y1 += Y_SEP + 1; new_area -= X_DIVISIONS;}
  x1 += X_SEP * (new_area - 1);
  new_area = area(x,y,disp);
  
  if ((new_area != last_area) || (value == LEAVING))
    {
      /* First invert whatever area we last came into */
      if (last_area)
	XCopyArea (disp->display, disp->menu_win->win,disp->menu_win->win,
		   disp->inversion_gc, last_x1, last_y1, 
		   X_SEP, Y_SEP,
		   last_x1, last_y1);
      if (value == EXPOSE_MENU) return;
      else if (value == LEAVING) {last_area = 0; return;}
      if (new_area)
	XCopyArea (disp->display, disp->menu_win->win,disp->menu_win->win,
		   disp->inversion_gc, x1, y1,
		   X_SEP, Y_SEP,
		   x1, y1);
      last_x1 = x1; last_y1 = y1;
    }
  last_area = new_area;
  if (value == RETURN_AREA) return new_area;
}

void setup_menu(disp)
     disp_fields disp;
{
  disp->menu_win->num_selections = NUM_ITEMS;
}

void handle_message (disp,flag)
     disp_fields	disp;
     boolean		flag;
{
  XClearArea (disp->display, disp->win, MESSAGE_X, BITM_WINDOW_Y - 25, WINDOW_WIDTH - 10,
	      BITM_WINDOW_Y - 25,FALSE);
  if (flag)
    {
      text_write (disp->win, disp->bigfont, MESSAGE_X, MESSAGE_Y,
		  0,0,disp->message, disp->display, disp->regular_gc);
      disp->message_there = TRUE;
    }
  else
    {
      disp->message_there = FALSE;
      strcpy (disp->message, "");
    }
}

void handle_menu_options (x,y,disp)
     int x,y;
     disp_fields	disp;
{
  int Area;
  char input_text[MAX_TEXT_LINE_LENGHT];
  int beenexposed;
  
  Area = area (x,y,disp);
  switch (Area) {
    
  case 1: /* Load Bitmap File */
    {
      if (load_bitmap_mask (disp, TRUE))
        XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
      break;
    }
    
  case 2: /* Cancel */
    {
      handle_confirm_cancel (disp, FALSE);
      break;
    }
    
  case 3: /* Save bitmap file */
    {
      if (!(strcmp (disp->bm_file, "None")))
	{
	  strcpy (disp->message, "No Bitmap File Name");
	  handle_message (disp, TRUE);
	  break;
	}
      else if (!(disp->memory_bm))
	{
	  strcpy (disp->message, "No Bitmap In Memory");
	  handle_message (disp, TRUE);
	  break;
	}
      else
	{
	  XWriteBitmapFile (disp->display, disp->bm_file,
			    disp->memory_bitmap, 32,32,0,0);
	  strcpy (disp->message, "Bitmap Saved");
	  handle_message (disp, TRUE);
	  break;
	}
    }
  case 4: /* Load Mask File */
    {
      if (load_bitmap_mask (disp, FALSE))
        XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
      break;
    }
    
  case 5: /* Confirm */
    {
      handle_confirm_cancel (disp, TRUE);
      break;
    }
  case 6: /* Save mask file */
    {
      if (!(strcmp (disp->bm_m_file, "None")))
	{
	  strcpy (disp->message, "No Mask File Name");
	  handle_message (disp, TRUE);
	  break;
	}
      else if (!(disp->memory_m))
	{
	  strcpy (disp->message, "No Mask In Memory");
	  handle_message (disp, TRUE);
	  break;
	}
      else
	{
	  XWriteBitmapFile (disp->display, disp->bm_m_file,
			    disp->memory_mask, 32,32,0,0);
	  strcpy (disp->message, "Mask Saved");
	  handle_message (disp, TRUE);
	  break;
	}
    }
  }
}

void redraw_menu_win (disp)
     disp_fields	disp;
{
  display_menu (disp);
  handle_menu_area_cursor (0,0,disp,EXPOSE_MENU);
}
