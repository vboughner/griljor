h37522
s 00000/00000/00026
d D 1.3 92/08/07 01:02:48 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00002/00000/00024
d D 1.2 91/12/03 17:29:35 labc-4lc 2 1
c implemented header items for object def files
e
s 00024/00000/00000
d D 1.1 91/07/13 17:25:24 labc-3id 1 0
c date and time created 91/07/13 17:25:24 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

/* Object header and message text dechipering - header file */

VLine *make_count_accurate();
I 2
VLine *os_set_name(), *os_set_author(), *os_set_codefilename();
char *os_extract_name(), *os_extract_author(), *os_extract_codefilename();
E 2
E 1
