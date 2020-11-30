h09134
s 00000/00000/00024
d D 1.4 92/08/07 01:04:28 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00004/00002/00020
d D 1.3 91/07/07 18:47:30 labc-3id 3 2
c added more ifndef's
e
s 00002/00000/00020
d D 1.2 91/07/04 15:36:44 labc-3id 2 1
c changes made to help effect new object set
e
s 00020/00000/00000
d D 1.1 91/07/04 11:58:40 labc-3id 1 0
c date and time created 91/07/04 11:58:40 by labc-3id
e
u
U
f e 0
t
T
I 1
/* Volvox Software Group - volvox.h
 * Last change: 23 April 1990
 * Specs: Van A. Boughner
 */

I 3
#ifndef FALSE
E 3
#define FALSE	0
#define	TRUE	1
I 3
#endif /* FALSE */
E 3

I 2
#ifndef demand
E 2
#define demand(pred,string)                                             \
  {                                                                     \
    if (!pred) {                                                        \
      printf("%s\n",string);                                            \
      exit(1);                                                          \
    }                                                                   \
  }
I 2
#endif /* demand */
E 2

I 3
#ifndef max
E 3
#define min(a,b) ((a>b) ? b : a)
#define max(a,b) ((a>b) ? a : b)
D 3

typedef int boolean;
E 3
I 3
#endif /* max */
E 3
E 1
