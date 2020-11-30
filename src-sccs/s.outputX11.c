h40224
s 00001/00219/00766
d D 1.20 92/08/27 20:28:34 vbo 20 19
c removed map related functions to mapX11.c
e
s 00000/00000/00985
d D 1.19 92/08/07 20:55:22 vbo 19 18
c fixes for sun port merged in
e
s 00009/00000/00976
d D 1.18 92/03/11 20:53:25 vanb 18 17
c added to structures for containers, fixed more bugs
e
s 00028/00016/00948
d D 1.17 92/03/02 22:07:50 vanb 17 16
c finished getting editmap to work on basics
e
s 00056/00110/00908
d D 1.16 92/02/15 01:07:26 vanb 16 15
c 
e
s 00002/00003/01016
d D 1.15 91/12/07 17:37:42 labc-4lc 15 14
c made error message window bigger and scrollable
e
s 00054/00017/00965
d D 1.14 91/11/25 23:03:24 labc-4lc 14 13
c fixed color problems drawing objects
e
s 00006/00003/00976
d D 1.13 91/11/25 20:43:22 labc-4lc 13 12
c attempted fix of color/mono object drawing problem
e
s 00020/00011/00959
d D 1.12 91/10/16 20:06:10 labc-4lc 12 11
c changed made for DecStations
e
s 00000/00000/00970
d D 1.11 91/08/28 21:10:06 vanb 11 10
c made editmap work in color X windows
e
s 00009/00009/00961
d D 1.10 91/08/03 21:01:56 labc-3id 10 9
c 
e
s 00001/00000/00969
d D 1.9 91/08/03 16:27:24 labc-3id 9 8
c fixed a small problem with large n in obj_place_pixmap()
e
s 00004/00004/00965
d D 1.8 91/06/16 14:15:56 labc-3id 8 7
c trevor was making some color graphics changes
e
s 00027/00001/00942
d D 1.7 91/05/17 03:32:06 labc-3id 7 6
c added cool make_popup_help_window procedure
e
s 00018/00000/00925
d D 1.6 91/04/14 19:52:08 labc-3id 6 5
c 
e
s 00002/00002/00923
d D 1.5 91/03/24 18:20:43 labc-3id 5 4
c 
e
s 00001/00001/00924
d D 1.4 91/03/23 00:43:23 labc-3id 4 3
c 
e
s 00002/00004/00923
d D 1.3 91/02/23 01:48:57 labc-3id 3 2
c 
e
s 00002/00000/00925
d D 1.2 91/02/18 21:14:14 labc-3id 2 1
c 
e
s 00925/00000/00000
d D 1.1 91/02/16 12:55:18 labc-3id 1 0
c date and time created 91/02/16 12:55:18 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 **************************************************************************/

/* Output Routines for window system */

#include "def.h"
#include "windowsX11.h"
I 16
D 20
#include "mapfunc.h"
E 16
#include "plot_order.h"
E 20

I 20

E 20
/* place the picture of a mouse (computer mouse) in given window at x y */

place_mouse_picture(win, x, y)
Window win;
int x, y;
{
  /* place the mouse picture image */
D 8
  XCopyArea(display, mouse_pix, win, mainGC, 0, 0,
	    mouse_width, mouse_height, x, y);
E 8
I 8
D 10
  XCopyPlane(display, mouse_pix, win, mainGC, 0, 0,
	      mouse_width, mouse_height, x, y,1);
E 10
I 10
  XCopyArea(display, mouse_pix, win, mainGC, 0, 0,
	    mouse_width, mouse_height, x, y);
E 10
E 8
}


/* place one of the blocks inside mouse picture, given window, x, and y
   of the placement of the mouse image previously, blocks are numbered
   0, 1, and 2  */

place_mouse_block(win, x, y, blocknum, objnum)
Window win;
int x, y, blocknum, objnum;
{
  int wx, wy;

  /* find the proper place to put the block */
  wx = x + MOUSE_START_X + blocknum * (BITMAP_WIDTH + MOUSE_SPACING);
  wy = y + MOUSE_START_Y;

  /* place the given object at the spot */
D 14
  XCopyArea(display, obj_pixmap[(unsigned char)objnum], win, mainGC, 
	    0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy);
E 14
I 14
D 16
  DrawObject(win, obj_pixmap[(uc)objnum], NULL, NULL, wx, wy);
E 16
I 16
  DrawObject(win, obj_pixmap[objnum], NULL, NULL, wx, wy);
E 16
E 14
}



/* place numbers in the corners of an object just placed in mouse picture,
   x and y are to be given as the upper left of mouse picture bitmap. */

place_mouse_item_number(win, x, y, blocknum, which_corner, number)
Window win;
int x, y, blocknum, which_corner, number;
{
  int wx, wy;

  /* find the proper place to put the block number */
  wx = x + MOUSE_START_X + blocknum * (BITMAP_WIDTH + MOUSE_SPACING);
  wy = y + MOUSE_START_Y;

  /* place number in corner */
  place_corner_number(win, wx, wy, which_corner, number);
}



/* place a string given a box x1, y1, x2, y2, in which to place it.
   will choose correctly whether to use big font or reg font, and will
   start it from the left side and top of the given box */

place_a_string(win, line, x1, y1, x2, y2)
Window	win;
char	*line;
int	x1, y1, x2, y2;
{
  /* check to see if big font will fit */
  if (text_width(bigfont, line) <= (x2 - x1) &&
      text_height(bigfont) <= (y2 - y1))
	text_write(win, bigfont, x1, y1, 0, 0, line);
  else  text_write(win, regfont, x1, y1, 0, 0, line);
}



/* clear a boxed area of whatever it contained */

clear_area(win, x1, y1, x2, y2)
Window win;
{
  XClearArea(display, win, x1, y1, (x2-x1+1), (y2-y1+1), FALSE);
}



D 20
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
D 16
  char s[40];

  /* determine what string should contain */
  switch (room[roomnum].team) {
	case 0:	strcpy(s, "neutral");
		break;
	default:strcpy(s, mapstats.team_name[room[roomnum].team -1]);
		break;
  }

  /* place that string in given area */
E 16
  clear_area(win, x1, y1, x2, y2);
D 16
  place_a_string(win, s, x1, y1, x2, y2);
E 16
I 16
  if (mapstats.team_name[room[roomnum].team])
    place_a_string(win, mapstats.team_name[room[roomnum].team],
		   x1, y1, x2, y2);
E 16
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

D 4
  /* but if this is a normal request (probably change room) redraw all */
E 4
I 4
  /* but if area_exposure is FALSE redraw all, probably a room change */
E 4
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



D 16
/* This procedure puts an object on the map, it will determine the
   spot x and y and pass these on to put_down_mapped_obj or to
   put_down_recorded_obj as appropriate */

E 16
put_down_obj(wx, wy, n)
I 16
/* This procedure puts an object on the map, it will determine the
   spot x and y and pass these on to put_down_object(). */
E 16
int wx, wy;
unsigned char n;
{
  int x, y;

  /* convert window pixel x and y to spot x and y */
  x = CONVERTRX(wx);
  y = CONVERTRY(wy);

D 16
  if (info[(unsigned char)n]->recorded) put_down_recorded_obj(x, y, n);
  else put_down_mapped_obj(x, y, n);
E 16
I 16
  put_down_object(x, y, n);
E 16

  /* redraw the square we changed */
  draw_room_square(x, y, FALSE);
}



D 16

/* this procedure clears a square of all objects, placing obj #0 in all
   spots in the square and redraws the square */

E 16
clear_square(x, y)
I 16
/* this procedure clears a square of all objects and redraws the square */
E 16
int x, y;
{
D 16
  register int i;

  /* clear mapped objects */
  for (i=0; i<ROOM_DEPTH; i++)
    room[current_room].spot[x][y][i] = 0;

  /* clear a possible recorded object */
  remove_recorded_obj(current_room, x, y);
  
E 16
I 16
  destroy_all_objects_on_square(&mapstats, current_room, x, y);
E 16
  draw_room_square(x, y, FALSE);
}



D 16
/* this procedure clears a square of all masked objects, placing obj #0 in all
   those spots in the square and redraws the square */

E 16
clear_masked_in_square(x, y)
I 16
/* this procedure clears a square of all masked objects, and redraws
   the square */
E 16
int x, y;
{
D 16
  register int i;

  /* clear mapped and masked objects */
  for (i=0; i<ROOM_DEPTH; i++)
    if (info[(unsigned char)room[current_room].spot[x][y][i]]->masked)
      room[current_room].spot[x][y][i] = 0;

  /* clear a possible recorded object */
  remove_recorded_obj(current_room, x, y);
  
E 16
I 16
  destroy_objects_on_square_masked(&mapstats, current_room, x, y, TRUE);
E 16
  draw_room_square(x, y, FALSE);
}



D 16
/* clears all the squares in a room */

E 16
clear_room()
I 16
/* clears all the squares in a room */
E 16
{
D 16
  register int i, j;
E 16
I 16
  int i, j;
E 16

  for (j=0; j<ROOM_HEIGHT; j++)
    for (i=0; i<ROOM_WIDTH; i++)
      clear_square(i, j);
}



E 20
D 16
/* Draw a single square of a room, juggle around the objects so that
   masks work out right, if the window recently "was_cleared" then skip
   the drawing of floor characters */
E 16
I 16
void draw_object(obj, win, x, y)
/* sees to the drawing of a particular object in the right window.  Checks
   for the existence of a special pixmap pair for this object instance. */
OI *obj;
Window win;
int x, y;
{
  /*** COMPLETE: look for instance pixmap ***/
  place_obj_pixmap(win, x, y, obj->type, FALSE);
D 20
}
E 16

I 16


E 16
D 17
draw_room_square(x, y, was_cleared)
I 16
/* Draw a single square of a room, if the square was recently cleared
   then skip the drawing of dominating floor character. */
E 17
I 17
draw_objects_on_square(square, win, x, y, was_cleared)
/* draws all the objects found on a particular square in the given window
   at given x and y location */
SquareRec *square;
Window win;
E 17
E 16
int x, y, was_cleared;
{
D 16
  register int i;
  int floor;			/* current floor character */
  int num_items;		/* num of items on one spot */
  int unmasked = -1;
  int wx, wy;			/* pixel coords of draw spot */
  unsigned char *spot_content;	/* objects on this spot */
  int skip_this_mask;		/* do we skip drawing this object's mask? */
  int one_skipped = 0;		/* becomes 1 when mask draw is skipped once */
E 16
I 16
  OI *ptr;
D 17
  int wx, wy;
  SquareRec *square;
E 17
E 16

D 16
  /* check for invalid room number */
  if (current_room < 0) return;
  
E 16
I 16
D 17
  /* get the information on square */
  square = get_square(&mapstats, current_room, x, y);
  if (!square) return;

E 16
  /* find proper draw spot */
  wx = PIXELX(x);
  wy = PIXELY(y);

E 17
D 16
  /* figure out current floor */
  floor = room[current_room].floor;
E 16
I 16
  /* draw the last of the unmasked objects, unless it's the same as
     the background floor.  If there isn't one, and the square hasn't been
     cleared, then draw the background floor on it.  If the background is
     something other than zero (the blank) and there is no floor here,
     then the spot will have to be zeroed out. */
  ptr = square_last_masked(square, FALSE);
  if (ptr && !(was_cleared && ptr->type == room[current_room].floor))
D 17
    draw_object(ptr, roomwin, wx, wy);
E 17
I 17
    draw_object(ptr, win, x, y);
E 17
  else if ((!ptr && !was_cleared) ||
	   (!ptr && was_cleared && room[current_room].floor))
D 17
    place_obj_pixmap(roomwin, wx, wy, 0, FALSE);
E 17
I 17
    place_obj_pixmap(win, x, y, 0, FALSE);
E 17
E 16

D 16
  /* get the array of things on the spot */
  spot_content = whats_on_square(current_room, x, y);
  num_items = ROOM_DEPTH + 1;
  
  /* find which is the first object on the spot that is not masked, or
     if the first unmasked object is #0, look for a better one */
  for (i=0; i<num_items; i++) {
    if (unmasked == -1) {
      if (!info[(unsigned char)spot_content[i]]->masked) unmasked = i;
    }
    else {
      if (spot_content[unmasked] == 0 &&
D 3
	     !info[(unsigned char)spot_content[i]]->masked &&
	     spot_content[i] != 0)  unmasked = i;
E 3
I 3
D 5
	  !info[(unsigned char)spot_content[i]]->masked)
	unmasked = i;
E 5
I 5
	  !info[(unsigned char)spot_content[i]]->masked &&
	  spot_content[i] != 0)  unmasked = i;
E 5
E 3
    }
  }
E 16
I 16
  /* draw the rest of the objects onto the spot */
  if (!ptr) ptr = square->first;
  else ptr = ptr->next;
E 16

D 16
  /* if there was no unmasked char then draw first in list first,
     but before that, we might have to clear the square */
  if (unmasked == -1) {
    unmasked = 0;
    if (!was_cleared) place_obj_pixmap(roomwin, wx, wy, 0, FALSE);
  }

  /* draw the unmasked character onto spot, skipping the opportunity if it
     is a floor character and the window was just cleared */
  if ((was_cleared && spot_content[unmasked] != floor) || (!was_cleared))
    place_obj_pixmap(roomwin, wx, wy, spot_content[unmasked], FALSE);

  /* draw the rest of the objects onto the spot (skip unmasked ones)
      if the unmasked one was object #0 then skip drawing the mask 
      for the first masked object */
  for (i=0; i<num_items; i++)
D 10
    if (info[(unsigned char)spot_content[i]]->masked) {
E 10
I 10
      if (info[(unsigned char)spot_content[i]]->masked) {
E 10

D 10
      /* figure out whether to skip drawing mask for this object */
      skip_this_mask = (!one_skipped && spot_content[unmasked]==0);
      one_skipped  = one_skipped ? one_skipped : skip_this_mask;
E 10
I 10
        /* figure out whether to skip drawing mask for this object */
        skip_this_mask = (!one_skipped && spot_content[unmasked]==0);
        one_skipped  = one_skipped ? one_skipped : skip_this_mask;
E 10

D 10
      /* place object according to information we have */
      place_obj_pixmap(roomwin, wx, wy, spot_content[i], skip_this_mask);
    }
E 10
I 10
        /* place object according to information we have */
        place_obj_pixmap(roomwin, wx, wy, spot_content[i], skip_this_mask);
      }
E 16
I 16
  for (; ptr; ptr = ptr->next)
D 17
    draw_object(ptr, roomwin, wx, wy);
E 17
I 17
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
E 17
E 16
E 10
}



I 6
D 16
/* clear a square to blank (the zero object pixmap) */

E 16
erase_room_square(x, y)
I 16
/* clear a square to blank (the zero object pixmap) */
E 16
int x, y;
{
  int wx, wy;			/* pixel coords of draw spot */

  /* check for invalid room number */
  if (current_room < 0) return;
  
  /* find proper draw spot */
  wx = PIXELX(x);
  wy = PIXELY(y);

  place_obj_pixmap(roomwin, wx, wy, 0, FALSE);
E 20
}


E 6

/* place an object pixmap on screen, if there is a mask then place it
   on first.  You must give the window and pixel x and y coords to
   draw at.  You must specify the object number, and whether we can assume
   the spot underneath has nothing at all drawn on it already */

place_obj_pixmap(win, x, y, n, is_clear_under)
Window win;
int x, y, n, is_clear_under;
{
D 14
  int func;
E 14
I 9
  if (n<0 || n>=objects) return;
E 9

D 14
  /* if there is a mask but screen is clear underneath, skip the drawing
   * of the mask before drawing the object */
D 12
  if (!info[(unsigned char)n]->masked || is_clear_under) {
    XCopyArea(display, obj_pixmap[(unsigned char)n], win, mainGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
  }
  else {
    XCopyArea(display, obj_mask[(unsigned char)n], win, maskGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
    XCopyArea(display, obj_pixmap[(unsigned char)n], win, drawGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
  }
E 12
I 12
 
  if (!info[(unsigned char)n]->masked)
    {
      XCopyArea(display, obj_pixmap[(unsigned char)n], win, mainGC, 0, 0,
		BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
    }
  else 
    {
      DrawObjectMask(obj_mask[(unsigned char)n], win, x, y);
D 13
/*      XCopyArea(display, obj_pixmap[(unsigned char)n], win, xorGC, 0, 0,
		BITMAP_WIDTH, BITMAP_HEIGHT, x, y); */
E 13
I 13
      XCopyArea(display, obj_pixmap[(unsigned char)n], win, drawGC, 0, 0,
		BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
E 13
    }
E 14
I 14
D 16
  if (info[(unsigned char)n]->masked && !is_clear_under) {
    DrawObject(win, obj_pixmap[(uc)n], obj_mask[(uc)n], obj_clipGC[(uc)n],
E 16
I 16
  if (info[n]->masked && !is_clear_under) {
    DrawObject(win, obj_pixmap[n], obj_mask[n], obj_clipGC[n],
E 16
	       x, y);
  }
  else {
D 16
    DrawObject(win, obj_pixmap[(uc)n], NULL, NULL, x, y);
E 16
I 16
    DrawObject(win, obj_pixmap[n], NULL, NULL, x, y);
E 16
  }
E 14
E 12
}

I 13


E 13
D 12

E 12
I 12
DrawObjectMask(mask,dest,x,y)
Window mask,dest;
int x,y;
{
  XCopyArea (display, mask, dest, maskGC, 0, 0, 
	     BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
D 13
  
E 13
}
I 14



DrawObject(win, pix, mask, gc, x, y)
/* draws a possibly masked object into a window.  Mask should be zero
   if there isn't a mask.  gc should be NULL if there is no GC already
   established for this object. */
Window win;
Pixmap pix, mask;
GC gc;
int x, y;
{
  XGCValues values;
  static GC tempGC = NULL;
  if (!pix && !mask) return;

  if (!mask)
    {
      XCopyArea(display, pix, win, mainGC, 0, 0,
		BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
    }
  else 
    {
      if (MONOCHROME) {
	DrawObjectMask(mask, win, x, y);
	XCopyArea(display, pix, win, drawGC, 0, 0,
		  BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
      }
      else {
        if (!gc) {
	  if (!tempGC) {
	    values.foreground = fgcolor;
	    values.background = bgcolor;
	    tempGC = XCreateGC(display, RootWindow(display, screen),
			       GCForeground | GCBackground, &values);
	  }
	  XSetClipMask(display, tempGC, mask);
	  gc = tempGC;
	}
        XSetClipOrigin(display, gc, x, y);
	XCopyArea(display, pix, win, gc, 0, 0,
		  BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
      }
    }
}

E 14
I 13


E 13
E 12
D 15

E 15
/* this routine is used for placing numbers in the corners of item bitmaps
   in the game.  which_corner is to be one of: 1 - upper left,
   2 - upper right, 3 - lower right, 4 - lower left.  The routine will
   place the numbers flush with the corner edges, using the tinyfont
   size of print.  The x and y you give are the upper left coordinates
   of the item bitmap */

place_corner_number(win, x, y, which_corner, number)
Window win;
int x, y, which_corner, number;
{
  char s[50];
  int wx, wy, wheight, wwidth;
  
  /* do nothing if you are given a bad corner number */
  if (which_corner < 1 || which_corner > 4) return;

  /* convert number to a string */
  sprintf(s, "%d", number);

  /* if number takes too much room then forget it */
  if (strlen(s) * char_width(tinyfont) > BITMAP_WIDTH) return;

  /* figure out height and width of number in pixels */
  wheight = char_height(tinyfont);
  wwidth = char_width(tinyfont) * strlen(s);

  /* given corner to use, figure out the x and y coords inside item bitmap */
  switch (which_corner) {
	case 1:	wx = 1;				wy = 1;
		break;
	case 2: wx = BITMAP_WIDTH - wwidth - 1;	wy = 1;
		break;
	case 3:	wx = BITMAP_WIDTH - wwidth - 1;	
		wy = BITMAP_HEIGHT - wheight -1;
		break;
	case 4: wx = 1;				
		wy = BITMAP_HEIGHT - wheight - 1;
		break;
  }

  /* place the number there */
  text_write(win, tinyfont, x + wx, y + wy, 0, 0, s);
}



/* ===================================================================== */

/* This procedure prints out a string of characters in the desired window.
   You must give it the window, font, x and y location, number of characters
   in each direction to offset the printing,  and the string to print */

text_write(win, fi, x, y, xoff, yoff, s)
Window	win;
XFontStruct  *fi;
int	  x, y, xoff, yoff;
char	  *s;
{
  XSetFont(display, mainGC, fi->fid);
  XDrawImageString(display, win, mainGC, (x + char_width(fi) * xoff),
		   (y + char_height(fi) * yoff + fi->ascent), s, strlen(s));
}



/* This procedure erases earlier printed text characters in the desired
   window.  You must give it parameters similar to text_write above. */

text_erase(win, fi, x, y, xoff, yoff, len)
Window    win;
XFontStruct  *fi;
int	  x, y, xoff, yoff, len;
{
  XClearArea(display, win, (x + xoff * char_width(fi)),
	     (y + yoff * char_height(fi)), len * char_width(fi),
	     char_height(fi), FALSE);
}

/* the same this as above, but with EXPOSE events */

text_clear(win, fi, x, y, xoff, yoff, len)
Window    win;
XFontStruct  *fi;
int	  x, y, xoff, yoff, len;
{
  XClearArea(display, win, (x + xoff * char_width(fi)),
	     (y + yoff * char_height(fi)), len * char_width(fi),
	     char_height(fi), TRUE);
}



/* This procedure will center a line of text for you.  You must specify
   the window, the font, the two x locations to center between, the two
   y locations to center between, and the line of text */

text_center(win, fi, x1, x2, y1, y2, s)
Window	win;
XFontStruct *fi;
int	x1, x2, y1, y2;
char	*s;
{
  int pixwide, pixhigh;
  int xput, yput;

  pixwide = text_width(fi, s);
  pixhigh = text_height(fi);

  if (x2 - x1 < pixwide) xput = x1;
  else xput = x1 + ((x2 - x1 - pixwide) / 2);

  if (y2 - y1 < pixhigh) yput = y1;
  else yput = y1 + ((y2 - y1 - pixhigh) / 2);

  text_write(win, fi, xput, yput, 0, 0, s);
}



/* scroll contents of a window up one line, you give the destination X and
   Y and the size of the area to be moved in rows and columns.  The size
   you give will generally be one line less than the window size, and the
   destination X and Y will be the upper left corner pixel XY of the text */

scroll_window(win, fi, dstX, dstY, cols, lines)
Window win;
XFontStruct *fi;
int dstX, dstY;
int cols, lines;
{
  int srcX, srcY, width, height;

  /* set up source and size variables */
  srcX = dstX;
  srcY = dstY + char_height(fi);
  width = cols * char_width(fi);
  height = lines * char_height(fi);

  /* copy the text */
  XCopyArea(display, win, win, mainGC, srcX, srcY, width, height, dstX, dstY);

  /* clear the area we moved away from */
  clear_area(win, srcX, dstY + height,
	     srcX + (cols * char_width(fi)), srcY + height);
}



/* ============= H E L P window package ========================= */

/* set up the window (ie.  make it but don't map it) according
   to the given information, return a pointer to a valid
   help window record */

Help *make_help_window(parentwin, x, y, width, height, fi)
Window parentwin;
int x, y, width, height;
XFontStruct *fi;
{
  int i;
  Help *newrec;
D 7
  XSetWindowAttributes attr;
E 7
  
  /* allocate the record in memory */
  demand((newrec = (Help *) malloc(sizeof(Help))), 
	 "No memory for help window");

  /* if the width or height is too large, then limit it */
  if (width >= MAX_HELP_COLS) width = MAX_HELP_COLS - 1;
  if (height >= MAX_HELP_LINES) height = MAX_HELP_LINES - 1;

  /* open the window */
  newrec->win = XCreateSimpleWindow(display, parentwin, x, y,
				   width * char_width(fi) + WINDOW_PADDING*2,
				   height * char_height(fi) + WINDOW_PADDING*2,
				   WINDOW_BORDERS, fgcolor, bgcolor);

  /* set the record information */
  newrec->fi = fi;
  newrec->width = width;
  newrec->height = height;
  newrec->is_up = FALSE;

  /* clear text storage area */
  clear_help_window(newrec);

  /* add the window to the help expose list */
  help_add_expose_check(newrec);

  /* return the successfully made help window record */
  return newrec;
}
I 7



I 18
void resize_help_window_pixel_size(rec, width, height)
Help *rec;
int width, height;
{
  XResizeWindow(display, rec->win, width, height);
}



E 18
/* create a help window like the above procedure does, but make it
   a pop-up window.  This should be called for things like the info
   window in editmap, that shouldn't be decorated by the window manager
   but that will appear in the root window. */

Help *make_popup_help_window(toplevelwin, parentwin, name,
			     x, y, width, height, fi)
Window toplevelwin, parentwin;
char *name;
int x, y, width, height;
XFontStruct *fi;
{
  Help *result;
  XSetWindowAttributes attr;

  attr.save_under = TRUE;
  result = make_help_window(parentwin, x, y, width, height, fi);
  if (name) XStoreName(display, result->win, name);
  XSetTransientForHint(display, result->win, toplevelwin);
  XChangeWindowAttributes(display, result->win, CWSaveUnder, &attr);

  return result;
}

E 7

/*****************************************************************/

check_help_expose(win)
Window win;
{
  HelpExposeCheck *thischeck;
D 3

I 2
  return(False);
E 3

E 2
  thischeck = GlobalHelpExposeList;
  while(thischeck)
    if (thischeck->help_rec->win == win)
      {
	write_help_window_contents(thischeck->help_rec);	  
	return(True);
      }
    else
      thischeck = thischeck->next;

  return(False);
}

/*****************************************************************/

help_add_expose_check(rec)
Help *rec;
{
  HelpExposeCheck *newcheck;

  demand((newcheck = (HelpExposeCheck *) malloc(sizeof(HelpExposeCheck))),
	 "No memory for help expose check");

  newcheck->next = GlobalHelpExposeList;
  newcheck->help_rec = rec;
  GlobalHelpExposeList = newcheck;
}

/*****************************************************************/

help_remove_expose_check(rec)
Help *rec;
{
  HelpExposeCheck *newcheck,temp,*save;
 
  temp.next = GlobalHelpExposeList;
  newcheck = &temp;

  while(newcheck->next)
    if (newcheck->next->help_rec == rec)
      {
	save = newcheck->next;
	newcheck->next = newcheck->next->next;
	free(save);
	GlobalHelpExposeList = temp.next;
	return;
      }
    else
      newcheck = newcheck->next;
}

/*****************************************************************/


/* toggle the state of a help window, if it is up, then hide it,
   if it's not up, then turn it on! */

toggle_help_window(rec)
Help *rec;
{
  if (rec->is_up) hide_help_window(rec);
  else show_help_window(rec);
}



/* show the help window, map it */

show_help_window(rec)
Help *rec;
{
  /* make sure it ends up on top */
  XRaiseWindow(display, rec->win);

  /* make it visible */
  XMapWindow(display, rec->win);
  rec->is_up = TRUE;

  /* draw some stuff because some parts of other programs
     don't handle the events correctly.  This MAY cause some
     unncessary redrawing */
  write_help_window_contents(rec);
  
}



/* hide the help window referred to */

hide_help_window(rec)
Help *rec;
{
  XUnmapWindow(display, rec->win);
  rec->is_up = FALSE;
}


/* write the contents of the help window referred to */

write_help_window_contents(rec)
Help *rec;
{
  int i;

  /* write out each line of text one by one */
  if (rec->is_up)
    for (i=0; i<rec->height; i++)
      if (strlen(rec->text[i]))
        text_write(rec->win, rec->fi, WINDOW_PADDING, WINDOW_PADDING,
		   0, i, rec->text[i]);
}



/* load the contents of a help window from a text file */

load_help_window_contents(rec, filename)
Help *rec;
char *filename;
{
  int i;
  FILE *fp;
  char s[120];

  /* open the file */
  fp = fopen(filename, "r");
  if (fp == NULL) {
    sprintf(s, "   '%s'", filename);
    set_line(rec, 1, "Unable to read help file:");
    set_line(rec, 2, s);
    return;
  }

  /* read lines until we've got a full window or there are no more */
  for (i=0; i<rec->height && !feof(fp); i++)
    strcpy(rec->text[i], read_line(fp));

  /* close the file */
  fclose(fp);
}

/* clear all of the text that was in a window, write_help_window_contents
   will need to be used after this to see the result */

clear_help_window(rec)
Help *rec;
{
  int i, j;

  for (i=0; i < rec->height; i++)
    for (j=0; j < rec->width; j++)
      rec->text[i][j] = '\0';
}

/*****************************************************************/
/* find the maximum string length in a compound string */

maxstrlen(s)
char *s;
{
  int high,count;
  high = 0;

  while(*s)
    {
      count = 0;
      while((*s != '\n') && (*s != '\0'))
	s++, count++;
      s++;
      if (count > high) high = count;
    }
  return(high);
}

/*****************************************************************/
/* count the number of lines in a compound string */

count_lines(s)
char *s;
{
  int count = 0;

  while (*s != '\0')
    {
      while ((*s != '\n')&&(*s != '\0'))
	s++;
      count++;

      /* advance only if it was a newline, to prevent running
	 off of the end of the string */
      if (*s == '\n')
	s++;
    }

  return(count);
}

/*****************************************************************/
/* set all of the lines of a help window to the compound string */

set_lines(rec, s)
char *s;
Help *rec;
{
  char *nextend,*temp;
  int lines,linenum,length;
  
  nextend = s;
  lines = count_lines(s);
  
  for (linenum = 0;((linenum < lines)&&(*s != '\0'));linenum++)
    {
      while ((*nextend != '\n')&&(*nextend != '\0'))
	nextend++;

      length = nextend - s;
      temp = (char *) malloc(length + 1);
      strncpy(temp,s,length);
      temp[length] = '\0';
      set_line(rec,linenum,temp);
      nextend++;
      s = nextend;
      free(temp);
    }
}


/* set a certain line in a help window to be a particular string */

set_line(rec, linenum, s)
Help *rec;
int  linenum;
char *s;
{
  if (linenum < rec->height) {
    if (strlen(s) <= rec->width)
      strcpy(rec->text[linenum], s);
    else {
      strncpy(rec->text[linenum], s, rec->width);
      rec->text[linenum][rec->width] = '\0';
    }
  }
}



/* set a certain line starting at a certain column, make sure that
   any nulls before that put are changed to spaces */

set_at(rec, col, linenum, s)
Help *rec;
int  col, linenum;
char *s;
{
  int i;
  
  if (linenum < rec->height) {

    if (strlen(s) < (rec->width - col))
      strcpy((rec->text[linenum] + col), s);
    else {
      strncpy((rec->text[linenum] + col), s, (rec->width - col - 1));
      rec->text[linenum][rec->width] = '\0';
    }

    /* make sure that there are no NULL characters before start point */
    for (i=0; i<col; i++)
      if (!rec->text[linenum][i]) rec->text[linenum][i] = ' ';
  }
}



/* set several lines with one given string, cutting it for wrap
   with knowledge of the line length of the help window */

set_lines_long_text(rec, start_line, s)
Help *rec;
int  start_line;
char *s;
{
  int i, current_line, strpnt=0;
  char tmp[200];

  current_line = start_line;
  
  /* continue to set lines until there is no more string left */
  while (strlen(s)>strpnt) {

    /* look for closest white space to end of line */
    i = strpnt + rec->width;
    if (i > strlen(s))
      i = strlen(s) -1;
    else 
      for (i = strpnt + rec->width; i>strpnt && !isspace(s[i]); i--);

    /* set this line */
    strncpy(tmp, s+strpnt, (i - strpnt));
    tmp[i - strpnt] = '\0';
    strpnt = i + 1;
    set_line(rec, current_line, tmp);
    current_line++;
  }
}



/* move a help window to a new location, redrawing it if it was up */

move_help_window(rec, x, y)
Help *rec;
int  x, y;
{
  /* move the window */
  XMoveWindow(display, rec->win, x, y);

  /* redraw contents */
  write_help_window_contents(rec);
}



/* destroy a previously made help window, never to be used again */

destroy_help_window(rec)
Help *rec;
{
  help_remove_expose_check(rec);
  XDestroyWindow(display, rec->win);
  free(rec);
}



/* place a pixmap in a help window */

pixmap_in_help_window(rec, x, y, pix, width, height)
Help *rec;
int x, y;
Pixmap pix;
int width, height;
{
D 8
  XCopyArea(display, pix, rec->win, mainGC, 0, 0, width, height,
	    (x * char_width(rec->fi)), (y * char_height(rec->fi)));
E 8
I 8
D 15
  XCopyPlane(display, pix, rec->win, mainGC, 0, 0, width, height,
	    (x * char_width(rec->fi)), (y * char_height(rec->fi)),1);
E 15
I 15
  XCopyArea(display, pix, rec->win, mainGC, 0, 0, width, height,
	    (x * char_width(rec->fi)), (y * char_height(rec->fi)));
E 15
E 8
}
E 1
