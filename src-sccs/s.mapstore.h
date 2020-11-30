h63803
s 00002/00000/00035
d D 1.4 92/09/03 16:14:09 vbo 4 3
c added a couple more prototypes
e
s 00000/00000/00035
d D 1.3 92/08/07 21:31:35 vbo 3 2
c fixes for sun port merged in
e
s 00003/00002/00032
d D 1.2 92/03/02 22:07:38 vanb 2 1
c finished getting editmap to work on basics
e
s 00034/00000/00000
d D 1.1 92/02/15 01:18:40 vanb 1 0
c date and time created 92/02/15 01:18:40 by vanb
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

/* prototypes for mapstore.c routines */

ObjectHash *find_hash_by_id();
void add_object_to_hash();
void remove_object_from_hash();
SquareRec *create_square();
MapInfo *lib_create_new_map();
D 2
void set_room_defaults();
E 2
void lib_add_new_room();
MapInfo *lib_load_map();
D 2
void lib_save_map();
E 2
I 2
int lib_save_map();
E 2
void free_instance();
void lib_free_map();
I 2
MapInfo *lib_read_map_header_from_fd();
int lib_read_map_obj_from_fd();
I 4
void lib_write_map_header_to_fd();
void lib_write_map_objects_to_fd();
E 4
E 2
E 1
