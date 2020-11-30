/* Obtor's global variables */

#include "obtor.h"

Window	objwin[MAX_BUFFERS];
Window  infowin[MAX_BUFFERS];
Window  upwin[MAX_BUFFERS];
Window  downwin[MAX_BUFFERS];
Pixmap  try_pix[MAX_BUFFERS];
int grab_win_width[MAX_BUFFERS];
int grab_win_height[MAX_BUFFERS];
Pixmap	*item_pixmap[MAX_BUFFERS];	
Pixmap	*item_mask[MAX_BUFFERS];	
int baseobj[MAX_BUFFERS];

int window_space[WIN_X_SPACE][WIN_Y_SPACE];

Window	obtorwin;
Window	obtmsgwin;
Window  mainwin;   
Window  rootwin;   
Window  trywin;
Window  intro1win,intro2win;
Help	*help_rec;
Pixmap  backg_pix,border_pix,up_pix,down_pix,icon_pix,nothing_pix;
Pixmap  intro1_pix,intro2_pix,intro3_pix,foo_pix,info_pix;
int     window_info_up = FALSE;
Cursor  GumbyCursor,HandCursor,SpiralCursor,ArrowCursor,DragCursor;
Cursor  UpArrowCursor,DownArrowCursor,PirateCursor,WatchCursor;
int     intro1w,intro2w,intro1h,intro2h;
int     all_shown[MAX_BUFFERS],arrowtop[MAX_BUFFERS];
int     arrowbottom[MAX_BUFFERS];
int     transmark;
int     try_win_up,show_numbers;
Pixmap  try_masks,try_pixmaps;
int     mass_field,mass_changed;
Dialog  *mass_rec;

/*****************************************************************/

char    *temp_files[MAX_TEMPFILES];
char	objfile_name[MAX_BUFFERS][PATH_LENGTH];   	
ObjInfo **objdef[MAX_BUFFERS];
ObjHeader *objheader[MAX_BUFFERS];
VLine	*objfile_msgs[MAX_BUFFERS];
int	num_objects[MAX_BUFFERS];
int	file_changed[MAX_BUFFERS];
int     access_time[MAX_BUFFERS];
int     data_loaded[MAX_BUFFERS];
int     CurrentAccessTime = 0;

/*****************************************************************/
