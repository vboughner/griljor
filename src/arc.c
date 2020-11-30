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

#define degrade(degrees) (((double)degrees / 180.0) * M_PI)
#define sgn(num)	 ((num < 0.0) ? (-1.0) : (1.0))


makecircle(x, y, radius, curdir, cx, cy)
int x, y, radius;
double curdir;
int *cx, *cy;
{
  double dx,dy;
  double rd;

  rd = (double)radius + 0.4;

  dx = ((double)x + 0.5 + rd*cos(degrade(curdir)));
  dy = ((double)y + 0.5 + rd*sin(degrade(curdir)));
  *cx = floor(dx);
  *cy = floor(dy);
}


Location *missile_arc(x, y, radius, dir, Dtheta, fan, spread)
int x, y, radius, dir, Dtheta, fan, spread;
/* given a center at x,y and a radius, returns an arc of 
 * squares at distance radius in direction dir (in degrees) 
 * plus and minus Dtheta degrees */
{
  double step;
  double curdir,end;
  int cx, cy;
  double dx, dy;
  Location *result;
  Location *cur, *makelocation();

  if (radius == 0) return makelocation(x, y, NULL);
  if (Dtheta > 180) return NULL;
  if (Dtheta <= 0) Dtheta = 0;
  
  if (spread > 0)
    step = 2.0 * (double) Dtheta / ((double) ((Dtheta==180)?spread:spread-1));
  else if (spread == 0)
    step = 22.5 / (double) radius;
  else
    step = (1-spread ) * 45.0 / (double) radius;
  
  curdir = dir-Dtheta;
  makecircle(x,y,radius,curdir,&cx,&cy);
  result = makelocation(cx,cy,NULL);
  cur = result;
  curdir += step;
  end = ((double) (Dtheta + dir)) + 0.005;

  for (; curdir < end; curdir += step) 
    {
      makecircle(x,y,radius,curdir,&cx,&cy);
      if (cx == cur->x && cy == cur->y) continue;
      if (cx == result->x && cy == cur->y) continue;

      cur->next = makelocation(cx,cy,NULL);
      cur = cur->next;
    }
  return result;
}
