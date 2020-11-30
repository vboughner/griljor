h48137
s 00000/00000/00160
d D 1.7 92/08/07 01:02:57 vbo 7 6
c source copied to a separate tree for work on new map and object format
e
s 00003/00000/00157
d D 1.6 91/12/03 17:30:01 labc-4lc 6 5
c implemented header items for object def files
e
s 00001/00000/00156
d D 1.5 91/09/05 21:28:43 labb-3li 5 4
c made obtor use the new objprops.c code for dialog lists
e
s 00001/00000/00155
d D 1.4 91/07/07 18:47:16 labc-3id 4 3
c made messages from object files be loaded and saved right
e
s 00003/00000/00152
d D 1.3 91/06/30 20:27:51 labc-3id 3 2
c fixed up editing order of properties for obtor
e
s 00001/00001/00151
d D 1.2 91/05/17 02:01:43 labc-3id 2 1
c Added code for showing object numbers using '#'
e
s 00152/00000/00000
d D 1.1 91/03/24 18:45:23 labc-3id 1 0
c date and time created 91/03/24 18:45:23 by labc-3id
e
u
U
f e 0
t
T
I 1
/*********************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 ********************************************************************/

#include "windowsX11.h"
#include "X11/cursorfont.h"
#include "def.h"
I 4
#include "vline.h"
E 4
#include "editobj.h"
#include "dialog.h"
I 5
#include "objheader.h"
I 6
#include "objinfo.h"
E 6
E 5

/* Header file for map object editor */

#define MAX_BUFFERS     10
#define MAX_TEMPFILES   5
#define NORM_WINDOW     1

#define IS_VALID_OBJ(group, num)      ((group < 0) ? False : \
				       ((num < num_objects[group])&& \
					(num >= 0)))
#define OBJ_DEF(group)		        (objdef[group])
#define NUM_OBJ(group)                  (num_objects[group])

#define MAIN_WIN_X		10
#define MAIN_WIN_Y		10
#define INTRO_X                 30
#define INTRO_Y                 (MAIN_WIN_H/4*3)
#define MAIN_WIN_W              OBTOR_WIN_W
#define MAIN_WIN_H              (OBTOR_WIN_H + OBTMSG_WIN_H + WINDOW_BORDERS)
#define GRAB_SPACING		WINDOW_BORDERS	
#define OBTOR_WIN_X		(-WINDOW_BORDERS)
#define OBTOR_WIN_Y		(OBTMSG_WIN_H)
#define OBTOR_WIN_W             ((BITMAP_WIDTH + GRAB_SPACING) * SPACE_SIZE* \
				  WIN_X_SPACE - GRAB_SPACING)
#define OBTOR_WIN_H             ((BITMAP_HEIGHT + GRAB_SPACING) *SPACE_SIZE* \
				 WIN_Y_SPACE - GRAB_SPACING + CUTWIN_H + \
				 WINDOW_BORDERS)
#define OBJWIN_W(winnum)	(grab_win_width[winnum] * \
				 (BITMAP_WIDTH + GRAB_SPACING) - GRAB_SPACING)
#define OBJWIN_H(winnum)	(grab_win_height[winnum] * \
				 (BITMAP_HEIGHT + GRAB_SPACING) - GRAB_SPACING)
#define OBTMSG_WIN_X		(- WINDOW_BORDERS)
#define OBTMSG_WIN_Y		(- WINDOW_BORDERS)
#define OBTMSG_PAD		(4)
#define OBTINFO_PAD		(1)
#define OBTMSG_WIN_W		(MAIN_WIN_W)
#define OBTMSG_WIN_H		(char_height(regfont) + OBTMSG_PAD * 2)
#define CUTWIN_R                2
#define CUTWIN_X                (-WINDOW_BORDERS)
#define CUTWIN_Y                (OBTOR_WIN_H - CUTWIN_H - WINDOW_BORDERS)
#define CUTWIN_H                ((BITMAP_HEIGHT + GRAB_SPACING)*CUTWIN_R - \
                                GRAB_SPACING)
#define CUTWIN_W                OBTOR_WIN_W
#define MASSPAD                 2

#define TRY_WIN_X               10 
#define TRY_WIN_Y               10
#define TRY_WIN_W               (TRY_SIZE*BITMAP_WIDTH)
#define TRY_WIN_H              (TRY_SIZE*BITMAP_HEIGHT)

#define WIN_X_SPACE              (4)
#define WIN_Y_SPACE              (2)
#define SPACE_SIZE               (8)
#define TRY_SIZE                8

/* constants used by information window */
#define INFO_LINES	5		/* num of lines in window */
#define INFO_COLS	30		/* num of columns window will hold */
#define INFO_FONT	(regfont)	/* font to use in help window */
#define MAX_CUTOBJS     100    /* max  items in the cut-buffer */
#define CUT_DELJUMP     40    /* delete at a time */

/* constants used by help window */
#define HELP_X		(OBTOR_WIN_X)
#define HELP_Y		(OBTOR_WIN_H - char_height((HELP_FONT)) * HELP_LINES \
			 - WINDOW_BORDERS)
#define HELP_LINES	25		/* num of lines in window */
#define HELP_COLS	80		/* num of columns window will hold */
#define HELP_FONT	(regfont)	/* font to use in help window */
I 3
#define MASS_FONT	(regfont)	/* font to use in mass edit window */
#define DIAG_FONT	(regfont)	/* font to use in dialog window */

E 3

/* global variables */

extern char    *temp_files[MAX_TEMPFILES];
extern char	objfile_name[MAX_BUFFERS][PATH_LENGTH];   	
                                        /* names of object def files */
extern ObjInfo **objdef[MAX_BUFFERS];	
                                        /* info on object definitions */
extern int	num_objects[MAX_BUFFERS];
                                   	/* how many objects defined */
I 6
extern ObjHeader *objheader[MAX_BUFFERS];
					/* obj file info headers */
E 6
extern int	file_changed[MAX_BUFFERS];
                                    	/* TRUE when a change has been made */
extern int      access_time[MAX_BUFFERS]; 
                                        /* last time of access */
extern int     data_loaded[MAX_BUFFERS];       
                                        /* True is other dada is valid */
extern int      CurrentAccessTime;

extern Window	obtorwin;	/* main parent window for obtor windows */
extern Window	objwin[MAX_BUFFERS];	
extern Window   infowin[MAX_BUFFERS];
extern Window   upwin[MAX_BUFFERS];
extern Window   downwin[MAX_BUFFERS];
extern Pixmap   try_pix[MAX_BUFFERS];
extern int	grab_win_width[MAX_BUFFERS];
extern int	grab_win_height[MAX_BUFFERS];
extern Pixmap	*item_pixmap[MAX_BUFFERS];	
extern Pixmap	*item_mask[MAX_BUFFERS];	
extern int      baseobj[MAX_BUFFERS];

extern int window_space[WIN_X_SPACE][WIN_Y_SPACE];

/*****************************************************************/
extern Window  intro1win,intro2win;
extern Window  obtmsgwin;	/* window for messages and input */
extern Window  mainwin;
extern Window  rootwin;
extern Window  trywin;
extern Help    *help_rec;	/* record containing info for help window */
extern int     msg_up;	        /* TRUE when message window is mapped */
extern Pixmap  backg_pix,border_pix,up_pix,down_pix,icon_pix,nothing_pix;
extern Pixmap  intro1_pix,intro2_pix,intro3_pix,foo_pix,info_pix;
extern int     window_info_up;
extern Cursor  GumbyCursor,HandCursor,SpiralCursor,ArrowCursor,DragCursor;
extern Cursor  UpArrowCursor,DownArrowCursor,PirateCursor,WatchCursor;
extern int     intro1w,intro2w,intro1h,intro2h;
extern int     all_shown[MAX_BUFFERS],arrowtop[MAX_BUFFERS];
extern int     arrowbottom[MAX_BUFFERS];
extern int     transmark;
D 2
extern int     try_win_up;
E 2
I 2
extern int     try_win_up,show_numbers;
E 2
extern Pixmap  try_masks,try_pixmaps;
extern int     mass_field,mass_changed;
extern Dialog  *mass_rec;

/* procedure declarations */
char *get_obtor_input_line();
int query_save_win();
int redraw_an_obtor_window();
ObjInfo *copy_obj_definition();
ObjInfo *make_blank_definition();







E 1
