h12612
s 00000/00000/00035
d D 1.3 92/08/07 01:00:39 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00001/00000/00034
d D 1.2 91/05/03 23:10:29 labc-3id 2 1
c 
e
s 00034/00000/00000
d D 1.1 91/03/24 18:44:28 labc-3id 1 0
c date and time created 91/03/24 18:44:28 by labc-3id
e
u
U
f e 0
t
T
I 1


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
I 2
  adj_clear(0,num_objects[0]-1);
E 2

  while (num_objects[0] > MAX_CUTOBJS)
    for (i = 0;i < CUT_DELJUMP;i++)
      remove_obj_from_group(0,0);

  ClickArrowWin(0,Button1,False);
}

/*****************************************************************/
E 1
