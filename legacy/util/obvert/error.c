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

/* Error Handling Program File */

#include <stdio.h>
#include "config.h"
#include "def.h"

Gerror(s)
char *s;
{
  fprintf(stderr, "%s: %s\n", PROGRAM_NAME, s);
  exit(1);
}
