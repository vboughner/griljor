/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989 / January 1991
 **************************************************************************/

#include "importx.h"

/* this global variable will hold the contents of the last event occuring
   during get_input */
XEvent last_event;

/* This routine looks for keypresses and ignores any other kind
   of input, including window exposures.  It returns the keypress found
   in the variable keyp.  The function returns a value of 1 if a keypress
   is found or a zero if there are no keypresses at the moment.
   This routine assumes you have the right display selected and that
   you have previously used XSelectInput to pay attention to keypresses. */

int get_key_press(what_win, keyp, display)
Window what_win;
char    *keyp;
Display	*display;
{
  int x, y, type, detail;
  Window happenwin;

  type = get_input(what_win, keyp, &x, &y, &detail, &happenwin, display);
  if (type!=KEYBOARD) return 0;
  else return 1;
}

/* This routine is in charge of getting an entire string.  If an
   exposure event occurs while it is getting the string, it will
   redraw the things it knows about (the input line) and set the
   exposure flag.  The correct display and input selection is
   assumed. This routine will echo the characters typed at the
   given location. */

char *get_string(what_win, fi, xstart, y, prompt, maxlen, beenexposed,display,gc)
GC	gc;
Display	*display;
Window  what_win;
XFontStruct *fi;
int     xstart, y;
char    *prompt;
int     maxlen, *beenexposed;
{
  int x, promptlen, detail, i = 0;
  int type, mousex, mousey, done = 0;
  char c, line[120], s[2];
  Window *happenwin;

  *beenexposed = 0;

  /* write out prompt and set x to be right after the prompt */
  promptlen = text_width(fi, prompt);
  text_write(what_win, fi, xstart, y, 0, 0, prompt,display,gc);
  x = xstart + promptlen;

  /* write out the initial underline char as a cursor */
  text_write(what_win, fi, x, y, i, 0, "_", display,gc);
  do {

    /* Look for input until some is found */

  do {
      type = get_input(what_win, &c, &mousex, &mousey, &detail, &happenwin, display);
  } while (type==NOTHING);

    /* Now act according to the input received */
    switch (type) {

        case KEYBOARD:
            if (c=='\n' || c=='\r') {

            /* Erase underline char and quit on return key */
            done = 1;
            text_erase(what_win, fi, x, y, i, 0, 1,display,gc);
	    }

            else if (c=='\b' || c==127 || c==8) {

              /* Erase chars when backspacing */
              text_erase(what_win, fi, x, y, i + 1, 0, 1,display);
              text_erase(what_win, fi, x, y, i, 0, 1,display);
              if (i>0) i--;
              text_erase(what_win, fi, x, y, i, 0, 1,display);
              text_write(what_win, fi, x, y, i, 0, "_",display,gc);

	    }
            else {

              /* Add and write chars when they are received */
              s[0] = c;
              s[1] = '\0';
              text_erase(what_win, fi, x, y, i, 0, 1,display);
              text_write(what_win, fi, x, y, i, 0, s,display,gc);
              line[i++] = c;
              if (i>=maxlen) done = 1;
              else text_write(what_win, fi, x, y, i, 0, "_",display,gc);
	    }
            break;

        case MOUSE:
            break;

        case EXPOSED:
            text_write(what_win, fi, xstart, y, 0, 0, prompt,display,gc);
            line[i] = '\0';
            text_write(what_win, fi, x, y, 0, 0, line,display,gc);
            text_write(what_win, fi, x, y, i, 0, "_",display,gc);
            *beenexposed = 1;
            break;

        default:
            break;
    }

}while (!done);

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

get_input(what_win, keyp, mx, my, detail, where_happened, display)
Window what_win;
char   *keyp;
int    *mx, *my, *detail;
Window *where_happened;
Display	*display;
{
  XEvent        evt;
  KeySym        keysym;
  char          buf[20];
  int           nchar, found = 0;

  /* Check how many input events are on the queue */
 found = XPending(display);
  if (!found) return NOTHING;

  /* get the event at head of queue and the window it happened in */
  XNextEvent(display, &evt);
  *where_happened = evt.xany.window;
  bcopy(&evt, &last_event, sizeof(XEvent));     /* save this as last event */

  /* act according to the type of the event */
  switch (evt.type) {
      case KeyPress:
          *mx = evt.xkey.x;
          *my = evt.xkey.y;
          nchar = XLookupString((XKeyEvent *) &evt, buf, 19, &keysym,
                                (XComposeStatus *) NULL);
          if (nchar == 1) {
            *keyp = buf[0];
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

/* This procedure prints out a string of characters in the desired window.
   You must give it the window, font, x and y location, number of characters
   in each direction to offset the printing,  and the string to print */

text_write(win, fi, x, y, xoff, yoff, s,display,gc)
Display	*display;
GC	gc;
Window  win;
XFontStruct  *fi;
int       x, y, xoff, yoff;
char      *s;
{
  XSetFont(display, gc, fi->fid);
  XDrawImageString(display, win, gc, (x + char_width(fi) * xoff),
                   (y + char_height(fi) * yoff + fi->ascent), s, strlen(s));
}

/* This procedure erases earlier printed text characters in the desired
   window.  You must give it parameters similar to text_write above. */

text_erase(win, fi, x, y, xoff, yoff, len,display)
Display	*display;
Window    win;
XFontStruct  *fi;
int       x, y, xoff, yoff, len;
{
  XClearArea(display, win, (x + xoff * char_width(fi)),
             (y + yoff * char_height(fi)), len * char_width(fi),
             char_height(fi), FALSE);
}
