h45428
s 00000/00000/00052
d D 1.5 92/08/07 01:01:31 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00001/00000/00051
d D 1.4 91/12/03 17:29:08 labc-4lc 4 3
c implemented header items for object def files
e
s 00003/00000/00048
d D 1.3 91/07/07 18:47:13 labc-3id 3 2
c made messages from object files be loaded and saved right
e
s 00001/00001/00047
d D 1.2 91/05/17 02:01:33 labc-3id 2 1
c Added code for showing object numbers using '#'
e
s 00048/00000/00000
d D 1.1 91/03/24 18:44:57 labc-3id 1 0
c date and time created 91/03/24 18:44:57 by labc-3id
e
u
U
f e 0
t
T
I 3
/* Obtor's global variables */

E 3
I 1
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
D 2
int     try_win_up;
E 2
I 2
int     try_win_up,show_numbers;
E 2
Pixmap  try_masks,try_pixmaps;
int     mass_field,mass_changed;
Dialog  *mass_rec;

/*****************************************************************/

char    *temp_files[MAX_TEMPFILES];
char	objfile_name[MAX_BUFFERS][PATH_LENGTH];   	
ObjInfo **objdef[MAX_BUFFERS];
I 4
ObjHeader *objheader[MAX_BUFFERS];
E 4
I 3
VLine	*objfile_msgs[MAX_BUFFERS];
E 3
int	num_objects[MAX_BUFFERS];
int	file_changed[MAX_BUFFERS];
int     access_time[MAX_BUFFERS];
int     data_loaded[MAX_BUFFERS];
int     CurrentAccessTime = 0;

/*****************************************************************/
E 1
