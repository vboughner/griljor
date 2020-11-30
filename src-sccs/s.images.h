h13294
s 00000/00000/00016
d D 1.3 92/08/07 01:01:44 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00003/00003/00013
d D 1.2 91/08/29 01:40:07 vanb 2 1
c fixed up more compatibility problems
e
s 00016/00000/00000
d D 1.1 91/02/16 13:00:42 labc-3id 1 0
c date and time created 91/02/16 13:00:42 by labc-3id
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

/* Bitmap rotation and flipping procedure declarations */

D 2
char	*rotate(int width, int height, char *bitmap, char dir);
char	*hflip(int width, int height, char *bitmap);
char	*vflip(int width, int height, char *bitmap);
E 2
I 2
char	*rotate(/* int width, int height, char *bitmap, char dir */);
char	*hflip(/* int width, int height, char *bitmap */);
char	*vflip(/* int width, int height, char *bitmap */);
E 2
E 1
