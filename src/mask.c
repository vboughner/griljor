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

char *make_mask(bitmap)
char *bitmap;
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
