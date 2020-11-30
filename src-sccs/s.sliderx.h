h59798
s 00000/00000/00013
d D 1.3 92/08/07 01:04:09 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00000/00003/00013
d D 1.2 92/01/20 16:26:34 labc-4lc 2 1
c removed references to bitmaps sliderx.c used, moved them to config.h
e
s 00016/00000/00000
d D 1.1 91/04/14 19:33:28 labc-3id 1 0
c date and time created 91/04/14 19:33:28 by labc-3id
e
u
U
f e 0
t
T
I 1
#include "xx.h"

D 2
#define		SLIDER_FILE_NAME	"bit/slider.bit"
#define		ICON_FILE_NAME		"bit/icon.bit"

E 2
#define		HALF_WAY_Y		(SLID_WINDOW_HEIGHT / 2)
#define		UPPER_QUARTER_Y		(HALF_WAY_Y / 2)
#define		LOWER_QUARTER_Y		(HALF_WAY_Y + UPPER_QUARTER_Y)

#define		HALF_WAY_X_1		0
#define		HALF_WAY_X_2		SLID_WINDOW_WIDTH

#define		FULL_LENGHT		SLID_WINDOW_HEIGHT

#define		SLIDER_BUTTON_WIDTH	(SLID_WINDOW_WIDTH - 2)
#define		SLIDER_BUTTON_HEIGHT	41
E 1
