h11900
s 00000/00000/00030
d D 1.2 92/08/07 01:04:04 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00030/00000/00000
d D 1.1 91/04/14 19:33:24 labc-3id 1 0
c date and time created 91/04/14 19:33:24 by labc-3id
e
u
U
f e 0
t
T
I 1
#include "xx.h"

#define		SEP_Y		14
#define		Y_DIVISIONS	12
#define		FONT_ADJUST	3

#define		SCROLL_X	(SLID_WINDOW_X + SLID_WINDOW_WIDTH + 2 * SLID_WINDOW_BORDER) 
#define		SCROLL_Y	(SLID_WINDOW_Y + SEP_Y)
#define		SCROLL_WIDTH    (BITM_WINDOW_WIDTH - SLID_WINDOW_WIDTH)
#define		SCROLL_HEIGHT	SLID_WINDOW_HEIGHT

#define		X_NUMBER	(SCROLL_X + 3)
#define		X_NAME		(X_NUMBER + 50)
#define		X_EXP_PTS	(X_NAME + 190)
#define		X_LOGIN		(X_EXP_PTS + 60)
#define		X_HOST		(X_LOGIN + 80)
#define		X_DEITY		(X_HOST + 58)
#define		X_BITMAP	(X_DEITY + 6)

#define		DEITY_SCALER	8
#define		Y_DEITY_SEP	3
#define		SIZE_DEITY	3
#define		DEITY_HEIGHT	SEP_Y - 4

/* The following are the maximum sizes of stuff displayed in the scroll field */

#define		NAME_MAX_SIZE	26
#define		EXP_PT_MAX_SIZE 8
#define		LOGIN_NAME_MAX_SIZE 9
#define		HOST_MAX_SIZE	7
E 1
