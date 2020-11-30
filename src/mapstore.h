/***************************************************************************
 * @(#) mapstore.h 1.4 - last change made 09/03/92
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
void lib_add_new_room();
MapInfo *lib_load_map();
int lib_save_map();
void free_instance();
void lib_free_map();
MapInfo *lib_read_map_header_from_fd();
int lib_read_map_obj_from_fd();
void lib_write_map_header_to_fd();
void lib_write_map_objects_to_fd();
