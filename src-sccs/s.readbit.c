h34699
s 00000/00000/00231
d D 1.2 92/08/07 01:03:49 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00231/00000/00000
d D 1.1 91/05/17 04:16:05 labc-3id 1 0
c date and time created 91/05/17 04:16:05 by labc-3id
e
u
U
f e 0
t
T
I 1
/* title.c for Griljor Copyright 1990 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "free.h"

#include "bitmaps/g"
#include "bitmaps/r1"
#include "bitmaps/i"
#include "bitmaps/l"
#include "bitmaps/j"
#include "bitmaps/o1"
#include "bitmaps/o2"
#include "bitmaps/o3"
#include "bitmaps/o4"
#include "bitmaps/o5"
#include "bitmaps/o6"
#include "bitmaps/o7"
#include "bitmaps/o8"
#include "bitmaps/r2"
#include "bitmaps/beach0"
#include "bitmaps/beach1"
#include "bitmaps/beach2"
#include "bitmaps/forest"
#include "bitmaps/sand"
#include "bitmaps/llpath"
#include "bitmaps/lrpath"
#include "bitmaps/ulpath"
#include "bitmaps/urpath"
#include "bitmaps/vpath"
#include "bitmaps/hpath"

#include "bitmaps/river0"
#include "bitmaps/river1"
#include "bitmaps/river2"
#include "bitmaps/river3"
#include "bitmaps/river4"
#include "bitmaps/river5"
#include "bitmaps/river6"
#include "bitmaps/river7"
#include "bitmaps/river8"
#include "bitmaps/river9"
#include "bitmaps/river10"
#include "bitmaps/river11"
#include "bitmaps/river12"
#include "bitmaps/river13"
#include "bitmaps/river14"
#include "bitmaps/river15"
#include "bitmaps/river16"
#include "bitmaps/river17"
#include "bitmaps/river18"
#include "bitmaps/river20"
#include "bitmaps/mask0"
#include "bitmaps/mask1"
#include "bitmaps/mask2"
#include "bitmaps/mask3"
#include "bitmaps/mask4"
#include "bitmaps/mask5"
#include "bitmaps/mask6"
#include "bitmaps/mask7"
#include "bitmaps/mask8"
#include "bitmaps/mask9"
#include "bitmaps/mask10"
#include "bitmaps/mask11"
#include "bitmaps/mask12"
#include "bitmaps/mask13"
#include "bitmaps/mask14"
#include "bitmaps/mask15"
#include "bitmaps/mask20"

readSceneBitmaps(display, mainWindow, fg, bg, depth, river, mask,
		 beach, path, forest, sand)
     Display *display;
     Window mainWindow;
     unsigned long fg, bg;
     int depth;
     Pixmap *river;
     Pixmap *mask;
     Pixmap *beach;
     Pixmap *path;
     Pixmap *forest;
     Pixmap *sand;
{
  river[0] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river0_bits, river0_width, river0_height, fg, bg, depth);
  river[1] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river1_bits, river1_width, river1_height, fg, bg, depth);
  river[2] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river2_bits, river2_width, river2_height, fg, bg, depth);
  river[3] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river3_bits, river3_width, river3_height, fg, bg, depth);
  river[4] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river4_bits, river4_width, river4_height, fg, bg, depth);
  river[5] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river5_bits, river5_width, river5_height, fg, bg, depth);
  river[6] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river6_bits, river6_width, river6_height, fg, bg, depth);
  river[7] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river7_bits, river7_width, river7_height, fg, bg, depth);
  river[8] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river8_bits, river8_width, river8_height, fg, bg, depth);
  river[9] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river9_bits, river9_width, river9_height, fg, bg, depth);
  river[10] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river10_bits, river10_width, river10_height, fg, bg, depth);
  river[11] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river11_bits, river11_width, river11_height, fg, bg, depth);
  river[12] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river12_bits, river12_width, river12_height, fg, bg, depth);
  river[13] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river13_bits, river13_width, river13_height, fg, bg, depth);
  river[14] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river14_bits, river14_width, river14_height, fg, bg, depth);
  river[15] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river15_bits, river15_width, river15_height, fg, bg, depth);
  river[16] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river16_bits, river16_width, river16_height, fg, bg, depth);
  river[17] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river17_bits, river17_width, river17_height, fg, bg, depth);
  river[18] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river18_bits, river18_width, river18_height, fg, bg, depth);
  river[20] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		river20_bits, river20_width, river20_height, fg, bg, depth);
  mask[0] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask0_bits, mask0_width, mask0_height, fg, bg, depth);
  mask[1] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask1_bits, mask1_width, mask1_height, fg, bg, depth);
  mask[2] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask2_bits, mask2_width, mask2_height, fg, bg, depth);
  mask[3] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask3_bits, mask3_width, mask3_height, fg, bg, depth);
  mask[4] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask4_bits, mask4_width, mask4_height, fg, bg, depth);
  mask[5] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask5_bits, mask5_width, mask5_height, fg, bg, depth);
  mask[6] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask6_bits, mask6_width, mask6_height, fg, bg, depth);
  mask[7] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask7_bits, mask7_width, mask7_height, fg, bg, depth);
  mask[8] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask8_bits, mask8_width, mask8_height, fg, bg, depth);
  mask[9] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask9_bits, mask9_width, mask9_height, fg, bg, depth);
  mask[10] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask10_bits, mask10_width, mask10_height, fg, bg, depth);
  mask[11] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask11_bits, mask11_width, mask11_height, fg, bg, depth);
  mask[12] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask12_bits, mask12_width, mask12_height, fg, bg, depth);
  mask[13] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask13_bits, mask13_width, mask13_height, fg, bg, depth);
  mask[14] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask14_bits, mask14_width, mask14_height, fg, bg, depth);
  mask[15] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask15_bits, mask15_width, mask15_height, fg, bg, depth);
  mask[20] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		mask20_bits, mask20_width, mask20_height, fg, bg, depth);

  beach[0] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 beach0_bits, beach0_width, beach0_height, fg, bg, depth);
  beach[1] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 beach1_bits, beach1_width, beach1_height, fg, bg, depth);
  beach[2] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 beach2_bits, beach2_width, beach2_height, fg, bg, depth);
  path[0] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 hpath_bits, hpath_width, hpath_height, fg, bg, depth);
  path[1] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 vpath_bits, vpath_width, vpath_height, fg, bg, depth);
  path[2] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 lrpath_bits, lrpath_width, lrpath_height, fg, bg, depth);
  path[3] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 llpath_bits, llpath_width, llpath_height, fg, bg, depth);
  path[4] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 urpath_bits, urpath_width, urpath_height, fg, bg, depth);
  path[5] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 ulpath_bits, ulpath_width, ulpath_height, fg, bg, depth);
  *forest = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 forest_bits, forest_width, forest_height, fg, bg, depth);
  *sand = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 sand_bits, sand_width, sand_height, fg, bg, depth);
}



readLetterBitmaps(display, mainWindow, fg, bg, depth, letter, letterO,
		  finalPos)
     Display *display;
     Window mainWindow;
     unsigned long fg, bg;
     int depth;
     Pixmap *letter;
     Pixmap *letterO;
     int *finalPos;
{

  letter[0] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         g_bits, g_width, g_height, fg, bg, depth);
  letter[1] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         r1_bits, r1_width, r1_height, fg, bg, depth);
  letter[2] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         i_bits, i_width, i_height, fg, bg, depth);
  letter[3] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         l_bits, l_width, l_height, fg, bg, depth);
  letter[4] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         j_bits, j_width, j_height, fg, bg, depth);
  letter[5] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
		 o1_bits, o1_width, o1_height, fg, bg, depth);
  letter[6] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         r2_bits, r2_width, r2_height, fg, bg, depth);

  letterO[0] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o1_bits, o1_width, o1_height, fg, bg, depth);
  letterO[1] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o2_bits, o2_width, o2_height, fg, bg, depth);
  letterO[2] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o3_bits, o3_width, o3_height, fg, bg, depth);
  letterO[3] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o4_bits, o4_width, o4_height, fg, bg, depth);
  letterO[4] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o5_bits, o5_width, o5_height, fg, bg, depth);
  letterO[5] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o6_bits, o6_width, o6_height, fg, bg, depth);
  letterO[6] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o7_bits, o7_width, o7_height, fg, bg, depth);
  letterO[7] = MyXCreatePixmapFromBitmapData(display, mainWindow, 
	         o8_bits, o8_width, o8_height, fg, bg, depth);

  *finalPos = g_width + r1_width + i_width + l_width + j_width + 
    o1_width + r2_width;
}
E 1
