/* flips a bitmap vertically */
/* --> width and height of bitmap */
/* --> pointer to bitmap */
/* <-- a pointer to another bitmap that is the original flipped vertically */

#include <math.h>

char *vflip(int width, int height, char *bitmap)

{
  int wordwidth, wordheight;  /* length and width of bitmap in full char words */
  int i,j;		      /* counters */
  char *result;

  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) ); 
  
  wordheight = height;

  result = (char *)malloc(wordwidth * wordheight * sizeof(char));
  if (!result) {
    printf("malloc failed in vflip\n");
    exit(1);
  }
  
  for ( i = wordwidth * (wordheight - 1)  ; i >= 0 ; i-= wordwidth)
    for (j = 0 ; j <= wordwidth - 1 ; j ++) {
      result[ (wordwidth * (wordheight - 1)) - i  + j] = bitmap[i+j];
    }
  return result;
}

  
