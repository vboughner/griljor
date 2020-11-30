h26022
s 00000/00000/00030
d D 1.2 92/08/07 21:49:07 vbo 2 1
c fixes for sun port merged in
e
s 00030/00000/00000
d D 1.1 92/02/15 09:47:56 vanb 1 0
c date and time created 92/02/15 09:47:56 by vanb
e
u
U
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

/* prototypes for routines in emap.c */

void link_one_room_to_another();
int opposite_direction();
void destroy_direction_link();
void destroy_exit_link();
void copy_edge_of_room();
OI *nearby_exit_character();
int follow_insinuation();
E 1
