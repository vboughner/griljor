#include <stdio.h>

char get_nth_bit(int num,char *source)
/* returns either zero or one depending on the value of the
 * nth bit from the source pointer */

{
  while ((num / (sizeof(char) * 8)) >= 1) {
    num -= (sizeof(char) * 8);
    source ++;
  }
  return ((*source >> num) & 1);

}


char getBit(char *source,
		      int wpos,int hpos,
		      int wsize,int hsize)
/* returns the specified bit from source */
{
  return get_nth_bit(wpos,source + hpos * wsize);
}



put_nth_bit(char bit, int num,char *target)
/* puts bit into the num'th bit of target */

{
  while ((num / (sizeof(char) * 8)) >= 1) {
    num -= (sizeof(char) * 8);
    target ++;
  }
 *target |= (bit << num);
}


putBit(char *source,
		      int wpos,int hpos,
		      int wsize,int hsize,
		      char bit)
/* returns the specified bit from source */
{
  put_nth_bit(bit,wpos,source + hpos * wsize);
}


/* takes a bitmap (char * format) and writes it to screen, with * for
 * WhitePixel and ' ' for BlackPixel */

void textbit(int width,int height,char *bitmap)
{
  int wordwidth;  /* width of bitmap in full char ints */
  int i,j;		      /* counters */
    
  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) ); 

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
