h59245
s 00000/00000/00048
d D 1.3 92/08/07 01:02:15 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00002/00001/00046
d D 1.2 91/08/29 01:40:22 vanb 2 1
c fixed up more compatibility problems
e
s 00047/00000/00000
d D 1.1 91/02/16 12:54:47 labc-3id 1 0
c date and time created 91/02/16 12:54:47 by labc-3id
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
 * October 1989
 **************************************************************************/

/* Bitmap mask creating facilities */

#include <stdio.h>
#include <math.h>
#include "config.h"
#include "def.h"
#include "lib.h"



/* make a mask for a given bitmap and return a pointer to it */

D 2
char *make_mask(char *bitmap)
E 2
I 2
char *make_mask(bitmap)
char *bitmap;
E 2
/* currently hardwired for the bitmap size of griljor bitmaps */

{
  char *result;
  int width = BITMAP_WIDTH, height = BITMAP_HEIGHT;
  int i,j,k,l;
  int wordwidth;  /* width of bitmap in full char ints */
    
  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) );
  result = (char *)malloc(BITMAP_ARRAY_SIZE * sizeof(char));
  demand(result,"Error: malloc failed in make_mask");

  bzero(result, BITMAP_ARRAY_SIZE * sizeof(char));

  for (i = 0 ; i < width ; i ++) 
    for (j = 0 ; j < height ; j ++) 
      if (getBit(bitmap,i,j,wordwidth,height)) 
        for (k = -1 * (i != 0) ; k <= (i != width-1) ; k ++) 
          for (l = -1 * (j != 0) ; l <= (j != height-1) ; l ++) 
            putBit(result,i+k,j+l,wordwidth,height,1);

  return result;
}
E 1
