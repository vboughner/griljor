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

/* Main monster branchoffs */

#include <stdio.h>
#include <string.h>
#include "bit/gm"
#include "bit/gmask"
#include "bit/dodger"

#define FALSE	0
#define TRUE	1

/* this procedure is called by the main procedure in order to start
   up the monsters */

mel_be_monster(char *name)
{ 
   MLoadSpecs(); /* set all default info for monster and load variable specs*/

   for(i=0; i<NumBodies; i++) MInit(/*blah, blah*/);

   MEnterGame();

   for(i=0; i<NumBodies; i++) {
     Mbirth(i);
     MSetPacketPeekRoutine(i);
     MSetActionRoutine(i);
   }

   while(!MUpdate()) for(i=0; i<NumBodies; i++) MDoAction(i);

   MLeaveGame();
}
/* The below is about to be made outdated */
be_monster(char *name)
{
  if      (!strcmp(name, "dweeb")) be_dweeb();
  else if (!strcmp(name, "dodger")) be_dodger();
  else if (!strcmp(name, "test")) be_dodger();
  else if (!strcmp(name, "GM")) be_gm();
}


MDoAction(int i)
{
  if(MQAlive(i)) (gameperson[i]->monsterstuff->movefunct)(i);
}

be_dweeb()
{
  register int i;
  int num_dweebs = 5;

  /* initialize five different dweebs */
  for (i=0; i<num_dweebs; i++)  MInit("dweeb", 3, FALSE, NULL, NULL);

  /* enter the game */
  MEnterGame();

  /* find starting locations for all five dweebs */
  for (i=0; i<num_dweebs; i++)  MBirth(i);

  /* loop through their limited intelligence routine */
  while (!MUpdate()) {
    /* look for a chance to move each one*/
    for (i=0; i<num_dweebs; i++) dweeby_move(i);
  }

  /* leave the game */
  MLeaveGame();
}



be_dodger()
{
  register int i;
  int num_dodgers = 10;

  /* initialize ten different dodgers */
  for (i=0; i<num_dodgers; i++)  MInit("dodger", 2, FALSE, dodger_bits, NULL);

  /* enter the game */
  MEnterGame();

  /* find starting locations for all dodgers */
  for (i=0; i<num_dodgers; i++)  MBirth(i);

  /* loop through their limited intelligence routine */
  while (!MUpdate()) {
    /* look for a chance to move each one*/
    for (i=0; i<num_dodgers; i++) dodgy_move(i);
  }

  /* leave the game */
  MLeaveGame();
}




/* the gm character is meant to be used as a machine assisted player */

be_gm()
{
  register int i;

  /* initialize gm */
  MInit("GM", 30, TRUE, gm_bits, gmask_bits);

  /* enter the game */
  MEnterGame();

  /* find starting locations for gms */
  MBirth(0);

  /* loop through his limited intelligence routine */
  while (!MUpdate()) {
    /* look for a chance to move */
    dodgy_move(0);
  }

  /* leave the game */
  MLeaveGame();
}




dweeby_move(int num)
{
  int x, y;

  if (!MQMoveDone(num)) return;
  x = MQRandX();
  y = MQRandY();
  MLineMove(num, x, y);
}



dodgy_move(int num)
{
  /* move when we are not moving and we are in danger */
  if (MQMoveDone(num))
    if (MQBangSquare(MQRoom(num), MQX(num), MQY(num), 4))
      safe_monster_move(num);
}



/* look for a safe random direction to move into */

safe_monster_move(int num)
{
  int x, y, nx, ny, okay, count = 0;

  x = MQRandX();
  y = MQRandY();
  MLineMove(num, x, y);
}
