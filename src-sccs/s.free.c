h07811
s 00000/00000/00129
d D 1.2 92/08/07 01:01:21 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00129/00000/00000
d D 1.1 91/05/17 04:16:00 labc-3id 1 0
c date and time created 91/05/17 04:16:00 by labc-3id
e
u
U
f e 0
t
T
I 1
#include <stdio.h>
#include <malloc.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


typedef struct free_me {
  Pixmap myPixmap;
  struct free_me *next;
} FreeMe;

typedef struct free_me_too {
  char *mem;
  struct free_me_too *next;
} FreeMeToo;


static FreeMe *beginFree = NULL;
static FreeMeToo *beginMalloc = NULL;


char *mymalloc(size)
     unsigned size;
{
  char *chunk;
  FreeMeToo *link;

  chunk = malloc(size);
  if(!chunk) {
    printf("Requested malloc failed in my malloc\n");
    exit(1);
  }

  link = (FreeMeToo *)malloc(sizeof(FreeMe));
  if(!link) {
    printf("Malloc failed in mymalloc\n");
    exit(1);
  }

  link->mem = chunk;
  link->next = beginMalloc;
  beginMalloc = link;

  return chunk;
}


Pixmap MyXCreatePixmap(display, window, width, height, depth)
     Display *display;
     Window window;
     int width, height;
     int depth;
{
  Pixmap thisPixmap;
  FreeMe *curFree;
  

  thisPixmap = XCreatePixmap(display, window, width, height, depth);
  curFree = (FreeMe *)malloc(sizeof(FreeMe));
  if(!curFree) {
    printf("Malloc failed in MyXCreatePixmapFromBitmapData\n");
    exit(1);
  }
  curFree->myPixmap = thisPixmap;
  curFree->next = beginFree;
  beginFree = curFree;

  return thisPixmap;
}
  
  


Pixmap MyXCreatePixmapFromBitmapData(display, window, bits, width, height,
				     fg, bg, depth)
     Display *display;
     Window window;
     char *bits;
     int width, height;
     unsigned long fg, bg;
     int depth;
{
  Pixmap thisPixmap;
  FreeMe *curFree;
  

  thisPixmap = XCreatePixmapFromBitmapData(display, window,
					   bits, width, height,
					   fg, bg, depth);
  curFree = (FreeMe *)malloc(sizeof(FreeMe));
  if(!curFree) {
    printf("Malloc failed in MyXCreatePixmapFromBitmapData\n");
    exit(1);
  }
  curFree->myPixmap = thisPixmap;
  curFree->next = beginFree;
  beginFree = curFree;

  return thisPixmap;
}
  
  


void cleanUp(display, window)
     Display *display;
     Window window;
{
  FreeMe *cur;
  FreeMeToo *cur2;

  cur = beginFree;
  cur2 = beginMalloc;

  while(cur2) {
    free(cur2->mem);
    cur2 = cur2->next;
    free(beginMalloc);
    beginMalloc = cur2;
  }

  while(cur) {
    XFreePixmap(display, cur->myPixmap);
    cur = cur->next;
    free(beginFree);
    beginFree = cur;
  }
  XDestroySubwindows(display, window);
}
E 1
