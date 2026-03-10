/***************************************************************************
 * @(#) mapstat.h 1.2 - last change made 08/07/92
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

/* map information header */

typedef struct _map_info_record {
	char	*map_name;
	char	*author_name;
	char	*object_filename;
	char	*execute_filename;
	char	*startup_filename;
	char	*placement_filename;
	char	*code_filename;
	char	neutrals_allowed;
	short	num_of_teams;
	char	*team_name[];
	RoomInfoRecord *room[];
} MapInfoRecord;



/* selectors */

#define MAP_name(map)			((map)->map_name)
#define MAP_author_name(map)		((map)->author_name)
#define MAP_object_filename(map)	((map)->object_filename)
#define MAP_execute_filename(map)	((map)->execute_filename)
#define MAP_startup_filename(map)	((map)->startup_filename)
#define MAP_placement_filename(map)	((map)->placement_filename)
#define MAP_code_filename(map)		((map)->code_filename)
#define MAP_neutrals_allowed(map)	((map)->neutrals_allowed)
#define MAP_num_of_teams(map)		((map)->num_of_teams)
#define MAP_team_name(map, num)		((map)->team_name[(num)])
#define MAP_room(map, num)		((map)->room[(num)])
