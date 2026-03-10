#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "free.h"
#include "credits.h"

Pixmap credits;


/* Macro definitions for font functions, returning the number of pixels
   high a font is or how many pixels long a string of text in a certain
   font will be. 'fi' is a (XFontStruct *) */

#define text_width(fi, s)       (XTextWidth((fi), (s), strlen(s)))
#define text_height(fi)         (((fi)->ascent) + ((fi)->descent))
#define char_height(fi)         (text_height(fi))
#define char_width(fi)          ((fi)->max_bounds.rbearing -            \
                                 (fi)->min_bounds.lbearing)
#define max(a, b)		(((a) > (b)) ? (a) : (b))

#define LSPACE 4
#define BSPACE 15
#define INC 8
#define SCROLL_DELAY 70000
#define BITMARGIN 6
#define MINMARGIN 8

static int marginx;
static int marginy;

void trevWriteBlock(display, window, xstart, ystart, thisName, fontGC, 
		    inverseGC, reg, tiny, blockWidth, blockHeight)
     Display *display;
     Window window;
     int xstart, ystart;
     Credits *thisName;
     GC fontGC;
     GC inverseGC;
     XFontStruct *reg;
     XFontStruct *tiny;
     int blockWidth;
     int blockHeight;
{
  char buf[80];
  int xpos, ypos, i, addon;

  xpos = xstart + 32 + BITMARGIN;
  ypos = ystart + text_height(reg);

  addon = 32 + BITMARGIN + 2*marginx;
  strcpy(buf, thisName->name);
  i = strlen(buf);
  while(i > 0 && text_width(reg, buf) + addon > blockWidth) {
    *(buf + (--i)) = '\0';
  }
  XSetFont(display, fontGC, reg->fid);
  XDrawString(display, credits, fontGC, xpos, ypos, 
	      buf, strlen(buf));

  ypos += text_height(tiny) + LSPACE;
  
  strcpy(buf, thisName->contrib1);
  i = strlen(buf);
  while(i > 0 && text_width(tiny, buf) + addon > blockWidth) {
    *((buf) + (--i)) = '\0';
  }
  XSetFont(display, fontGC, tiny->fid);
  XDrawString(display, credits, fontGC, xpos, ypos, 
	      buf, strlen(buf));

  ypos += text_height(tiny) + LSPACE;
  
  strcpy(buf, thisName->contrib2);
  i = strlen(buf);
  while(i > 0 && text_width(tiny, buf) + addon > blockWidth) {
    *((buf) + (--i)) = '\0';
  }
  XSetFont(display, fontGC, tiny->fid);
  XDrawString(display, credits, fontGC, xpos, ypos, 
	      buf, strlen(buf));

  if(thisName->face) {
    XCopyArea(display, thisName->face, credits, fontGC,
	      0, 0, thisName->width, thisName->height, 
	      xstart, ystart + ((blockHeight - 32) / 2));
  }
}



void displayCredits(display, window, names, depth, clearGC, fontGC,
		    reg, tiny)
     Display *display;
     Window window;
     Credits *names;
     int depth;
     GC clearGC;
     GC fontGC;
     XFontStruct *reg;
     XFontStruct *tiny;
{
  Window dummy;
  int x, y, xpix;
  unsigned int winWidth, winHeight;
  unsigned int border_width, dippy;
  int curx, cury;
  int blockHeight, blockWidth;
  Credits *thisName;
  int maxSize = 0;


  thisName = names;
  XGetGeometry(display, window, &dummy, &x, &y, &winWidth, &winHeight,
	       &border_width, &dippy);
  credits = MyXCreatePixmap(display, window, winWidth, winHeight, depth);
  XFillRectangle(display, credits, clearGC, 0, 0, winWidth, winHeight);
  XDrawLine(display, credits, fontGC, 
	    winWidth/2-1, 0, winWidth/2-1, winHeight);
  XDrawLine(display, credits, fontGC, winWidth/2, 0, winWidth/2, winHeight);

  blockHeight = text_height(tiny) + 2 * text_height(tiny) 
                + 3 * LSPACE + BSPACE;
  blockWidth = (int)(winWidth) / 2;

  marginx = 0;
  marginy = (winHeight - (winHeight/blockHeight) * blockHeight) / 2;

  curx = marginx;
  cury = marginy;

  while(thisName && (cury + blockHeight < winHeight - marginy)) {
    maxSize = max(maxSize, text_width(tiny, thisName->contrib1));
    maxSize = max(maxSize, text_width(tiny, thisName->contrib2));
    thisName = thisName->next;
    curx += blockWidth;
    if(curx > winWidth - marginx) {
      cury += blockHeight;
      curx = marginx;
    }
  }

  marginx = (blockWidth - maxSize - 32 - BITMARGIN) / 2;
  marginx = max(marginx, MINMARGIN);

  curx = marginx;
  cury = marginy;
  thisName = names;

  while(thisName && (cury + blockHeight < winHeight - marginy)) {
    trevWriteBlock(display, window, curx, cury, thisName, fontGC, 
		   clearGC, reg, tiny, blockWidth, blockHeight);
    thisName = thisName->next;
    curx += blockWidth;
    if(curx > winWidth - marginx) {
      cury += blockHeight;
      curx = marginx;
    }
  }

  xpix = winWidth/2;
  for(x=INC; x<xpix; x+=INC) {
    XCopyArea(display, credits, window, fontGC,
	      0, 0, x, winHeight, xpix-x, 0);
    XCopyArea(display, credits, window, fontGC,
	      winWidth-x, 0, x, winHeight, xpix, 0);
    XFlush(display);
    usleep(SCROLL_DELAY);
  }    
  XCopyArea(display, credits, window, fontGC,
	    0, 0, winWidth, winHeight, 0, 0);
  XFlush(display);
}
  
