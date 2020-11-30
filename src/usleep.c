#include <sys/types.h>
#include <sys/time.h>

void usleep(val)
long val;
{
  struct timeval temp;

  temp.tv_sec = 0;
  temp.tv_usec = val;
  select(0, 0, 0, 0, &temp);
}
