h36651
s 00000/00000/00042
d D 1.3 92/08/07 01:00:27 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00004/00003/00038
d D 1.2 91/08/29 01:39:40 vanb 2 1
c fixed up more compatibility problems
e
s 00041/00000/00000
d D 1.1 91/02/16 12:53:43 labc-3id 1 0
c date and time created 91/02/16 12:53:43 by labc-3id
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

/* Check to see that the bitmaps players want to use are acceptable */

#include <math.h>
#include "config.h"
#include "def.h"


/* returns TRUE (1) if there are the given minimum bits in the bitmap */

D 2
int bitok(char *bitmap, int minpix)

E 2
I 2
int bitok(bitmap, minpix)
char *bitmap;
int minpix;
E 2
{
  int wordwidth;  /* width of bitmap in char sized ints */
  int i,j;		      /* counters */
  int count = 0;

D 2
  /* TEMPORARY */
E 2
I 2
  /* *********** TEMPORARY ************* */
E 2
  return 1;


  wordwidth = (int)ceil(((double)BITMAP_WIDTH) /
			((double)(sizeof(char) * 8))); 
  
  for (i = 0 ; i < BITMAP_WIDTH ; i ++) 
    for (j = 0 ; j < BITMAP_HEIGHT ; j ++) 
      if (getBit(bitmap,j,i,wordwidth,BITMAP_HEIGHT))
        count ++;

  return ((count < minpix) ? 0 : 1);
}
E 1
