
#include <time.h>
#include <stdio.h>

#define XOUTFILE	"x.nums"
#define YOUTFILE	"y.nums"

main()
{
  int used[20][20];
  int i, j;
  int left = 400;
  FILE *xout, *yout;

  initrand();
  
  /* open files */
  xout = fopen(XOUTFILE, "w");
  yout = fopen(YOUTFILE, "w");
  
  /* clear array */
  for (i=0; i<20; i++)
    for (j=0; j<20; j++)
      used[i][j] = 0;

  /* assign randomly until done */
  for (;left;) {
    i = lrand48() % 20;
    j = lrand48() % 20;

    if (!used[i][j]) {
      used[i][j] = 1;
      fprintf(xout, "%d, ", i);
      fprintf(yout, "%d, ", j);
      if (!(left % 20)) {
        fprintf(xout, "\n");
	fprintf(yout, "\n");
      }
      left--;
      printf("%d\tleft\n", left);
    }
  }



  fclose(xout);
  fclose(yout);
}



/* variable initializer for use with lrand48 you need to include
   <time.h> and <stdio.h> in the file that calls this */

initrand()
{ int j;
  long k;

  /* initialize random variable */

  k = time(NULL);
  j = (unsigned int) k/2;
  srand48(j);

}
