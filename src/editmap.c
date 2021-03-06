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

/* Map Editing Main File */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.c"
#include "objects.h"
#include "map.h"
#include "externX11.h"



main(argc, argv)
int argc;
char *argv[];
{
  /* set default map file and object file names */
  set_editor_defaults();

  /* get possibly new and different file names */
  interpret_editor_options(argc, argv);

  /* load in the map or make a new one if this is a new file name */
  editor_load_or_create_map();

  /* open up the window display */
  open_display();

  /* load the fonts we'll use */
  load_both_fonts();

  /* start the windows for the editor */
  editor_window_setup();

  /* define the cursors inside the above windows */
  load_editor_cursors();

  /* define the bitmaps neccessary for object plotting */
  define_object_pixmaps();

  /* map the editor windows now that bitmaps are defined */
  map_editor_windows();

  /* set up the initial contents of the editing screen */
  editor_setup_variables();

  /* get input and execute commands */
  editor_get_input();

  exit(0);
}



/* sets default filenames for map file and object file to load */

set_editor_defaults()
{
  strcpy(mapfile, DEFAULT_MAP_PATH);
  strcpy(objectfile, UNKNOWN);
}



/* interprets the options fed to editor */

interpret_editor_options(argc, argv)
int argc;
char *argv[];
{
  int i=1;

  if (argc>1)
    if (argv[1][0] == '-') {
      i = 2;
      if (!strcmp(argv[1], "-r")) reversed_screen = TRUE;
      if (!strcmp(argv[1], "-h")) print_editor_usage_help(argv[0]);
    }
  if (argc>i) strcpy(mapfile, argv[i]);
  if (argc>i+1) strcpy(objectfile, argv[i+1]);
  if (argc>i+2) print_editor_usage_help(argv[0]);
}



/* print usage information */

print_editor_usage_help(progname)
char *progname;
{
    printf("Usage: %s [-r] map_file [object_file]\n", progname);
    printf("[-r] reverse graphics mode (very obnoxious)\n");
    printf("[object_file] compiled file of object descriptions\n");	   
    exit(0);
}



/* editor loads or creates a map depending on whether the map file
   specified is in existance already or not */

editor_load_or_create_map()
{
  FILE *fp;
  
  fp = fopen(mapfile, "r");

  if (fp == NULL) {
    if (!strcmp(objectfile, UNKNOWN)) strcpy(objectfile, DEFAULT_OBJ_PATH);
    create_new_map(objectfile);
  }
  else {
    fclose(fp);
    load_map();
  }
}
