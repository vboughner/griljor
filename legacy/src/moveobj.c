

#include "obtor.h"

  
/* ======================================================================= */

int obtor_obj_in_grab_win(x, y,num)
int x, y;
int num;
{
  int line, col, result;
      
  line = (y + GRAB_SPACING/2) / (BITMAP_HEIGHT + GRAB_SPACING);
  col = (x+ GRAB_SPACING/2) / (BITMAP_WIDTH + GRAB_SPACING);

  if ((window_info_up)&&(line > 0)&&(num > 0)) /* not for win #0 */
    line = line-1;
 
 result = (line * grab_win_width[num]) + col;

  result = result + baseobj[num];
  return result;
}


/* copy the definition from a click buffer out one of the object groups.  If
   there aren't that many objects in the group (0 or 1) then make a bunch of
   blank ones to fill in the space */

copy_obj_to_group(obj, groupnum, objnum)
int groupnum, objnum;
ObjInfo *obj;
{
  ObjInfo *copy;
  
  /* if there aren't enough objects in this group yet, then fill it up */
  if (objnum >= num_objects[groupnum])
    resize_group(groupnum, objnum); 
  
  /* allocate a copy of the click buffer's definition */
  copy = copy_obj_definition(obj); 
  
  /* replace the old definition with this new copy */
  replace_obj_definition(&(OBJ_DEF(groupnum)[objnum]), copy); 
  FileChanged(groupnum,TRUE);
  
  /* update the pixmaps for the object */
  certain_object_pixmap(groupnum,objnum);
  redraw_grab_square(groupnum, objnum,False);
}

/*****************************************************************/

insert_obj_in_group(obj,groupnum,objnum)
int groupnum,objnum;
ObjInfo *obj;
{
  int count,lastobj,x,y,x2,y2;

  lastobj = num_objects[groupnum];
  if (objnum >= lastobj)
    {
      copy_obj_to_group(obj,groupnum,objnum);
      free(obj);
      return(0);  /* no need to move anything around */
    }

  copy_obj_to_group(obj,groupnum,lastobj); /* create a new object */

  if (item_pixmap[groupnum][lastobj])
    XFreePixmap(display,item_pixmap[groupnum][lastobj]);
  if (item_mask[groupnum][lastobj])
    XFreePixmap(display,item_mask[groupnum][lastobj]);
  free(objdef[groupnum][lastobj]);

  for (count = lastobj-1;count >= objnum;count--)
   {
     objdef[groupnum][count+1] = objdef[groupnum][count];
     item_pixmap[groupnum][count+1] = item_pixmap[groupnum][count];
     item_mask[groupnum][count+1] = item_mask[groupnum][count];
    }
  item_pixmap[groupnum][objnum] = 0; /* erase them so they don't get freed */
  item_mask[groupnum][objnum] = 0;
  objdef[groupnum][objnum] = copy_obj_definition(obj);
  /* copy it so that copy_obj_to_group will work */

  calc_obj_xy(groupnum,objnum,&x,&y);
  calc_obj_xy(groupnum,(lastobj+1),&x2,&y2);
  XCopyArea(display,objwin[groupnum],objwin[groupnum],mainGC,
	    x,y,
	    OBJWIN_W(groupnum)-x,y2-y+BITMAP_WIDTH,
	    x+BITMAP_WIDTH+GRAB_SPACING,y);
  XCopyArea(display,objwin[groupnum],objwin[groupnum],mainGC,
	    0,y+BITMAP_HEIGHT+GRAB_SPACING,
	    x,y2-y+BITMAP_WIDTH,
	    BITMAP_WIDTH+GRAB_SPACING,y+BITMAP_HEIGHT+GRAB_SPACING);
  
  copy_obj_to_group(obj,groupnum,objnum); 
  free(obj);
  redraw_grab_square(groupnum,lastobj+1); /* redraw a few squares that */
  redraw_grab_square(groupnum,lastobj);    /* need it */

  XClearArea(display,objwin[groupnum],0,y+BITMAP_WIDTH+GRAB_SPACING,
	     BITMAP_WIDTH,y2-y+BITMAP_WIDTH,True);

  adj_depend(groupnum,objnum,lastobj,1);
}

/*****************************************************************/

delete_obj_from_group(groupnum,objnum)
int groupnum,objnum;
{
  if (objnum >= num_objects[groupnum])
    return(0);
  
  put_obj_in_cut(objdef[groupnum][objnum]);
  remove_obj_from_group(groupnum,objnum);
}

/*****************************************************************/
  
remove_obj_from_group(groupnum,objnum)
int groupnum,objnum;
{
  int count,lastobj,x,y,x2,y2;
  
  if (objnum >= num_objects[groupnum])
    return(0);
  
  lastobj = num_objects[groupnum] - 1;
  free(objdef[groupnum][objnum]);
  if (item_pixmap[groupnum][objnum])
    XFreePixmap(display,item_pixmap[groupnum][objnum]);
  if (item_mask[groupnum][objnum])
    XFreePixmap(display,item_mask[groupnum][objnum]);

  for (count = objnum;count < lastobj;count++)
    {
      item_pixmap[groupnum][count] = item_pixmap[groupnum][count+1];
      item_mask[groupnum][count] = item_mask[groupnum][count+1];
      objdef[groupnum][count] = objdef[groupnum][count+1];
    }
  
  item_pixmap[groupnum][lastobj] = 0;
  item_mask[groupnum][lastobj] = 0;
  objdef[groupnum][lastobj] = NULL;
  num_objects[groupnum]--;
  AdjArrowWindows(groupnum);

  calc_obj_xy(groupnum,objnum,&x,&y);
  calc_obj_xy(groupnum,lastobj,&x2,&y2);
  XCopyArea(display,objwin[groupnum],objwin[groupnum],mainGC,
	    x+BITMAP_WIDTH+GRAB_SPACING,y,
	    OBJWIN_W(groupnum)-x-BITMAP_WIDTH-GRAB_SPACING,BITMAP_HEIGHT,
	    x,y);
  XCopyArea(display,objwin[groupnum],objwin[groupnum],mainGC,
	    BITMAP_WIDTH+GRAB_SPACING,y + BITMAP_HEIGHT,
	    OBJWIN_W(groupnum)-BITMAP_WIDTH-GRAB_SPACING,y2-y+BITMAP_HEIGHT,
	    0,y+BITMAP_HEIGHT);
  XClearArea(display,objwin[groupnum],
	     OBJWIN_W(groupnum)-BITMAP_WIDTH-GRAB_SPACING,y,
	     0,y2-y+BITMAP_HEIGHT+GRAB_SPACING,True);
  
  FileChanged(groupnum,True);
  adj_depend(groupnum,objnum,lastobj-1,-1);
}

/*****************************************************************/
