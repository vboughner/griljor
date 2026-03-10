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
#include "vline.h"
#include "editobj.h"
#include "dialog.h"
#include "objheader.h"
#include "objinfo.h"

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
#define MASS_FONT	(regfont)	/* font to use in mass edit window */
#define DIAG_FONT	(regfont)	/* font to use in dialog window */


/* global variables */

extern char    *temp_files[MAX_TEMPFILES];
extern char	objfile_name[MAX_BUFFERS][PATH_LENGTH];   	
                                        /* names of object def files */
extern ObjInfo **objdef[MAX_BUFFERS];	
                                        /* info on object definitions */
extern int	num_objects[MAX_BUFFERS];
                                   	/* how many objects defined */
extern ObjHeader *objheader[MAX_BUFFERS];
					/* obj file info headers */
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
extern int     try_win_up,show_numbers;
extern Pixmap  try_masks,try_pixmaps;
extern int     mass_field,mass_changed;
extern Dialog  *mass_rec;

/* procedure declarations */
char *get_obtor_input_line();
int query_save_win();
int redraw_an_obtor_window();
ObjInfo *copy_obj_definition();
ObjInfo *make_blank_definition();







