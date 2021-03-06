/***************************************************************************
 * @(#) inventory.h 1.1 - last change made 09/02/92
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

/* prototypes */
int person_burden();
void swap_hand_and_inv();
int take_object_from_ground();
int drop_object_from_inv();
void drop_everything();
int drop_type_near_square();
int remove_with_reload();
int drop_out_of_hand();
OI *copy_fired_object();
void reduce_count();
void increase_count();
int query_count();
int types_match();
int ids_compatible();
int has_uses_left();
int find_object_in_inventory();
