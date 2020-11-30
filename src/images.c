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

/* Routines for flipping and rotating bitmaps */


#include <stdio.h>
#include <math.h>
#include "lib.h"


/* procedure declarations */
void revline();



char *vflip(width, height, bitmap)
int width, height;
char *bitmap;
/* flips a bitmap vertically */
/* --> width and height of bitmap */
/* --> pointer to bitmap */
/* <-- a pointer to another bitmap that is the original flipped vertically */
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

  


char *hflip(width, height, bitmap)
int width, height;
char *bitmap;
/* flips a bitmap horizontally */
/* --> width and height of bitmap */
/* --> pointer to bitmap */
/* <-- a pointer to another bitmap that is the original flipped horizontally */
{
  int wordwidth;  /* width of bitmap in full char ints */
  int i,j;		      /* counters */
  char *result;

  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) ); 
  
  result = (char *)malloc(wordwidth * height * sizeof(char));
  if (!result) {
    printf("malloc failed in vflip\n");
    exit(1);
  }
  for (i = 0 ; i < wordwidth * height ; i += wordwidth) {
    revline(&result[i],&bitmap[i],wordwidth,width);


  }
  return (char *) result;
}




void revline(target, source, charsize, bitsize)
char *target, *source;
int charsize, bitsize;
/* reverses bitsize bits in the next charsize bytes and puts them 
 * into target */
{
  int i;

  for (i = 0 ; i < charsize ; i++)
    target[i] = 0;

  for (i = 0 ; i < bitsize ; i ++)
    put_nth_bit(get_nth_bit(i,source),
		bitsize-i-1,
		target);
}




char *rotate(width, height, bitmap, dir)
int width, height;
char *bitmap, dir;
/* returns the bitmap (must be square) rotated 90 degrees */
/* dir is 1 for rotate clockwise, and 0 for rotate counterclockwise) */
{
  int wordwidth;  /* width of bitmap in full char ints */
  int i,j;		      /* counters */
  char *result;
    
  wordwidth = (int)ceil( ((double)width) / (double)(sizeof(char) * 8) ); 
  
  result = (char *)malloc(wordwidth * height * sizeof(char));
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

