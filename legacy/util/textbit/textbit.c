/* takes a bitmap (short * format) and writes it to screen, with * for
 * WhitePixel and ' ' for BlackPixel */

#include <math.h>
#include <stdio.h>

void textbit(int width,int height,short *bitmap)
{
  int wordwidth;  /* width of bitmap in full short ints */
  int i,j;		      /* counters */
    
  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(short) * 8) ); 

  for (j = 0; j < height ;j ++){
    for (i = 0; i < width ; i ++) {
      if (getBit(bitmap,i,j,wordwidth,height))
        printf("* ");
      else
        printf("  ");
    }
    printf("|\n");
  }
}
