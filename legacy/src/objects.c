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

/* Object Handling routines */


#define OBJ_MAIN	/* will tell objects.h not to define externals */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "def.h"
#include "objects.h"
#include "lib.h"
#include "objinfo.h"


/* Global Variables */

ObjInfo		**info;			/* array of pointers to obj info */
void		*obj_messages;		/* linked list of object messages */
ObjHeader	*obj_header;		/* header information for obj set */
int		objects = 0;		/* how many objects are defined */



/* ============================ M I S C ================================= */


ObjInfo *allocate_obj()
{
  ObjInfo *result;
  result = (ObjInfo *) malloc(sizeof(ObjInfo));
  if (result == NULL) Gerror("out of memory while loading object info");

  return result;
}



initialize_obj_record(record)
/* initialize the object to make all flags false, or empty */
ObjInfo *record;
{
  char *p;
  int  i;
  
  for (i=0,p=(char *) record; i<sizeof(ObjInfo); i++,p++) *p = 0;
}



void free_objects()
/* free the objects */
{
  int i;

  for (i=0; i<objects; i++) free(info[(uc)i]);
  objects = 0;
}



/* ================ R E C O R D S  to strings ============================ */


char *object_info_line(record)
/* makes a string containing information about an object's characteristics,
   this string is suitable as the flag line in an object info file */
ObjInfo *record;
{
  static char *result = "New obj info under construction";

  return result;
}
  


/* =============== L O A D I N G  and  S A V I N G ======================= */


load_obj_file(filename)
/* load the objects for a file into the global info array */
char *filename;
{
  ObjInfo **read_objects_from_file();

  info = read_objects_from_file(filename, &obj_header, &obj_messages,
				&objects, NULL);
  if (!info) {
    printf("Griljor: could not find file %s\n", filename);
    Gerror("could not find object definition file");
  }
}



save_obj_file(filename)
/* saves the default object file from the global info array,
   return FALSE when object file cannot be written. */
char *filename;
{
  return write_objects_to_file(filename, obj_header, obj_messages,
			       info, objects);
}



/* ================= S O C K E T  T R A N S F E R R A L ================== */


void receive_obj_file(socknum)
/* read the object file from the given file device number */
int socknum;
{
  ObjInfo **read_objects_from_fd();
  info = read_objects_from_fd(socknum, &obj_header, &obj_messages, &objects);
}



void send_obj_file(socknum)
/* transmit the object file over the given stream socket number */
int socknum;
{
  write_objects_to_fd(socknum, obj_header, obj_messages, info, objects);
}
