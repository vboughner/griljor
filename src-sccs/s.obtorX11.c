h61900
s 00000/00000/00884
d D 1.10 92/08/07 01:02:58 vbo 10 9
c source copied to a separate tree for work on new map and object format
e
s 00006/00002/00878
d D 1.9 92/05/27 21:45:42 vbo 9 8
c made minimal changes required for compile on Sparc2 SVR4
e
s 00003/00000/00877
d D 1.8 91/12/03 17:30:09 labc-4lc 8 7
c implemented header items for object def files
e
s 00006/00003/00871
d D 1.7 91/10/16 20:06:01 labc-4lc 7 6
c changed made for DecStations
e
s 00002/00002/00872
d D 1.6 91/09/15 23:33:41 labc-4lc 6 5
c made mapinfo dialog editing window work
e
s 00000/00000/00874
d D 1.5 91/05/26 22:45:38 labc-3id 5 4
c Just looking around...
e
s 00027/00001/00847
d D 1.4 91/05/17 02:01:18 labc-3id 4 3
c Added code for showing object numbers using '#'
e
s 00001/00002/00847
d D 1.3 91/05/03 23:09:39 labc-3id 3 2
c 
e
s 00628/00661/00221
d D 1.2 91/03/24 17:58:16 labc-3id 2 1
c 
e
s 00882/00000/00000
d D 1.1 91/02/16 12:55:15 labc-3id 1 0
c date and time created 91/02/16 12:55:15 by labc-3id
e
u
U
f e 0
t
T
I 1
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

D 2
/* X Routines used only by the object editor, not by player program */
E 2
I 2
#include "obtor.h"
#include "help.h"
E 2

D 2
#include "windowsX11.h"
#include "def.h"
#include "editobj.h"
E 2
I 2
/* set up the windows needed by the object editor */
E 2

D 2
#define OBTOR_WIN_X		30	/* x of main window, upper left */
#define OBTOR_WIN_Y		5
#define GRAB_WIN_WIDTH		10	/* # of objects wide grab window is */
#define GRAB_WIN_HEIGHT		26	/* # of objects high */
#define GRAB_SPACING		2	/* space between characters */
#define OBJWIN_W		(GRAB_WIN_WIDTH * \
				(BITMAP_WIDTH + GRAB_SPACING))
#define OBJWIN_H		(GRAB_WIN_HEIGHT * \
				(BITMAP_HEIGHT + GRAB_SPACING))
#define OBTOR_WIN_W		((OBJWIN_W * 2) + WINDOW_BORDERS)
#define OBTOR_WIN_H		(OBJWIN_H)
#define STAT_WIN_X		(OBTOR_WIN_X + OBTOR_WIN_W + WINDOW_BORDERS)
#define STAT_WIN_Y		(OBTOR_WIN_Y + OBTOR_WIN_H - STAT_WIN_H)
#define STAT_WIN_W		(mouse_width + 20)
#define STAT_WIN_H		(mouse_height + 20)
#define CLICK_X			(10)
#define CLICK_Y			(10)
#define OBTMSG_WIN_X		(- WINDOW_BORDERS)
#define OBTMSG_WIN_Y		(OBTOR_WIN_H - OBTMSG_WIN_H - WINDOW_BORDERS)
#define OBTMSG_PAD		(4)
#define OBTMSG_WIN_W		(OBTOR_WIN_W)
#define OBTMSG_WIN_H		(char_height(regfont) + OBTMSG_PAD * 2)
E 2
I 2
obtor_window_setup()
{
  XSizeHints size;
  XWMHints hint;
  int count,count2;
D 4
  
E 4
I 4
  XGCValues values;

E 4
  rootwin = RootWindow(display, screen);
E 2

I 4
  values.graphics_exposures = True;
  XChangeGC(display,mainGC,GCGraphicsExposures,&values);

E 4
I 2
  GumbyCursor = XCreateFontCursor(display,XC_gumby);
  HandCursor = XCreateFontCursor(display,XC_hand2);
  DragCursor = XCreateFontCursor(display,XC_hand1);
  SpiralCursor = XCreateFontCursor(display,XC_box_spiral);
  ArrowCursor = XCreateFontCursor(display,XC_top_left_arrow);
  UpArrowCursor = XCreateFontCursor(display,XC_sb_up_arrow);
  DownArrowCursor = XCreateFontCursor(display,XC_sb_down_arrow);
  PirateCursor = XCreateFontCursor(display,XC_man);
  WatchCursor = XCreateFontCursor(display,XC_watch);
  load_window_pixmaps();
  
  mainwin = XCreateSimpleWindow(display, rootwin,
				MAIN_WIN_X,MAIN_WIN_Y,
				MAIN_WIN_W,MAIN_WIN_H,
				1,fgcolor,bgcolor);
E 2

D 2
/* constants used by information window */
#define INFO_LINES	5		/* num of lines in window */
#define INFO_COLS	30		/* num of columns window will hold */
#define INFO_FONT	(regfont)	/* font to use in help window */
E 2
I 2
  hint.flags = IconPixmapHint;
  hint.icon_pixmap = icon_pix;
  XSetWMHints(display,mainwin,&hint);
  size.flags = PPosition | PMinSize | PMaxSize;
  size.x = MAIN_WIN_X;
  size.y = MAIN_WIN_Y;
  size.min_width = MAIN_WIN_W;
  size.max_width = MAIN_WIN_W;
  size.min_height = MAIN_WIN_H;
  size.max_height = MAIN_WIN_H;
  XSetNormalHints(display,mainwin,&size);
  XStoreName(display, mainwin, "Griljor Object Editor");
  XDefineCursor(display,mainwin,WatchCursor);
  XSetWindowBackgroundPixmap(display,mainwin,foo_pix);
  XSelectInput(display, mainwin,KeyPressMask | ButtonPressMask);
E 2


D 2
/* constants used by help window */
#define HELP_X		(OBTOR_WIN_X)
#define HELP_Y		(OBTOR_WIN_H - char_height((HELP_FONT)) * HELP_LINES \
			 - WINDOW_BORDERS)
#define HELP_LINES	13		/* num of lines in window */
#define HELP_COLS	73		/* num of columns window will hold */
#define HELP_FONT	(regfont)	/* font to use in help window */
E 2
I 2
  intro1win = XCreateSimpleWindow(display, mainwin,
				  INTRO_X, INTRO_X, 
				  intro1w,intro1h,
				  WINDOW_BORDERS, fgcolor, bgcolor);
  XSetWindowBackgroundPixmap(display,intro1win,intro1_pix);
  XSetWindowBorderPixmap(display,intro1win,border_pix);
E 2


D 2
/* global variables */
Window	obtorwin;	/* main parent window for obtor windows */
Window	objwin[2];	/* windows where objects are displayed */
Window	statwin;	/* window where obtor stats are displayed */
Window	clickwin;	/* window for mouse diagram */
Window	obtmsgwin;	/* window for messages and input */
Pixmap	item_pixmap[2][MAX_OBJECTS];	/* pixmaps of objects */
Pixmap	item_mask[2][MAX_OBJECTS];	/* pixmap masks for objects */
ObjInfo	click[3];	/* click buffers that may contain obj defs */
Pixmap	click_pixmap[3];/* pixmaps for click buffer objects */
Pixmap	click_mask[3];	/* masks for click buffer objects */
Help	*help_rec;	/* record containing info for help window */
Help	*info_rec;	/* record containing info for info window */
int	msg_up = TRUE;	/* TRUE when message window is mapped */
E 2
I 2
  intro2win = XCreateSimpleWindow(display, mainwin,
				  INTRO_X, INTRO_Y, 
				  intro2w,intro2h,
				  WINDOW_BORDERS, fgcolor, bgcolor);
  XSetWindowBackgroundPixmap(display,intro2win,intro2_pix);
  XSetWindowBorderPixmap(display,intro2win,border_pix);
E 2

I 2
  XMapRaised(display,intro1win);  /* map it now for intro screen */
  XMapRaised(display,intro2win);  /* map it now for intro screen */
  XMapRaised(display,mainwin);  /* map it now for intro screen */
  XFlush(display); /* make sure it gets out */
E 2

D 2
/* procedure declarations */
char *get_obtor_input_line();
E 2
I 2
  trywin = XCreateSimpleWindow(display, rootwin,
			       TRY_WIN_X, TRY_WIN_Y, 
			       TRY_WIN_W, TRY_WIN_H, 
			       WINDOW_BORDERS, fgcolor, bgcolor);
  XSelectInput(display, trywin, ButtonPressMask | ExposureMask );
  XDefineCursor(display,trywin,GumbyCursor);
  XStoreName(display,trywin,"Obtor Try Window");
  init_try_pixmaps();
E 2

I 2
  obtorwin = XCreateSimpleWindow(display, mainwin,
				 OBTOR_WIN_X, OBTOR_WIN_Y, 
				 OBTOR_WIN_W, OBTOR_WIN_H, 
				 WINDOW_BORDERS, fgcolor, bgcolor);
E 2

I 2
  XSetWindowBackgroundPixmap(display,obtorwin,backg_pix);
  XSelectInput(display, obtorwin,
	       KeyPressMask | ButtonPressMask | ExposureMask );
  XDefineCursor(display,obtorwin,ArrowCursor);
  
  obtmsgwin = 
    XCreateSimpleWindow(display, mainwin, OBTMSG_WIN_X, OBTMSG_WIN_Y, 
			OBTMSG_WIN_W, OBTMSG_WIN_H, WINDOW_BORDERS,
			fgcolor, bgcolor);
  XDefineCursor(display,obtmsgwin,SpiralCursor);
  XSetWindowBackgroundPixmap(display,obtmsgwin,foo_pix);
  XSelectInput(display, obtmsgwin,
	       KeyPressMask | ButtonPressMask | ExposureMask );
  
  for (count = 0;count < MAX_BUFFERS;count++)
    {
      objwin[count] =
	XCreateSimpleWindow(display,obtorwin,
			    0,0,1,1,WINDOW_BORDERS,fgcolor,bgcolor);
      /* with dummy location */
      XSetWindowBorderPixmap(display,objwin[count],border_pix);
E 2

D 2
/* load the cursors for the obtor program */
E 2
I 2
      try_pix[count] =
	XCreatePixmap(display,objwin[count],
D 7
		      BITMAP_WIDTH+GRAB_SPACING,BITMAP_HEIGHT+GRAB_SPACING,1);
E 7
I 7
		      BITMAP_WIDTH+GRAB_SPACING,
		      BITMAP_HEIGHT+GRAB_SPACING,depth);
E 7
      set_window_background(count,-1);
      XSelectInput(display, objwin[count],
		   KeyPressMask | ButtonPressMask | ExposureMask );
      
      infowin[count] =
	XCreateSimpleWindow(display,objwin[count],
			    -WINDOW_BORDERS,-WINDOW_BORDERS,
			    OBTOR_WIN_W,BITMAP_HEIGHT,
			    GRAB_SPACING,fgcolor,bgcolor);
      /* with dummy location */
      XSelectInput(display, infowin[count], ExposureMask );
      XSetWindowBackgroundPixmap(display,infowin[count],info_pix);
      XDefineCursor(display,infowin[count],PirateCursor);
E 2

D 2
load_obtor_cursors()
{
  define_thin_arrow_cursor(obtorwin);
  define_thin_arrow_cursor(statwin);
  define_thin_arrow_cursor(help_rec->win);
  define_thin_arrow_cursor(info_rec->win);
E 2
I 2
      upwin[count] =
	XCreateSimpleWindow(display,objwin[count],
			    -WINDOW_BORDERS,-WINDOW_BORDERS,
			    BITMAP_WIDTH,BITMAP_HEIGHT,
			    GRAB_SPACING,fgcolor,bgcolor);
      /* with dummy location */
      XSelectInput(display, upwin[count], ButtonPressMask);
      XDefineCursor(display,upwin[count],UpArrowCursor);
      XSetWindowBackgroundPixmap(display,upwin[count],up_pix);

      downwin[count] =
	XCreateSimpleWindow(display,objwin[count],
			    -WINDOW_BORDERS,-WINDOW_BORDERS,
			    BITMAP_WIDTH,BITMAP_HEIGHT,
			    GRAB_SPACING,fgcolor,bgcolor);
      /* with dummy location */
      XSelectInput(display, downwin[count], ButtonPressMask);
      XDefineCursor(display,downwin[count],DownArrowCursor);
      XSetWindowBackgroundPixmap(display,downwin[count],down_pix);
    }
  
  set_up_help();
  
  /* set the global redraw procedure */
  GlobalRedraw = redraw_an_obtor_window;
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* set up the windows needed by the object editor */

obtor_window_setup()
E 2
I 2
set_up_help()
E 2
{
D 2
  obtorwin = XCreateSimpleWindow(display, RootWindow(display, screen),
		OBTOR_WIN_X, OBTOR_WIN_Y, OBTOR_WIN_W, OBTOR_WIN_H, 
		WINDOW_BORDERS, fgcolor, bgcolor);
E 2
  
D 2
  objwin[0] = XCreateSimpleWindow(display, obtorwin, - WINDOW_BORDERS, 
		- WINDOW_BORDERS, OBJWIN_W, OBJWIN_H, WINDOW_BORDERS, 
		fgcolor, bgcolor);
E 2
I 2
  help_rec = make_help_window(rootwin, HELP_X, HELP_Y,
			      HELP_COLS, HELP_LINES, HELP_FONT);
  XStoreName(display,help_rec->win,"Obtor Help");
  XDefineCursor(display,help_rec->win,SpiralCursor);
  XSelectInput(display, help_rec->win,
	       KeyPressMask | ExposureMask );
E 2

D 2
  objwin[1] = XCreateSimpleWindow(display, obtorwin, OBJWIN_W,
		- WINDOW_BORDERS, OBJWIN_W, OBJWIN_H, WINDOW_BORDERS, 
		fgcolor, bgcolor);
E 2
I 2
  load_help_strings(help_rec, help_data);
}
E 2

D 2
  obtmsgwin = XCreateSimpleWindow(display, obtorwin, OBTMSG_WIN_X, 
		OBTMSG_WIN_Y, OBTMSG_WIN_W, OBTMSG_WIN_H, WINDOW_BORDERS,
		fgcolor, bgcolor);
E 2
I 2
/*****************************************************************/
E 2

D 2
  statwin = XCreateSimpleWindow(display, RootWindow(display, screen),
		STAT_WIN_X, STAT_WIN_Y, 
		STAT_WIN_W, STAT_WIN_H, WINDOW_BORDERS, fgcolor, bgcolor);
E 2
I 2
load_help_strings(rec, data)
Help *rec;
char *data;
{
  int i;
  char *s,*p;
  extern char *malloc();
E 2

D 2
  XStoreName(display, obtorwin, "Editobj: Object Definitions");
  XStoreName(display, statwin, "Editobj: Status Window");
E 2
I 2
  s = malloc(strlen(data)+1);  /* +1 for /0 */
  if (s == NULL)
    return;
E 2

I 2
  strcpy(s,data);
E 2

D 2
  /* set up the help and information windows */
  info_rec = make_help_window(RootWindow(display, screen), 20, 20, 
			      INFO_COLS, INFO_LINES, INFO_FONT);
  help_rec = make_help_window(RootWindow(display, screen), HELP_X, HELP_Y,
			      HELP_COLS, HELP_LINES, HELP_FONT);
E 2
I 2
  p = strtok(s,"\n");
E 2

D 2
  /* load appropriate text into help window */
  load_help_window_contents(help_rec, OBTOR_HELP_FILE);
  
  /* store the mouse representation bitmap */
  define_mouse_bitmap();
E 2
I 2
  for (i=0; (i<rec->height && p) ; i++)
    {
      strcpy(rec->text[i], p);
      p = strtok(NULL,"\n");
    }

  free(s);
E 2
}

I 2
/*****************************************************************/
E 2

I 2
wait_for_ack()
{
  int what;
  int mx,my,detail;
  char keyp;
  Window where;
E 2

D 2
/* make the previously set up windows visible */
E 2
I 2
  XSetWindowBackgroundPixmap(display,intro2win,intro3_pix);
  XClearWindow(display,intro2win);
  XDefineCursor(display,mainwin,GumbyCursor);
  XBell(display,100);
  XSync(display,True);  /* start waiting for things */
E 2

D 2
map_obtor_windows()
{
  /* start up input in the windows */
  init_input_lines(objwin[0]);
  init_input_lines(objwin[1]);
  init_input_lines(obtmsgwin);
  init_input_lines(statwin);
  init_input_lines(help_rec->win);
  init_input_lines(info_rec->win);
E 2
I 2
  do
    what = get_input(mainwin, &keyp, &mx, &my, &detail, &where);
  while((what != KEYBOARD)&&(what != MOUSE));
E 2

I 2
  XDestroyWindow(display,intro1win);
  XDestroyWindow(display,intro2win);
  XFreePixmap(display,intro1_pix);
  XFreePixmap(display,intro2_pix);  
  XFreePixmap(display,intro3_pix);
}
E 2

I 2
/*****************************************************************/

map_obtor_windows()
{
E 2
  /* map the windows */
D 2
  XMapWindow(display, obtorwin);
  XMapSubwindows(display, obtorwin);
  toggle_message_window(msg_up);
  XMapWindow(display, statwin);
E 2
I 2
  XMapRaised(display, obtorwin);
  XMapRaised(display, obtmsgwin);
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* map or unmap the stat window, map if flag is TRUE, otherwise unmap */

toggle_stat_window(flag)
int flag;
E 2
I 2
FindObtorWindow(win)
     Window win;
E 2
{
D 2
  if (flag) XMapWindow(display, statwin);
  else XUnmapWindow(display, statwin);
E 2
I 2
  int place;
  
  for (place = 0;place < MAX_BUFFERS;place++)
    if (objwin[place] == win)
      return(place);
  
  return(-1);
E 2
}

I 4
/*****************************************************************/
/* change the show numbering mode */
E 4

I 4
SetShowNumbers(int value)
{
  int place;

  if (value == show_numbers)
    return;

  show_numbers = value;

  for (place = 1;place < MAX_BUFFERS;place++)
    redraw_a_grab_window(place);
    
  return;
}


E 4
D 2

E 2
I 2
/*****************************************************************/
E 2
/* set up the variables about what is on the screen */

obtor_setup_variables()
{
D 2
  int i;
E 2
I 2
  int i,j;
  
  for (i = 0;i < WIN_X_SPACE;i++)
    for (j = 0;j < WIN_Y_SPACE;j++)
      window_space[i][j] = -1;
E 2

D 2
  /* clear click buffers */
  for (i=0; i<3; i++) {
    bzero(&(click[i]), sizeof(ObjInfo));
    click_pixmap[i] = (Pixmap) NULL;
    click_pixmap[i] = (Pixmap) NULL;
  }

  /* clear pixmap arrays */
  for (i=0; i<MAX_OBJECTS; i++) {
    item_pixmap[0][i] = (Pixmap) NULL;
    item_mask[0][i] = (Pixmap) NULL;
    item_pixmap[1][i] = (Pixmap) NULL;
    item_mask[1][i] = (Pixmap) NULL;
  }
E 2
I 2
  transmark = -1;
  mass_rec = NULL;
  try_win_up = False;
I 4
  show_numbers = False;
E 4
E 2
}

I 2
/*****************************************************************/
E 2

I 2
ClearXData(group,obj)
{
  item_pixmap[group][obj] = (Pixmap) NULL;
  item_mask[group][obj] = (Pixmap) NULL;
}
E 2

D 2
/* define both sets of object pixmaps */
E 2
I 2
/*****************************************************************/
E 2

D 2
define_starting_pixmap_sets()
E 2
I 2
MakeBlankXData(group,size)
E 2
{
D 2
  one_set_object_pixmaps(0);
  one_set_object_pixmaps(1);
E 2
I 2
  int i;
  
  ResizeXData(group,size);
  
  for (i = 0;i < size;i++)
    ClearXData(group,i);
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* given that the object file is already loaded in, load one of the two
   sets of pixmaps, given 0 or 1 denoting which to do */

one_set_object_pixmaps(num)
int num;
E 2
I 2
ResizeXData(groupnum,size)
     int groupnum,size;
E 2
{
D 2
  free_pixmap_array(item_pixmap[num]);
  free_pixmap_array(item_mask[num]);
  objects_into_pixmaps(objdef[num], item_pixmap[num],
		       item_mask[num], num_objects[num]);
E 2
I 2
D 9
  RESIZE(item_pixmap[groupnum],size,"no more memory for the pixmap array");
  RESIZE(item_mask[groupnum],size,"no more memory for the mask array");
E 9
I 9
  item_pixmap[groupnum] = (Pixmap *) realloc(item_pixmap[groupnum],
					     (sizeof(Pixmap) * size));
  demand(item_pixmap[groupnum], "no more memory for the pixmap array");
  item_mask[groupnum] = (Pixmap *) realloc(item_mask[groupnum],
					     (sizeof(Pixmap) * size));
  demand(item_pixmap[groupnum], "no more memory for the mask array");
E 9
E 2
}

I 2
/*****************************************************************/
E 2

I 2
/* map a given object window into position, resizing
   and clearing as necessary */
E 2

D 2
/* define the objects in a given info array and place pixmaps into 
   the given pixmap array. */

objects_into_pixmaps(objdef, pixlist, masklist, count)
ObjInfo *objdef[];
Pixmap  pixlist[], masklist[];
int count;
E 2
I 2
map_data_window(int which)
E 2
{
D 2
  register int i;
E 2
I 2
  clear_window_space(which);
  /* go and figure a nice place to put the window */
  if (!SetNewWindowRoot(which))
    return(FALSE);
  ExpandWindow(which);
  
  XMapWindow(display,objwin[which]);
  return(TRUE);
}
E 2

D 2
  for (i=0; i<count; i++) {
E 2

D 2
    /* store the bitmap for this object in a pixmap */
    pixlist[i] = XCreatePixmapFromBitmapData(display, 
			RootWindow(display,screen), 
			objdef[(unsigned char)i]->bitmap, BITMAP_WIDTH,
			BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 2
I 2
/*****************************************************************/
E 2

D 2
    /* store the mask for this object, if there is one, in a pixmap */
    if (objdef[(unsigned char)i]->masked) {
      masklist[i] = XCreatePixmapFromBitmapData(display,
			RootWindow(display, screen),
			objdef[(unsigned char)i]->mask, BITMAP_WIDTH,
			BITMAP_HEIGHT, fgcolor, bgcolor, 1);
    }
  }
E 2
I 2
unmap_data_window(int which)
{
  if (transmark == which)
    transmark = -1;
  clear_window_space(which);
  XUnmapWindow(display,objwin[which]);
E 2
}

I 2
/*****************************************************************/
E 2

I 2
/* take the objects in a given info array and place pixmaps into 
   the pixmap array. */
E 2

I 2
objects_into_pixmaps(group)
     int group;
{
  int i;
  
  for (i=0; i<NUM_OBJ(group); i++) 
    certain_object_pixmap(group,i);
}
/*****************************************************************/

E 2
/* redo just one object's pixmap and mask, freeing the old ones if they
   are not NULL */

single_object_pixmap(objdef, pixvar, maskvar)
D 2
ObjInfo *objdef;
Pixmap  *pixvar, *maskvar;
E 2
I 2
     ObjInfo *objdef;
     Pixmap  *pixvar, *maskvar;
E 2
{
D 2
  if (*pixvar) {
    XFreePixmap(display, *pixvar);
    *pixvar = (Pixmap) NULL;
  }
  if (*maskvar) {
    XFreePixmap(display, *maskvar);
    *maskvar = (Pixmap) NULL;
  }
E 2
I 2
  if (*pixvar) 
    {
      XFreePixmap(display, *pixvar);
      *pixvar = (Pixmap) NULL;
    }
  if (*maskvar) 
    {
      XFreePixmap(display, *maskvar);
      *maskvar = (Pixmap) NULL;
    }
E 2
  
  /* load/store the bitmap for this object */
D 2
  *pixvar = XCreatePixmapFromBitmapData(display, 
			RootWindow(display,screen), 
			objdef->bitmap, BITMAP_WIDTH,
			BITMAP_HEIGHT, fgcolor, bgcolor, 1);

E 2
I 2
  *pixvar = XCreatePixmapFromBitmapData(display, rootwin,
					objdef->bitmap, BITMAP_WIDTH,
D 7
					BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 7
I 7
					BITMAP_HEIGHT,
					fgcolor, bgcolor, depth);
E 7
  
E 2
  /* load/store the mask for this object, if it is used */
  if (objdef->masked) {
D 2
    *maskvar = XCreatePixmapFromBitmapData(display, 
			RootWindow(display,screen), 
			objdef->mask, BITMAP_WIDTH,
			BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 2
I 2
    *maskvar = XCreatePixmapFromBitmapData(display, rootwin,
					   objdef->mask, BITMAP_WIDTH,
D 7
					   BITMAP_HEIGHT, fgcolor, bgcolor, 1);
E 7
I 7
					   BITMAP_HEIGHT,
					   fgcolor, bgcolor, depth);
E 7
E 2
  }
}


I 2
/*****************************************************************/
E 2

/* redo a certain object pixmap given group and number */

certain_object_pixmap(groupnum, objnum)
D 2
int groupnum, objnum;
E 2
I 2
     int groupnum, objnum;
E 2
{
D 2
  single_object_pixmap(OBJ_DEF(groupnum, objnum),
E 2
I 2
  single_object_pixmap(OBJ_DEF(groupnum)[objnum],
E 2
		       &(item_pixmap[groupnum][objnum]),
		       &(item_mask[groupnum][objnum]));
}


D 2

/* given a pixmap array, free any defined pixmaps in it */

free_pixmap_array(pixlist)
Pixmap pixlist[];
{
  register int i;

  for (i=0; i<MAX_OBJECTS; i++)
    if (pixlist[i]) {
      XFreePixmap(display, pixlist[i]);
      pixlist[i] = (Pixmap) NULL;
    }
}



E 2
/* ============================== I N P U T ============================== */

/* the main editing procedure for the program */

obtor_get_input()
{
  int   i, x, y, detail, quit = FALSE;
  char  c;
  Window happenwin;
  
D 2
  while (!quit) {

    i = get_input(statwin, &c, &x, &y, &detail, &happenwin);

    switch (i) {
      case NOTHING:	break;
      case KEYBOARD:	quit = handle_obtor_keypress(c, x, y, happenwin);
			break;
      case EXPOSED:	redraw_an_obtor_window(happenwin);
			break;
      case MOUSE:	handle_obtor_mouse(x, y, c, detail, happenwin);
			break;
      default:		break;
E 2
I 2
  while (!quit) 
    {
      i = get_input(obtorwin, &c, &x, &y, &detail, &happenwin);
      
      switch (i) 
	{
	  
	case KEYBOARD:
	  quit = handle_obtor_keypress(c, x, y, happenwin);
	  break;
	case MOUSE:
	  handle_obtor_mouse(x, y, c, detail, happenwin);
	  break;
	default:
	  break;
	}
E 2
    }
D 2
    
  }
  
E 2
}
I 2
/*****************************************************************/
E 2

D 2


E 2
/* handle key press and return TRUE if we should quit, otherwise return 0 */

handle_obtor_keypress(c, x, y, what_win)
D 2
char c;
int x, y;
Window what_win;
E 2
I 2
     char c;
     int x, y;
     Window what_win;
E 2
{
D 2
  /* message area of previous garbage */
  place_obtor_message(" ");
  
  if (isupper(c)) c = tolower(c);
E 2
I 2
  int win,dest,num;
  char message[105];
  /* clear message area of previous garbage */
E 2

D 2
  switch (c) {
	case 'q':	if (!placed_want_query(RootWindow(display, screen),
				OBTOR_WIN_X,
				OBTOR_WIN_Y, OBTOR_WIN_X + OBTOR_WIN_W,
				OBTOR_WIN_Y + OBTOR_WIN_H,
				"Do you really want to quit?")) break;
			opt_to_save_in_windows();
			return TRUE;
			break;
	case '!':	if (!placed_want_query(RootWindow(display, screen),
				OBTOR_WIN_X,
				OBTOR_WIN_Y, OBTOR_WIN_X + OBTOR_WIN_W,
				OBTOR_WIN_Y + OBTOR_WIN_H,
				"Really quit without saving?")) break;
			return TRUE;
			break;
	case 's':	if (what_win == objwin[0]) query_save_win(0);
			else if (what_win == objwin[1]) query_save_win(1);
			else opt_to_save_in_windows();
			break;
	case 'i':	handle_obtor_info_request(what_win, x, y);
			break;
	case '<':	if (what_win == objwin[0]) query_load_info(0);
			else if (what_win == objwin[1]) query_load_info(1);
			else place_obtor_message("Cursor not in a window.");
			break;
	case '>':	if (what_win == objwin[0]) query_save_info(0);
			else if (what_win == objwin[1]) query_save_info(1);
			else place_obtor_message("Cursor not in a window.");
			break;
	case '?':	toggle_help_window(help_rec);
			break;
	case '.':	toggle_message_window(!msg_up);
			break;
	default:	break;
  }
E 2
I 2
  place_obtor_message("");
  win = FindObtorWindow(what_win);
E 2
  
I 2
  switch (c)
    {
    case 'Q':
      opt_to_save_in_windows();
      return TRUE;
      break;
    case 'q':
      if (!placed_want_query(obtorwin,0,0,OBTOR_WIN_W,OBTOR_WIN_H,
			     "Do you really want to quit?")) break;
      opt_to_save_in_windows();
      return TRUE;
      break;
    case '!':
      if (!placed_want_query(obtorwin,0,0,OBTOR_WIN_W,OBTOR_WIN_H,
			     "Really quit without saving?")) break;
      return TRUE;
      break;
    case 's':
      if (win >= 0)
	save_win(win);
      else 
	opt_to_save_in_windows();
      break;
    case 'S':
      opt_to_save_in_windows();
      break;
    case 'n':
      if ((win = FindNewWindow()) >= 0)
	obtor_create_new(win);
I 4
      break;
    case '#':
      SetShowNumbers(!show_numbers);
E 4
      break;
    case 'i':
D 6
      handle_obtor_info_request(what_win, x, y);
E 6
I 6
      handle_obtor_info_request();
E 6
      break;
    case 'a':
      handle_mass_info(what_win, x, y);
      break;
    case 'x':
      if ((win = FindObjToEdit(&num)) > -1)
	delete_obj_from_group(win,num);
      break;
    case 'o':
      if ((win = FindObjToEdit(&num)) > -1)
	insert_obj_in_group(make_blank_definition(),win,num);
      break;
    case 'I':
      if (window_info_up)
	HideWindowInfo();
      else
	ShowWindowInfo();
      break;
    case 'b':
      edit_obtor_bitmap(what_win);
      break;
    case 'm':
      edit_obtor_mask(what_win);
      break;
    case 't':
      if ((win = FindObjToEdit(&num)) > -1)
	if (IS_VALID_OBJ(win,num))
	  set_window_background(win,num);
      break;
    case 'T':
      if (win >= 0)
	set_window_background(win,-1);
      break;
    case 'p':
      if (win >= 0)
	{
	  dest = transmark;
	  transmark = win;
	  sprintf(message,"Mark set in window #%d",win);
	  place_obtor_message(message);
	  redraw_info_window(win);
	  if (dest >= 0)
	    redraw_info_window(dest);
	}
      break;
    case 'u':
      toggle_try_window();
      break;
    case '?':
      toggle_help_window(help_rec);
      break;
    case 'h':
      if (win >= NORM_WINDOW)
	splitwindow(win,True);
      break;
    case 'v':
      if (win >= NORM_WINDOW)
	splitwindow(win,False);
      break;
    case 'l':
      if (win >= NORM_WINDOW)
	get_bitmap(win);
      break;
    case 'L':
      if (win >= NORM_WINDOW)
	get_bitmap_directory(win);
      break;
    case 'e':
      if (win >= NORM_WINDOW)
	ExpandWindow(win);
      break;
      /* Note: The result of these numbers is dependent
	 on WIN_X_SPACE and WIN_Y_SPACE to some extent */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (c == '0') /* calculate for '10' instead of '0' */
	dest = 9;
      else
	dest = c - '1';
      
      if (win >= NORM_WINDOW)
	force_move_window(win,dest);
      break;
    case 'k':
      if (win >= NORM_WINDOW)
	unload_window(win);
      break;
    case 'f':	
      load_file();
      break;
    case 'd':	
      win = FindNewWindow();
      if (win >= NORM_WINDOW)
	obtor_load_object_file(DEFAULT_OBJ_PATH,win);
      break;
    case 'w':
      if (win >= 0)
	write_info(win);
      break;
I 8
    case '.':
      handle_header_edit_request();
      break;
E 8
    default:
      break;
    }
  
E 2
  return FALSE;
}

I 2
/*****************************************************************/
E 2


D 2
handle_obtor_mouse(x, y, button, detail, what_window)
int x, y;
char button;
int detail;
Window what_window;
E 2
I 2
handle_obtor_mouse(x, y, button, detail, what_win)
     int x, y;
     char button;
     int detail;
     Window what_win;
E 2
{
D 2
  int objnum, groupnum;
E 2
I 2
  int win,i,num;
  
  place_obtor_message("");
E 2

D 2
  if (what_window == objwin[0] || what_window == objwin[1]) {
    groupnum = ((what_window == objwin[0]) ? 0 : 1);
    objnum = obtor_obj_in_grab_win(x, y);
    if (objnum >= 0) {
      if (detail & ShiftMask) 
        switch (button) {
		case Button1:
				copy_click_out_to_group(0, groupnum, objnum);
				break;
		case Button2:
				copy_click_out_to_group(1, groupnum, objnum);
				break;
		case Button3:
				copy_click_out_to_group(2, groupnum, objnum);
				break;
		default:	break;
	}
      else
        switch (button) {
		case Button1:
			if (IS_VALID_OBJ(groupnum, objnum))
			  put_def_in_click(OBJ_DEF(groupnum, objnum), 0);
			break;
		case Button2:
			if (IS_VALID_OBJ(groupnum, objnum))
			  put_def_in_click(OBJ_DEF(groupnum, objnum), 1);
			break;
		case Button3:
			if (IS_VALID_OBJ(groupnum, objnum))
			  put_def_in_click(OBJ_DEF(groupnum, objnum), 2);
			break;
		default:break;
	}
E 2
I 2
  if ((win = FindObtorWindow(what_win)) >= 0)
    switch (button)
      {
      case Button1:
	drag_object(win,x,y);
	break;
      case Button3:
D 6
	handle_obtor_info_request(what_win, x, y);
E 6
I 6
	handle_obtor_info_request();
E 6
	break;
      case Button2:
	if (transmark >= 0)
	  {
	    num = obtor_obj_in_grab_win(x,y,win);
	    if (IS_VALID_OBJ(win,num))
D 3
	      copy_obj_to_group(objdef[win][num],transmark,
				num_objects[transmark]);
E 3
I 3
	      recursive_transfer(win,num,transmark);
E 3
	  }
	else
	  {
	    XBell(display,100);
	    place_obtor_message("The destination mark is not set.");
	  }
	break;
      }
  
  for (i = 0; i < MAX_BUFFERS;i++)
    {
      if (upwin[i] == what_win)
	ClickArrowWin(i,(int) button,True);
      if (downwin[i] == what_win)
	ClickArrowWin(i,(int) button,False);
E 2
    }
D 2
  }
  else if (what_window == statwin) 
    handle_obtor_statwin_mouse(x, y, button, detail);
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* check to see where the mouse was pressed in the status window and
   from that determine what we should do about it */

handle_obtor_statwin_mouse(x, y, button, detail)
int x, y;
char button;
int detail;
{
}




E 2
/* get a line of input in obtor message area */

char *get_obtor_input_line(prompt, maxlen)
D 2
char *prompt;
int maxlen;
E 2
I 2
     char *prompt;
     int maxlen;
E 2
{
  int exposed;
D 2
  
E 2
I 2
  char *foo;

E 2
  /* clear the message area */
  XClearWindow(display, obtmsgwin);
D 2
	     
  return get_string(obtmsgwin, regfont, OBTMSG_PAD, OBTMSG_PAD,
		    prompt, maxlen, &exposed);
E 2
I 2
  
  foo = get_string(obtmsgwin, regfont, OBTMSG_PAD, OBTMSG_PAD,
		   prompt, maxlen, &exposed);
  place_obtor_message("");
  return(foo);
  
E 2
}


D 2

E 2
/* ask user if he wishes to save the two files */

opt_to_save_in_windows()
{
D 2
  query_save_win(0);
  query_save_win(1);
E 2
I 2
  int place;
  
  for (place = NORM_WINDOW;place < MAX_BUFFERS;place++)
    if (data_loaded[place])
      query_save_win(place);
E 2
}


D 2

E 2
/* ask the user for yes or no on saving one of the object groups */

query_save_win(num)
D 2
int num;
E 2
I 2
     int num;
E 2
{
D 2
  if (placed_want_query(objwin[num], 0, 0, OBJWIN_W, OBJWIN_H,
			"Save these (y/n)?")) {
    obtor_save_object_file(objfile_name[num], num);
  }
}


/* ask the user for yes or no on loading info about a group */

query_load_info(num)
int num;
{
  char *filename;

  if (placed_want_query(objwin[num], 0, 0, OBJWIN_W, OBJWIN_H,
			"Load new info (y/n)?")) {
    filename = get_obtor_input_line("Filename: ", 119);
    if (strlen(filename)) {
      if (overwrite_record_info(objdef[num], filename))
        place_obtor_message("Done.");
      else
        place_obtor_message("Error.");
E 2
I 2
  int letter;
  
  if (!file_changed[num])
    return(True);
  
  letter = placed_letter_query(objwin[num], 0, 0, OBJWIN_W(num), OBJWIN_H(num),
			       "Save these (y/n/c)?","ync");
  
  if (letter == 'y')
    {
      save_win(num);
      return(True);
E 2
    }
D 2
    else place_obtor_message("No File Loaded.");
  }
E 2
I 2
  
  if (letter == 'n')
    return(True);
  
  return(False);    /* cancel */
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* ask the user for yes or no on saving info about a group */

query_save_info(num)
int num;
E 2
I 2
save_win(num)
     int num;
E 2
{
  char *filename;
D 2

  if (placed_want_query(objwin[num], 0, 0, OBJWIN_W, OBJWIN_H,
			"Write info out (y/n)?")) {
    filename = get_obtor_input_line("Filename: ", 119);
    if (strlen(filename)) {
      if (write_out_record_info(objdef[num], num_objects[num], filename))
        place_obtor_message("Done.");
      else
        place_obtor_message("Error.");
E 2
I 2
  
  if (strlen(objfile_name[num]) == 0)
    {
      filename = get_obtor_input_line("Filename: ", 119);
      if (strlen(filename) == 0)
	{
	  place_obtor_message("No file saved");
	  return(False);
	}
      strcpy(objfile_name[num],filename);
      redraw_info_window(num);
E 2
    }
D 2
    else place_obtor_message("No File Saved.");
  }
}
E 2
I 2
  
  if (!obtor_save_object_file(objfile_name[num],num))
    return(False);
E 2

D 2


/* ======================================================================= */

/* find out which object should be in the x y coords of the grab window,
   if they click on an object that doesn't exist, return -1 instead of the
   expected object number */

int obtor_obj_in_grab_win(x, y)
int x, y;
{
  int line, col, result;

  line = y / (BITMAP_HEIGHT + GRAB_SPACING);
  col = x / (BITMAP_WIDTH + GRAB_SPACING);

  result = (line * GRAB_WIN_WIDTH) + col;
  if (result < 0 | result >= MAX_OBJECTS) result = -1;
  return result;
E 2
I 2
    
  return(True);
E 2
}

I 2
/* ask the user for yes or no on loading info about a group */
E 2

D 2

/* place an object into click buffer */

put_def_in_click(def, clicknum)
ObjInfo *def;
int clicknum;
E 2
I 2
load_file()
E 2
{
D 2
  /* copy the object definition into click buffer */
  bcopy(def, &(click[clicknum]), sizeof(ObjInfo));
E 2
I 2
  char *filename,message[1024];
  int which;
  
  filename = get_obtor_input_line("Filename: ", 119);
  if (strlen(filename)) 
    {
      sprintf(message,"Loading %s...",filename);
      place_obtor_message(message);
E 2

D 2
  /* free previous pixmap(s) if there were any and make new pixmap(s) */
  single_object_pixmap(&(click[clicknum]), &(click_pixmap[clicknum]),
		       &(click_mask[clicknum]));

  /* redraw click buffer */
  redraw_one_click_buffer(clicknum);
E 2
I 2
      which = FindNewWindow();
      if (which >= 0)
	{
	  if (obtor_load_object_file(filename,which))
	    sprintf(message,"%s loaded.",filename);
	  else
	    sprintf(message,"Error, %s not loaded.",filename);
	  
	  place_obtor_message(message);	
	  return(0);
	}
    }
  place_obtor_message("No File Loaded.");
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* copy the definition from a click buffer out one of the object groups.  If
   there aren't that many objects in the group (0 or 1) then make a bunch of
   blank ones to fill in the space */

copy_click_out_to_group(clicknum, groupnum, objnum)
int clicknum, groupnum, objnum;
E 2
I 2
write_info(num)
     int num;
E 2
{
D 2
  ObjInfo *copy;

  /* if there aren't enough objects in this group yet, then fill it up */
  if (!IS_VALID_OBJ(groupnum, objnum))
    fill_with_blanks_upto(groupnum, objnum);

  /* allocate a copy of the click buffer's definition */
  copy = copy_obj_definition(&(click[clicknum]));

  /* replace the old definition with this new copy */
  replace_obj_definition(&(objdef[groupnum][objnum]), copy);
  file_changed[groupnum]++;

  /* update the pixmaps for the object */
  single_object_pixmap(OBJ_DEF(groupnum, objnum),
		       &(item_pixmap[groupnum][objnum]),
		       &(item_mask[groupnum][objnum]));
  redraw_grab_square(groupnum, objnum);
E 2
I 2
  char *filename,oldname[1024],message[1024];
  
  filename = get_obtor_input_line("Filename: ", 119);
  if (strlen(filename))
    {
      strcpy(oldname,objfile_name[num]);
      strcpy(objfile_name[num],filename);
      sprintf(message,"Saving %s...",filename);
      place_obtor_message(message);
      if (!save_win(num))
	{
	  sprintf(message,"Error, not saved as %s.",filename);
	  place_obtor_message(message);
	  strcpy(objfile_name[num],oldname);
	  return(False);
	}
      sprintf(message,"File saved as %s.",filename);
      redraw_info_window(num);
      place_obtor_message(message);
    }
  else
    place_obtor_message("No File Saved.");
E 2
}



/* ======================== O U T P U T =============================== */


/* redraw the indicated window for the obtor */

redraw_an_obtor_window(win)
D 2
Window win;
E 2
I 2
     Window win;
E 2
{
D 2
  if (win == objwin[0]) redraw_a_grab_window(0);
  if (win == objwin[1]) redraw_a_grab_window(1);
  else if (win == statwin) redraw_obtor_stat_window();
}




/* draw the contents of the object grabbing window */

redraw_a_grab_window(num)
int num;
{
  int i, j, nextobj = 0;
  int x, y;

  for (j=0; j<GRAB_WIN_HEIGHT && nextobj<num_objects[num]; j++)
    for (i=0; i<GRAB_WIN_WIDTH && nextobj<num_objects[num]; i++) {
      x = (i * (BITMAP_WIDTH + GRAB_SPACING)) + GRAB_SPACING / 2;
      y = (j * (BITMAP_HEIGHT + GRAB_SPACING)) + GRAB_SPACING / 2;
      obtor_draw_object(objwin[num], x, y, objdef[num][nextobj],
			item_pixmap[num][nextobj], item_mask[num][nextobj],
			FALSE);
      nextobj++;
    }
}


/* redraw just one object in one grab window */

redraw_grab_square(groupnum, objnum)
int groupnum, objnum;
{
  int i, j, x, y;

  if (IS_VALID_OBJ(groupnum, objnum)) {
      i = objnum % GRAB_WIN_WIDTH;
      j = objnum / GRAB_WIN_WIDTH;
      x = (i * (BITMAP_WIDTH + GRAB_SPACING)) + GRAB_SPACING / 2;
      y = (j * (BITMAP_HEIGHT + GRAB_SPACING)) + GRAB_SPACING / 2;
      obtor_draw_object(objwin[groupnum], x, y, objdef[groupnum][objnum],
			item_pixmap[groupnum][objnum],
			item_mask[groupnum][objnum], FALSE);
  }
}



/* place an object pixmap on screen, if there is a mask then place it
   on first.  You must give the window and pixel x and y coords to
   draw at.  You must specify the object definition, pixmap, and mask,
   and whether we can assume the spot underneath has nothing at all
   drawn on it already */

obtor_draw_object(win, x, y, def, pix, mask, is_clear_under)
Window win;
int x, y;
ObjInfo *def;
Pixmap pix, mask;
int is_clear_under;
{
  int func;

  /* if there is a mask but screen is clear underneath, skip the draw */
  if (!def->masked || is_clear_under) {
    if (pix) 
      XCopyArea(display, pix, win, mainGC, 0, 0, 
	      BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
  }
  else {
    if (mask)
      XCopyArea(display, mask, win, maskGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
    if (pix) 
      XCopyArea(display, pix, win, drawGC, 0, 0,
	      BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
  }
}




/* redraw the stat window */

redraw_obtor_stat_window()
{
  redraw_obtor_click_buffer();
}



/* redraws the graphic representation of the mouse with the three click
   buffers inside */

redraw_obtor_click_buffer()
{
  int i;

  /* place the mouse picture pixmap */
  place_mouse_picture(statwin, CLICK_X, CLICK_Y);
E 2
I 2
  int winnum;
E 2
  
D 2
  /* place each of the three click buffer images */
  for (i=0; i<3; i++)
    redraw_one_click_buffer(i);
}
E 2
I 2
  if ((winnum = FindObtorWindow(win)) >= 0) 
    redraw_a_grab_window(winnum);
  
  for (winnum = 0;winnum < MAX_BUFFERS;winnum++)
    if (infowin[winnum] == win)
      redraw_info_window(winnum);
  
  if (win == obtmsgwin)
    place_obtor_message(NULL);
E 2

I 2
  if (win == trywin)
    draw_try_window();
E 2

D 2

/* draw the contents of one of the click buffer boxes */

redraw_one_click_buffer(num)
int num;
{
  int wx, wy;

  /* place the object in the right box on the mouse picture */
  /* find the proper place to put the block */
  wx = CLICK_X + MOUSE_START_X + num * (BITMAP_WIDTH + MOUSE_SPACING);
  wy = CLICK_Y + MOUSE_START_Y;

  /* place the given object at the spot */
  obtor_draw_object(statwin, wx, wy, &(click[num]), click_pixmap[num],
		    click_mask[num], FALSE);
E 2
I 2
  check_help_expose(win);
E 2
}

D 2



/* toggle whether the message window is mapped or not, depending on the
   global 'is message window mapped?' variable "msg_up" */

toggle_message_window(up)
int up;
{
  if (up) {
    msg_up = TRUE;
    XMapWindow(display, obtmsgwin);
    XRaiseWindow(display, obtmsgwin);
  }
  else {
    msg_up = FALSE;
    XUnmapWindow(display, obtmsgwin);
  }
}


E 2
/* place a message in the stat window in the appropriate font.  If a null
   length message is given as the message, then redraw the last message */

place_obtor_message(line)
D 2
char *line;
E 2
I 2
     char *line;
E 2
{
D 2
  static int message_inited = FALSE;
  static char current_msg[120];
E 2
I 2
  static char current_msg[120] = "";
E 2

D 2
  if (!message_inited) {
    current_msg[0] = '\0';
    message_inited = TRUE;
  }
  if (strlen(line) != 0) strcpy(current_msg, line);
E 2

I 2
  if (line != NULL)
    strcpy(current_msg, line);
    
  
E 2
  XClearWindow(display, obtmsgwin);
  text_write(obtmsgwin, regfont, OBTMSG_PAD, OBTMSG_PAD, 0, 0, current_msg);
I 2
  XFlush(display);  /* make sure this is shown now! */
E 2
}


D 2

E 2
/* place message about the fact that map was just saved on the screen */

obtor_save_msg()
{
  char msg[80];
  
  strcpy(msg, "Saved.");
  place_obtor_message(msg);
}

I 2
/* place message about the fact that map was not saved on the screen */
E 2

D 2

/* ============================ I N F O requests ========================= */

/* handle a request for information on a square, results depend on which
   window it is called from.  If there is already an info window up, this
   command puts it away. */

handle_obtor_info_request(win, x, y)
Window win;
int x, y;
E 2
I 2
obtor_nosave_msg()
E 2
{
D 2
  int newx, newy;	/* where in RootWindow to place the info window */
  int wwide, whigh;	/* size of info window */
  int dummyx, dummyy, dummykeys;
  Window dummy1, dummy2;

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
E 2
I 2
  char msg[80];
E 2
  
D 2

    /* get the right information placed into the window */
    clear_help_window(info_rec);
    if (win == objwin[0])
      get_object_info(info_rec, 0, obtor_obj_in_grab_win(x, y));
    else if (win == objwin[1])
      get_object_info(info_rec, 1, obtor_obj_in_grab_win(x, y));
    else if (win == statwin)
      get_status_info(info_rec);
    else return;

    /* now make the window appear */
    show_help_window(info_rec);
  }
E 2
I 2
  strcpy(msg, "Error, file not saved.");
  XBell(display,100);
  place_obtor_message(msg);
E 2
}



D 2
/* get the information about an object and place into info help window */
E 2
I 2
/*****************************************************************/
E 2

D 2
get_object_info(rec, which_objfile, objnum)
Help *rec;
int  which_objfile, objnum;
E 2
I 2
FindObjToEdit(num)
     int *num;
E 2
{
D 2
  char s[200];
E 2
I 2
  int groupnum;
E 2
  
D 2
  /* check to make sure this is one of the defined objects */
  if (objnum >= num_objects[which_objfile] | objnum < 0) {
    set_line(rec, 0, "  undefined object");
    return;
  }

  /* first line is number and name of object */
  sprintf(s, "  #%3d %s", objnum, objdef[which_objfile][objnum]->name);
  set_line(rec, 0, s);

  /* set up the following lines to contain the flag text for the obj */
  set_lines_long_text(rec, 1,object_info_line(objdef[which_objfile][objnum]));
E 2
I 2
  groupnum = FindObtorWindow(last_event.xany.window);
  if (groupnum >= 0)
    *num = 
      obtor_obj_in_grab_win(last_event.xkey.x,last_event.xkey.y,groupnum);
  
  if (*num < 0) groupnum = -1;
  return(groupnum);
E 2
}

I 2
/*****************************************************************/
E 2

D 2

/* get the information about the map in general and place in the
   info help window */

get_status_info(rec)
Help *rec;
{
  char s[80];
  
  set_line(rec, 1, " use 'i' in main window");
}
E 2
E 1
