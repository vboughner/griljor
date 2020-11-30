/***************************************************************************
 * @(#) mapstore.c 1.2 - last change made 08/07/92
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
