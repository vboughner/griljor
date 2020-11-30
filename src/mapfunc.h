/***************************************************************************
 * @(#) mapfunc.h 1.4 - last change made 08/07/92
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


#define REC_DETAIL	0
#define REC_INFOX	1
#define REC_INFOY	2
#define REC_ZINGER	3
#define REC_EXTRA1	4
#define REC_EXTRA2	5
#define REC_EXTRA3	6
#define REC_NUM_FIELDS	7	/* keep current as number of fields */


#define ANY_ID			(-1)

#define MAX_MOVEMENT_NUMBER	(9)


int rec_obj_default();
void set_record();
int get_record();
OI *create_object();
OI *create_object_from_mapobj();
void change_object_type();
OI *duplicate_object();
int is_valid_square();
SquareRec *get_square();
OI *square_first_masked();
OI *square_last_masked();
int view_obstructed_by_square();
int movement_on_square();
int firing_onto_square();
int firing_through_square();
void copy_square_overwrite();
OI *first_obj_here();
OI *last_obj_here();
OI *what_exit_obj_here();
void object_in_map();
void add_object_to_square();
void add_duplicate_to_square();
int number_of_objects_on_square();
OI *object_i_on_square();
int is_object_type_here();
void add_object_replacing_same_mask();
void remove_object_from_map();
void destroy_object();
void destroy_objects_on_square_masked();
void destroy_objects_in_list();
void destroy_all_objects_on_square();
void destroy_all_objects_in_room();
