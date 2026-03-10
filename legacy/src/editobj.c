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
#include "externX11.h"
#include "obtor.h"
#include "extern.c"

/*****************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
  /* init obtor variables */
  init_obtor_vars();

  /* set object file names */
  set_obtor_defaults();
 
  /* get possibly new and different file names */
  interpret_obtor_options(argc, argv);
  
  /* open up the window display */
  open_display();
 
  /* load the fonts we'll use */
  load_all_fonts();
  
  /* start the windows for the editor */
  obtor_window_setup();
  
  /* set up the initial contents of some global variables */
  obtor_setup_variables();

  /* load default or user-specified files */
  obtor_load_startup_files();

  /* wait for user to do something to the title screen */
  wait_for_ack();

  /* map the editor windows now that bitmaps are defined */
  map_obtor_windows(); 
  
  /* get input and execute commands */
  obtor_get_input();
  
  exit(0);
}

/*****************************************************************/

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



/* sets default filenames object files to load, set other default values */

set_obtor_defaults()
{
  temp_files[1] = DEFAULT_OBJ_PATH;
}



/* interprets the options fed to obtor */

interpret_obtor_options(argc, argv)
     int argc;
     char *argv[];
{
  int i,use;

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
    }
}



/* print usage information */

print_obtor_usage_help(progname)
char *progname;
{
    printf("Usage: %s [-r] objfile1 [objfile2]\n", progname);
    printf("	[-r]           reverse graphics mode\n");
    printf("	object_file1   file of object descriptions\n");
    printf("	[object_file2] file to put in second window\n");
    exit(0);
}

FileChanged(groupnum,flag)
int flag;
int groupnum;
{
  file_changed[groupnum] = flag;
  access_time[groupnum] = CurrentAccessTime++;
  redraw_info_window(groupnum);
}


/* =============== O B J E C T  definitions ============================== */

/* obtor loads object files when they can be found */

obtor_load_startup_files()
{
  int i;
  
  obtor_create_new(0);
  clear_window_space(0);
  place_cut_window(0);

  for(i = 0;i < MAX_TEMPFILES;i++)  /* two possible windows at startup */
    {
      if (temp_files[i])
	obtor_load_object_file(temp_files[i],i+NORM_WINDOW); 
        /* buffer 0 is for the cut buffer */
      else if (i < 2)
	obtor_create_new(i+NORM_WINDOW);
    }
}

/*****************************************************************/

unload_window(num)
{
  if (file_changed[num])
    if (!query_save_win(num))
      return(FALSE);
      
  unmap_data_window(num);

  data_loaded[num] = FALSE;
  return(TRUE);
}

/*****************************************************************/

/* load a given file into a certain object list (num), if the file doesn't
   exist, then just leave the list empty and return FALSE */

obtor_load_object_file(filename, num)
char *filename;
int num;
{
  ObjInfo **read_objects_from_file();
  char newfilename[1024];

  if (data_loaded[num])
    if (!unload_window(num))
      return(FALSE);

  if (file_exists(filename))
    strcpy(newfilename,filename);
  else
    sprintf(newfilename,libbed_filename(OBJ_LIB_DIR,filename));
    
  if (file_exists(newfilename)) 
    if (map_data_window(num))
      {
	objdef[num] = read_objects_from_file(newfilename, &(objheader[num]),
			&(objfile_msgs[num]), &(num_objects[num]), NULL);
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

/*****************************************************************/

obtor_create_new(num)
{
  if (data_loaded[num])
    if (!unload_window(num))
      return(FALSE);
  
  if (map_data_window(num))
    {
      objdef[num] = NULL;
      num_objects[num] = 0;
      objfile_msgs[num] = NULL;
      objheader[num] = new_objheader();
      MakeBlankXData(num,num_objects[num]);
      FileChanged(num,FALSE);
      strcpy(objfile_name[num],"");
      data_loaded[num] = TRUE;
      return(TRUE);
    }
}

/* save the given object group to the given filename */

obtor_save_object_file(filename, num)
char *filename;
int num;
{
  if (!write_objects_to_file(filename, objheader[num], objfile_msgs[num],
			     objdef[num], num_objects[num]))
    {
      obtor_nosave_msg();
      return(False);
    }

  FileChanged(num,FALSE);
  obtor_save_msg();
  return(True);
}

/* replace the definition of an object with something different, by changing
   the pointer on the old one to point to the new definition.  If the old
   one already pointed to something then free it first */

replace_obj_definition(olddef, newdef)
ObjInfo **olddef, *newdef;
{
  if (*olddef) free(*olddef);
  *olddef = newdef;
}


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


/* create a blank definition */

ObjInfo *make_blank_definition()
{
  ObjInfo *result;

  result = allocate_obj();
  initialize_obtor_obj(result);
  return result;
}


/*****************************************************************/

/* a group of objects is in need of filling up to a certain point, fill
   the as yet non-existant definitions with blanks, and redefine their
   pixmaps */

resize_group(groupnum, objnum)
int groupnum, objnum;
{
  int currentobj,firstobj;
  
  if (num_objects[groupnum] <= objnum)
    {
      ResizeXData(groupnum,objnum+1);
      objdef[groupnum] = (ObjInfo *) realloc(objdef[groupnum],
					     (objnum + 1) * sizeof(ObjInfo));
      demand(objdef[groupnum], "No memory for resizing the group");
      
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
}

/* initialize an object to make all flags false */

initialize_obtor_obj(record)
ObjInfo *record;
{
  bzero(record, sizeof(ObjInfo));
}
