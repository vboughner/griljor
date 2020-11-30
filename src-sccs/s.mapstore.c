h32698
s 00000/00000/00031
d D 1.2 92/08/07 01:02:37 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00031/00000/00000
d D 1.1 91/12/17 22:47:32 labc-4lc 1 0
c date and time created 91/12/17 22:47:32 by labc-4lc
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

/* Map storage and transfer routines.  These routines comprise the only
   method by which a map should be written or read from a file device. */

#include <stdio.h>
#include <errno.h>
#include "def.h"
#include "objects.h"
#include "dialog.h"
#include "vline.h"
#include "map.h"
E 1
