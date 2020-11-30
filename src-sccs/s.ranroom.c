h42518
s 00000/00000/00414
d D 1.9 92/08/07 21:34:09 vbo 9 8
c fixes for sun port merged in
e
s 00002/00001/00412
d D 1.8 92/02/21 22:45:16 vanb 8 7
c 
e
s 00023/00048/00390
d D 1.7 92/02/15 01:07:02 vanb 7 6
c 
e
s 00022/00000/00416
d D 1.6 91/10/16 20:06:25 labc-4lc 6 5
c changed made for DecStations
e
s 00044/00017/00372
d D 1.5 91/08/26 01:11:52 vanb 5 4
c minor pocedure parameter syntax changes
e
s 00000/00000/00389
d D 1.4 91/06/08 14:20:07 labc-3id 4 3
c Just found it checked out.
e
s 00015/00020/00374
d D 1.3 91/05/03 23:10:11 labc-3id 3 2
c 
e
s 00000/00000/00394
d D 1.2 91/05/02 16:50:46 labc-3id 2 1
c Added by Trevor Pering
e
s 00394/00000/00000
d D 1.1 91/05/02 16:43:42 labc-3id 1 0
c date and time created 91/05/02 16:43:42 by labc-3id
e
u
U
f e 0
t
T
I 6
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

E 6
I 1
#include "stdio.h"
#include "ranroom.h"
#include "config.h"
D 8
#include "map.h"
E 8
I 8
#include "def.h"
E 8
#include "objects.h"
I 8
#include "map.h"
E 8
#include "math.h"

double FractalIt();
double StartFrac();
double DoFract();
double drand48();
long   lrand48();

extern long display;

/*****************************************************************/

PropDef MountMain = { 0.03 , 0.05, 0.02 };
PropDef MountProp = { 0.01 , 0.04, 0.05 };
PropDef TreeMain =  { 0.03 , 0.03, 0.03 };
PropDef TreeProp =  { 0.005, 0.03, 0.03 };

/*****************************************************************/
D 7
/* this procedure make a random world out there. */
E 7

I 7
void change_obj_on_square(x,y,n,thisroom)
int x, y;
int n;
int thisroom;
{
  /* putting down the nothing object does nothing */
  if (n == 0) return;

  /* if it is already there then leave without doing anything */
  if (is_object_type_here(&mapstats, thisroom, x, y, n)) return;

  /* replace any previous object of same masking type here */
  add_object_replacing_same_mask(&mapstats, thisroom, x, y, n);

  if (thisroom == current_room) draw_room_square(x, y, 0);
}

/*****************************************************************/

/* this procedure makes a random world out there. */

E 7
D 5
make_random_outside(int x,int y)
E 5
I 5
make_random_outside(x, y)
int x, y;
E 5
{
I 6
  char *getenv();
E 6
  int target_room, dir,start,i;
  SquareDef roomsquare;
  long seed;
  double fnum;
  int room_array[NEW_X_RAN][NEW_Y_RAN];
  double loading1[ROOM_WIDTH][ROOM_HEIGHT];
  double loading2[ROOM_WIDTH][ROOM_HEIGHT];
D 3
  int terrain[20];
  int tree[20][20];
E 3
I 3
  int terrain[ROOM_WIDTH];
  int tree[ROOM_WIDTH][ROOM_HEIGHT];
E 3
  StatDef stats;

  stats.terrmark = terrain;
  stats.treemark = tree;
  
  place_editor_message("Thinking...");
  XFlush(display);

  read_terrain(&stats);

  /* now throw together all sorts of stuff to get a random seed */
D 3
  seed = 0;
  for (i = 0;i < strlen(mapstats.name);i++)
    seed = seed*27 + mapstats.name[i];
  seed += current_room + x*200 + y* 429874;
E 3
I 3
  seed = clock();
E 3
  srand48(seed);
  seed = lrand48();

  for (x = 0;x < NEW_X_RAN;x++)
    for (y = 0;y < NEW_Y_RAN;y++)
      {
	add_new_room();
	room_array[x][y] = mapstats.rooms-1;
      }

  for (x = 0;x < NEW_X_RAN;x++)
    for (y = 1;y < NEW_Y_RAN;y++)
      link_one_room_to_another(room_array[x][y-1],SOUTH,room_array[x][y]);
  
  for (x = 1;x < NEW_X_RAN;x++)
    for (y = 0;y < NEW_Y_RAN;y++)
      link_one_room_to_another(room_array[x-1][y],EAST,room_array[x][y]);

  if (getenv("LAND") == NULL)
    fnum = 0.5;
  else
    sscanf(getenv("LAND"),"%lf",&fnum);
  SetSquare(&roomsquare,0,0,fnum,fnum,fnum,fnum);
  StartFrac(&roomsquare,&MountMain,seed,loading1);

  if (getenv("TREE") == NULL)
D 3
    fnum = 0.2;
E 3
I 3
    fnum = 0.3;
E 3
  else
    sscanf(getenv("TREE"),"%lf",&fnum);
  SetSquare(&roomsquare,0,0,fnum,fnum,fnum,fnum);
  StartFrac(&roomsquare,&TreeMain,seed,loading2);

  place_editor_message("Processing rooms...");

  for (x = 0;x < NEW_X_RAN;x++)
    for (y = 0;y < NEW_Y_RAN;y++)
      ProcessRoom(x+2,y+2,seed,room_array[x][y],loading1,loading2,&stats);

  place_editor_message("Done with room processing.");
}

/*****************************************************************/

D 5
print_matrix(double mat[20][20])
E 5
I 5
print_matrix(mat)
double mat[20][20];
E 5
{
  int i,j;
  
  for(i = 0;i < 20;i++)
    {
      for(j = 0;j < 20;j++)
	printf("%2.2lf ",mat[i][j]);
      printf("\n");
    }
}

/*****************************************************************/

D 5
read_terrain(StatDef *stat)
E 5
I 5
read_terrain(stat)
StatDef *stat;
E 5
{
  int i,j;
  
  stat->range = 0;
D 3
  
E 3
  for (i = 0;i < 20;i++)
D 3
    {
      if (stat->terrmark[i] = room[current_room].spot[i][0][0])
	stat->range = i;
    }
E 3
I 3
    if (stat->terrmark[i] = most_prominant_on_square(current_room,i,0))
      stat->range = i;
E 3

  for (i = 0;i < stat->range;i++)
    for (j = 0;j < stat->range;j++)
D 3
      stat->treemark[j][i] = room[current_room].spot[i][j+1][0];
E 3
I 3
      stat->treemark[j][i] = most_prominant_on_square(current_room,i,j);
E 3
}

/*****************************************************************/

D 5
ProcessRoom(int x,int y,long seed,int room,
	    double loading1[ROOM_WIDTH][ROOM_HEIGHT],
	    double loading2[ROOM_WIDTH][ROOM_HEIGHT],
	    StatDef *stats)
E 5
I 5
ProcessRoom(x, y, seed, room, loading1, loading2, stats)
int x, y;
long seed;
int room;
double loading1[ROOM_WIDTH][ROOM_HEIGHT];
double loading2[ROOM_WIDTH][ROOM_HEIGHT];
StatDef *stats;
E 5
{
  int i,j,what;
  SquareDef roomsquare;
  double holding1[ROOM_WIDTH][ROOM_HEIGHT];
  double holding2[ROOM_WIDTH][ROOM_HEIGHT];

  SetSquare(&roomsquare,x*(ROOM_WIDTH-1),y*(ROOM_WIDTH-1),
	    loading1[x][y],loading1[x+1][y],
	    loading1[x][y+1],loading1[x+1][y+1]);
  StartFrac(&roomsquare,&MountProp,seed,holding1);
  
  SetSquare(&roomsquare,x*(ROOM_WIDTH-1),y*(ROOM_WIDTH-1),
	    loading2[x][y],loading2[x+1][y],
	    loading2[x][y+1],loading2[x+1][y+1]);
  StartFrac(&roomsquare,&TreeProp,seed,holding2); 

  change_rooms(room);
  for (i = 0;i < ROOM_WIDTH;i++)
    for (j = 0;j < ROOM_HEIGHT;j++) 
      {
	what = LookupTerrain(holding1[i][j],stats);
D 7
	square(i,j,what,room);
E 7
I 7
	change_obj_on_square(i,j,what,room);
E 7
	what = LookupFoliage(holding2[i][j],holding1[i][j],stats);
D 7
	square(i,j,what,room);
E 7
I 7
	change_obj_on_square(i,j,what,room);
E 7
      }
  XFlush(display);
}

/*****************************************************************/

D 5
clear_holding(double holding[ROOM_WIDTH][ROOM_HEIGHT])
E 5
I 5
clear_holding(holding)
double holding[ROOM_WIDTH][ROOM_HEIGHT];
E 5
{
  int i,j;
  
  for (i = 0;i < ROOM_WIDTH;i++)
    for (j = 0;j < ROOM_HEIGHT;j++)
      holding[i][j] = -10.0;
}

/*****************************************************************/

D 5
SetSquare(SquareDef *square,int x,int y,double a,double b,double c,double d)
E 5
I 5
SetSquare(square, x, y, a, b, c, d)
SquareDef *square;
int x, y;
double a, b, c, d;
E 5
{
  square->nw = a;
  square->ne = b;
  square->sw = c;
  square->se = d;
  square->gx = x;
  square->gy = y;
  square->x1 = 0;
  square->x2 = ROOM_WIDTH-1;
  square->y1 = 0;
  square->y2 = ROOM_HEIGHT-1;
}

/*****************************************************************/

D 5
LookupTerrain(double h,StatDef *stat)
E 5
I 5
LookupTerrain(h, stat)
double h;
StatDef *stat;
E 5
{
  int index;

  index = h * stat->range;

  if (index < 0) 
    index = 0;
D 3
  if (index > 9 )
    index = 9;
E 3
I 3
  if (index > stat->range )
    index = stat->range;
E 3

  return(stat->terrmark[index]);
}

/*****************************************************************/

D 5
LookupFoliage(double fol,double h,StatDef *stat)
E 5
I 5
LookupFoliage(fol, h, stat)
double fol, h;
StatDef *stat;
E 5
{
  int hindex,findex;

D 3
  hindex = h * stat->range;
E 3
I 3
  hindex = h * stat->range + 0.5;
E 3
  if (hindex < 0) 
    hindex = 0;
D 3
  if (hindex > 9 )
    hindex = 9;
E 3
I 3
  if (hindex > stat->range )
    hindex = stat->range;
E 3

  findex = fol * stat->range;
  if (findex < 0) 
    findex = 0;
D 3
  if (findex > 9 )
    findex = 9;
E 3
I 3
  if (findex > stat->range )
    findex = stat->range;
E 3

  return(stat->treemark[findex][hindex]);
}

/*****************************************************************/

D 5
double StartFrac(SquareDef *dosquare,PropDef *prop,long seed,
		 double holding[ROOM_WIDTH][ROOM_HEIGHT])
E 5
I 5
double StartFrac(dosquare, prop, seed, holding)
SquareDef *dosquare;
PropDef *prop;
long seed;
double holding[ROOM_WIDTH][ROOM_HEIGHT];
E 5
{
  clear_holding(holding);
  holding[dosquare->x1][dosquare->y1] = dosquare->nw;
  holding[dosquare->x2][dosquare->y1] = dosquare->ne;
  holding[dosquare->x1][dosquare->y2] = dosquare->sw;
  holding[dosquare->x2][dosquare->y2] = dosquare->se;
  return(FractalIt(dosquare,prop,seed,holding));
}

/*****************************************************************/

D 5
double FractalIt(SquareDef *dosquare,PropDef *prop,long seed,
		 double holding[ROOM_WIDTH][ROOM_HEIGHT])
E 5
I 5
double FractalIt(dosquare, prop, seed, holding)
SquareDef *dosquare;
PropDef *prop;
long seed;
double holding[ROOM_WIDTH][ROOM_HEIGHT];
E 5
{
  int middlex,middley,i,j,what;
  SquareDef nsquare;
  
  if ((dosquare->x1 >= dosquare->x2-1)&&
      (dosquare->y1 >= dosquare->y2-1))
    return;

  middlex = (dosquare->x1 + dosquare->x2)/2;
  middley = (dosquare->y1 + dosquare->y2)/2;

  holding[middlex][middley]    = DoFract(0,3,dosquare,prop,seed);
  if ((middley != dosquare->y1)&&(middley != dosquare->y2))
    {
      holding[dosquare->x1][middley] = DoFract(0,2,dosquare,prop,seed);
      holding[dosquare->x2][middley] = DoFract(1,3,dosquare,prop,seed);
    }
  if ((middlex != dosquare->x1)&&(middlex != dosquare->x2))
    {
      holding[middlex][dosquare->y1] = DoFract(0,1,dosquare,prop,seed);
      holding[middlex][dosquare->y2] = DoFract(2,3,dosquare,prop,seed);
    }

  nsquare = *dosquare;
  nsquare.x2 = middlex;
  nsquare.y2 = middley;
  nsquare.ne = holding[middlex][dosquare->y1];
  nsquare.sw = holding[dosquare->x1][middley];
  nsquare.se = holding[middlex][middley];
  FractalIt(&nsquare,prop,seed,holding);
  nsquare.x1 = middlex;
  nsquare.x2 = dosquare->x2;
  nsquare.nw = nsquare.ne;
  nsquare.sw = nsquare.se;
  nsquare.ne = holding[dosquare->x2][dosquare->y1];
  nsquare.se = holding[dosquare->x2][middley];
  FractalIt(&nsquare,prop,seed,holding);
  nsquare.y1 = middley;
  nsquare.y2 = dosquare->y2;
  nsquare.nw = nsquare.sw;
  nsquare.ne = nsquare.se;
  nsquare.sw = holding[middlex][dosquare->y2];
  nsquare.se = holding[dosquare->x2][dosquare->y2];
  FractalIt(&nsquare,prop,seed,holding);
  nsquare.x1 = dosquare->x1;
  nsquare.x2 = middlex;
  nsquare.ne = nsquare.nw;
  nsquare.se = nsquare.sw;
  nsquare.nw = holding[dosquare->x1][middley];
  nsquare.sw = holding[dosquare->x1][dosquare->y2];
  FractalIt(&nsquare,prop,seed,holding);
}
	   
/*****************************************************************/

#define GETLEVEL(quad,sq)  ((quad == 0)? sq->nw :   \
			     ((quad == 1)? sq->ne :   \
			     ((quad == 2)? sq->sw : sq->se)))

#define GETX(quad,sq)  ((quad == 0)? sq->x1 :   \
			((quad == 1)? sq->x2 :   \
			 ((quad == 2)? sq->x1 : sq->x2)))
#define GETY(quad,sq)  ((quad == 0)? sq->y1 :   \
			((quad == 1)? sq->y1 :   \
			 ((quad == 2)? sq->y2 : sq->y2)))
			         
D 5
double DoFract(int point1,int point2,SquareDef *square,PropDef *prop,long seed)
E 5
I 5
double DoFract(point1, point2, square, prop, seed)
int point1, point2;
SquareDef *square;
PropDef *prop;
long seed;
E 5
{
  double dist,distx,disty,new,ruse,diff;

  if (point1%2 == point2%2)
    distx = 0.0;
  else
    distx = square->x1 - square->x2;

  if (point1/2 == point2/2)
    disty = 0.0;
  else
    disty = square->y1 - square->y2;

  dist = sqrt(distx*distx + disty*disty);
  
  new = ((GETLEVEL(point1,square) + GETLEVEL(point2,square))/2.0);

  diff = GETLEVEL(point1,square) - GETLEVEL(point2,square);
  if (diff < 0.0) diff = -diff;

  ruse = dist*prop->dismult + diff*prop->diffmult + prop->ranadd;

  seed = MakeSeed(square,point1,point2,seed);
  srand48(seed);

  new += ruse * (drand48() - drand48());

  return(new);
}

/*****************************************************************/

/* This must be a function of the endpoints only (and a constant 'seed' */
D 5
MakeSeed(SquareDef *square,int a,int b,long seed)
E 5
I 5
MakeSeed(square, a, b, seed)
SquareDef *square;
int a, b;
long seed;
E 5
{
  long newseed;

  newseed = (GETX(a,square)+square->gx)*21804 + 
            (GETY(b,square)+square->gy)*1290847;
  srand48(newseed);

  return(seed + lrand48());
}
D 7

/*****************************************************************/

square(x,y,n,thisroom)
int x, y;
int n;
int thisroom;
{
  register int i;
  int found;

  /* putting down the nothing object does nothing */
  if (n == 0)
    return;

  /* check to see if this kind of char is already here */
  for (i=0,found = -1; i<ROOM_DEPTH && found == -1; i++)
    if (room[thisroom].spot[x][y][i] == n) found = i;

  /* if it is already there then leave without doing anything */
  /* if its object #0 then it doesn't matter if it's already there */
  if (found != -1 && n != 0) return;

  /* look for obj of same masking type */
  for (i=0,found = -1; i<ROOM_DEPTH && found == -1; i++)
      if (info[(unsigned char)room[thisroom].spot[x][y][i]]->masked ==
	  info[(unsigned char)n]->masked) found = i;

  /* if no same type found, look for empty obj spot */
  if (found == -1 )
    for (i=0; i<ROOM_DEPTH && found == -1; i++)
      if (room[thisroom].spot[x][y][i] == 0)
        found = i;

  /* if still nothing found then just replace the first one */
  if (found == -1) found = 0;
  /* replace the object */

  room[thisroom].spot[x][y][found] = n;
  if (thisroom == current_room) draw_room_square(x, y, 0);
}

/*****************************************************************/
I 3

E 3

E 7
E 1
