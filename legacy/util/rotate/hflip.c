/* flips a bitmap horizontally */
/* --> width and height of bitmap */
/* --> pointer to bitmap */
/* <-- a pointer to another bitmap that is the original flipped horizontally */

#include <math.h>

void revline();

unsigned char *hflip(int width, int height, unsigned char *bitmap)

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
  for (i = 0 ; i < wordwidth * height ; i += wordwidth) {
    revline(&result[i],&bitmap[i],wordwidth,width);


  }
  return result;
}


void revline(unsigned char *target, unsigned char *source, 
	     int shortsize, int bitsize)
/* reverses bitsize bits in the next shortsize bytes and puts them 
 * into target */
{
  int i;

  for (i = 0 ; i < shortsize ; i++)
    target[i] = 0;

  for (i = 0 ; i < bitsize ; i ++)
    put_nth_bit(get_nth_bit(i,source),
		bitsize-i-1,
		target);
}

