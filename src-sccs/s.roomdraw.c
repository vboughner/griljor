h14788
s 00000/00000/00166
d D 1.10 92/08/07 01:03:51 vbo 10 9
c source copied to a separate tree for work on new map and object format
e
s 00001/00009/00165
d D 1.9 91/11/25 23:03:45 labc-4lc 9 8
c fixed color problems drawing objects
e
s 00002/00003/00172
d D 1.8 91/10/16 20:06:29 labc-4lc 8 7
c changed made for DecStations
e
s 00002/00001/00173
d D 1.7 91/09/26 18:59:13 labc-4lc 7 6
c Fixed the color problem
e
s 00016/00013/00158
d D 1.6 91/09/03 21:39:26 labb-3li 6 5
c Trevor's misc changes - fixed missile heap
e
s 00007/00003/00164
d D 1.5 91/08/29 01:40:47 vanb 5 4
c fixed up more compatibility problems
e
s 00020/00011/00147
d D 1.4 91/05/26 22:43:52 labc-3id 4 3
c Worked on drawing improvements
e
s 00022/00014/00136
d D 1.3 91/05/18 15:44:35 labc-3id 3 2
c Fixed order-of-draw bug and added procedure for Expose events -T
e
s 00001/00001/00149
d D 1.2 91/05/17 04:56:04 labc-3id 2 1
c got rid of strange little p on line 116
e
s 00150/00000/00000
d D 1.1 91/05/17 02:02:32 labc-3id 1 0
c date and time created 91/05/17 02:02:32 by labc-3id
e
u
U
f e 0
t
T
I 1
#include "windowsX11.h"
#include "playerX11.h"
#include "def.h"
#include "extern.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gametime.h"
#include "gamelist.h"
#include "movement.h"
#include "images.h"
#include "level.h"
#include "queued.h"
#include "roomdraw.h"

I 7
int depth;
E 7
Pixmap offscreen;
int    state[ROOM_WIDTH][ROOM_HEIGHT];

/*****************************************************************/
  
RoomDrawInitialize()
{

  offscreen = NULL;

I 3
#ifdef USEPIXMAP
E 3
  offscreen = XCreatePixmap(display,DefaultRootWindow(display),
			    BITMAP_WIDTH*ROOM_WIDTH + ROOM_BORDER *2,
			    BITMAP_HEIGHT*ROOM_HEIGHT + ROOM_BORDER*2,
D 7
			    1);
E 7
I 7
			    depth);
E 7
I 3
#endif
E 3

  ClearRoomDraw();
}

/*****************************************************************/

free_draw_pixmap()
{
D 3
  XFreePixmap(display,offscreen);
E 3
I 3
  if (offscreen != NULL)
    XFreePixmap(display,offscreen);
E 3
  offscreen = NULL;
}

/*****************************************************************/

ClearRoomDraw()
{
  int i,j;

  for (i = 0;i < ROOM_WIDTH;i++)
    for (j = 0;j < ROOM_HEIGHT;j++)
D 6
      state[i][j] = NOTDRAWN;
E 6
I 6
      state[i][j] = -1;
E 6
}

I 3
D 5
MarkSquareAsBlank(int x,int y)
E 5
I 5
MarkSquareAsBlank(x, y)
int x, y;
E 5
{
D 6
  state[x][y] = TEMPDRAW;
E 6
I 6
  state[x][y] = 1;
E 6
}

E 3
/*****************************************************************/
  
D 5
DrawOnSquare(Pixmap pix,Pixmap mask,int x,int y,int mode)
E 5
I 5
DrawOnSquare(pix, mask, x, y, mode)
Pixmap pix, mask;
int x, y, mode;
E 5
{
  int wx,wy;

I 4
  QDraw_notify();

E 4
D 3
  wx = PIXELX(x);
  wy = PIXELY(y);

E 3
  switch(mode)
    {
I 4
    case MDRAW:
E 4
    case TEMPDRAW:
I 4

E 4
D 3
      if (mask)
E 3
I 3
D 6
      if (state[x][y] == NOTDRAWN)
E 6
I 6
      if (state[x][y] == -1)
E 6
E 3
	{
I 3
	  save_room_square(x,y);
	  copy_room_square(x,y);
	}

D 4
      wx = PIXELX(x);
      wy = PIXELY(y);
E 4
D 6

D 4
      if ((mask)&&(pix))
E 4
I 4
      if ( !((mode == MDRAW)&&(state[x][y] == MDRAW)))
E 6
I 6
      if ( ((mode == TEMPDRAW)||(state[x][y] == 0)) )
E 6
E 4
	{
E 3
D 4
	  XCopyArea(display,mask,roomwin,maskGC,0,0,
		    BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
	  XCopyArea(display,pix, roomwin,drawGC,0,0,
		    BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
E 4
I 4
	  wx = PIXELX(x);
	  wy = PIXELY(y);
D 6

E 6
I 6
	  
E 6
D 9
	  if ((mask)&&(pix))
	    {
D 8
	      XCopyArea(display,mask,roomwin,maskGC,0,0,
			BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
	      XCopyArea(display,pix, roomwin,drawGC,0,0,
E 8
I 8
	      DrawObjectMask(mask,roomwin,wx,wy);
	      XCopyArea(display,pix, roomwin,xorGC,0,0,
E 8
			BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
	    }
	  else if (pix)
	    XCopyArea(display,pix, roomwin,mainGC,0,0,
		      BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
E 9
I 9
	  DrawObject(roomwin, pix, mask, NULL, wx, wy);
E 9
D 6
	  
	  state[x][y] = mode;
E 6
E 4
	}
D 3
      else
	  XCopyArea(display,pix, roomwin,mainGC,0,0,
		    BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 

      if (state[x][y] == NOTDRAWN)
	{
	  save_room_square(x,y);
	  copy_room_square(x,y);
	}
E 3
I 3
D 4
      else if (pix)
	XCopyArea(display,pix, roomwin,mainGC,0,0,
		  BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
E 4
E 3

I 6
      if ((mode == MDRAW)||(state[x][y] == 0))
	state[x][y]++;

E 6
D 4
      state[x][y] = TEMPDRAW;
E 4
      break;
    case DRAWBASE:
D 6
      if (state[x][y] != DRAWBASE)
E 6
I 6
      if (state[x][y] != 0)
E 6
	{
D 6
	  if (state[x][y] == NOTDRAWN)
	    save_room_square(x,y);
E 6
I 6
	  if (state[x][y] == -1)
	    {
	      save_room_square(x,y);
	      state[x][y] = 1;
	    }
E 6

	  copy_room_square(x, y);
	  
D 6
	  state[x][y] = DRAWBASE;
E 6
I 6
	  state[x][y]--;
E 6
	}
      break;
    default:
      printf("Undefined draw:%d\n",mode);
      break;
    }
}

/*****************************************************************/

copy_room_square(x,y)
int x,y;
{
  int wx,wy;

  if (offscreen == NULL)
    draw_room_square(x,y,False);
  else
    {
      wx = PIXELX(x);
      wy = PIXELY(y);

      XCopyArea(display,offscreen,roomwin,mainGC,wx,wy,
D 2
p		BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
E 2
I 2
		BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
E 2
    }
  
}

/*****************************************************************/

save_room_square(x,y)
int x,y;
{
  Window temp;

  if (offscreen != NULL)
    {
      temp = roomwin;
      roomwin = offscreen; 

      draw_room_square(x,y,False);
      
      roomwin = temp;
    }
}

/*****************************************************************/

D 5
ChangedRoomSquare(int roomnum,int x,int y)
E 5
I 5
ChangedRoomSquare(roomnum, x, y)
int roomnum, x, y;
E 5
{
  if (roomnum != current_room)
    return;

D 6
  state[x][y] = NOTDRAWN;
E 6
I 6
  state[x][y] = -1;
E 6
  redraw_player_room_square(roomnum,x,y);
}

/*****************************************************************/
E 1
