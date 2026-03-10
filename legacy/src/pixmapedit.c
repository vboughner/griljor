

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
