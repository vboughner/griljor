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
#include "mapfunc.h"
#include "mapstore.h"
#include "emap.h"
#include "bit/link"		/* bitmap of link box containing room number */
#include "bit/editmapicon"	/* icon bitmap */
#include "bit/scrollup"		/* scroll button */
#include "bit/scrolldown"	/* scroll button */
#include "bit/pageup"		/* scroll button */
#include "bit/pagedown"		/* scroll button */
#include "bit/add"		/* cursor for add editing mode */
#include "bit/ovr"	/* cursor for overwrite editing mode */
#include "bit/xcg"		/* cursor for exchange editing mode */
#include "bit/del"		/* cursor for delete editing mode */
#include "bit/modemask"		/* bitmap mask for the mode cursors */
#include "bit/box"		/* box to place around pile displayed */
#include "bit/bmask"		/* mask for pile display box */


/* placement and size of toplevel window */
#define EDITOR_WIN_X		30
#define EDITOR_WIN_Y		5
#define EDITOR_WIN_W	(ROOM_PIXELS_WIDE + PILE_WIN_W + \
			 GRAB_WIN_W + WINDOW_BORDERS * 2)
#define EDITOR_WIN_H	(GRAB_WIN_H + CLICK_WIN_H + WINDOW_BORDERS)

/* constants for placement of the object definition file display window */
#define GRAB_WIN_WIDTH		8	/* # of objects wide grab window is */
#define GRAB_WIN_HEIGHT		18	/* # of objects high */
#define GRAB_WIN_TOPAREA	1	/* # of objects high top area is */
#define GRAB_SPACING		1	/* space (pixels) between objects */
#define GRAB_WIN_W	(GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING))
#define GRAB_WIN_H	(GRAB_WIN_HEIGHT * (BITMAP_HEIGHT + GRAB_SPACING))
#define GRAB_WIN_X	(ROOM_PIXELS_WIDE + PILE_WIN_W + WINDOW_BORDERS)
#define GRAB_WIN_Y	(CLICK_WIN_H)

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

/* definitions for link box picture, amount of space for room number */
#define LINK_SPACE_HEIGHT	29
#define LINK_SPACE_WIDTH	(link_width - 4)
#define LINK_PIX_X	(MOUSE_PIX_X + (mouse_width - link_width) / 2)
#define LINK_PIX_Y	(MOUSE_PIX_Y + mouse_height / 2)

/* constants for use by the room linking function */
#define DORMANT		0	/* no links in progress now */
#define EXPECTING	1	/* one side of link has been made */

/* constants used by help window */
#define HELP_X		(-WINDOW_BORDERS)
#define HELP_Y		(-WINDOW_BORDERS)
#define HELP_LINES	18		/* num of lines in window */
#define HELP_COLS	80		/* num of columns window will hold */
#define HELP_FONT	(regfont)	/* font to use in help window */

/* key definitions for a couple of command keys */
#define NEUTRALS_KEY		'^'
#define NUM_OF_TEAMS_KEY	'&'

/* editing modes */
#define ADD_MODE	0
#define OVERWRITE_MODE	1
#define EXCHANGE_MODE	2
#define DELETE_MODE	3
#define NUM_OF_MODES	4	/* keep this current */

#define IN_ADD_MODE	(editor_mode == ADD_MODE)



typedef struct _link {
/* the informational record containing the location of a linking end */
	short	room;		/* room where link end is */
	short	direction;	/* direction out of room link extends */
	short	x, y;		/* coords of possible link exit char */
} Link;



typedef struct _paste {
/* the informational record for storing the cut and paste pointer */
	short		is_set;		/* TRUE if mark is set */
	short		room;		/* room where mark was set */
	short		x, y;		/* point where mark was set */
	MapInfo		*map;		/* one-room paste storage map */
	short		width, height;	/* size of buffer contents in room */
} Paste;



typedef struct _pile {
/* information concerning what is displayed in pile window */
	char		pertinent;	/* is TRUE when something displayed */
	MapInfo		*map;		/* pointer to map of square */
	SquareRec	*square;	/* pointer to the square displayed */
	int		roomnum, x, y;	/* which square in map displayed */
} Pile;


/* is TRUE when pile pointer is set in room window somewhere */
#define PILE_IN_ROOMWIN		(pile_content.map == &mapstats)



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

	

/* global variables */
Window	editwin;	/* main parent window for editor windows */
Window	grabwin;	/* window where editor characters displayed */
Window	statwin;	/* window where editor stats are displayed */
Window  clickwin;	/* window where click buffer contents displayed */
Window  pilewin;	/* window where pile of stuff on floor displayed */
Pixmap	link_pix;	/* room linking status picture */
Pixmap  scrollup, scrolldown, pageup, pagedown; /* scrolling buttons */
Pixmap	box_pix, box_mask;	/* pile display square outline box */
Cursor	add_cur, ovr_cur, xcg_cur, del_cur;	/* editing cursors */
Click	click;		/* object click buffers */
int	link_status = DORMANT;	/* status of room linking function */
Link	link_info;		/* the particulars of the first link */
Paste	paste;		/* the cut and paste buffer information */
Help	*help_rec;	/* record containing info for help window */
int	first_in_grab=0;	/* object number of object at top left */
char	last_command_key = ' ';	/* last key command entered */
int	editor_mode = ADD_MODE;	/* initial mode */
Pile	pile_content;	/* contents of pile window */


/* procedure declarations */
char *get_editor_input_line();
int redraw_a_window();


load_editor_cursors()
/* load the cursors for the editor program */
{
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
  define_thin_arrow_cursor(grabwin);
  define_thin_arrow_cursor(statwin);
  define_thin_arrow_cursor(clickwin);
  define_thin_arrow_cursor(help_rec->win);
}



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

}



toggle_cursor_mode()
/* change to the next mode in the set of possible modes */
{
  editor_mode++;
  if (editor_mode >= NUM_OF_MODES) editor_mode = 0;
  change_cursor_mode(editor_mode);
}



editor_window_setup()
/* set up the windows needed by the map editor */
{
  editwin = XCreateSimpleWindow(display, RootWindow(display, screen),
				EDITOR_WIN_X, EDITOR_WIN_Y, EDITOR_WIN_W,
				EDITOR_WIN_H, WINDOW_BORDERS, fgcolor,
				bgcolor);
  
  /* set up name and icon for top level window */
  set_up_top_level_icon(editwin);

  roomwin = XCreateSimpleWindow(display, editwin, - WINDOW_BORDERS, STAT_WIN_H,
				ROOM_PIXELS_WIDE, ROOM_PIXELS_HIGH,
				WINDOW_BORDERS, fgcolor, bgcolor);

  grabwin = XCreateSimpleWindow(display, editwin, GRAB_WIN_X,
				GRAB_WIN_Y, GRAB_WIN_W, GRAB_WIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);

  statwin = XCreateSimpleWindow(display, editwin, -WINDOW_BORDERS,
				-WINDOW_BORDERS, STAT_WIN_W, STAT_WIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);

  clickwin = XCreateSimpleWindow(display, editwin, STAT_WIN_W,
				 -WINDOW_BORDERS, CLICK_WIN_W, CLICK_WIN_H,
				 WINDOW_BORDERS, fgcolor, bgcolor);

  pilewin = XCreateSimpleWindow(display, editwin, ROOM_PIXELS_WIDE,
				CLICK_WIN_H, PILE_WIN_W, PILE_WIN_H,
				WINDOW_BORDERS, fgcolor, bgcolor);

  /* set up the help window */ 
  help_rec = make_help_window(roomwin, HELP_X, HELP_Y,
			      HELP_COLS, HELP_LINES, HELP_FONT);

  /* load appropriate text into help window */
  load_help_window_contents(help_rec, EDITOR_HELP_FILE);
  
  /* store the mouse representation bitmap */
  define_mouse_bitmap();

  /* store the link picture representation */
  link_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			link_bits, link_width, link_height,
			fgcolor, bgcolor, depth);

  /* store the pile box pixmaps */
  box_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			box_bits, box_width, box_height, fgcolor, bgcolor,
			depth);
  box_mask = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			bmask_bits, bmask_width, bmask_height,
			maskfg, maskbg, 1);

  /* store scroll button representations */
  load_scroll_button_pixmaps();

  /* set the global redraw procedure */
  GlobalRedraw = redraw_a_window;
}



set_up_top_level_icon(win)
Window win;
{
  Pixmap icon_pix;
  XWMHints wmhints;

  XStoreName(display, win, "Editmap");
  icon_pix = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
		editmapicon_bits, editmapicon_width, editmapicon_height,
		fgcolor, bgcolor, depth);
  wmhints.icon_pixmap = icon_pix;
  wmhints.flags = IconPixmapHint;
  XSetWMHints(display, win, &wmhints);
  /* don't free the pixmap, it has to stick around to be the icon */
}



map_editor_windows()
/* make the previously set up windows visible */
{
  /* start up input in the windows */
  init_input_lines(editwin);
  init_input_lines(grabwin);
  init_input_lines(roomwin);
  init_input_lines(statwin);
  init_input_lines(clickwin);
  init_input_lines(pilewin);
  init_input_lines(help_rec->win);

  /* map the windows */
  XMapWindow(display, editwin);
  XMapWindow(display, roomwin);
  XMapWindow(display, grabwin);
  XMapWindow(display, statwin);
  XMapWindow(display, clickwin);
  XMapWindow(display, pilewin);
}



editor_setup_variables()
/* set up the variables about what is on the screen, and set the initial
   background of the room window */
{
  /* pick starting room and set window background */
  current_room = 0;
  XSetWindowBackgroundPixmap(display, roomwin,
		      obj_pixmap[room[current_room].floor]);
  XClearWindow(display, roomwin);

  /* initialize click buffer */
  click.num_buffers = 3;
  click.map = lib_create_new_map(NULL);

  /* init paste buffer variables */
  paste.is_set = FALSE;
  paste.map = lib_create_new_map(NULL);
  paste.width = 0;
  paste.height = 0;

  /* initialize the pile window contents */
  bzero(&pile_content, sizeof(Pile));
}



load_scroll_button_pixmaps()
{
  scrollup = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			scrollup_bits, scrollup_width, scrollup_height,
			fgcolor, bgcolor, depth);
  scrolldown = XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
			scrolldown_bits, scrolldown_width, scrolldown_height,
			fgcolor, bgcolor, depth);
  pageup = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			pageup_bits, pageup_width, pageup_height,
			fgcolor, bgcolor, depth);
  pagedown = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			pagedown_bits, pagedown_width, pagedown_height,
			fgcolor, bgcolor, depth);
}



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
      case EXPOSED:	/* no longer need this, get_input() handle these */
			/* redraw_a_window(happenwin); */
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
  int result = FALSE;

  /* clear message area of previous garbage */
  place_editor_message(" ");
  
  if (isupper(c)) c = tolower(c);

  switch (c) {
	case 'm':	DialogEditMapInfo(roomwin, -WINDOW_BORDERS,
					  -WINDOW_BORDERS);
			break;
	case 'r':	DialogEditRoomInfo(roomwin, -WINDOW_BORDERS,
					   -WINDOW_BORDERS);
			break;
	case 'q':	if (!want_query("Do you really want to quit?")) break;
	  		will_save_msg();
			save_map();
			save_msg();
			result = TRUE;
			break;
	case '!':	if (!want_query("Really quit without saving?")) break;
			result = TRUE;
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
			break;
	case 'g':	if (what_win == roomwin)
			  follow_cursor_to_next_room(CONVERTRX(x),
						     CONVERTRY(y));
	  		else enter_new_room_number();
			break;
	case 't':	toggle_cursor_mode();
			break;
	case 'a':	change_cursor_mode(ADD_MODE);
	  		break;
	case 'd':	change_cursor_mode(DELETE_MODE);
	  		break;
	case 'x':	change_cursor_mode(EXCHANGE_MODE);
	  		break;
	case 'v':	change_cursor_mode(OVERWRITE_MODE);
	  		break;
	case 'n':	if (what_win == roomwin) {
			  follow_cursor_and_make_room(CONVERTRX(x),
						      CONVERTRY(y));
			}
	  		else {
			  add_new_room();
			  change_rooms(mapstats.rooms - 1);
			}
			break;
	case 'j':	if (what_win == roomwin)
	  		  make_random_room(CONVERTRX(x),
					   CONVERTRY(y));
			break;
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
	case 'k':	if (what_win == roomwin)
			  destroy_link(current_room, CONVERTRX(x),
				       CONVERTRY(y));
	  		else if (what_win == clickwin)
			  stop_linking();
			break;
	case ')':	if (what_win == roomwin) clear_room();
			break;
	case 's':	will_save_msg();
	  		save_map();
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
	case 'i':	handle_info_request(what_win, x, y);
			break;
	case 'o':	toggle_owning_team();
			break;
	case NUM_OF_TEAMS_KEY:
	  		change_num_of_teams();
			break;
	case NEUTRALS_KEY:
			toggle_neutrals_allowed();
			break;
	case 'h':
	case '?':	toggle_help_window(help_rec);
			break;
	default:	break;
  }

  last_command_key = c;    /* some command like to apply on second request */
  return(result);
}



handle_editor_mouse(x, y, button, detail, what_window)
/* determine what to do with a mouse button press event depending on
   which window it occurred in. */
int x, y;
char button;
int  detail;
Window what_window;
{
  if (what_window == grabwin) handle_grabwin_mouse(x, y, button, detail);
  else if (what_window == roomwin) handle_roomwin_mouse(x, y, button, detail);
  else if (what_window == pilewin) handle_pilewin_mouse(x, y, button, detail);
  else if (what_window == clickwin) handle_clickwin_mouse(x, y, button, detail);
  else if (what_window == statwin) handle_statwin_mouse(x, y, button, detail);
}



handle_grabwin_mouse(x, y, button, detail)
/* handle the actions taken if mouse button was pressed in the grab window */
int x, y;
char button;
int detail;
{
  /* check mouse clicked in a scroll button and that scrolling is possible */
  if ((y >= 0) && (y < (GRAB_WIN_TOPAREA * (BITMAP_HEIGHT + GRAB_SPACING))) &&
      (objects > (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) * GRAB_WIN_WIDTH)) {
    /* figure out which scroll button was selected and do the scrolling */
    int col = x / (BITMAP_WIDTH + GRAB_SPACING);
    switch (col) {
	case 0:
	  scroll_grab_window(-1);
	  break;
	case 1:
	  scroll_grab_window(1);
	  break;
	case 2:
	  scroll_grab_window(-(GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
	  break;
	case 3:
	  scroll_grab_window((GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
	  break;
	default:
	  break;
    }
  }
  else {
    /* put the object selected into the click buffer */
    switch (button) {
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
    }
  }
}



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



handle_pilewin_mouse(x, y, button, detail)
/* handle the actions taken if mouse button was pressed in the pile window */
int x, y;
char button;
int detail;
{
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
}



handle_roomwin_mouse(x, y, button, detail)
/* handle the actions taken if mouse button was pressed in the room window */
int x, y;
char button;
int detail;
{
  int cx, cy;

  /* make a converted x and y for use in locating squares in map */
  cx = CONVERTRX(x);
  cy = CONVERTRY(y);
    
  /* check to see whether the mark has been set and we want to use
     the fill procedure */

  if (paste.is_set)
    switch (button) {
    case Button1:		fill_area(current_room, cx, cy,
					  CLICK_SQUARE(0));
				break;
    case Button2:		fill_area(current_room, cx, cy,
					  CLICK_SQUARE(1));
				break;
    case Button3:		fill_area(current_room, cx, cy,
					  CLICK_SQUARE(2));
    default:		break;
    }
  else {
    if (!(detail & ShiftMask)) 
      switch (button) {
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
      }
    else
      switch (button) {
	/* shift button held down, grab room characters */
      case Button1:		pull_square_into_click(cx, cy, 0);
				redraw_click_buffer(0);
				break;
      case Button2:		pull_square_into_click(cx, cy, 1);
				redraw_click_buffer(1);
				break;
      case Button3:		pull_square_into_click(cx, cy, 2);
				redraw_click_buffer(2);
				break;
      default:		break;
      }
  }
}



handle_statwin_mouse(x, y, button, detail)
/* check to see where the mouse was pressed in the status window and
   from that determine what we should do about it */
int x, y;
char button;
int detail;
{
  if (y > STAT_TOP_EDGE) {
    if (y < STAT_TOP_EDGE + STAT_REG_SPACE + STAT_PAD_SPACE)
	enter_new_map_name();
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE * 2 + STAT_PAD_SPACE * 2)
	enter_new_room_name(current_room);
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE * 3 + STAT_PAD_SPACE * 3)
        enter_new_team_name(room[current_room].team);
    else return;
  }

  place_editor_message(" ");
}



scroll_grab_window(lines)
/* scroll the grab window the given number of lines, positive number
   for scrolling upward, negative number for scrolling downward */
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



enter_new_map_name()
/* get a new name for the map */
{
  FREE(mapstats.name);
  mapstats.name =
    create_string(get_editor_input_line("Map name: ", MAP_NAME_LENGTH - 1));
  place_editor_map_name();
}



enter_new_room_name(roomnum)
/* get a new name for some room and assign it */
int roomnum;
{
  FREE(room[roomnum].name);
  room[roomnum].name =
    create_string(get_editor_input_line("Room name: ", ROOM_NAME_LENGTH - 1));
  place_editor_room_name();
}



enter_new_team_name(teamnum)
/* get a new name for some team */
int teamnum;
{
  /* check for team number out of range */
  if (teamnum < 0 || teamnum > NUM_OF_TEAMS) return;
  
  FREE(mapstats.team_name[teamnum]);
  mapstats.team_name[teamnum] =
    create_string(get_editor_input_line("New team name: ",
					TEAM_NAME_LENGTH - 1));
  place_editor_team_name();
}



enter_new_room_number()
/* get a new current room number and change to there */
{
  char s[250];
  int newroom;

  if (mapstats.rooms > 1) {
    sprintf(s, "Switch to room number (0-%d): ", mapstats.rooms - 1);
    newroom = atoi(get_editor_input_line(s, 10));
    if (newroom >= 0 && newroom < mapstats.rooms) change_rooms(newroom);
    else place_editor_message("Invalid room number.");
  }
  else {
    place_editor_message("There is only one room in this map.");
  }
}



toggle_owning_team()
/* toggle the team that owns current room and redraw the ownership */
{
  room[current_room].team++;
  if (room[current_room].team > mapstats.teams_supported)
    room[current_room].team = 0;

  place_editor_team_name();
}



change_num_of_teams()
/* change the number of teams allowed in the game map */
{
  char msg[80], *neu = "(and no neutrals)";

  /* add one to the number of teams */
  if (last_command_key == NUM_OF_TEAMS_KEY) mapstats.teams_supported++;
  if (mapstats.teams_supported > NUM_OF_TEAMS)  mapstats.teams_supported = 1;

  /* tell user about the change */
  if (mapstats.neutrals_allowed) 
    neu = "(plus neutrals)";
  if (mapstats.teams_supported == 1)
    sprintf(msg, "Now only one team %s", neu);
  else
    sprintf(msg, "Now %d teams %s", mapstats.teams_supported, neu);

  place_editor_message(msg);
}



toggle_neutrals_allowed()
{
  if (last_command_key == NEUTRALS_KEY)
    mapstats.neutrals_allowed = !mapstats.neutrals_allowed;

  if (mapstats.neutrals_allowed)
    place_editor_message("Neutrals now allowed in this map.");
  else
    place_editor_message("Neutrals not allowed.");
}



char *get_editor_input_line(prompt, maxlen)
/* get a line of input in editor message area */
char *prompt;
int maxlen;
{
  int exposed;
  
  /* clear the message area */
  clear_area(statwin, 8, STAT_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
	     STAT_WIN_W, STAT_WIN_H);
	     
  return get_string(statwin, regfont, STAT_LEFT_END,
		    STAT_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
		    prompt, maxlen, &exposed);
}



/* ============ C L I C K  B U F F E R S  A N D  P I L E ================== */



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
/* find out which object should be in the x y coords of the grab window,
   if they click on an object that doesn't exist, return 0 instead of the
   expected object number */
int x, y;
{
  int line, col, result;

  line = (y / (BITMAP_HEIGHT + GRAB_SPACING)) - GRAB_WIN_TOPAREA;
  col = x / (BITMAP_WIDTH + GRAB_SPACING);

  result =  first_in_grab + (line * GRAB_WIN_WIDTH) + col;
  if (result >= objects || result < 0) result = 0;

  return result;
}



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



/* ======================== O U T P U T =============================== */



redraw_a_window(win)
/* redraw the indicated window for the editor */
Window win;
{
  if (win == roomwin) {
    redraw_room_window(TRUE);
    draw_pile_box();
  }
  else if (win == grabwin) redraw_grab_window(-1, -1);
  else if (win == statwin) redraw_stat_window();
  else if (win == clickwin) redraw_click_window();
  else if (win == pilewin) redraw_pile_window();
  else check_help_expose(win);
}



redraw_grab_window(first, last)
/* draw the contents of the object grabbing window, given any
   subset of the lines within to redraw (from 0 to 
   (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA - 1),
   -1 in either means to redraw the entire window. */
int first, last;
{
  int i, j, nextobj;
  int x, y;

  if (first<0 || first>last || last>=(GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA)) {
    first = 0;
    last = (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) - 1;
  }

  if (GRAB_WIN_TOPAREA > 0) redraw_grab_scroll_buttons();

  nextobj = first_in_grab + (first * GRAB_WIN_WIDTH);
  for (j=GRAB_WIN_TOPAREA+first; 
       j<GRAB_WIN_TOPAREA+last+1 && nextobj<objects; j++)
    for (i=0; i<GRAB_WIN_WIDTH && nextobj<objects; i++) {
      x = (i * (BITMAP_WIDTH + GRAB_SPACING)) + GRAB_SPACING;
      y = (j * (BITMAP_HEIGHT + GRAB_SPACING)) + GRAB_SPACING;
      place_obj_pixmap(grabwin, x, y, nextobj, TRUE);
      nextobj++;
    }
}



redraw_grab_scroll_buttons()
/* redraw the scroll buttons at top of grab window */
{
  char s[500];

  /* place scroll buttons there (if scrolling is possible) */
  if (objects > (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) * GRAB_WIN_WIDTH) {
    XCopyArea(display, scrollup, grabwin, mainGC, 0, 0, 
	      scrollup_width, scrollup_height,
	      (0 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
    XCopyArea(display, scrolldown, grabwin, mainGC, 0, 0, 
	      scrolldown_width, scrolldown_height,
	      (1 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
    XCopyArea(display, pageup, grabwin, mainGC, 0, 0, 
	      pageup_width, pageup_height,
	      (2 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);
    XCopyArea(display, pagedown, grabwin, mainGC, 0, 0, 
	      pagedown_width, pagedown_height,
	      (3 * (BITMAP_WIDTH + GRAB_SPACING) + GRAB_SPACING),GRAB_SPACING);

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
}



redraw_stat_window()
/* redraw the stat window */
{
  place_various_editor_names();
  place_editor_message("");	  /* redraw last message */
}



redraw_click_window()
/* redraw the window with the click buffer display in it */
{
  redraw_mouse_click_buffer();
  redraw_link_picture();
  place_editor_room_number();
  draw_pile_box();
}



redraw_pile_window()
/* redraws the window that contains the contents of a particular square */
{
  XClearWindow(display, pilewin);

  if (pile_content.pertinent && pile_content.square) {
    int y = GRAB_SPACING;
    OI *ptr = pile_content.square->first;
    
    for (; ptr; ptr=ptr->next) {
      place_obj_pixmap(pilewin, 0, y, ptr->type, TRUE);
      y += BITMAP_HEIGHT + GRAB_SPACING;
    }
  }
}



redraw_mouse_click_buffer()
/* redraws the graphic representation of the mouse with the three click
   buffers inside */
{
  int i;

  /* place the mouse picture pixmap */
  place_mouse_picture(clickwin, MOUSE_PIX_X, MOUSE_PIX_Y);
  
  /* place each of the three click buffer images */
  for (i=0; i<3; i++)
    redraw_click_buffer(i);
}



redraw_click_buffer(num)
/* draw the contents of one of the click buffer boxes */
int num;
{
  SquareRec *square = get_square(click.map, 0, num, 0);

  /* place the object in the right box on the mouse picture */
  if (square) {
    int x, y;
    x = (MOUSE_PIX_X + MOUSE_START_X + num * (BITMAP_WIDTH + MOUSE_SPACING));
    y = (MOUSE_PIX_Y + MOUSE_START_Y);
    draw_objects_on_square(square, clickwin, x, y, FALSE);
  }
}



redraw_link_picture()
/* draw the link picture and room number, this procedure checks for itself
   whether there is a current link or not */
{
  char s[10];
  
  if (link_status == DORMANT) {
    redraw_mouse_click_buffer();
  }
  else {
    XCopyArea(display, link_pix, clickwin, mainGC, 0, 0, link_width,
	      link_height, LINK_PIX_X, LINK_PIX_Y);
    sprintf(s, "%d", link_info.room);
    text_center(clickwin, bigfont, LINK_PIX_X + link_x_hot, LINK_PIX_X +
		link_x_hot + LINK_SPACE_WIDTH, LINK_PIX_Y + link_y_hot,
		LINK_PIX_Y + link_y_hot + LINK_SPACE_HEIGHT, s);
  }
}



place_various_editor_names()
/* place the various name associated with the editor program */
{
  place_editor_map_name();
  place_editor_room_name();
  place_editor_team_name();
}



place_editor_map_name()
{
  place_map_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE, STAT_RIGHT_END,
		 STAT_TOP_EDGE + STAT_REG_SPACE + STAT_PAD_SPACE);
}



place_editor_room_name()
{
  place_room_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE + STAT_REG_SPACE +
		  STAT_PAD_SPACE, STAT_RIGHT_END, STAT_TOP_EDGE +
		  STAT_REG_SPACE * 2 + STAT_PAD_SPACE * 2);
}



place_editor_team_name()
{
  place_room_team(current_room, statwin, STAT_LEFT_END,
		  STAT_TOP_EDGE + STAT_REG_SPACE * 2 + STAT_PAD_SPACE * 2,
		  STAT_RIGHT_END, STAT_TOP_EDGE + STAT_REG_SPACE * 3 +
		  STAT_PAD_SPACE * 3);
}



place_editor_room_number()
{
  char s[40];

  sprintf(s, "Room %d", current_room);
  clear_area(clickwin, 8, CLICK_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
		 MOUSE_PIX_X, CLICK_WIN_H - STAT_PAD_SPACE);
  place_a_string(clickwin, s, 8, CLICK_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
		 MOUSE_PIX_X, CLICK_WIN_H - STAT_PAD_SPACE);
}


place_editor_message(line)
/* place a message in the stat window in the appropriate font.  If a null
   length message is given as the message, then redraw the last message */
char *line;
{
  static char	current_msg[200] = "";

  if (strlen(line) != 0) strcpy(current_msg, line);

  clear_area(statwin, 8, STAT_WIN_H - STAT_REG_SPACE - STAT_PAD_SPACE,
	     STAT_WIN_W, STAT_WIN_H);
  place_a_string(statwin, current_msg, 8, STAT_WIN_H - STAT_REG_SPACE -
		 STAT_PAD_SPACE, STAT_WIN_W, STAT_WIN_H);
}



will_save_msg()
/* place message about the fact that map will be saved on the screen */
{
  char msg[250];
  
  /* let person using editor know that we're saving */
  sprintf(msg, "Saving %d rooms to file %s ...", mapstats.rooms, mapfile);
  place_editor_message(msg);
  XFlush(display);	/* necessary */
}



save_msg()
/* place message about the fact that map was just saved on the screen */
{
  char msg[250];
  
  /* let person using editor know that we've saved */
  sprintf(msg, "%d rooms saved in file %s", mapstats.rooms, mapfile);
  place_editor_message(msg);
}



change_rooms(newroom)
/* editor wants to change to a different room */
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
      place_editor_room_name();
      place_editor_room_number();
      place_editor_team_name();
      redraw_room_window(FALSE);
      if (PILE_IN_ROOMWIN) draw_pile_box();
    }
    else {
      /* otherwise, let him know it we are going to same room */
      place_editor_message("Exit led back into room");
    }
  }
  else {
    /* report that you asked to change to an invalid room number */
    place_editor_message("Exit to non-existant room");
  }
  
}



set_room_floor(objnum, set)
/* set the floor character for the entire room, if set flag is TRUE
   then actually replace the current floor character on all spaces */
int objnum, set;
{
  int i, j;

  /* if the object is a recorded one then this is not a good idea */
  if (info[(uc)objnum]->recorded) {
    place_editor_message("no recorded objects for floors");
    return;
  }

  /* place floor character on map */
  if (set) {
    for (j=0; j<ROOM_HEIGHT; j++)
      for (i=0; i<ROOM_WIDTH; i++) {
        put_down_object(i, j, objnum);
        draw_room_square_check_pile(i, j, FALSE);
      }
  }
      
  /* place floor character in room record */
  room[current_room].floor = objnum;

  /* change the background of the window to reflect new floor */
  XSetWindowBackgroundPixmap(display, roomwin, 
			     obj_pixmap[room[current_room].floor]);
  place_editor_message("official floor character set");
}



replace_in_room(victim, replacement)
/* replaces all instances of a certain object in a room with another obj */
int victim, replacement;
{
  OI *ptr;
  int i, j;

  for (j=0; j<ROOM_HEIGHT; j++)
    for (i=0; i<ROOM_WIDTH; i++) {
      ptr = first_obj_here(&mapstats, current_room, i, j);
      for (; ptr; ptr=ptr->next) {
	if (ptr->type == victim) change_object_type(ptr, replacement);
      }
      draw_room_square_check_pile(i, j, FALSE);
    }
}



/* ===================== C U R S O R   F O L L O W ======================= */


follow_cursor_to_next_room(x, y)
/* follow the direction or exit character that the cursor points us to,
   it is assumed that this was called while cursor was in roomwin, we
   are given the character x and y coords, we will change rooms to the
   appropriate room */
int x, y;
{
  int target_room, dir, start_room;
  OI *exit;
  
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
    exit = what_exit_obj_here(&mapstats, start_room, x, y);
    warp_mouse_through_exit(exit);
  }
}



follow_cursor_and_make_room(x, y)
/* this is like follow_cursor_to_next_room, but is meant to be used when
   there is no such room yet, and it must be created before we can go
   into it for editing purposes.  Make it and change into it. */
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
  /* otherwise link the directions between the two new rooms */
  else {
    link_one_room_to_another(current_room, dir, mapstats.rooms-1);
    copy_edge_of_room(current_room, dir, mapstats.rooms-1);
  }

  /* now change to that room */
  change_rooms(mapstats.rooms-1);
}



startup_linkage(x, y)
/* start up a linkage in the current room given a particular x and y */
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



finalize_link(dest_room, x, y)
/* complete a link started previously, this will be a one way link from one
   room (or room's exit char) to another room */
int dest_room, x, y;
{
  OI *obj;
  
  link_status = DORMANT;
  redraw_link_picture();

  /* if the direction from the previous room was a compass direction, then
     we alter the exit flag for the previous room */
  if (link_info.direction >= 0) {
    room[link_info.room].exit[link_info.direction] = dest_room;
    return;
  }

  /* otherwise, it was an exit character, and we need to find out which one */
  obj = what_exit_obj_here(&mapstats, link_info.room,
			   link_info.x, link_info.y);
  if (!obj) {
    place_editor_message("Bad start of link - no link made");
    return;
  }

  /* place into that char the required info */
  set_record(obj, REC_DETAIL, dest_room);
  set_record(obj, REC_INFOX, x);
  set_record(obj, REC_INFOY, y);
}



finalize_two_way_link(dest_room, x, y)
/* complete a link started previously, this will be a TWO way link from one
   room (or room's exit char) to another room and back.  For exit chars,
   if the second room has another exit char at the link point, then this
   will work, if not, only a one way link will be created. */
int dest_room, x, y;
{
  OI *exitchar, *destchar;
  int dir;
  
  link_status = DORMANT;
  redraw_link_picture();

  /* if the direction from the previous room was a compass direction, then
     we link the two rooms easily enough */
  if (link_info.direction >= 0) {
    link_one_room_to_another(link_info.room, link_info.direction, dest_room);
    return;
  }

  /* otherwise, it was an exit character, and we need to find out which one */
  exitchar = what_exit_obj_here(&mapstats, link_info.room,
				link_info.x, link_info.y);
  if (!exitchar) {
    place_editor_message("Bad start of link - no link made");
    return;
  }

  /* look for an exit char in the destination square, if there is one
     then we scold the user for trying to make an infinite loop */
  destchar = what_exit_obj_here(&mapstats, dest_room, x, y);
  if (destchar) {
    place_editor_message("No making infinite loops");
    return;
  }

  /* place into that char the required info */
  set_record(exitchar, REC_DETAIL, dest_room);
  set_record(exitchar, REC_INFOX, x);
  set_record(exitchar, REC_INFOY, y);

  /* look for nearby exit character */
  destchar = nearby_exit_character(dest_room, x, y, &dir);

  /* if no exit character nearby make one way link only */
  if (!destchar) {
    place_editor_message("No nearby exit - one way link only");
    return;
  }

  /* there was an exit char nearby, lets link */
  set_record(destchar, REC_DETAIL, link_info.room);
  set_record(destchar, REC_INFOX, link_info.x + xdir[dir]);
  set_record(destchar, REC_INFOY, link_info.y + ydir[dir]);
}



stop_linking()
/* stop a link that is in progress */
{
  link_status = DORMANT;
  place_editor_message("Linkage aborted");
  redraw_link_picture();
}



destroy_link(roomnum, x, y)
/* destroy the link in this room insinuated by cursor position, and destroy
   its link back partner in another room if applicable */
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



int is_mark_error(roomnum, x, y)
/* checks to see whether we are pointing to a correct lower right
   corner.  We return TRUE if we find an error condition, FALSE
   if we don't */
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



copy_into_paste_buffer(roomnum, spotx, spoty)
/* copy into the buffer contents of rectangle between the previously
   set mark and the given lower right corner coordinates */
int roomnum, spotx, spoty;
{
  OI *ptr;
  int i, j, x, y;
  char s[80];
  
  if (!is_mark_error(roomnum, spotx, spoty)) {
    destroy_all_objects_in_room(paste.map, 0);

    for (x=0,i=paste.x; i<=spotx; i++,x++)
      for (y=0,j=paste.y; j<=spoty; j++,y++) {
	ptr=first_obj_here(&mapstats, roomnum, i, j);
	for (; ptr; ptr=ptr->next) {
	  add_duplicate_to_square(paste.map, 0, x, y, ptr);
	}
      }

    /* set buffer size */
    paste.width = spotx - paste.x + 1;
    paste.height = spoty - paste.y + 1;

    /* mark may now be unset */
    paste.is_set = FALSE;

    /* tell user about what we just did */
    sprintf(s, "Stored %d squares in cut buffer", paste.width * paste.height);
    place_editor_message(s);
  }
}



put_out_buffer_contents(roomnum, x, y)
/* put out the buffer contents with the given coords as the upper left
   corner, check for empty buffer condition, if the buffer contents
   overflow the screen, then the extra chars are ignored */
int roomnum, x, y;
{
  OI *ptr;
  int i, j, count = 0;
  char s[80];
  
  if (paste.width == 0 || paste.height == 0)
    place_editor_message("Cut buffer is empty");
  else {

    for (i=0; i<paste.width; i++)
      for (j=0; j<paste.height; j++)
        if ( x+i < ROOM_WIDTH  &&  y+j < ROOM_HEIGHT) {
	  destroy_all_objects_on_square(&mapstats, roomnum, x+i, y+j);
	  ptr = first_obj_here(paste.map, 0, i, j);
	  for (; ptr; ptr=ptr->next) {
	    add_duplicate_to_square(&mapstats, roomnum, x+i, y+j, ptr);
	  }
	  if (roomnum == current_room)
	    draw_room_square_check_pile(x+i, y+j, FALSE);
	  count++;
	}

    sprintf(s, "%d squares pasted", count);
    place_editor_message(s);
  }
}



erase_area(roomnum, x, y, soft)
/* once the mark has been previously set, you may use this function to erase
   all the objects between the mark and the given lower right corner, if
   soft is TRUE, then only masked characters (ie. walls) in the squares
   will be erased */
int roomnum, x, y, soft;
{
  int	i, j, count = 0;
  char	s[80];

  if (!is_mark_error(roomnum, x, y)) {

    /* clear the area and draw it again if nessessary */
    for (i=paste.x; i<=x; i++)
      for (j=paste.y; j<=y; j++) {
        if (soft) clear_masked_in_square(i, j);
	else clear_square(i, j);

	/* draw the spot if it is on the screen */
	if (roomnum == current_room) draw_room_square_check_pile(i, j, FALSE);

	count++;
      }

    /* report how much was cleared */
    sprintf(s, "%d squares cleared", count);
    place_editor_message(s);

    /* take away the mark */
    paste.is_set = FALSE;
  }
}



fill_area(roomnum, x, y, square)
/* this command fills the area defined by mark and given lower
   right corner with copies of the objects on the given square. */
int roomnum, x, y;
SquareRec *square;
{
  int i, j, count = 0;
  char s[80];

  if (!is_mark_error(roomnum, x, y)) {

    /* place this object in each square */
    for (i=paste.x; i<=x; i++)
      for (j=paste.y; j<=y; j++) {
	  apply_square_using_mode(&mapstats, roomnum, i, j,
				  square, editor_mode);
	  if (current_room == roomnum)
	    draw_room_square_check_pile(i, j, FALSE);
	  count++;
      }

    sprintf(s, "%d squares filled", count);
    place_editor_message(s);

    /* unset the mark */
    paste.is_set = FALSE;
  }
}



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



/* ============================ I N F O requests ========================= */


put_down_obj_check_pile(x, y, n)
/* calls put_down_obj(), but also checks to see if changes should be made
   in pile window */
int x, y, n;
{
  put_down_obj(x, y, n);

  x = CONVERTRX(x);
  y = CONVERTRY(y);
  if (pile_content.pertinent && PILE_IN_ROOMWIN &&
      pile_content.roomnum == current_room &&
      pile_content.x == x && pile_content.y == y) {
    redraw_pile_window();
    draw_pile_box();
  }
}



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
  }
}



draw_pile_box_outline(win, x, y)
/* draws outline on screen used for showing which square is under display */
Window win;
int x, y;
{
  DrawObject(win, box_pix, box_mask, NULL, x, y);
}



draw_pile_box()
/* draws the pile display box whereever it's supposed to be */
{
  if (pile_content.pertinent) {
    if (PILE_IN_ROOMWIN && pile_content.roomnum == current_room) {
      draw_pile_box_outline(roomwin, PIXELX(pile_content.x),
			    PIXELY(pile_content.y));
    }
  }
}



set_pile_pointer(map, roomnum, x, y)
MapInfo *map;
int roomnum, x, y;
{
  SquareRec *square = get_square(map, roomnum, x, y);

  if (pile_content.pertinent && PILE_IN_ROOMWIN) {
    /* erase previous pile display box */
    draw_room_square(pile_content.x, pile_content.y, FALSE);
  }

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

  redraw_pile_window();
  draw_pile_box();
}



clear_pile_pointer()
{
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
}
