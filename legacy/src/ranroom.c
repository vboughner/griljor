/***************************************************************************
 * @(#) ranroom.c 1.9 - last change made 08/07/92
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

#include "stdio.h"
#include "ranroom.h"
#include "config.h"
#include "def.h"
#include "objects.h"
#include "map.h"
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

make_random_outside(x, y)
int x, y;
{
  char *getenv();
  int target_room, dir,start,i;
  SquareDef roomsquare;
  long seed;
  double fnum;
  int room_array[NEW_X_RAN][NEW_Y_RAN];
  double loading1[ROOM_WIDTH][ROOM_HEIGHT];
  double loading2[ROOM_WIDTH][ROOM_HEIGHT];
  int terrain[ROOM_WIDTH];
  int tree[ROOM_WIDTH][ROOM_HEIGHT];
  StatDef stats;

  stats.terrmark = terrain;
  stats.treemark = tree;
  
  place_editor_message("Thinking...");
  XFlush(display);

  read_terrain(&stats);

  /* now throw together all sorts of stuff to get a random seed */
  seed = clock();
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
    fnum = 0.3;
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

print_matrix(mat)
double mat[20][20];
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

read_terrain(stat)
StatDef *stat;
{
  int i,j;
  
  stat->range = 0;
  for (i = 0;i < 20;i++)
    if (stat->terrmark[i] = most_prominant_on_square(current_room,i,0))
      stat->range = i;

  for (i = 0;i < stat->range;i++)
    for (j = 0;j < stat->range;j++)
      stat->treemark[j][i] = most_prominant_on_square(current_room,i,j);
}

/*****************************************************************/

ProcessRoom(x, y, seed, room, loading1, loading2, stats)
int x, y;
long seed;
int room;
double loading1[ROOM_WIDTH][ROOM_HEIGHT];
double loading2[ROOM_WIDTH][ROOM_HEIGHT];
StatDef *stats;
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
	change_obj_on_square(i,j,what,room);
	what = LookupFoliage(holding2[i][j],holding1[i][j],stats);
	change_obj_on_square(i,j,what,room);
      }
  XFlush(display);
}

/*****************************************************************/

clear_holding(holding)
double holding[ROOM_WIDTH][ROOM_HEIGHT];
{
  int i,j;
  
  for (i = 0;i < ROOM_WIDTH;i++)
    for (j = 0;j < ROOM_HEIGHT;j++)
      holding[i][j] = -10.0;
}

/*****************************************************************/

SetSquare(square, x, y, a, b, c, d)
SquareDef *square;
int x, y;
double a, b, c, d;
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

LookupTerrain(h, stat)
double h;
StatDef *stat;
{
  int index;

  index = h * stat->range;

  if (index < 0) 
    index = 0;
  if (index > stat->range )
    index = stat->range;

  return(stat->terrmark[index]);
}

/*****************************************************************/

LookupFoliage(fol, h, stat)
double fol, h;
StatDef *stat;
{
  int hindex,findex;

  hindex = h * stat->range + 0.5;
  if (hindex < 0) 
    hindex = 0;
  if (hindex > stat->range )
    hindex = stat->range;

  findex = fol * stat->range;
  if (findex < 0) 
    findex = 0;
  if (findex > stat->range )
    findex = stat->range;

  return(stat->treemark[findex][hindex]);
}

/*****************************************************************/

double StartFrac(dosquare, prop, seed, holding)
SquareDef *dosquare;
PropDef *prop;
long seed;
double holding[ROOM_WIDTH][ROOM_HEIGHT];
{
  clear_holding(holding);
  holding[dosquare->x1][dosquare->y1] = dosquare->nw;
  holding[dosquare->x2][dosquare->y1] = dosquare->ne;
  holding[dosquare->x1][dosquare->y2] = dosquare->sw;
  holding[dosquare->x2][dosquare->y2] = dosquare->se;
  return(FractalIt(dosquare,prop,seed,holding));
}

/*****************************************************************/

double FractalIt(dosquare, prop, seed, holding)
SquareDef *dosquare;
PropDef *prop;
long seed;
double holding[ROOM_WIDTH][ROOM_HEIGHT];
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
			         
double DoFract(point1, point2, square, prop, seed)
int point1, point2;
SquareDef *square;
PropDef *prop;
long seed;
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
MakeSeed(square, a, b, seed)
SquareDef *square;
int a, b;
long seed;
{
  long newseed;

  newseed = (GETX(a,square)+square->gx)*21804 + 
            (GETY(b,square)+square->gy)*1290847;
  srand48(newseed);

  return(seed + lrand48());
}
