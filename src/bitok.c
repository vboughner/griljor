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

/* Check to see that the bitmaps players want to use are acceptable */

#include <math.h>
#include "config.h"
#include "def.h"


/* returns TRUE (1) if there are the given minimum bits in the bitmap */

int bitok(bitmap, minpix)
char *bitmap;
int minpix;
{
  int wordwidth;  /* width of bitmap in char sized ints */
  int i,j;		      /* counters */
  int count = 0;

  /* *********** TEMPORARY ************* */
  return 1;


  wordwidth = (int)ceil(((double)BITMAP_WIDTH) /
			((double)(sizeof(char) * 8))); 
  
  for (i = 0 ; i < BITMAP_WIDTH ; i ++) 
    for (j = 0 ; j < BITMAP_HEIGHT ; j ++) 
      if (getBit(bitmap,j,i,wordwidth,BITMAP_HEIGHT))
        count ++;

  return ((count < minpix) ? 0 : 1);
}
