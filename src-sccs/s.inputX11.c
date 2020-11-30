h06128
s 00000/00000/00684
d D 1.7 92/08/07 01:01:50 vbo 7 6
c source copied to a separate tree for work on new map and object format
e
s 00003/00002/00681
d D 1.6 92/07/31 17:40:34 vbo 6 5
c corrected a state problem with keypress input
e
s 00003/00002/00680
d D 1.5 91/11/29 16:09:09 labc-4lc 5 4
c finished making entry of hidden text properties in obtor
e
s 00120/00000/00562
d D 1.4 91/11/28 00:10:26 labc-4lc 4 3
c nearly finished hidden text items in dialogs
e
s 00010/00005/00552
d D 1.3 91/05/26 22:44:45 labc-3id 3 2
c 
e
s 00004/00000/00553
d D 1.2 91/05/10 04:50:30 labc-3id 2 1
c eliminates extra key presses now
e
s 00553/00000/00000
d D 1.1 91/02/16 12:54:24 labc-3id 1 0
c date and time created 91/02/16 12:54:24 by labc-3id
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

/* Input Routines for window system */

#include "def.h"
#include "windowsX11.h"


/*****************************************************************/

/* procedure to call to redraw some window or other */
int (*GlobalRedraw)() = NULL;

/*****************************************************************/

/* This routine looks for keypresses and ignores any other kind
   of input, including window exposures.  It returns the keypress found
   in the variable keyp.  The function returns a value of 1 if a keypress
   is found or a zero if there are no keypresses at the moment.
   This routine assumes you have the right display selected and that
   you have previously used XSelectInput to pay attention to keypresses. */

int get_key_press(what_win, keyp)
Window what_win;
char	*keyp;
{
  int x, y, type, detail;
  Window happenwin;

  type = get_input(what_win, keyp, &x, &y, &detail, &happenwin);
  if (type!=KEYBOARD) return 0;
  else return 1;
}

/* This routine looks for keypresses and ignores any other kind
   of input, including window exposures.  It returns the keypress found
   in the variable keyp.  The function returns a value of 1 if a keypress
   is found or a zero if there are no keypresses at the moment.
   This routine assumes you have the right display selected and that
   you have previously used XSelectInput to pay attention to keypresses. */

int get_help_key_press(help_rec, keyp)
Help *help_rec;
char	*keyp;
{
  int x, y, type, detail;
  Window happenwin;

  type = get_input(help_rec->win, keyp, &x, &y, &detail, &happenwin);
  RedrawGlobaly(&last_event);

  if (type!=KEYBOARD) return 0;
  else return 1;
}

/* This routine is in charge of getting an entire string.  If an
   exposure event occurs while it is getting the string, it will
   redraw the things it knows about (the input line) and set the
   exposure flag.  The correct display and input selection is 
   assumed. This routine will echo the characters typed at the 
   given location. */

char *get_string(what_win, fi, xstart, y, prompt, maxlen, beenexposed)
Window 	what_win;
XFontStruct *fi;
int	xstart, y;
char    *prompt;
int     maxlen, *beenexposed;
{
  int x, promptlen, detail, i = 0;
  int type, mousex, mousey, done = 0;
  char c, s[2];
  static char line[TEXTINPLEN];
  Window *happenwin;

  *beenexposed = 0;

  /* write out prompt and set x to be right after the prompt */
  promptlen = text_width(fi, prompt);
  text_write(what_win, fi, xstart, y, 0, 0, prompt);
  x = xstart + promptlen;

  /* write out the initial underline char as a cursor */
  text_write(what_win, fi, x, y, i, 0, "_");
  
  do {

    /* Look for input until some is found */
    do {
      type = get_input(what_win, &c, &mousex, &mousey, &detail, &happenwin);
    } while (type==NOTHING);

    /* Now act according to the input received */
    switch (type) {

        case KEYBOARD:
	    if (c=='\n' || c=='\r') {
	    
	    /* Erase underline char and quit on return key */
	    done = 1;
	    text_erase(what_win, fi, x, y, i, 0, 1);
	    }

	    else if (c=='\b' || c==127 || c==8) {

	      /* Erase chars when backspacing */
              text_erase(what_win, fi, x, y, i + 1, 0, 1);
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      if (i>0) i--;
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      text_write(what_win, fi, x, y, i, 0, "_");

	    }
	    else {

	      /* Add and write chars when they are received */
	      s[0] = c;
	      s[1] = '\0';
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      text_write(what_win, fi, x, y, i, 0, s);
	      line[i++] = c;
	      if (i>=maxlen) done = 1;
	      else text_write(what_win, fi, x, y, i, 0, "_");
	    }
	    break;

	case MOUSE:
	    break;

	case EXPOSED:
	    text_write(what_win, fi, xstart, y, 0, 0, prompt);
	    line[i] = '\0';
	    text_write(what_win, fi, x, y, 0, 0, line);
	    text_write(what_win, fi, x, y, i, 0, "_");
	    *beenexposed = 1;
	    break;

	default:
	    break;
    }
    
  } while (!done);

  /* Adjust the end of the string (null character) and return */
  line[i] = '\0';
  return line;
}



/* The object of this routine is to get the next input item from
   the queue for a given window.  The routine returns KEYBOARD or
   MOUSE, depending on where the input is coming form and places
   the pertinent values in keyp, mx, my, and detail.  For mouse button
   presses, keyp will contain Button1, Button2, or Button3, and detail
   will contain state of control keys.  Routine assumes that XSelectInput has
   been previously set properly.  If no input is forthcoming the
   value NOTHING is returned.  If a window exposure event is
   received then EXPOSED is returned.
   Events are saved in last_event variable for later examination. */

get_input(what_win, keyp, mx, my, detail, where_happened)
Window what_win;
char   *keyp;
int    *mx, *my, *detail;
Window *where_happened;
{
  XEvent 	evt;
  KeySym	keysym;
  char   	buf[20];
  int	 	nchar, found = 0;
I 3
  static int count = 0;
E 3

  *where_happened = 0;
  last_event.type = 0;

D 3
  /* Check how many input events are on the queue */
  found = XPending(display);
  if (!found) return NOTHING;
E 3
I 3
  do
    {
      /* Check how many input events are on the queue */
      found = XPending(display);
      if (!found) return NOTHING;
E 3

D 3
  /* get the event at head of queue and the window it happened in */
  XNextEvent(display, &evt);
E 3
I 3
      /* get the event at head of queue and the window it happened in */
      XNextEvent(display, &evt);
    }
  while(evt.type == NoExpose);
E 3
I 2

  if (evt.type == KeyPress && found > 10)
    while(XCheckMaskEvent(display,KeyPressMask,&evt));

E 2
  *where_happened = evt.xany.window;
  bcopy(&evt, &last_event, sizeof(XEvent));	/* save this as last event */

  /* act according to the type of the event */
  switch (evt.type) {
      case KeyPress:
D 6
	  *mx = evt.xkey.x;
	  *my = evt.xkey.y;
E 6
	  nchar = XLookupString((XKeyEvent *) &evt, buf, 19, &keysym, 
				(XComposeStatus *) NULL);
          if (nchar == 1) {
            *keyp = buf[0];
I 6
	    *mx = evt.xkey.x;
	    *my = evt.xkey.y;
	    *detail = evt.xkey.state;
E 6
            return KEYBOARD;
	  }
          else
            return NOTHING;
	  break;

      case ButtonPress:
	  *mx = evt.xbutton.x;
	  *my = evt.xbutton.y;
	  *keyp = (char) evt.xbutton.button;
	  *detail = evt.xbutton.state;
	  return MOUSE;
	  break;

      case Expose:
      case GraphicsExpose:
	  RedrawGlobaly(&evt);
          return EXPOSED;
	  break;

      case EnterNotify:
          return ENTER;
	  break;

      case MappingNotify:
	  XRefreshKeyboardMapping((XMappingEvent *)&evt);
	  return NOTHING;
	  break;

      default:
          return NOTHING;
          break;
  }
}



/* X event request initialization */

init_input_lines(win)
Window win;
{
  XSelectInput(display, win, KeyPressMask | ButtonPressMask | ExposureMask );
}

input_and_catch_mouse_entering(win)
Window win;
{
  XSelectInput(display, win, EnterWindowMask | KeyPressMask |
	       ButtonPressMask | ExposureMask );
}

close_input_lines(win)
Window win;
{
  XSelectInput(display, win, NoEventMask);
}


  

/* ask the user if he wants to do something, wait for a keypress in any
   window and return TRUE if he presses 'y' or 'Y'.  Return FALSE for
   any other repsonse. */

int want_query(s)
char *s;
{
  int  pixel_width, pixel_height;
  Help *help_window;
  char c;

  /* figure out how big window has to be */
  pixel_width = char_width(bigfont) * strlen(s) + WINDOW_BORDERS*2;
  pixel_height = char_height(bigfont) + WINDOW_BORDERS*2;

  /* make a help window to display message in and get it displayed */
  help_window = make_help_window(roomwin, (ROOM_PIXELS_WIDE - pixel_width)/2,
				 (ROOM_PIXELS_HIGH - pixel_height)/2,
				 strlen(s), 1, bigfont);
  set_line(help_window, 0, s);
  show_help_window(help_window);
  init_input_lines(help_window->win);

  /* wait for a keypress */
  while (!get_help_key_press(help_window, &c));

  /* make the help window go away */
  destroy_help_window(help_window);
  
  /* return TRUE or FALSE depending on key press */
  if (c=='y' || c=='Y') return TRUE;
  else return FALSE;
}

/* ask the user if he wants to do something, wait for a keypress in any
   window and return TRUE if he presses 'y' or 'Y'.  Return FALSE for
   any other repsonse. This one differs from the above in that you must
   supply a window and box coordinate corners to center text in. */

int placed_want_query(win, x1, y1, x2, y2, s)
Window win;
int x1, y1, x2, y2;
char *s;
{
  int  pixel_width, pixel_height;
  Help *help_window;
  char c;

  /* figure out how big window has to be */
  pixel_width = char_width(bigfont) * strlen(s) + WINDOW_BORDERS*2;
  pixel_height = char_height(bigfont) + WINDOW_BORDERS*2;

  /* make a help window to display message in and get it displayed */
  help_window = make_help_window(win, x1 + ((x2 - x1 - pixel_width)/2),
				 y1 + ((y2 - y1 - pixel_height)/2),
				 strlen(s), 1, bigfont);
  set_line(help_window, 0, s);
  show_help_window(help_window);
  init_input_lines(help_window->win);

  /* wait for a keypress */
  while (!get_help_key_press(help_window, &c));

  /* make the help window go away */
  destroy_help_window(help_window);
  
  /* return TRUE or FALSE depending on key press */
  if (c=='y' || c=='Y') return TRUE;
  else return FALSE;
}

/* ask the user if he wants to do something, wait for a keypress in any
   window and return the letter he/she pressed, that is in the press-set */

int placed_letter_query(win, x1, y1, x2, y2, s, letters)
Window win;
int x1, y1, x2, y2;
char *s,*letters;
{
  int  pixel_width, pixel_height;
  Help *help_window;
  char c;
  int letter,i;

  /* figure out how big window has to be */
  pixel_width = char_width(bigfont) * maxstrlen(s) + WINDOW_BORDERS*2;
  pixel_height = char_height(bigfont) + WINDOW_BORDERS*2;

  /* make a help window to display message in and get it displayed */
  help_window = make_help_window(win, x1 + ((x2 - x1 - pixel_width)/2),
				 y1 + ((y2 - y1 - pixel_height)/2),
				 maxstrlen(s), count_lines(s), bigfont);
  set_lines(help_window,s);
  show_help_window(help_window);
  init_input_lines(help_window->win);

  letter = -1;
  do
    {
      /* wait for a keypress */
      while (!get_help_key_press(help_window, &c));
      if (isupper(c)) c = tolower(c);

      for (i = 0;letters[i] != '\0';i++)
	if (c == letters[i]) 
	  letter = c;
    }
  while(letter == -1);

  /* make the help window go away */
  destroy_help_window(help_window);
  
  return(letter);
}
/*****************************************************************/

char *get_string_shift(what_win, xstart, y, prompt, width)
Window  what_win;
int	xstart, y;
char    *prompt;
int width;
{
  XFontStruct *fi;
  int x, promptlen, detail, i = 0,offset;
  int type, mousex, mousey, done = 0;
  int maxx,rightside;
  char c, s[2];
  static char line[TEXTINPLEN];
  Window *happenwin;

  fi = regfont;
  maxx = -1;
  line[0] = '\0';
  promptlen = text_width(fi, prompt);
  x = promptlen + xstart;
  while ((x > width)&&(width > 0))
    {
      offset = text_width(fi,"12345");
      xstart -= offset;
      x -= offset;
    }
  text_write(what_win, fi, xstart, y, 0, 0, prompt);
  text_write(what_win, fi, x, y, 0, 0, "_");
  maxx = (x + text_width(fi,line) + text_width(fi,"_   "));

  do
    {
      /* Look for input until some is found */
      do
	{
	  type =
	    get_input(what_win, &c, &mousex, &mousey, &detail, &happenwin);
	} while (type==NOTHING);

    /* Now act according to the input received */
      switch (type)
	{
	case KEYBOARD:
	  if (c=='\n' || c=='\r') 
	    {
	      /* Erase underline char and quit on return key */
	      done = 1;
	      text_erase(what_win, fi, x, y, i, 0, 1);
	    }
	  else if (c=='\b' || c==127 || c==8) 
	    {
	      /* Erase chars when backspacing */
	      text_erase(what_win, fi, x, y, i + 1, 0, 1);
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      if (i>0) i--;
	      line[i] = '\0';
	      text_erase(what_win, fi, x, y, i, 0, 1);
	      text_write(what_win, fi, x, y, i, 0, "_");
	    }
	  else 
	    {
	      /* Add and write chars when they are received */
	      if (i < TEXTINPLEN-2) 
		{
		  s[0] = c;
		  s[1] = '\0';

		  if (((text_width(fi,line)+x+text_width(fi,"_ ")) > width)&&
		      (width > 0))
		    {
		      text_erase(what_win,fi,x,y,i-5,0,5);
		      offset = text_width(fi,"12345");
		      xstart -= offset;
		      x -= offset;
		      text_write(what_win,fi,xstart,y,0,0,prompt);
		      text_write(what_win,fi,x,y,0,0,line);
		    }

		  text_erase(what_win, fi, x, y, i, 0, 1);
		  text_write(what_win, fi, x, y, i, 0, s);
		  line[i++] = c;
		  line[i] = '\0';
		  text_write(what_win, fi, x, y, i, 0, "_");

		  rightside = (x + text_width(fi,line) + text_width(fi,"_ "));
		  if (rightside > maxx)
		    maxx= rightside;
		}
	    }
	  break;
	case EXPOSED:
	  text_write(what_win, fi, xstart, y, 0, 0, prompt);
	  text_write(what_win, fi, x, y, 0, 0, line);
	  text_write(what_win, fi, x, y, i, 0, "_");
	  break;
	default:
	  break;
	}
    } while (!done);
  
  XClearArea(display,what_win,xstart,y,maxx-xstart,text_height(fi),True);
  return line;
}

/*****************************************************************/

I 4
/* Opens a window and displays a multiple-line string in it.  The parent
   window is given, along with whether to center the new child window
   in the parent window.  If so, then x and y are the width and height of
   the parent window rather than the location to place the upper left corner
   of the child window.  'Initial' is the string to be displayed in the
   window, and edit is to be TRUE if the user is allowed to edit the text.
   Editing will be done either on a one-liner basis or by calling up the
   favorite editor.  Routine returns a pointer to the new string entered.
   Returns NULL if there is no change to the string.  Caller must not
   free the string returned, and should copy it before calling this
   routine again.  Returns length zero string when user indicates desire
   to erase all text stored in the string. */

char *display_edit_string(win, center, x, y, prompt, initial, edit, fi, width)
Window win;
int center, x, y;
char *prompt, *initial;
int edit;
XFontStruct *fi;
int width;   /* maximum width (in characters) of the window */
{
  static char *freelater = NULL;
  char c, *linebuf, *ptr, *result = NULL;
  Help *help_window;
  int i, current, done = 0, lines = 1, longest = 10;
  demand((linebuf = (char *) malloc(sizeof(char) * (width + 80))),
	 "no memory for line buffer");
  if (freelater) {
    free(freelater);
    freelater = NULL;
  }

  /* figure out what the longest line is, and count lines */
  if (prompt) longest = max(longest, strlen(prompt) + 10);
  if (initial) {
    current = 0;
    for (ptr=initial; !done; ptr++) {
      if ((*ptr == '\n') || (*ptr == '\0') || (current >= width)) {
        if (current > longest) longest = current;
        lines++;
        current = 0;
        if (*ptr == '\0') done = TRUE;
      }
      else current++;
    }
  }

  /* center the window if necessary */
  if (center) {
    int pixel_width, pixel_height;

    /* figure out how big window has to be */
    pixel_width = char_width(regfont) * longest + WINDOW_BORDERS*2;
    pixel_height = char_height(regfont) * lines + WINDOW_BORDERS*2;

    x = (x - pixel_width) / 2;
    y = (y - pixel_width) / 2;
  }

  /* create help window */
  help_window = make_help_window(win, x, y, longest, lines, fi);

  if (prompt) set_line(help_window, 0, prompt);
  if (initial && strlen(initial)) {
    int lcount = 1;
    current = 0;
    done = 0;
    for (ptr=initial; !done; ptr++) {
      if ((*ptr == '\0') || (*ptr == '\n') || (current >= width)) {
	linebuf[current] = '\0';
	set_line(help_window, lcount, linebuf);
	lcount++;
	current = 0;
        if ((*ptr != '\n') && (*ptr != '\0'))
	  linebuf[current++] = *ptr;
        if (*ptr == '\0') done = TRUE;
      }
      else {
        linebuf[current++] = *ptr;
      }
    }
  }
  init_input_lines(help_window->win);
  show_help_window(help_window);
  done = 0;
  do {
    while (!get_help_key_press(help_window, &c));
    switch (c) {
	case '\b':
	  result = "";
	  done = TRUE;
	  break;
	case '\r':
	case '\n':
	  done = TRUE;
	  break;
	case 'e':
	case 'E':
          if (edit) {
D 5
	    result = (char *) call_up_editor(initial);
	    freelater = result;
E 5
I 5
	    result = call_up_editor(initial);
	    if (result == initial) result = NULL;
	    else freelater = result;
E 5
	  }
	  done = TRUE;
	  break;
	case ' ':
	  if (edit)
  	    result = get_string_shift(help_window->win, 0, 0, prompt, width);
	  done = TRUE;
	  break;
	default:
	  break;
    }
  } while (!done);

  destroy_help_window(help_window);
  return result;
}

/*****************************************************************/

E 4
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

RedrawGlobaly(event)
XEvent *event;
{
  if (event->type == GraphicsExpose)  /* Nasty quick way to do it! */
    {
      event->type = Expose;
      last_event.type = Expose;
    }

  if ((event->type == Expose)&&(GlobalRedraw != NULL))
    (*GlobalRedraw)(event->xany.window);
}

/*****************************************************************/
E 1
