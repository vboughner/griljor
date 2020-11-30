h54121
s 00000/00000/00333
d D 1.10 92/08/07 01:00:55 vbo 10 9
c source copied to a separate tree for work on new map and object format
e
s 00003/00001/00330
d D 1.9 92/05/27 21:45:35 vbo 9 8
c made minimal changes required for compile on Sparc2 SVR4
e
s 00003/00004/00328
d D 1.8 91/12/03 17:29:13 labc-4lc 8 7
c implemented header items for object def files
e
s 00000/00004/00332
d D 1.7 91/11/28 00:10:20 labc-4lc 7 6
c nearly finished hidden text items in dialogs
e
s 00015/00054/00321
d D 1.6 91/07/07 18:47:03 labc-3id 6 5
c made messages from object files be loaded and saved right
e
s 00000/00001/00375
d D 1.5 91/06/30 18:38:50 labc-3id 5 4
c took out references to now out-dated object property flags
e
s 00000/00000/00376
d D 1.4 91/05/17 02:06:08 labc-3id 4 3
c Changed for showing object numbers
e
s 00010/00009/00366
d D 1.3 91/03/24 20:17:34 labc-3id 3 2
c y
e
s 00213/00094/00162
d D 1.2 91/03/24 18:39:32 labc-3id 2 1
c 
e
s 00256/00000/00000
d D 1.1 91/02/16 12:53:54 labc-3id 1 0
c date and time created 91/02/16 12:53:54 by labc-3id
e
u
U
f e 0
t
T
I 1
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

/* Map Object Editing Program */
/* In this file, uses of the word "obtor" mean "Map Object Editor" */

#define EDITOBJ_MAIN
#include <stdio.h>
#include <string.h>
D 2
#include "config.h"
#include "def.h"
#include "lib.h"
E 2
I 2
#include "externX11.h"
#include "obtor.h"
E 2
#include "extern.c"
D 2
#include "objects.h"
#include "externX10.h"
#include "editobj.h"
E 2

I 2
/*****************************************************************/
E 2

D 2
/* Global variables for two different object sets */
char	objfile_name[2][PATH_LENGTH];	/* names of object def files */
ObjInfo *objdef[2][MAX_OBJECTS];	/* info on object definitions */
int	num_objects[2];			/* how many objects defined */
int	file_changed[2];		/* TRUE when a change has been made */


E 2
main(argc, argv)
int argc;
char *argv[];
{
I 2
  /* init obtor variables */
  init_obtor_vars();

E 2
  /* set object file names */
  set_obtor_defaults();
D 2

E 2
I 2
 
E 2
  /* get possibly new and different file names */
  interpret_obtor_options(argc, argv);
D 2

  /* load in the map or make a new one if this is a new file name */
  obtor_load_or_create_objects();

E 2
I 2
  
E 2
  /* open up the window display */
  open_display();
D 2

E 2
I 2
 
E 2
  /* load the fonts we'll use */
D 2
  load_both_fonts();

E 2
I 2
  load_all_fonts();
  
E 2
  /* start the windows for the editor */
  obtor_window_setup();
D 2

  /* define the cursors inside the above windows */
  load_obtor_cursors();

E 2
I 2
  
E 2
  /* set up the initial contents of some global variables */
  obtor_setup_variables();

D 2
  /* define the bitmaps neccessary for object plotting */
  define_starting_pixmap_sets();
E 2
I 2
  /* load default or user-specified files */
  obtor_load_startup_files();
E 2

D 2
  /* map the editor windows now that bitmaps are defined */
  map_obtor_windows();
E 2
I 2
  /* wait for user to do something to the title screen */
  wait_for_ack();
E 2

I 2
  /* map the editor windows now that bitmaps are defined */
  map_obtor_windows(); 
  
E 2
  /* get input and execute commands */
  obtor_get_input();
D 2

E 2
I 2
  
E 2
D 6
  if (DEBUG) printf("Chow.\n");
E 6
  exit(0);
}

I 2
/*****************************************************************/
E 2

I 2
init_obtor_vars()
{
  int which;
  
  for (which = 0;which < MAX_TEMPFILES;which++)
    {
      temp_files[0] = NULL;
      temp_files[1] = NULL;
    }
  
  for (which = 0;which < MAX_BUFFERS;which++)
    data_loaded[which] = FALSE;
}
E 2

I 6


E 6
/* sets default filenames object files to load, set other default values */

set_obtor_defaults()
{
D 2
  register int i, j;

  strcpy(objfile_name[0], DEFAULT_OBJ_PATH);
  strcpy(objfile_name[1], "tmp");
  for (j=0; j<2; j++) {
    for (i=0; i<MAX_OBJECTS; i++) objdef[j][i] = NULL;
    num_objects[j] = 0;
    file_changed[j] = FALSE;
  }
E 2
I 2
  temp_files[1] = DEFAULT_OBJ_PATH;
E 2
}

I 2
D 6
/* interrets the options fed to obtor */
E 6
E 2

I 6

/* interprets the options fed to obtor */

E 6
D 2

/* interprets the options fed to obtor */

E 2
interpret_obtor_options(argc, argv)
D 2
int argc;
char *argv[];
E 2
I 2
     int argc;
     char *argv[];
E 2
{
D 2
  int i=1;
E 2
I 2
  int i,use;
E 2

D 2
  if (argc>1)
    if (argv[1][0] == '-') {
      i = 2;
      if (!strcmp(argv[1], "-r")) reversed_screen = TRUE;
      if (!strcmp(argv[1], "-h")) print_obtor_usage_help(argv[0]);
E 2
I 2
  use = 0;
  for(i = 1;i < argc;i++)
    {
      if (argv[i][0] == '-') 
	{
	  if (!strcmp(argv[1], "-r")) reversed_screen = TRUE;
	  if (!strcmp(argv[1], "-h")) print_obtor_usage_help(argv[0]);
	}
      else if (use < MAX_TEMPFILES)
	temp_files[use++] = argv[i];
      else
	print_obtor_usage_help(argv[0]);
E 2
    }
D 2
  if (argc>i) strcpy(objfile_name[0], argv[i]);
  if (argc>i+1) strcpy(objfile_name[1], argv[i+1]);
  if (argc>i+2) print_obtor_usage_help(argv[0]);
E 2
}



/* print usage information */

print_obtor_usage_help(progname)
char *progname;
{
    printf("Usage: %s [-r] objfile1 [objfile2]\n", progname);
D 6
    printf("	[-r] reverse graphics mode (very obnoxious)\n");
    printf("	[object_file1] compiled file of object descriptions\n");
    printf("	[object_file2] second file of object descriptions\n");
E 6
I 6
    printf("	[-r]           reverse graphics mode\n");
    printf("	object_file1   file of object descriptions\n");
    printf("	[object_file2] file to put in second window\n");
E 6
    exit(0);
}

I 2
FileChanged(groupnum,flag)
int flag;
int groupnum;
{
  file_changed[groupnum] = flag;
  access_time[groupnum] = CurrentAccessTime++;
  redraw_info_window(groupnum);
}
E 2


/* =============== O B J E C T  definitions ============================== */

/* obtor loads object files when they can be found */

D 2
obtor_load_or_create_objects()
E 2
I 2
obtor_load_startup_files()
E 2
{
D 2
  printf("load object files\n");
  obtor_load_object_file(objfile_name[0], 0);
  obtor_load_object_file(objfile_name[1], 1);
E 2
I 2
  int i;
  
  obtor_create_new(0);
  clear_window_space(0);
  place_cut_window(0);

  for(i = 0;i < MAX_TEMPFILES;i++)  /* two possible windows at startup */
    {
      if (temp_files[i])
	obtor_load_object_file(temp_files[i],i+NORM_WINDOW); 
D 6
      /* buffer 0 is for the
	 cut buffer */
E 6
I 6
        /* buffer 0 is for the cut buffer */
E 6
      else if (i < 2)
	obtor_create_new(i+NORM_WINDOW);
    }
E 2
}

I 2
/*****************************************************************/
E 2

I 2
unload_window(num)
{
  if (file_changed[num])
    if (!query_save_win(num))
      return(FALSE);
      
  unmap_data_window(num);
E 2

I 2
  data_loaded[num] = FALSE;
  return(TRUE);
}

/*****************************************************************/

E 2
D 3
/* load a given file into either object list 0 or 1, if the file doesn't
   exist, then just leave the list empty */
E 3
I 3
/* load a given file into a certain object list (num), if the file doesn't
   exist, then just leave the list empty and return FALSE */
E 3

obtor_load_object_file(filename, num)
char *filename;
int num;
{
I 6
  ObjInfo **read_objects_from_file();
E 6
D 2
  free_object_definitions(objdef[num]);
  num_objects[num] = 0;
  file_changed[num] = FALSE;
  if (file_exists(filename)) 
    load_file_into_obj_array(filename, objdef[num], &(num_objects[num]));
}
E 2
I 2
  char newfilename[1024];
E 2

I 2
  if (data_loaded[num])
    if (!unload_window(num))
      return(FALSE);
E 2

I 2
  if (file_exists(filename))
D 3
    strcpy(newfilename,filename); /* use this filename */
E 3
I 3
    strcpy(newfilename,filename);
E 3
  else
D 3
    sprintf(newfilename,"lib/%s",filename); /* try an alternate */
E 3
I 3
    sprintf(newfilename,libbed_filename(OBJ_LIB_DIR,filename));
E 3
    
  if (file_exists(newfilename)) 
    if (map_data_window(num))
      {
D 6
	dynamic_load_file(newfilename, &(objdef[num]), &(num_objects[num]));
E 6
I 6
D 8
	objdef[num] = read_objects_from_file(newfilename,
E 8
I 8
	objdef[num] = read_objects_from_file(newfilename, &(objheader[num]),
E 8
			&(objfile_msgs[num]), &(num_objects[num]), NULL);
E 6
	MakeBlankXData(num,num_objects[num]);
	FileChanged(num,FALSE);
	strcpy(objfile_name[num],newfilename);
	objects_into_pixmaps(num);
	data_loaded[num] = TRUE;
	AdjArrowWindows(num);
	return(TRUE);
      }
  
  return(FALSE);
}
E 2

D 2
/* save the given object group to the given filename */
E 2
I 2
/*****************************************************************/
E 2

D 2
obtor_save_object_file(filename, num)
E 2
I 2
D 6
/* load the objects from a file into an object record array, and set count
   to be the number of objects loaded, quit with an error when object file
   cannot be read from either the given name or a file of the same name
   in the object library directory. */

dynamic_load_file(filename,o,count)
E 2
char *filename;
D 2
int num;
E 2
I 2
ObjInfo **o[];
int *count;
E 2
{
D 2
  save_file_from_obj_array(filename, objdef[num], num_objects[num]);
  file_changed[num] = FALSE;
  obtor_save_msg();
}
E 2
I 2
  FILE *fp;
  int i,size;
  ObjInfo *nextobj;
  
  fp = fopen(filename, "r");
I 3
  if (!fp) 
    fp = fopen(libbed_filename(OBJ_LIB_DIR, filename), "r");
E 3
  
D 3
  if (fp == NULL)
    {
      printf("Griljor: was searching for object def file %s\n", filename);
      Gerror("could not find object definition file");
    }
E 3
I 3
  if (!fp) {
    printf("Griljor: was searching for object def file %s\n", filename);
    Gerror("could not find object definition file");
  }
E 3
E 2

I 2
  size = 1;
  *o = (ObjInfo **) malloc(size * sizeof(**o));
E 2

I 2
  for (i=0; !feof(fp); i++)
    {
      nextobj = allocate_obj();
      fread((char *)nextobj, sizeof(ObjInfo), 1, fp);
      if (i >= size)
	{
	  size += 15;
	  *o = (ObjInfo **) realloc(*o,size * sizeof(**o));
	}
      (*o)[i] = nextobj;
    }
E 2

D 2
/* free the records in an object definition list, set count to zero */
E 2
I 2
  fclose(fp);
  *count = i - 1;
}
E 2

D 2
free_object_definitions(o)
ObjInfo *o[];
{
  register int i;
E 2

D 2
  for (i=0; i<MAX_OBJECTS; i++)
    if (o[i]) {
      free(o[i]);
      o[i] = NULL;
E 2
I 2
/*****************************************************************/

E 6
obtor_create_new(num)
{
  if (data_loaded[num])
    if (!unload_window(num))
      return(FALSE);
  
  if (map_data_window(num))
    {
      objdef[num] = NULL;
      num_objects[num] = 0;
I 6
      objfile_msgs[num] = NULL;
I 8
      objheader[num] = new_objheader();
E 8
E 6
      MakeBlankXData(num,num_objects[num]);
      FileChanged(num,FALSE);
      strcpy(objfile_name[num],"");
      data_loaded[num] = TRUE;
      return(TRUE);
E 2
    }
}

I 2
/* save the given object group to the given filename */
E 2

I 2
obtor_save_object_file(filename, num)
char *filename;
int num;
{
D 6
  if (!save_file_from_obj_array(filename, objdef[num], num_objects[num]))
E 6
I 6
D 8
  if (!write_objects_to_file(filename, objfile_msgs[num],
E 8
I 8
  if (!write_objects_to_file(filename, objheader[num], objfile_msgs[num],
E 8
			     objdef[num], num_objects[num]))
E 6
    {
      obtor_nosave_msg();
      return(False);
    }
E 2

I 2
  FileChanged(num,FALSE);
  obtor_save_msg();
  return(True);
}

E 2
/* replace the definition of an object with something different, by changing
   the pointer on the old one to point to the new definition.  If the old
   one already pointed to something then free it first */

replace_obj_definition(olddef, newdef)
ObjInfo **olddef, *newdef;
{
  if (*olddef) free(*olddef);
  *olddef = newdef;
}


D 2

E 2
/* make a copy of an object definition by allocating it in memory, and
   return the address */

ObjInfo *copy_obj_definition(def)
ObjInfo *def;
{
  ObjInfo *result;

  result = allocate_obj();
  bcopy(def, result, sizeof(ObjInfo));
  return result;
}


D 2

E 2
/* create a blank definition */

ObjInfo *make_blank_definition()
{
  ObjInfo *result;

  result = allocate_obj();
  initialize_obtor_obj(result);
  return result;
}


I 2
/*****************************************************************/
E 2

/* a group of objects is in need of filling up to a certain point, fill
   the as yet non-existant definitions with blanks, and redefine their
   pixmaps */

D 2
fill_with_blanks_upto(groupnum, objnum)
E 2
I 2
resize_group(groupnum, objnum)
E 2
int groupnum, objnum;
{
D 2
  int currentobj;

  for (currentobj = num_objects[groupnum];
       currentobj <= objnum; currentobj++) {
		replace_obj_definition(&(objdef[groupnum][currentobj]),
				       make_blank_definition());
		certain_object_pixmap(groupnum, currentobj);
  }
  file_changed[groupnum]++;
  num_objects[groupnum] = objnum + 1;
E 2
I 2
  int currentobj,firstobj;
  
  if (num_objects[groupnum] <= objnum)
    {
      ResizeXData(groupnum,objnum+1);
D 9
      RESIZE(objdef[groupnum],objnum+1,"No memory for resizing the group");
E 9
I 9
      objdef[groupnum] = (ObjInfo *) realloc(objdef[groupnum],
					     (objnum + 1) * sizeof(ObjInfo));
      demand(objdef[groupnum], "No memory for resizing the group");
E 9
      
      firstobj = num_objects[groupnum];
      num_objects[groupnum] = objnum + 1;
      
      for (currentobj = firstobj; currentobj <= objnum; currentobj++) 
	{
	  objdef[groupnum][currentobj] = NULL;  /* so replace_obj_definition
						   does not try and free it */
	  ClearXData(groupnum,currentobj);
	  replace_obj_definition(&(objdef[groupnum][currentobj]),
				 make_blank_definition());
	  certain_object_pixmap(groupnum, currentobj);
	  redraw_grab_square(groupnum,currentobj,FALSE);
	}
      FileChanged(groupnum,TRUE);
      AdjArrowWindows(groupnum);
      redraw_info_window(groupnum);
    }
E 2
}

D 2


E 2
/* initialize an object to make all flags false */

initialize_obtor_obj(record)
ObjInfo *record;
{
  bzero(record, sizeof(ObjInfo));
D 7
  strcpy(record->name, "no name");
I 2
  record->transparent = True;
  record->permeable = True;
D 5
  record->move = True;
E 5
  record->movement = 9;
E 7
E 2
}
I 2
D 8


E 8
E 2
E 1
