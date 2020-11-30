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
#include "bit/link"		/* bitmap of link box containing room number */
#include "bit/editmapicon"	/* icon bitmap */
#include "bit/scrollup"		/* scroll button */
#include "bit/scrolldown"	/* scroll button */
#include "bit/pageup"		/* scroll button */
#include "bit/pagedown"		/* scroll button */

#define EDITOR_WIN_X		30	/* x of main window, upper left */
#define EDITOR_WIN_Y		5
#define GRAB_WIN_WIDTH		8	/* # of objects wide grab window is */
#define GRAB_WIN_HEIGHT		25	/* # of objects high */
#define GRAB_WIN_TOPAREA	1	/* # of objects high top area is */
#define GRAB_SPACING		1	/* space (pixels) between objects */
#define STAT_WIN_WIDTH	ROOM_PIXELS_WIDE
#define STAT_WIN_HEIGHT	(GRAB_WIN_HEIGHT * (BITMAP_HEIGHT + GRAB_SPACING) \
			- ROOM_PIXELS_HIGH - WINDOW_BORDERS)

/* definitions for link box picture, amount of space for room number */
#define LINK_SPACE_HEIGHT	29
#define LINK_SPACE_WIDTH	(link_width - 4)
#define LINK_PIX_X	(MOUSE_PIX_X + (mouse_width - link_width) / 2)
#define LINK_PIX_Y	(MOUSE_PIX_Y + mouse_height / 2)

/* constants for placement of lines in editor status window */
#define STAT_LEFT_END	8	/* start of printing on left */
#define STAT_RIGHT_END	(MOUSE_PIX_X - 20)	/* right edge */
#define STAT_TOP_EDGE	6	/* top edge of printing */
#define STAT_REG_SPACE	16	/* space for a line of regular font */
#define STAT_BIG_SPACE	32	/* space for a line of large font */

/* constants for use by the room linking function */
#define DORMANT		0	/* no links in progress now */
#define EXPECTING	1	/* one side of link has been made */


/* constants used by information window */
#define INFO_LINES	5		/* num of lines in window */
#define INFO_COLS	30		/* num of columns window will hold */
#define INFO_FONT	(regfont)	/* font to use in help window */

/* constants used by help window */
#define HELP_X		(-WINDOW_BORDERS)
#define HELP_Y		(-WINDOW_BORDERS)
#define HELP_LINES	18		/* num of lines in window */
#define HELP_COLS	80		/* num of columns window will hold */
#define HELP_FONT	(regfont)	/* font to use in help window */

/* the informational record containing the location of a linking end */

typedef struct _link {
	short	room;		/* room where link end is */
	short	direction;	/* direction out of room link extends */
	short	x, y;		/* coords of possible link exit char */
} Link;

/* the informational record for storing the cut and paste pointer */

typedef struct _paste {
	short		is_set;		/* TRUE if mark is set */
	short		room;		/* room where mark was set */
	short		x, y;		/* point where mark was set */
	/* the information you store in the cut and paste buffer: */
	unsigned char	spot[ROOM_WIDTH][ROOM_HEIGHT][ROOM_DEPTH];
	short		width, height;	/* size of buffer contents */
} Paste;



/* global variables */
Window	editwin;	/* main parent window for editor windows */
Window	grabwin;	/* window where editor characters displayed */
Window	statwin;	/* window where editor stats are displayed */
Pixmap	link_pix;	/* room linking status picture */
Pixmap  scrollup, scrolldown, pageup, pagedown;  /* scrolling buttons */
unsigned char click_buffer[3];	/* three element object click buffer */
MemObj  *click_recd[3];		/* recorded info for click buffer objs */
int	link_status = DORMANT;	/* status of room linking function */
Link	link_info;		/* the particulars of the first link */
Paste	paste;		/* the cut and paste buffer information */
Help	*help_rec;	/* record containing info for help window */
Help	*info_rec;	/* record containing info for info window */
int	first_in_grab=0;/* object number of object at top left of grab win */

/* procedure declarations */

char *get_editor_input_line();
int redraw_a_window();


/* load the cursors for the editor program */

load_editor_cursors()
{
  define_thin_arrow_cursor(editwin);
  define_thin_arrow_cursor(roomwin);
  define_thin_arrow_cursor(grabwin);
  define_thin_arrow_cursor(statwin);
  define_thin_arrow_cursor(help_rec->win);
  define_thin_arrow_cursor(info_rec->win);
}



/* set up the windows needed by the map editor */

editor_window_setup()
{
  editwin = XCreateSimpleWindow(display, RootWindow(display, screen),
			  EDITOR_WIN_X, EDITOR_WIN_Y, ROOM_PIXELS_WIDE +
			  (GRAB_WIN_WIDTH * (BITMAP_WIDTH + GRAB_SPACING)) +
			  WINDOW_BORDERS, ROOM_PIXELS_HIGH + STAT_WIN_HEIGHT
			  + WINDOW_BORDERS, WINDOW_BORDERS, fgcolor,
			  bgcolor);
  
  /* set up name and icon for top level window */
  set_up_top_level_icon(editwin);

  roomwin = XCreateSimpleWindow(display, editwin, - WINDOW_BORDERS,
			  -WINDOW_BORDERS, ROOM_PIXELS_WIDE,
			  ROOM_PIXELS_HIGH, WINDOW_BORDERS, fgcolor,
			  bgcolor);

  grabwin = XCreateSimpleWindow(display, editwin, ROOM_PIXELS_WIDE,
			  -WINDOW_BORDERS, GRAB_WIN_WIDTH * (BITMAP_WIDTH +
			  GRAB_SPACING), GRAB_WIN_HEIGHT * (BITMAP_HEIGHT +
			  GRAB_SPACING), WINDOW_BORDERS, fgcolor, bgcolor);

  statwin = XCreateSimpleWindow(display, editwin, ROOM_PIXELS_WIDE -
			  STAT_WIN_WIDTH - WINDOW_BORDERS, ROOM_PIXELS_HIGH,
			  STAT_WIN_WIDTH, STAT_WIN_HEIGHT, WINDOW_BORDERS,
			  fgcolor, bgcolor);

  /* set up the help and information windows */ 
  info_rec = make_popup_help_window(editwin, RootWindow(display, screen),
		"information", 20, 20, INFO_COLS, INFO_LINES, INFO_FONT);
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



/* make the previously set up windows visible */

map_editor_windows()
{
  /* start up input in the windows */
  init_input_lines(editwin);
  init_input_lines(grabwin);
  init_input_lines(roomwin);
  init_input_lines(statwin);
  init_input_lines(help_rec->win);
  init_input_lines(info_rec->win);

  /* map the windows */
  XMapWindow(display, editwin);
  XMapWindow(display, roomwin);
  XMapWindow(display, grabwin);
  XMapWindow(display, statwin);
}



/* set up the variables about what is on the screen, and set the initial
   background of the room window */

editor_setup_variables()
{
  int i;
  
  /* pick starting room and set window background */
  current_room = 0;
  XSetWindowBackgroundPixmap(display, roomwin,
		      obj_pixmap[(unsigned char)room[current_room].floor]);
  XClearWindow(display, roomwin);

  /* clean out click buffer */
  for (i=0; i<3; i++) {
    click_buffer[i] = 0;
    click_recd[i] = NULL;
  }

  /* init paste buffer variables */
  paste.is_set = FALSE;
  paste.width = 0;
  paste.height = 0;
}



load_scroll_button_pixmaps()
{
  scrollup = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
			scrollup_bits, scrollup_width, scrollup_height,
			fgcolor, bgcolor, depth);
  scrolldown = XCreatePixmapFromBitmapData(display, RootWindow(display, screen),
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
  /* message area of previous garbage */
  place_editor_message(" ");
  
  if (isupper(c)) c = tolower(c);

  switch (c) {
	case '*':	DialogEditMapInfo();
			break;
	case '#':	DialogEditRoomInfo();
			break;
	case '%':	edit_recorded_object_variables(current_room,
					CONVERTRX(x), CONVERTRY(y));
			break;
	case 'q':	if (!want_query("Do you really want to quit?")) break;
			save_map();
			save_msg();
			return TRUE;
			break;
	case '!':	if (!want_query("Really quit without saving?")) break;
			return TRUE;
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
	case 'r':	if (what_win == grabwin)
  			  replace_in_room(obj_in_grab_win(x, y),
					  click_buffer[0]);
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
			else change_to_new_room();
			break;
	case 'g':	if (what_win == roomwin)
			  follow_cursor_to_next_room(CONVERTRX(x),
						     CONVERTRY(y));
			break;
	case 't':	enter_new_room_number();
			break;
	case 'm':	if (what_win == roomwin)
			  follow_cursor_and_make_room(CONVERTRX(x),
						      CONVERTRY(y));
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
	case 'a':	if (what_win == roomwin && link_status != DORMANT)
			  stop_linking();
			break;
	case 'k':	if (what_win == roomwin)
			  destroy_link(current_room, CONVERTRX(x),
				       CONVERTRY(y));
			break;
	case ')':	if (what_win == roomwin) clear_room();
			break;
	case 's':	save_map();
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
	case 'i':	handle_info_request(what_win, x, y, FALSE);
			break;
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
	case 'o':	toggle_owning_team();
			break;
	case 'l':	toggle_room_lighting();
			break;
	case '&':	change_num_of_teams();
			break;
	case '^':	toggle_neutrals_allowed();
			break;
	case 'h':
	case '?':	toggle_help_window(help_rec);
			break;
	default:	break;
  }
  
  return FALSE;
}



handle_editor_mouse(x, y, button, detail, what_window)
int x, y;
char button;
int  detail;
Window what_window;
{
  int cx, cy;

  if (what_window == grabwin) handle_grabwin_mouse(x, y, button, detail);
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

  
  else if (what_window == statwin) handle_statwin_mouse(x, y, button, detail);
  
}




/* handle the actions taken if mouse button was pressed in the grab window */
handle_grabwin_mouse(x, y, button, detail)
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
	  scroll_grab_window(1);
	  break;
	case 1:
	  scroll_grab_window(-1);
	  break;
	case 2:
	  scroll_grab_window((GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
	  break;
	case 3:
	  scroll_grab_window(-(GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA));
	  break;
	default:
	  break;
    }
  }
  else {
    /* put the object selected into the click buffer */
    switch (button) {
      case Button1:		click_buffer[0] = obj_in_grab_win(x, y);
				click_recd[0] = NULL;
				redraw_click_buffer(0);
				break;
      case Button2:		click_buffer[1] = obj_in_grab_win(x, y);
				click_recd[1] = NULL;
				redraw_click_buffer(1);
				break;
      case Button3:		click_buffer[2] = obj_in_grab_win(x,y);
				click_recd[2] = NULL;
				redraw_click_buffer(2);
      default:			break;
    }
  }
}



/* check to see where the mouse was pressed in the status window and
   from that determine what we should do about it */

handle_statwin_mouse(x, y, button, detail)
int x, y;
char button;
int detail;
{
  if (y > STAT_TOP_EDGE) {
    if (y < STAT_TOP_EDGE + STAT_REG_SPACE + STAT_BIG_SPACE)
	enter_new_map_name();
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE*2 + STAT_BIG_SPACE)
	enter_new_room_number();
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE*2 + STAT_BIG_SPACE*2)
	enter_new_room_name(current_room);
    else if (y < STAT_TOP_EDGE + STAT_REG_SPACE*3 + STAT_BIG_SPACE*3)
        enter_new_team_name(room[current_room].team);
    else return;
  }

  place_various_editor_names();
  place_editor_message(" ");
}



/* scroll the grab window the given number of lines, positive number
   for scrolling upward, negative number for scrolling downward */

scroll_grab_window(lines)
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



/* get a new name for the map */

enter_new_map_name()
{
  strcpy(mapstats.name,
	 get_editor_input_line("Map name: ", MAP_NAME_LENGTH - 1));
}


/* get a new name for some room and assign it */

enter_new_room_name(roomnum)
int roomnum;
{
  strcpy(room[roomnum].name,
	 get_editor_input_line("Room name: ", ROOM_NAME_LENGTH - 1));
}


/* get a new name for some team */

enter_new_team_name(teamnum)
int teamnum;
{
  /* check for team number out of range */
  if (teamnum < 1 || teamnum > NUM_OF_TEAMS) return;
  
  strcpy(mapstats.team_name[teamnum - 1],
	 get_editor_input_line("New team name: ", TEAM_NAME_LENGTH - 1));
}


/* get a new current room number and change to there */

enter_new_room_number()
{
  int newroom;
  
  newroom = atoi(get_editor_input_line("Room number: ", 10));

  if (newroom >= 0 && newroom < mapstats.rooms) change_rooms(newroom);
}



/* toggle the team that owns current room and redraw the ownership */

toggle_owning_team()
{
  room[current_room].team++;
  if (room[current_room].team > mapstats.teams_supported)
    room[current_room].team = 0;

  place_various_editor_names();
}



/* toggle the "is dark" variable for the current room, and display
   a message about its new status. */

toggle_room_lighting()
{
  if (room[current_room].dark == DAYLIT) {
    room[current_room].dark = LIT;
    place_editor_message("Room is now marked as lit.");
  }
  else if (room[current_room].dark == LIT) {
    room[current_room].dark = GLOBAL;
    place_editor_message("Room is now globally visible.");
  }
  else if (room[current_room].dark == GLOBAL) {
    room[current_room].dark = DARK;
    place_editor_message("Room is now marked as dark.");
  }
  else if (room[current_room].dark == DARK) {
    room[current_room].dark = DAYLIT;
    place_editor_message("Room is now marked as day lit.");
  }
}



/* change the number of teams in the game */

change_num_of_teams()
{
  char msg[80], *neu = "(and no neutrals)";

  /* add one to the number of teams */
  mapstats.teams_supported++;
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
  if (mapstats.neutrals_allowed) {
    mapstats.neutrals_allowed = FALSE;
    place_editor_message("Neutrals no longer allowed.");
  }
  else {
    mapstats.neutrals_allowed = TRUE;
    place_editor_message("Neutrals now allowed in this map.");
  }
}



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
  if (!objrec) {
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



edit_recorded_object_variables(roomnum, x, y)
/* look for a recorded object on the square and allow editing of the
   instance values saved in the recorded object record with a dialog box. */
int roomnum, x, y;
{
  MemObj *objrec;

  /* get hold of the recorded obj at this spot */
  objrec = what_recorded_obj_here(roomnum, x, y);

  /* if there is none here print message and return */
  if (!objrec) {
    place_editor_message("No recorded object there");
    return;
  }

  DialogEditRecObj(&(objrec->obj));
}



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

char *get_editor_input_line(prompt, maxlen)
char *prompt;
int maxlen;
{
  int exposed;
  
  /* clear the message area */
  clear_area(statwin, 8, STAT_WIN_HEIGHT - 32, MOUSE_PIX_X - 20,
	     STAT_WIN_HEIGHT);
	     
  return get_string(statwin, regfont, STAT_LEFT_END, STAT_WIN_HEIGHT - 32,
		    prompt, maxlen, &exposed);

}



/* ======================================================================= */



/* find out which object should be in the x y coords of the grab window,
   if they click on an object that doesn't exist, return 0 instead of the
   expected object number */

int obj_in_grab_win(x, y)
int x, y;
{
  int line, col, result;

  line = (y / (BITMAP_HEIGHT + GRAB_SPACING)) - GRAB_WIN_TOPAREA;
  col = x / (BITMAP_WIDTH + GRAB_SPACING);

  result =  first_in_grab + (line * GRAB_WIN_WIDTH) + col;
  if (result >= objects || result < 0) result = 0;

  return result;
}



/* ======================== O U T P U T =============================== */


/* redraw the indicated window for the editor */

redraw_a_window(win)
Window win;
{
  if (win == grabwin) redraw_grab_window(-1, -1);
  else if (win == statwin) redraw_stat_window();
  else if (win == roomwin) redraw_room_window(TRUE);
  else check_help_expose(win);
}




/* draw the contents of the object grabbing window, given any
   subset of the lines within to redraw (from 0 to 
   (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA - 1),
   -1 in either means to redraw the entire window. */

redraw_grab_window(first, last)
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



/* redraw the scroll buttons at top of grab window */

redraw_grab_scroll_buttons()
{
  char s[500];

  /* place scroll buttons there (if scrolling is possible) */
  if (objects > (GRAB_WIN_HEIGHT - GRAB_WIN_TOPAREA) * GRAB_WIN_WIDTH) {
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



/* redraw the stat window */

redraw_stat_window()
{
  redraw_mouse_click_buffer();

  redraw_link_picture();

  place_various_editor_names();

  /* redraw last message */
  place_editor_message("");
}



/* redraws the graphic representation of the mouse with the three click
   buffers inside */

redraw_mouse_click_buffer()
{
  int i;

  /* place the mouse picture pixmap */
  place_mouse_picture(statwin, MOUSE_PIX_X, MOUSE_PIX_Y);
  
  /* place each of the three click buffer images */
  for (i=0; i<3; i++)
    redraw_click_buffer(i);
}



/* draw the contents of one of the click buffer boxes */

redraw_click_buffer(num)
int num;
{
  /* place the object in the right box on the mouse picture */
  place_mouse_block(statwin, MOUSE_PIX_X, MOUSE_PIX_Y, num, click_buffer[num]);
}



/* draw the link picture and room number, this procedure checks for itself
   whether there is a current link or not */

redraw_link_picture()
{
  char s[10];
  
  if (link_status == DORMANT) {
    redraw_mouse_click_buffer();
    /* clear_area(statwin, LINK_PIX_X, LINK_PIX_Y, 
	       LINK_PIX_X + link_width, LINK_PIX_Y + link_height); */
  }
  else {
    XCopyArea(display, link_pix, statwin, mainGC, 0, 0, link_width,
	      link_height, LINK_PIX_X, LINK_PIX_Y);
    sprintf(s, "%d", link_info.room);
    text_center(statwin, bigfont, LINK_PIX_X + link_x_hot, LINK_PIX_X +
		link_x_hot + LINK_SPACE_WIDTH, LINK_PIX_Y + link_y_hot,
		LINK_PIX_Y + link_y_hot + LINK_SPACE_HEIGHT, s);
  }
}



/* place the various name associated with the editor program */

place_various_editor_names()
{
  char roomnum[20];

  /* name of map */
  text_write(statwin, regfont, STAT_LEFT_END, STAT_TOP_EDGE, 0, 0, "map name");
  place_map_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE+STAT_REG_SPACE,
		 STAT_RIGHT_END, STAT_TOP_EDGE+STAT_REG_SPACE+STAT_BIG_SPACE);

  /* name of room */
  sprintf(roomnum, "room %d   ", current_room);
  text_write(statwin, regfont, STAT_LEFT_END, STAT_TOP_EDGE+STAT_REG_SPACE+
	     STAT_BIG_SPACE, 0, 0, roomnum);
  place_room_name(statwin, STAT_LEFT_END, STAT_TOP_EDGE+(STAT_REG_SPACE*2)+
		  STAT_BIG_SPACE, STAT_RIGHT_END, STAT_TOP_EDGE+
		  (STAT_REG_SPACE*2)+(STAT_BIG_SPACE*2));

  /* team for which this is a "home room" */
  place_room_team(current_room, statwin, STAT_LEFT_END, STAT_TOP_EDGE+
		  (STAT_REG_SPACE*3)+(STAT_BIG_SPACE*2), STAT_RIGHT_END,
		  STAT_TOP_EDGE+(STAT_REG_SPACE*3)+(STAT_BIG_SPACE*3));
}



/* place a message in the stat window in the appropriate font.  If a null
   length message is given as the message, then redraw the last message */

place_editor_message(line)
char *line;
{
  static char	current_msg[200] = "";

  if (strlen(line) != 0) strcpy(current_msg, line);

  clear_area(statwin, 8, STAT_WIN_HEIGHT - 32, MOUSE_PIX_X - 20,
	     STAT_WIN_HEIGHT);
  place_a_string(statwin, current_msg, 8, STAT_WIN_HEIGHT - 32,
		 MOUSE_PIX_X - 20, STAT_WIN_HEIGHT);
}


/* place message about the fact that map was just saved on the screen */

save_msg()
{
  char msg[250];
  
  /* let person using editor know that we've saved */
  sprintf(msg, "Saved %d rooms in %s\n", mapstats.rooms, mapfile);
  place_editor_message(msg);
}



/* editor wants to change to a different room */

change_rooms(newroom)
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
      redraw_room_window(FALSE);
      place_various_editor_names();
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



/* add a room and change to it */

change_to_new_room()
{
  add_new_room();
  change_rooms(mapstats.rooms - 1);
}



/* set the floor character for the entire room, if set flag is TRUE
   then actually replace the current floor character on all spaces */

set_room_floor(objnum, set)
int objnum, set;
{
  register int i, j;

  /* if the object is a recorded one then this is not a good idea */
  if (info[(uc)objnum]->recorded) {
    place_editor_message("no recorded objects for floors");
    return;
  }

  /* place floor character on map */
  if (set) {
    for (j=0; j<ROOM_HEIGHT; j++)
      for (i=0; i<ROOM_WIDTH; i++) {
        put_down_mapped_obj(i, j, (unsigned char) objnum);
        draw_room_square(i, j, FALSE);
      }
  }
      
  /* place floor character in room record */
  room[current_room].floor = objnum;

  /* change the background of the window to reflect new floor */
  XSetWindowBackgroundPixmap(display, roomwin, 
			     obj_pixmap[room[current_room].floor]);
  place_editor_message("official floor character set");
}



/* replaces all instances of a certain object in a room with another obj,
   only works for mapped characters, not recorded objects */

replace_in_room(victim, replacement)
unsigned char victim, replacement;
{
  register int i, j, k;

  /* check to see if victim or replacement is a recorded object */
  if (info[(uc)victim]->recorded || info[(uc)replacement]->recorded) {
    place_editor_message("no replacing recorded objects");
    return;
  }

  for (j=0; j<ROOM_HEIGHT; j++)
    for (i=0; i<ROOM_WIDTH; i++) {
      for (k=0; k<ROOM_DEPTH; k++)
        if (room[current_room].spot[i][j][k] == victim)
	  room[current_room].spot[i][j][k] = replacement;
      draw_room_square(i, j, FALSE);
    }
}



/* ===================== C U R S O R   F O L L O W ======================= */

/* follow the direction or exit character that the cursor points us to,
   it is assumed that this was called while cursor was in roomwin, we
   are given the character x and y coords, we will change rooms to the
   appropriate room */

follow_cursor_to_next_room(x, y)
int x, y;
{
  int target_room, dir, start_room;
  MemObj *exit;
  
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
    exit = what_recorded_obj_here(start_room, x, y);
    warp_mouse_through_exit(exit);
  }
}


/* this is like follow_cursor_to_next_room, but is meant to be used when
   there is no such room yet, and it must be created before we can go
   into it for editing purposes.  Make it and change into it. */

follow_cursor_and_make_room(x, y)
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
  /* other wise link the directions between the two new rooms */
  else {
    link_one_room_to_another(current_room, dir, mapstats.rooms-1);
    copy_edge_of_room(current_room, dir, mapstats.rooms-1);
  }

  /* now change to that room */
  change_rooms(mapstats.rooms-1);
}



/* start up a linkage in the current room given a particular x and y */

startup_linkage(x, y)
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



/* complete a link started previously, this will be a one way link from one
   room (or room's exit char) to another room */

finalize_link(dest_room, x, y)
int dest_room, x, y;
{
  MemObj *exitchar;
  
  link_status = DORMANT;
  redraw_link_picture();

  /* if the direction from the previous room was a compass direction, then
     we alter the exit flag for the previous room */
  if (link_info.direction >= 0) {
    room[link_info.room].exit[link_info.direction] = dest_room;
    return;
  }

  /* otherwise, it was an exit character, and we need to find out which one */
  exitchar = what_recorded_obj_here(link_info.room, link_info.x, link_info.y);
  if (exitchar == NULL) {
    place_editor_message("Bad start of link - no link made");
    return;
  }

  /* place into that char the required info */
  exitchar->obj.detail = dest_room;
  exitchar->obj.infox = x;
  exitchar->obj.infoy = y;
}



/* complete a link started previously, this will be a TWO way link from one
   room (or room's exit char) to another room and back.  For exit chars,
   if the second room has another exit char at the link point, then this
   will work, if not, only a one way link will be created. */

finalize_two_way_link(dest_room, x, y)
int dest_room, x, y;
{
  MemObj *exitchar, *destchar;
  int	 dir;
  
  link_status = DORMANT;
  redraw_link_picture();

  /* if the direction from the previous room was a compass direction, then
     we link the two rooms easily enough */
  if (link_info.direction >= 0) {
    link_one_room_to_another(link_info.room, link_info.direction, dest_room);
    return;
  }

  /* otherwise, it was an exit character, and we need to find out which one */
  exitchar = what_recorded_obj_here(link_info.room, link_info.x, link_info.y);
  if (exitchar == NULL) {
    place_editor_message("Bad start of link - no link made");
    return;
  }

  /* look for an exit char in the destination square, if there is one
     then we scold the user for trying to make an infinite loop */
  destchar = what_recorded_obj_here(dest_room, x, y);
  if (destchar != NULL)
    if (info[(uc)destchar->obj.objtype]->exit) {
    place_editor_message("No making infinite loops");
    return;
  }

  /* place into that char the required info */
  exitchar->obj.detail = dest_room;
  exitchar->obj.infox = x;
  exitchar->obj.infoy = y;

  /* look for nearby exit character */
  destchar = nearby_exit_character(dest_room, x, y, &dir);

  /* if no exit character nearby make one way link only */
  if (destchar == NULL) {
    place_editor_message("No nearby exit - one way link only");
    return;
  }

  /* there was an exit char nearby, lets link */
  destchar->obj.detail = link_info.room;
  destchar->obj.infox = link_info.x + xdir[dir];
  destchar->obj.infoy = link_info.y + ydir[dir];
}



/* stop a link that is in progress */

stop_linking()
{
  link_status = DORMANT;
  place_editor_message("Linkage aborted");
  redraw_link_picture();
}



/* destroy the link in this room insinuated by cursor position, and destroy
   its link back partner in another room if applicable */

destroy_link(roomnum, x, y)
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



/* checks to see whether we are pointing to a correct lower right
   corner.  We return TRUE if we find an error condition, FALSE
   if we don't */

int is_mark_error(roomnum, x, y)
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



/* copy into the buffer contents of rectangle between the previously
   set mark and the given lower right corner coordinates */

copy_into_paste_buffer(roomnum, spotx, spoty)
int roomnum, spotx, spoty;
{
  int	i, j, k, x, y, z;
  char	s[80];
  
  if (!is_mark_error(roomnum, spotx, spoty)) {

    /* copy room contents into buffer */
    for (x=0,i=paste.x;   i<=spotx;     i++,x++)
      for (y=0,j=paste.y; j<=spoty;       j++,y++)
        for (z=0,k=0;     k<ROOM_DEPTH; k++,z++)
	  paste.spot[x][y][z] = room[roomnum].spot[i][j][k];

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



/* put out the buffer contents with the given coords as the upper left
   corner, check for empty buffer condition, if the buffer contents
   overflow the screen, then the extra chars are ignored */

put_out_buffer_contents(roomnum, x, y)
int roomnum, x, y;
{
  int i, j, k, count = 0;
  char s[80];
  
  if (paste.width == 0 || paste.height == 0)
    place_editor_message("Cut buffer is empty");
  else {

    /* place chars straight into room record */
    for (i=0; i<paste.width; i++)
      for (j=0; j<paste.height; j++)
        if ( x+i < ROOM_WIDTH  &&  y+j < ROOM_HEIGHT) {
	  for (k=0; k<ROOM_DEPTH; k++) {
	    room[roomnum].spot[x+i][y+j][k] = paste.spot[i][j][k];
	    if (roomnum == current_room)
	      draw_room_square(x+i, y+j, FALSE);
	  }
	  count++;
	}

    /* tell user we are done */
    sprintf(s, "%d squares pasted", count);
    place_editor_message(s);
  }
  
}



/* once the mark has been previously set, you may use this function to erase
   all the objects between the mark and the given lower right corner, if
   soft is TRUE, then only masked characters (ie. walls) in the squares
   will be erased */

erase_area(roomnum, x, y, soft)
int roomnum, x, y, soft;
{
  int	i, j, k, count = 0;
  char	s[80];

  if (!is_mark_error(roomnum, x, y)) {

    /* clear the area and draw it again if nessessary */
    for (i=paste.x; i<=x; i++)
      for (j=paste.y; j<=y; j++) {
        if (soft) clear_masked_in_square(i, j);
	else clear_square(i, j);

	/* draw the spot if it is on the screen */
	if (roomnum == current_room) draw_room_square(i, j, FALSE);

	count++;
      }

    /* report how much was cleared */
    sprintf(s, "%d squares cleared", count);
    place_editor_message(s);

    /* take away the mark */
    paste.is_set = FALSE;
  }
	  
}




/* this command fills the area defined by mark and given lower
   right corner with the given object type (it adds the object to
   the square */

fill_area(roomnum, x, y, objtype)
int roomnum, x, y, objtype;
{
  int i, j, count = 0;
  char s[80];

  if (!is_mark_error(roomnum, x, y)) {

    /* place this object in each square */
    for (i=paste.x; i<=x; i++)
      for (j=paste.y; j<=y; j++) {
          if (info[(uc)objtype]->recorded) put_down_recorded_obj(i, j, objtype);
	  else put_down_mapped_obj(i, j, objtype);

	  if (current_room == roomnum) draw_room_square(i, j, FALSE);
	  count++;
      }

    sprintf(s, "%d squares filled", count);
    place_editor_message(s);

    /* unset the mark */
    paste.is_set = FALSE;
  }
}



/* ============================ I N F O requests ========================= */

/* handle a request for information on a square, results depend on which
   window it is called from.  If there is already an info window up, this
   command puts it away, if they pressed 'e' instead of 'i', they want
   special info and the special variable should be passes to this procedure
   as TRUE */

handle_info_request(win, x, y, special)
Window win;
int x, y, special;
{
  int newx, newy;	/* where in RootWindow to place the info window */
  int wwide, whigh;	/* size of info window */
  Window dummy1, dummy2;
  unsigned int dummykeys;
  int dummyx, dummyy;

  /* if the window is already up, then remove it */
  if (info_rec->is_up) {
    hide_help_window(info_rec);
    return;
  }
  else {

    /* determine new spot for info window, find out where mouse is in
       the RootWindow and use that for the upper left corner, unless
       that would place part of it off screen, if so then pull it back so
       that the entire window will be visible */
       
    wwide = info_rec->width * char_width(info_rec->fi);
    whigh = info_rec->height * char_height(info_rec->fi);

    XQueryPointer(display, RootWindow(display, screen), &dummy1, &dummy2,
		  &newx, &newy, &dummyx, &dummyy, &dummykeys);

    if (newx + wwide > DisplayWidth(display, screen))
      newx = DisplayWidth(display, screen) - wwide - WINDOW_BORDERS * 2;
    if (newy + whigh > DisplayHeight(display, screen))
      newy = DisplayHeight(display, screen) - whigh - WINDOW_BORDERS * 2;
    
    move_help_window(info_rec, newx, newy);
  

    /* get the right information placed into the window */
    clear_help_window(info_rec);
    if (win == roomwin) {
      if (special) recorded_square_info(info_rec, current_room,
					CONVERTRX(x), CONVERTRY(y));
      else map_square_info(info_rec, current_room, CONVERTRX(x), CONVERTRY(y));
    }
    else if (win == grabwin)
      get_object_info(info_rec, obj_in_grab_win(x, y));
    else if (win == statwin)
      get_status_info(info_rec);
    else return;

    /* now make the window appear */
    show_help_window(info_rec);
  }
}



/* get the information about the contents of a certain square on the map */

map_square_info(rec, roomnum, x, y)
Help *rec;
int  roomnum, x, y;
{
  int i;
  char s[200];
  unsigned char *spot;
  
  /* find out what is on this square */
  spot = whats_on_square(roomnum, x, y);

  /* first line is roomnum and location */
  sprintf(s, "  Room %d    x:%d    y:%d", roomnum, x, y);
  set_line(rec, 0, s);

  /* other lines list objects names and numbers */
  for (i=0; i < ROOM_DEPTH + 1; i++) {
    sprintf(s, "  #%3d %s", spot[i], info[(uc)spot[i]]->name);
    set_line(rec, i + 1, s);
  }
}



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

get_object_info(rec, objnum)
Help *rec;
int  objnum;
{
  char s[200];
  
  /* check to make sure this is one of the defined objects */
  if (objnum >= objects) {
    set_line(rec, 0, "  undefined object");
    return;
  }

  /* first line is number and name of object */
  sprintf(s, "  #%3d %s", objnum, info[(uc)objnum]->name);
  set_line(rec, 0, s);

  /* set up the following lines to contain the flag text for the obj */
  set_lines_long_text(rec, 1, object_info_line(info[(uc)objnum]));
}


/* get the information about the map in general and place in the
   info help window */

get_status_info(rec)
Help *rec;
{
  char s[80];
  
  /* In this window we will want to place some relevant information
     about the current map */
  sprintf(s, "  %s", mapstats.name);
  set_line(rec, 0, s);
  sprintf(s, "  %d rooms in this map", mapstats.rooms);
  set_line(rec, 1, s);
  sprintf(s, "  %d team(s) supported", mapstats.teams_supported);
  set_line(rec, 2, s);
  if (mapstats.neutrals_allowed)
    sprintf(s, "  neutrals are allowed");
  else
    sprintf(s, "  neutrals not allowed");
  set_line(rec, 3, s);
  
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
}
