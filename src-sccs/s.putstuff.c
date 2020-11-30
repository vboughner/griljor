h41558
s 00000/00000/00111
d D 1.4 92/08/07 01:03:34 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00008/00003/00103
d D 1.3 91/08/26 00:34:42 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00006/00003/00100
d D 1.2 91/04/17 16:56:33 labc-3id 2 1
c added ratio of stuff to players in game
e
s 00103/00000/00000
d D 1.1 91/02/16 12:55:41 labc-3id 1 0
c date and time created 91/02/16 12:55:41 by labc-3id
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
 * February 1991
 **************************************************************************/


#include <stdio.h>
#include "config.h"
#include "def.h"
I 2
#include "lib.h"
E 2
#include "putstuff.h"


D 2
CheckForPutStuffIn(DropShipment *data, unsigned long t)
E 2
I 2
D 3
CheckForPutStuffIn(DropShipment *data, unsigned long t, int players)
E 3
I 3
CheckForPutStuffIn(data, t, players)
DropShipment *data;
unsigned long t;
int players;
E 3
E 2
/* this checks the time of last placement with t, the current time,
D 2
   and calls for placement of something if enough time has passed. */
E 2
I 2
   and calls for placement of something if enough time has passed.
   A greater number of players will bring about placements more often. */
E 2
{
  if (!data) return;
I 2
  if (!players) return;
E 2

D 2
  if (t >= data->LastPlacement + data->HowOften) {
E 2
I 2
  if (t >= data->LastPlacement + max((data->HowOften / players), 1)) {
E 2
    data->LastPlacement = t;
    (void) PutStuffInGame(data);
  }
}


D 3
boolean PutStuffInGame(DropShipment *data)
E 3
I 3
boolean PutStuffInGame(data)
DropShipment *data;
E 3
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



D 3
DropShipment *ReadStuffFile(char *filename)
E 3
I 3
DropShipment *ReadStuffFile(filename)
char *filename;
E 3
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
E 1
