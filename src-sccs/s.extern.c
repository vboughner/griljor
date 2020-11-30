h05235
s 00000/00000/00022
d D 1.3 92/08/07 01:01:11 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00004/00000/00018
d D 1.2 91/04/14 23:06:11 labc-3id 2 1
c 
e
s 00018/00000/00000
d D 1.1 91/02/16 12:54:06 labc-3id 1 0
c date and time created 91/02/16 12:54:06 by labc-3id
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

I 2
#ifndef NORMAL
#include "def.h"
#endif

E 2
/* External definitions for use by Main Proogram File */

int	what_am_i;		/* DRIVER, PLAYER, or MONSTER */
int	what_mode = NORMAL;	/* NORMAL, QUITTING, or DYING */
int	time_left = 0;		/* clicks left until quit or death occurs
				   (gametime click when it will happen) */
int	use_display = 0;	/* TRUE if we try to use an X display */
E 1
