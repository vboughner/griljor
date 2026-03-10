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

#ifndef NORMAL
#include "def.h"
#endif

/* External definitions for use by Main Proogram File */

int	what_am_i;		/* DRIVER, PLAYER, or MONSTER */
int	what_mode = NORMAL;	/* NORMAL, QUITTING, or DYING */
int	time_left = 0;		/* clicks left until quit or death occurs
				   (gametime click when it will happen) */
int	use_display = 0;	/* TRUE if we try to use an X display */
