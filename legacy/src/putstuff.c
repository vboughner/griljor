/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * February 1991
 **************************************************************************/


#include <stdio.h>
#include "config.h"
#include "def.h"
#include "lib.h"
#include "putstuff.h"


CheckForPutStuffIn(data, t, players)
DropShipment *data;
unsigned long t;
int players;
/* this checks the time of last placement with t, the current time,
   and calls for placement of something if enough time has passed.
   A greater number of players will bring about placements more often. */
{
  if (!data) return;
  if (!players) return;

  if (t >= data->LastPlacement + max((data->HowOften / players), 1)) {
    data->LastPlacement = t;
    (void) PutStuffInGame(data);
  }
}


boolean PutStuffInGame(data)
DropShipment *data;
/* called during game, puts one line worth of stuff down randomly as 
 * specified by the information in data.  Returns TRUE if it was
   successful in placing things. */

{
  int i,outroom,stat,line,tries;

  if (!data) return FALSE;
  line = lrand48() % data->NumLines;

  for (i = 0 ; i < data->file[line].quantity ; i ++) {
    if (data->file[line].command == 't') {
      for (tries=0,stat=0; (tries<16 && !stat); tries++)  {
        outroom = random_team_room(data->file[line].where);
        stat = random_placement(data->file[line].objnum,outroom);
      }
    }
    else if (data->file[line].command == 'r') {
      outroom = data->file[line].where;
      stat = random_placement(data->file[line].objnum,outroom);
    }
    else stat = FALSE;
    if (!stat) break;
  }

  return stat;
}



DropShipment *ReadStuffFile(filename)
char *filename;
/* reads the file to contain in game random placement information */
{
  FILE *infile = fopen(filename,"r");
  char line[MAXSTRLEN];
  char com;
  int objnum,quant,where,often;
  DropShipment *data = (DropShipment *)malloc(sizeof(DropShipment));
  demand(data,"Malloc failed in ReadStuffFile.\n");

  data->NumLines = 0;

  if (!infile) infile = fopen(libbed_filename(MAP_LIB_DIR, filename), "r");
  if (!infile) {
    printf("warning: obj placement file '%s' couldn't be read\n", filename);
    return 0;
  }
 
  data->HowOften = DEFAULT_HOW_OFTEN;
  data->LastPlacement = 0;

  while (fgets(line,MAXSTRLEN-1,infile)) {
    if (data->NumLines == MAXLINES) break;
    if (*line == '#') continue;
    if (*line == 's' || *line == 'S') {
      if (sscanf(line, "%c %d",&com,&often)!=2) continue;
      data->HowOften = often;
    }
    else {
      if (sscanf(line,"%c %d %d %d",&com,&objnum,&quant,&where)!=4) continue;
      data->file[data->NumLines].command  = com;
      data->file[data->NumLines].objnum   = objnum;
      data->file[data->NumLines].quantity = quant;
      data->file[data->NumLines].where    = where;    
      data->NumLines++;
    }
  }

  fclose(infile);
  return data;
}
