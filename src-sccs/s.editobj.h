h12479
s 00000/00000/00054
d D 1.4 92/08/07 01:00:57 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00019/00006/00035
d D 1.3 91/07/07 18:47:07 labc-3id 3 2
c made messages from object files be loaded and saved right
e
s 00025/00010/00016
d D 1.2 91/03/24 18:40:05 labc-3id 2 1
c 
e
s 00026/00000/00000
d D 1.1 91/02/16 13:00:26 labc-3id 1 0
c date and time created 91/02/16 13:00:26 by labc-3id
e
u
U
f e 0
t
T
I 1
D 2
/***************************************************************************
E 2
I 2
/*********************************************************************
E 2
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
D 2
 **************************************************************************/
E 2
I 2
 ********************************************************************/
E 2

/* Header file for map object editor */

I 2
#define MAX_BUFFERS     10
#define MAX_TEMPFILES   5
D 3
#define NORM_WINDOW     1
E 3
I 3
#define NORM_WINDOW     1	/* one window for the cut buffer */
E 3

E 2
/* Global variables */
D 2
#ifndef EDITOBJ_MAIN
extern char	objfile_name[2][PATH_LENGTH];	/* names of object def files */
extern ObjInfo	*objdef[2][MAX_OBJECTS];	/* info on object defs */
extern int	num_objects[2];			/* how many objects defined */
extern int	file_changed[2];		/* TRUE when a change made */
#endif
E 2
I 2
D 3
extern char    *temp_files[MAX_TEMPFILES];
E 3
I 3
extern char	*temp_files[MAX_TEMPFILES];

E 3
extern char	objfile_name[MAX_BUFFERS][PATH_LENGTH];   	
                                        /* names of object def files */
D 3
extern ObjInfo **objdef[MAX_BUFFERS];	
E 3
I 3

extern ObjInfo	**objdef[MAX_BUFFERS];	
E 3
                                        /* info on object definitions */
I 3

extern VLine	*objfile_msgs[MAX_BUFFERS];
					/* pointers to message lists */

E 3
extern int	num_objects[MAX_BUFFERS];
                                   	/* how many objects defined */
I 3

E 3
extern int	file_changed[MAX_BUFFERS];
                                    	/* TRUE when a change has been made */
D 3
extern int      access_time[MAX_BUFFERS]; 
E 3
I 3

extern int	access_time[MAX_BUFFERS]; 
E 3
                                        /* last time of access */
D 3
extern int     data_loaded[MAX_BUFFERS];       
E 3
I 3

extern int	data_loaded[MAX_BUFFERS];       
E 3
                                        /* True is other dada is valid */
D 3
extern int      CurrentAccessTime;
E 3
E 2

I 3
extern int	CurrentAccessTime;


E 3
I 2
#define IS_VALID_OBJ(group, num)      ((group < 0) ? False : \
				       ((num < num_objects[group])&& \
					(num >= 0)))
I 3

E 3
#define OBJ_DEF(group)		        (objdef[group])
I 3

E 3
#define NUM_OBJ(group)                  (num_objects[group])
E 2

D 2
#define IS_VALID_OBJ(group, num)	(num < num_objects[group])
#define OBJ_DEF(group, num)		(objdef[group][num])
E 2
ObjInfo *copy_obj_definition();
ObjInfo *make_blank_definition();
E 1
