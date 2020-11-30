h20146
s 00000/00000/00073
d D 1.3 92/08/07 01:00:22 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00011/00011/00062
d D 1.2 91/10/16 20:05:18 labc-4lc 2 1
c changed made for DecStations
e
s 00073/00000/00000
d D 1.1 91/03/24 18:44:25 labc-3id 1 0
c date and time created 91/03/24 18:44:25 by labc-3id
e
u
U
f e 0
t
T
I 1


#include "obtor.h"

#include "bit/backg.b"
#include "bit/border.b"
#include "bit/up.b"
#include "bit/down.b"
#include "bit/icon.b"
#include "bit/nothing.b"
#include "bit/intro1.b"
#include "bit/intro2.b"
#include "bit/intro3.b"
#include "bit/foo.b"
#include "bit/info.b"

load_window_pixmaps()
{
  backg_pix = 
    XCreatePixmapFromBitmapData(display, rootwin,
				backg_bits, backg_width, backg_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2

  border_pix = 
    XCreatePixmapFromBitmapData(display, rootwin,
				border_bits, border_width, border_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
    
  up_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				up_bits, up_width, up_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2

  down_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				down_bits, down_width, down_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  icon_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				icon_bits, icon_width, icon_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  foo_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				foo_bits, foo_width, foo_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  nothing_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				nothing_bits, nothing_width, nothing_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  info_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				info_bits, info_width, info_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  intro1_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				intro1_bits, intro1_width, intro1_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  intro2_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				intro2_bits, intro2_width, intro2_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2
  intro3_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				intro3_bits, intro3_width, intro3_height,
D 2
				fgcolor, bgcolor, 1);
E 2
I 2
				fgcolor, bgcolor, depth);
E 2

  intro1w = intro1_width;
  intro1h = intro1_height;
  intro2w = MAX(intro2_width,intro3_width);
  intro2h = MAX(intro2_height,intro3_height);
}


E 1
