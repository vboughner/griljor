h31048
s 00000/00000/00087
d D 1.6 92/08/07 01:00:13 vbo 6 5
c source copied to a separate tree for work on new map and object format
e
s 00006/00002/00081
d D 1.5 91/08/29 01:39:37 vanb 5 4
c fixed up more compatibility problems
e
s 00011/00007/00072
d D 1.4 91/05/26 22:43:00 labc-3id 4 3
c Worked on drawing improvements
e
s 00020/00014/00059
d D 1.3 91/05/20 23:52:56 labc-3id 3 2
c 
e
s 00015/00008/00058
d D 1.2 91/02/23 01:48:18 labc-3id 2 1
c 
e
s 00066/00000/00000
d D 1.1 91/02/16 12:53:41 labc-3id 1 0
c date and time created 91/02/16 12:53:41 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 **************************************************************************/

/* Routines for creating arcs of missiles */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "def.h"
    

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

D 2
#define degrade(degrees) ((double)degrees / 180.0 * M_PI)
E 2
I 2
#define degrade(degrees) (((double)degrees / 180.0) * M_PI)
E 2
#define sgn(num)	 ((num < 0.0) ? (-1.0) : (1.0))


I 3
D 5
makecircle(int x,int y,int radius,double curdir,int *cx,int *cy)
E 5
I 5
makecircle(x, y, radius, curdir, cx, cy)
int x, y, radius;
double curdir;
int *cx, *cy;
E 5
{
  double dx,dy;
I 4
  double rd;
E 4

D 4
  dx = ((double)x+(double)radius*cos(degrade(curdir)));
  dy = ((double)y+(double)radius*sin(degrade(curdir)));
  *cx = (dx + (sgn(dx) * 0.5));
  *cy = (dy + (sgn(dy) * 0.5));
E 4
I 4
  rd = (double)radius + 0.4;

  dx = ((double)x + 0.5 + rd*cos(degrade(curdir)));
  dy = ((double)y + 0.5 + rd*sin(degrade(curdir)));
  *cx = floor(dx);
  *cy = floor(dy);
E 4
}


E 3
D 2
Location *missile_arc(int x,int y,int radius,int dir,int Dtheta)
E 2
I 2
D 5
Location *missile_arc(int x,int y,int radius,int dir,int Dtheta,int fan,int spread)
E 5
I 5
Location *missile_arc(x, y, radius, dir, Dtheta, fan, spread)
int x, y, radius, dir, Dtheta, fan, spread;
E 5
E 2
/* given a center at x,y and a radius, returns an arc of 
 * squares at distance radius in direction dir (in degrees) 
 * plus and minus Dtheta degrees */
{
D 2
  int step;
  int curdir;
E 2
I 2
  double step;
  double curdir,end;
E 2
  int cx, cy;
  double dx, dy;
  Location *result;
  Location *cur, *makelocation();

  if (radius == 0) return makelocation(x, y, NULL);
  if (Dtheta > 180) return NULL;
I 2
  if (Dtheta <= 0) Dtheta = 0;
E 2
  
D 2
  step = 45/radius;
  if (step < 5) step = 5;

E 2
I 2
  if (spread > 0)
    step = 2.0 * (double) Dtheta / ((double) ((Dtheta==180)?spread:spread-1));
I 4
  else if (spread == 0)
    step = 22.5 / (double) radius;
E 4
  else
    step = (1-spread ) * 45.0 / (double) radius;
  
E 2
  curdir = dir-Dtheta;
D 3
  dx = ((double)x+(double)radius*cos(degrade(curdir)));
  dy = ((double)y+(double)radius*sin(degrade(curdir)));
  cx = (dx + (sgn(dx) * 0.5));
  cy = (dy + (sgn(dy) * 0.5));
E 3
I 3
  makecircle(x,y,radius,curdir,&cx,&cy);
E 3
  result = makelocation(cx,cy,NULL);
  cur = result;
  curdir += step;
I 2
  end = ((double) (Dtheta + dir)) + 0.005;
D 4
  if (Dtheta == 180)
    end = end - step/2.0;
E 4
E 2

D 2
  for (; curdir <=  dir+Dtheta; curdir += step) {
E 2
I 2
D 3
  for (; curdir < end; curdir += step) {
E 2
    dx = ((double)x+(double)radius*cos(degrade(curdir)));
    dy = ((double)y+(double)radius*sin(degrade(curdir)));
    cx = (dx + (sgn(dx) * 0.5));
    cy = (dy + (sgn(dy) * 0.5));
    if (cx == cur->x && cy == cur->y) continue;
I 2
    
E 2
    cur->next = makelocation(cx,cy,NULL);
    cur = cur->next;
  }
E 3
I 3
  for (; curdir < end; curdir += step) 
    {
      makecircle(x,y,radius,curdir,&cx,&cy);
      if (cx == cur->x && cy == cur->y) continue;
D 4
      
E 4
I 4
      if (cx == result->x && cy == cur->y) continue;

E 4
      cur->next = makelocation(cx,cy,NULL);
      cur = cur->next;
    }
E 3
  return result;
}
E 1
