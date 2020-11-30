h57379
s 00000/00000/00199
d D 1.3 92/08/07 01:02:06 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00001/00000/00198
d D 1.2 91/10/16 20:05:43 labc-4lc 2 1
c changed made for DecStations
e
s 00198/00000/00000
d D 1.1 91/03/24 18:45:08 labc-3id 1 0
c date and time created 91/03/24 18:45:08 by labc-3id
e
u
U
f e 0
t
T
I 1
#include <dirent.h>
#include "obtor.h"
  
/*****************************************************************/

get_bitmap_directory(win)
int win;
{
I 2
  char *getenv();
E 2
  char *directory,message[1000],fullname[1000],newdir[1000];
  DIR *dir;
  struct dirent *next;
  int count=0,masks=0,length;

  directory = get_obtor_input_line("Directory: ", 119);
  if (strlen(directory) != 0)
    {  
      if (directory[0] == '~')
	{
	  strcpy(newdir,getenv("HOME"));
	  strcat(newdir,&(directory[1]));
	  directory = newdir;
	}

      dir = opendir(directory);
      if (dir == NULL)
	{
	  sprintf(message,"Directory %s could not be opened.",directory);
	  place_obtor_message(message);
	  return;
	}

      while (next = readdir(dir))
	{
	  sprintf(fullname,"%s/%s",directory,next->d_name);
	  length = strlen(fullname);
	  if (length > 6)
	    if (strcmp(".mask",fullname+length-5))
	      if (try_bitmap(fullname,win))
		{
		  count++;
		  
		  if (try_mask(fullname,win))
		    {
		      sprintf(message,"Bitmap %s and mask loaded.",fullname);
		      place_obtor_message(message);
		      masks++;
		    }
	    }
	}
      
      closedir(dir);
      sprintf(message,"%d bitmaps and %d masks loaded.",count,masks);
      place_obtor_message(message);
    }
  else
    place_obtor_message("No directory loaded.");
}

/*****************************************************************/

get_bitmap(win)
int win;
{
  char *filename,message[1000];

  filename = get_obtor_input_line("Filename: ", 119);
  if (strlen(filename) != 0)
    {
      if (try_bitmap(filename,win))
	if (try_mask(filename,win))
	  {
	    sprintf(message,"Bitmap and mask loaded for %s.",filename);
	    place_obtor_message(message);
	    return;
	  }
    }
  else
    place_obtor_message("No file loaded.");
}

/*****************************************************************/

try_bitmap(name,win)
char *name;
int win;
{
  char newname[1024],message[1024]; 
  unsigned int width,height;
  int hx,hy;
  Pixmap bitmap;
  ObjInfo *obj;

  strcpy(newname,name);
  if (!file_exists(newname))
    {
      sprintf(newname,"%s.b",name);
      if (!file_exists(newname))
	{
	  sprintf(newname,"%s.bit",name);
	  if (!file_exists(newname))
	    {
	      sprintf(message,"Bitmap for %s not found.",name);
	      place_obtor_message(message);
	      return(False);
	    }
	}  
    }
       
  if ((XReadBitmapFile(display,mainwin,newname,&width,&height,&bitmap,&hx,&hy))
      != BitmapSuccess)
    {
      sprintf(message,"Error reading bitmap file %s.",newname);
      place_obtor_message(message);
      return(False);
    }      

  if ((width != BITMAP_WIDTH)||(height != BITMAP_HEIGHT))
    {
      sprintf(message,"%dx%d are impropper bitmap dimensions. (%dx%d)",
	      width,height,BITMAP_WIDTH,BITMAP_HEIGHT);
      place_obtor_message(message);
      return(False);
    }

  if (fgcolor == 0)
    XFillRectangle(display,bitmap,xorGC,
		   0,0,BITMAP_WIDTH,BITMAP_HEIGHT);

  obj = make_blank_definition();
  TransferPixmapToData(bitmap,obj->bitmap);
  XFreePixmap(display,bitmap);
  copy_obj_to_group(obj,win,num_objects[win]);
  free(obj);
  return(True);
}

/*****************************************************************/

try_mask(name,win)
char *name;
int win;
{
  char newname[1024],intname[1024],message[1024],*p; 
  unsigned int width,height;
  int hx,hy,num;
  Pixmap bitmap;
  ObjInfo *obj;

  strcpy(intname,name);
  sprintf(newname,"%s.m",intname);
  if (!file_exists(newname))
    {
      sprintf(newname,"%s.msk",intname);
      if (!file_exists(newname))
	{
	  sprintf(newname,"%s.mask",intname);
	  if (!file_exists(newname))
	    {
	      sprintf(message,"Bitmap for %s loaded, mask not found.",
		      name);
	      place_obtor_message(message);
	      return(False);
	    }
	}  
    }

  if ((XReadBitmapFile(display,mainwin,newname,&width,&height,&bitmap,&hx,&hy))
      != BitmapSuccess)
    {
      sprintf(message,"Bitmap loaded, error reading mask file %s.",newname);
      place_obtor_message(message);
      return(False);
    }      

  if ((width != BITMAP_WIDTH)||(height != BITMAP_HEIGHT))
    {
      sprintf(message,
	      "Bitmap loaded, impropper mask dimensions: %dx%d. (%dx%d)",
	      width,height,BITMAP_WIDTH,BITMAP_HEIGHT);
      place_obtor_message(message);
      return(False);
    }
  
  if (fgcolor == 0)
    XFillRectangle(display,bitmap,xorGC,
		   0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
  num = num_objects[win]-1;
  obj = objdef[win][num];  /* new object already created */
  TransferPixmapToData(bitmap,obj->mask);
  XFreePixmap(display,bitmap);
  obj->masked = True;
  certain_object_pixmap(win,num);
  redraw_grab_square(win,num,False);
  return(True);
}

/*****************************************************************/
  
E 1
