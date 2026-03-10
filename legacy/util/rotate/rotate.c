
/* flips a bitmap horizontally */
/* --> width and height of bitmap */
/* --> pointer to bitmap */
/* <-- a pointer to another bitmap that is the original flipped horizontally */

#include <math.h>

unsigned char *rotate(int width, int height, unsigned char *bitmap, char dir)
/* returns the bitmap (must be square) rotated 90 degrees */
/* dir is 1 for rotate clockwise, and 0 for rotate counterclockwise) */
{
  int wordwidth;  /* width of bitmap in full char ints */
  int i,j;		      /* counters */
  unsigned char *result;
    
  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) ); 
  
  result = (unsigned char *)malloc(wordwidth * height * sizeof(char));
  if (!result) {
    printf("malloc failed in vflip\n");
    exit(1);
  }
  for (i = 0 ; i < wordwidth*height ; i ++ )
    result[i] = 0;

  for (i = 0 ; i < width ; i ++) 
    for (j = 0 ; j < height ; j ++) 
      putBit(result, 
	     dir ? width-i-1 : i ,
	     dir ? j : height-j-1,
	     wordwidth,height,
	     getBit(bitmap,j,i,wordwidth,height));

  return result;
}

