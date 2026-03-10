#include "obtor.h"
#include <math.h>

#define BITMAP_WIDTH 32
#define BITMAP_HEIGHT 32
#define BITMAP_ARRAY_SIZE 128

char *make_mask(char *bitmap,char *result)
/* currently hardwired for the bitmap size of griljor bitmaps */
{
  int width = BITMAP_WIDTH, height = BITMAP_HEIGHT;
  int i,j,k,l;
  int wordwidth;  /* width of bitmap in full char ints */
    
  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) );

  bzero(result, BITMAP_ARRAY_SIZE * sizeof(char));

  for (i = 0 ; i < width ; i ++) 
    for (j = 0 ; j < height ; j ++) 
      if (getBit(bitmap,i,j,wordwidth,height)) 
        for (k = -1 * (i != 0) ; k <= (i != width-1) ; k ++) 
          for (l = -1 * (j != 0) ; l <= (j != height-1) ; l ++) 
            putBit(result,i+k,j+l,wordwidth,height,1);
}
