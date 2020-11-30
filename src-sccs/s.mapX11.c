h36000
s 00300/00000/00000
d D 1.1 92/08/27 20:29:51 vbo 1 0
c date and time created 92/08/27 20:29:51 by vbo
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

/* Output routines for window routines that refer to map information */
/* This code is separated so as not to be included in obtor */

#include "def.h"
#include "windowsX11.h"
#include "mapfunc.h"
#include "plot_order.h"


/* clear and write in the map name, given an area in which to do it */

place_map_name(win, x1, y1, x2, y2)
Window win;
{
  clear_area(win, x1, y1, x2, y2);
  place_a_string(win, mapstats.name, x1, y1, x2, y2);
}



/* clear and write in the room name, given an area to do it in */

place_room_name(win, x1, y1, x2, y2)
Window win;
{
  clear_area(win, x1, y1, x2, y2);
  place_a_string(win, room[current_room].name, x1, y1, x2, y2);
}



/* clear and write the team name that 'owns' this room */

place_room_team(roomnum, win, x1, y1, x2, y2)
int roomnum;
Window win;
int x1, y1, x2, y2;
{
  clear_area(win, x1, y1, x2, y2);
  if (mapstats.team_name[room[roomnum].team])
    place_a_string(win, mapstats.team_name[room[roomnum].team],
		   x1, y1, x2, y2);
}


/* redraw the window where the room is represented, pass TRUE for
   area_exposure if this redraw request is from an XExposeEvent.  When
   it is from an XExposeEvent we will try to be clever and only redraw
   the area requested in the event.  We can get info about the last event
   from the global variable last_event. */

redraw_room_window(area_exposure)
int area_exposure;
{
  register int i;
  int j = ROOM_HEIGHT * ROOM_WIDTH;
  XExposeEvent *evt;

  /* if current_room is -1 then don't redraw as per normal */
  if (current_room == -1) return;

  evt = (XExposeEvent *) &last_event;

  /* if this was an exposure event, redraw only part */
  if (area_exposure && evt->type == Expose)
    redraw_room_pixel_area(evt->x, evt->y, evt->width, evt->height);

  /* but if area_exposure is FALSE redraw all, probably a room change */
  else {
    /* set the background to be like the dominant floor tile in this room */
    XSetWindowBackgroundPixmap(display, roomwin,
		      obj_pixmap[(unsigned char)room[current_room].floor]);
    XClearWindow(display, roomwin);
    for (i=0; i<j; i++) draw_room_square(plotx[i], ploty[i], TRUE);
  }
}



/* Redraw a given pixel area in the room.  That means find the smallest area
   of character blocks that contains these pixels and redraw them. */

redraw_room_pixel_area(x, y, width, height)
int x, y, width, height;
{
  int x1, y1, x2, y2, i, j;

  /* assign the character block coordinates and check for oversize region */
  x1 = CONVERTRX(x);
  y1 = CONVERTRY(y);
  x2 = CONVERTRX((x+width));
  x2 = (x2 >= ROOM_WIDTH) ? (ROOM_WIDTH-1) : x2;
  y2 = CONVERTRY((y+height));
  y2 = (y2 >= ROOM_HEIGHT) ? (ROOM_HEIGHT-1) : y2;
  

  /* redraw the appropriate squares */
  for (i=x1; i<=x2; i++)
    for (j=y1; j<=y2; j++)
      draw_room_square(i, j, TRUE);
}



put_down_obj(wx, wy, n)
/* This procedure puts an object on the map, it will determine the
   spot x and y and pass these on to put_down_object(). */
int wx, wy;
unsigned char n;
{
  int x, y;

  /* convert window pixel x and y to spot x and y */
  x = CONVERTRX(wx);
  y = CONVERTRY(wy);

  put_down_object(x, y, n);

  /* redraw the square we changed */
  draw_room_square(x, y, FALSE);
}



clear_square(x, y)
/* this procedure clears a square of all objects and redraws the square */
int x, y;
{
  destroy_all_objects_on_square(&mapstats, current_room, x, y);
  draw_room_square(x, y, FALSE);
}



clear_masked_in_square(x, y)
/* this procedure clears a square of all masked objects, and redraws
   the square */
int x, y;
{
  destroy_objects_on_square_masked(&mapstats, current_room, x, y, TRUE);
  draw_room_square(x, y, FALSE);
}



clear_room()
/* clears all the squares in a room */
{
  int i, j;

  for (j=0; j<ROOM_HEIGHT; j++)
    for (i=0; i<ROOM_WIDTH; i++)
      clear_square(i, j);
}



draw_objects_on_square(square, win, x, y, was_cleared)
/* draws all the objects found on a particular square in the given window
   at given x and y location */
SquareRec *square;
Window win;
int x, y, was_cleared;
{
  OI *ptr;

  /* draw the last of the unmasked objects, unless it's the same as
     the background floor.  If there isn't one, and the square hasn't been
     cleared, then draw the background floor on it.  If the background is
     something other than zero (the blank) and there is no floor here,
     then the spot will have to be zeroed out. */
  ptr = square_last_masked(square, FALSE);
  if (ptr && !(was_cleared && ptr->type == room[current_room].floor))
    draw_object(ptr, win, x, y);
  else if ((!ptr && !was_cleared) ||
	   (!ptr && was_cleared && room[current_room].floor))
    place_obj_pixmap(win, x, y, 0, FALSE);

  /* draw the rest of the objects onto the spot */
  if (!ptr) ptr = square->first;
  else ptr = ptr->next;

  for (; ptr; ptr = ptr->next)
    draw_object(ptr, win, x, y);
}



draw_room_square(x, y, was_cleared)
/* Draw a single square of a room, if the square was recently cleared
   then skip the drawing of dominating floor character. */
int x, y, was_cleared;
{
  int wx, wy;
  SquareRec *square;

  /* get the information on square */
  square = get_square(&mapstats, current_room, x, y);
  if (!square) return;

  /* find proper draw spot amd draw square */
  wx = PIXELX(x);
  wy = PIXELY(y);
  draw_objects_on_square(square, roomwin, wx, wy, was_cleared);
}



erase_room_square(x, y)
/* clear a square to blank (the zero object pixmap) */
int x, y;
{
  int wx, wy;			/* pixel coords of draw spot */

  /* check for invalid room number */
  if (current_room < 0) return;
  
  /* find proper draw spot */
  wx = PIXELX(x);
  wy = PIXELY(y);

  place_obj_pixmap(roomwin, wx, wy, 0, FALSE);
}

/*****************************************************************/
/* select an item from a given menu by number.  Top item is number
   one, second is number two etc.  This procedure prints the menu
   and returns a valid choice.  Max number items in menu is 9.*/

int menu_query(line, num_lines, num_options, do_own_redraw)
char *line[];		/* array of display lines */
int num_lines;		/* num lines to display */
int num_options;	/* valid choices: 1 thru num_options
int do_own_redraw;	/* is TRUE if we clean up after ourselves */
{
  int  pixel_width, pixel_height, longest, i, result;
  Help *help_window;
  char c;

  /* figure out what the longest line is */
  for (i=0,longest=1; i<num_lines; i++)
    if (strlen(line[i]) > longest) longest = strlen(line[i]);

  /* figure out how big window has to be */
  pixel_width = char_width(regfont) * longest + WINDOW_BORDERS*2;
  pixel_height = char_height(regfont) + WINDOW_BORDERS*2;

  /* take care that it's not too wide */
  if (pixel_width > ROOM_PIXELS_WIDE) pixel_width = ROOM_PIXELS_WIDE;

  /* make a help window to display menu in and get it displayed */
  help_window = make_help_window(roomwin, (ROOM_PIXELS_WIDE - pixel_width)/2,
				 (ROOM_PIXELS_HIGH - pixel_height)/2,
				 longest, num_lines, regfont);
  for (i=0; i<num_lines; i++) set_line(help_window, i, line[i]);
  show_help_window(help_window);
  init_input_lines(help_window->win);

  /* wait for a keypress which is a valid result */
  do {
    while (!get_help_key_press(help_window, &c));

    result = c - '0';
  } while (result < 1 || result > num_options);

  /* make the help window go away */
  destroy_help_window(help_window);

  /* redraw the part of the room window that was destroyed */
  if (do_own_redraw)
    redraw_room_pixel_area((ROOM_PIXELS_WIDE - pixel_width)/2,
			   (ROOM_PIXELS_HIGH - pixel_height)/2,
			   pixel_width, pixel_height);

  /* return our result */
  return result;
}

/*****************************************************************/
E 1
