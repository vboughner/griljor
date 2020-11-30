h49508
s 00000/00000/00088
d D 1.2 92/08/07 01:01:47 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00088/00000/00000
d D 1.1 91/04/14 19:33:12 labc-3id 1 0
c date and time created 91/04/14 19:33:12 by labc-3id
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

/* Header file for general window routines */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#define TRUE		1
#define FALSE		0

/* different kinds of events/messages from X server to us */
#define NOTHING		0	/* nothing yet */
#define KEYBOARD	1	/* key pressed */
#define MOUSE		2	/* mouse button clicked */
#define EXPOSED		3	/* window exposed */
#define ENTER		4	/* mouse entering main window */

#define WINDOW_BORDERS		2
#define WINDOW_PADDING          2
#define ROOM_BORDER		0	/* blank space around drawn room */
#define ROOM_PIXELS_WIDE	(ROOM_WIDTH * BITMAP_WIDTH + ROOM_BORDER * 2)
#define ROOM_PIXELS_HIGH	(ROOM_HEIGHT * BITMAP_HEIGHT + ROOM_BORDER * 2)

/* definitions for mouse click buffer pix */
#define MOUSE_PIX_X	(STAT_WIN_WIDTH - mouse_width - 20)
#define MOUSE_PIX_Y	20	/* y location for pixmap in statwin */
#define MOUSE_START_X	8	/* place on pixmap where first buffer obj */
#define MOUSE_START_Y	18	/* should be stored */
#define MOUSE_SPACING	8	/* space between buffer spots */

/* constants used by help windows */
#define MAX_HELP_LINES	70		/* num of lines in window */
#define MAX_HELP_COLS	250		/* num of columns window will hold */


/* Macro definitions for font functions, returning the number of pixels
   high a font is or how many pixels long a string of text in a certain
   font will be. */
#define text_width(fi, s)       (XTextWidth((fi), (s), strlen(s)))
#define text_height(fi)         (((fi)->ascent) + ((fi)->descent))
#define char_height(fi)		(text_height(fi))
#define char_width(fi)		((fi)->max_bounds.rbearing -		\
				 (fi)->min_bounds.lbearing)

/* Macro definitions for converting window pixel x and y coordinates
   into the numbered box type spots that define a room */

#define CONVERTRX(x)	((x - ROOM_BORDER / 2) / BITMAP_WIDTH)
#define CONVERTRY(y)	((y - ROOM_BORDER / 2) / BITMAP_HEIGHT)


/* Macro definitions for converting square coordinates back into room
   window pixel x and y coordinates */

#define PIXELX(x)	(x * BITMAP_WIDTH + ROOM_BORDER)
#define PIXELY(y)	(y * BITMAP_HEIGHT + ROOM_BORDER)

/*****************************************************************/

/* a generic help type window */

typedef struct _help {
	Window		win;		/* window this help is in */
	XFontStruct	*fi;		/* font used in this window */
	short		width, height;	/* size, in chars of the window */
	short		is_up;		/* TRUE when window is up and full */
	char		text[MAX_HELP_LINES][MAX_HELP_COLS];
} Help;

typedef struct _help_expose
{
  Help *help_rec;
  struct _help_expose *next;
} HelpExposeCheck;
E 1