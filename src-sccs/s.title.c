h46587
s 00000/00000/00408
d D 1.5 92/08/07 01:04:22 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00006/00010/00402
d D 1.4 91/08/29 01:40:55 vanb 4 3
c fixed up more compatibility problems
e
s 00001/00001/00411
d D 1.3 91/08/03 23:19:36 labc-3id 3 2
c fixed a little inverseness problem with title screen ll window
e
s 00018/00003/00394
d D 1.2 91/06/16 14:16:12 labc-3id 2 1
c trevor was making some color graphics changes
e
s 00397/00000/00000
d D 1.1 91/05/17 04:16:13 labc-3id 1 0
c date and time created 91/05/17 04:16:13 by labc-3id
e
u
U
f e 0
t
T
I 1
D 3
/* title.c for Griljor Copyright 1990 */
E 3
I 3
/* title.c for Griljor (ideas and code by Doug Stein) */
E 3

#include <stdio.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "bitmaps/spookbit"
#include "bitmaps/spookmask"
#include "free.h"
#include "credits.h"
#include "animate.h"

extern Credits *setUpCredits();
extern Player *createPlayers();

#define MARGIN 20
#define YCOORD 20
#define INC 5
#define SCROLL 4
#define SCROLL_DELAY 75000

/* Macros you might like to use: */

#define PROGRAM_NAME    "Griljor"
#define PROGRAM_VERSION "0.9"
#define WINDOW_BORDERS	2		/* the number of pixels wide the
					   X window borders are in the game */

/* Macro definitions for font functions, returning the number of pixels
   high a font is or how many pixels long a string of text in a certain
   font will be. 'fi' is a (XFontStruct *) */

#define text_width(fi, s)       (XTextWidth((fi), (s), strlen(s)))
#define text_height(fi)         (((fi)->ascent) + ((fi)->descent))
#define char_height(fi)         (text_height(fi))
#define char_width(fi)          ((fi)->max_bounds.rbearing -            \
                                 (fi)->min_bounds.lbearing)

static char *van = "project leader:  Van A. Boughner";

static Display *display;
static int done = 0;
static Window mainWindow;
static Window subWindow[3];
static Pixmap vanPix;
static Pixmap letter[7];
static Pixmap letterO[8];
static int xpositions[7];
static int xVan, yVan;
static GC context;
GC drawGC, maskGC;
static int do_expose = 0;

extern Pixmap credits;
Pixmap background;

int max(a,b)
     int a,b;
{
  if(a>b)
    return a;
  return b;
}

int min(a,b)
     int a,b;
{
  if(a<b)
    return a;
  return b;
}


/*
static void usleep(val)
     long val;
{
  struct timeval temp;

  temp.tv_sec = 0;
  temp.tv_usec = val;
  select(0, 0, 0, 0, &temp);
}
*/



/*
 * refresh determines which window needs to be updated and updates
 * it accordingly.  All things needed for updates are stored as
 * global variables (ie pixmaps and windows)
 */
void refresh(report)
     XEvent *report;
{
  Window dummy;
  Window where;
  int x, y, i;
  unsigned int pixWidth, pixHeight;
  unsigned int border_width, depth;
  
  if(!do_expose) return;
  where = report->xexpose.window;
  if(where == subWindow[0]) {
    for(i=6; i>=0; i--) {
      XGetGeometry(display, letter[i], &dummy, &x, &y, &pixWidth, &pixHeight,
		   &border_width, &depth);
      if(i==4) {
	XCopyArea(display, letter[i], subWindow[0], context,
		  0, 0, pixWidth, pixHeight, xpositions[i], YCOORD+15);
      }
      else {
	XCopyArea(display, letter[i], subWindow[0], context,
		  0, 0, pixWidth, pixHeight, xpositions[i], YCOORD);
      }
    }
    XGetGeometry(display, vanPix, &dummy, &x, &y, &pixWidth, &pixHeight,
		 &border_width, &depth);
    XCopyArea(display, vanPix, subWindow[0], context,
	      0, 0, pixWidth, pixHeight, xVan, yVan);
  }
  else if(where == subWindow[1]) {}
  else if(where == subWindow[2]) {
    XGetGeometry(display, subWindow[2], &dummy, &x, &y, &pixWidth, &pixHeight,
		 &border_width, &depth);
    XCopyArea(display, credits, subWindow[2], context,
	      0, 0, pixWidth, pixHeight, 0, 0);
  }
}



/*
 * handle_events polls for events and processes them until there are
 * no more events left on the queue
 */
void handle_events()
{
  XEvent report;

  while(XPending(display)) {
    XNextEvent(display, &report);
    switch(report.type) {
    case KeyPress:
      cleanUp(display, mainWindow);
      done = 1;
      break;
    case ButtonPress:
      cleanUp(display, mainWindow);
      done = 1;
      break;
    case Expose:
      refresh(&report);
      break;
    }
  }
}


/* Main title screen procedure */

void title_screen(disp, screen, wind, gc, big, reg, tiny, depth)
     Display *disp;
     int screen;	
     Window wind;	
     GC gc;
     XFontStruct *big;	/* probably "12x24" font */
     XFontStruct *reg;	/* probably "9x15" font */
     XFontStruct *tiny;	/* probably "5x8" font */
     int depth;
{
  Player *players;
  Credits *names;
  Window dummy;
  Pixmap temp;
  int x, y, i, num, inc;
  int twidth, theight;
  int xpos;
  int finalPos;
  unsigned int pixWidth, pixHeight;
  unsigned int winWidth, winHeight;
  unsigned int border_width, dippy;
D 2
  unsigned long fg, bg;
E 2
I 2
  unsigned long fg, bg,maskfunc,drawfunc;
E 2
  GC clearGC;
  GC fontGC;
  unsigned long event_mask = KeyPressMask | ButtonPressMask;
  XEvent report;

  /* Set up the global variables */
  display = disp;
  mainWindow = wind;
  context = gc;

  XSelectInput(display, mainWindow, event_mask);
  fg = WhitePixel(display, screen);
  bg = BlackPixel(display, screen);

  clearGC = XCreateGC(display, mainWindow, 0, NULL);
  XSetForeground(display, clearGC, bg);
  XSetBackground(display, clearGC, fg);

  names = setUpCredits(display, mainWindow, fg, bg, depth);

I 2
  /* figure out the right functions to use for drawing */
  if (fg > 0) {
    maskfunc = GXandInverted;
    drawfunc = GXor;
  }
  else {
    maskfunc = GXorInverted;
    drawfunc = GXand;
  }

E 2
  maskGC = XCreateGC(display, mainWindow, 0, NULL);
I 2
  XSetFunction(display, maskGC, maskfunc);
  XSetForeground(display, maskGC, fg);
  XSetBackground(display, maskGC, bg);
E 2
  drawGC = XCreateGC(display, mainWindow, 0, NULL);
I 2
  XSetFunction(display, drawGC, drawfunc);
  XSetForeground(display, drawGC, fg);
  XSetBackground(display, drawGC, bg);
E 2
  fontGC = XCreateGC(display, mainWindow, 0, NULL);
  XSetForeground(display, fontGC, fg);
  XSetBackground(display, fontGC, bg);
D 2
  XSetFunction(display, drawGC, GXand);
  XSetFunction(display, maskGC, GXorInverted);
E 2
I 2
  
E 2

  XGetGeometry(display, mainWindow, &dummy, &x, &y, &winWidth, &winHeight,
	       &border_width, &dippy);
  xVan = 0;
  yVan = winHeight/2;

  /* If the person is really impatient, they will exit here */
  while(XPending(display)) {
    XNextEvent(display, &report);
    switch(report.type) {
    case KeyPress:
    case ButtonPress:
      return;
    case Expose:
      break;
    }
  }
/*  upper left window */
  subWindow[0] = XCreateSimpleWindow(display, mainWindow, 0, 0,
                  winWidth/2, winHeight/4, WINDOW_BORDERS, fg, bg);
/*  lower left window */
  subWindow[1] = XCreateSimpleWindow(display, mainWindow, 0, winHeight/4,
		  winWidth/2, 3*winHeight/4 - WINDOW_BORDERS*2, 
		  WINDOW_BORDERS, fg, bg);
/*  right window  */
  subWindow[2] = XCreateSimpleWindow(display, mainWindow, winWidth/2, 0,
                  winWidth/2 - WINDOW_BORDERS*2, winHeight - WINDOW_BORDERS*2,
                  WINDOW_BORDERS, fg, bg);


  XClearWindow(display, mainWindow);
  XMapRaised(display, subWindow[0]);
  XMapRaised(display, subWindow[1]);
  XMapRaised(display, subWindow[2]);


  players = createPlayers(display, mainWindow, depth, names, 10);
  twidth = text_width(big, van);
  theight = text_height(big);
  vanPix = MyXCreatePixmap(display, mainWindow, twidth, theight+1, depth);
  background = MyXCreatePixmap(display, mainWindow, 32*40, 32*20, depth);
  readLetterBitmaps(display, mainWindow, fg, bg, depth, letter, letterO,
		    &finalPos);

  /*  Check the events again to see if anyone wants to exit yet */
  handle_events();
  if(done == 1) return;

  /* Do the drawing for the van pixmap */
  XFillRectangle(display, vanPix, clearGC, 0, 0, twidth, theight+1);
  XSetFont(display, fontGC, big->fid);
  XDrawString(display, vanPix, fontGC, 0, theight - big->descent - 1,
	      van, strlen(van));

  /* Select exposure input for all of the sub windows */
  event_mask = ExposureMask | ButtonPressMask;
  XSelectInput(display, subWindow[0], event_mask);
  XSelectInput(display, subWindow[1], event_mask);
  XSelectInput(display, subWindow[2], event_mask);


  /* Do the GRILJOR animation */
  for(i=6; i>=0; i--) {
    XGetGeometry(display, letter[i], &dummy, &x, &y, &pixWidth, &pixHeight,
		 &border_width, &dippy);
    if(i==0)
      finalPos -= (pixWidth+5);
    else if(i==6)
      finalPos -= pixWidth;
    else
      finalPos -= (pixWidth-5);
    xpositions[i] = finalPos;
    if(i==4) {
D 4
      for(xpos=-50; xpos <= finalPos; xpos += INC) {
E 4
I 4
      for(xpos = -50; xpos <= finalPos; xpos += INC) {
E 4
	XCopyArea(display, letter[i], subWindow[0], context, 
		  0, 0, pixWidth, pixHeight, xpos, YCOORD+15);
	XFlush(display);
      }
      XCopyArea(display, letter[i], subWindow[0], context,
		0, 0, pixWidth, pixHeight, finalPos, YCOORD+15);
    }
    else {
D 4
      for(xpos=-50; xpos <= finalPos; xpos += INC) {
E 4
I 4
      for(xpos = -50; xpos <= finalPos; xpos += INC) {
E 4
	XCopyArea(display, letter[i], subWindow[0], context, 
		  0, 0, pixWidth, pixHeight, xpos, YCOORD);
	XFlush(display);
      }
      XCopyArea(display, letter[i], subWindow[0], context,
		0, 0, pixWidth, pixHeight, finalPos, YCOORD);
    }
    /*  Check the events again to see if anyone wants to exit yet */
    handle_events();
    if(done == 1) return;
  }

  /* Do the VAN animation */
  xVan = winWidth/2 - text_width(big, van) - MARGIN;
  yVan = winHeight/4 - theight - 15;
  for(i=winHeight/4 + theight; i>=yVan; i--) {
    XCopyArea(display, vanPix, subWindow[0], context,
	      0, 0, twidth, theight, xVan, i);
    XFlush(display);
    usleep(20000);
  }

  /*  Check the events again to see if anyone wants to exit yet */
  handle_events();
  if(done == 1) return;

  /*  Display all the credits here  */
  displayCredits(display, subWindow[2], names, depth, clearGC, fontGC,
		 reg, tiny);

  do_expose = 1;

  /*  Check the events again to see if anyone wants to exit yet */
  handle_events();
  if(done == 1) return;

  drawScene(display, mainWindow, background, context, maskGC, drawGC,
	    fg, bg, depth);

  XGetGeometry(display, subWindow[1], &dummy, &x, &y, &winWidth, &winHeight,
	       &border_width, &dippy);
D 4
  while(1) {
E 4
I 4
  while(!done) {
E 4
    x = 40*32/SCROLL-winWidth/SCROLL;
    for(i=0; i<x; i++) {
      doMovement(display, context, players);
      XCopyArea(display, background, subWindow[1], context,
		i*SCROLL, 0, winWidth, winHeight, 0, 0);
      XFlush(display);
      handle_events();
      if(done == 1) return;
      usleep(SCROLL_DELAY);
    }
    y = 20*32/SCROLL - winHeight/SCROLL;
    for(i=0; i<y; i++) {
      doMovement(display, context, players);
      XCopyArea(display, background, subWindow[1], context,
		x*SCROLL, i*SCROLL, winWidth, winHeight, 0, 0);
      XFlush(display);
      handle_events();
      if(done == 1) return;
      usleep(SCROLL_DELAY);
    }
    for(i=x; i>=0; i--) {
      doMovement(display, context, players);
      XCopyArea(display, background, subWindow[1], context,
		i*SCROLL, y*SCROLL, winWidth, winHeight, 0, 0);
      XFlush(display);
      handle_events();
      if(done == 1) return;
      usleep(SCROLL_DELAY);
    }
    for(i=y; i>=0; i--) {
      doMovement(display, context, players);
      XCopyArea(display, background, subWindow[1], context,
		0, i*SCROLL, winWidth, winHeight, 0, 0);
      XFlush(display);
      handle_events();
      if(done == 1) return;
      usleep(SCROLL_DELAY);
    }
  }
  
D 4
  while(1) {
    handle_events();
    if(done == 1) return;
  }

/*  
  while(1) {
E 4
I 4
/*  *** would make radioactive symbol in letter 'o' glow in a cycle
    *** but didn't quite look right
  while(!done) {
E 4
    for(i=0; i<8; i++) {
      XCopyArea(display, letterO[i], subWindow[0], context, 
		0, 0, o1_width, o1_height, xpositions[4], YCOORD);
      XFlush(display);
      usleep(200000);
    }
    for(i=6; i>0; i--) {
      XCopyArea(display, letterO[i], subWindow[0], context, 
		0, 0, o1_width, o1_height, xpositions[4], YCOORD);
      XFlush(display);
      usleep(200000);
    }
  }*/

}
E 1
