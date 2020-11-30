h33840
s 00000/00000/00137
d D 1.3 92/08/07 01:01:42 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00012/00005/00125
d D 1.2 91/08/29 01:40:04 vanb 2 1
c fixed up more compatibility problems
e
s 00130/00000/00000
d D 1.1 91/02/16 12:54:22 labc-3id 1 0
c date and time created 91/02/16 12:54:22 by labc-3id
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

/* Routines for flipping and rotating bitmaps */


#include <stdio.h>
#include <math.h>
#include "lib.h"


/* procedure declarations */
void revline();



D 2
char *vflip(int width, int height, char *bitmap)
E 2
I 2
char *vflip(width, height, bitmap)
int width, height;
char *bitmap;
E 2
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

  


D 2
char *hflip(int width, int height, char *bitmap)
E 2
I 2
char *hflip(width, height, bitmap)
int width, height;
char *bitmap;
E 2
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




D 2
void revline(char *target, char *source, 
	     int charsize, int bitsize)
E 2
I 2
void revline(target, source, charsize, bitsize)
char *target, *source;
int charsize, bitsize;
E 2
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




D 2
char *rotate(int width, int height, char *bitmap, char dir)
E 2
I 2
char *rotate(width, height, bitmap, dir)
int width, height;
char *bitmap, dir;
E 2
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

E 1
