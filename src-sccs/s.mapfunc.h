h60176
s 00000/00000/00069
d D 1.4 92/08/07 21:04:50 vbo 4 3
c fixes for sun port merged in
e
s 00004/00000/00065
d D 1.3 92/03/11 20:53:10 vanb 3 2
c added to structures for containers, fixed more bugs
e
s 00004/00001/00061
d D 1.2 92/03/02 22:07:34 vanb 2 1
c finished getting editmap to work on basics
e
s 00062/00000/00000
d D 1.1 92/02/15 01:18:16 vanb 1 0
c date and time created 92/02/15 01:18:16 by vanb
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


#define REC_DETAIL	0
#define REC_INFOX	1
#define REC_INFOY	2
#define REC_ZINGER	3
#define REC_EXTRA1	4
#define REC_EXTRA2	5
#define REC_EXTRA3	6
#define REC_NUM_FIELDS	7	/* keep current as number of fields */


D 2
#define MAX_MOVEMENT_NUMBER	9
E 2
I 2
#define ANY_ID			(-1)
E 2

I 2
#define MAX_MOVEMENT_NUMBER	(9)
E 2

I 2

E 2
int rec_obj_default();
void set_record();
int get_record();
OI *create_object();
I 2
OI *create_object_from_mapobj();
E 2
void change_object_type();
OI *duplicate_object();
I 3
int is_valid_square();
E 3
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
I 3
int number_of_objects_on_square();
OI *object_i_on_square();
E 3
int is_object_type_here();
void add_object_replacing_same_mask();
void remove_object_from_map();
void destroy_object();
void destroy_objects_on_square_masked();
I 3
void destroy_objects_in_list();
E 3
void destroy_all_objects_on_square();
void destroy_all_objects_in_room();
E 1
