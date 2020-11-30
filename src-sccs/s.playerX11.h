h44324
s 00000/00000/00098
d D 1.7 92/08/07 01:03:25 vbo 7 6
c source copied to a separate tree for work on new map and object format
e
s 00007/00005/00091
d D 1.6 92/01/20 16:28:03 labc-4lc 6 5
c added some of code necessary for square contents window
e
s 00002/00001/00094
d D 1.5 91/12/18 20:15:53 labc-4lc 5 4
c made message windows handle messages of multiple lines
e
s 00000/00001/00095
d D 1.4 91/12/08 03:33:53 labc-4lc 4 3
c implemented usage messages for objects
e
s 00001/00000/00095
d D 1.3 91/12/08 01:44:49 labc-4lc 3 2
c added description code for info on objects
e
s 00008/00004/00087
d D 1.2 91/12/07 17:38:26 labc-4lc 2 1
c made error message window bigger and scrollable
e
s 00091/00000/00000
d D 1.1 91/02/16 13:01:11 labc-3id 1 0
c date and time created 91/02/16 13:01:11 by labc-3id
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
 * January 1991
 **************************************************************************/

/* Header file for the many X routines used by player program */

/* Window placement definitions */

#define PLAYER_MAIN_X	20	/* main player window left side */
#define PLAYER_MAIN_Y	80	/* main player window upper border */
#define PLAYER_MAIN_W	1100	/* width of main player window */
#define PLAYER_MAIN_H	(BITMAP_HEIGHT * (ROOM_HEIGHT + 2))	/* height */

/* width of the room picture window */
#define EXIT_WIN_W	(BITMAP_WIDTH * (ROOM_WIDTH + 2))
#define EXIT_NAME_OFF	((BITMAP_HEIGHT - char_height(bigfont)) / 2)

/* common variables for all windows on the right */
#define RIGHT_WIN_X	(EXIT_WIN_W)
#define RIGHT_WIN_W	(PLAYER_MAIN_W - RIGHT_WIN_X)


/* size of statistics window is fixed */
#define STAT_WIN_Y	(- WINDOW_BORDERS)
#define STAT_WIN_W	RIGHT_WIN_W
#define STAT_WIN_H	(BITMAP_HEIGHT + 15 + char_height(regfont) * 5)

/* item window height and inside particulars */
D 6
#define ITEM_WIN_H	(BITMAP_HEIGHT * INV_HEIGHT)
E 6
I 6
#define ITEM_WIN_H	(INV_WIN_H + SQUARE_WIN_H + WINDOW_BORDERS)
E 6
#define ITEM_WIN_Y	(STAT_WIN_H)
#define INV_WIN_W	(BITMAP_WIDTH * INV_WIDTH)
D 6
#define INV_WIN_H	ITEM_WIN_H
E 6
I 6
#define INV_WIN_H	(BITMAP_HEIGHT * INV_HEIGHT)
E 6
#define INV_WIN_X	(- WINDOW_BORDERS)
#define INV_WIN_Y	(- WINDOW_BORDERS)
#define MOUSE_WIN_X	(INV_WIN_W + (RIGHT_WIN_W -INV_WIN_W -mouse_width) / 2)
D 6
#define MOUSE_WIN_Y	((ITEM_WIN_H - mouse_height) / 2)
E 6
I 6
#define MOUSE_WIN_Y	((INV_WIN_H - mouse_height) / 2)
#define SQUARE_WIN_H	(BITMAP_HEIGHT)
#define SQUARE_WIN_Y	(INV_WIN_Y + INV_WIN_H + WINDOW_BORDERS)
#define SQUARE_WIN_X	(- WINDOW_BORDERS)
E 6

D 6

E 6
/* message window particulars */
#define GEN_GAP		4			/* space around text */
#define GEN_WIN_X	(- WINDOW_BORDERS)	/* left x of small windows */
I 3
D 4
#define PLAYER_MESSAGE_COLUMNS 70	/* width of message windows */
E 4
E 3
D 2
#define GEN_WIN_H	(char_height(tinyfont) + GEN_GAP * 2)
E 2

#define ERR_WIN_Y	(- WINDOW_BORDERS)
D 2
#define IN_WIN_Y	GEN_WIN_H
E 2
I 2
#define ERR_WIN_H	(char_height(tinyfont) * MAX_ERROR_MESSAGES + \
			 GEN_GAP * 2)
#define ERR_WIN_LINES	(ERR_WIN_H / char_height(tinyfont))
E 2

D 2
#define OUT_WIN_Y	(IN_WIN_Y + GEN_WIN_H + WINDOW_BORDERS)
E 2
I 2
#define IN_WIN_Y	(ERR_WIN_Y + ERR_WIN_H + WINDOW_BORDERS)
#define IN_WIN_H	(char_height(tinyfont) + GEN_GAP * 2)

#define OUT_WIN_Y	(IN_WIN_Y + IN_WIN_H + WINDOW_BORDERS)
E 2
#define OUT_WIN_H	(char_height(tinyfont) * MAX_MESSAGES + GEN_GAP * 2)
#define OUT_WIN_LINES	(OUT_WIN_H / char_height(tinyfont))

#define MSG_WIN_Y	(ITEM_WIN_H + STAT_WIN_H + WINDOW_BORDERS)
D 2
#define MSG_WIN_H	(GEN_WIN_H * 2 + OUT_WIN_H + WINDOW_BORDERS * 2)
E 2
I 2
#define MSG_WIN_H	(ERR_WIN_H + OUT_WIN_H + IN_WIN_H + WINDOW_BORDERS * 2)
E 2
D 5
#define MSG_WIN_COLS	(RIGHT_WIN_W / char_width(tinyfont))
E 5
I 5
#define MSG_WIN_COLS	(min((RIGHT_WIN_W / char_width(tinyfont)), \
			     PLAYER_MESSAGE_COLUMNS))
E 5



/* misc information window at bottom */
#define MISC_WIN_Y	(MSG_WIN_Y + MSG_WIN_H + WINDOW_BORDERS)
#define MISC_WIN_H	(PLAYER_MAIN_H - MISC_WIN_Y)


/* lines and columns that fit in help window */
#define HELP_WIN_COLS	(RIGHT_WIN_W / char_width(tinyfont))
#define HELP_WIN_LINES	(min(((PLAYER_MAIN_H - MSG_WIN_Y) / char_height(tinyfont)),\
			     MAX_HELP_LINES))

/* inventory window conversion macros, pixel xy to inventory number */
#define INVXY_TO_NUM(x, y)	\
	((((y) / BITMAP_HEIGHT) * INV_WIDTH) + ((x) / BITMAP_WIDTH))

/* number of additional missile rotation views besides the main one */
#define VIEWS	3

#define GAME_JOINED	7467		/* a flag */
#define PLAYERWIN_AUTORAISE	FALSE	/* TRUE if main window to autoraise */

#ifndef PLAYERX11_MAIN

extern Window	mainwin, exitwin, itemwin, statwin, msgwin, unwin;
D 6
extern Window	invwin, mousewin, errwin, inwin, outwin;
E 6
I 6
extern Window	invwin, mousewin, squarewin, errwin, inwin, outwin;
E 6
extern Help	*misc_rec;

#endif

E 1
