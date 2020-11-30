h46046
s 00000/00000/00012
d D 1.4 92/08/07 01:04:43 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00002/00001/00010
d D 1.3 91/10/16 20:36:07 labc-4lc 3 2
c fixed include file references
e
s 00011/00000/00000
d D 1.2 91/08/30 01:32:48 vanb 2 1
c made windows code color compatible
e
s 00000/00000/00000
d D 1.1 91/08/29 01:57:12 vanb 1 0
c date and time created 91/08/29 01:57:12 by vanb
e
u
U
t
T
I 2
D 3
#include <time.h>
E 3
I 3
#include <sys/types.h>
#include <sys/time.h>
E 3

void usleep(val)
long val;
{
  struct timeval temp;

  temp.tv_sec = 0;
  temp.tv_usec = val;
  select(0, 0, 0, 0, &temp);
}
E 2
I 1
E 1
