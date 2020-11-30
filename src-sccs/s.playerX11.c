h37429
s 00060/00038/02204
d D 1.30 92/09/03 16:43:17 vbo 30 29
c fixed a few segfault type problems with inventory and hand objects
e
s 00004/00007/02238
d D 1.29 92/09/03 14:59:00 vbo 29 28
c removed use of whats_on_square routine
e
s 00021/00024/02224
d D 1.28 92/09/03 14:38:18 vbo 28 27
c replace occurances of InvObj with ObjectInstances
e
s 00003/00003/02245
d D 1.27 92/09/01 18:59:17 vbo 27 26
c changed args to take_object_from_ground in inventory.c
e
s 00000/00000/02248
d D 1.26 92/08/07 01:03:21 vbo 26 25
c source copied to a separate tree for work on new map and object format
e
s 00007/00007/02241
d D 1.25 92/07/31 17:40:02 vbo 25 24
c updated the player key commands
e
s 00008/00001/02240
d D 1.24 92/01/20 16:27:48 labc-4lc 24 23
c added some of code necessary for square contents window
e
s 00055/00002/02186
d D 1.23 91/12/18 20:15:42 labc-4lc 23 22
c made message windows handle messages of multiple lines
e
s 00004/00003/02184
d D 1.22 91/12/17 19:13:34 labc-4lc 22 21
c fixed some casting problems
e
s 00003/00003/02184
d D 1.21 91/12/09 00:29:22 labc-4lc 21 20
c added alteration messages
e
s 00001/00001/02186
d D 1.20 91/12/08 02:02:51 labc-4lc 20 19
c minor change in a comment
e
s 00093/00007/02094
d D 1.19 91/12/08 01:44:35 labc-4lc 19 18
c added description code for info on objects
e
s 00051/00025/02050
d D 1.18 91/12/07 17:38:08 labc-4lc 18 17
c made error message window bigger and scrollable
e
s 00016/00015/02059
d D 1.17 91/11/25 23:03:32 labc-4lc 17 16
c fixed color problems drawing objects
e
s 00005/00003/02069
d D 1.16 91/10/16 20:06:16 labc-4lc 16 15
c changed made for DecStations
e
s 00035/00032/02037
d D 1.15 91/08/30 01:32:15 vanb 15 14
c made windows code color compatible
e
s 00001/00001/02068
d D 1.14 91/08/24 20:59:10 vanb 14 13
c shortened the names of a number of files
e
s 00002/00002/02067
d D 1.13 91/06/30 16:57:43 labc-3id 13 12
c removed all traces of old explosion 'special' types
e
s 00011/00011/02058
d D 1.12 91/05/26 22:44:51 labc-3id 12 11
c 
e
s 00023/00002/02046
d D 1.11 91/05/23 11:56:06 labc-3id 11 10
c added icon for griljor
e
s 00005/00000/02043
d D 1.10 91/05/18 15:42:47 labc-3id 10 9
c Fixed expose events for roomdraw.c
e
s 00004/00002/02039
d D 1.9 91/05/17 06:45:02 labc-3id 9 8
c fixed background bitmap so get defined after title
e
s 00011/00001/02030
d D 1.8 91/05/17 05:18:51 labc-3id 8 7
c added a call to the title screen
e
s 00034/00095/01997
d D 1.7 91/05/17 02:04:36 labc-3id 7 6
c Intermediate work on offscreen pixmap
e
s 00002/00002/02090
d D 1.6 91/05/16 00:35:00 labc-3id 6 5
c commented out explosion diety command
e
s 00048/00011/02044
d D 1.5 91/05/10 04:51:16 labc-3id 5 4
c 
e
s 00006/00003/02049
d D 1.4 91/04/27 21:26:36 labc-3id 4 3
c small change to accomodate change in inventory.c
e
s 00004/00000/02048
d D 1.3 91/04/14 19:52:19 labc-3id 3 2
c 
e
s 00002/00085/02046
d D 1.2 91/02/23 01:49:05 labc-3id 2 1
c 
e
s 02131/00000/00000
d D 1.1 91/02/16 12:55:31 labc-3id 1 0
c date and time created 91/02/16 12:55:31 by labc-3id
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

/* X routines used by player program */

#define PLAYERX11_MAIN

#include "windowsX11.h"
#include "playerX11.h"
#include "def.h"
#include "extern.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gametime.h"
#include "gamelist.h"
#include "movement.h"
#include "images.h"
#include "level.h"
D 2
#include "special.h"
E 2
#include "queued.h"
D 14
#include "circle_order.h"
E 14
I 14
#include "circle.h"
E 14
I 7
#include "roomdraw.h"
I 19
#include "message.h"
E 19
E 7

/* mouse cursor bitmaps */
#include "bit/target"
#include "bit/tmask"
#include "bit/circle"
#include "bit/cmask"
#include "bit/pen"
#include "bit/pmask"

/* other dress-up-the-screen bitmaps */
I 11
#include "bit/griljoricon"
E 11
#include "bit/movemark"
#include "bit/hit"
#include "bit/hmask"
#include "bit/friend"
#include "bit/fmask"
#include "bit/box"
#include "bit/bmask"
#include "bit/logo"
#include "bit/mainback"

/* alternate person appearance bitmaps */
#include "bit/quit"	/* appearance of a quitting person */
#include "bit/qmask"
#include "bit/stone"	/* appearance of a dead person */
#include "bit/smask"
#include "bit/invis"	/* appearance of an invisible person */
#include "bit/imask"
#include "bit/loser"	/* appearance of someone on a losing team */
#include "bit/lmask"
#include "bit/winner"	/* appearance of someone on the winning team */
#include "bit/wmask"



/* Global variables */

Window	mainwin, exitwin, itemwin, statwin, msgwin, unwin;   /* main windows */
D 24
Window	invwin, mousewin, errwin, inwin, outwin;           /* sub windows */
E 24
I 24
Window	invwin, mousewin, squarewin, errwin, inwin, outwin;  /* sub windows */
E 24
Help	*misc_rec;	/* a miscellaneous help window */


/* variables for this file's use only */
Help	*player_list = NULL;		/* window to contain player listing */
Pixmap	person_pix[MAX_GAME_SIZE];	/* window resource numbers of  */
Pixmap	person_mask[MAX_GAME_SIZE];	/* pixmaps for persons in game */
Pixmap	movemark_pix;			/* movement keyclick encourager */
Pixmap	hit_pix;			/* registered hit pixmap */
Pixmap	hit_mask;			/* mask to cover person for above */
Pixmap	friend_pix, friend_mask;	/* to place over people on your team */
Pixmap	box_pix, box_mask;		/* outlines for players */
Pixmap	appearance_pix[NUM_APPEARANCES];    /* assorted special appearances */
Pixmap	appearance_mask[NUM_APPEARANCES];   /* masks for above */
Pixmap	rotated_pix[MAX_OBJECTS][VIEWS];    /* rotated missile pixmaps */
Pixmap	rotated_mask[MAX_OBJECTS][VIEWS];   /* masks for above missiles */
D 2
Pixmap	special_pix[MAX_SPECIALS][VIEWS+1]; /* pixmaps for special missiles */
Pixmap	special_mask[MAX_SPECIALS][VIEWS+1];/* masks for above */
E 2
char	can_see[MAX_GAME_SIZE];		/* other persons are visible? */
char	old_see[MAX_GAME_SIZE];		/* old visibility array */
char	*keymap;			/* keymap that player is using */
char	inmsg[MSG_LENGTH];		/* player's input line */
int	inpnt = 0;			/* num of characters entered so far */
int	priviledged_mode = 0;		/* TRUE when in diety mode */
int	box_other_players = FALSE;	/* TRUE when we should outline */
int	windows_initialized = FALSE;	/* TRUE once main windows are set up */


/* default keymapping list */
D 19
char	default_keymap[] = { 'Q', 't', 'D', '*', '#', 'p', '!',
			     '|', '\\','o', 'C', '_', 'f', 'E',
			     'g', '?', '%', '^', 'Z', 'X',
					'q', 'w', 'e',
					'a', 's', 'd',
					'z', 'x', 'c'   };
E 19
I 19
char	default_keymap[] =
{ 
	'Q', 't', 'D', '*', '#', 'p', '!', '|', '\\','o', 'C', '_', 'f', 'E',
	'g', '?', '%', '^', 'Z', 'X', 'q', 'w', 'e', 'a', 's', 'd', 'z', 'x',
	'c', 'i'
};
E 19



/* set up the top level window for the player and map it */

player_main_window_setup()
{
I 8
  XEvent report;

E 8
  /* create the one window which will contain all others */
  if (DEBUG) printf("Open main window\n");
  mainwin = XCreateSimpleWindow(display, RootWindow(display, screen),
				PLAYER_MAIN_X, PLAYER_MAIN_Y,
				PLAYER_MAIN_W, PLAYER_MAIN_H, WINDOW_BORDERS,
				fgcolor, bgcolor);
D 11
  /* give the window a name for sake of window manager and set background */
  XStoreName(display, mainwin, PROGRAM_NAME);
E 11
I 11
  /* give the window a name for sake of window manager and set icon */
  setup_griljor_icon(mainwin);
E 11
D 9
  give_window_background_bitmap(mainwin, mainback_bits, mainback_width,
				mainback_height);
E 9

  /* load screen dressup pixmaps */
  define_dress_pixmaps();
  define_mouse_bitmap();

D 2
  /* define the special missile pixmaps */
  define_special_missile_pixmaps();

E 2
  /* load alternate person appearance pixmaps */
  define_appearance_pixmaps();

D 8
  /* map main window after starting input events */
E 8
I 8
  /* map main window after starting input events, wait for mapping */
E 8
  init_input_lines(mainwin);
  XMapWindow(display, mainwin);
I 8
  do {
    XNextEvent(display, &report);
  } while (report.type != Expose);

  /* do the title screen */
  title_screen(display, DefaultScreen(display), mainwin, mainGC,
	       bigfont, regfont, tinyfont, 
	       DefaultDepth(display, DefaultScreen(display)));
I 9

  give_window_background_bitmap(mainwin, mainback_bits,
				mainback_width,	mainback_height);
  XClearWindow(display, mainwin);
I 11
}



/* set up the icon with a name and pixmap */

setup_griljor_icon(win)
Window win;
{
  Pixmap icon_pix;
  XWMHints wmhints;

  XStoreName(display, win, PROGRAM_NAME);
  icon_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
		griljoricon_bits, griljoricon_width, griljoricon_height,
D 15
		fgcolor, bgcolor, 1);
E 15
I 15
		fgcolor, bgcolor, depth);
E 15
  wmhints.icon_pixmap = icon_pix;
  wmhints.flags = IconPixmapHint;
  XSetWMHints(display, win, &wmhints);
  /* don't free the pixmap, it has to stick around to be the icon */
E 11
E 9
E 8
}



/* set up the windows needed by the player, most of this stuff is only to
   be done once.  The global variable 'windows_initialized' will see that
   even if called again, this routine won't do certain things more than once */

player_window_setup()
{
 if (!windows_initialized) {
  /* create the window that will contain the room picture */
  if (DEBUG) printf("Open main subwindows\n");
  exitwin = XCreateSimpleWindow(display, mainwin, - WINDOW_BORDERS,
				- WINDOW_BORDERS, EXIT_WIN_W, PLAYER_MAIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);

  /* create the window containing inventory windows */
  itemwin = XCreateSimpleWindow(display, mainwin, RIGHT_WIN_X, ITEM_WIN_Y,
				RIGHT_WIN_W, ITEM_WIN_H, WINDOW_BORDERS,
				fgcolor, bgcolor);

  /* create the window containing player statistics */
  statwin = XCreateSimpleWindow(display, mainwin, RIGHT_WIN_X, STAT_WIN_Y,
				STAT_WIN_W, STAT_WIN_H, WINDOW_BORDERS,
				fgcolor, bgcolor);

  /* create the window that will contain messages */
  msgwin = XCreateSimpleWindow(display, mainwin, RIGHT_WIN_X, MSG_WIN_Y,
			       RIGHT_WIN_W, MSG_WIN_H, WINDOW_BORDERS,
			       fgcolor, bgcolor);

  /* create the misc window that will reside at bottom */
  unwin = XCreateSimpleWindow(display, mainwin,  RIGHT_WIN_X, MISC_WIN_Y,
			      RIGHT_WIN_W, MISC_WIN_H, WINDOW_BORDERS,
			      fgcolor, bgcolor);

  /* create the help/information window */
  misc_rec = make_help_window(mainwin, RIGHT_WIN_X, MSG_WIN_Y,
			      HELP_WIN_COLS, HELP_WIN_LINES, tinyfont);
  load_help_window_contents(misc_rec, PLAYER_HELP_FILE);

  /* set up the subwindows for all of these windows */
  if (DEBUG) printf("Open subwindow inferiors\n");
  player_subwindow_setup();

  /* set up the various cursors for the windows */
  if (DEBUG) printf("Define window cursors\n");
  player_cursor_setup();
 }

  /* the following is redone every time, because each game may contain
     a different set of bitmaps of objects, missiles, etc. */

  /* load rotated versions of directional missiles */
  define_rotated_pixmaps();
}



/* create a background pixmap for use in a window from bitmap data */

give_window_background_bitmap(win, bits, width, height)
Window win;
char *bits;
int width, height;
{
  Pixmap pix;

  pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
D 15
			bits, width, height, fgcolor, bgcolor, 1);
E 15
I 15
			bits, width, height, fgcolor, bgcolor, depth);
E 15
  XSetWindowBackgroundPixmap(display, win, pix);
  XFreePixmap(display, pix);
}


/* create a border pixmap for use around a window from bitmap data */

give_window_border_bitmap(win, bits, width, height)
Window win;
char *bits;
int width, height;
{
  Pixmap pix;

  pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
D 15
			bits, width, height, fgcolor, bgcolor, 1);
E 15
I 15
			bits, width, height, fgcolor, bgcolor, depth);
E 15
  XSetWindowBorderPixmap(display, win, pix);
  XFreePixmap(display, pix);
}



/* set up the various sub windows for each of the windows created above.
   these things are only to be done once. */

player_subwindow_setup()
{
  /* set up room window inside exit window */
  setup_room_window();
  
  /* item window */
  setup_item_windows();

  /* message window */
  setup_message_windows();
}



/* set up the room window so that it is inside the exit window, no border */

setup_room_window()
{
  /* create the room window inside the exit window, no border, we are going
     to want a big blank area around the room so that players may easily
     exit off of the edge of the room */

  roomwin = XCreateSimpleWindow(display, exitwin, BITMAP_WIDTH, BITMAP_HEIGHT,
				ROOM_WIDTH * BITMAP_WIDTH,
				ROOM_HEIGHT * BITMAP_HEIGHT, 0,
				fgcolor, bgcolor);
}


/* set up the various subwindows for item window */

setup_item_windows()
{
  /* make a window where picture of mouse may reside */
  mousewin = XCreateSimpleWindow(display, itemwin, MOUSE_WIN_X, MOUSE_WIN_Y,
				 mouse_width, mouse_height, 0,
				 fgcolor, bgcolor);

  /* make the window to contain player's inventory */
  invwin = XCreateSimpleWindow(display, itemwin, INV_WIN_X, INV_WIN_Y,
			       INV_WIN_W, INV_WIN_H, WINDOW_BORDERS,
			       fgcolor, bgcolor);
I 24

  /* make room for a display of the items on a particular square */
  squarewin = XCreateSimpleWindow(display, itemwin, SQUARE_WIN_X,
				  SQUARE_WIN_Y, RIGHT_WIN_W, SQUARE_WIN_H,
				  WINDOW_BORDERS, fgcolor, bgcolor);
E 24
}



/* set up the various message windows */

setup_message_windows()
{
  /* set up a window for error messages and quick info output */
  errwin = XCreateSimpleWindow(display, msgwin, GEN_WIN_X, ERR_WIN_Y,
D 18
			       RIGHT_WIN_W, GEN_WIN_H, WINDOW_BORDERS,
E 18
I 18
			       RIGHT_WIN_W, ERR_WIN_H, WINDOW_BORDERS,
E 18
			       fgcolor, bgcolor);

  /* set up the window for accepting message input from the player */
  inwin = XCreateSimpleWindow(display, msgwin, GEN_WIN_X, IN_WIN_Y,
D 18
			      RIGHT_WIN_W, GEN_WIN_H, WINDOW_BORDERS,
E 18
I 18
			      RIGHT_WIN_W, IN_WIN_H, WINDOW_BORDERS,
E 18
			      fgcolor, bgcolor);

  /* set up the big message scroll window */
  outwin = XCreateSimpleWindow(display, msgwin, GEN_WIN_X, OUT_WIN_Y,
			       RIGHT_WIN_W, OUT_WIN_H, WINDOW_BORDERS,
			       fgcolor, bgcolor);
}



/* startup the input lines for these player windows */

startup_player_input()
{
  input_and_catch_mouse_entering(mainwin);
  init_input_lines(invwin);
  init_input_lines(mousewin);
I 24
  init_input_lines(squarewin);
E 24
  init_input_lines(statwin);
  init_input_lines(unwin);
  init_input_lines(misc_rec->win);
}



/* map the windows created for the player */

map_player_windows()
{
  /* map the sub windows of the various "sub main" windows */
  XMapSubwindows(display, exitwin);
  XMapSubwindows(display, itemwin);
  XMapSubwindows(display, msgwin);
  
  /* map the major windows, but not help windows under main window */
  XMapWindow(display, exitwin);
  XMapWindow(display, itemwin);
  XMapWindow(display, statwin);
  XMapWindow(display, msgwin);
  XMapWindow(display, unwin);

  /* we hold off on some window input openers because they will be drawn
     by the routine that moves player to his first room */
  init_input_lines(roomwin);
  init_input_lines(exitwin);  
  init_input_lines(errwin);
  init_input_lines(inwin);  
  init_input_lines(outwin);
  init_input_lines(msgwin);
}



/* This routine undoes many of the things the setup routines above do.
   This routine is used when leaving a game to return to the game
   selection arena.  All the game playing windows have to be put away. */

close_player_windows()
{
  close_input_lines(roomwin);
  close_input_lines(exitwin);  
  close_input_lines(errwin);
  close_input_lines(inwin);  
  close_input_lines(outwin);
  close_input_lines(msgwin);
  close_input_lines(invwin);
  close_input_lines(mousewin);
I 24
  close_input_lines(squarewin);
E 24
  close_input_lines(statwin);
  close_input_lines(unwin);
  close_input_lines(misc_rec->win);
  hide_help_window(misc_rec);
  shut_down_player_list();
  XUnmapWindow(display, exitwin);
  XUnmapWindow(display, itemwin);
  XUnmapWindow(display, statwin);
  XUnmapWindow(display, msgwin);
  XUnmapWindow(display, unwin);
}



/* set up certain variables we need to know about */

setup_player_variables()
{
  register int i;

  current_room = gameperson[0]->room;
  
  /* set keymap to be the default */
  keymap = default_keymap;

  /* initialize visibility arrays */
  for (i=0; i<MAX_GAME_SIZE; i++) {
    can_see[i] = FALSE;
    old_see[i] = FALSE;
  }

  /* start out player's health and magic points at maximum,
     and initialize all other things that get done before life starts anew */
  for (i=0; i<num_persons; i++) dead_person_ritual(i);
}



/* define person bitmaps for the array of persons we have earlier defined,
   that should now contain all the masks and all the bitmaps of persons
   in the game */

define_all_person_pixmaps()
{
  int i;

  for (i=0; i<persons_in_game; i++) {
    person_pix[i] = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), gameperson[i]->bitmap,
D 15
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 15
I 15
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, depth);
E 15
    person_mask[i] = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), gameperson[i]->mask,
D 15
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 15
I 15
D 17
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, depth);
E 17
I 17
			BITMAP_WIDTH, BITMAP_HEIGHT, maskfg, maskbg, 1);
E 17
E 15
  }
  for (i=persons_in_game; i<MAX_GAME_SIZE; i++) {
    person_pix[i] = 0;
    person_mask[i] = 0;
  }
}


/* undo the work of the above procedure */

free_person_pixmaps()
{
  register int i;

  for (i=0; i<persons_in_game; i++) {
    XFreePixmap(display, person_pix[i]);
    person_pix[i] = NULL;
    XFreePixmap(display, person_mask[i]);	
    person_mask[i] = NULL;
  }
}



/* update the pixmaps for person in array element num */

update_pixmaps(num)
int num;
{
  /* check for need to free old one */
  if (DEBUG)
    printf("Updating pixmaps for person id %d\n", gameperson[num]->id);
  if (person_pix[num]) XFreePixmap(display, person_pix[num]);
  if (person_mask[num]) XFreePixmap(display, person_mask[num]);

  /* assign new ones */
  person_pix[num] = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), gameperson[num]->bitmap,
D 15
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 15
I 15
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, depth);
E 15
  person_mask[num] = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), gameperson[num]->mask,
D 15
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 15
I 15
D 17
			BITMAP_WIDTH, BITMAP_HEIGHT, fgcolor, bgcolor, depth);
E 17
I 17
			BITMAP_WIDTH, BITMAP_HEIGHT, maskfg, maskbg, 1);
E 17
E 15

  /* attempt to redraw this person with his new pixmaps */
  redraw_player_room_square(gameperson[num]->room, gameperson[num]->x,
			    gameperson[num]->y);
}



/* Delete a person from the pixmap array when they leave the game.  This
   routine moves all the pixmap id numbers after this one up into the
   space left by the deleted one.  This routine is called by the
   delete_game_person procedure, do not game persons_in_game from
   inside this procedure because delete_game_person isn't done with it. */

delete_person_pixmap(num)
int num;
{
  register int i;

  /* free the pixmap and mask currently defined for that person */
  XFreePixmap(display, person_pix[num]);
  XFreePixmap(display, person_mask[num]);
  
  /* loop through and copy all after this one up one space */
  if (DEBUG) printf("Deleting person %d pixmap\n", num);
  for (i=(num+1); i<persons_in_game; i++) {
    person_pix[i-1] = person_pix[i];
    person_mask[i-1] = person_mask[i];
  }

  /* zero out the leftover at the end */
  person_pix[persons_in_game-1] = 0;
  person_mask[persons_in_game-1] = 0;
}



/* set up cursors for windows */

player_cursor_setup()
{
  define_target_cursor(mainwin);
  define_target_cursor(exitwin);
  define_thin_arrow_cursor(itemwin);
D 19
  define_pen_cursor(msgwin);
E 19
I 19
  define_circle_cursor(msgwin);
  define_pen_cursor(inwin);
  define_pen_cursor(outwin);
E 19
  define_circle_cursor(statwin);
  define_circle_cursor(unwin);
}



/* define the target mouse cursor for a window */

define_target_cursor(win)
Window win;
{
  /* create the cursor in the window system and define it for window */
  make_cursor_from_bitmap_data(display, win, target_width, target_height,
			       target_bits, tmask_bits, target_x_hot,
			       target_y_hot, fgcolor, bgcolor);
}



/* define the circle mouse cursor for a window */

define_circle_cursor(win)
Window win;
{
  Cursor cursor;

  /* create the cursor in the window system */
  make_cursor_from_bitmap_data(display, win, circle_width, circle_height,
			       circle_bits, cmask_bits,
			       circle_x_hot, circle_y_hot, fgcolor, bgcolor);
}



/* define the circle mouse cursor for a window */

define_pen_cursor(win)
Window win;
{
  Cursor cursor;

  /* create the cursor in the window system */
  make_cursor_from_bitmap_data(display, win, pen_width, pen_height,
			       pen_bits, pmask_bits,
			       pen_x_hot, pen_y_hot, fgcolor, bgcolor);
}




/* define the pixmap used for encouraging use of right mouse button
   for movement.  This pixmap will be placed in the right mouse button box.
   Define pixmaps for other things as well. */

define_dress_pixmaps()
{
  /* store the movemark pixmap */ 
  movemark_pix =  XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), movemark_bits,
D 15
			movemark_width, movemark_height, fgcolor, bgcolor, 1);
E 15
I 15
			movemark_width, movemark_height, fgcolor, bgcolor,
			depth);
E 15

  /* define the registered hit pixmap and mask */
  hit_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
D 15
			hit_bits, hit_width, hit_height, fgcolor, bgcolor, 1);
E 15
I 15
			hit_bits, hit_width, hit_height, fgcolor, bgcolor,
			depth);
E 15
  hit_mask = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			hmask_bits, hmask_width, hmask_height,
D 15
			fgcolor, bgcolor, 1);
E 15
I 15
D 17
			fgcolor, bgcolor, depth);
E 17
I 17
			maskfg, maskbg, 1);
E 17
E 15

  /* define the player on own team masking pixmaps */
  friend_pix = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), friend_bits,
D 15
			friend_width, friend_height, fgcolor, bgcolor, 1);
E 15
I 15
			friend_width, friend_height, fgcolor, bgcolor, depth);
E 15
  friend_mask = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen), fmask_bits,
D 15
			fmask_width, fmask_height, fgcolor, bgcolor, 1);
E 15
I 15
D 17
			fmask_width, fmask_height, fgcolor, bgcolor, depth);
E 17
I 17
			fmask_width, fmask_height, maskfg, maskbg, 1);
E 17
E 15

  /* define the player outlining masking pixmaps */
  box_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
D 15
			box_bits, box_width, box_height, fgcolor, bgcolor, 1);
E 15
I 15
			box_bits, box_width, box_height, fgcolor, bgcolor,
			depth);
E 15
  box_mask = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			bmask_bits, bmask_width, bmask_height,
D 15
			fgcolor, bgcolor, 1);
E 15
I 15
D 17
			fgcolor, bgcolor, depth);
E 17
I 17
			maskfg, maskbg, 1);
E 17
E 15
}



/* define the pixmaps used for drawing quitting and dying persons */

define_appearance_pixmaps()
{
  /* load in quitter pixmap */
  appearance_pix[APPEAR_QUITTING] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      quit_bits, quit_width, quit_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
			      fgcolor, bgcolor, depth);
E 15
  appearance_mask[APPEAR_QUITTING] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      qmask_bits, qmask_width, qmask_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
D 17
			      fgcolor, bgcolor, depth);
E 17
I 17
			      maskfg, maskbg, 1);
E 17
E 15

  /* load in the dying person pixmap */
  appearance_pix[APPEAR_DYING] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      stone_bits, stone_width, stone_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
			      fgcolor, bgcolor, depth);
E 15
  appearance_mask[APPEAR_DYING] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      smask_bits, smask_width, smask_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
D 17
			      fgcolor, bgcolor, depth);
E 17
I 17
			      maskfg, maskbg, 1);
E 17
E 15

  /* load in the invisible person pixmap */
  appearance_pix[APPEAR_INVISIBLE] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      invis_bits, invis_width, invis_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
			      fgcolor, bgcolor, depth);
E 15
  appearance_mask[APPEAR_INVISIBLE] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      imask_bits, imask_width, imask_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
D 17
			      fgcolor, bgcolor, depth);
E 17
I 17
			      maskfg, maskbg, 1);
E 17
E 15

  /* load in the losing team person pixmap */
  appearance_pix[APPEAR_LOST] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      loser_bits, loser_width, loser_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
			      fgcolor, bgcolor, depth);
E 15
  appearance_mask[APPEAR_LOST] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      lmask_bits, lmask_width, lmask_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
D 17
			      fgcolor, bgcolor, depth);
E 17
I 17
			      maskfg, maskbg, 1);
E 17
E 15

  /* load in the winning team person pixmap */
  appearance_pix[APPEAR_WON] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      winner_bits, winner_width, winner_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
			      fgcolor, bgcolor, depth);
E 15
  appearance_mask[APPEAR_WON] = XCreatePixmapFromBitmapData(display,
			      RootWindow(display, screen),
			      wmask_bits, wmask_width, wmask_height,
D 15
			      fgcolor, bgcolor, 1);
E 15
I 15
D 17
			      fgcolor, bgcolor, depth);
E 17
I 17
			      maskfg, maskbg, 1);
E 17
E 15
}



/* define the rotated view pixmaps of directional missiles */

define_rotated_pixmaps()
{
  register int i, j;
  char *first, *second, *third;
I 16
  unsigned long color;
E 16

  /* make pixmaps for the missile objects */
  for (i=0; i<objects; i++)
    if (info[(uc)i]->directional) {
I 16
      color = ObjectPixelValue(i);
E 16
D 15
      first = rotate(BITMAP_WIDTH, BITMAP_HEIGHT, info[(uc)i]->bitmap, 1);
E 15
I 15
      first = rotate(BITMAP_WIDTH, BITMAP_HEIGHT, info[(uc)i]->bitmap, depth);
E 15
      second = vflip(BITMAP_WIDTH, BITMAP_HEIGHT, info[(uc)i]->bitmap);
      third = hflip(BITMAP_WIDTH, BITMAP_HEIGHT, first);
      rotated_pix[i][0] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  first, BITMAP_WIDTH, BITMAP_HEIGHT,
D 15
				  fgcolor, bgcolor, 1);
E 15
I 15
D 16
				  fgcolor, bgcolor, depth);
E 16
I 16
				  color, bgcolor, depth);
E 16
E 15
      rotated_pix[i][1] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  second, BITMAP_WIDTH, BITMAP_HEIGHT,
D 15
				  fgcolor, bgcolor, 1);
E 15
I 15
D 16
				  fgcolor, bgcolor, depth);
E 16
I 16
				  color, bgcolor, depth);
E 16
E 15
      rotated_pix[i][2] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  third, BITMAP_WIDTH, BITMAP_HEIGHT,
D 15
				  fgcolor, bgcolor, 1);
E 15
I 15
D 16
				  fgcolor, bgcolor, depth);
E 16
I 16
				  color, bgcolor, depth);
E 16
E 15
      free(first);	free(second);		free(third);
    }
    else for (j=0; j<VIEWS; j++) rotated_pix[i][j] = (Pixmap) NULL;

  /* make pixmaps for the missile masks */
I 17
  /* all masks are depth one for use as a clip mask */
E 17
  for (i=0; i<objects; i++)
    if (info[(uc)i]->directional && info[(uc)i]->masked) {
D 15
      first = rotate(BITMAP_WIDTH, BITMAP_HEIGHT, info[(uc)i]->mask, 1);
E 15
I 15
      first = rotate(BITMAP_WIDTH, BITMAP_HEIGHT, info[(uc)i]->mask, depth);
E 15
      second = vflip(BITMAP_WIDTH, BITMAP_HEIGHT, info[(uc)i]->mask);
      third = hflip(BITMAP_WIDTH, BITMAP_HEIGHT, first);
      rotated_mask[i][0] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  first, BITMAP_WIDTH, BITMAP_HEIGHT,
D 15
				  fgcolor, bgcolor, 1);
E 15
I 15
D 17
				  fgcolor, bgcolor, depth);
E 17
I 17
				  maskfg, maskbg, 1);
E 17
E 15
      rotated_mask[i][1] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  second, BITMAP_WIDTH, BITMAP_HEIGHT,
D 15
				  fgcolor, bgcolor, 1);
E 15
I 15
D 17
				  fgcolor, bgcolor, depth);
E 17
I 17
				  maskfg, maskbg, 1);
E 17
E 15
      rotated_mask[i][2] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  third, BITMAP_WIDTH, BITMAP_HEIGHT,
D 15
				  fgcolor, bgcolor, 1);
E 15
I 15
D 17
				  fgcolor, bgcolor, depth);
E 17
I 17
				  maskfg, maskbg, 1);
E 17
E 15
      free(first);	free(second);		free(third);
    }
    else for (j=0; j<VIEWS; j++) rotated_mask[i][j] = (Pixmap) NULL;
}



free_rotated_pixmaps()
{
  register int i, j;

  for (i=0; i<objects; i++)
    if (info[(uc)i]->directional)
      for (j=0; j<VIEWS; j++) {
        if (rotated_pix[i][j])
	  XFreePixmap(display, rotated_pix[i][j]);
	if (info[(uc)i]->masked && rotated_mask[i][j])
	  XFreePixmap(display, rotated_mask[i][j]);
      }
}



D 2
/* define the (possibly) rotated view pixmaps of special missiles */

define_special_missile_pixmaps()
{
  register int i, j;
  char *first, *second, *third;

  /* make pixmaps for the missile objects */
  for (i=0; i<num_of_smissiles; i++) {
    /* store the original bitmap */
    special_pix[i][0] = XCreatePixmapFromBitmapData(display,
				RootWindow(display, screen),
				smissile[i]->bitmap,
				BITMAP_WIDTH, BITMAP_HEIGHT,
				fgcolor, bgcolor, 1);
    
    /* store the directional ones if necessary */
    if (smissile[i]->directional) {
      first = rotate(BITMAP_WIDTH, BITMAP_HEIGHT, smissile[i]->bitmap, 1);
      second = vflip(BITMAP_WIDTH, BITMAP_HEIGHT, smissile[i]->bitmap);
      third = hflip(BITMAP_WIDTH, BITMAP_HEIGHT, first);
      special_pix[i][1] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  first, BITMAP_WIDTH, BITMAP_HEIGHT,
				  fgcolor, bgcolor, 1);
      special_pix[i][2] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  second, BITMAP_WIDTH, BITMAP_HEIGHT,
				  fgcolor, bgcolor, 1);
      special_pix[i][3] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  third, BITMAP_WIDTH, BITMAP_HEIGHT, 
				  fgcolor, bgcolor, 1);
      free(first);	free(second);		free(third);
    }
  }

  /* make pixmaps for the missile masks */
  for (i=0; i<num_of_smissiles; i++) {

    /* store the original mask */
    special_mask[i][0] = XCreatePixmapFromBitmapData(display,
				RootWindow(display, screen),
				smissile[i]->mask,
				BITMAP_WIDTH, BITMAP_HEIGHT,
				fgcolor, bgcolor, 1);
    
    /* store the directional ones if necessary */
    if (smissile[i]->directional) {
      first = rotate(BITMAP_WIDTH, BITMAP_HEIGHT, smissile[i]->mask, 1);
      second = vflip(BITMAP_WIDTH, BITMAP_HEIGHT, smissile[i]->mask);
      third = hflip(BITMAP_WIDTH, BITMAP_HEIGHT, first);
      special_mask[i][1] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  first, BITMAP_WIDTH, BITMAP_HEIGHT,
				  fgcolor, bgcolor, 1);
      special_mask[i][2] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  second, BITMAP_WIDTH, BITMAP_HEIGHT,
				  fgcolor, bgcolor, 1);
      special_mask[i][3] = XCreatePixmapFromBitmapData(display,
				  RootWindow(display, screen),
				  third, BITMAP_WIDTH, BITMAP_HEIGHT,
				  fgcolor, bgcolor, 1);
      free(first);	free(second);		free(third);
    }
  }
}


E 2
/* check the reverse mode, if it is set in player option record then
   set the display flag */

check_reverse_mode(player)
PlayerInfo *player;
{
  reversed_screen = player->reversed;
}



/* ============================== I N P U T ============================= */


/* get the player's commands and execute them faithfully as long as the
   player wishes to play, return 1 when a quit command is given.  When
   in DYING mode, only window events are handled, no player input is taken */

int player_get_input()
{
  int   i, x, y, detail, quit = FALSE;
  char  c;
  Window happenwin;
  
  i = get_input(mainwin, &c, &x, &y, &detail, &happenwin);

  /* if there was actually some input, check to see whether that would
     mean aborting a quit mode */
  if (i != NOTHING  && what_mode == QUITTING) abort_player_quit();

  /* set io received flags if io was received here */
  if (i != NOTHING) IO_RECEIVED();

  switch (i) {
      case NOTHING:	break;
      case KEYBOARD:	if (what_mode != DYING && !IS_GAME_ENDING)
			  quit = handle_player_keypress(c, detail, x, y,
							happenwin);
			break;
      case EXPOSED:	redraw_a_player_window(happenwin);
			break;
      case MOUSE:	if (what_mode != DYING && !IS_GAME_ENDING)
			  handle_player_mouse(x, y, c, detail, happenwin);
			break;
      case ENTER:	if (PLAYERWIN_AUTORAISE)
			  XRaiseWindow(display, mainwin);
			break;
      default:		break;
  }

  return quit;
}



/* handle key press and return TRUE if we should quit, otherwise return 0 */

handle_player_keypress(c, detail, x, y, what_win)
char c;
int detail, x, y;
Window what_win;
{
D 18
  if (what_win == inwin || what_win == errwin ||
      what_win == outwin || what_win == msgwin)
E 18
I 18
  if (what_win == inwin || what_win == outwin)
E 18
	add_to_input_line(c);	/* player sending a message */
  else {
    if (islower(c) && (ShiftMask & detail)) c = toupper(c);

    /* 'Q' quit game */
    if (c==keymap[0]) {
      show_player_quitting();
      return TRUE;
    }

D 25
    /* 't' take object on square */
    else if (c==keymap[1]) pickup_object(0);
E 25
I 25
    /* 't' keymap entry free */
    /* else if (c==keymap[1]) pickup_object(0); */
E 25

D 25
    /* 'D' drop object from inventory */
    else if (c==keymap[2]) {
E 25
I 25
    /* 'D' keymap entry free */
    /* else if (c==keymap[2]) {
E 25
      if (what_win == invwin) inventory_drop(0, x, y);
D 25
    }
E 25
I 25
    } */
E 25

    /* '*' die instantly */
    else if (c==keymap[3] && priviledged_mode) person[0]->health = 0;

    /* '#' drop everything */
    else if (c==keymap[4]) disperse_belongings(0);

    /* 'p' list players in game */
    else if (c==keymap[5]) call_for_player_list();

    /* '!' give game over flag */
    else if (c==keymap[6] && priviledged_mode)
      notify_of_game_over(gameperson[0]->id, gameperson[0]->team);

    /* '|' give myself some experience points */
    else if (c==keymap[7] && priviledged_mode)
      alter_experience(0, 300, "You have given yourself 300 points");

    /* '\' take away points */
    else if (c==keymap[8] && priviledged_mode)
      alter_experience(0, -300, "You have taken away 300 of your points");

    /* 'o' toggle player outlining */
    else if (c==keymap[9]) {
      box_other_players = !box_other_players;
      move_in_place(0);
    }

    /* 'C' dump variables from the core */
D 25
    else if (c==keymap[10]) dump_player_variables();
E 25
I 25
    else if (c==keymap[10] && priviledged_mode) dump_player_variables();
E 25

    /* toggle priviledged mode */
    else if (c==keymap[11]) priviledged_mode = (!priviledged_mode);

    /* 'f' send divine fist all around the room */
    else if (c==keymap[12] && priviledged_mode) operation_big_fist(0);

D 13
    /* 'E' send explosion to a square */
E 13
I 13
D 25
    /* 'E' send explosion to a square [no longer available] */
E 25
I 25
    /* 'E' keymap entry free */
E 25
E 13
D 6
    else if (c==keymap[13] && priviledged_mode)
E 6
I 6
    /* else if (c==keymap[13] && priviledged_mode)
E 6
      explode_at_square(gameperson[0]->id, gameperson[0]->room,
D 6
			CONVERTRX(x), CONVERTRY(y), 2, 0);
E 6
I 6
D 13
			CONVERTRX(x), CONVERTRY(y), 2, 0); */
E 13
I 13
			CONVERTRX(x), CONVERTRY(y), 2, 0, 0, 0, 0); */
E 13
E 6

    /* 'g' print game persons */
    else if (c==keymap[14] && priviledged_mode) print_all_game_persons();

    /* '?' bring up/put down help window */
    else if (c==keymap[15]) toggle_a_player_help_window(misc_rec);

    /* '%' teleport self to random location */
    else if (c==keymap[16] && priviledged_mode) select_person_place(0);

    /* '^' reset player stats */
    else if (c==keymap[17]) reset_player_stats(0);

    /* 'Z' drop object in left hand */
    else if (c==keymap[18]) hand_drop(0, 0);

    /* 'X' drop object in right hand */
    else if (c==keymap[19]) hand_drop(0, 1);

    /* 'q' movement key */
    else if (c==keymap[20]) directional_move(0, 0);

    /* 'w' movement key */
    else if (c==keymap[21]) directional_move(0, 1);

    /* 'e' movement key */
    else if (c==keymap[22]) directional_move(0, 2);

    /* 'a' movement key */
    else if (c==keymap[23]) directional_move(0, 3);

    /* 's' pickup key */
    else if (c==keymap[24]) pickup_object(0);

    /* 'd' movement key */
    else if (c==keymap[25]) directional_move(0, 5);

    /* 'z' movement key */
    else if (c==keymap[26]) directional_move(0, 6);

    /* 'x' movement key */
    else if (c==keymap[27]) directional_move(0, 7);

    /* 'c' movement key */
    else if (c==keymap[28]) directional_move(0, 8);
I 19

D 20
    /* ' ' examine key */
E 20
I 20
    /* 'i' examine key */
E 20
    else if (c==keymap[29]) examine_current(what_win, x, y);
E 19
  }
  
  return FALSE;
}




/* figure out what to do with a mouse button press */

handle_player_mouse(x, y, button, detail, what_window)
int x, y;
char button;
int detail;
Window what_window;
{
  int a;
  
  if (what_window == roomwin) {
    if (!(detail & ShiftMask)) {
      switch (button) {
        /* use an object in one of your hands */
        case Button1:	use_object(0, 0, CONVERTRX(x), CONVERTRY(y));
			break;
        case Button2:	use_object(0, 1, CONVERTRX(x), CONVERTRY(y));
			break;

        /* move or take object on floor below you */
        case Button3:	if (CONVERTRX(x) == gameperson[0]->x &&
			    CONVERTRY(y) == gameperson[0]->y) pickup_object(0);
			else request_movement(0, CONVERTRX(x), 
					      CONVERTRY(y), FALSE);
			break;
      }
    }
    else {
      switch (button) {
        /* take objects off of floor into hands */
D 4
        case (Button1):	a = take_object_into_hand(0, 0);
E 4
I 4
        case (Button1):	a = take_object_from_ground(gameperson[0]->room,
D 27
				gameperson[0]->x, gameperson[0]->y, 0, -1, 0);
E 27
I 27
				gameperson[0]->x, gameperson[0]->y, 0, 0);
E 27
E 4
			if (a != -1) redraw_inv_square(a);
			redraw_hand_contents(0);
			break;
D 4
        case (Button2):	a = take_object_into_hand(0, 1);
E 4
I 4
        case (Button2):	a = take_object_from_ground(gameperson[0]->room,
D 27
				gameperson[0]->x, gameperson[0]->y, 0, -1, 1);
E 27
I 27
				gameperson[0]->x, gameperson[0]->y, 0, 1);
E 27
E 4
			if (a != -1) redraw_inv_square(a);
			redraw_hand_contents(1);
			break;
      }
    }
  }
  else if (what_window == exitwin)
    switch (button) {
      /* use an object towards edge of room */
      case Button1:	use_object(0, 0, CONVERTRX(x) - 1, CONVERTRY(y) - 1);
			break;
      case Button2:	use_object(0, 1, CONVERTRX(x) - 1, CONVERTRY(y) - 1);
			break;

      /* try to move out of room */
      case Button3:	request_movement(0, CONVERTRX(x) - 1,
					 CONVERTRY(y) - 1, FALSE);
			break;

      /* unknown or ignored command with cursor in exitwin */
      default:		break;
    }

  else if (what_window == invwin)
    switch (button) {
      /* switch inventory item with one of items in hands */
      case Button1:	swap_hand_and_inv(0, 0, INVXY_TO_NUM(x, y));
			redraw_inv_square(INVXY_TO_NUM(x, y));
			redraw_hand_contents(0);
			break;
      case Button2:	swap_hand_and_inv(0, 1, INVXY_TO_NUM(x, y));
			redraw_inv_square(INVXY_TO_NUM(x, y));
			redraw_hand_contents(1);
			break;

      /* drop the item pointing to */
      case Button3:	inventory_drop(0, x, y);
      default:		break;
    }
}



/* try to take an object you are standing on and update the screen */

pickup_object(num)
int num;
{
  int slot;
  
  /* take the object */
D 4
  slot = take_object_into_inventory(num);
E 4
I 4
  slot = take_object_from_ground(gameperson[num]->room, gameperson[num]->x,
D 27
				 gameperson[num]->y, 0, -1, -1);
E 27
I 27
				 gameperson[num]->y, 0, -1);
E 27
E 4

  /* if we could then redraw inventory square */
  if (slot != -1) {
    redraw_inv_square(slot);
    redraw_player_weight();
  }
}



/* try to drop an object from inventory */

inventory_drop(person_num, x, y)
int person_num, x, y;
{
  int inv_num, succeeded;
  
  /* find inventory number of item refered to */
  if (DEBUG) printf("Attempting to drop inventory item\n");
  inv_num = INVXY_TO_NUM(x, y);

  /* if it is not a valid inventory number then return */
  if (inv_num < 0  || inv_num >= INV_SIZE) return;

  /* attempt to drop the item */
  succeeded = drop_object_from_inv(person_num, inv_num);

  /* if the drop succeeded then redraw the inventory square */
  if (succeeded) {
    redraw_inv_square(inv_num);
    redraw_player_weight();
  }
}


/* try to drop the contents of a person's hand */

hand_drop(person_num, hand_num)
int person_num, hand_num;
{
  int inv_num;

  inv_num = drop_out_of_hand(person_num, hand_num);
  if (inv_num >= 0) {
    redraw_inv_square(inv_num);
    redraw_hand_contents(hand_num);
    redraw_player_weight();
  }
}



/* add a character to the input line */

add_to_input_line(c)
char c;
{
  /* if it is a newline char or carraige return then send the message off */
  if (c=='\n' || c=='\r') send_input_line();

  /* if it is a backspace or delete char then go back one */
  else if (c==127 || c==8 || c=='\b') {
    if (inpnt > 0) {
      inpnt--;
      inmsg[inpnt] = '\0';
      redraw_input_window();
    }
  }

  /* or add the character if there is room */
  else if (inpnt < MSG_LENGTH-10) {
    inmsg[inpnt] = c;
    inpnt++;
    inmsg[inpnt] = '\0';
    add_char_to_input_window(c, inpnt-1);
I 19
  }
}



D 30
examine_one_object(objnum, near)
E 30
I 30
examine_one_object(obj, near)
E 30
/* causes description of one object to appear in reporting window */
D 30
int objnum, near;
E 30
I 30
OI *obj;
int near;
E 30
{
I 30
  int objnum;
E 30
  MessageInfo *msginfo;

D 30
  if (!objnum) return;
E 30
I 30
  if (!obj) return;
  if (!(objnum = obj->type)) return;
E 30
  msginfo = create_message_info();
  player_error_out_message("--------");
  MI_USAGE_INFO(msginfo, info[objnum]->name, gameperson[0]->name,
		gameperson[0]->id, mapstats.team_name[gameperson[0]->team],
		room[gameperson[0]->room].name);
  echo_description(objnum, near, PLAYER_MESSAGE_COLUMNS, msginfo);
  player_error_out_message("--------");

  destroy_message_info(msginfo);
}



examine_at_square(roomnum, x, y, near)
/* causes a description of objects on the particular square to come up in
   the error reporting window of the player */
int roomnum, x, y;
int near;	/* TRUE when player is next to or on the square */
{
  int i;
D 29
  unsigned char *here;
E 29
I 29
  OI *o;
E 29
  MessageInfo *msginfo = create_message_info();
  if (OUT_OF_BOUNDS(roomnum, x, y)) return;

D 29
  here = whats_on_square(roomnum, x, y);
E 29
  MI_USAGE_INFO(msginfo, NULL, gameperson[0]->name, gameperson[0]->id,
		mapstats.team_name[gameperson[0]->team],
		room[roomnum].name);

  player_error_out_message("--------");

D 29
  for (i=0; i<(ROOM_DEPTH+1); i++) {
    if (here[i]) {
	MI_object_name(msginfo, info[here[i]]->name);
	echo_description(here[i], near, PLAYER_MESSAGE_COLUMNS, msginfo);
    }
E 29
I 29
  for (o = first_obj_here(&mapstats, roomnum, x, y); o; o = o->next) {
    MI_object_name(msginfo, info[o->type]->name);
    echo_description(o->type, near, PLAYER_MESSAGE_COLUMNS, msginfo);
E 29
  }

  player_error_out_message("--------");
  destroy_message_info(msginfo);
}



examine_current(win, x, y)
/* examine the contents of the window at given x and y,
   usually involves calling up the descriptions of some objects. */
Window win;
int x, y;
{
  if (win == roomwin) {
    /* examine contents of a square in our current room */
    int x1, y1, x2, y2, nearby;
    x1 = gameperson[0]->x;	y1 = gameperson[0]->y;
    x2 = CONVERTRX(x);		y2 = CONVERTRY(y);
    nearby = ((abs(x1 - x2) < 2) && (abs(y1 - y2) < 2));
    examine_at_square(gameperson[0]->room, x2, y2, nearby);
  }
  else if (win == invwin) {
    int inv_num = INVXY_TO_NUM(x, y);
    if (inv_num < 0  || inv_num >= INV_SIZE) return;
D 21
    examine_one_object(person[0]->inventory[inv_num].type, TRUE);
E 21
I 21
D 22
    examine_one_object((int)person[0]->inventory[inv_num].type, TRUE);
E 22
I 22
D 28
    examine_one_object((unsigned char) person[0]->inventory[inv_num].type,
		       TRUE);
E 28
I 28
D 30
    examine_one_object(person[0]->inventory_object[inv_num]->type, TRUE);
E 30
I 30
    examine_one_object(person[0]->inventory_object[inv_num], TRUE);
E 30
E 28
E 22
E 21
  }
  else if (win == mousewin) {
    if (x < (MOUSE_START_X + (BITMAP_WIDTH + MOUSE_SPACING)))
D 21
      examine_one_object(person[0]->hand[0].type, TRUE);
E 21
I 21
D 22
      examine_one_object((int)person[0]->hand[0].type, TRUE);
E 22
I 22
D 28
      examine_one_object((unsigned char) person[0]->hand[0].type, TRUE);
E 28
I 28
D 30
      examine_one_object(person[0]->hand_object[0]->type, TRUE);
E 30
I 30
      examine_one_object(person[0]->hand_object[0], TRUE);
E 30
E 28
E 22
E 21
    else
D 21
      examine_one_object(person[0]->hand[1].type, TRUE);
E 21
I 21
D 22
      examine_one_object((int)person[0]->hand[1].type, TRUE);
E 22
I 22
D 28
      examine_one_object((unsigned char) person[0]->hand[1].type, TRUE);
E 28
I 28
D 30
      examine_one_object(person[0]->hand_object[1]->type, TRUE);
E 30
I 30
      examine_one_object(person[0]->hand_object[1], TRUE);
E 30
E 28
E 22
E 21
E 19
  }
}



/* ============================ R E D R A W ============================ */

/* redraw the indicated window for the player */

redraw_a_player_window(win)
Window win;
{
  /* figure out which window to redraw and do it */
  
  if      (win == roomwin) redraw_player_room_window(TRUE);
  else if (win == mousewin) redraw_mouse_button_window();
  else if (win == invwin) redraw_inventory_window();
D 18
  else if (win == errwin) redraw_error_window("");
E 18
I 18
  else if (win == errwin) redraw_error_out_window();
E 18
  else if (win == exitwin) redraw_exit_window();
  else if (win == outwin) redraw_out_window();
  else if (win == inwin) redraw_input_window();
  else if (win == statwin) redraw_player_stat_window();
  else if (win == misc_rec->win) write_help_window_contents(misc_rec);
  else if (player_list) {
    /* when requests come to redraw player list window, erase it instead */
    if (win == HW_WINDOW(player_list)) shut_down_player_list();
  }
}



/* redraw the room window for the player, including all persons that are
   in the room and that are visible, exposure should be passed as TRUE
   if this redraw request comes from an XExposeEvent */

redraw_player_room_window(exposure)
int exposure;
{
  /* draw his square first */
  QDraw_priority_request(gameperson[0]->room, gameperson[0]->x,
			 gameperson[0]->y, EVERYTHING);

  /* draw the mapped version of the room */
  redraw_player_room_window_squares(exposure);

  /* draw the people into the room */
  /* redraw_persons_in_room(); */
}



/* redraw everything that is supposed to be on a certain square, the map,
   recorded objects, persons, and missiles */

redraw_player_room_square(roomnum, x, y)
int roomnum, x, y;
{
  /* if roomnum is not current_room then don't bother */
  if (roomnum != current_room) return;

  /* abort if location requested is out of bounds */
  if (x < 0 || x >= ROOM_WIDTH || y < 0 || y >= ROOM_HEIGHT) return;

  /* redraw the map on that square */
D 7
  draw_room_square(x, y, FALSE);
E 7
I 7
  DrawOnSquare(NULL,NULL,x,y,DRAWBASE);
E 7

  /* redraw any persons that might be on this square */
  redraw_persons_here(roomnum, x, y);

  /* redraw any missiles that might be on this square */
  /* NOTE: It was decided that because missiles move so quickly it is
     not worth the computation time to figure out which missiles would
     be redrawn here.  Same for explosions. */
  
  /* redraw any hits on this square */
  redraw_hit(roomnum, x, y);
}



/* redraw the persons in the current_room that are visible */

redraw_persons_in_room()
{
  register int i;

  /* look through all persons for those in your room */
  for (i=0; i<persons_in_game; i++)
    if (gameperson[i]->room == current_room) redraw_game_person(i);
}



/* redraw persons in room that have just become visible, and erase those
   that just became invisible */

redraw_newly_visible_persons()
{
  register int i;

  /* redraw yourself first to get the room right */
  redraw_game_person(0);

  /* look through all persons for those in your room */
  for (i=0; i<persons_in_game; i++) {
    if (gameperson[i]->room == current_room &&  i != 0 &&
        old_see[i] == FALSE && can_see[i] == TRUE)
		redraw_game_person(i);
    else if (gameperson[i]->room == current_room && i != 0 &&
	     old_see[i] == TRUE && can_see[i] == FALSE)
		erase_current_game_person(i);
  }
}



D 7
/* redraw any gamepersons found on the square xy in the room */
E 7
I 7
/* redraw any gamepreersons found on the square xy in the room */
E 7

redraw_persons_here(roomnum, x, y)
int roomnum, x, y;
{
  PersonList	*ptr;
  
  /* if roomnum is not current_room then don't bother */
  if (roomnum != current_room || roomnum < 0) return;


  /* go through all game persons in room, looking for those on this spot */
  for (ptr = room_persons[roomnum].next; ptr; ptr = ptr->next)
    if (ptr->person->room == roomnum && ptr->person->x == x &&
	ptr->person->y == y)
	redraw_game_person(person_array(ptr->person->id));
}




/* redraw a person in the game onto the current room map, if we are asked
   to redraw person #0 for the player in another room, then he must
   have changed rooms and we must change room display.  Pay attention to
   the appearance flag of the person we are drawing.  If a person is
   invisible, don't draw him unless he is one of ours. 
   Concerning visibility: always draw my own persons, check visibility
   array for the rest */

redraw_game_person(num)
int num;
{
  int wx, wy;

  /* look for bad game person number */
  if (num<0 || num>=persons_in_game) return;
  
  /* look for an out of bounds condition */
  if (gameperson[num]->x <  0  ||
      gameperson[num]->x >= ROOM_WIDTH ||
      gameperson[num]->y <  0  ||
      gameperson[num]->y >= ROOM_HEIGHT)  return;

  /* check for player changing to new room */
  if (has_display && num==0 && current_room != gameperson[0]->room) {
      current_room = gameperson[0]->room;
      redraw_exit_window();
      redraw_player_room_window(FALSE);	/* this draws player for us */
D 18
      redraw_error_window(" ");		/* clear last error message */
E 18
      return;
  }

  /* now if person's room is still not the current one then don't draw */
  if (gameperson[num]->room != current_room) return;  

  /* if person is invisible and is not us and we don't have all seeing power,
     then don't draw anything */
  if (gameperson[num]->appearance == APPEAR_INVISIBLE &&
      num >= num_persons)  return;

I 2
#ifndef RAYCODE
E 2
  /* if the person is not one of us and is not visible then don't draw */
  if (num >= num_persons && can_see[num] == FALSE) return;
I 2
#endif
E 2

  /* if the person is not of normal appearance then draw otherwise */
  if (gameperson[num]->appearance != APPEAR_NORMAL)
    redraw_person_appearance(num);
  else {    
    /* find proper draw spot */
D 7
    wx = PIXELX(gameperson[num]->x);
    wy = PIXELY(gameperson[num]->y);
E 7
I 7
    wx = gameperson[num]->x;
    wy =gameperson[num]->y;
E 7

    /* place the mask and bitmap onto the spot */
    if (DEBUG) printf("Drawing person (id %d)\n", gameperson[num]->id);
D 7
    XCopyArea(display, person_mask[num], roomwin, maskGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
    XCopyArea(display, person_pix[num], roomwin, drawGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
E 7
I 7
    DrawOnSquare(person_pix[num],person_mask[num],wx,wy,TEMPDRAW);
E 7

    /* if person is on my own team, and there are teams in this game,
       then draw a box or whatever around him to signify as friendly */
    if (mapstats.teams_supported > 1  &&  num != 0  &&
	gameperson[num]->team == gameperson[0]->team &&
D 7
	gameperson[0]->team != 0) {
	XCopyArea(display, friend_mask, roomwin, maskGC, 0, 0,
		  BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
	XCopyArea(display, friend_pix, roomwin, drawGC, 0, 0,
		  BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
    }
    else if (box_other_players) {
E 7
I 7
	gameperson[0]->team != 0)
      DrawOnSquare(friend_pix,friend_mask,wx,wy,TEMPDRAW);
    else if (box_other_players)
E 7
      /* put a box around all others */
D 7
      XCopyArea(display, box_mask, roomwin, maskGC, 0, 0,
		BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
      XCopyArea(display, box_pix, roomwin, drawGC, 0, 0,
		BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
    }
E 7
I 7
      DrawOnSquare(box_pix,box_mask,wx,wy,TEMPDRAW);
E 7
  }
}



/* redraw a gameperson under some other than normal appearance, we also
   draw supposed invisible types if we get to this point.  All error
   conditions should have been previously checked in redraw_game_person */

redraw_person_appearance(num)
int num;
{
  int wx, wy, type;

  /* figure out type of alternate appearance */
  type = gameperson[num]->appearance;
  if (type < 0 || type >= NUM_APPEARANCES) return;

  /* find draw spot in roomwin */
D 7
  wx = PIXELX(gameperson[num]->x);
  wy = PIXELY(gameperson[num]->y);
E 7
I 7
  wx = gameperson[num]->x;
  wy = gameperson[num]->y;
E 7

  /* place the mask and bitmap onto the spot */
  if (DEBUG) printf("Drawing person (id %d)\n", gameperson[num]->id);
D 7
  XCopyArea(display, appearance_mask[type], roomwin, maskGC, 0, 0,
	    BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
  XCopyArea(display, appearance_pix[type], roomwin, drawGC, 0, 0,
	    BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
E 7
I 7

  DrawOnSquare(appearance_pix[type],appearance_mask[type],wx,wy,TEMPDRAW);
E 7
}



/* redraw the window containing information on what is in each hand */

redraw_mouse_button_window()
{
  int wx, wy;
  
  /* put the mouse box bitmap in place */
  place_mouse_picture(mousewin, 0, 0);

  /* put movement encourager in right button box */
  wx = MOUSE_START_X + (2 * (BITMAP_WIDTH + MOUSE_SPACING));
  wy = MOUSE_START_Y;

  /* place pixmap of movement encourager on screen */
  XCopyArea(display, movemark_pix, mousewin, mainGC, 0, 0,
	    BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 

  /* redraw the contents of player's hands */
  redraw_hand_contents(0);
  redraw_hand_contents(1);
}



/* redraw the contents of the given hand (0 or 1) of current person */

redraw_hand_contents(which_hand)
int which_hand;
{
D 30
  /* the place_mouse_block procedure can figure out where it goes inside
     the mouse picture representation */
  place_mouse_block(mousewin, 0, 0, which_hand,
D 28
		    person[0]->hand[which_hand].type);
E 28
I 28
		    person[0]->hand_object[which_hand]->type);
E 30
I 30
  if (person[0]->hand_object[which_hand]) {
E 30
E 28

D 30
  /* check for opportunity to place charge counter (lower right) */
D 28
  if (info[(uc)person[0]->hand[which_hand].type]->numbered &&
      info[(uc)person[0]->hand[which_hand].type]->recorded &&
      person[0]->hand[which_hand].record)
	place_mouse_item_number(mousewin, 0, 0, which_hand, 3, 
			    person[0]->hand[which_hand].record->obj.detail);
E 28
I 28
  if (info[(uc)person[0]->hand_object[which_hand]->type]->numbered &&
      info[(uc)person[0]->hand_object[which_hand]->type]->recorded)
    place_mouse_item_number(mousewin, 0, 0, which_hand, 3, 
			    get_record(person[0]->hand_object[which_hand],
				       REC_DETAIL));
E 30
I 30
    /* the place_mouse_block procedure can figure out where it goes inside
       the mouse picture representation */
    place_mouse_block(mousewin, 0, 0, which_hand,
		      person[0]->hand_object[which_hand]->type);
E 30
E 28

D 30
  /* check for chance to place an id number (upper right) */
D 28
  if (info[(uc)person[0]->hand[which_hand].type]->id && 
      info[(uc)person[0]->hand[which_hand].type]->recorded &&
      person[0]->hand[which_hand].record)
	place_mouse_item_number(mousewin, 0, 0, which_hand, 2, 
			    person[0]->hand[which_hand].record->obj.infox);
E 28
I 28
  if (info[(uc)person[0]->hand_object[which_hand]->type]->id && 
      info[(uc)person[0]->hand_object[which_hand]->type]->recorded)
    place_mouse_item_number(mousewin, 0, 0, which_hand, 2, 
			    get_record(person[0]->hand_object[which_hand],
				       REC_INFOX));
E 30
I 30
    /* check for opportunity to place charge counter (lower right) */
    if (info[(uc)person[0]->hand_object[which_hand]->type]->numbered &&
	info[(uc)person[0]->hand_object[which_hand]->type]->recorded)
      place_mouse_item_number(mousewin, 0, 0, which_hand, 3, 
			      get_record(person[0]->hand_object[which_hand],
					 REC_DETAIL));

    /* check for chance to place an id number (upper right) */
    if (info[(uc)person[0]->hand_object[which_hand]->type]->id && 
	info[(uc)person[0]->hand_object[which_hand]->type]->recorded)
      place_mouse_item_number(mousewin, 0, 0, which_hand, 2, 
			      get_record(person[0]->hand_object[which_hand],
					 REC_INFOX));
  }
  else {
    place_mouse_block(mousewin, 0, 0, which_hand, 0);
  }
E 30
E 28
}



/* Redraw the window containing player's inventory */

redraw_inventory_window()
{
  register int i;

  /* redraw each square in turn */
  for (i=0; i<INV_SIZE; i++)  redraw_inv_square(i);
}



/* redraw one square of the inventory window */

redraw_inv_square(num)
int num;
{
  int item;

D 30
  /* which item in person's inventory */
D 28
  item = person[0]->inventory[num].type;
E 28
I 28
  item = person[0]->inventory_object[num]->type;
E 30
I 30
  /* is there an item in person's inventory at this number */
  if (person[0]->inventory_object[num]) {
    item = person[0]->inventory_object[num]->type;

    /* place object on the player's person at this inventory spot on screen */
    DrawObject(invwin, obj_pixmap[(uc)item], NULL, NULL,
	       (num % INV_WIDTH * BITMAP_WIDTH), 
	       (num / INV_WIDTH * BITMAP_HEIGHT));

    /* place possible numbers in corners of item pixmap */
    possibly_place_corner_numbers(invwin,(num % INV_WIDTH * BITMAP_WIDTH),
				  (num / INV_WIDTH * BITMAP_HEIGHT),
				  person[0]->inventory_object[num]);
  }
  else {
    item = 0;
E 30
E 28
  
D 30
  /* place object on the player's person at this inventory spot on screen */
D 17
  XCopyArea(display, obj_pixmap[(uc)item], invwin, mainGC, 0, 0,
	    BITMAP_WIDTH, BITMAP_HEIGHT, (num % INV_WIDTH * BITMAP_WIDTH), 
E 17
I 17
  DrawObject(invwin, obj_pixmap[(uc)item], NULL, NULL,
	     (num % INV_WIDTH * BITMAP_WIDTH), 
E 17
	     (num / INV_WIDTH * BITMAP_HEIGHT));
E 30
I 30
    /* place object on the player's person at this inventory spot on screen */
    DrawObject(invwin, obj_pixmap[(uc)item], NULL, NULL,
	       (num % INV_WIDTH * BITMAP_WIDTH), 
	       (num / INV_WIDTH * BITMAP_HEIGHT));
E 30

D 30
  /* place possible numbers in corners of item pixmap */
  possibly_place_corner_numbers(invwin,(num % INV_WIDTH * BITMAP_WIDTH),
				(num / INV_WIDTH * BITMAP_HEIGHT),
D 28
				&(person[0]->inventory[num]));
E 28
I 28
				person[0]->inventory_object[num]);
E 30
I 30
  }
E 30
E 28
}



/* look for a chance to place numbers about an object in the square it 
   was just drawn in.  This code also duplicated in redraw_hand_contents. */

possibly_place_corner_numbers(win, x, y, obj)
Window win;
int x, y;
D 28
InvObj *obj;
E 28
I 28
OI *obj;
E 28
{
D 30
  /* check for opportunity to place charge counter (lower right) */
D 28
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded &&
      obj->record)
	place_corner_number(win, x, y, 3, obj->record->obj.detail);
E 28
I 28
  if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
    place_corner_number(win, x, y, 3, get_record(obj, REC_DETAIL));
E 30
I 30
  if (obj) {
    /* check for opportunity to place charge counter (lower right) */
    if (info[(uc)obj->type]->numbered && info[(uc)obj->type]->recorded)
      place_corner_number(win, x, y, 3, get_record(obj, REC_DETAIL));
E 30
E 28

D 30
  /* check for chance to place an id number (upper right) */
D 28
  if (info[(uc)obj->type]->id && info[(uc)obj->type]->recorded &&
      obj->record)
	place_corner_number(win, x, y, 2, obj->record->obj.infox);
E 28
I 28
  if (info[(uc)obj->type]->id && info[(uc)obj->type]->recorded)
    place_corner_number(win, x, y, 2, get_record(obj, REC_INFOX));
E 30
I 30
    /* check for chance to place an id number (upper right) */
    if (info[(uc)obj->type]->id && info[(uc)obj->type]->recorded)
      place_corner_number(win, x, y, 2, get_record(obj, REC_INFOX));
  }
E 30
E 28
}




D 18
/* Place message in the error window in regular font.  If a null
   length message is given as the message, then redraw the last message */

redraw_error_window(line)
char *line;
{
  static char	current_msg[120] = "";

  if (strlen(line) != 0) strcpy(current_msg, line);

  /* clear window and place text line */
  XClearWindow(display, errwin);
  text_write(errwin, tinyfont, GEN_GAP, GEN_GAP, 0, 0, current_msg);
}



E 18
/* redraw the player's input window */

redraw_input_window()
{
  XClearWindow(display, inwin);
  if (inpnt == 0) inmsg[0] = '\0';
  else {
    text_write(inwin, tinyfont, GEN_GAP, GEN_GAP, 0, 0, inmsg);
    text_write(inwin, tinyfont, GEN_GAP, GEN_GAP, inpnt, 0, "_");
  }
}



/* place the room name and map name in appropriate spots in the exit
   window, this is the redraw procedure */

redraw_exit_window()
{
  /* check for invalid room number */
  if (current_room < 0) return;
  
  place_room_name(exitwin, BITMAP_WIDTH, EXIT_NAME_OFF, EXIT_WIN_W,
		 BITMAP_HEIGHT);

  place_map_name(exitwin, EXIT_WIN_W - BITMAP_WIDTH - char_width(bigfont) *
		 strlen(mapstats.name), PLAYER_MAIN_H - BITMAP_HEIGHT +
		 EXIT_NAME_OFF, EXIT_WIN_W, PLAYER_MAIN_H);
}



I 18
/* redraw the error message scroll window, messages are kept in the
   person #0 record (ie. person[0].errmsg) */

redraw_error_out_window()
{
  int i;
  
  XClearWindow(display, errwin);

  for (i=0; i<MAX_ERROR_MESSAGES; i++)
    if (person[0]->errmsg[i] != NULL)
      text_write(errwin, tinyfont, GEN_GAP, GEN_GAP, 0, i,
		 person[0]->errmsg[i]);
}



E 18
/* redraw the out message scroll window, messages are kept in the
   person #0 record (ie. person[0].msg) */

redraw_out_window()
{
  int i;
  
  XClearWindow(display, outwin);

  for (i=0; i<MAX_MESSAGES; i++)
    if (person[0]->msg[i] != NULL)
      text_write(outwin, tinyfont, GEN_GAP, GEN_GAP, 0, i, person[0]->msg[i]);
}



/* redraw status window, with his name and numbers and all */
#define STX	(5)
#define STY	(10 + BITMAP_HEIGHT)

redraw_player_stat_window()
{
  int top, x, y;
  char s[120];
  
  /* put the players picture on screen */
  XCopyArea(display, person_pix[0], statwin, mainGC, 0, 0,
	    BITMAP_WIDTH, BITMAP_HEIGHT, 5, 5); 

  /* place their name nearby */
  text_write(statwin, bigfont, 10 + BITMAP_WIDTH, 5 + BITMAP_HEIGHT -
	     char_height(bigfont), 0, 0, gameperson[0]->name);

  /* place team name */
  text_write(statwin, regfont, STX, STY, 0, 0,
	     get_team_name(gameperson[0]->team));

  /* place all the numbers into their spots */
  redraw_all_stats();
}



redraw_all_stats()
{
  redraw_player_experience();
  redraw_player_health();
  redraw_player_magic();
  redraw_player_weight();
  redraw_player_kills();
}



/* redraw player's experience point and level line */

redraw_player_experience()
{
  char s[80];

  sprintf(s, "%d Exp", person[0]->experience);
  text_write(statwin, regfont, STX, STY, 0, 1, left_justify(s, 11));
  sprintf(s, "Level %d (%s)", gameperson[0]->level, gameperson[0]->rank);
  text_write(statwin, regfont, STX, STY, 14, 1, left_justify(s, 35));
}



/* redraw a player's health points in the status window */

redraw_player_health()
{
  char s[40];

  /* figure out hit point total and maximum */
  sprintf(s, "%d/%d hp", person[0]->health, max_person_health(0));

  /* place the numbers on screen */
  text_write(statwin, regfont, STX, STY, 0, 2, left_justify(s, 13));
}



/* redraw a player's magic points in the status window */

redraw_player_magic()
{
  char s[40];

  /* figure out hit point total and maximum */
  sprintf(s, "%d/%d magic", person[0]->power, max_person_power(0));

  /* place the numbers on screen */
  text_write(statwin, regfont, STX, STY, 14, 2, left_justify(s, 20));
}



/* redraw a player's weight points in the status window */

redraw_player_weight()
{
  char s[40];

  /* figure out hit point total and maximum */
  sprintf(s, "%d/%d lbs", person_burden(0), gameperson[0]->weight_limit);

  /* place the numbers on screen */
  text_write(statwin, regfont, STX, STY, 14, 4, left_justify(s, 16));
}



/* redraw the player's kill/losses/ratio line and games won total */

redraw_player_kills()
{
  char s[80];
  double ratio;

  sprintf(s, "%d kills", gameperson[0]->kills);
  text_write(statwin, regfont, STX, STY, 0, 3, left_justify(s, 12));

  if (gameperson[0]->losses)
    ratio = (double) gameperson[0]->kills / (double) gameperson[0]->losses;
  else
    ratio = (double) gameperson[0]->kills;

  sprintf(s, "%d losses  ratio %4.2f", gameperson[0]->losses, ratio);
  text_write(statwin, regfont, STX, STY, 14, 3, left_justify(s, 30));

  sprintf(s, "%d games", gameperson[0]->games);
  text_write(statwin, regfont, STX, STY, 0, 4, left_justify(s, 13));
}



D 18
/* add a line to the message scroll window, scolling if neccessary, add
E 18
I 18
/* add a line to the error message scroll window, scrolling if neccessary,
   add the message to the person[0] record */

D 23
player_error_out_message(s)
E 23
I 23
static void player_error_out_line(s)
E 23
char *s;
{
  int place;	/* what line message was added to */
  
  /* call person add message procedure */
  place = add_person_error_message(0, s);

  /* if we don't have to scroll to place it then skip the scroll part */
  if (place >= MAX_ERROR_MESSAGES) {
    /* scroll the screen */
    scroll_window(errwin, tinyfont, GEN_GAP, GEN_GAP,
		  MSG_WIN_COLS, MAX_ERROR_MESSAGES - 1);

    /* see that it will be placed correctly */
    place = MAX_ERROR_MESSAGES - 1;
  }

  /* place the message on the screen at appropriate line */
  text_write(errwin, tinyfont, GEN_GAP, GEN_GAP, 0, place, s);
}


I 23
/* add one or more lines to the error message scroll window */
E 23

I 23
player_error_out_message(s)
char *s;
{
  int i;
  char *start, *ptr, tmp[PLAYER_MESSAGE_COLUMNS + 1];
  if (!s) return;
E 23

I 23
  start = s;
  for (i=0,ptr=start; (*ptr); i++,ptr++)
    if (*ptr == '\n') {
      if (i) strncpy(tmp, start, i);
      tmp[i] = '\0';
      player_error_out_line(tmp);
      i = -1;
      start = ptr+1;
    }

  if (i) {
    strncpy(tmp, start, i);
    tmp[i] = '\0';
     player_error_out_line(tmp);
  }
}



E 23
/* add a line to the message scroll window, scrolling if neccessary, add
E 18
   the message to the person[0] record */

D 23
player_out_message(s)
E 23
I 23
static void player_out_line(s)
E 23
char *s;
{
  int place;	/* what line message was added to */
  
  /* call person add message procedure */
  place = add_person_message(0, s);

  /* if we don't have to scroll to place it then skip the scroll part */
  if (place >= MAX_MESSAGES) {
    /* scroll the screen */
    scroll_window(outwin, tinyfont, GEN_GAP, GEN_GAP,
		  MSG_WIN_COLS, MAX_MESSAGES - 1);

    /* see that it will be placed correctly */
    place = MAX_MESSAGES - 1;
  }

  /* place the message on the screen at appropriate line */
  text_write(outwin, tinyfont, GEN_GAP, GEN_GAP, 0, place, s);
}

I 23


/* add one or more lines to the message scroll window */

player_out_message(s)
char *s;
{
  int i;
  char *start, *ptr, tmp[PLAYER_MESSAGE_COLUMNS + 1];
  if (!s) return;

  start = s;
  for (i=0,ptr=start; (*ptr); i++,ptr++)
    if (*ptr == '\n') {
      if (i) strncpy(tmp, start, i);
      tmp[i] = '\0';
      player_out_line(tmp);
      i = -1;
      start = ptr+1;
    }

  if (i) {
    strncpy(tmp, start, i);
    tmp[i] = '\0';
     player_out_line(tmp);
  }
}
E 23



/* redraw, or draw for the first time, a missile if that missile is in
   the currently displayed room.  Check for error conditions */

redraw_missile(m, roomnum)
Missile *m;
int roomnum;
{
  int wx, wy;

  /* check that missile is in this room */
  if (roomnum != current_room) return;

  /* look for an out of bounds condition */
  if (m->x <  0  ||  m->x >= ROOM_WIDTH ||
      m->y <  0  ||  m->y >= ROOM_HEIGHT)  return;

D 7
  /* find proper draw spot */
  wx = PIXELX(m->x);
  wy = PIXELY(m->y);

E 7
  /* if this is a normal object figure out which pixmap to use, possibly,
     we will need to use a directional pixmap for this kind of missile */
D 2
  if (!m->special) {
E 2
D 7
   if (m->direction == 0) { 
      /* place the normal object mask and bitmap onto the spot */
      place_obj_pixmap(roomwin, wx, wy, (uc)m->type, FALSE);
    }
D 5
    else {
      /* draw rotated pixmap */
E 5
I 5
    else 
      {
E 7
I 7
   if (m->direction == 0) 
     { 
       /* place the normal object mask and bitmap onto the spot */
       
D 12
       if (!info[(unsigned char)m->type]->masked)
	 DrawOnSquare(obj_pixmap[(unsigned char)m->type],NULL,
		      m->x,m->y,TEMPDRAW);
       else
E 12
I 12
       if (info[(unsigned char)m->type]->masked)
E 12
	 DrawOnSquare(obj_pixmap[(unsigned char)m->type],
		      obj_mask[(unsigned char)m->type],
D 12
		      m->x,m->y,TEMPDRAW);
E 12
I 12
		      m->x,m->y,MDRAW);
       else
	 DrawOnSquare(obj_pixmap[(unsigned char)m->type],NULL,
		      m->x,m->y,MDRAW);
E 12
     }
   else 
     {
E 7
D 12
	/* draw rotated pixmap */
	if (!info[(uc)m->type]->masked)
D 7
	  XCopyArea(display,
		    rotated_pix[(uc)m->type][m->direction - 1],
		    roomwin, mainGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT,
		    wx, wy);
E 7
I 7
	  DrawOnSquare(rotated_pix[(uc)m->type][m->direction - 1],NULL,
		       m->x,m->y,TEMPDRAW);
E 7
	else
E 12
I 12
       /* draw rotated pixmap */
       if (info[(uc)m->type]->masked)
E 12
D 7
	  {
	    XCopyArea(display,
		      rotated_mask[(uc)m->type][m->direction - 1],
		      roomwin, maskGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT,
		      wx, wy);
	    
	    XCopyArea(display, 
		      rotated_pix[(uc)m->type][m->direction - 1],
		      roomwin, drawGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, 
		      wx, wy);
	  }
E 7
I 7
	  DrawOnSquare(rotated_pix[(uc)m->type][m->direction - 1],
		       rotated_mask[(uc)m->type][m->direction - 1],
D 12
		       m->x,m->y,TEMPDRAW);
E 12
I 12
		       m->x,m->y,MDRAW);
	else
	  DrawOnSquare(rotated_pix[(uc)m->type][m->direction - 1],NULL,
		       m->x,m->y,MDRAW);
E 12
E 7
      }
}  
E 5

D 5
      if (info[(uc)m->type]->masked)
        XCopyArea(display,
		  rotated_mask[(uc)m->type][m->direction - 1],
		  roomwin, maskGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy);
E 5
I 5
D 7
/*****************************************************************/
/* draw the missile using the XOR function for speed */
E 5

D 5
      XCopyArea(display, 
		rotated_pix[(uc)m->type][m->direction - 1],
		roomwin, drawGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy);
    }
D 2
  }
  else {	/* special missile draw */

    XCopyArea(display, special_mask[m->type][m->direction],
	      roomwin, mainGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
    XCopyArea(display, special_pix[m->type][m->direction],
	      roomwin, mainGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
  }
E 2
}
E 5
I 5
xor_missile(m, roomnum)
Missile *m;
int roomnum;
{
  int wx, wy;
E 5

I 5
  /* check that missile is in this room */
  if (roomnum != current_room) return;
E 5

I 5
  /* look for an out of bounds condition */
  if (m->x <  0  ||  m->x >= ROOM_WIDTH ||
      m->y <  0  ||  m->y >= ROOM_HEIGHT)  return;

  /* find proper draw spot */
  wx = PIXELX(m->x);
  wy = PIXELY(m->y);

  /* if this is a normal object figure out which pixmap to use, possibly,
     we will need to use a directional pixmap for this kind of missile */
  if (m->direction == 0) 
    XCopyArea(display, obj_pixmap[(unsigned char)m->type], roomwin,
	      xoriGC,0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy);
  else 
    XCopyArea(display, rotated_pix[(uc)m->type][m->direction - 1],roomwin,
	      xoriGC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy);
}  
E 5

E 7
/* draw a hit pixmap onto given square if there is one here */

redraw_hit(roomnum, x, y)
int roomnum, x, y;
{
  /* look for an out of bounds condition */
  if (x <  0  ||  x >= ROOM_WIDTH ||
      y <  0  ||  y >= ROOM_HEIGHT ||
      roomnum < 0 || roomnum >= mapstats.rooms)  return;

  if (is_hit_on_square(roomnum, x, y)) draw_hit(roomnum, x, y);
}



/* draw a hit pixmap onto this square */

draw_hit(roomnum, x, y)
int roomnum, x, y;
{
D 7
  int wx, wy;

E 7
  /* check that hit is in this room */
  if (roomnum != current_room) return;

D 7
  /* find proper draw spot */
  wx = PIXELX(x);
  wy = PIXELY(y);


   /* draw the hit mask */
   XCopyArea(display, hit_mask, roomwin, maskGC, 0, 0,
	     BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 

   /* put down the hit pixmap */
   XCopyArea(display, hit_pix, roomwin, drawGC, 0, 0,
	     BITMAP_WIDTH, BITMAP_HEIGHT, wx, wy); 
E 7
I 7
  DrawOnSquare(hit_pix,hit_mask,x,y,TEMPDRAW);
E 7
}



/* redraw the window where the room is represented, pass TRUE for
   area_exposure if this redraw request is from an XExposeEvent.  When
   it is from an XExposeEvent we will try to be clever and only redraw
   the area requested in the event.  We can get info about the last event
   from the global variable last_event. */

redraw_player_room_window_squares(area_exposure)
int area_exposure;
{
  int i, x, y;
  XExposeEvent *evt;

  /* if current_room is -1 then don't redraw as per normal */
  if (current_room == -1) return;

  evt = (XExposeEvent *) &last_event;

  /* if this was an exposure event, redraw only part */
  if (area_exposure && evt->type == Expose)
    redraw_player_room_pixel_area(evt->x, evt->y, evt->width, evt->height);

  /* but if this is a normal request (probably change room) redraw all */
  else {
    /* set the background to be like the dominant floor tile in this room */
    XSetWindowBackgroundPixmap(display, roomwin,
		      obj_pixmap[(unsigned char)room[current_room].floor]);
    XClearWindow(display, roomwin);
I 10
    /* let the drawing optimizer to know that things have changed room */
    ClearRoomDraw();
E 10

    for (i=0; i<CIRCLE_OUT; i++) {
      x = gameperson[0]->x + circlex[i];
      y = gameperson[0]->y + circley[i];
      if (x>=0 && x<ROOM_WIDTH && y>=0 && y<ROOM_HEIGHT)
        QDraw_request(current_room, x, y, EVERYTHING);
    }
  }
}



/* Redraw a given pixel area in the room.  That means find the smallest area
   of character blocks that contains these pixels and redraw them. */

redraw_player_room_pixel_area(x, y, width, height)
int x, y, width, height;
{
  int x1, y1, x2, y2, i, ax, ay;

  /* assign the character block coordinates and check for oversize region */
  x1 = CONVERTRX(x);
  y1 = CONVERTRY(y);
  x2 = CONVERTRX((x+width));
  x2 = (x2 >= ROOM_WIDTH) ? (ROOM_WIDTH-1) : x2;
  y2 = CONVERTRY((y+height));
  y2 = (y2 >= ROOM_HEIGHT) ? (ROOM_HEIGHT-1) : y2;
  
  /* redraw the appropriate squares, starting near the player */
  for (i=0; i<CIRCLE_OUT; i++) {
    ax = gameperson[0]->x + circlex[i];
    ay = gameperson[0]->y + circley[i];
    if (ax>=x1 && ax<=x2 && ay>=y1 && ay<=y2)
I 10
      {
	MarkSquareAsBlank(ax,ay);
E 10
        QDraw_request(current_room, ax, ay, EVERYTHING);
I 10
      }
E 10
  }
}



/* ============================ O U T P U T ============================ */



/* wait for graphics to catch up with game play */

wait_for_graphics()
{
  XSync(display, False);
}



/* remove a game person from his current location, but only if he is
   just now becoming non-visible */

erase_current_game_person(num)
int num;
{
  /* look for an out of bounds condition */
  if (gameperson[num]->room != current_room ||
      gameperson[num]->x <  0  ||
      gameperson[num]->x >= ROOM_WIDTH ||
      gameperson[num]->y <  0  ||
      gameperson[num]->y >= ROOM_HEIGHT)  return;

I 3
#ifndef RAYCODE
E 3
  /* check that he used to be visible */
  if (old_see[num] == FALSE || can_see[num] == TRUE) return;
I 3
#endif
E 3

  /* redraw the room square so that he will disappear */
  QDraw_priority_request(gameperson[num]->room, gameperson[num]->x,
		gameperson[num]->y, CONCEALED);
}




/* remove a game person from his old location */

erase_old_game_person(num)
int num;
{
  /* look for bad game person number */
  if (num<0 || num>=persons_in_game) return;

  /* look for an out of bounds condition */
  if (gameperson[num]->oldroom != current_room ||
      gameperson[num]->oldx <  0  ||
      gameperson[num]->oldx >= ROOM_WIDTH ||
      gameperson[num]->oldy <  0  ||
      gameperson[num]->oldy >= ROOM_HEIGHT)  return;

I 3
#ifndef RAYCODE
E 3
  /* we don't erase the persons that we could not see anyway */
  if (num >= num_persons && !can_see[num] && !old_see[num]) return;
I 3
#endif
E 3

  /* redraw the room square so that he will disappear */
  QDraw_priority_request(gameperson[num]->oldroom, gameperson[num]->oldx,
		gameperson[num]->oldy, CONCEALED);
}




/* add a character to the input window */

add_char_to_input_window(c, pnt)
char c;
int  pnt;
{
  char s[3];

  sprintf(s, "%c_", c);
  text_write(inwin, tinyfont, GEN_GAP, GEN_GAP, pnt, 0, s);
}



/* send the input line as a message to someone */

send_input_line()
{
  char copy[MSG_LENGTH], to[MSG_LENGTH], msg[MSG_LENGTH], out[MSG_LENGTH];
  int receiver;
  
  /* check that there is content */
  if (inpnt == 0) return;

  /* separate into address and message */
  strcpy(copy, inmsg);
  strcpy(to,first_and_rest(copy, msg));

  /* send the message off */
  sprintf(out, "%d->%s %s", gameperson[0]->id, to, msg);
  receiver = notify_of_message(gameperson[0]->id, to, out);

  /* clear the input window */
  inpnt = 0;
  redraw_input_window();

  /* place a copy of the message in our message window if there was a valid
     receiver for the message */
  if (receiver >= 0 || receiver == TO_ALL || receiver == TO_GM) 
    player_out_message(out);
  else {
    sprintf(out, "Unknown message recipient: %s", to);
D 18
    redraw_error_window(out);
E 18
I 18
    player_error_out_message(out);
E 18
  }
}



/* ====================== V I S I B I L I T Y ======================== */

/* update a person's visiblity flag for the current person */

flag_visibility(pnum)
int pnum;
{
  old_see[pnum] = can_see[pnum];
  if (pnum < num_persons) can_see[pnum] = TRUE;
  else update_person_visibility(0, pnum, can_see);
}


/* update all person visibilty flags */

flag_all_visibilities()
{
  register int i;

  for (i=0; i<persons_in_game; i++) {
    old_see[i] = can_see[i];
    if (i < num_persons) can_see[i] = TRUE;
    else update_person_visibility(0, i, can_see);
  }
}



/* ========================= Player Listing ============================== */

#define	COLS_PER_PERSON		50
#define LINES_PER_PERSON	4
#define	TOP_BOTTOM_PAD		1	/* blanks lines at top and bottom */
#define	COLS_BETWEEN		1	/* columns between person listings */
#define	COLS_FOR_BITMAP		6	/* columns reserved for bitmap */
#define	COLS_FOR_PADDING	1	/* columns between bitmap and info */
#define	COLS_FOR_INFO		(COLS_PER_PERSON - COLS_FOR_BITMAP -	\
				 COLS_FOR_PADDING)
#define	LIST_FONT		(tinyfont)
#define	MAX_LIST_COLUMNS	(PLAYER_MAIN_W / char_width(LIST_FONT))
#define	LIST_MAJOR_COLUMNS	(MAX_LIST_COLUMNS /			\
				 (COLS_PER_PERSON + COLS_BETWEEN))
#define ACTUAL_LIST_COLUMNS	(LIST_MAJOR_COLUMNS *			\
				 (COLS_PER_PERSON + COLS_BETWEEN))


/* Initially ask for window listing all players in the game */

call_for_player_list()
{
  int i, num_to_list, num_major_list_lines, line, col;
  
  /* if there is already a player list window in existence then kill it
     instead of creating a new one.  This is sort of a toggling function */
  if (player_list) {
    shut_down_player_list();
    return;
  }

  /* figure out how many players we are going to have to list */
  for (i=0,num_to_list = 0; i < persons_in_game; i++)
    if (gameperson[i]->listed) num_to_list++;

  /* figure out how many lines (entry parcels in height) that will be */
  num_major_list_lines = ((num_to_list - 1) / LIST_MAJOR_COLUMNS) + 1;

  /* ask for a help window of the appropriate size */
  player_list = make_help_window(mainwin, 0, 0, ACTUAL_LIST_COLUMNS,
				 num_major_list_lines * LINES_PER_PERSON +
				 TOP_BOTTOM_PAD * 2,
				 LIST_FONT);

  /* go through all persons in game, placing an entry in window for each */
  line = 1;	col = 0;	/* initial line and col to use */
  for (i=0; i<persons_in_game; i++)
    if (gameperson[i]->listed) {
      /* place entry about person in the appropriate location */
      place_player_entry(i, player_list, col, line);
      /* move line and col pointers to next location */
      col += (COLS_PER_PERSON + COLS_BETWEEN);
      if (col >= ACTUAL_LIST_COLUMNS) {
	col = 0;
	line += LINES_PER_PERSON;
      }
    }

  /* place the window according to its size an map it */
  move_help_window(player_list, (PLAYER_MAIN_W - HW_WIDTH_PIXELS(player_list) -
		   WINDOW_BORDERS) / 2, PLAYER_MAIN_H -
		   HW_HEIGHT_PIXELS(player_list) - WINDOW_BORDERS);
  show_help_window(player_list);

  /* now go through all persons in game, placing bitmap for each */
  line = 1;	col = 0;	/* initial line and col to use */
  for (i=0; i<persons_in_game; i++)
    if (gameperson[i]->listed) {
      /* place bitmap for person in the appropriate location */
      place_player_image(i, player_list, col, line);
      /* move line and col pointers to next location */
      col += (COLS_PER_PERSON + COLS_BETWEEN);
      if (col >= ACTUAL_LIST_COLUMNS) {
	col = 0;
	line += LINES_PER_PERSON;
      }
    }

  /* open up input lines for the help window */
  init_input_lines(HW_WINDOW(player_list));
}




/* place information about a player in the given location in help window */

place_player_entry(pnum, help_rec, col, line)
int pnum;
Help *help_rec;
int col, line;
{
  char s[200];
  int left_col = col + COLS_FOR_BITMAP + COLS_FOR_PADDING;
  double ratio;

  /* place player's name, id number, and login */
  sprintf(s, "%d %s %s@%s", gameperson[pnum]->id, gameperson[pnum]->name,
	  gameperson[pnum]->login, gameperson[pnum]->host);
  set_at(help_rec, left_col, line, s);

  /* place player's rank */
  sprintf(s, "Level %d (%s)", gameperson[pnum]->level, gameperson[pnum]->rank);
  set_at(help_rec, left_col, line + 1, s);

  /* place player's team name */
  if (gameperson[pnum]->team == 0)
    strcpy(s, "neutral");
  else
    strcpy(s, mapstats.team_name[gameperson[pnum]->team - 1]);
  set_at(help_rec, left_col, line + 2, s);

  /* figure out kill ratio */
  if (gameperson[pnum]->losses)
    ratio = (double) gameperson[pnum]->kills /
	    (double) gameperson[pnum]->losses;
  else
    ratio = (double) gameperson[pnum]->kills;

  /* place kills losses games and ratio line */
  sprintf(s, "%d kills  %d losses  ratio %4.2f  %d games", 
	  gameperson[pnum]->kills, gameperson[pnum]->losses, ratio,
	  gameperson[pnum]->games);
  set_at(help_rec, left_col, line + 3, s);
}




/* place bitmap for a player in the given location in help window */

place_player_image(pnum, help_rec, col, line)
int pnum;
Help *help_rec;
int col, line;
{
  /* place person's bitmap down */
  pixmap_in_help_window(help_rec, col, line, person_pix[pnum],
			BITMAP_WIDTH, BITMAP_HEIGHT);
}




/* close up the player list window */

shut_down_player_list()
{
  /* unmap the player list help window and destroy it */
  if (player_list) {
    hide_help_window(player_list);
    destroy_help_window(player_list);
    player_list =  NULL;
  }
}
E 1
