/* create a set of values (constants) to be put in a file */

#include <stdio.h>

float mlimit(int s, int c)
{ 
  float fs, fc, fa;

  fs = (float) s;
  fc = (float) c;

  fa = fs +.5 - (fs+.5)*(2*fc-1)/(2*fc+1);
  return fa;
}

main()
{ float a[20][20];
  int s, c;
  int i, j;

  for(c=0; c<20; c++)
    for(s=0; s<20; s++)
      a[s][c]= mlimit(s,c);

  for(c=0; c<20; c++)
    for(s=0; s<20; s++) {
      i = a[s][c] + 0.99999999999;
      j = a[s][c];
      if (!s) printf("\n");
      printf("{%d, %d}, ",i , j);
      /* printf("S: %d C: %d  A: %f\n", s, c, a[s][c]); */
    }
}

  

