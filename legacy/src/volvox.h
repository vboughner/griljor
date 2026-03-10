/* Volvox Software Group - volvox.h
 * Last change: 23 April 1990
 * Specs: Van A. Boughner
 */

#ifndef FALSE
#define FALSE	0
#define	TRUE	1
#endif /* FALSE */

#ifndef demand
#define demand(pred,string)                                             \
  {                                                                     \
    if (!pred) {                                                        \
      printf("%s\n",string);                                            \
      exit(1);                                                          \
    }                                                                   \
  }
#endif /* demand */

#ifndef max
#define min(a,b) ((a>b) ? b : a)
#define max(a,b) ((a>b) ? a : b)
#endif /* max */
