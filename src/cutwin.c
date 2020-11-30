

#include "obtor.h"


/*****************************************************************/

place_cut_window(which)
int which;
{
  XMoveResizeWindow(display,objwin[which],CUTWIN_X,CUTWIN_Y,CUTWIN_W,CUTWIN_H);
  XMapRaised(display,objwin[which]);  /* just in case */
  grab_win_width[which] = 
    (CUTWIN_W + GRAB_SPACING)/(BITMAP_WIDTH+GRAB_SPACING);
  grab_win_height[which] = CUTWIN_R;
}

/*****************************************************************/

put_obj_in_cut(obj)
ObjInfo *obj;
{
  int i;

  copy_obj_to_group(obj,0,num_objects[0]);
  adj_clear(0,num_objects[0]-1);

  while (num_objects[0] > MAX_CUTOBJS)
    for (i = 0;i < CUT_DELJUMP;i++)
      remove_obj_from_group(0,0);

  ClickArrowWin(0,Button1,False);
}

/*****************************************************************/
