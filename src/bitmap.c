

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
				fgcolor, bgcolor, depth);

  border_pix = 
    XCreatePixmapFromBitmapData(display, rootwin,
				border_bits, border_width, border_height,
				fgcolor, bgcolor, depth);
    
  up_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				up_bits, up_width, up_height,
				fgcolor, bgcolor, depth);

  down_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				down_bits, down_width, down_height,
				fgcolor, bgcolor, depth);
  icon_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				icon_bits, icon_width, icon_height,
				fgcolor, bgcolor, depth);
  foo_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				foo_bits, foo_width, foo_height,
				fgcolor, bgcolor, depth);
  nothing_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				nothing_bits, nothing_width, nothing_height,
				fgcolor, bgcolor, depth);
  info_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				info_bits, info_width, info_height,
				fgcolor, bgcolor, depth);
  intro1_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				intro1_bits, intro1_width, intro1_height,
				fgcolor, bgcolor, depth);
  intro2_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				intro2_bits, intro2_width, intro2_height,
				fgcolor, bgcolor, depth);
  intro3_pix =
    XCreatePixmapFromBitmapData(display, rootwin,
				intro3_bits, intro3_width, intro3_height,
				fgcolor, bgcolor, depth);

  intro1w = intro1_width;
  intro1h = intro1_height;
  intro2w = MAX(intro2_width,intro3_width);
  intro2h = MAX(intro2_height,intro3_height);
}


