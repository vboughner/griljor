h28238
s 00000/00000/00081
d D 1.3 92/08/07 01:03:16 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00000/00078/00081
d D 1.2 91/09/15 23:33:48 labc-4lc 2 1
c made mapinfo dialog editing window work
e
s 00159/00000/00000
d D 1.1 91/03/24 18:45:30 labc-3id 1 0
c date and time created 91/03/24 18:45:30 by labc-3id
e
u
U
f e 0
t
T
I 1


#include "obtor.h"


/*****************************************************************/

edit_obtor_bitmap(win)
Window win;
{
  int group,num;

  if ((group = FindObjToEdit(&num)) > -1)
    if (IS_VALID_OBJ(group,num))
      edit_bitmap_item(group,num);
}

/*****************************************************************/

edit_bitmap_item(group,num)
int group,num;
{
  Pixmap *pixmap;

  XDefineCursor(display,obtorwin,WatchCursor);
  pixmap = &(item_pixmap[group][num]);
  if (edit_pixmap(pixmap))
    TransferPixmapToData(*pixmap,&(objdef[group][num]->bitmap));
  redraw_grab_square(group,num,FALSE);
  XDefineCursor(display,obtorwin,ArrowCursor);
}

/*****************************************************************/

edit_obtor_mask(win)
Window win;
{
  int group,num;

  if ((group = FindObjToEdit(&num)) > -1)
    if (IS_VALID_OBJ(group,num))
      edit_mask_item(group,num);
}

/*****************************************************************/

edit_mask_item(group,num)
int group,num;
{
  Pixmap *pixmap;
  
  XDefineCursor(display,obtorwin,WatchCursor);
  if (objdef[group][num]->masked)
    {
      pixmap = &(item_mask[group][num]);
      if (edit_pixmap(pixmap))
	TransferPixmapToData(*pixmap,objdef[group][num]->mask);
      redraw_grab_square(group,num,FALSE);
    }
  else
    {
      if (placed_want_query(objwin[group], 0, 0, 
			    OBJWIN_W(group), OBJWIN_H(group),
			    "Create a new mask?"))
	{
	  pixmap = &(item_mask[group][num]);
	  if (*pixmap)
	    XFreePixmap(display,*pixmap);
	  *pixmap = 0;
	  if (edit_pixmap(pixmap))
	    {
	      objdef[group][num]->masked = True;
	      TransferPixmapToData(*pixmap,objdef[group][num]->mask);
	    }
	  redraw_grab_square(group,num,FALSE);
	}
    }
  XDefineCursor(display,obtorwin,ArrowCursor);
}

/*****************************************************************/
D 2

TransferPixmapToData(pixmap,array)
Pixmap pixmap;
char array[];
{
  char *data;
  char temp[L_tmpnam];
  int width,height,x,y,i;

  tmpnam(temp);
  
  if (fgcolor == 0)
    XFillRectangle(display,pixmap,xorGC,
		   0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
  XWriteBitmapFile(display,temp,pixmap,BITMAP_WIDTH,BITMAP_HEIGHT,-1,-1);
  if (fgcolor == 0)
    XFillRectangle(display,pixmap,xorGC,
		   0,0,BITMAP_WIDTH,BITMAP_HEIGHT);

  XXReadBitmapFile(temp, &width, &height, &data, &x, &y);

  if (data != NULL)
    {
      for (i = 0;i < BITMAP_ARRAY_SIZE;i++)
	array[i] = data[i];
      free(data);
    }
  unlink(temp);
}

/*****************************************************************/

edit_pixmap(pixmap)
Pixmap *pixmap;
{
  unsigned int width,height;
  int x,y;
  char temp[L_tmpnam],out[L_tmpnam+10];
  Pixmap new;

  tmpnam(temp);

  if (*pixmap)
    {
      if (fgcolor == 0)
	XFillRectangle(display,*pixmap,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
      XWriteBitmapFile(display,temp,*pixmap,BITMAP_WIDTH,BITMAP_HEIGHT,-1,-1);
      if (fgcolor == 0)
	XFillRectangle(display,*pixmap,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);
    }
  
  sprintf(out,"bitmap %s %dx%d",temp,BITMAP_WIDTH,BITMAP_HEIGHT);
  system(out);
  
  XReadBitmapFile(display,RootWindow(display,screen),
		  temp,&width,&height,&new,&x,&y);
  unlink(temp);

  if ((width != BITMAP_WIDTH)||(height != BITMAP_HEIGHT))
    {
      printf("Bitmap wrong size!\n");
      return(False);
    }

  if (fgcolor == 0)
    XFillRectangle(display,new,xorGC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT);


  if (*pixmap)
    XFreePixmap(display,*pixmap);
  *pixmap = new;

  return(True);
}


/*****************************************************************/

E 2
E 1
