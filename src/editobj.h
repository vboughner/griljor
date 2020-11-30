/*********************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 ********************************************************************/

/* Header file for map object editor */

#define MAX_BUFFERS     10
#define MAX_TEMPFILES   5
#define NORM_WINDOW     1	/* one window for the cut buffer */

/* Global variables */
extern char	*temp_files[MAX_TEMPFILES];

extern char	objfile_name[MAX_BUFFERS][PATH_LENGTH];   	
                                        /* names of object def files */

extern ObjInfo	**objdef[MAX_BUFFERS];	
                                        /* info on object definitions */

extern VLine	*objfile_msgs[MAX_BUFFERS];
					/* pointers to message lists */

extern int	num_objects[MAX_BUFFERS];
                                   	/* how many objects defined */

extern int	file_changed[MAX_BUFFERS];
                                    	/* TRUE when a change has been made */

extern int	access_time[MAX_BUFFERS]; 
                                        /* last time of access */

extern int	data_loaded[MAX_BUFFERS];       
                                        /* True is other dada is valid */

extern int	CurrentAccessTime;


#define IS_VALID_OBJ(group, num)      ((group < 0) ? False : \
				       ((num < num_objects[group])&& \
					(num >= 0)))

#define OBJ_DEF(group)		        (objdef[group])

#define NUM_OBJ(group)                  (num_objects[group])

ObjInfo *copy_obj_definition();
ObjInfo *make_blank_definition();
