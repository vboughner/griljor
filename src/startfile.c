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

/* Routines for the driver to use a map's startup object placement file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "lib.h"


int PutStuff(filename)
char *filename;
/* called at beginning of game: randomly puts stuff down as designated by
 * a map's startup_file.  This helps insure that each incarnation of a
 * certain map is unique */
{
  FILE *infile = fopen(filename,"r");
  int i,objnum,amount,room,outroom,stat,tries,c;
  char type;

  if (!infile) infile = fopen(libbed_filename(MAP_LIB_DIR, filename), "r");

  if (!infile) {
    printf("Warning: map's startup file '%s' could not be read\n", filename);
    return;
  }

  while ((c = fgetc(infile)) != EOF) {

    if (c == '#') {
      for (; c != '\n' && c != EOF ; c = fgetc(infile));
      continue;
    }

    /* get information on what kind of placement to do */
    fscanf(infile,"%d %d %d\n", &objnum, &amount, &room);
    type = tolower(c);
    if (type == 'r') outroom = room;
    else if (type != 't')
      Gerror("map startup file syntax error - first char not t or r");

    /* attempt to make the placement requested, team placements allow
       for several attempts to find a good room, but room placements
       will give up immediately if the room selected was bad. */
    for (i = 0; (i < amount) ; i++) {
      if (type == 't')
        for (tries=0,stat=0; (tries<25 && !stat); tries++) { 
          outroom= random_team_room(room);
	  stat = initial_random_placement(objnum,outroom);
	}
      else stat = initial_random_placement(objnum,outroom);
      if (!stat) {
        printf("Warning: couldn't place startup request:  %c %d %d %d\n",
	       type, objnum, amount, room);
	break;
      }
    }

  }
  fclose(infile);
}
