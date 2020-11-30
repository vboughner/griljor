h56795
s 00000/00000/00593
d D 1.7 92/08/07 01:02:03 vbo 7 6
c source copied to a separate tree for work on new map and object format
e
s 00055/00000/00538
d D 1.6 91/11/29 16:09:18 labc-4lc 6 5
c finished making entry of hidden text properties in obtor
e
s 00001/00001/00537
d D 1.5 91/10/16 20:05:39 labc-4lc 5 4
c changed made for DecStations
e
s 00017/00013/00521
d D 1.4 91/08/26 00:33:31 vanb 4 3
c fixed up procedure defs and other compatibilty problems
e
s 00003/00002/00531
d D 1.3 91/03/30 22:24:17 labc-3id 3 2
c fixed read_line to return empty string at end of file
e
s 00012/00008/00521
d D 1.2 91/03/04 23:21:34 labc-3id 2 1
c 
e
s 00529/00000/00000
d D 1.1 91/02/16 12:54:37 labc-3id 1 0
c date and time created 91/02/16 12:54:37 by labc-3id
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

/* Library of very generic routines */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
D 2
/* #include <unistd.h> */ /* would be neccessary on Sun for file locking */
E 2
#include <errno.h>
#include <math.h>
I 2

/* unistd.h include file would be neccessary on Sun 3/50's for file locking */
/* see code in oldlock.c if you want to re-include that feature */
/* #include <unistd.h> */

E 2
#include "config.h"
#include "def.h"
#include "lib.h"



/* get a filename given a prompt and a string to place it in */

get_filename(prompt, name)
char *prompt, *name;
{
D 2
  printf("%s", prompt);
E 2
I 2
  if (prompt) printf("%s", prompt);
E 2
  strcpy(name, read_line(stdin));
}



I 3
/* returns zero length string at end of file */
E 3
char *read_line(fp)
FILE *fp;
{
D 3
  char c;
E 3
I 3
  int c;
E 3
  static char line[300];
  int  i;

  c = getc(fp);
D 2
  for (i=0; ((c!='\n') && (i<120)); i++) {
E 2
I 2
D 3
  for (i=0; ((c!='\n') && (i<299)); i++) {
E 3
I 3
  for (i=0; ((c != '\n') && (c != EOF) && (i < 299)); i++) {
E 3
E 2
    line[i] = c;
    c = getc(fp);
  }

  line[i] = '\0';
  return line;
}



/* get a line of text from user after giving a prompt */

char *prompt_for_input(prompt)
char *prompt;
{
D 2
  printf("%s", prompt);
E 2
I 2
  if (prompt) printf("%s", prompt);
E 2
  return read_line(stdin);
}



/* get a line of text from user, limiting his response to a certain number
   of characters, ie.  no matter how much he types, only the maximum number
   of characters given (less one) followed by a nice NULL character
   will be returned */

char *limited_prompt(prompt, limit)
char *prompt;
int limit;
{
  static char result[300];

  strcpy(result, prompt_for_input(prompt));
  if (strlen(result) >= limit) result[limit-1] = '\0';

  return result;
}



/* get the first word in the line and also return the rest,
   it would be really bad for the first char in line to be a space */
   
char *first_and_rest(line, rest)
char *line, *rest;
{
D 2
  static char result[90];
E 2
I 2
  static char result[300];
E 2
  int i;

  /* find the first space character */
  for (i=0; i<strlen(line) && !isspace(line[i]); i++);

  strncpy(result, line, i);
  result[i] = '\0';

  /* find the beginning of the second word */
  if (i==strlen(line))
    rest[0] = '\0';
  else {
    for (; i<strlen(line) && !isspace(line[i]); i++);

    strcpy(rest, line+i+1);
  }

  return result;
}

    
  
/* gets the next word out of a line, returns it and alters the given
   string so that it no longer contains that first word */

char *get_next_word(line)
char *line;
{
D 2
  static char result[50], rest[120];
E 2
I 2
  static char result[300], rest[300];
E 2

  strcpy(result, first_and_rest(line, rest));

  strcpy(line, rest);

  return result;
}



/* returns 1 if file exists and is readable, returns 0 if not */

int file_exists(filename)
char *filename;
{
  FILE *fp;

  fp = fopen(filename, "r");

  if (fp == NULL)
    return 0;
  else {
    fclose(fp);
    return 1;
  }
}



/* returns a FILE pointer to a newly opened file.  A lock will be made on
   this file as you open it so that no one else alters it.
   Currently, these locking aspects are not implmented, due to the lack
   of knowledge of a fully portable way to do this. */

FILE *fopen_locking(filename, mode, wait_sec)
char *filename, *mode;
int wait_sec;
{
  FILE *fp;

  fp = fopen(filename, mode);
  return fp;
}



/* go back to the beginning of a locked file, keeping it locked */

rewind_keeplock(fp)
FILE *fp;
{
  rewind(fp);
}



/* closes a file previously opened with fopen_locking */

fclose_unlocking(fp)
FILE *fp;
{
  fclose(fp);
}




/* take a string and return a changed one (allocated from memory).
   We are to be given the amount of characters in new string, and we should
   left justify the old string into new space, or if there is too much,
   cut off from the right side */

char *left_justify(s, size)
char *s;
int  size;
{
  char *new;
  int  i;

  new = allocate_string(size+1);

  /* put in blanks */
  for (i=0; i<size; i++) new[i] = ' ';
  new[size] = '\0';

  /* put in old string */
  for (i=0; i<size && i<strlen(s); i++)  new[i] = s[i];

  /* return the result */
  return new;
}



/* left justify a number like above worked for a string */

char *left_justify_number(n, size)
int n, size;
{
D 2
  char s[30];
E 2
I 2
  char s[300];
E 2

  sprintf(s, "%d", n);
  return left_justify(s, size);
}



/* allocate a string from memory */

char *allocate_string(size)
int size;
{
   char *result;
   
   result = (char *) malloc(size);
   if (result == NULL) Gerror("not enough room for a string");

   return result;
}



/* allocate a string and copy contents into it */

char *create_string(s)
char *s;
{
  char *result;

  result = allocate_string(strlen(s) + 1);
  strcpy(result, s);
  return result;
}

  

/* given a library directory name and a filename, concatenate the two
   of them and return a statically allocated string containing the
   result */

char *libbed_filename(libdir, filename)
char *libdir, *filename;
{
D 2
  static char result[PATH_LENGTH];
E 2
I 2
  static char result[500];
E 2
  char *tail;
  int i, found = 0;

  /* find the tail end of the given filename (after last /) */
  tail = filename;
  for (i = strlen(filename)-2; (i>=0 && !found); i--)
    if (filename[i] == '/') {
      tail = filename + i + 1;
      found = 1;
    }

  /* combine the library dir name and the filename */
  i = strlen(libdir)-1;
  if (libdir[i] == '/')
    sprintf(result, "%s%s", libdir, tail);
  else
    sprintf(result, "%s/%s", libdir, tail);

  return result;
}



/* return a string representation of the current time */

char *string_time()
{
  static char	result[40];
  long		long_time;
  struct tm	*current_time;

  /* get the current time if it is avaliable */
  long_time = time(NULL);
  if (long_time == -1) {
    fprintf(stderr, "Warning: system does not keep time\n");
    return "unknown";
  }

  /* convert it to a nice string */
D 5
  current_time = localtime(&long_time);
E 5
I 5
  current_time = (struct tm *) localtime((time_t *)&long_time);
E 5

  sprintf(result, "%s:%s %s/%s/%s", twostud(current_time->tm_hour),
	  twostud(current_time->tm_min), twostud(current_time->tm_mon + 1),
	  twostud(current_time->tm_mday), twostud(current_time->tm_year));

  return result;
}



/* take a number and return a string containing a zero on the left if the
   number is less than 10.  If number is more than 100, use only two right
   most digits */

char *twostud(num)
int num;
{
  char *result;
  
  /* make number small enough */
  num = num % 100;

  /* allocate result string */
  result = allocate_string(3);

  /* place number in string and check to place zero on left */
  sprintf(result, "%2d", num);
  if (num < 10) result[0] = '0';

  return result;
}




/* Checks for a match between the two strings.  Case is ignored.  0 is
   returned if there is no match, or the number of initial characters that
   match is returned.  ie.  strmatch("Aren't", "Are") returns 3.
   strmatch("Bob", "Bill") returns 1. */

int strmatch(s1, s2)
char *s1, *s2;
{
  int matched, okay = 1;
  char c1, c2;

  for (matched=0; (*s1 && *s2 && okay); s1++,s2++) {
    c1 = (isupper(*s1)) ? (tolower(*s1)) : *s1;
    c2 = (isupper(*s2)) ? (tolower(*s2)) : *s2;
    okay = (c1 == c2);
    if (okay) matched++;
  }

  return matched;
}



/* return a string containing spaces */

char *spaces(size)
int size;
{
  int i;
  char *new;

  new = allocate_string(size + 1);
  for (i=0; i<size; i++) new[i] = ' ';
  new[size] = '\0';

  return new;
}



I 6
int write_string_to_file(filename, s)
/* opens the file for writing and places the string in it, returns
   TRUE if all worked out. */
char *filename, *s;
{
  FILE *fp;

  fp = fopen(filename, "w");
  if (!fp) return FALSE;

  fputs(s, fp);
  fclose(fp);
  return TRUE;
}


char *read_string_from_file(filename)
/* opens the file for reading and reads it all out as one large string, which
   the caller will need to free after use.  Returns NULL if the file
   couldn't be opened, or is empty. */
char *filename;
{
  FILE *fp;
  char *result = NULL;
  int c, amount = 0, size = 0, done = 0;

  fp = fopen(filename, "r");
  if (!fp) return NULL;

  do {
    c = getc(fp);
    if (c != EOF) {
      if (amount >= (size - 1)) {
	if (size) {
          size += 100;
	  result = (char *) realloc(result, sizeof(char) * size);
	}
	else {
	  size = 200;
	  result = (char *) malloc(sizeof(char) * size);
	}
	demand(result, "out of memory in string read");
      }
      result[amount] = c;
      amount++;
    }
    else done = 1;
  } while (!done);

  if (size) result[amount] = '\0';
  return result;
}



E 6
/* variable initializer for use with lrand48 */

initrand()
{ int j;
  long k;

  /* initialize random variable */

  k = time(NULL);
  j = (unsigned int) k/2;
  srand48(j);
}



/* ===================== B I T M A P  manipulation ===================== */
/*                           ( X11 dependent)			         */

/* copy a bitmap from one array to another */

bitmapcpy(dst, src)
char *dst, *src;
{
  register int i;

  for (i=0; i<BITMAP_ARRAY_SIZE; i++) dst[i] = src[i];
}



/* allocate an array for a bitmap and copy another bitmap into it */

char *dup_bitmap_array(src)
char *src;
{
  char *new;

  new = (char *) malloc(BITMAP_ARRAY_SIZE * sizeof(char));
  if (new == NULL) Gerror("not enough memory for duplicate bitmap");

  /* copy source bitmap into newly allocated array */
  bitmapcpy(new, src);

  return new;
}



/* load in a bitmap and place into a record array */

load_bitmap(filename, bitarray)
char *filename;
char *bitarray;
{
  int width, height, x_hot, y_hot, i, j;
  char *temp;
  char emsg[160];

  j = XXReadBitmapFile(filename, &width, &height, &temp, &x_hot, &y_hot);

  if (j != 1)
    switch (j) {
      case 0: sprintf(emsg, "bitmap file %s not found", filename);
	      Gerror(emsg);
      case -1:Gerror("bad width specification in bitmap file");
      case -2:Gerror("bad height specification in bitmap file");
      case -3:Gerror("out of memory while allocating bit arrays");
      case -4:Gerror("bad bit array in bitmap file");
      case -5:Gerror("unusual bit array format in bitmap file");
      default:Gerror("error during bitmap read");
    }

  for (i=0; i<BITMAP_ARRAY_SIZE; i++) bitarray[i] = temp[i];
  free(temp);
}




/* bit manipulation routines */

D 4
char getBit(char *source,
		      int wpos,int hpos,
		      int wsize,int hsize)
E 4
I 4
char getBit(source, wpos, hpos, wsize, hsize)
char *source;
int wpos, hpos, wsize, hsize;
E 4
/* returns the specified bit from source */
{
  return get_nth_bit(wpos,source + hpos * wsize);
}



D 4
char get_nth_bit(int num,char *source)
E 4
I 4
char get_nth_bit(num, source)
int num;
char *source;
E 4
/* returns either zero or one depending on the value of the
 * nth bit from the source pointer */
D 4

E 4
{
  while ((num / (sizeof(char) * 8)) >= 1) {
    num -= (sizeof(char) * 8);
    source ++;
  }
  return ((*source >> num) & 1);

}


D 4
putBit(char *source,
		      int wpos,int hpos,
		      int wsize,int hsize,
		      char bit)
E 4
I 4
putBit(source, wpos, hpos, wsize, hsize, bit)
char *source;
int wpos, hpos, wsize, hsize;
char bit;
E 4
/* returns the specified bit from source */
{
  put_nth_bit(bit,wpos,source + hpos * wsize);
}


D 4
put_nth_bit(char bit, int num,char *target)
E 4
I 4
put_nth_bit(bit, num, target)
char bit;
int num;
char *target;
E 4
/* puts bit into the num'th bit of target */
D 4

E 4
{
  while ((num / (sizeof(char) * 8)) >= 1) {
    num -= (sizeof(char) * 8);
    target ++;
  }
 *target |= (bit << num);
}


/* takes a bitmap (char * format) and writes it to screen, with * for
 * WhitePixel and ' ' for BlackPixel */
D 4

void textbit(int width,int height,char *bitmap)
E 4
I 4
void textbit(width, height, bitmap)
int width, height;
char *bitmap;
E 4
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
E 1
