h47278
s 00000/00000/02038
d D 1.16 92/08/07 21:20:17 vbo 16 15
c fixes for sun port merged in
e
s 00245/00051/01793
d D 1.15 92/03/11 20:52:24 vanb 15 14
c added to structures for containers, fixed more bugs
e
s 00285/00194/01559
d D 1.14 92/03/02 22:06:46 vanb 14 13
c finished getting editmap to work on basics
e
s 00277/00170/01476
d D 1.13 92/02/20 21:57:49 vanb 13 12
c 
e
s 00135/00486/01511
d D 1.12 92/02/15 01:09:50 vanb 12 11
c major changes to map structures
e
s 00026/00001/01971
d D 1.11 91/09/16 22:20:16 labc-4lc 11 10
c made room and recobj dialog edit windows work for editmap
e
s 00002/00000/01970
d D 1.10 91/09/15 23:33:22 labc-4lc 10 9
c made mapinfo dialog editing window work
e
s 00006/00006/01964
d D 1.9 91/08/28 21:09:54 vanb 9 8
c made editmap work in color X windows
e
s 00001/00001/01969
d D 1.8 91/05/23 11:43:29 labc-3id 8 7
c fixed small abstraction error in setup_toplevel_icon()
e
s 00032/00020/01938
d D 1.7 91/05/17 04:05:09 labc-3id 7 6
c added feature to not display scroll buttons when there are few objects
e
s 00002/00001/01956
d D 1.6 91/05/17 03:31:26 labc-3id 6 5
c fixed multiple redraw problem
e
s 00225/00046/01732
d D 1.5 91/05/17 03:00:38 labc-3id 5 4
c added scrolling item window, improved info popup and help window
e
s 00004/00001/01774
d D 1.4 91/05/02 16:51:36 labc-3id 4 3
c 
e
s 00057/00010/01718
d D 1.3 91/03/22 16:45:00 labc-3id 3 2
c 
e
s 00000/00000/01728
d D 1.2 91/03/21 14:53:07 labc-3id 2 1
c 
e
s 01728/00000/00000
d D 1.1 91/02/16 12:53:56 labc-3id 1 0
c date and time created 91/02/16 12:53:56 by labc-3id
e
u
U
f e 0
t
T
I 1
/****************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 **************************************************************************/

/* X Routines used only by the editor, not by player program */

#include "windowsX11.h"
#include "def.h"
I 12
#include "mapfunc.h"
I 13
#include "mapstore.h"
#include "emap.h"
E 13
E 12
D 5
#include "bit/link"		/* picture of link box containing room number */
E 5
I 5
#include "bit/link"		/* bitmap of link box containing room number */
#include "bit/editmapicon"	/* icon bitmap */
#include "bit/scrollup"		/* scroll button */
#include "bit/scrolldown"	/* scroll button */
#include "bit/pageup"		/* scroll button */
#include "bit/pagedown"		/* scroll button */
I 13
#include "bit/add"		/* cursor for add editing mode */
#include "bit/ovr"	/* cursor for overwrite editing mode */
#include "bit/xcg"		/* cursor for exchange editing mode */
#include "bit/del"		/* cursor for delete editing mode */
#include "bit/modemask"		/* bitmap mask for the mode cursors */
I 14
#include "bit/box"		/* box to place around pile displayed */
#include "bit/bmask"		/* mask for pile display box */
E 14
E 13
E 5

I 14

E 14
D 13
#define EDITOR_WIN_X		30	/* x of main window, upper left */
E 13
I 13
/* placement and size of toplevel window */
#define EDITOR_WIN_X		30
E 13
#define EDITOR_WIN_Y		5
I 13
#define EDITOR_WIN_W	(ROOM_PIXELS_WIDE + PILE_WIN_W + \
			 GRAB_WIN_W + WINDOW_BORDERS * 2)
#define EDITOR_WIN_H	(GRAB_WIN_H + CLICK_WIN_H + WINDOW_BORDERS)

/* constants for placement of the object definition file display window */
E 13
D 5
#define GRAB_WIN_WIDTH		10	/* # of objects wide grab window is */
#define GRAB_WIN_HEIGHT		26	/* # of objects high */
#define GRAB_SPACING		2	/* space between characters */
E 5
I 5
#define GRAB_WIN_WIDTH		8	/* # of objects wide grab window is */
D 13
#define GRAB_WIN_HEIGHT		25	/* # of objects high */
E 13
I 13
#define GRAB_WIN_HEIGHT		18	/* # of objects high */
E 13
#define GRAB_WIN_TOPAREA	1	/* # of objects high top area is */
#define GRAB_SPACING		1	/* space (pixels) between objects */
E 5
D 13
#define STAT_WIN_WIDTH	ROOM_PIXELS_WIDE
#define STAT_WIN_HEIGHT	(GRAB_WIN_HEIGHT * (BITMAP_HEIGHT + GRAB_SPACING) \
			- ROOM_PIXELS_HIGH - WINDOW_BORDERS)
E 13
I 13
#define GRAB_WIN_W	(GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING))
#define GRAB_WIN_H	(GRAB_WIN_HEIGHT * (BITMAP_HEIGHT + GRAB_SPACING))
#define GRAB_WIN_X	(ROOM_PIXELS_WIDE + PILE_WIN_W + WINDOW_BORDERS)
#define GRAB_WIN_Y	(CLICK_WIN_H)
E 13

I 13
/* constants for size of click buffer display window */
#define CLICK_WIN_W		(GRAB_WIN_W + PILE_WIN_W + WINDOW_BORDERS)
#define CLICK_WIN_H		(mouse_height + 16)
#define MOUSE_PIX_X		((CLICK_WIN_W - mouse_width) / 2)
#define MOUSE_PIX_Y		((CLICK_WIN_H - mouse_height) / 2)

/* constants for size of pile window */
#define PILE_WIN_W		(BITMAP_WIDTH)
#define PILE_WIN_H		(GRAB_WIN_H)

/* constants for placement of lines in status window */
#define STAT_WIN_W	(ROOM_PIXELS_WIDE)
#define STAT_WIN_H	(EDITOR_WIN_H - ROOM_PIXELS_HIGH - WINDOW_BORDERS)
#define STAT_LEFT_END	8	/* start of printing on left */
#define STAT_RIGHT_END	(STAT_WIN_W - 8)	/* right edge */
#define STAT_TOP_EDGE	6	/* top edge of printing */
#define STAT_REG_SPACE	(char_height(regfont))
#define STAT_BIG_SPACE	(char_height(bigfont))
#define STAT_PAD_SPACE	4

E 13
/* definitions for link box picture, amount of space for room number */
#define LINK_SPACE_HEIGHT	29
#define LINK_SPACE_WIDTH	(link_width - 4)
#define LINK_PIX_X	(MOUSE_PIX_X + (mouse_width - link_width) / 2)
D 5
#define LINK_PIX_Y	(MOUSE_PIX_Y + mouse_height + (STAT_WIN_HEIGHT - \
			MOUSE_PIX_Y - mouse_height - link_height) / 2)
E 5
I 5
#define LINK_PIX_Y	(MOUSE_PIX_Y + mouse_height / 2)
E 5

D 13
/* constants for placement of lines in editor status window */
#define STAT_LEFT_END	8	/* start of printing on left */
#define STAT_RIGHT_END	(MOUSE_PIX_X - 20)	/* right edge */
#define STAT_TOP_EDGE	6	/* top edge of printing */
#define STAT_REG_SPACE	16	/* space for a line of regular font */
#define STAT_BIG_SPACE	32	/* space for a line of large font */

E 13
/* constants for use by the room linking function */
#define DORMANT		0	/* no links in progress now */
#define EXPECTING	1	/* one side of link has been made */

D 13

E 13
D 14
/* constants used by information window */
D 13
#define INFO_LINES	5		/* num of lines in window */
#define INFO_COLS	30		/* num of columns window will hold */
E 13
I 13
#define INFO_LINES	8		/* num of lines in window */
#define INFO_COLS	35		/* num of columns window will hold */
E 13
#define INFO_FONT	(regfont)	/* font to use in help window */

E 14
/* constants used by help window */
D 5
/* #define HELP_X		EDITOR_WIN_X
  #define HELP_Y		(EDITOR_WIN_Y + ROOM_PIXELS_HIGH + WINDOW_BORDERS * 2)
*/
E 5
#define HELP_X		(-WINDOW_BORDERS)
#define HELP_Y		(-WINDOW_BORDERS)
D 5
#define HELP_LINES	15		/* num of lines in window */
#define HELP_COLS	71		/* num of columns window will hold */
E 5
I 5
#define HELP_LINES	18		/* num of lines in window */
#define HELP_COLS	80		/* num of columns window will hold */
E 5
#define HELP_FONT	(regfont)	/* font to use in help window */

D 13
/* the informational record containing the location of a linking end */
E 13
I 13
/* key definitions for a couple of command keys */
#define NEUTRALS_KEY		'^'
#define NUM_OF_TEAMS_KEY	'&'
E 13

I 13
/* editing modes */
#define ADD_MODE	0
#define OVERWRITE_MODE	1
#define EXCHANGE_MODE	2
#define DELETE_MODE	3
#define NUM_OF_MODES	4	/* keep this current */

I 15
#define IN_ADD_MODE	(editor_mode == ADD_MODE)
E 15


I 15

E 15
D 14
/* the informational record containing the location of a linking end */
E 14
E 13
typedef struct _link {
I 14
/* the informational record containing the location of a linking end */
E 14
	short	room;		/* room where link end is */
	short	direction;	/* direction out of room link extends */
	short	x, y;		/* coords of possible link exit char */
} Link;

D 14
/* the informational record for storing the cut and paste pointer */
E 14
I 14


E 14
D 13

E 13
typedef struct _paste {
I 14
/* the informational record for storing the cut and paste pointer */
E 14
	short		is_set;		/* TRUE if mark is set */
	short		room;		/* room where mark was set */
	short		x, y;		/* point where mark was set */
D 12
	/* the information you store in the cut and paste buffer: */
	unsigned char	spot[ROOM_WIDTH][ROOM_HEIGHT][ROOM_DEPTH];
	short		width, height;	/* size of buffer contents */
E 12
I 12
	MapInfo		*map;		/* one-room paste storage map */
	short		width, height;	/* size of buffer contents in room */
E 12
} Paste;



I 14
typedef struct _pile {
/* information concerning what is displayed in pile window */
	char		pertinent;	/* is TRUE when something displayed */
	MapInfo		*map;		/* pointer to map of square */
	SquareRec	*square;	/* pointer to the square displayed */
	int		roomnum, x, y;	/* which square in map displayed */
} Pile;


/* is TRUE when pile pointer is set in room window somewhere */
#define PILE_IN_ROOMWIN		(pile_content.map == &mapstats)


I 15

typedef struct _click {
/* information concerning the click buffers.  Click buffer contents are stored
   in the first three squares (0,0) (1,0) (2,0), of a simple one-room
   map especially for click buffers.  This way, click buffers may contain
   any number of objects. */
  	int		num_buffers;	/* usually 3 */
	MapInfo		*map;
} Click;

#define CLICK_SQUARE(n)		(get_square(click.map, 0, n, 0))
#define PILE_APPEND	9999	/* signifies a point beyond end of pile */
#define PILE_NONE	-1	/* signifies that there is no pile */

	

E 15
E 14
/* global variables */
Window	editwin;	/* main parent window for editor windows */
Window	grabwin;	/* window where editor characters displayed */
Window	statwin;	/* window where editor stats are displayed */
I 13
Window  clickwin;	/* window where click buffer contents displayed */
Window  pilewin;	/* window where pile of stuff on floor displayed */
E 13
Pixmap	link_pix;	/* room linking status picture */
I 5
D 13
Pixmap  scrollup, scrolldown, pageup, pagedown;  /* scrolling buttons */
E 13
I 13
Pixmap  scrollup, scrolldown, pageup, pagedown; /* scrolling buttons */
I 14
Pixmap	box_pix, box_mask;	/* pile display square outline box */
E 14
Cursor	add_cur, ovr_cur, xcg_cur, del_cur;	/* editing cursors */
E 13
E 5
D 12
unsigned char click_buffer[3];	/* three element object click buffer */
I 3
MemObj  *click_recd[3];		/* recorded info for click buffer objs */
E 12
I 12
D 15
short	click_buffer[3];	/* three element object click buffer */
E 15
I 15
Click	click;		/* object click buffers */
E 15
E 12
E 3
int	link_status = DORMANT;	/* status of room linking function */
Link	link_info;		/* the particulars of the first link */
Paste	paste;		/* the cut and paste buffer information */
Help	*help_rec;	/* record containing info for help window */
D 14
Help	*info_rec;	/* record containing info for info window */
E 14
I 5
D 13
int	first_in_grab=0;/* object number of object at top left of grab win */
E 13
I 13
int	first_in_grab=0;	/* object number of object at top left */
char	last_command_key = ' ';	/* last key command entered */
int	editor_mode = ADD_MODE;	/* initial mode */
I 14
Pile	pile_content;	/* contents of pile window */
E 14
E 13
E 5

D 13
/* procedure declarations */
E 13

I 13
/* procedure declarations */
E 13
char *get_editor_input_line();
int redraw_a_window();


D 13
/* load the cursors for the editor program */

E 13
load_editor_cursors()
I 13
/* load the cursors for the editor program */
E 13
{
D 13
  define_thin_arrow_cursor(editwin);
  define_thin_arrow_cursor(roomwin);
E 13
I 13
  ovr_cur = make_cursor_from_bitmap_data(display, editwin, ovr_width,
					 ovr_height, ovr_bits, modemask_bits,
					 ovr_width / 2, ovr_height / 2,
					 fgcolor, bgcolor);
  xcg_cur = make_cursor_from_bitmap_data(display, editwin, xcg_width,
					 xcg_height, xcg_bits, modemask_bits,
					 xcg_width / 2, xcg_height / 2,
					 fgcolor, bgcolor);
  del_cur = make_cursor_from_bitmap_data(display, editwin, del_width,
					 del_height, del_bits, modemask_bits,
					 del_width / 2, del_height / 2,
					 fgcolor, bgcolor);
  add_cur = make_cursor_from_bitmap_data(display, editwin, add_width,
					 add_height, add_bits, modemask_bits,
					 add_width / 2, add_height / 2,
					 fgcolor, bgcolor);
E 13
  define_thin_arrow_cursor(grabwin);
  define_thin_arrow_cursor(statwin);
I 13
  define_thin_arrow_cursor(clickwin);
E 13
  define_thin_arrow_cursor(help_rec->win);
D 14
  define_thin_arrow_cursor(info_rec->win);
E 14
}



D 13
/* set up the windows needed by the map editor */
E 13
I 13
change_cursor_mode(newmode)
/* set new mode, of ADD_MODE, DELETE_MODE, EXCHANGE_MODE, or OVERWRITE_MODE */
int newmode;
{
  editor_mode = newmode;
  switch(editor_mode) {
  case ADD_MODE:
    XDefineCursor(display, editwin, add_cur);
    break;
  case OVERWRITE_MODE:
    XDefineCursor(display, editwin, ovr_cur);
    break;
  case EXCHANGE_MODE:
    XDefineCursor(display, editwin, xcg_cur);
    break;
  case DELETE_MODE:
    XDefineCursor(display, editwin, del_cur);
    break;
  default:
    fprintf(stderr, "bad cursor mode requested\n");
    break;
  }
E 13

I 13
}



toggle_cursor_mode()
/* change to the next mode in the set of possible modes */
{
  editor_mode++;
  if (editor_mode >= NUM_OF_MODES) editor_mode = 0;
  change_cursor_mode(editor_mode);
}



E 13
editor_window_setup()
I 13
/* set up the windows needed by the map editor */
E 13
{
  editwin = XCreateSimpleWindow(display, RootWindow(display, screen),
D 13
			  EDITOR_WIN_X, EDITOR_WIN_Y, ROOM_PIXELS_WIDE +
			  (GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING)) +
			  WINDOW_BORDERS, ROOM_PIXELS_HIGH + STAT_WIN_HEIGHT
			  + WINDOW_BORDERS, WINDOW_BORDERS, fgcolor,
			  bgcolor);
E 13
I 13
				EDITOR_WIN_X, EDITOR_WIN_Y, EDITOR_WIN_W,
				EDITOR_WIN_H, WINDOW_BORDERS, fgcolor,
				bgcolor);
E 13
  
I 5
  /* set up name and icon for top level window */
  set_up_top_level_icon(editwin);

E 5
D 13
  roomwin = XCreateSimpleWindow(display, editwin, - WINDOW_BORDERS,
			  -WINDOW_BORDERS, ROOM_PIXELS_WIDE,
			  ROOM_PIXELS_HIGH, WINDOW_BORDERS, fgcolor,
			  bgcolor);
E 13
I 13
  roomwin = XCreateSimpleWindow(display, editwin, - WINDOW_BORDERS, STAT_WIN_H,
				ROOM_PIXELS_WIDE, ROOM_PIXELS_HIGH,
				WINDOW_BORDERS, fgcolor, bgcolor);
E 13

D 13
  grabwin = XCreateSimpleWindow(display, editwin, ROOM_PIXELS_WIDE,
			  -WINDOW_BORDERS, GRAB_WIN_WIDTH * (BITMAP_WIDTH +
			  GRAB_SPACING), GRAB_WIN_HEIGHT * (BITMAP_HEIGHT +
			  GRAB_SPACING), WINDOW_BORDERS, fgcolor, bgcolor);
E 13
I 13
  grabwin = XCreateSimpleWindow(display, editwin, GRAB_WIN_X,
				GRAB_WIN_Y, GRAB_WIN_W, GRAB_WIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);
E 13

D 13
  statwin = XCreateSimpleWindow(display, editwin, ROOM_PIXELS_WIDE -
			  STAT_WIN_WIDTH - WINDOW_BORDERS, ROOM_PIXELS_HIGH,
			  STAT_WIN_WIDTH, STAT_WIN_HEIGHT, WINDOW_BORDERS,
			  fgcolor, bgcolor);
E 13
I 13
  statwin = XCreateSimpleWindow(display, editwin, -WINDOW_BORDERS,
				-WINDOW_BORDERS, STAT_WIN_W, STAT_WIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);
E 13

I 13
  clickwin = XCreateSimpleWindow(display, editwin, STAT_WIN_W,
				 -WINDOW_BORDERS, CLICK_WIN_W, CLICK_WIN_H,
				 WINDOW_BORDERS, fgcolor, bgcolor);

  pilewin = XCreateSimpleWindow(display, editwin, ROOM_PIXELS_WIDE,
				CLICK_WIN_H, PILE_WIN_W, PILE_WIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);

E 13
D 5
  XStoreName(display, editwin, "Editmap");

E 5
D 14
  /* set up the help and information windows */ 
D 5
  info_rec = make_help_window(RootWindow(display, screen), 20, 20, INFO_COLS,
			      INFO_LINES, INFO_FONT);
  help_rec = make_help_window(statwin, HELP_X, HELP_Y,
E 5
I 5
  info_rec = make_popup_help_window(editwin, RootWindow(display, screen),
		"information", 20, 20, INFO_COLS, INFO_LINES, INFO_FONT);
E 14
I 14
  /* set up the help window */ 
E 14
  help_rec = make_help_window(roomwin, HELP_X, HELP_Y,
E 5
			      HELP_COLS, HELP_LINES, HELP_FONT);

  /* load appropriate text into help window */
  load_help_window_contents(help_rec, EDITOR_HELP_FILE);
  
  /* store the mouse representation bitmap */
  define_mouse_bitmap();

  /* store the link picture representation */
  link_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			link_bits, link_width, link_height,
D 9
			fgcolor, bgcolor, 1);
E 9
I 9
			fgcolor, bgcolor, depth);
E 9

I 14
  /* store the pile box pixmaps */
  box_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			box_bits, box_width, box_height, fgcolor, bgcolor,
			depth);
  box_mask = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			bmask_bits, bmask_width, bmask_height,
			maskfg, maskbg, 1);

E 14
I 5
  /* store scroll button representations */
  load_scroll_button_pixmaps();

E 5
  /* set the global redraw procedure */
  GlobalRedraw = redraw_a_window;
}



I 5
set_up_top_level_icon(win)
Window win;
{
  Pixmap icon_pix;
  XWMHints wmhints;

  XStoreName(display, win, "Editmap");
  icon_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
		editmapicon_bits, editmapicon_width, editmapicon_height,
D 9
		fgcolor, bgcolor, 1);
E 9
I 9
		fgcolor, bgcolor, depth);
E 9
  wmhints.icon_pixmap = icon_pix;
  wmhints.flags = IconPixmapHint;
D 8
  XSetWMHints(display, editwin, &wmhints);
E 8
I 8
  XSetWMHints(display, win, &wmhints);
E 8
  /* don't free the pixmap, it has to stick around to be the icon */
}



E 5
D 14
/* make the previously set up windows visible */

E 14
map_editor_windows()
I 14
/* make the previously set up windows visible */
E 14
{
  /* start up input in the windows */
  init_input_lines(editwin);
  init_input_lines(grabwin);
  init_input_lines(roomwin);
  init_input_lines(statwin);
I 13
  init_input_lines(clickwin);
  init_input_lines(pilewin);
E 13
  init_input_lines(help_rec->win);
D 14
  init_input_lines(info_rec->win);
E 14

  /* map the windows */
  XMapWindow(display, editwin);
  XMapWindow(display, roomwin);
  XMapWindow(display, grabwin);
  XMapWindow(display, statwin);
I 13
  XMapWindow(display, clickwin);
  XMapWindow(display, pilewin);
E 13
}



I 14
editor_setup_variables()
E 14
D 3
/* set up the variables about what is on the screen */
E 3
I 3
/* set up the variables about what is on the screen, and set the initial
   background of the room window */
E 3
D 14

editor_setup_variables()
E 14
{
D 15
  int i;
  
E 15
I 3
  /* pick starting room and set window background */
E 3
  current_room = 0;
D 3
  for (i=0; i<3; i++) click_buffer[i] = 0;
E 3
I 3
  XSetWindowBackgroundPixmap(display, roomwin,
D 12
		      obj_pixmap[(unsigned char)room[current_room].floor]);
E 12
I 12
		      obj_pixmap[room[current_room].floor]);
E 12
  XClearWindow(display, roomwin);
E 3

I 3
D 15
  /* clean out click buffer */
  for (i=0; i<3; i++) {
    click_buffer[i] = 0;
D 12
    click_recd[i] = NULL;
E 12
  }
E 15
I 15
  /* initialize click buffer */
  click.num_buffers = 3;
  click.map = lib_create_new_map(NULL);
E 15

E 3
  /* init paste buffer variables */
  paste.is_set = FALSE;
I 12
  paste.map = lib_create_new_map(NULL);
D 14
  add_new_room(paste.map);
E 14
E 12
  paste.width = 0;
  paste.height = 0;
I 14

  /* initialize the pile window contents */
  bzero(&pile_content, sizeof(Pile));
E 14
}



I 5
load_scroll_button_pixmaps()
{
  scrollup = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			scrollup_bits, scrollup_width, scrollup_height,
D 9
			fgcolor, bgcolor, 1);
E 9
I 9
			fgcolor, bgcolor, depth);
E 9
D 12
  scrolldown = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
E 12
I 12
  scrolldown = XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
E 12
			scrolldown_bits, scrolldown_width, scrolldown_height,
D 9
			fgcolor, bgcolor, 1);
E 9
I 9
			fgcolor, bgcolor, depth);
E 9
  pageup = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			pageup_bits, pageup_width, pageup_height,
D 9
			fgcolor, bgcolor, 1);
E 9
I 9
			fgcolor, bgcolor, depth);
E 9
  pagedown = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			pagedown_bits, pagedown_width, pagedown_height,
D 9
			fgcolor, bgcolor, 1);
E 9
I 9
			fgcolor, bgcolor, depth);
E 9
}
E 5

I 5


E 5
/* ============================== I N P U T ============================== */

/* the main editing procedure for the program */

editor_get_input()
{
  int   i, x, y, detail, quit = FALSE;
  char  c;
  Window happenwin;
  
  while (!quit) {
    i = get_input(grabwin, &c, &x, &y, &detail, &happenwin);

    switch (i) {
      case NOTHING:	break;
      case KEYBOARD:	quit = handle_editor_keypress(c, x, y, happenwin);
			break;
D 6
      case EXPOSED:	redraw_a_window(happenwin);
E 6
I 6
      case EXPOSED:	/* no longer need this, get_input() handle these */
			/* redraw_a_window(happenwin); */
E 6
			break;
      case MOUSE:	handle_editor_mouse(x, y, c, detail, happenwin);
			break;
      default:		break;
    }

  }
  /* if quitting, see that last few X events occur */
  if (quit) XFlush(display);
}



/* handle key press and return TRUE if we should quit, otherwise return 0 */

handle_editor_keypress(c, x, y, what_win)
char c;
int x, y;
Window what_win;
{
D 13
  /* message area of previous garbage */
E 13
I 13
  int result = FALSE;

  /* clear message area of previous garbage */
E 13
  place_editor_message(" ");
  
  if (isupper(c)) c = tolower(c);

  switch (c) {
I 10
D 13
	case '*':	DialogEditMapInfo();
E 13
I 13
D 14
	case 'm':	DialogEditMapInfo();
E 14
I 14
	case 'm':	DialogEditMapInfo(roomwin, -WINDOW_BORDERS,
					  -WINDOW_BORDERS);
E 14
E 13
			break;
I 11
D 13
	case '#':	DialogEditRoomInfo();
E 13
I 13
D 14
	case 'r':	DialogEditRoomInfo();
E 14
I 14
	case 'r':	DialogEditRoomInfo(roomwin, -WINDOW_BORDERS,
					   -WINDOW_BORDERS);
E 14
E 13
			break;
D 13
	case '%':	edit_recorded_object_variables(current_room,
					CONVERTRX(x), CONVERTRY(y));
			break;
E 13
E 11
E 10
	case 'q':	if (!want_query("Do you really want to quit?")) break;
I 14
	  		will_save_msg();
E 14
			save_map();
			save_msg();
D 13
			return TRUE;
E 13
I 13
			result = TRUE;
E 13
			break;
	case '!':	if (!want_query("Really quit without saving?")) break;
D 13
			return TRUE;
E 13
I 13
			result = TRUE;
E 13
			break;

	case 'c':	if (what_win == roomwin) {
			  if (!paste.is_set)
			    clear_masked_in_square(CONVERTRX(x), CONVERTRY(y));
			  else
			    erase_area(current_room, CONVERTRX(x),
				       CONVERTRY(y), TRUE);
	                }
			break;
	case 'w':	if (what_win == roomwin) {
			  if (!paste.is_set)
			    clear_square(CONVERTRX(x),
						   CONVERTRY(y));
			  else
			    erase_area(current_room, CONVERTRX(x),
				       CONVERTRY(y), FALSE);
	                }
			break;
D 13
	case 'r':	if (what_win == grabwin)
E 13
I 13
D 15
	case 'z':	if (what_win == grabwin)
E 13
  			  replace_in_room(obj_in_grab_win(x, y),
					  click_buffer[0]);
			break;
E 15
	case 'f':	if (what_win == grabwin)
			  set_room_floor(obj_in_grab_win(x, y), TRUE);
			else if (what_win == roomwin)
			  set_room_floor(probable_floor_here(current_room,
					 CONVERTRX(x), CONVERTRY(y)), FALSE);
			break;
	case '-':	if (current_room>0) change_rooms(current_room - 1);
			break;
	case '=':
	case '+':	if ((current_room + 1) < mapstats.rooms)
			  change_rooms(current_room + 1);
D 13
			else change_to_new_room();
E 13
			break;
	case 'g':	if (what_win == roomwin)
			  follow_cursor_to_next_room(CONVERTRX(x),
						     CONVERTRY(y));
I 13
	  		else enter_new_room_number();
E 13
			break;
D 13
	case 't':	enter_new_room_number();
E 13
I 13
	case 't':	toggle_cursor_mode();
E 13
			break;
I 14
	case 'a':	change_cursor_mode(ADD_MODE);
	  		break;
	case 'd':	change_cursor_mode(DELETE_MODE);
	  		break;
	case 'x':	change_cursor_mode(EXCHANGE_MODE);
	  		break;
	case 'v':	change_cursor_mode(OVERWRITE_MODE);
	  		break;
E 14
D 13
	case 'm':	if (what_win == roomwin)
E 13
I 13
	case 'n':	if (what_win == roomwin) {
E 13
			  follow_cursor_and_make_room(CONVERTRX(x),
						      CONVERTRY(y));
I 13
			}
	  		else {
			  add_new_room();
			  change_rooms(mapstats.rooms - 1);
			}
E 13
			break;
I 4
	case 'j':	if (what_win == roomwin)
D 12
	  make_random_room(CONVERTRX(x),
			   CONVERTRY(y));
E 12
I 12
	  		  make_random_room(CONVERTRX(x),
					   CONVERTRY(y));
E 12
			break;
E 4
	case '1':	if (what_win == roomwin) {
			  if (link_status == DORMANT)
			    startup_linkage(CONVERTRX(x), CONVERTRX(y));
			  else finalize_link(current_room, CONVERTRX(x),
					     CONVERTRY(y));
	                }
			break;
	case '2':	if (what_win == roomwin) {
			  if (link_status == DORMANT)
			    startup_linkage(CONVERTRX(x), CONVERTRX(y));
			  else finalize_two_way_link(current_room,
						CONVERTRX(x), CONVERTRY(y));
	                }
			break;
D 13
	case 'a':	if (what_win == roomwin && link_status != DORMANT)
			  stop_linking();
			break;
E 13
	case 'k':	if (what_win == roomwin)
			  destroy_link(current_room, CONVERTRX(x),
				       CONVERTRY(y));
I 13
	  		else if (what_win == clickwin)
			  stop_linking();
E 13
			break;
	case ')':	if (what_win == roomwin) clear_room();
			break;
D 14
	case 's':	save_map();
E 14
I 14
	case 's':	will_save_msg();
	  		save_map();
E 14
			save_msg();
			break;
	case '.':	if (what_win == roomwin)
			  set_buffer_mark(current_room, CONVERTRX(x),
					  CONVERTRY(y));
			break;
	case 'b':	if (what_win == roomwin)
			  copy_into_paste_buffer(current_room,
						 CONVERTRX(x), CONVERTRY(y));
			break;
	case 'p':	if (what_win == roomwin)
			  put_out_buffer_contents(current_room,
						 CONVERTRX(x), CONVERTRY(y));
			break;
D 12
	case 'i':	handle_info_request(what_win, x, y, FALSE);
E 12
I 12
	case 'i':	handle_info_request(what_win, x, y);
E 12
			break;
D 12
	case 'v':	handle_info_request(what_win, x, y, TRUE);
			break;
	case 'd':	if (what_win == roomwin)
			  enter_recorded_obj_variable(current_room,
					CONVERTRX(x), CONVERTRY(y), 1);
			break;
	case 'x':	if (what_win == roomwin)
			  enter_recorded_obj_variable(current_room,
					CONVERTRX(x), CONVERTRY(y), 2);
			break;
	case 'y':	if (what_win == roomwin)
			  enter_recorded_obj_variable(current_room,
					CONVERTRX(x), CONVERTRY(y), 3);
			break;
	case 'z':	if (what_win == roomwin)
			  enter_recorded_obj_variable(current_room,
					CONVERTRX(x), CONVERTRY(y), 4);
			break;
	case 'e':	if (what_win == roomwin)
			  enter_recorded_obj_variable(current_room,
					CONVERTRX(x), CONVERTRY(y), -1);
			break;
	case 'u':	enter_a_user_filename();
			break;
	case 'n':	enter_appearance_mode();
			break;
E 12
	case 'o':	toggle_owning_team();
			break;
D 12
	case 'l':	toggle_room_lighting();
			break;
E 12
D 13
	case '&':	change_num_of_teams();
E 13
I 13
	case NUM_OF_TEAMS_KEY:
	  		change_num_of_teams();
E 13
			break;
I 3
D 13
	case '^':	toggle_neutrals_allowed();
E 13
I 13
	case NEUTRALS_KEY:
			toggle_neutrals_allowed();
E 13
			break;
E 3
	case 'h':
	case '?':	toggle_help_window(help_rec);
			break;
	default:	break;
  }
D 13
  
  return FALSE;
E 13
I 13

D 14
  last_command_key = c;
E 14
I 14
  last_command_key = c;    /* some command like to apply on second request */
E 14
  return(result);
E 13
}



handle_editor_mouse(x, y, button, detail, what_window)
I 14
/* determine what to do with a mouse button press event depending on
   which window it occurred in. */
E 14
int x, y;
char button;
int  detail;
Window what_window;
{
D 14
  int cx, cy;

E 14
D 5
  if (what_window == grabwin) {
    switch (button) {
      case Button1:		click_buffer[0] = obj_in_grab_win(x, y);
I 3
				click_recd[0] = NULL;
E 3
				redraw_click_buffer(0);
				break;
      case Button2:		click_buffer[1] = obj_in_grab_win(x, y);
I 3
				click_recd[1] = NULL;
E 3
				redraw_click_buffer(1);
				break;
      case Button3:		click_buffer[2] = obj_in_grab_win(x,y);
I 3
				click_recd[2] = NULL;
E 3
				redraw_click_buffer(2);
      default:			break;
    }
  }


E 5
I 5
  if (what_window == grabwin) handle_grabwin_mouse(x, y, button, detail);
E 5
D 14
  else if (what_window == roomwin) {

    /* make a converted x and y for use by fill_area */
    cx = CONVERTRX(x);
    cy = CONVERTRY(y);
    
    /* check to see whether the mark has been set and we want to use
       the fill procedure */

    if (paste.is_set)
      switch (button) {
        case Button1:		fill_area(current_room, cx, cy,
					  click_buffer[0]);
				break;
        case Button2:		fill_area(current_room, cx, cy,
					  click_buffer[1]);
				break;
        case Button3:		fill_area(current_room, cx, cy,
					  click_buffer[2]);
	default:		break;
    }
    else {
      if (!(detail & ShiftMask)) 
       switch (button) {
        case Button1:		put_down_obj(x, y, click_buffer[0]);
				break;
        case Button2:		put_down_obj(x, y, click_buffer[1]);
				break;
        case Button3:		put_down_obj(x, y, click_buffer[2]);
				break;
	default:		break;
       }
      else
       switch (button) {
	/* shift button held down, grab room characters */
	case Button1:		click_buffer[0] =
				  most_prominant_on_square(
				    current_room, cx, cy);
				redraw_click_buffer(0);
				break;
	case Button2:		click_buffer[1] =
				  most_prominant_on_square(
				    current_room, cx, cy);
				redraw_click_buffer(1);
				break;
	case Button3:		click_buffer[2] =
				  most_prominant_on_square(
				    current_room, cx, cy);
				redraw_click_buffer(2);
				break;
        default:		break;
       }
    }
  }

  
E 14
I 14
  else if (what_window == roomwin) handle_roomwin_mouse(x, y, button, detail);
  else if (what_window == pilewin) handle_pilewin_mouse(x, y, button, detail);
  else if (what_window == clickwin) handle_clickwin_mouse(x, y, button, detail);
E 14
  else if (what_window == statwin) handle_statwin_mouse(x, y, button, detail);
D 14
  
E 14
}



I 5
D 14

/* handle the actions taken if mouse button was pressed in the grab window */
E 14
handle_grabwin_mouse(x, y, button, detail)
I 14
/* handle the actions taken if mouse button was pressed in the grab window */
E 14
int x, y;
char button;
int detail;
{
D 7
  if (y >= 0 && y < (GRAB_WIN_TOPAREA * (BITMAP_HEIGHT + GRAB_SPACING))) {
E 7
I 7
  /* check mouse clicked in a scroll button and that scrolling is possible */
  if ((y >= 0) && (y < (GRAB_WIN_TOPAREA * (BITMAP_HEIGHT + GRAB_SPACING))) &&
      (objects > (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) * GRAB_WIN_WIDTH)) {
E 7
    /* figure out which scroll button was selected and do the scrolling */
    int col = x / (BITMAP_WIDTH + GRAB_SPACING);
    switch (col) {
	case 0:
D 13
	  scroll_grab_window(1);
E 13
I 13
	  scroll_grab_window(-1);
E 13
	  break;
	case 1:
D 13
	  scroll_grab_window(-1);
E 13
I 13
	  scroll_grab_window(1);
E 13
	  break;
	case 2:
D 13
	  scroll_grab_window((GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
E 13
I 13
	  scroll_grab_window(-(GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
E 13
	  break;
	case 3:
D 13
	  scroll_grab_window(-(GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
E 13
I 13
	  scroll_grab_window((GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
E 13
	  break;
	default:
	  break;
    }
  }
  else {
    /* put the object selected into the click buffer */
    switch (button) {
D 15
      case Button1:		click_buffer[0] = obj_in_grab_win(x, y);
D 12
				click_recd[0] = NULL;
E 12
				redraw_click_buffer(0);
				break;
      case Button2:		click_buffer[1] = obj_in_grab_win(x, y);
D 12
				click_recd[1] = NULL;
E 12
				redraw_click_buffer(1);
				break;
      case Button3:		click_buffer[2] = obj_in_grab_win(x,y);
D 12
				click_recd[2] = NULL;
E 12
				redraw_click_buffer(2);
      default:			break;
E 15
I 15
      case Button1:
      	if (detail & ShiftMask)
	  add_object_to_click_buffer(0, obj_in_grab_win(x, y));
	else
	  new_object_in_click_buffer(0, obj_in_grab_win(x, y));
	redraw_click_buffer(0);
	break;
      case Button2:
      	if (detail & ShiftMask)
	  add_object_to_click_buffer(1, obj_in_grab_win(x, y));
	else
	  new_object_in_click_buffer(1, obj_in_grab_win(x, y));
	redraw_click_buffer(1);
	break;
      case Button3:
      	if (detail & ShiftMask)
	  add_object_to_click_buffer(2, obj_in_grab_win(x, y));
	else
	  new_object_in_click_buffer(2, obj_in_grab_win(x, y));
	redraw_click_buffer(2);
	break;
      default:
	break;
E 15
    }
  }
}



E 5
D 14
/* check to see where the mouse was pressed in the status window and
   from that determine what we should do about it */
E 14
I 14
handle_clickwin_mouse(x, y, button, detail)
/* handle the actions taken if mouse button was pressed in the pick window */
int x, y;
char button;
int detail;
{
  if ((x < MOUSE_PIX_X) &&
      (y > (CLICK_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE))) {
    /* button was clicked on room number */
    enter_new_room_number();
  }
  else {
    /* button was cilcked in an icon box in mouse picture */
  }
}
E 14

I 14


handle_pilewin_mouse(x, y, button, detail)
/* handle the actions taken if mouse button was pressed in the pile window */
int x, y;
char button;
int detail;
{
D 15
  printf("button in pile win\n");
E 15
I 15
  if (detail & ShiftMask) {
    /* attempt to pick up and object from pilewin into clickbuffer */
    switch (button) {
    case Button1:
      pull_pile_object_into_click(x, y, 0);
      redraw_click_buffer(0);
      break;
    case Button2:
      pull_pile_object_into_click(x, y, 1);
      redraw_click_buffer(1);
      break;
    case Button3:
      pull_pile_object_into_click(x, y, 2);
      redraw_click_buffer(2);
      break;
    default:
      break;
    }
  }
  else {
    /* apply contents of click buffer to pile */
    switch (button) {
    case Button1:
      apply_square_into_pile(CLICK_SQUARE(0), index_in_pile_win(x, y));
      break;
    case Button2:
      apply_square_into_pile(CLICK_SQUARE(1), index_in_pile_win(x, y));
      break;
    case Button3:
      apply_square_into_pile(CLICK_SQUARE(2), index_in_pile_win(x, y));
      break;
    default:
      break;
    }
  }
E 15
}



handle_roomwin_mouse(x, y, button, detail)
/* handle the actions taken if mouse button was pressed in the room window */
int x, y;
char button;
int detail;
{
  int cx, cy;

D 15
  /* make a converted x and y for use by fill_area */
E 15
I 15
  /* make a converted x and y for use in locating squares in map */
E 15
  cx = CONVERTRX(x);
  cy = CONVERTRY(y);
    
  /* check to see whether the mark has been set and we want to use
     the fill procedure */

  if (paste.is_set)
    switch (button) {
    case Button1:		fill_area(current_room, cx, cy,
D 15
					  click_buffer[0]);
E 15
I 15
					  CLICK_SQUARE(0));
E 15
				break;
    case Button2:		fill_area(current_room, cx, cy,
D 15
					  click_buffer[1]);
E 15
I 15
					  CLICK_SQUARE(1));
E 15
				break;
    case Button3:		fill_area(current_room, cx, cy,
D 15
					  click_buffer[2]);
E 15
I 15
					  CLICK_SQUARE(2));
E 15
    default:		break;
    }
  else {
    if (!(detail & ShiftMask)) 
      switch (button) {
D 15
      case Button1:		put_down_obj_check_pile(x, y, click_buffer[0]);
				break;
      case Button2:		put_down_obj_check_pile(x, y, click_buffer[1]);
				break;
      case Button3:		put_down_obj_check_pile(x, y, click_buffer[2]);
				break;
      default:		break;
E 15
I 15
      case Button1:
	apply_square_using_mode(&mapstats, current_room, cx, cy,
				CLICK_SQUARE(0), editor_mode);
	draw_room_square_check_pile(cx, cy, FALSE);
	break;
      case Button2:
	apply_square_using_mode(&mapstats, current_room, cx, cy,
				CLICK_SQUARE(1), editor_mode);
	draw_room_square_check_pile(cx, cy, FALSE);
	break;
      case Button3:
	apply_square_using_mode(&mapstats, current_room, cx, cy,
				CLICK_SQUARE(2), editor_mode);
	draw_room_square_check_pile(cx, cy, FALSE);
	break;
      default:
	break;
E 15
      }
    else
      switch (button) {
	/* shift button held down, grab room characters */
D 15
      case Button1:		click_buffer[0] =
				  most_prominant_on_square(
				    current_room, cx, cy);
E 15
I 15
      case Button1:		pull_square_into_click(cx, cy, 0);
E 15
				redraw_click_buffer(0);
				break;
D 15
      case Button2:		click_buffer[1] =
				  most_prominant_on_square(
				    current_room, cx, cy);
E 15
I 15
      case Button2:		pull_square_into_click(cx, cy, 1);
E 15
				redraw_click_buffer(1);
				break;
D 15
      case Button3:		click_buffer[2] =
				  most_prominant_on_square(
				    current_room, cx, cy);
E 15
I 15
      case Button3:		pull_square_into_click(cx, cy, 2);
E 15
				redraw_click_buffer(2);
				break;
      default:		break;
      }
  }
}



E 14
handle_statwin_mouse(x, y, button, detail)
I 14
/* check to see where the mouse was pressed in the status window and
   from that determine what we should do about it */
E 14
int x, y;
char button;
int detail;
{
  if (y > STAT_TOP_EDGE) {
D 13
    if (y < STAT_TOP_EDGE + STAT_REG_SPACE + STAT_BIG_SPACE)
E 13
I 13
    if (y < STAT_TOP_EDGE + STAT_REG_SPACE + STAT_PAD_SPACE)
E 13
	enter_new_map_name();
D 13
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE*2 + STAT_BIG_SPACE)
	enter_new_room_number();
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE*2 + STAT_BIG_SPACE*2)
E 13
I 13
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE * 2 + STAT_PAD_SPACE * 2)
E 13
	enter_new_room_name(current_room);
D 13
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE*3 + STAT_BIG_SPACE*3)
E 13
I 13
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE * 3 + STAT_PAD_SPACE * 3)
E 13
        enter_new_team_name(room[current_room].team);
    else return;
  }

D 13
  place_various_editor_names();
E 13
  place_editor_message(" ");
}



I 14
scroll_grab_window(lines)
E 14
I 5
/* scroll the grab window the given number of lines, positive number
   for scrolling upward, negative number for scrolling downward */
D 14

scroll_grab_window(lines)
E 14
int lines;
{
  int num_copy_lines, non_copy_lines, scroll_up;

  /* figure out whether boundry conditions keep us from scrolling the
     full amount requested */
  if ((first_in_grab + (lines * GRAB_WIN_WIDTH)) < 0)
    lines = -(first_in_grab / GRAB_WIN_WIDTH);
  if ((first_in_grab + (lines * GRAB_WIN_WIDTH) +
       ((GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA - 1) * GRAB_WIN_WIDTH)) >= objects)
    lines = (objects - (first_in_grab +
	    ((GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA - 1) * GRAB_WIN_WIDTH))) /
	    GRAB_WIN_WIDTH;
  if (lines == 0) return;

  num_copy_lines = (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) - abs(lines);
  non_copy_lines = (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) - num_copy_lines;
  scroll_up = (lines >= 0);

  /* copy the lines that we can copy */
  if (num_copy_lines > 0) {
    int srcX, srcY, dstX, dstY, width, height;
    srcX = dstX = GRAB_SPACING;
    width = GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING);
    height = num_copy_lines * (BITMAP_HEIGHT + GRAB_SPACING);
    srcY = (non_copy_lines + GRAB_WIN_TOPAREA) * 
	   (BITMAP_HEIGHT + GRAB_SPACING) + GRAB_SPACING;
    dstY = GRAB_WIN_TOPAREA * (BITMAP_HEIGHT + GRAB_SPACING) + GRAB_SPACING;
    if (!scroll_up) {
	int tmp = srcY;
	srcY = dstY;
	dstY = tmp;
    }
    XCopyArea(display, grabwin, grabwin, mainGC, srcX, srcY,
	      width, height, dstX, dstY);
  }

  first_in_grab += (lines * GRAB_WIN_WIDTH);

  /* rewrite the lines we couldn't copy */
  if (non_copy_lines > 0) {
    int total_lines = (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA);

    if (scroll_up) {
      /* clear the area covered by last line, in case new line is not full */
      clear_area(grabwin, 0, (GRAB_WIN_HEIGHT - 1) *
		 (BITMAP_HEIGHT + GRAB_SPACING) + GRAB_SPACING,
		 GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING),
		 GRAB_WIN_HEIGHT * (BITMAP_HEIGHT + GRAB_SPACING));
      redraw_grab_window(total_lines - non_copy_lines, total_lines - 1);
    }
    else redraw_grab_window(0, non_copy_lines - 1);
  }
}



E 5
D 13
/* get a new name for the map */

E 13
enter_new_map_name()
I 13
/* get a new name for the map */
E 13
{
D 12
  strcpy(mapstats.name,
	 get_editor_input_line("Map name: ", MAP_NAME_LENGTH - 1));
E 12
I 12
  FREE(mapstats.name);
  mapstats.name =
    create_string(get_editor_input_line("Map name: ", MAP_NAME_LENGTH - 1));
I 13
  place_editor_map_name();
E 13
E 12
}


D 13
/* get a new name for some room and assign it */
E 13

enter_new_room_name(roomnum)
I 13
/* get a new name for some room and assign it */
E 13
int roomnum;
{
D 12
  strcpy(room[roomnum].name,
	 get_editor_input_line("Room name: ", ROOM_NAME_LENGTH - 1));
E 12
I 12
  FREE(room[roomnum].name);
  room[roomnum].name =
    create_string(get_editor_input_line("Room name: ", ROOM_NAME_LENGTH - 1));
I 13
  place_editor_room_name();
E 13
E 12
}


D 13
/* get a new name for some team */
E 13

enter_new_team_name(teamnum)
I 13
/* get a new name for some team */
E 13
int teamnum;
{
  /* check for team number out of range */
D 12
  if (teamnum < 1 || teamnum > NUM_OF_TEAMS) return;
E 12
I 12
  if (teamnum < 0 || teamnum > NUM_OF_TEAMS) return;
E 12
  
D 12
  strcpy(mapstats.team_name[teamnum - 1],
	 get_editor_input_line("New team name: ", TEAM_NAME_LENGTH - 1));
E 12
I 12
  FREE(mapstats.team_name[teamnum]);
  mapstats.team_name[teamnum] =
    create_string(get_editor_input_line("New team name: ",
					TEAM_NAME_LENGTH - 1));
I 13
  place_editor_team_name();
E 13
E 12
}


D 13
/* get a new current room number and change to there */
E 13

enter_new_room_number()
I 13
/* get a new current room number and change to there */
E 13
{
I 14
  char s[250];
E 14
  int newroom;
D 14
  
  newroom = atoi(get_editor_input_line("Room number: ", 10));
E 14

D 14
  if (newroom >= 0 && newroom < mapstats.rooms) change_rooms(newroom);
E 14
I 14
  if (mapstats.rooms > 1) {
    sprintf(s, "Switch to room number (0-%d): ", mapstats.rooms - 1);
    newroom = atoi(get_editor_input_line(s, 10));
    if (newroom >= 0 && newroom < mapstats.rooms) change_rooms(newroom);
    else place_editor_message("Invalid room number.");
  }
  else {
    place_editor_message("There is only one room in this map.");
  }
E 14
}



D 13
/* toggle the team that owns current room and redraw the ownership */

E 13
toggle_owning_team()
I 13
/* toggle the team that owns current room and redraw the ownership */
E 13
{
  room[current_room].team++;
  if (room[current_room].team > mapstats.teams_supported)
    room[current_room].team = 0;

D 13
  place_various_editor_names();
E 13
I 13
  place_editor_team_name();
E 13
}



D 12
/* toggle the "is dark" variable for the current room, and display
   a message about its new status. */

toggle_room_lighting()
{
D 3
  if (room[current_room].dark) {
    room[current_room].dark = FALSE;
E 3
I 3
  if (room[current_room].dark == DAYLIT) {
    room[current_room].dark = LIT;
E 3
    place_editor_message("Room is now marked as lit.");
  }
D 3
  else {
    room[current_room].dark = TRUE;
E 3
I 3
  else if (room[current_room].dark == LIT) {
    room[current_room].dark = GLOBAL;
    place_editor_message("Room is now globally visible.");
  }
  else if (room[current_room].dark == GLOBAL) {
    room[current_room].dark = DARK;
E 3
    place_editor_message("Room is now marked as dark.");
  }
I 3
  else if (room[current_room].dark == DARK) {
    room[current_room].dark = DAYLIT;
    place_editor_message("Room is now marked as day lit.");
  }
E 3
}



E 12
D 13
/* change the number of teams in the game */

E 13
change_num_of_teams()
I 13
/* change the number of teams allowed in the game map */
E 13
{
D 3
  char msg[80];
E 3
I 3
  char msg[80], *neu = "(and no neutrals)";
E 3

  /* add one to the number of teams */
D 13
  mapstats.teams_supported++;
E 13
I 13
  if (last_command_key == NUM_OF_TEAMS_KEY) mapstats.teams_supported++;
E 13
  if (mapstats.teams_supported > NUM_OF_TEAMS)  mapstats.teams_supported = 1;

  /* tell user about the change */
I 3
  if (mapstats.neutrals_allowed) 
    neu = "(plus neutrals)";
E 3
  if (mapstats.teams_supported == 1)
D 3
    sprintf(msg, "Just one player team now.");
E 3
I 3
    sprintf(msg, "Now only one team %s", neu);
E 3
  else
D 3
    sprintf(msg, "Now there are %d teams.", mapstats.teams_supported);
E 3
I 3
    sprintf(msg, "Now %d teams %s", mapstats.teams_supported, neu);
E 3

  place_editor_message(msg);
}



I 3
toggle_neutrals_allowed()
{
D 13
  if (mapstats.neutrals_allowed) {
    mapstats.neutrals_allowed = FALSE;
    place_editor_message("Neutrals no longer allowed.");
  }
  else {
    mapstats.neutrals_allowed = TRUE;
E 13
I 13
  if (last_command_key == NEUTRALS_KEY)
    mapstats.neutrals_allowed = !mapstats.neutrals_allowed;

  if (mapstats.neutrals_allowed)
E 13
    place_editor_message("Neutrals now allowed in this map.");
D 13
  }
E 13
I 13
  else
    place_editor_message("Neutrals not allowed.");
E 13
}



E 3
D 12
/* get a piece of information to place into a recorded object on some
   square, check of course to see if a recorded object exists there.
   which_variable may have following values:
     1: change detail variable
     2: change x      variable
     3: change y      variable
     4: change zinger variable
     5 or more: extra variables */

enter_recorded_obj_variable(roomnum, x, y, which_variable)
int roomnum, x, y, which_variable;
{
  MemObj *objrec;
  char prompt[80];
  int  result;

  /* get hold of the recorded obj at this spot */
  objrec = what_recorded_obj_here(roomnum, x, y);

  /* if there is none here print message and return */
D 11
  if (objrec == NULL) {
E 11
I 11
  if (!objrec) {
E 11
    place_editor_message("No recorded object there");
    return;
  }

  /* if which_variable is not a good value, then get a good one */
  if (which_variable < 1 || which_variable > 4) 
    which_variable = get_obj_variable_number();

  /* set the correct prompt */
  switch (which_variable) {
	case 1:	strcpy(prompt, "Detail: ");
		break;
	case 2:	strcpy(prompt, "X: ");
		break;
	case 3:	strcpy(prompt, "Y: ");
		break;
	case 4: strcpy(prompt, "Zinger: ");
		break;
	case 8: return;	/* no change option selected */
		break;
	default:sprintf(prompt, "Extra %d: ", which_variable-4);
		break;
  }

  /* ask the question and get result */
  result = atoi(get_editor_input_line(prompt, 15));

  /* assign the result to the appropriate variable */
  switch (which_variable) {
	case 1:	objrec->obj.detail = result;
		break;
	case 2:	objrec->obj.infox = result;
		break;
	case 3:	objrec->obj.infoy = result;
		break;
	case 4:	objrec->obj.zinger = result;
		break;
	default:objrec->obj.extra[which_variable - 5] = result;
		break;
  }
}

I 11


E 12
D 13
edit_recorded_object_variables(roomnum, x, y)
/* look for a recorded object on the square and allow editing of the
   instance values saved in the recorded object record with a dialog box. */
int roomnum, x, y;
{
D 12
  MemObj *objrec;
E 12
I 12
  place_editor_message("No way to select which object yet.");
E 12

D 12
  /* get hold of the recorded obj at this spot */
  objrec = what_recorded_obj_here(roomnum, x, y);

  /* if there is none here print message and return */
  if (!objrec) {
    place_editor_message("No recorded object there");
    return;
  }

  DialogEditRecObj(&(objrec->obj));
E 12
I 12
  /*  DialogEditRecObj(&(objrec->obj)); */
E 12
}
E 11



E 13
D 12
/* get a choice about which variable in a recorded object to alter */

int get_obj_variable_number()
{
  char *line[11];

  line[0] = "";
  line[1] = "      Select which variable to change:      ";
  line[2] = "  1 - 'detail' (ammo counting/room number)  ";
  line[3] = "  2 - 'x'      (id number/x location)";
  line[4] = "  3 - 'y'      (y location)";
  line[5] = "  4 - 'zinger' (magical property)";
  line[6] = "  5 - 'extra 1'";
  line[7] = "  6 - 'extra 2'";
  line[8] = "  7 - 'extra 3'";
  line[9] = "  8 - Change nothing";
  line[10] = "";

  return menu_query(line, 11, 8, TRUE);
}



/* get one of the user file names:
     1: execute file
     2: startup file
     3: placement file
     4: object definition file */

enter_a_user_filename()
{
  char prompt[80], *result;
  int  which_file;

  /* figure out which file name they would like to change */
  which_file = get_filename_number();

  /* set the correct prompt */
  switch (which_file) {
	case 1:	strcpy(prompt, "Execute file: ");
		break;
	case 2:	strcpy(prompt, "Startup file: ");
		break;
	case 3:	strcpy(prompt, "Placement file: ");
		break;
	case 4: strcpy(prompt, "Object file: ");
		break;
	default:return;	/* quit without changing option given */
		break;
  }

  /* ask the question and get result */
  result = get_editor_input_line(prompt, PATH_LENGTH);

  /* assign the result to the appropriate variable */
  switch (which_file) {
	case 1:	strcpy(mapstats.execute_file, result);
		break;
	case 2:	strcpy(mapstats.startup_file, result);
		break;
	case 3:	strcpy(mapstats.placement_file, result);
		break;
	case 4:	strcpy(mapstats.objfilename, result);
		break;
  }
}




/* get a choice about which variable in a recorded object to alter */

int get_filename_number()
{
  char *line[10];
  char name[4][PATH_LENGTH + 50];

  /* make up the lines that contain file names */
  sprintf(name[0], " 1 - Exec:  %s", mapstats.execute_file);
  sprintf(name[1], " 2 - Start: %s", mapstats.startup_file);
  sprintf(name[2], " 3 - Place: %s", mapstats.placement_file);
  sprintf(name[3], " 4 - Def:   %s", mapstats.objfilename);

  line[0] = "";
  line[1] = "     Select which file name to change:    ";
  line[2] = name[0];
  line[3] = name[1];
  line[4] = name[2];
  line[5] = name[3];
  line[6] = " 5 - Change nothing";
  line[7] = "";

  return menu_query(line, 8, 5, TRUE);
}




/* enter one of the following room appearance modes:
     1: objects never appear here
     2: objects appear on designated floor only
     3: objects appear anywhere in room */

enter_appearance_mode()
{
  int result;
  char *line[10];
  char state[120];

  /* make up the line that contains current appearance mode */
  switch (room[current_room].appearance) {
	case PLACE_NEVER:	
	  strcpy(state, " Currently, objects will not appear in this room ");
	  break;
	case PLACE_FLOOR_ONLY:
	  strcpy(state, " Currently, objects will appear only on set floor ");
	  break;
	case PLACE_ANYWHERE:
	  strcpy(state, " Currently, objects may appear anywhere in room ");
	  break;
	default:
	  strcpy(state, "Appearance mode is not yet defined");
	  break;
  }

  /* find out what they want to change the mode to */
  line[0] = "";
  line[1] = state;
  line[2] = " Select a new appearance mode: ";
  line[3] = " 1 - Objects and persons never appear here ";
  line[4] = " 2 - Objects and persons appear only on designated floor ";
  line[5] = " 3 - They may appear on any legal location in room ";
  line[6] = " 4 - Do not change the appearance mode for this room ";
  line[7] = "";

  result = menu_query(line, 8, 4, FALSE);


  /* assign the appropriate mode */
  switch (result) {
	case 1:	room[current_room].appearance = PLACE_NEVER;
		place_editor_message("Objects won't appear here anymore.");
		break;
	case 2:	room[current_room].appearance = PLACE_FLOOR_ONLY;
		place_editor_message("Objects now appear on floor only.");
		break;
	case 3:	room[current_room].appearance = PLACE_ANYWHERE;
		place_editor_message("Objects may now appear anywhere.");
		break;
	case 4:	place_editor_message("Appearance mode unchanged.");
		break;
  }
}




/* get a line of input in editor message area */

E 12
char *get_editor_input_line(prompt, maxlen)
I 12
/* get a line of input in editor message area */
E 12
char *prompt;
int maxlen;
{
  int exposed;
  
  /* clear the message area */
D 13
  clear_area(statwin, 8, STAT_WIN_HEIGHT - 32, MOUSE_PIX_X - 20,
	     STAT_WIN_HEIGHT);
E 13
I 13
  clear_area(statwin, 8, STAT_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
	     STAT_WIN_W, STAT_WIN_H);
E 13
	     
D 13
  return get_string(statwin, regfont, STAT_LEFT_END, STAT_WIN_HEIGHT - 32,
E 13
I 13
  return get_string(statwin, regfont, STAT_LEFT_END,
		    STAT_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
E 13
		    prompt, maxlen, &exposed);
D 13

E 13
}



D 15
/* ======================================================================= */
E 15
I 15
/* ============ C L I C K  B U F F E R S  A N D  P I L E ================== */
E 15



I 15
add_object_to_click_buffer(bufnum, type)
int bufnum, type;
{
  add_object_to_square(click.map, 0, bufnum, 0,
		       create_object(click.map, type, ANY_ID));
}



new_object_in_click_buffer(bufnum, type)
int bufnum, type;
{
  destroy_all_objects_on_square(click.map, 0, bufnum, 0);
  add_object_to_click_buffer(bufnum, type);
}



pull_square_into_click(x, y, bufnum)
/* copies contents of current_room square (x, y) into the click buffer,
   after destroying what was already in the click buffer. */
int x, y, bufnum;
{
  SquareRec *square = get_square(&mapstats, current_room, x, y);
  destroy_all_objects_on_square(click.map, 0, bufnum, 0);
  if (square) {
    OI *ptr = square->first;
    for (; ptr; ptr=ptr->next) {
      add_duplicate_to_square(click.map, 0, bufnum, 0, ptr);
    }
  }
}



int index_in_pile_win(x, y)
/* figures out which object mouse is pointing to and returns n where
   n is the nth object in the pile (starting at 0 for first object).
   If mouse points to a location past last object, then PILE_APPEND
   is returned.  If there's no pile displayed then PILE_NONE is returned. */
int x, y;
{
  int result = PILE_NONE;

  if (pile_content.pertinent) {
    result = y / (BITMAP_HEIGHT + GRAB_SPACING);
    if (result >= number_of_objects_on_square(pile_content.square))
      result = PILE_APPEND;
  }
  return(result);
}



int obj_in_grab_win(x, y)
E 15
/* find out which object should be in the x y coords of the grab window,
   if they click on an object that doesn't exist, return 0 instead of the
   expected object number */
D 15

int obj_in_grab_win(x, y)
E 15
int x, y;
{
  int line, col, result;

D 5
  line = y / (BITMAP_HEIGHT + GRAB_SPACING);
E 5
I 5
  line = (y / (BITMAP_HEIGHT + GRAB_SPACING)) - GRAB_WIN_TOPAREA;
E 5
  col = x / (BITMAP_WIDTH + GRAB_SPACING);

D 5
  result = (line * GRAB_WIN_WIDTH) + col;
  if (result >= objects) result = 0;
E 5
I 5
  result =  first_in_grab + (line * GRAB_WIN_WIDTH) + col;
  if (result >= objects || result < 0) result = 0;
E 5

  return result;
}



I 15
static void apply_square_adding(map, roomnum, x, y, square)
/* applies the objects on square to the map at given location */
MapInfo *map;
int roomnum, x, y;
SquareRec *square;
{
  copy_square_add(square, map, roomnum, x, y);
}



static void apply_square_overwriting(map, roomnum, x, y, square)
/* applies the objects on square to the map at given location */
MapInfo *map;
int roomnum, x, y;
SquareRec *square;
{
  copy_square_overwrite(square, map, roomnum, x, y);
}



apply_square_using_mode(map, roomnum, x, y, square, mode)
/* use the editor mode given to apply the square (usually the contents
   of a click buffer) to the given square on given map. */
MapInfo *map;
int roomnum, x, y;
SquareRec *square;
int mode;
{
  switch (mode) {
  case ADD_MODE:
    apply_square_adding(map, roomnum, x, y, square);
    break;
  case DELETE_MODE:
  case EXCHANGE_MODE:
  case OVERWRITE_MODE:
  default:
    apply_square_overwriting(map, roomnum, x, y, square);
    break;
  }
}



apply_square_into_pile(square, pile_index)
/* takes a square, like that in a click buffer, and uses it to act on the
   current pile contents at the given index. */
SquareRec *square;
int pile_index;
{
  if (pile_index == PILE_NONE) return;
  printf("apply_square_into_pile\n");
}



pull_pile_object_into_click(x, y, click_num)
/* figures which object is referred to in pile window and pulls it into the
   click buffer, adding it to current contents. */
int x, y, click_num;
{
  OI *ptr;
  int pile_index = index_in_pile_win(x, y);
  if (pile_index == PILE_NONE || pile_index == PILE_APPEND) return;
  ptr = object_i_on_square(pile_content.square, pile_index);
  if (ptr) add_duplicate_to_square(click.map, 0, click_num, ptr);
}



E 15
/* ======================== O U T P U T =============================== */


D 13
/* redraw the indicated window for the editor */
E 13

redraw_a_window(win)
I 13
/* redraw the indicated window for the editor */
E 13
Window win;
{
D 5
  if (win == grabwin) redraw_grab_window();
E 5
I 5
D 14
  if (win == grabwin) redraw_grab_window(-1, -1);
E 14
I 14
  if (win == roomwin) {
    redraw_room_window(TRUE);
    draw_pile_box();
  }
  else if (win == grabwin) redraw_grab_window(-1, -1);
E 14
E 5
  else if (win == statwin) redraw_stat_window();
D 14
  else if (win == roomwin) redraw_room_window(TRUE);
E 14
I 13
  else if (win == clickwin) redraw_click_window();
  else if (win == pilewin) redraw_pile_window();
E 13
  else check_help_expose(win);
}



D 13

E 13
I 13
redraw_grab_window(first, last)
E 13
D 5
/* draw the contents of the object grabbing window */
E 5
I 5
/* draw the contents of the object grabbing window, given any
   subset of the lines within to redraw (from 0 to 
   (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA - 1),
   -1 in either means to redraw the entire window. */
E 5
D 13

D 5
redraw_grab_window()
E 5
I 5
redraw_grab_window(first, last)
E 13
int first, last;
E 5
{
D 5
  int i, j, nextobj = 0;
E 5
I 5
  int i, j, nextobj;
E 5
  int x, y;

D 5
  for (j=0; j<GRAB_WIN_HEIGHT && nextobj<objects; j++)
E 5
I 5
  if (first<0 || first>last || last>=(GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA)) {
    first = 0;
    last = (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) - 1;
  }

  if (GRAB_WIN_TOPAREA > 0) redraw_grab_scroll_buttons();

  nextobj = first_in_grab + (first * GRAB_WIN_WIDTH);
  for (j=GRAB_WIN_TOPAREA+first; 
       j<GRAB_WIN_TOPAREA+last+1 && nextobj<objects; j++)
E 5
    for (i=0; i<GRAB_WIN_WIDTH && nextobj<objects; i++) {
D 5
      x = (i * (BITMAP_WIDTH + GRAB_SPACING)) + GRAB_SPACING / 2;
      y = (j * (BITMAP_HEIGHT + GRAB_SPACING)) + GRAB_SPACING / 2;
E 5
I 5
      x = (i * (BITMAP_WIDTH + GRAB_SPACING)) + GRAB_SPACING;
      y = (j * (BITMAP_HEIGHT + GRAB_SPACING)) + GRAB_SPACING;
E 5
      place_obj_pixmap(grabwin, x, y, nextobj, TRUE);
      nextobj++;
    }
}



I 5
D 13
/* redraw the scroll buttons at top of grab window */

E 13
redraw_grab_scroll_buttons()
I 13
/* redraw the scroll buttons at top of grab window */
E 13
{
  char s[500];

D 7
  /* place scroll buttons there */
  XCopyArea(display, scrollup, grabwin, mainGC, 0, 0, 
	    scrollup_width, scrollup_height,
	    (0 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
  XCopyArea(display, scrolldown, grabwin, mainGC, 0, 0, 
	    scrolldown_width, scrolldown_height,
	    (1 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
  XCopyArea(display, pageup, grabwin, mainGC, 0, 0, 
	    pageup_width, pageup_height,
	    (2 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
  XCopyArea(display, pagedown, grabwin, mainGC, 0, 0, 
	    pagedown_width, pagedown_height,
	    (3 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
E 7
I 7
  /* place scroll buttons there (if scrolling is possible) */
  if (objects > (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) * GRAB_WIN_WIDTH) {
    XCopyArea(display, scrollup, grabwin, mainGC, 0, 0, 
	      scrollup_width, scrollup_height,
D 12
	      (0 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
E 12
I 12
	      (0 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
E 12
    XCopyArea(display, scrolldown, grabwin, mainGC, 0, 0, 
	      scrolldown_width, scrolldown_height,
D 12
	      (1 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
E 12
I 12
	      (1 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
E 12
    XCopyArea(display, pageup, grabwin, mainGC, 0, 0, 
	      pageup_width, pageup_height,
D 12
	      (2 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
E 12
I 12
	      (2 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
E 12
    XCopyArea(display, pagedown, grabwin, mainGC, 0, 0, 
	      pagedown_width, pagedown_height,
D 12
	      (3 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING), GRAB_SPACING);
E 12
I 12
	      (3 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
E 12
E 7

D 7
  /* place text about object set there */
  sprintf(s, "%d objects", objects);
  text_center(grabwin, regfont,
	      (4 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),
	      (GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING)),
	       GRAB_SPACING, BITMAP_HEIGHT + GRAB_SPACING, s);
E 7
I 7
    /* place text about object set there */
    sprintf(s, "%d objects", objects);
    text_center(grabwin, regfont,
		(4 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),
		(GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING)),
		GRAB_SPACING, BITMAP_HEIGHT + GRAB_SPACING, s);
  }
  else {
    /* place text about object set there, no scroll buttons */
    sprintf(s, "%d objects", objects);
    text_center(grabwin, regfont,
		(0 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),
		(GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING)),
		GRAB_SPACING, BITMAP_HEIGHT + GRAB_SPACING, s);
  }
E 7
}



I 13
redraw_stat_window()
E 13
E 5
/* redraw the stat window */
I 13
{
  place_various_editor_names();
  place_editor_message("");	  /* redraw last message */
}
E 13

D 13
redraw_stat_window()
E 13
I 13


redraw_click_window()
/* redraw the window with the click buffer display in it */
E 13
{
  redraw_mouse_click_buffer();
D 12

E 12
  redraw_link_picture();
D 12

E 12
D 13
  place_various_editor_names();
D 12

  /* redraw last message */
  place_editor_message("");
E 12
I 12
  place_editor_message("");	  /* redraw last message */
E 13
I 13
  place_editor_room_number();
I 14
  draw_pile_box();
E 14
E 13
E 12
}



D 13
/* redraws the graphic representation of the mouse with the three click
   buffers inside */
E 13
I 13
redraw_pile_window()
/* redraws the window that contains the contents of a particular square */
{
I 14
  XClearWindow(display, pilewin);

  if (pile_content.pertinent && pile_content.square) {
    int y = GRAB_SPACING;
    OI *ptr = pile_content.square->first;
    
    for (; ptr; ptr=ptr->next) {
      place_obj_pixmap(pilewin, 0, y, ptr->type, TRUE);
      y += BITMAP_HEIGHT + GRAB_SPACING;
    }
  }
E 14
}
E 13

I 13


E 13
redraw_mouse_click_buffer()
I 13
/* redraws the graphic representation of the mouse with the three click
   buffers inside */
E 13
{
  int i;

  /* place the mouse picture pixmap */
D 13
  place_mouse_picture(statwin, MOUSE_PIX_X, MOUSE_PIX_Y);
E 13
I 13
  place_mouse_picture(clickwin, MOUSE_PIX_X, MOUSE_PIX_Y);
E 13
  
  /* place each of the three click buffer images */
  for (i=0; i<3; i++)
    redraw_click_buffer(i);
}



D 13
/* draw the contents of one of the click buffer boxes */

E 13
redraw_click_buffer(num)
I 13
/* draw the contents of one of the click buffer boxes */
E 13
int num;
{
I 15
  SquareRec *square = get_square(click.map, 0, num, 0);

E 15
  /* place the object in the right box on the mouse picture */
D 13
  place_mouse_block(statwin, MOUSE_PIX_X, MOUSE_PIX_Y, num, click_buffer[num]);
E 13
I 13
D 15
  place_mouse_block(clickwin, MOUSE_PIX_X, MOUSE_PIX_Y,
		    num, click_buffer[num]);
E 15
I 15
  if (square) {
    int x, y;
    x = (MOUSE_PIX_X + MOUSE_START_X + num * (BITMAP_WIDTH + MOUSE_SPACING));
    y = (MOUSE_PIX_Y + MOUSE_START_Y);
    draw_objects_on_square(square, clickwin, x, y, FALSE);
  }
E 15
E 13
}



I 13
redraw_link_picture()
E 13
/* draw the link picture and room number, this procedure checks for itself
   whether there is a current link or not */
D 13

redraw_link_picture()
E 13
{
  char s[10];
  
  if (link_status == DORMANT) {
D 5
    clear_area(statwin, LINK_PIX_X, LINK_PIX_Y, 
	       LINK_PIX_X + link_width, LINK_PIX_Y + link_height);
E 5
I 5
    redraw_mouse_click_buffer();
D 13
    /* clear_area(statwin, LINK_PIX_X, LINK_PIX_Y, 
	       LINK_PIX_X + link_width, LINK_PIX_Y + link_height); */
E 13
E 5
  }
  else {
D 13
    XCopyArea(display, link_pix, statwin, mainGC, 0, 0, link_width,
E 13
I 13
    XCopyArea(display, link_pix, clickwin, mainGC, 0, 0, link_width,
E 13
	      link_height, LINK_PIX_X, LINK_PIX_Y);
    sprintf(s, "%d", link_info.room);
D 13
    text_center(statwin, bigfont, LINK_PIX_X + link_x_hot, LINK_PIX_X +
E 13
I 13
    text_center(clickwin, bigfont, LINK_PIX_X + link_x_hot, LINK_PIX_X +
E 13
		link_x_hot + LINK_SPACE_WIDTH, LINK_PIX_Y + link_y_hot,
		LINK_PIX_Y + link_y_hot + LINK_SPACE_HEIGHT, s);
  }
}



I 13
place_various_editor_names()
E 13
/* place the various name associated with the editor program */
I 13
{
  place_editor_map_name();
  place_editor_room_name();
  place_editor_team_name();
}
E 13

D 13
place_various_editor_names()
E 13
I 13


place_editor_map_name()
E 13
{
D 13
  char roomnum[20];
E 13
I 13
  place_map_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE, STAT_RIGHT_END,
		 STAT_TOP_EDGE + STAT_REG_SPACE + STAT_PAD_SPACE);
}
E 13

D 13
  /* name of map */
  text_write(statwin, regfont, STAT_LEFT_END, STAT_TOP_EDGE, 0, 0, "map name");
  place_map_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE+STAT_REG_SPACE,
		 STAT_RIGHT_END, STAT_TOP_EDGE+STAT_REG_SPACE+STAT_BIG_SPACE);
E 13

D 13
  /* name of room */
  sprintf(roomnum, "room %d   ", current_room);
  text_write(statwin, regfont, STAT_LEFT_END, STAT_TOP_EDGE+STAT_REG_SPACE+
	     STAT_BIG_SPACE, 0, 0, roomnum);
  place_room_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE+(STAT_REG_SPACE*2)+
		  STAT_BIG_SPACE, STAT_RIGHT_END, STAT_TOP_EDGE+
		  (STAT_REG_SPACE*2)+(STAT_BIG_SPACE*2));
E 13

D 13
  /* team for which this is a "home room" */
  place_room_team(current_room, statwin, STAT_LEFT_END, STAT_TOP_EDGE+
		  (STAT_REG_SPACE*3)+(STAT_BIG_SPACE*2), STAT_RIGHT_END,
		  STAT_TOP_EDGE+(STAT_REG_SPACE*3)+(STAT_BIG_SPACE*3));
E 13
I 13
place_editor_room_name()
{
  place_room_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE + STAT_REG_SPACE +
		  STAT_PAD_SPACE, STAT_RIGHT_END, STAT_TOP_EDGE +
		  STAT_REG_SPACE * 2 + STAT_PAD_SPACE * 2);
E 13
}



D 13
/* place a message in the stat window in the appropriate font.  If a null
   length message is given as the message, then redraw the last message */
E 13
I 13
place_editor_team_name()
{
  place_room_team(current_room, statwin, STAT_LEFT_END,
		  STAT_TOP_EDGE + STAT_REG_SPACE * 2 + STAT_PAD_SPACE * 2,
		  STAT_RIGHT_END, STAT_TOP_EDGE + STAT_REG_SPACE * 3 +
		  STAT_PAD_SPACE * 3);
}
E 13

I 13


place_editor_room_number()
{
  char s[40];

  sprintf(s, "Room %d", current_room);
  clear_area(clickwin, 8, CLICK_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
		 MOUSE_PIX_X, CLICK_WIN_H - STAT_PAD_SPACE);
  place_a_string(clickwin, s, 8, CLICK_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
		 MOUSE_PIX_X, CLICK_WIN_H - STAT_PAD_SPACE);
}


E 13
place_editor_message(line)
I 13
/* place a message in the stat window in the appropriate font.  If a null
   length message is given as the message, then redraw the last message */
E 13
char *line;
{
  static char	current_msg[200] = "";

  if (strlen(line) != 0) strcpy(current_msg, line);

D 13
  clear_area(statwin, 8, STAT_WIN_HEIGHT - 32, MOUSE_PIX_X - 20,
	     STAT_WIN_HEIGHT);
  place_a_string(statwin, current_msg, 8, STAT_WIN_HEIGHT - 32,
		 MOUSE_PIX_X - 20, STAT_WIN_HEIGHT);
E 13
I 13
  clear_area(statwin, 8, STAT_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
	     STAT_WIN_W, STAT_WIN_H);
  place_a_string(statwin, current_msg, 8, STAT_WIN_H - STAT_REG_SPACE -
		 STAT_PAD_SPACE, STAT_WIN_W, STAT_WIN_H);
E 13
}


D 13
/* place message about the fact that map was just saved on the screen */
E 13

I 14
will_save_msg()
/* place message about the fact that map will be saved on the screen */
{
  char msg[250];
  
  /* let person using editor know that we're saving */
  sprintf(msg, "Saving %d rooms to file %s ...", mapstats.rooms, mapfile);
  place_editor_message(msg);
  XFlush(display);	/* necessary */
}



E 14
save_msg()
I 13
/* place message about the fact that map was just saved on the screen */
E 13
{
  char msg[250];
  
  /* let person using editor know that we've saved */
D 14
  sprintf(msg, "Saved %d rooms in %s\n", mapstats.rooms, mapfile);
E 14
I 14
  sprintf(msg, "%d rooms saved in file %s", mapstats.rooms, mapfile);
E 14
  place_editor_message(msg);
}



D 13
/* editor wants to change to a different room */

E 13
change_rooms(newroom)
I 13
/* editor wants to change to a different room */
E 13
int newroom;
{
  int oldroom;

  if (newroom < mapstats.rooms && newroom >= 0) {

    /* save old location */
    oldroom = current_room;

    /* change global room number */
    current_room = newroom;

    /* if we've moved to a different room then redraw */
    if (oldroom != newroom) {
I 13
      place_editor_room_name();
      place_editor_room_number();
      place_editor_team_name();
E 13
      redraw_room_window(FALSE);
I 14
      if (PILE_IN_ROOMWIN) draw_pile_box();
E 14
D 13
      place_various_editor_names();
E 13
    }
    else {
D 13

E 13
      /* otherwise, let him know it we are going to same room */
      place_editor_message("Exit led back into room");
    }
  }
  else {
D 13

  /* report that you asked to change to an invalid room number */
  place_editor_message("Exit to non-existant room");
E 13
I 13
    /* report that you asked to change to an invalid room number */
    place_editor_message("Exit to non-existant room");
E 13
  }
  
}



D 13
/* add a room and change to it */

change_to_new_room()
{
  add_new_room();
  change_rooms(mapstats.rooms - 1);
}



E 13
I 13
set_room_floor(objnum, set)
E 13
/* set the floor character for the entire room, if set flag is TRUE
   then actually replace the current floor character on all spaces */
D 13

set_room_floor(objnum, set)
E 13
int objnum, set;
{
D 12
  register int i, j;
E 12
I 12
  int i, j;
E 12

  /* if the object is a recorded one then this is not a good idea */
  if (info[(uc)objnum]->recorded) {
    place_editor_message("no recorded objects for floors");
    return;
  }

  /* place floor character on map */
  if (set) {
    for (j=0; j<ROOM_HEIGHT; j++)
      for (i=0; i<ROOM_WIDTH; i++) {
D 12
        put_down_mapped_obj(i, j, (unsigned char) objnum);
E 12
I 12
        put_down_object(i, j, objnum);
E 12
D 14
        draw_room_square(i, j, FALSE);
E 14
I 14
        draw_room_square_check_pile(i, j, FALSE);
E 14
      }
  }
      
  /* place floor character in room record */
  room[current_room].floor = objnum;

  /* change the background of the window to reflect new floor */
  XSetWindowBackgroundPixmap(display, roomwin, 
			     obj_pixmap[room[current_room].floor]);
  place_editor_message("official floor character set");
}



D 12
/* replaces all instances of a certain object in a room with another obj,
   only works for mapped characters, not recorded objects */

E 12
replace_in_room(victim, replacement)
D 12
unsigned char victim, replacement;
E 12
I 12
/* replaces all instances of a certain object in a room with another obj */
int victim, replacement;
E 12
{
D 12
  register int i, j, k;
E 12
I 12
  OI *ptr;
  int i, j;
E 12

D 12
  /* check to see if victim or replacement is a recorded object */
  if (info[(uc)victim]->recorded || info[(uc)replacement]->recorded) {
    place_editor_message("no replacing recorded objects");
    return;
  }

E 12
  for (j=0; j<ROOM_HEIGHT; j++)
    for (i=0; i<ROOM_WIDTH; i++) {
D 12
      for (k=0; k<ROOM_DEPTH; k++)
        if (room[current_room].spot[i][j][k] == victim)
	  room[current_room].spot[i][j][k] = replacement;
E 12
I 12
      ptr = first_obj_here(&mapstats, current_room, i, j);
      for (; ptr; ptr=ptr->next) {
	if (ptr->type == victim) change_object_type(ptr, replacement);
      }
E 12
D 14
      draw_room_square(i, j, FALSE);
E 14
I 14
      draw_room_square_check_pile(i, j, FALSE);
E 14
    }
}



/* ===================== C U R S O R   F O L L O W ======================= */

I 12

follow_cursor_to_next_room(x, y)
E 12
/* follow the direction or exit character that the cursor points us to,
   it is assumed that this was called while cursor was in roomwin, we
   are given the character x and y coords, we will change rooms to the
   appropriate room */
D 12

follow_cursor_to_next_room(x, y)
E 12
int x, y;
{
  int target_room, dir, start_room;
D 12
  MemObj *exit;
E 12
I 12
  OI *exit;
E 12
  
  dir = follow_insinuation(current_room, x, y, &target_room);

  if (target_room == -1) {
    place_editor_message("No room defined for this exit");
    return;
  }

  /* change current_room to new room, redraw things, etc */
  start_room = current_room;
  change_rooms(target_room);

  /* if we were following an exit character then warp mouse to entry spot */
  if (dir == -1) {
D 12
    exit = what_recorded_obj_here(start_room, x, y);
E 12
I 12
    exit = what_exit_obj_here(&mapstats, start_room, x, y);
E 12
    warp_mouse_through_exit(exit);
  }
}
D 4

E 4


I 12

follow_cursor_and_make_room(x, y)
E 12
/* this is like follow_cursor_to_next_room, but is meant to be used when
   there is no such room yet, and it must be created before we can go
   into it for editing purposes.  Make it and change into it. */
D 12

follow_cursor_and_make_room(x, y)
E 12
int x, y;
{
  int target_room, dir;
  
  dir = follow_insinuation(current_room, x, y, &target_room);

  if (target_room != -1) {
    place_editor_message("There is already a room made for this exit");
    return;
  }

  add_new_room();

  /* if it was an exit character then link it with new room */
  if (dir == -1) {
    initiate_first_link(current_room, dir, x, y);
  }
D 13
  /* other wise link the directions between the two new rooms */
E 13
I 13
  /* otherwise link the directions between the two new rooms */
E 13
  else {
    link_one_room_to_another(current_room, dir, mapstats.rooms-1);
    copy_edge_of_room(current_room, dir, mapstats.rooms-1);
  }

  /* now change to that room */
  change_rooms(mapstats.rooms-1);
}



D 12
/* start up a linkage in the current room given a particular x and y */

E 12
startup_linkage(x, y)
I 12
/* start up a linkage in the current room given a particular x and y */
E 12
int x, y;
{
  int direction;
  int destination;
  
  /* find out what is already defined for this exit */
  direction = follow_insinuation(current_room, x, y, &destination);

  /* if there is a destination room already, tell them they have to destroy
     the old link first */
  if (destination != -1) {
    place_editor_message("You must destroy the old link first");
    return;
  }

  /* otherwise go ahead and startup a link */
  initiate_first_link(current_room, direction, x, y);
}



initiate_first_link(roomnum, direction, x, y)
int roomnum, direction, x, y;
{
  link_status = EXPECTING;

  link_info.room = roomnum;
  link_info.direction = direction;

  /* if this link starts from an exit character then save x and y coords */
  if (direction == -1) {
    link_info.x = x;
    link_info.y = y;
  }

  redraw_link_picture();
}



I 12
finalize_link(dest_room, x, y)
E 12
/* complete a link started previously, this will be a one way link from one
   room (or room's exit char) to another room */
D 12

finalize_link(dest_room, x, y)
E 12
int dest_room, x, y;
{
D 12
  MemObj *exitchar;
E 12
I 12
  OI *obj;
E 12
  
  link_status = DORMANT;
  redraw_link_picture();

  /* if the direction from the previous room was a compass direction, then
     we alter the exit flag for the previous room */
  if (link_info.direction >= 0) {
    room[link_info.room].exit[link_info.direction] = dest_room;
    return;
  }

  /* otherwise, it was an exit character, and we need to find out which one */
D 12
  exitchar = what_recorded_obj_here(link_info.room, link_info.x, link_info.y);
  if (exitchar == NULL) {
E 12
I 12
  obj = what_exit_obj_here(&mapstats, link_info.room,
			   link_info.x, link_info.y);
  if (!obj) {
E 12
    place_editor_message("Bad start of link - no link made");
    return;
  }

  /* place into that char the required info */
D 12
  exitchar->obj.detail = dest_room;
  exitchar->obj.infox = x;
  exitchar->obj.infoy = y;
E 12
I 12
  set_record(obj, REC_DETAIL, dest_room);
  set_record(obj, REC_INFOX, x);
  set_record(obj, REC_INFOY, y);
E 12
}



I 12
finalize_two_way_link(dest_room, x, y)
E 12
/* complete a link started previously, this will be a TWO way link from one
   room (or room's exit char) to another room and back.  For exit chars,
   if the second room has another exit char at the link point, then this
   will work, if not, only a one way link will be created. */
D 12

finalize_two_way_link(dest_room, x, y)
E 12
int dest_room, x, y;
{
D 12
  MemObj *exitchar, *destchar;
  int	 dir;
E 12
I 12
  OI *exitchar, *destchar;
  int dir;
E 12
  
  link_status = DORMANT;
  redraw_link_picture();

  /* if the direction from the previous room was a compass direction, then
     we link the two rooms easily enough */
  if (link_info.direction >= 0) {
    link_one_room_to_another(link_info.room, link_info.direction, dest_room);
    return;
  }

  /* otherwise, it was an exit character, and we need to find out which one */
D 12
  exitchar = what_recorded_obj_here(link_info.room, link_info.x, link_info.y);
  if (exitchar == NULL) {
E 12
I 12
  exitchar = what_exit_obj_here(&mapstats, link_info.room,
				link_info.x, link_info.y);
  if (!exitchar) {
E 12
    place_editor_message("Bad start of link - no link made");
    return;
  }

  /* look for an exit char in the destination square, if there is one
     then we scold the user for trying to make an infinite loop */
D 12
  destchar = what_recorded_obj_here(dest_room, x, y);
  if (destchar != NULL)
    if (info[(uc)destchar->obj.objtype]->exit) {
E 12
I 12
  destchar = what_exit_obj_here(&mapstats, dest_room, x, y);
  if (destchar) {
E 12
    place_editor_message("No making infinite loops");
    return;
  }

  /* place into that char the required info */
D 12
  exitchar->obj.detail = dest_room;
  exitchar->obj.infox = x;
  exitchar->obj.infoy = y;
E 12
I 12
  set_record(exitchar, REC_DETAIL, dest_room);
  set_record(exitchar, REC_INFOX, x);
  set_record(exitchar, REC_INFOY, y);
E 12

  /* look for nearby exit character */
  destchar = nearby_exit_character(dest_room, x, y, &dir);

  /* if no exit character nearby make one way link only */
D 12
  if (destchar == NULL) {
E 12
I 12
  if (!destchar) {
E 12
    place_editor_message("No nearby exit - one way link only");
    return;
  }

  /* there was an exit char nearby, lets link */
D 12
  destchar->obj.detail = link_info.room;
  destchar->obj.infox = link_info.x + xdir[dir];
  destchar->obj.infoy = link_info.y + ydir[dir];
E 12
I 12
  set_record(destchar, REC_DETAIL, link_info.room);
  set_record(destchar, REC_INFOX, link_info.x + xdir[dir]);
  set_record(destchar, REC_INFOY, link_info.y + ydir[dir]);
E 12
}



D 12
/* stop a link that is in progress */

E 12
stop_linking()
I 12
/* stop a link that is in progress */
E 12
{
  link_status = DORMANT;
  place_editor_message("Linkage aborted");
  redraw_link_picture();
}



I 12
destroy_link(roomnum, x, y)
E 12
/* destroy the link in this room insinuated by cursor position, and destroy
   its link back partner in another room if applicable */
D 12

destroy_link(roomnum, x, y)
E 12
int roomnum, x, y;
{
  int direction;
  int destination;
  
  /* find out what kind of a link is there */
  direction = follow_insinuation(current_room, x, y, &destination);

  /* if there is no link then return an error message */
  if (destination == -1) {
    place_editor_message("No link here");
    return;
  }

  /* if this is a directional room link, destroy it and its partner, or if
     it is an exit character, handle it appropriately */
  if (direction >= 0) destroy_direction_link(roomnum, direction, destination);
  else destroy_exit_link(roomnum, x, y);

  place_editor_message("Link(s) destroyed");
}



/* ============================== P A S T E ============================== */


/* set the upper left corner mark of a buffer operation */

set_buffer_mark(roomnum, x, y)
int roomnum, x, y;
{
  char s[80];
  
  paste.is_set = TRUE;
  paste.room = roomnum;
  paste.x = x;
  paste.y = y;

  sprintf(s, "Mark set at %d %d", x, y);
  place_editor_message(s);
}



I 12
int is_mark_error(roomnum, x, y)
E 12
/* checks to see whether we are pointing to a correct lower right
   corner.  We return TRUE if we find an error condition, FALSE
   if we don't */
D 12

int is_mark_error(roomnum, x, y)
E 12
int roomnum, x, y;
{
  int result = TRUE;
  
  if (!paste.is_set)
    place_editor_message("Set upper right corner mark first");
  else if (roomnum != paste.room)
    place_editor_message("Mark not set in this room");
  else if (x < paste.x  ||  y < paste.y)
    place_editor_message("Must be below and right of mark");
  else result = FALSE;

  return result;
}



I 12
copy_into_paste_buffer(roomnum, spotx, spoty)
E 12
/* copy into the buffer contents of rectangle between the previously
   set mark and the given lower right corner coordinates */
D 12

copy_into_paste_buffer(roomnum, spotx, spoty)
E 12
int roomnum, spotx, spoty;
{
D 12
  int	i, j, k, x, y, z;
  char	s[80];
E 12
I 12
  OI *ptr;
  int i, j, x, y;
  char s[80];
E 12
  
  if (!is_mark_error(roomnum, spotx, spoty)) {
I 12
    destroy_all_objects_in_room(paste.map, 0);
E 12

D 12
    /* copy room contents into buffer */
    for (x=0,i=paste.x;   i<=spotx;     i++,x++)
      for (y=0,j=paste.y; j<=spoty;       j++,y++)
        for (z=0,k=0;     k<ROOM_DEPTH; k++,z++)
	  paste.spot[x][y][z] = room[roomnum].spot[i][j][k];
E 12
I 12
    for (x=0,i=paste.x; i<=spotx; i++,x++)
      for (y=0,j=paste.y; j<=spoty; j++,y++) {
	ptr=first_obj_here(&mapstats, roomnum, i, j);
	for (; ptr; ptr=ptr->next) {
	  add_duplicate_to_square(paste.map, 0, x, y, ptr);
	}
      }
E 12

    /* set buffer size */
    paste.width = spotx - paste.x + 1;
    paste.height = spoty - paste.y + 1;

    /* mark may now be unset */
    paste.is_set = FALSE;

    /* tell user about what we just did */
    sprintf(s, "Stored %d squares in cut buffer", paste.width * paste.height);
    place_editor_message(s);
  }
D 12
  
E 12
}



I 12
put_out_buffer_contents(roomnum, x, y)
E 12
/* put out the buffer contents with the given coords as the upper left
   corner, check for empty buffer condition, if the buffer contents
   overflow the screen, then the extra chars are ignored */
D 12

put_out_buffer_contents(roomnum, x, y)
E 12
int roomnum, x, y;
{
D 12
  int i, j, k, count = 0;
E 12
I 12
  OI *ptr;
  int i, j, count = 0;
E 12
  char s[80];
  
  if (paste.width == 0 || paste.height == 0)
    place_editor_message("Cut buffer is empty");
  else {

D 12
    /* place chars straight into room record */
E 12
    for (i=0; i<paste.width; i++)
      for (j=0; j<paste.height; j++)
        if ( x+i < ROOM_WIDTH  &&  y+j < ROOM_HEIGHT) {
D 12
	  for (k=0; k<ROOM_DEPTH; k++) {
	    room[roomnum].spot[x+i][y+j][k] = paste.spot[i][j][k];
	    if (roomnum == current_room)
	      draw_room_square(x+i, y+j, FALSE);
E 12
I 12
	  destroy_all_objects_on_square(&mapstats, roomnum, x+i, y+j);
	  ptr = first_obj_here(paste.map, 0, i, j);
	  for (; ptr; ptr=ptr->next) {
	    add_duplicate_to_square(&mapstats, roomnum, x+i, y+j, ptr);
E 12
	  }
I 12
	  if (roomnum == current_room)
D 14
	    draw_room_square(x+i, y+j, FALSE);
E 14
I 14
	    draw_room_square_check_pile(x+i, y+j, FALSE);
E 14
E 12
	  count++;
	}

D 12
    /* tell user we are done */
E 12
    sprintf(s, "%d squares pasted", count);
    place_editor_message(s);
  }
D 12
  
E 12
}



I 12
erase_area(roomnum, x, y, soft)
E 12
/* once the mark has been previously set, you may use this function to erase
   all the objects between the mark and the given lower right corner, if
   soft is TRUE, then only masked characters (ie. walls) in the squares
   will be erased */
D 12

erase_area(roomnum, x, y, soft)
E 12
int roomnum, x, y, soft;
{
D 12
  int	i, j, k, count = 0;
E 12
I 12
  int	i, j, count = 0;
E 12
  char	s[80];

  if (!is_mark_error(roomnum, x, y)) {

    /* clear the area and draw it again if nessessary */
    for (i=paste.x; i<=x; i++)
      for (j=paste.y; j<=y; j++) {
        if (soft) clear_masked_in_square(i, j);
	else clear_square(i, j);

	/* draw the spot if it is on the screen */
D 14
	if (roomnum == current_room) draw_room_square(i, j, FALSE);
E 14
I 14
	if (roomnum == current_room) draw_room_square_check_pile(i, j, FALSE);
E 14

	count++;
      }

    /* report how much was cleared */
    sprintf(s, "%d squares cleared", count);
    place_editor_message(s);

    /* take away the mark */
    paste.is_set = FALSE;
  }
D 12
	  
E 12
}



D 12

E 12
I 12
D 15
fill_area(roomnum, x, y, objtype)
E 15
I 15
fill_area(roomnum, x, y, square)
E 15
E 12
/* this command fills the area defined by mark and given lower
D 15
   right corner with the given object type (it adds the object to
   the square */
D 12

fill_area(roomnum, x, y, objtype)
E 12
int roomnum, x, y, objtype;
E 15
I 15
   right corner with copies of the objects on the given square. */
int roomnum, x, y;
SquareRec *square;
E 15
{
  int i, j, count = 0;
  char s[80];

  if (!is_mark_error(roomnum, x, y)) {

    /* place this object in each square */
    for (i=paste.x; i<=x; i++)
      for (j=paste.y; j<=y; j++) {
D 12
          if (info[(uc)objtype]->recorded) put_down_recorded_obj(i, j, objtype);
	  else put_down_mapped_obj(i, j, objtype);

E 12
I 12
D 15
	  put_down_object(i, j, objtype);
E 15
I 15
	  apply_square_using_mode(&mapstats, roomnum, i, j,
				  square, editor_mode);
E 15
E 12
D 14
	  if (current_room == roomnum) draw_room_square(i, j, FALSE);
E 14
I 14
	  if (current_room == roomnum)
	    draw_room_square_check_pile(i, j, FALSE);
E 14
	  count++;
      }

    sprintf(s, "%d squares filled", count);
    place_editor_message(s);

    /* unset the mark */
    paste.is_set = FALSE;
  }
}



I 12
warp_mouse_through_exit(exit)
/* given an exit object record, find out what x and y it was pointing
   toward and warp the mouse to that spot in the room window */
OI *exit;
{
  int x, y;

  if (!exit) return;

  x = get_record(exit, REC_INFOX);
  y = get_record(exit, REC_INFOY);
  if (x < 0 || y < 0) return;

  /* warp mouse to given location in the room window */
  warp_mouse_in_room_win(x, y);
}



E 12
/* ============================ I N F O requests ========================= */

I 12

D 14
handle_info_request(win, x, y)
E 12
/* handle a request for information on a square, results depend on which
   window it is called from.  If there is already an info window up, this
D 12
   command puts it away, if they pressed 'e' instead of 'i', they want
   special info and the special variable should be passes to this procedure
   as TRUE */

handle_info_request(win, x, y, special)
E 12
I 12
   command puts it away. */
E 12
Window win;
D 12
int x, y, special;
E 12
I 12
int x, y;
E 14
I 14
put_down_obj_check_pile(x, y, n)
/* calls put_down_obj(), but also checks to see if changes should be made
   in pile window */
int x, y, n;
E 14
E 12
{
D 14
  int newx, newy;	/* where in RootWindow to place the info window */
  int wwide, whigh;	/* size of info window */
  Window dummy1, dummy2;
  unsigned int dummykeys;
  int dummyx, dummyy;
E 14
I 14
  put_down_obj(x, y, n);
E 14

D 14
  /* if the window is already up, then remove it */
  if (info_rec->is_up) {
    hide_help_window(info_rec);
    return;
E 14
I 14
  x = CONVERTRX(x);
  y = CONVERTRY(y);
  if (pile_content.pertinent && PILE_IN_ROOMWIN &&
      pile_content.roomnum == current_room &&
      pile_content.x == x && pile_content.y == y) {
    redraw_pile_window();
    draw_pile_box();
E 14
  }
D 14
  else {
E 14
I 14
}
E 14

D 14
    /* determine new spot for info window, find out where mouse is in
       the RootWindow and use that for the upper left corner, unless
       that would place part of it off screen, if so then pull it back so
       that the entire window will be visible */
       
    wwide = info_rec->width * char_width(info_rec->fi);
    whigh = info_rec->height * char_height(info_rec->fi);
E 14

D 14
    XQueryPointer(display, RootWindow(display, screen), &dummy1, &dummy2,
		  &newx, &newy, &dummyx, &dummyy, &dummykeys);
E 14

D 14
    if (newx + wwide > DisplayWidth(display, screen))
      newx = DisplayWidth(display, screen) - wwide - WINDOW_BORDERS * 2;
    if (newy + whigh > DisplayHeight(display, screen))
      newy = DisplayHeight(display, screen) - whigh - WINDOW_BORDERS * 2;
    
    move_help_window(info_rec, newx, newy);
  

    /* get the right information placed into the window */
    clear_help_window(info_rec);
    if (win == roomwin) {
D 12
      if (special) recorded_square_info(info_rec, current_room,
					CONVERTRX(x), CONVERTRY(y));
      else map_square_info(info_rec, current_room, CONVERTRX(x), CONVERTRY(y));
E 12
I 12
      map_square_info(info_rec, current_room, CONVERTRX(x), CONVERTRY(y));
E 12
    }
    else if (win == grabwin)
      get_object_info(info_rec, obj_in_grab_win(x, y));
    else if (win == statwin)
      get_status_info(info_rec);
    else return;

    /* now make the window appear */
    show_help_window(info_rec);
E 14
I 14
draw_room_square_check_pile(x, y, b)
/* calls draw_room_square(), but also checks to see if pile displayed in
   pile window should be updated */
int x, y, b;
{
  draw_room_square(x, y, b);
  if (pile_content.pertinent && PILE_IN_ROOMWIN &&
      pile_content.roomnum == current_room &&
      pile_content.x == x && pile_content.y == y) {
    redraw_pile_window();
    draw_pile_box();
E 14
  }
}



D 12
/* get the information about the contents of a certain square on the map */

E 12
D 14
map_square_info(rec, roomnum, x, y)
I 12
/* get the information about the contents of a certain square on the map */
E 12
Help *rec;
int  roomnum, x, y;
E 14
I 14
draw_pile_box_outline(win, x, y)
/* draws outline on screen used for showing which square is under display */
Window win;
int x, y;
E 14
{
D 12
  int i;
E 12
I 12
D 14
  OI *ptr;
  int i = 0;
E 12
  char s[200];
D 12
  unsigned char *spot;
E 12
  
D 12
  /* find out what is on this square */
  spot = whats_on_square(roomnum, x, y);

E 12
  /* first line is roomnum and location */
  sprintf(s, "  Room %d    x:%d    y:%d", roomnum, x, y);
D 12
  set_line(rec, 0, s);
E 12
I 12
  set_line(rec, i++, s);
E 14
I 14
  DrawObject(win, box_pix, box_mask, NULL, x, y);
}
E 14
E 12

D 14
  /* other lines list objects names and numbers */
D 12
  for (i=0; i < ROOM_DEPTH + 1; i++) {
    sprintf(s, "  #%3d %s", spot[i], info[(uc)spot[i]]->name);
    set_line(rec, i + 1, s);
E 12
I 12
  ptr = first_obj_here(&mapstats, roomnum, x, y);
  for (; (ptr && (i < rec->height)); ptr = ptr->next) {
    sprintf(s, "  #%3d %s", ptr->type, info[ptr->type]->name);
    set_line(rec, i++, s);
E 14
I 14


draw_pile_box()
/* draws the pile display box whereever it's supposed to be */
{
  if (pile_content.pertinent) {
    if (PILE_IN_ROOMWIN && pile_content.roomnum == current_room) {
      draw_pile_box_outline(roomwin, PIXELX(pile_content.x),
			    PIXELY(pile_content.y));
    }
E 14
E 12
  }
}



D 12
/* get the information about the contents of a certain recorded object
   on a particular square on the map */

recorded_square_info(rec, roomnum, x, y)
Help *rec;
int  roomnum, x, y;
{
  MemObj *objrec;
  char s[200];

  /* find out what recorded object is here */
  objrec = what_recorded_obj_here(roomnum, x, y);

  /* if there is no recorded object here, then say so, and send for
     the normal square information */
  if (objrec == NULL) {
    set_line(rec, rec->height - 1, "  no recorded object here");
    map_square_info(rec, roomnum, x, y);
  }

  /* otherwise, give full details on the recorded object */
  else {
    sprintf(s, " #%3d %s", objrec->obj.objtype,
	    info[(uc)objrec->obj.objtype]->name);
    set_line(rec, 0, s);
    sprintf(s, " detail =%4d   ext 1 =%4d", 
	    objrec->obj.detail, objrec->obj.extra[0]);
    set_line(rec, 1, s);
    sprintf(s, "      x =%4d   ext 2 =%4d", 
	    objrec->obj.infox, objrec->obj.extra[1]);
    set_line(rec, 2, s);
    sprintf(s, "      y =%4d   ext 3 =%4d",
	    objrec->obj.infoy, objrec->obj.extra[2]);
    set_line(rec, 3, s);
    sprintf(s, " zinger =%4d", objrec->obj.zinger);
    set_line(rec, 4, s);
  }
}



/* get the information about an object and place into info help window */

E 12
D 14
get_object_info(rec, objnum)
I 12
/* get the information about an object and place into info help window */
E 12
Help *rec;
int  objnum;
E 14
I 14
set_pile_pointer(map, roomnum, x, y)
MapInfo *map;
int roomnum, x, y;
E 14
{
D 14
  char s[200];
  
  /* check to make sure this is one of the defined objects */
  if (objnum >= objects) {
    set_line(rec, 0, "  undefined object");
    return;
E 14
I 14
  SquareRec *square = get_square(map, roomnum, x, y);

  if (pile_content.pertinent && PILE_IN_ROOMWIN) {
    /* erase previous pile display box */
    draw_room_square(pile_content.x, pile_content.y, FALSE);
E 14
  }

D 14
  /* first line is number and name of object */
D 12
  sprintf(s, "  #%3d %s", objnum, info[(uc)objnum]->name);
E 12
I 12
  sprintf(s, "  #%3d %s", objnum, info[objnum]->name);
E 12
  set_line(rec, 0, s);
E 14
I 14
  if (square) {
    pile_content.pertinent = TRUE;
    pile_content.map = map;
    pile_content.square = square;
    pile_content.roomnum = roomnum;
    pile_content.x = x;
    pile_content.y = y;
  }
  else {
    pile_content.pertinent = FALSE;
  }
E 14

D 14
  /* set up the following lines to contain the flag text for the obj */
D 12
  set_lines_long_text(rec, 1, object_info_line(info[(uc)objnum]));
E 12
I 12
  set_lines_long_text(rec, 1, object_info_line(info[objnum]));
E 14
I 14
  redraw_pile_window();
  draw_pile_box();
E 14
E 12
}


D 12
/* get the information about the map in general and place in the
   info help window */
E 12

D 14
get_status_info(rec)
I 12
/* get the information about the map in general and place in the
   info help window */
E 12
Help *rec;
E 14
I 14
clear_pile_pointer()
E 14
{
D 14
  char s[80];
  
  /* In this window we will want to place some relevant information
D 3
     about the dungeon */
E 3
I 3
     about the current map */
E 3
  sprintf(s, "  %s", mapstats.name);
  set_line(rec, 0, s);
  sprintf(s, "  %d rooms in this map", mapstats.rooms);
  set_line(rec, 1, s);
I 3
  sprintf(s, "  %d team(s) supported", mapstats.teams_supported);
  set_line(rec, 2, s);
  if (mapstats.neutrals_allowed)
    sprintf(s, "  neutrals are allowed");
  else
    sprintf(s, "  neutrals not allowed");
  set_line(rec, 3, s);
E 14
I 14
  pile_content.pertinent = FALSE;
  redraw_pile_window();
}



handle_info_request(win, x, y)
/* handle a request for information on a square, results depend on which
   window it is called from.  If there is already an info window up, this
   command puts it away.  If the request is made in room window or over a
   click buffer, then that square's contents are displayed in pile window,
   otherwise, the help/info window is called up with appropriate contents. */
Window win;
int x, y;
{
  if (win == roomwin) {
    set_pile_pointer(&mapstats, current_room, CONVERTRX(x), CONVERTRY(y));
  }
  else if (win == grabwin) {
    ObjInfo *data = info[obj_in_grab_win(x, y)];
    DialogLookObjInfo(data, RootWindow(display, screen),
		      -WINDOW_BORDERS, -WINDOW_BORDERS);
  }
E 14
E 3
D 12
  
}



/* given an exit object record, find out what x and y it was pointing
   toward and warp the mouse to that spot in the room window */

warp_mouse_through_exit(exit)
MemObj *exit;
{
  /* check for illegal exit objects and do nothing if they are offensive */
  if (exit == NULL) return;
  if (exit->obj.infox < 0  ||  exit->obj.infoy < 0) return;

  /* warp mouse to given location in the room window */
  warp_mouse_in_room_win(exit->obj.infox, exit->obj.infoy);
E 12
}
E 1
