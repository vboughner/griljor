h14122
s 00000/00000/00483
d D 1.17 92/08/07 21:30:55 vbo 17 16
c fixes for sun port merged in
e
s 00002/00002/00481
d D 1.16 92/03/02 22:07:59 vanb 16 15
c finished getting editmap to work on basics
e
s 00016/00002/00467
d D 1.15 92/02/20 22:05:09 vanb 15 14
c 
e
s 00061/00007/00408
d D 1.14 91/11/29 16:09:27 labc-4lc 14 13
c finished making entry of hidden text properties in obtor
e
s 00019/00000/00396
d D 1.13 91/11/28 00:10:31 labc-4lc 13 12
c nearly finished hidden text items in dialogs
e
s 00044/00035/00352
d D 1.12 91/11/25 23:03:49 labc-4lc 12 11
c fixed color problems drawing objects
e
s 00021/00005/00366
d D 1.11 91/11/25 20:43:28 labc-4lc 11 10
c attempted fix of color/mono object drawing problem
e
s 00078/00034/00293
d D 1.10 91/10/16 20:06:52 labc-4lc 10 9
c changed made for DecStations
e
s 00080/00000/00247
d D 1.9 91/09/15 23:33:51 labc-4lc 9 8
c made mapinfo dialog editing window work
e
s 00000/00000/00247
d D 1.8 91/08/30 17:04:47 labb-3li 8 7
c 
e
s 00001/00001/00246
d D 1.7 91/08/30 01:32:28 vanb 7 6
c made windows code color compatible
e
s 00006/00005/00241
d D 1.6 91/08/28 21:10:12 vanb 6 5
c made editmap work in color X windows
e
s 00003/00000/00243
d D 1.5 91/08/03 16:27:36 labc-3id 5 4
c fixed a small problem with large n in obj_place_pixmap()
e
s 00014/00005/00229
d D 1.4 91/05/10 04:51:32 labc-3id 4 3
c in windowsX11.h
e
s 00007/00003/00227
d D 1.3 91/05/03 23:09:54 labc-3id 3 2
c 
e
s 00000/00000/00230
d D 1.2 91/03/24 18:08:19 labc-3id 2 1
c 
e
s 00230/00000/00000
d D 1.1 91/02/16 12:56:08 labc-3id 1 0
c date and time created 91/02/16 12:56:08 by labc-3id
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

/* General window routines */

#define WINDOW_MAIN		/* flag for use by windowsX11.h */
#include "def.h"
#include "windowsX11.h"
#include "bit/arrow"	/* arrow cursor */
#include "bit/amask"
I 14
#include <stdlib.h>
E 14


I 14

E 14
/* Global Variables */

Display *display;
D 6
int	screen;
E 6
I 6
D 12
int	screen, depth;
E 12
I 12
int	screen, depth, maskfg, maskbg;
E 12
E 6
D 4
GC	mainGC, inverseGC, drawGC, maskGC, xorGC;
E 4
I 4
D 10
GC	mainGC, inverseGC, drawGC, maskGC, xorGC, xoriGC;
E 10
I 10
D 11
GC	mainGC, inverseGC, maskGC, xorGC, xoriGC;
E 11
I 11
GC	mainGC, inverseGC, drawGC, maskGC, xorGC, xoriGC;
E 11
E 10
E 4
Pixmap	obj_pixmap[MAX_OBJECTS];
Pixmap	obj_mask[MAX_OBJECTS];
I 12
GC	obj_clipGC[MAX_OBJECTS];
E 12
Window	roomwin;	/* window where room is displayed */
Pixmap	mouse_pix;	/* mouse representation */
unsigned long fgcolor, bgcolor;	/* foreground and background pixels */
D 10
int	maskfunc, drawfunc;	/* drawing functions in place_obj_pixmap */
E 10
int	reversed_screen=0;	/* TRUE when screen is black on white */
XFontStruct  *bigfont;		/* the larger font we'll use quite often */
XFontStruct  *regfont;		/* the smaller, more normal sized font */
XFontStruct  *tinyfont;		/* the smallest font */
XEvent	last_event;		/* the last event that was received from X */

HelpExposeCheck *GlobalHelpExposeList = NULL;

D 15
/* Open the window display for our use */
E 15

I 15

int windows_error_handler(display, err)
/* will be called whenever a non-fatal X windows error occurs */
Display *display;
XErrorEvent *err;
{
  char msg[80];
  XGetErrorText(display, err->error_code, msg, 80);
  fprintf(stderr, "Error code %s\n", msg);
}



E 15
open_display()
I 15
/* Open the window display for our use */
E 15
{
I 11
  int maskfunc, drawfunc;
E 11
  XGCValues values;

  if ((display = XOpenDisplay(NULL)) == NULL)
    Gerror("could not open display");
  screen = DefaultScreen(display);
I 6
  depth = DefaultDepth(display, screen);
E 6

I 5
  /* For debugging: */
D 6
  /* XSynchronize(display, True); */
E 6
I 6
D 7
  XSynchronize(display, True);
E 7
I 7
D 15
  /* XSynchronize(display, True); */
E 15
I 15
D 16
  XSynchronize(display, True);
  XSetErrorHandler(windows_error_handler);
E 16
I 16
  /*  XSynchronize(display, True); */
  /*  XSetErrorHandler(windows_error_handler); */
E 16
E 15
E 7
E 6

E 5
  /* decide upon the colors we want */
  if (reversed_screen) {
    fgcolor = BlackPixel(display, screen);
    bgcolor = WhitePixel(display, screen);
  }
  else {
    fgcolor = WhitePixel(display, screen);
    bgcolor = BlackPixel(display, screen);
  }
I 11
  if (fgcolor > 0) {
     maskfunc = GXandInverted;
     drawfunc = GXor;
  }
  else {
     maskfunc = GXorInverted;
     drawfunc = GXand;
  }
I 12
  if (MONOCHROME) {
    maskfg = fgcolor;
    maskbg = bgcolor;
  }
  else {
    maskfg = 1;
    maskbg = 0;
  }
E 12
E 11

D 10
  /* figure out the right functions to use for drawing */
  if (fgcolor > 0) {
    maskfunc = GXandInverted;
    drawfunc = GXor;
  }
  else {
    maskfunc = GXorInverted;
    drawfunc = GXand;
  }
E 10
D 12

E 12
  /* create the various graphics contexts that will be used */
I 3
  values.graphics_exposures = False;
E 3
  values.foreground = fgcolor;
  values.background = bgcolor;
  mainGC = XCreateGC(display, RootWindow(display, screen),
D 3
		     (GCForeground | GCBackground), &values);
E 3
I 3
		     (GCForeground | GCBackground | GCGraphicsExposures),
		     &values);
E 3
D 10
  values.function = drawfunc; 
  drawGC = XCreateGC(display, RootWindow(display, screen),
D 3
		     (GCForeground | GCBackground | GCFunction), &values);
E 3
I 3
D 4
		     (GCForeground | GCBackground | GCGraphicsExposures),
E 4
I 4
		     (GCFunction |GCForeground |
		      GCBackground | GCGraphicsExposures),
E 4
		     &values);
E 3
  values.function = maskfunc;
E 10
I 10

  values.function = GXxor;
  values.foreground = ~0;
  values.background = 0;
  xorGC = XCreateGC(display, RootWindow(display, screen),
		    (GCForeground | GCBackground |
		     GCFunction | GCGraphicsExposures), &values);

D 11
  values.foreground = ~0;
  values.background = 0;
  values.function = GXand;
E 11
I 11
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
E 11
E 10
  maskGC = XCreateGC(display, RootWindow(display, screen),
D 3
		     (GCForeground | GCBackground | GCFunction), &values);
E 3
I 3
D 4
		     (GCForeground | GCBackground | GCGraphicsExposures),
E 4
I 4
		     (GCFunction |GCForeground |
		      GCBackground | GCGraphicsExposures),
E 4
		     &values);
E 3

  values.foreground = bgcolor;
  values.background = fgcolor;
  inverseGC = XCreateGC(display, RootWindow(display, screen),
D 4
			(GCForeground | GCBackground), &values);
E 4
I 4
			(GCForeground | GCBackground | GCGraphicsExposures),
			&values);
E 4

D 10
  values.function = GXxor;
  values.foreground = ~0;
  values.background = 0;
  xorGC = XCreateGC(display, RootWindow(display, screen),
D 4
		    (GCForeground | GCBackground | GCFunction), &values);
E 4
I 4
		    (GCForeground | GCBackground |
		     GCFunction | GCGraphicsExposures), &values);

E 10
  values.function = GXequiv;
  xoriGC = XCreateGC(display, RootWindow(display, screen),
		     (GCForeground | GCBackground |
		      GCFunction | GCGraphicsExposures), &values);
E 4
D 11

E 11
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



I 10

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


E 10
/* take the objects defined in objects.c and allocate the pixmaps
   for use of all windows routines */

define_object_pixmaps()
{
D 12
  register int i;
E 12
I 12
  int i;
E 12
D 10
    
E 10
I 10
  unsigned long color;
D 12
  Pixmap temp;
E 12
I 12
  XGCValues values;
E 12

E 10
  for (i=0; i<objects; i++) {
I 12
    obj_mask[i] = NULL;
    obj_clipGC[i] = NULL;
E 12
D 10
    /* load and store the pixmap for this object and its mask */
    obj_pixmap[i] = XCreatePixmapFromBitmapData(display, RootWindow(display,
		    screen), info[(unsigned char)i]->bitmap, BITMAP_WIDTH,
D 6
		    BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 6
I 6
		    BITMAP_HEIGHT, fgcolor, bgcolor, depth);
E 6
    if (info[(unsigned char)i]->masked) {
      obj_mask[i] = XCreatePixmapFromBitmapData(display, RootWindow(display,
		    screen), info[(unsigned char)i]->mask, BITMAP_WIDTH,
D 6
		    BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 6
I 6
		    BITMAP_HEIGHT, fgcolor, bgcolor, depth);
E 10
I 10
    color = ObjectPixelValue(i);

    if (info[(unsigned char)i]->masked) 
      {
D 12
	obj_mask[i] = 
	  XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
				      info[(unsigned char)i]->mask,
				      BITMAP_WIDTH,BITMAP_HEIGHT,
				      0L,(~0L),depth);
E 12
I 12
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
E 12

D 12
	temp =
	  XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
				      info[(unsigned char)i]->bitmap, 
				      BITMAP_WIDTH,BITMAP_HEIGHT,
				      color, 0L, depth);

	obj_pixmap[i] = 
	 XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
				      info[(unsigned char)i]->mask,
				      BITMAP_WIDTH,BITMAP_HEIGHT,
				      bgcolor,0L,depth);
	
	XCopyArea(display,temp,obj_pixmap[i],xorGC,0,0,
		  BITMAP_WIDTH,BITMAP_HEIGHT,0,0);

	XFreePixmap(display,temp);
E 10
E 6
    }
I 10
    else
      {
	/* load and store the pixmap for this object and its mask */
	obj_pixmap[i] =
	  XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
				      info[(unsigned char)i]->bitmap, 
				      BITMAP_WIDTH,BITMAP_HEIGHT,
				      color, bgcolor, depth);

E 12
      }
I 12

    /* load and store the pixmap for this object and its mask */
    obj_pixmap[i] =
      XCreatePixmapFromBitmapData(display, RootWindow(display,screen),
				  info[(unsigned char)i]->bitmap, 
				  BITMAP_WIDTH,BITMAP_HEIGHT,
				  color, bgcolor, depth);
E 12
E 10
  }
}

D 10
free_object_pixmaps()
E 10
I 10
  free_object_pixmaps()
E 10
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
D 6
		    fgcolor, bgcolor, 1);
E 6
I 6
		    fgcolor, bgcolor, depth);
E 6
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
I 9


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
I 13

I 14
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
E 14
/* calls up the users favorite editor and allows him to edit the string
   given.  If string is NULL, then user may create text at will in the
   editor.  Returns newly mallocced copy of the created string (which
D 14
   caller must eventually free) or NULL if there is no change to the initial
   text. */

char *call_up_editor(initial)
E 14
I 14
   caller must eventually free) or 'initial' if there is no change to
   the initial text.  Returns NULL if there is no longer any text. */
E 14
char *initial;
{
D 14
  char *result;
  demand((result = (char *) malloc(200)), "no memory");
  strcpy(result, "I've been edited by editor");
E 14
I 14
  char filename[L_tmpnam], *result, *editor, *cmd;
E 14

I 14
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
E 14
  return result;
}


/*****************************************************************/
E 13
E 9
E 1
