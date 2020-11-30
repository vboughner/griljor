h36596
s 00000/00000/00216
d D 1.3 92/08/07 01:00:48 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00025/00001/00191
d D 1.2 91/05/17 02:01:30 labc-3id 2 1
c Added code for showing object numbers using '#'
e
s 00192/00000/00000
d D 1.1 91/03/24 18:44:53 labc-3id 1 0
c date and time created 91/03/24 18:44:53 by labc-3id
e
u
U
f e 0
t
T
I 1


#include "obtor.h"


/* draw the contents of the object grabbing window */

redraw_a_grab_window(num)
     int num;
{
  int i, j, nextobj,lastobj;
  int x, y,x1,x2,y1,y2;
  
  if ((last_event.type == Expose)&&(last_event.xany.window == objwin[num]))
    {
      x = last_event.xexpose.x;
      y = last_event.xexpose.y;
      nextobj = obtor_obj_in_grab_win(x,y,num);
      calc_obj_ij(num,nextobj,&x1,&y1);

      x += last_event.xexpose.width - 1;
      y += last_event.xexpose.height - 1;
      lastobj = obtor_obj_in_grab_win(x,y,num);
      calc_obj_ij(num,lastobj,&x2,&y2);
    }
  else  /* Redraw the while thing... gasp! */
    {
D 2
      printf("Argh... redrawing the whole thing!\n");
E 2
      x1 = 0;
      y1 = 0;
      x2 = grab_win_width[num]-1;
      y2 = grab_win_height[num]-1;
      nextobj = 0;
    }

  for (j = y1; j <= y2 ; j++)
    {
      for (i = x1; i <= x2; i++) 
	redraw_grab_square(num, nextobj++,TRUE);

      nextobj += grab_win_width[num] + x1 - x2 - 1;
    }
}


/* redraw just one object in one grab window */

redraw_grab_square(groupnum, objnum,clear)
     int groupnum, objnum,clear;
{
  int x, y;
  
  calc_obj_xy(groupnum,objnum,&x,&y);
  
  if (objnum < num_objects[groupnum])
    {
      obtor_draw_object(objwin[groupnum], x, y,objnum, OBJ_DEF(groupnum),
			item_pixmap[groupnum],
			item_mask[groupnum], clear);
    }
  else
    XCopyArea(display,nothing_pix,objwin[groupnum],mainGC,0,0,
	      BITMAP_WIDTH,BITMAP_WIDTH,
	      x,y);

I 2
  if (show_numbers)
    redraw_show_numbers(groupnum,objnum);

E 2
  if (mass_rec != NULL)
    redraw_mass_info(groupnum,objnum);

}

I 2
/*****************************************************************/

redraw_show_numbers(win,i)
int win,i;
{
  int x,y,w,h;
  char string[100],*vptr;
  long value;

  if (!IS_VALID_OBJ(win,i))
    return;

  calc_obj_xy(win,i,&x,&y);
  x += MASSPAD;
  y += BITMAP_HEIGHT - MASSPAD - text_height(tinyfont) - 1;
  w = char_width(tinyfont) * 3 + 2;
  h = text_height(tinyfont)+2;
  XClearArea(display,objwin[win],x,y,w,h,False);
  
  sprintf(string,"%3d",i);
  text_write(objwin[win],tinyfont,x+1,y+1,0,0,string);
}
E 2

/*****************************************************************/

obtor_clear_space(win,x,y)
{
  XClearArea(display,win,x,y,BITMAP_WIDTH,BITMAP_HEIGHT,False);
}

/* place an object pixmap on screen, if there is a mask then place it
   on first.  You must give the window and pixel x and y coords to
   draw at.  You must specify the object definition, pixmap, and mask,
   and whether we can assume the spot underneath has nothing at all
   drawn on it already */

obtor_draw_object(win, x, y, index, def, pix, mask, is_clear_under)
     Window win;
     int x, y,index;
     ObjInfo *def[];
     Pixmap pix[], mask[];
     int is_clear_under;
{
  int func;
  
  /* if there is a mask but screen is clear underneath, skip the draw */
  
  if ((def == NULL) || 
      (pix == NULL) || 
      (def[index] == NULL) ||
      ((mask == NULL) && (def[index]->masked)))
    return(False);
  
  if (!is_clear_under)
    obtor_clear_space(win,x,y);

  if (!((def[index])->masked))
    {
      if (pix[index]) 
	XCopyArea(display, pix[index], win, mainGC, 0, 0, 
		  BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
    }
  else 
    {
      if (mask[index])
	XCopyArea(display, mask[index], win, maskGC, 0, 0,
		  BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
      if (pix[index])
	XCopyArea(display, pix[index], win, drawGC, 0, 0,
		  BITMAP_WIDTH, BITMAP_HEIGHT, x, y);
    }
}

/*****************************************************************/

set_window_background(groupnum,index)
int index,groupnum;
{
  if (index < 0)
    XSetWindowBackground(display,objwin[groupnum],bgcolor);
  else
    {
      XFillRectangle(display,try_pix[groupnum],inverseGC,0,0,
		     BITMAP_WIDTH+GRAB_SPACING,BITMAP_HEIGHT+GRAB_SPACING);
      
      obtor_draw_object(try_pix[groupnum], 0, 0, index, OBJ_DEF(groupnum),
			item_pixmap[groupnum],
			item_mask[groupnum], TRUE);
      
      XSetWindowBackgroundPixmap(display,objwin[groupnum],try_pix[groupnum]);
    }

  XClearArea(display,objwin[groupnum],0,0,0,0,True);
}

/*****************************************************************/

calc_obj_xy(groupnum,objnum,x,y)
int groupnum,objnum;
int *x,*y;
{
  int i,j;

  calc_obj_ij(groupnum,objnum,&i,&j);
  if ((window_info_up)&&(groupnum > 0))  /* not for win #0 */
    j++;
  *x = (i * (BITMAP_WIDTH + GRAB_SPACING));
  *y = (j * (BITMAP_HEIGHT + GRAB_SPACING));
}

/*****************************************************************/


calc_obj_ij(groupnum,objnum,i,j)
int groupnum,objnum;
int *i,*j;
{
  objnum -= baseobj[groupnum];
  *i = objnum % grab_win_width[groupnum];
  *j = objnum / grab_win_width[groupnum];
}

/*****************************************************************/

box_grab_square(groupnum,index,gc)
int groupnum,index;
GC gc;
{
  int x,y;

  calc_obj_xy(groupnum,index,&x,&y);
  XDrawLine(display,objwin[groupnum],gc,
	    x-1,y-1,x-1,y+BITMAP_HEIGHT);
  XDrawLine(display,objwin[groupnum],gc,
	    x,y-1,x+BITMAP_WIDTH,y-1);
  XDrawLine(display,objwin[groupnum],gc,
	    x+BITMAP_WIDTH,y,x+BITMAP_WIDTH,y+BITMAP_HEIGHT);
  XDrawLine(display,objwin[groupnum],gc,
	    x,y+BITMAP_HEIGHT,x+BITMAP_WIDTH-1,y+BITMAP_HEIGHT);
}

/*****************************************************************/


E 1
