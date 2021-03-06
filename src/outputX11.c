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


/* place the picture of a mouse (computer mouse) in given window at x y */

place_mouse_picture(win, x, y)
Window win;
int x, y;
{
  /* place the mouse picture image */
  XCopyArea(display, mouse_pix, win, mainGC, 0, 0,
	    mouse_width, mouse_height, x, y);
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
  DrawObject(win, obj_pixmap[objnum], NULL, NULL, wx, wy);
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



void draw_object(obj, win, x, y)
/* sees to the drawing of a particular object in the right window.  Checks
   for the existence of a special pixmap pair for this object instance. */
OI *obj;
Window win;
int x, y;
{
  /*** COMPLETE: look for instance pixmap ***/
  place_obj_pixmap(win, x, y, obj->type, FALSE);
}



/* place an object pixmap on screen, if there is a mask then place it
   on first.  You must give the window and pixel x and y coords to
   draw at.  You must specify the object number, and whether we can assume
   the spot underneath has nothing at all drawn on it already */

place_obj_pixmap(win, x, y, n, is_clear_under)
Window win;
int x, y, n, is_clear_under;
{
  if (n<0 || n>=objects) return;

  if (info[n]->masked && !is_clear_under) {
    DrawObject(win, obj_pixmap[n], obj_mask[n], obj_clipGC[n],
	       x, y);
  }
  else {
    DrawObject(win, obj_pixmap[n], NULL, NULL, x, y);
  }
}



DrawObjectMask(mask,dest,x,y)
Window mask,dest;
int x,y;
{
  XCopyArea (display, mask, dest, maskGC, 0, 0, 
	     BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
}



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



void resize_help_window_pixel_size(rec, width, height)
Help *rec;
int width, height;
{
  XResizeWindow(display, rec->win, width, height);
}



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


/*****************************************************************/

check_help_expose(win)
Window win;
{
  HelpExposeCheck *thischeck;

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
  XCopyArea(display, pix, rec->win, mainGC, 0, 0, width, height,
	    (x * char_width(rec->fi)), (y * char_height(rec->fi)));
}
