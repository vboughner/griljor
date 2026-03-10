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

int depth;
Pixmap offscreen;
int    state[ROOM_WIDTH][ROOM_HEIGHT];

/*****************************************************************/
  
RoomDrawInitialize()
{

  offscreen = NULL;

#ifdef USEPIXMAP
  offscreen = XCreatePixmap(display,DefaultRootWindow(display),
			    BITMAP_WIDTH*ROOM_WIDTH + ROOM_BORDER *2,
			    BITMAP_HEIGHT*ROOM_HEIGHT + ROOM_BORDER*2,
			    depth);
#endif

  ClearRoomDraw();
}

/*****************************************************************/

free_draw_pixmap()
{
  if (offscreen != NULL)
    XFreePixmap(display,offscreen);
  offscreen = NULL;
}

/*****************************************************************/

ClearRoomDraw()
{
  int i,j;

  for (i = 0;i < ROOM_WIDTH;i++)
    for (j = 0;j < ROOM_HEIGHT;j++)
      state[i][j] = -1;
}

MarkSquareAsBlank(x, y)
int x, y;
{
  state[x][y] = 1;
}

/*****************************************************************/
  
DrawOnSquare(pix, mask, x, y, mode)
Pixmap pix, mask;
int x, y, mode;
{
  int wx,wy;

  QDraw_notify();

  switch(mode)
    {
    case MDRAW:
    case TEMPDRAW:

      if (state[x][y] == -1)
	{
	  save_room_square(x,y);
	  copy_room_square(x,y);
	}

      if ( ((mode == TEMPDRAW)||(state[x][y] == 0)) )
	{
	  wx = PIXELX(x);
	  wy = PIXELY(y);
	  
	  DrawObject(roomwin, pix, mask, NULL, wx, wy);
	}

      if ((mode == MDRAW)||(state[x][y] == 0))
	state[x][y]++;

      break;
    case DRAWBASE:
      if (state[x][y] != 0)
	{
	  if (state[x][y] == -1)
	    {
	      save_room_square(x,y);
	      state[x][y] = 1;
	    }

	  copy_room_square(x, y);
	  
	  state[x][y]--;
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
		BITMAP_WIDTH,BITMAP_HEIGHT,wx,wy); 
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

ChangedRoomSquare(roomnum, x, y)
int roomnum, x, y;
{
  if (roomnum != current_room)
    return;

  state[x][y] = -1;
  redraw_player_room_square(roomnum,x,y);
}

/*****************************************************************/
