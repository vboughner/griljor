#include <stdio.h>
#include <X11/Xlib.h>

/* These might need to be changed */

#define		BIGX_FONT_PATH		BIG_FONT_PATH
#define		REGX_FONT_PATH		REG_FONT_PATH
#define		TINYX_FONT_PATH		TINY_FONT_PATH

#define		WINDOW_WIDTH		(500 + 4)
#define		WINDOW_HEIGHT		750
#define		WINDOW_BORDER		2
#define		WINDOW_X		50
#define		WINDOW_Y		50

/* For the menu window */

#define		MENU_WINDOW_WIDTH	(500 - 2)
#define		MENU_WINDOW_HEIGHT	36
#define		MENU_WINDOW_BORDER	2
#define		MENU_WINDOW_X		0
#define		MENU_WINDOW_Y		710

/* For the player_attributes window */

#define		ATTR_WINDOW_WIDTH	(500 - 2)
#define		ATTR_WINDOW_HEIGHT	280
#define		ATTR_WINDOW_BORDER	2
#define		ATTR_WINDOW_X		0
#define		ATTR_WINDOW_Y		408

/* For text-line input */

#define		TEXT_Y_OFFSET		6
#define		TEXT_LINE_X		5
#define TEXT_LINE_Y (ATTR_WINDOW_Y + ATTR_WINDOW_HEIGHT + TEXT_Y_OFFSET)
#define		MAX_TEXT_LINE_LENGHT	50

/* For the slider window */

#define		SLID_WINDOW_WIDTH	30
#define		SLID_WINDOW_HEIGHT	170
#define		SLID_WINDOW_BORDER	2
#define		SLID_WINDOW_X		0
#define		SLID_WINDOW_Y		236

/* for the bitmap window */

#define		BITM_WINDOW_WIDTH	(500 - 2)
#define		BITM_WINDOW_HEIGHT	180
#define		BITM_WINDOW_BORDER	2
#define		BITM_WINDOW_X		0
#define		BITM_WINDOW_Y		56

typedef struct _sub_window {
Window		win;
int		num_selections;
} sub_window;

typedef sub_window	*inferior;

#define		MAXSTRLEN2	201 /* Maximum size of stuff typed in */
typedef struct _disp {

Display		*display;
int		screen;
Window		win;
GC		regular_gc;
GC		regular_erase_gc;
GC		inversion_gc;
int		fgcolor,bgcolor;
inferior	bitmap_win;
inferior	player_attributes_win;
inferior	menu_win;
inferior	slider_win;
XFontStruct	*bigfont;
XFontStruct	*regfont;
XFontStruct	*tinyfont;
PersonLink      *file;
PersonLink	*end;
PersonLink	*current_edit;
int		current_edit_no;
char		line[MAXSTRLEN2];
char		line2[MAXSTRLEN2];
char		bm_m_file[MAXSTRLEN2];
char		bm_file[MAXSTRLEN2];
int		MaxUsers,user;
Pixmap		icon;
Pixmap		disc_icon;
Pixmap		slider_bitmap;
Pixmap		person_bitmap;
Pixmap		person_mask;
Pixmap		grey;
Pixmap		pict;
Pixmap		memory_bitmap;
Pixmap		memory_mask;
Pixmap		title;
boolean		scroll_flash;
boolean		high_light_off_screen;
char		message[MAXSTRLEN2];
boolean		message_there;
boolean		processor_on;
int		button_value;
boolean		show_bitmaps;
boolean		grid_on;
int		slave_x,slave_y;
boolean		attr_win_expose;
boolean		memory_bm, memory_m;
boolean		genocide_mode;
boolean		genocide_kluge;
boolean		erase_background;

} disp_struct;

typedef  disp_struct	*disp_fields;

#define		LEAVING		1
#define		EXPOSE_MENU	2
#define		PROC_SPEED	12
#define		NUM_SELECTIONS	12
#define		WAIT_CYCLES	40000
