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
