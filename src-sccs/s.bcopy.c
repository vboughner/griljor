h42408
s 00000/00000/00018
d D 1.3 92/08/07 01:00:20 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00018/00000/00000
d D 1.2 91/08/25 23:06:31 vanb 2 1
c 
e
s 00000/00000/00000
d D 1.1 91/08/25 23:05:40 vanb 1 0
c date and time created 91/08/25 23:05:40 by vanb
e
u
U
t
T
I 2
/* Small file which provides bcopy and bzero, not available on Stellar */

void bcopy(from, to, len)
char *from, *to;
int len;
{
  int i;
  for (i=0; i<len; i++) to[i] = from[i];
}


void bzero(targ, len)
char *targ;
int len;
{
  int i;
  for (i=0; i<len; i++) targ[i] = 0;
}
E 2
I 1
E 1
