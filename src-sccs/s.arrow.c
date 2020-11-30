h60705
s 00000/00000/00133
d D 1.2 92/08/07 01:00:15 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00133/00000/00000
d D 1.1 91/03/24 18:44:21 labc-3id 1 0
c date and time created 91/03/24 18:44:21 by labc-3id
e
u
U
f e 0
t
T
I 1
#include "obtor.h"

/*****************************************************************/

TempUnmapArrowWindows(win)
int win;
{
  if (all_shown[win])
    return;
  XUnmapWindow(display,downwin[win]);
  XUnmapWindow(display,upwin[win]);
  if (arrowbottom[win] > 0)
    redraw_grab_square(win,arrowbottom[win]-1,False);
  redraw_grab_square(win,arrowtop[win]+1,False);
}

/*****************************************************************/
  
AdjArrowWindows(win)
int win;
{
  int x,y;

  x = -WINDOW_BORDERS;
  y = -WINDOW_BORDERS + (window_info_up ? BITMAP_HEIGHT + GRAB_SPACING : 0 );
  XMoveWindow(display,upwin[win],x,y);

  x = OBJWIN_W(win)-BITMAP_WIDTH-GRAB_SPACING;
  y = OBJWIN_H(win)-BITMAP_HEIGHT-GRAB_SPACING;
  XMoveWindow(display,downwin[win],x,y);

  while ((baseobj[win] > 0)&&
	 (num_objects[win] - baseobj[win] < WinCanHold(win)/2))
    ClickArrowWin(win,Button1,True);  /* move the window up some if there
					 is room */

  all_shown[win] = True;
  arrowbottom[win] = baseobj[win];
  arrowtop[win] = MIN(arrowbottom[win]+WinCanHold(win)-1,num_objects[win]-1);

  if (baseobj[win] > 0)
    {
      all_shown[win] = False;
      XMapRaised(display,upwin[win]);
      arrowbottom[win]++;
    }
  else
    XUnmapWindow(display,upwin[win]);

  if ((num_objects[win] - baseobj[win]) > (WinCanHold(win)-1))
    {
      all_shown[win] = False;
      XMapRaised(display,downwin[win]);
      arrowtop[win]--;
    }
  else
    XUnmapWindow(display,downwin[win]);

  redraw_info_window(win);
}

/*****************************************************************/

arrow_jump(win,button,grain)
int win,button,*grain;
{
  int jump;
  
  jump = 0;
  *grain = MIN(grab_win_width[win],WinCanHold(win)/2);
  
  if (button == Button1)
    jump = ((WinCanHold(win)/2)/(*grain))*(*grain);
  else if (button == Button2)
    jump = ((WinCanHold(win))/(*grain))*(*grain) - (*grain);
  else if (button == Button3)
    jump = num_objects[win];

  return(jump);
}

/*****************************************************************/

arrow_move(win,button,go_up,x,y)
int win,button,go_up,*x,*y;
{
  int jump,grain;

  jump = arrow_jump(win,button,&grain);
  if (go_up) jump = -jump;

  if ((baseobj[win] + jump) < 0)
    jump = -baseobj[win];

  if ((baseobj[win] + jump) > (num_objects[win] - WinCanHold(win)/2))
    {
      jump = (num_objects[win]-WinCanHold(win)/2)-baseobj[win];
      jump = (jump/grain)*grain;  /* align it to the grain anyway */
    }
  
  if (jump < 0)
    {
      *x = -(-jump)%grain;
      *y = -(-jump)/grain;
    }
  else
    {
      *x = jump%grain;
      *y = jump/grain;
    }

  *x = *x * (BITMAP_WIDTH + GRAB_SPACING);
  *y = *y * (BITMAP_HEIGHT + GRAB_SPACING);
  baseobj[win] += jump;
  AdjArrowWindows(win);
}
  
/*****************************************************************/

ClickArrowWin(which,button,go_up)
int which,button;
int go_up;
{
  int clear,x,y;

  arrow_move(which,button,go_up,&x,&y);

  /* Graphics expose events will take care of the necessary redrawing */
  XCopyArea(display,objwin[which],objwin[which],mainGC,
	    x,y,OBJWIN_W(which)*2,OBJWIN_H(which)*2,0,0);
}

/*****************************************************************/
E 1
