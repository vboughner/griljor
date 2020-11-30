#include "stdio.h"
#include "config.h"
#include "map.h"
#include "objects.h"
#include "math.h"

extern long display;
extern unsigned char click_buffer[3];  /* it is not in any include file */

/*****************************************************************/

#define getsq most_prominant_on_square

#define MVX(dir)   ( ((dir)%4 == 1? 1 : ((dir)%4 == 3?-1:0)) )
#define MVY(dir)   ( ((dir)%4 == 2? 1 : ((dir)%4 == 0?-1:0)) )

#define CHKAHD(x,y,dir,rm,wall) \
  ((((x)+MVX(dir)*2  < 0 ? 0 : \
     ((x)+MVX(dir)*2 > (ROOM_WIDTH-1) ? 0 : \
      ((y)+MVY(dir)*2 < 0 ? 0 : \
       ((y)+MVY(dir)*2 > (ROOM_HEIGHT-1) ? 0 : \
	(getsq(rm,x+MVX(dir)*2,y+MVY(dir)*2) == 0) ))))))

/*****************************************************************/

make_random_room(x, y)
int x, y;
{
  int result;
  char *line[10];

  /* find out what they want to change the mode to */
  line[0] = "";
  line[1] = " What type of randomness do you wish to create?";
  line[2] = " 1 - Random outdoors";
  line[3] = " 2 - Random maze";
  line[4] = " 3 - Random anti-maze";
  line[5] = " 4 - Random room";
  line[6] = " 5 - None";
  line[7] = "";

  result = menu_query(line, 8, 5, 1);


  /* assign the appropriate mode */
  switch (result) 
    {
    case 1:
      make_random_outside(x,y);
      break;
    case 2:
      make_random_maze(x,y);
      break;
    case 3:
      make_random_antimaze(x,y);
      break;
    case 4:
      make_random_inside(x,y);
      break;
    }
}

/*****************************************************************/

make_random_inside(x, y)
int x, y;
{
  int wall,end,i,j;

  wall = click_buffer[0];

  if (getsq(current_room,1,0) != wall)
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(i,0,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(i,0,wall,current_room);
    }
  if (getsq(current_room,0,1) != wall)
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(0,i,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(0,i,wall,current_room);
    }
  if (getsq(current_room,ROOM_WIDTH-1,1) != wall)
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(ROOM_WIDTH-1,i,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(ROOM_WIDTH-1,i,wall,current_room);
    }
  if (getsq(current_room,1,ROOM_HEIGHT-1) != wall)
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(i,ROOM_HEIGHT-1,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(i,ROOM_HEIGHT-1,wall,current_room);
    }
  for (i = 1;i < ROOM_WIDTH-1;i++)
    for (j = 1;j < ROOM_HEIGHT-1;j++)
      clear_square(i,j);
}

/*****************************************************************/


make_random_maze(x, y)
int x, y;
{
  int dir,dir1;
  int tries,wall,rm;
  int i;

  wall = click_buffer[0];

  if (wall == 0)
    return;

  rm = current_room;

  x = ((lrand48()%(ROOM_WIDTH-2))/2)*2;
  y = ((lrand48()%(ROOM_WIDTH-2))/2)*2;
  square(x,y,wall,rm);
  
  tries = 0;
  while(tries < 500)
    {
      if (getsq(rm,x,y) != wall)
	{
	  x = ((lrand48()%(ROOM_WIDTH))/2)*2;
	  y = ((lrand48()%(ROOM_WIDTH))/2)*2;
	}
      else
	{
	  dir1 = dir = lrand48()%4;
	  while ((!CHKAHD(x,y,dir,rm,wall))&&(dir != dir1+4))
	    dir++;

	  if (dir != dir1+4)
	    {
	      tries = 0;
	      x += MVX(dir);
	      y += MVY(dir);
	      square(x,y,wall,rm);
	      x += MVX(dir);
	      y += MVY(dir);
	      square(x,y,wall,rm);
	    }
	  else
	    {
	      x = ((lrand48()%(ROOM_WIDTH))/2)*2;
	      y = ((lrand48()%(ROOM_WIDTH))/2)*2;
	    }
	}

      tries++;
    }
}

/*****************************************************************/

make_random_antimaze(x, y)
int x, y;
{
  int dir,dir1;
  int tries,wall,rm;
  int i;


  wall = click_buffer[0];

  if (wall == 0)
    return;

  rm = current_room;

  x = ((lrand48()%(ROOM_WIDTH-2))/2)*2;
  y = ((lrand48()%(ROOM_WIDTH-2))/2)*2;
  square(x,y,wall,rm);
  
  tries = 0;
  while(tries < 500)
    {
      dir1 = dir = lrand48()%4;
      while ((!CHKAHD(x,y,dir,rm,wall))&&(dir != dir1+4))
	dir++;
      
      if (dir != dir1+4)
	{
	  tries = 0;
	  square(x,y,wall,rm);
	  x += MVX(dir);
	  y += MVY(dir);
	  square(x,y,wall,rm);
	  x += MVX(dir);
	  y += MVY(dir);
	  square(x,y,wall,rm);
	}
      else
	{
	  do
	    {
	      x = ((lrand48()%(ROOM_WIDTH))/2)*2;
	      y = ((lrand48()%(ROOM_WIDTH))/2)*2;
	      tries++;
	    }
	  while( (getsq(rm,x,y) == wall) && (tries < 500) );
	}

      tries++;
    }
}

/*****************************************************************/

