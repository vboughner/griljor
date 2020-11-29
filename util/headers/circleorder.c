
#include <stdio.h>
#include <time.h>
#include <math.h>


/* creates plot points in a circle, emenating from center, outputting
   deltas rather than coordinates */

#define XOUTFILE	"x.nums"
#define YOUTFILE	"y.nums"
#define MAX		38
#define CENTER		19
#define RADIUS		40
#define sgn(num)         ((num < 0.0) ? (-1.0) : (1.0))

main()
{
  int used[MAX+1][MAX+1], not_used = 0;
  int i, j, ax, ay, r, deg, count = 0;
  double dx, dy;
  FILE *xout, *yout;

  /* open files */
  xout = fopen(XOUTFILE, "w");
  yout = fopen(YOUTFILE, "w");
  
  /* clear array */
  for (i=0; i<=MAX; i++)
    for (j=0; j<=MAX; j++)
      used[i][j] = 0;
  printf("array cleared\n");

  /* assign radial squares until done */
  for (r=0; r<RADIUS; r++) {
    printf("Radius at %d\n", r);
    for (deg=0; deg<360; deg++) {
      dx = (double) r * cos((double)deg);
      dy = (double) r * sin((double)deg);
      i = (dx + ((double) sgn(dx) * 0.5));
      j = (dy + ((double) sgn(dy) * 0.5));
      ax = CENTER + i;
      ay = CENTER + j;
      if (ax>=0 && ax<=MAX && ay>=0 && ay<=MAX)
        if (!used[ax][ay]) {
          used[ax][ay] = 1;
          fprintf(xout, "%d, ", i);
          fprintf(yout, "%d, ", j);
          if (!(count % 15)) {
            fprintf(xout, "\n");
  	    fprintf(yout, "\n");
          }
          count++;
	}
    }
  }

  fclose(xout);
  fclose(yout);

  /* were all the squares filled? */
  for (i=0; i<=MAX; i++)
    for (j=0; j<=MAX; j++)
      if (!used[i][j]) not_used++;

  printf("%d squares not accounted for\n", not_used);
}
