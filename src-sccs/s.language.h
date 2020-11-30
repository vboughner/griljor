h43340
s 00000/00000/00032
d D 1.2 92/08/07 01:01:58 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00032/00000/00000
d D 1.1 91/02/16 13:00:45 labc-3id 1 0
c date and time created 91/02/16 13:00:45 by labc-3id
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

/* File to be included when using monster language routines */


/* ================== Monster  L A N G U A G E  routines ================= */

/* The point of these routines is that the makers of monster programs
   will not need to ever call any routines inside of any of the other
   griljor source code.  The routines that follow are the only ones
   they should ever call. */


/* The programmer must call this one once for each of the persons he wishes
   to be allocated for him for the game.  He must finish all his
   calls to this procedure before calling MEnterGame() */
MInit(char *name, char *password, short *bitmap, short *mask);


/* Once the programmer has initialized his persons, he must enter the game
   by calling this procedure, which communicates with the driver and does
   most of the setup */
MEnterGame();
E 1
