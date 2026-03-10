/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * November 1990
 **************************************************************************/


#include "windowsX11.h"
#include "menusX11.h"

/* additional help window implementation routines, to make them work
   more like menus */


/* this routine returns the line number (starting at zero) where the
   mouse button was pressed, or -1 if the y location is out of range. */

int which_line_selected(rec, y)
Help *rec;	/* help window record */
int y;		/* pixel y location where mouse button was pressed */
{
  if ((y < 0) || (y >= char_height(rec->fi) * rec->height)) return -1;
  else return (y / char_height(rec->fi));
}



/* this routine flashes the selected line in the help window by
   inverting it momentarily */

flash_line_in_help_window(rec, line)
Help *rec;
int line;
{
  XGCValues values;
  GC invertGC;
  int x, y, w, h;
  
  /* create a GC capable of inverting the destination pixels of an XCopy */
  values.function = GXinvert;
  invertGC = XCreateGC(display, RootWindow(display, screen),
		       GCFunction, &values);

  /* invert the area in question */
  x = WINDOW_PADDING;
  y = line * char_height(rec->fi) + WINDOW_PADDING;
  w = rec->width * char_width(rec->fi);
  h = char_height(rec->fi);
  XCopyArea(display, rec->win, rec->win, invertGC, 0, 0, w, h, x, y);
  XFlush(display);

  /* wait a moment, and invert it back */
  sleep(1);
  XCopyArea(display, rec->win, rec->win, invertGC, 0, 0, w, h, x, y);
}



/* create a help window, print a prompt, and get some input.  The text entered
   is echoed only if 'echo' parameter is FALSE */

char *help_window_input_line(win, x, y, fi, prompt, limit, echo)
Window win;
int x, y;
XFontStruct *fi;
char *prompt;
int limit, echo;
{
  Help *rec;
  char *result;
  int exposed;

  /* create help window */
  rec = make_help_window(win, x, y, strlen(prompt)+limit, 3, fi);
  set_line(rec, 1, prompt);
  init_input_lines(rec->win);
  show_help_window(rec);

  /* get a string from the user */
  if (echo) 
    result = get_string(rec->win, fi, WINDOW_PADDING, char_height(fi) +
			WINDOW_PADDING, prompt, limit, &exposed);
  else
    result = get_string_noecho(rec->win, fi, WINDOW_PADDING, char_height(fi) +
			       WINDOW_PADDING, prompt, limit, &exposed);

  /* destroy help window and return result */
  destroy_help_window(rec);
  return result;
}


/* returns the characters found in the given line of the help window.
   The string returned will be comprised of all character between and
   including x1 and x2.  If the line is empty at that point, an empty
   string will be returned.  Any preceeding or trailing spaces in the
   result string will be removed. */

char *extract_from_help_line(rec, linenum, x1, x2)
Help *rec;
int linenum, x1, x2;
{
  int i;
  char *s, result[MAX_HELP_COLS];
  static char store[MAX_HELP_COLS];

  if (linenum >= rec->height || linenum < 0)
    Gerror("bad linenum in extract_from_help_line");
  if (x1 > rec->width) Gerror("x1 too large to extract_from_help_line");
  if (x2 > rec->width) Gerror("x2 too large to extract_from_help_line");

  for (i=0; i<=x2-x1; i++) result[i] = rec->text[linenum][x1+i];
  result[i] = '\0';

  /* get rid of preceeding or trailing blanks */
  for (s = result; *s == ' '; s++);
  strcpy(store, s);
  for (s = store + strlen(store) - 1; *s == ' '; s--) *s = '\0';

  return store;
}



/* either bring up or put away a help window that has input lines. */

toggle_a_player_help_window(rec)
Help *rec;
{
  if (rec->is_up) {
    XSelectInput(display, rec->win, NoEventMask);
    toggle_help_window(rec);
  }
  else {
    toggle_help_window(rec);
    init_input_lines(rec->win);
  }
}



/* This routine is in charge of getting an entire string.  If an
   exposure event occurs while it is getting the string, it will
   redraw the things it knows about (the input line) and set the
   exposure flag.  The correct display and input selection is 
   assumed. This routine will NOT echo the characters typed at the 
   given location. */

char *get_string_noecho(what_win, fi, xstart, y, prompt, maxlen, beenexposed)
Window 	what_win;
XFontStruct *fi;
int	xstart, y;
char    *prompt;
int     maxlen, *beenexposed;
{
  int x, promptlen, detail, i = 0;
  int type, mousex, mousey, done = 0;
  char c, s[2];
  static char line[120];
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

	      /* do not erase chars when backspacing */
              /* text_erase(what_win, fi, x, y, i + 1, 0, 1); */
	      /* text_erase(what_win, fi, x, y, i, 0, 1);     */
	      if (i>0) i--;
	      /* text_erase(what_win, fi, x, y, i, 0, 1);     */
	      /* text_write(what_win, fi, x, y, i, 0, "_");   */

	    }
	    else {

	      /* Add but do not write chars when they are received */
	      s[0] = c;
	      s[1] = '\0';
	      /* text_erase(what_win, fi, x, y, i, 0, 1); */
	      /* text_write(what_win, fi, x, y, i, 0, s); */
	      line[i++] = c;
	      if (i >= maxlen) i = maxlen - 1;
	      /* if (i>=maxlen) done = 1; */
	      /* else text_write(what_win, fi, x, y, i, 0, "_"); */
	    }
	    break;

	case MOUSE:
	    break;

	case EXPOSED:
	    text_write(what_win, fi, xstart, y, 0, 0, prompt);
	    line[i] = '\0';
	    /* text_write(what_win, fi, x, y, 0, 0, line); */
	    /* text_write(what_win, fi, x, y, i, 0, "_");  */
	    text_write(what_win, fi, x, y, 0, 0, "_");
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



/* given a filename and a maximum window area to use, load the file and then
   create a help window big enough to hold it, or up to the maximum size
   allowed.  If the file can't be found, then NULL is returned, and no
   help window is created. */

Help *make_help_from_loading_file(win, x, y, w, h, fi, filename, libdir)
Window win;
int x, y, w, h;
XFontStruct *fi;
char *filename, *libdir;
{
  FILE *fp;
  char *list[500];
  Help *result;
  int i, cols = 0, lines = 0, size = 0;

  /* open the file, using the library directory if possible and neccessary */
  fp = fopen(filename, "r");
  if (!fp && libdir)
    fp = fopen(libbed_filename(libdir, filename), "r");
    
  if (!fp) return NULL;
  else {
    /* read lines until we've got a full window or there are no more */
    for (i=0; i<500 && !feof(fp); i++) {
      list[i] = create_string(read_line(fp));
      if (strlen(list[i]) > cols) cols = strlen(list[i]);
      size++;
    }
    /* close the file */
    fclose(fp);

    /* create the help window to contain this information */
    if (cols * char_width(fi) > w) cols = w / char_width(fi);
    lines = min(size, h / char_height(fi));
    result = make_help_window(win, x, y, cols, lines, fi);
    for (i=0; i<lines; i++) set_line(result, i, list[i]);

    /* free the list */
    for (i=0; i<size; i++) free(list[i]);
    return result;
  }
}
