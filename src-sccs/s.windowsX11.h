h59030
s 00000/00000/00146
d D 1.8 92/08/07 01:04:35 vbo 8 7
c source copied to a separate tree for work on new map and object format
e
s 00002/00002/00144
d D 1.7 91/11/29 16:09:32 labc-4lc 7 6
c finished making entry of hidden text properties in obtor
e
s 00004/00002/00142
d D 1.6 91/11/25 23:03:55 labc-4lc 6 5
c fixed color problems drawing objects
e
s 00001/00001/00143
d D 1.5 91/08/28 21:10:15 vanb 5 4
c made editmap work in color X windows
e
s 00000/00000/00144
d D 1.4 91/08/28 01:42:41 vanb 4 3
c 
e
s 00001/00000/00143
d D 1.3 91/05/17 03:32:56 labc-3id 3 2
c added prototype for make_popup_help_window()
e
s 00001/00001/00142
d D 1.2 91/05/10 04:52:02 labc-3id 2 1
c 
e
s 00143/00000/00000
d D 1.1 91/02/16 13:01:30 labc-3id 1 0
c date and time created 91/02/16 13:01:30 by labc-3id
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
#include "config.h"
#include "objects.h"
#include "map.h"
#include "lib.h"
#include "bit/mouse"

#define TRUE		1
#define FALSE		0

#define TEXTINPLEN      120     /* max length of an input string */

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

/*****************************************************************/



/* External Global Variable Definitions */

#ifndef WINDOW_MAIN

extern Display	*display;
D 5
extern int	screen;
E 5
I 5
D 6
extern int	screen, depth;
E 6
I 6
extern int	screen, depth, maskfg, maskbg;
E 6
E 5
D 2
extern GC	mainGC, inverseGC, drawGC, maskGC, xorGC;
E 2
I 2
extern GC	mainGC, inverseGC, drawGC, maskGC, xorGC,xoriGC;
E 2
extern Pixmap	obj_pixmap[MAX_OBJECTS];
extern Pixmap	obj_mask[MAX_OBJECTS];
I 6
extern GC	obj_clipGC[MAX_OBJECTS];
E 6
extern Window	roomwin;	/* window where room is displayed */
extern Pixmap	mouse_pix;	/* mouse representation */
extern unsigned long fgcolor, bgcolor;	/* foreground and background pixels */
extern int	maskfunc, drawfunc;	/* drawing funcs in place_obj_pixmap */
extern int	reversed_screen;	/* TRUE: screen is black on white */
extern XFontStruct  *bigfont;	/* the larger font we'll use quite often */
extern XFontStruct  *regfont;	/* the smaller, more normal sized font */
extern XFontStruct  *tinyfont;	/* the smallest font */
extern XEvent	last_event;	/* the last event that was received from X */
extern HelpExposeCheck *GlobalHelpExposeList;
extern int      (*GlobalRedraw)(); /* procedure to redraw a window */
#endif



/*****************************************************************/

/* macros to get at the stats on a help window */
#define HW_WINDOW(rec)		((rec)->win)
#define HW_HEIGHT_PIXELS(rec)	((rec)->height * text_height((rec)->fi))
#define	HW_WIDTH_PIXELS(rec)	((rec)->width  * char_width((rec)->fi))
#define HW_IS_MAPPED(rec)	((rec)->is_up)
#define HW_FONT(rec)		((rec)->fi)


/* procedure declarations */

D 7
char *get_string();
char *get_string_shift();
E 7
I 7
char *get_string(), *display_edit_string();
char *get_string_shift(), *call_up_editor();
E 7
Help *make_help_window();
I 3
Help *make_popup_help_window();
E 3


I 6
/* more helpful macros */
E 6

D 6

E 6
I 6
#define MONOCHROME	(depth == 1)
E 6
E 1
