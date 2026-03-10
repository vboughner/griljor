/***********************************************
 *					       *
 * Bitmap conversion program (bmconversion).   *
 *					       *
 * Written by : Eric van Bezooijen.            *
 *					       *
 * To aid Van A. Boughner, Mel Nicholson, and  *
 * Albert C. Baker III with converting their   *
 * game GRILJOR to X11 format from X10.        *
 *					       *
 * We are all students at the University of    *
 * California at Berkeley.		       *
 *				               *
 * Description of Program :		       *
 *					       *
 * This program converts bitmaps stored in X10 *
 * format to X11 form.  This is done by setting*
 * up a graphics display area in X11, and load-*
 * ing in X10 (or X11, but that does not acc-  *
 * omplish anything), and then storing them.   *
 * X11 can read both X10 and X11 formats, so   *
 * bitmaps are automatically converted into X11*
 * bitmap form by the X11 routines.	       *
 *					       *
 * Features :				       *
 *					       *
 * To run the program simply type <object code *
 * name> <file list>. The <file list> is a list*
 * of bitmap files that are in X10 or X11 for- *
 * mat.  The X10 bitmaps WILL be erased, unless*
 * the NOT_ERASE parameter is uncommented.     *
 * This will tack the FILE_SUFFIX to every file*
 * being converted.  Also, uncomment VERBOSE to*
 * have a list of the files being converted.   *
 *					       *
 * Possible improvements :		       *
 *					       *
 * You might want to include error-checking.   * 
 * This program will cause an X-error if it    *
 * tries to convert a non-bitmap file. Also,   *
 * when saving files with a suffix attached,   *
 * not only will the name of the file stored   *
 * have a suffix, but also the name of the bit-*
 * map in the bitmap file.                     *
 *					       *
 * Usage suggestions :			       *
 *					       *
 * To convert a bunch of files at once, you    *
 * might try doing this in UNIX:	       *
 *					       *
 * foreach a (*)			       *
 * <object code name> $a		       *
 * end					       *
 *					       *
 * Or, if you have bitmaps and other files     *
 * mixed together, but all your bitmaps end    *
 * with '.X10' or something :		       *
 *					       *
 * foreach a (*.X10)			       *
 * <object code name> $a		       *
 * end					       *
 *					       *
 * This program was written on August 30, 1990,*
 * and is intended to be used as a utility to  * 
 * Griljor.  It, however may be freely dist-   *
 * ributed amongst anything as public domain.  *
 * But I am not responsible.		       *
 *					       *
 ***********************************************/

/* X includes */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/* Standard includes */

#include <stdio.h>

/* String includes */

/* #include <string.h> */ /* Uncomment these if you need to. */

#define MAX_FILE_NAME_SIZE  500  /* Maximum size of a file name, anything
				   bigger will result in a segmentation
				   fault */

#define VERBOSE 1 		     /* uncomment this for a verbose run */

/* #define NOT_ERASE 1 */            /* uncomment these to to have your  */
/* #define FILE_SUFFIX ".X11" */     /* original bitmaps not erased      */

void main(argc, argv)
     int argc;
     char *argv[];
     
{
  
  Display *display;
  int screen;
  
  char		file_name [MAX_FILE_NAME_SIZE];
  unsigned int	width, height;
  int		x_hot, y_hot, x = 0 , y = 0, border_width = 4;
  Pixmap	bitmap;
  char		*display_name = NULL;
  
  int		num_arguments, length;
  
  
  /* setup X environment */
  
  if ( (display = XOpenDisplay (display_name)) == NULL)
    
    {
      
      (void) fprintf (stderr, "Bmconversion : Cannot connect to X server! \n");
      exit (-1);
      
    }
  
  screen = XDefaultScreen (display);
  
  /* now go through the arguments, and load up the bitmaps */
  
  for (num_arguments = argc - 1; num_arguments ;--num_arguments)
    
    
    {
      
#ifdef VERBOSE
      printf ("Processing File : %s\n", argv [num_arguments]);
#endif
      /* copy command line argument to file_name, the string 
	 used by XReadBitmapFile and XWriteBitMapFile */
      
      sprintf (file_name, argv [num_arguments]);
      
      /* Load the bitmap */
      
      XReadBitmapFile (display, RootWindow (display, screen),
		       file_name,
		       &width, &height, &bitmap,
		       &x_hot, &y_hot );
#ifdef NOT_ERASE
      length = (strlen (file_name));
      
      if (strcmp (FILE_SUFFIX, 
		  &(file_name [length - strlen (FILE_SUFFIX)])))
	
	sprintf (&(file_name[strlen (file_name)]), FILE_SUFFIX);
#endif
      
      /* Write to the file, overwriting it 
	 if ERASE_NOT is commented */
      
      XWriteBitmapFile (display, file_name,
			bitmap, width, height,
			x_hot, y_hot);
      
      
    }
  
  /* Close up the display and exit */
  
  XCloseDisplay (display);
}

