h44047
s 00000/00000/00301
d D 1.2 92/08/07 01:04:38 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00301/00000/00000
d D 1.1 91/03/24 18:45:41 labc-3id 1 0
c date and time created 91/03/24 18:45:41 by labc-3id
e
u
U
f e 0
t
T
I 1


#include "obtor.h"


/*****************************************************************/

FindNewWindow()
{
  int i,t,min;

  t = CurrentAccessTime;
  min = -1;

  do
    {
      for (i = NORM_WINDOW;i < MAX_BUFFERS;i++)
	if (!data_loaded[i])
	  return(i);
	else
	  if (access_time[i] < t)
	    {
	      t = access_time[i];
	      min = i;
	    }

      if ((!query_use_win(min)) || (!unload_window(min)))
	{
	  t = (access_time[i] = CurrentAccessTime++);
	  min = -1;
	}
    }
  while (min == -1);
  return(min);
}


/*****************************************************************/

MakeNewSpot()
{
  int min,i,j;

  if (find_home(-1,&i,&j))
    return(TRUE);

  while (TRUE)
    {
      for (min = NORM_WINDOW;min < MAX_BUFFERS;)
	/* "min++" is handled by case 'n'
	   below or "else" below */
	if (!data_loaded[min])
	  min++;
	else
	  switch (query_split_win(min))
	    {
	    case 'y':
	      return(unload_window(min));
	    case 'h':
	      if (!splitwindow(min,True)) break;
	      return(TRUE);
	    case 'v':
	      if (!splitwindow(min,False)) break;
	      return(TRUE);
	    case 'c':
	      return(FALSE);
	    case 'n':
	      min++;
	      break;
	    default:
	      break;
	    }
    }
}

/*****************************************************************/

SetNewWindowRoot(int which)
{
  int i,j;

  while(True)
    {
      if (find_home(-1,&i,&j))
	{
	  window_space[i][j] = which;
	  return(TRUE);
	}

      if (!MakeNewSpot())  /* Ask the user some questions and try again */
	return(FALSE);
    }
}

/*****************************************************************/

force_move_window(which,dest)
int which,dest;
{
  int wx1,wy1,wx2,wy2;
  int target;

  wx2 = dest%WIN_X_SPACE;
  wy2 = dest/WIN_X_SPACE;

  if (wy2 >= WIN_Y_SPACE)
    return(False);

  target = window_space[wx2][wy2];

  if (!find_home(which,&wx1,&wy1))
    return(False);

  clear_window_space(target);
  clear_window_space(which);

  window_space[wx1][wy1] = target;
  window_space[wx2][wy2] = which;
  ExpandWindow(which);
  ExpandWindow(target);
  return(True);
}

/*****************************************************************/

WinCanHold(which)
int which;
{
  return(grab_win_width[which]*
	 (grab_win_height[which] - ( window_info_up ? 1 : 0)));
}

/*****************************************************************/

ExpandWindow(which)
int which;
{
  int block,i,j;
  int x,y,xs,ys;
  int all_expand;

  x = -1;
  
  if (which < 0)
    return(FALSE);
  /* Find the window in the window space */
  if (!find_home(which,&x,&y))
    return(FALSE);  /* not find the windoww */


  all_expand = ((WinCanHold(which) >= MAX_OBJECTS)&&data_loaded[which]);
  
  clear_window_space(which);
  ys = 1;
  /* find the largest possible verticle expansion */
  for (i = y;i < WIN_Y_SPACE;i++)
    if (window_space[x][i] == -1)
      ys = i-y+1;

  /* find the largest possible horiziontal expansion */
  xs = 1;
  block = False;
  for (j = x;(j < WIN_X_SPACE)&&(!block);j++)
    /* if it is still less than MAX_OBJECTS spaces */
    if ((all_expand) || ((ys*xs* SPACE_SIZE * SPACE_SIZE) < MAX_OBJECTS))
      {
	for (i = 0;i < ys;i++)
	  if (window_space[j][y + i] != -1)
	    block = True;
	
	if (!block)
	  xs = j-x+1;
      }

  /* now resize the window */
  set_window_position(which,x,y,xs,ys);
  return(TRUE);
}

/*****************************************************************/

clear_window_space(int which)
{
  int i,j;

  for (i = 0;i < WIN_X_SPACE;i++)
    for (j = 0;j < WIN_Y_SPACE;j++)
      if (window_space[i][j] == which)
	window_space[i][j] = -1;
}

/*****************************************************************/

find_home(which,x,y)
int which,*x,*y;
{
  int i,j;

  for (i = 0;i < WIN_X_SPACE;i++)
    for (j = 0;j < WIN_Y_SPACE;j++)
      if (window_space[i][j] == which)
	{
	  *x = i;
	  *y = j;
	  return(TRUE);
	}

  return(FALSE);
}

/*****************************************************************/

set_window_position(which,xpos,ypos,xsiz,ysiz)
int xpos,ypos,xsiz,ysiz;
int which;
{
  int pxs,pys;
  int pxp,pyp;
  int i,j;

  clear_window_space(which);

  for (i = xpos;i < xpos+xsiz;i++)
    for (j = ypos;j < ypos+ysiz;j++)
      window_space[i][j] = which;
  
  xsiz = xsiz * SPACE_SIZE;
  ysiz = ysiz * SPACE_SIZE;
  xpos = xpos * SPACE_SIZE;
  ypos = ypos * SPACE_SIZE;

  grab_win_width[which] = xsiz;
  grab_win_height[which] = ysiz;
  pxs = OBJWIN_W(which);
  pys = OBJWIN_H(which);
  pxp = xpos * (BITMAP_WIDTH+GRAB_SPACING) - WINDOW_BORDERS;
  pyp = ypos * (BITMAP_HEIGHT+GRAB_SPACING) - WINDOW_BORDERS;

  XMoveResizeWindow(display,objwin[which],pxp,pyp,pxs,pys);
  AdjArrowWindows(which);
}

/*****************************************************************/

resize_data_window(which,xsiz,ysiz)
int which,xsiz,ysiz;
{
 int pxs,pys,xpos,ypos;
  int i,j;

 if (find_home(which,&xpos,&ypos))
   {
     set_window_position(which,xpos,ypos,xsiz,ysiz);
     return(True);
   }

 printf("Could not find old window to resize!\n");
 return(False);
}

/*****************************************************************/

splitwindow(win,horiz)
int win,horiz;
{
  int xsiz,ysiz;

  xsiz = grab_win_width[win]/SPACE_SIZE;
  ysiz = grab_win_height[win]/SPACE_SIZE;

  if (((horiz)&&(xsiz == 1)) || ((!horiz)&&(ysiz == 1)))
    return(FALSE);
  
  if (horiz)
    xsiz = (xsiz + 1)/2;
  else
    ysiz = (ysiz + 1)/2;

  resize_data_window(win,xsiz,ysiz);
}

/*****************************************************************/

query_use_win(num)
     int num;
{
  return (placed_want_query(objwin[num], 0, 0, OBJWIN_W(num), OBJWIN_H(num),
			      "Use this buffer (y/n)?"));
}

/*****************************************************************/

query_split_win(num)
     int num;
{
  return (placed_letter_query(objwin[num], 0, 0, OBJWIN_W(num), OBJWIN_H(num),
			      "Use/split this buffer?\n(y/h/v/n/c)","ynhvc"));
}

/*****************************************************************/

E 1
