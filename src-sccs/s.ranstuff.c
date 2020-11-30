h37986
s 00000/00000/00220
d D 1.5 92/08/07 01:03:45 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00008/00004/00212
d D 1.4 91/08/26 01:11:59 vanb 4 3
c minor pocedure parameter syntax changes
e
s 00028/00020/00188
d D 1.3 91/05/03 23:11:17 labc-3id 3 2
c 
e
s 00000/00000/00208
d D 1.2 91/05/02 16:51:04 labc-3id 2 1
c Added by Trevor Pering
e
s 00208/00000/00000
d D 1.1 91/05/02 16:44:07 labc-3id 1 0
c date and time created 91/05/02 16:44:07 by labc-3id
e
u
U
f e 0
t
T
I 1
#include "stdio.h"
#include "config.h"
#include "map.h"
#include "objects.h"
#include "math.h"

extern long display;
I 3
extern unsigned char click_buffer[3];  /* it is not in any include file */
E 3

/*****************************************************************/

I 3
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

E 3
D 4
make_random_room(int x,int y)
E 4
I 4
make_random_room(x, y)
int x, y;
E 4
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
I 3
      break;
E 3
    case 4:
      make_random_inside(x,y);
      break;
    }
}

/*****************************************************************/

D 4
make_random_inside(int x,int y)
E 4
I 4
make_random_inside(x, y)
int x, y;
E 4
{
  int wall,end,i,j;

D 3
  wall = room[current_room].spot[0][0][0];
E 3
I 3
  wall = click_buffer[0];
E 3

D 3
  if (room[current_room].spot[1][0][0] != wall)
E 3
I 3
  if (getsq(current_room,1,0) != wall)
E 3
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(i,0,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(i,0,wall,current_room);
    }
D 3
  if (room[current_room].spot[0][1][0] != wall)
E 3
I 3
  if (getsq(current_room,0,1) != wall)
E 3
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(0,i,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(0,i,wall,current_room);
    }
D 3
  if (room[current_room].spot[ROOM_WIDTH-1][1][0] != wall)
E 3
I 3
  if (getsq(current_room,ROOM_WIDTH-1,1) != wall)
E 3
    {
      end = lrand48()%(ROOM_WIDTH-6)+3;
      for (i = 0;i < end;i++)
	square(ROOM_WIDTH-1,i,wall,current_room);
      for (i = end+1;i < ROOM_WIDTH;i++)
	square(ROOM_WIDTH-1,i,wall,current_room);
    }
D 3
  if (room[current_room].spot[1][ROOM_HEIGHT-1][0] != wall)
E 3
I 3
  if (getsq(current_room,1,ROOM_HEIGHT-1) != wall)
E 3
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

D 3
#define MVX(dir)   ( ((dir)%4 == 1? 1 : ((dir)%4 == 3?-1:0)) )
#define MVY(dir)   ( ((dir)%4 == 2? 1 : ((dir)%4 == 0?-1:0)) )
E 3

D 3
#define CHKAHD(x,y,dir,rm,wall)  ((((x)+MVX(dir)*2  < 0 ? 0 : \
				    ((x)+MVX(dir)*2 > (ROOM_WIDTH-1) ? 0 : \
				     ((y)+MVY(dir)*2 < 0 ? 0 : \
				      ((y)+MVY(dir)*2 > (ROOM_HEIGHT-1) ? 0 : \
				       (room[rm].spot[x+MVX(dir)*2]\
					[y+MVY(dir)*2][0] == 0) ))))))

E 3
D 4
make_random_maze(int x,int y)
E 4
I 4
make_random_maze(x, y)
int x, y;
E 4
{
  int dir,dir1;
  int tries,wall,rm;
  int i;

D 3
  wall = room[current_room].spot[0][0][0];
  room[current_room].spot[0][0][0] = 0;
E 3
I 3
  wall = click_buffer[0];

E 3
  if (wall == 0)
    return;

  rm = current_room;

  x = ((lrand48()%(ROOM_WIDTH-2))/2)*2;
  y = ((lrand48()%(ROOM_WIDTH-2))/2)*2;
  square(x,y,wall,rm);
  
  tries = 0;
  while(tries < 500)
    {
D 3
      if (room[rm].spot[x][y][0] != wall)
E 3
I 3
      if (getsq(rm,x,y) != wall)
E 3
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

D 4
make_random_antimaze(int x,int y)
E 4
I 4
make_random_antimaze(x, y)
int x, y;
E 4
{
  int dir,dir1;
  int tries,wall,rm;
  int i;

D 3
  wall = room[current_room].spot[0][0][0];
  room[current_room].spot[0][0][0] = 0;
E 3
I 3

  wall = click_buffer[0];

E 3
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
D 3
	  while( (room[rm].spot[x][y][0] == wall) && (tries < 500) );
E 3
I 3
	  while( (getsq(rm,x,y) == wall) && (tries < 500) );
E 3
	}

      tries++;
    }
}

/*****************************************************************/

E 1
