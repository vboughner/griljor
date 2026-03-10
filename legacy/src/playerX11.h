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
#define ITEM_WIN_H	(INV_WIN_H + SQUARE_WIN_H + WINDOW_BORDERS)
#define ITEM_WIN_Y	(STAT_WIN_H)
#define INV_WIN_W	(BITMAP_WIDTH * INV_WIDTH)
#define INV_WIN_H	(BITMAP_HEIGHT * INV_HEIGHT)
#define INV_WIN_X	(- WINDOW_BORDERS)
#define INV_WIN_Y	(- WINDOW_BORDERS)
#define MOUSE_WIN_X	(INV_WIN_W + (RIGHT_WIN_W -INV_WIN_W -mouse_width) / 2)
#define MOUSE_WIN_Y	((INV_WIN_H - mouse_height) / 2)
#define SQUARE_WIN_H	(BITMAP_HEIGHT)
#define SQUARE_WIN_Y	(INV_WIN_Y + INV_WIN_H + WINDOW_BORDERS)
#define SQUARE_WIN_X	(- WINDOW_BORDERS)

/* message window particulars */
#define GEN_GAP		4			/* space around text */
#define GEN_WIN_X	(- WINDOW_BORDERS)	/* left x of small windows */

#define ERR_WIN_Y	(- WINDOW_BORDERS)
#define ERR_WIN_H	(char_height(tinyfont) * MAX_ERROR_MESSAGES + \
			 GEN_GAP * 2)
#define ERR_WIN_LINES	(ERR_WIN_H / char_height(tinyfont))

#define IN_WIN_Y	(ERR_WIN_Y + ERR_WIN_H + WINDOW_BORDERS)
#define IN_WIN_H	(char_height(tinyfont) + GEN_GAP * 2)

#define OUT_WIN_Y	(IN_WIN_Y + IN_WIN_H + WINDOW_BORDERS)
#define OUT_WIN_H	(char_height(tinyfont) * MAX_MESSAGES + GEN_GAP * 2)
#define OUT_WIN_LINES	(OUT_WIN_H / char_height(tinyfont))

#define MSG_WIN_Y	(ITEM_WIN_H + STAT_WIN_H + WINDOW_BORDERS)
#define MSG_WIN_H	(ERR_WIN_H + OUT_WIN_H + IN_WIN_H + WINDOW_BORDERS * 2)
#define MSG_WIN_COLS	(min((RIGHT_WIN_W / char_width(tinyfont)), \
			     PLAYER_MESSAGE_COLUMNS))



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
extern Window	invwin, mousewin, squarewin, errwin, inwin, outwin;
extern Help	*misc_rec;

#endif

