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

/* General window routines */

#define WINDOW_MAIN		/* flag for use by windowsX11.h */
#include "def.h"
#include "windowsX11.h"
#include "bit/arrow"	/* arrow cursor */
#include "bit/amask"
#include <stdlib.h>



/* Global Variables */

Display *display;
int	screen, depth, maskfg, maskbg;
GC	mainGC, inverseGC, drawGC, maskGC, xorGC, xoriGC;
Pixmap	obj_pixmap[MAX_OBJECTS];
Pixmap	obj_mask[MAX_OBJECTS];
GC	obj_clipGC[MAX_OBJECTS];
Window	roomwin;	/* window where room is displayed */
Pixmap	mouse_pix;	/* mouse representation */
unsigned long fgcolor, bgcolor;	/* foreground and background pixels */
int	reversed_screen=0;	/* TRUE when screen is black on white */
XFontStruct  *bigfont;		/* the larger font we'll use quite often */
XFontStruct  *regfont;		/* the smaller, more normal sized font */
XFontStruct  *tinyfont;		/* the smallest font */
XEvent	last_event;		/* the last event that was received from X */

HelpExposeCheck *GlobalHelpExposeList = NULL;



int windows_error_handler(display, err)
/* will be called whenever a non-fatal X windows error occurs */
Display *display;
XErrorEvent *err;
{
  char msg[80];
  XGetErrorText(display, err->error_code, msg, 80);
  fprintf(stderr, "Error code %s\n", msg);
}



open_display()
/* Open the window display for our use */
{
  int maskfunc, drawfunc;
  XGCValues values;

  if ((display = XOpenDisplay(NULL)) == NULL)
    Gerror("could not open display");
  screen = DefaultScreen(display);
  depth = DefaultDepth(display, screen);

  /* For debugging: */
  /*  XSynchronize(display, True); */
  /*  XSetErrorHandler(windows_error_handler); */

  /* decide upon the colors we want */
  if (reversed_screen) {
    fgcolor = BlackPixel(display, screen);
    bgcolor = WhitePixel(display, screen);
  }
  else {
    fgcolor = WhitePixel(display, screen);
    bgcolor = BlackPixel(display, screen);
  }
  if (fgcolor > 0) {
     maskfunc = GXandInverted;
     drawfunc = GXor;
  }
  else {
     maskfunc = GXorInverted;
     drawfunc = GXand;
  }
  if (MONOCHROME) {
    maskfg = fgcolor;
    maskbg = bgcolor;
  }
  else {
    maskfg = 1;
    maskbg = 0;
  }

  /* create the various graphics contexts that will be used */
  values.graphics_exposures = False;
  values.foreground = fgcolor;
  values.background = bgcolor;
  mainGC = XCreateGC(display, RootWindow(display, screen),
		     (GCForeground | GCBackground | GCGraphicsExposures),
		     &values);

  values.function = GXxor;
  values.foreground = ~0;
  values.background = 0;
  xorGC = XCreateGC(display, RootWindow(display, screen),
		    (GCForeground | GCBackground |
		     GCFunction | GCGraphicsExposures), &values);

  values.foreground = fgcolor;
  values.background = bgcolor;
  values.function = drawfunc;
  drawGC = XCreateGC(display, RootWindow(display, screen),
		     (GCFunction |GCForeground |
		      GCBackground | GCGraphicsExposures),
		     &values);

  values.foreground = fgcolor;
  values.background = bgcolor;
  values.function = maskfunc;
  maskGC = XCreateGC(display, RootWindow(display, screen),
		     (GCFunction |GCForeground |
		      GCBackground | GCGraphicsExposures),
		     &values);

  values.foreground = bgcolor;
  values.background = fgcolor;
  inverseGC = XCreateGC(display, RootWindow(display, screen),
			(GCForeground | GCBackground | GCGraphicsExposures),
			&values);

  values.function = GXequiv;
  xoriGC = XCreateGC(display, RootWindow(display, screen),
		     (GCForeground | GCBackground |
		      GCFunction | GCGraphicsExposures), &values);
}



/* load the two bigger fonts for our later use on this display */

load_both_fonts()
{
  bigfont = XLoadQueryFont(display, BIG_FONT_PATH);
  regfont = XLoadQueryFont(display, REG_FONT_PATH);

  if (bigfont==NULL || regfont==NULL) Gerror("could not open big fonts");
}



/* load all the fonts for our later use */

load_all_fonts()
{
  /* load the bigger ones */
  load_both_fonts();

  /* load the smallest one */
  tinyfont = XLoadQueryFont(display, TINY_FONT_PATH);

  if (tinyfont == NULL) Gerror("could not open smallest font");
}




unsigned long ObjectPixelValue(i)
int i;
{
  unsigned long color;
  XColor rgbdef;
  char *cname;
  Colormap colormap;  /* required but not used */

  color = fgcolor; /* for b/w displays */
  colormap = DefaultColormap(display,screen);

  if (depth >= 4)  /* only attempt color if there is a resonable chance
		      that it might work */
    {
      cname = info[(unsigned char)i]->color;
      if (cname && (strlen(cname)>0) && 
	  (XParseColor(display,colormap,cname,&rgbdef) != 0) &&
	  (XAllocColor(display,colormap,&rgbdef) != 0))
	color = rgbdef.pixel;
    }

  return(color);
}


/* take the objects defined in objects.c and allocate the pixmaps
   for use of all windows routines */

define_object_pixmaps()
{
  int i;
  unsigned long color;
  XGCValues values;

  for (i=0; i<objects; i++) {
    obj_mask[i] = NULL;
    obj_clipGC[i] = NULL;
    color = ObjectPixelValue(i);

    if (info[(unsigned char)i]->masked) 
      {
	if (MONOCHROME) {
	  /* monochrome needs depth 1 mask for drawing on first */
	  obj_mask[i] = 
	    XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
					info[(unsigned char)i]->mask,
					BITMAP_WIDTH,BITMAP_HEIGHT,
					fgcolor, bgcolor, 1);
	}
	else {
	  /* color needs depth 1 mask for GC clipping */
	  obj_mask[i] = 
	    XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
					info[(unsigned char)i]->mask,
					BITMAP_WIDTH,BITMAP_HEIGHT,
					maskfg, maskbg, 1);
	  values.graphics_exposures = False;
	  values.foreground = fgcolor;
	  values.background = bgcolor;
	  values.clip_mask = obj_mask[i];
	  obj_clipGC[i] = XCreateGC(display, RootWindow(display, screen),
				    GCForeground | GCBackground | GCClipMask |
				    GCGraphicsExposures, &values);
	}

      }

    /* load and store the pixmap for this object and its mask */
    obj_pixmap[i] =
      XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
				  info[(unsigned char)i]->bitmap, 
				  BITMAP_WIDTH,BITMAP_HEIGHT,
				  color, bgcolor, depth);
  }
}

  free_object_pixmaps()
{
  register int i;
  
  for (i=0; i<objects; i++)
    {
      XFreePixmap(display, obj_pixmap[i]);
      obj_pixmap[i] = NULL;
      if (info[(unsigned char)i]->masked) 
	{
	  XFreePixmap(display, obj_mask[i]);
	  obj_mask[i] = NULL;
	}
    }
}


/* store mouse bitmap as a pixmap */

define_mouse_bitmap()
{
  /* store the mouse representation pixmap */
  mouse_pix = XCreatePixmapFromBitmapData(display, RootWindow(display,
		    screen), mouse_bits, mouse_width, mouse_height,
		    fgcolor, bgcolor, depth);
}



/* move the mouse to a particular square in the room window */

warp_mouse_in_room_win(x, y)
int x, y;
{
  /* move the mouse to the right spot in the room window */
  XWarpPointer(display, None, roomwin, 0, 0, 0, 0,
	       PIXELX(x) + BITMAP_WIDTH / 2, PIXELY(y) + BITMAP_HEIGHT / 2);
}



/* create a cursor from bitmap data and define it for a window */

Cursor make_cursor_from_bitmap_data(d, w, width, height, bits, mask,
				      x_hot, y_hot, fgcolor, bgcolor)
Display *d;
Window w;
int width, height;
char *bits, *mask;
int x_hot, y_hot, fgcolor, bgcolor;
{
  Cursor cursor;
  Pixmap pix1, pix2;
  XColor fgcolorcell, bgcolorcell;

  /* make the pixmaps that the cursor will be created out of */
  pix1 = XCreatePixmapFromBitmapData(d, w, bits, width, height,
				     1, 0, 1);
  pix2 = XCreatePixmapFromBitmapData(d, w, mask, width, height,
				     1, 0, 1);
  /* make the cursor */
  fgcolorcell.pixel = fgcolor;
  bgcolorcell.pixel = bgcolor;
  XQueryColor(display, DefaultColormap(display, screen), &fgcolorcell);
  XQueryColor(display, DefaultColormap(display, screen), &bgcolorcell);
  cursor = XCreatePixmapCursor(d, pix1, pix2, &fgcolorcell,
			       &bgcolorcell, x_hot, y_hot);
  XDefineCursor(d, w, cursor);

  /* free pixmaps */
  XFreePixmap(display, pix1);
  XFreePixmap(display, pix2);

  return cursor;
}



/* define cursor in the given window to be a thin arrow */

define_thin_arrow_cursor(win)
Window win;
{
  make_cursor_from_bitmap_data(display, win, arrow_width, arrow_height,
			       arrow_bits, amask_bits, arrow_x_hot,
			       arrow_y_hot, fgcolor, bgcolor);
}


/*****************************************************************/

TransferPixmapToData(pixmap,array)
Pixmap pixmap;
char array[];
{
  char *data;
  char temp[L_tmpnam];
  int width,height,x,y,i;

  tmpnam(temp);
  
  if (fgcolor == 0)
    XFillRectangle(display,pixmap,xorGC,
		   0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
  XWriteBitmapFile(display,temp,pixmap,BITMAP_WIDTH,BITMAP_HEIGHT,-1,-1);
  if (fgcolor == 0)
    XFillRectangle(display,pixmap,xorGC,
		   0,0,BITMAP_WIDTH,BITMAP_HEIGHT);

  XXReadBitmapFile(temp, &width, &height, &data, &x, &y);

  if (data != NULL)
    {
      for (i = 0;i < BITMAP_ARRAY_SIZE;i++)
	array[i] = data[i];
      free(data);
    }
  unlink(temp);
}

/*****************************************************************/

edit_pixmap(pixmap)
Pixmap *pixmap;
{
  unsigned int width,height;
  int x,y;
  char temp[L_tmpnam],out[L_tmpnam+10];
  Pixmap new;

  tmpnam(temp);

  if (*pixmap)
    {
      if (fgcolor == 0)
	XFillRectangle(display,*pixmap,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
      XWriteBitmapFile(display,temp,*pixmap,BITMAP_WIDTH,BITMAP_HEIGHT,-1,-1);
      if (fgcolor == 0)
	XFillRectangle(display,*pixmap,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
    }
  
  sprintf(out,"bitmap %s %dx%d",temp,BITMAP_WIDTH,BITMAP_HEIGHT);
  system(out);
  
  XReadBitmapFile(display,RootWindow(display,screen),
		  temp,&width,&height,&new,&x,&y);
  unlink(temp);

  if ((width != BITMAP_WIDTH)||(height != BITMAP_HEIGHT))
    {
      printf("Bitmap wrong size!\n");
      return(False);
    }

  if (fgcolor == 0)
    XFillRectangle(display,new,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);


  if (*pixmap)
    XFreePixmap(display,*pixmap);
  *pixmap = new;

  return(True);
}


/*****************************************************************/

char *add_into_or_on_end(s, add)
/* adds the 'add' string into s at the point where '%s' is found, or
   places add on the end if there is no '%s' in s string.  Returns
   a newly malloced string that the caller needs to free. */
char *s, *add;
{
  int found = 0;
  char *ptr, *result;

  if (!s || !add) return NULL;
  demand((result=(char *) malloc(sizeof(char)*(strlen(s) + strlen(add) + 2))),
	 "not enough memory for adding string");

  if (strlen(s)) {
    for (ptr=s+1; (*ptr && !found); ptr++)
      if (*ptr == 's' && *(ptr - 1) == '%') found = 1;
  }

  if (found)
    sprintf(result, s, add);
  else {
    sprintf(result, "%s %s", s, add);
  }
  return result;
}

/*****************************************************************/

char *call_up_editor(initial)
/* calls up the users favorite editor and allows him to edit the string
   given.  If string is NULL, then user may create text at will in the
   editor.  Returns newly mallocced copy of the created string (which
   caller must eventually free) or 'initial' if there is no change to
   the initial text.  Returns NULL if there is no longer any text. */
char *initial;
{
  char filename[L_tmpnam], *result, *editor, *cmd;

  editor = getenv("EDITOR");
  if (!editor) editor = DEFAULT_EDITOR;

  cmd = getenv("EDITOR_COMMAND");
  if (!cmd) cmd = XTERM_COMMAND;

  tmpnam(filename);
  editor = add_into_or_on_end(editor, filename);
  cmd = add_into_or_on_end(cmd, editor);

  if (cmd) {
    int stat;
    if (initial) stat = write_string_to_file(filename, initial);
    if (stat) {
      system(cmd);
      result = read_string_from_file(filename);
      unlink(filename);
    }
    else result = initial;
  }

  if (initial && result && !strcmp(initial, result) && (initial != result)) {
    free(result);
    result = initial;
  }
  if (cmd) free(cmd);
  if (editor) free(editor);
  return result;
}


/*****************************************************************/
