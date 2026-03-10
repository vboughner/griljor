#define BIT_SIZE 64
#include <stdio.h>

#define FALSE 0
#define TRUE (! FALSE)

#define demand(test,errmsg)						\
{									\
  if (!(test)) {							\
    printf("%s\n",errmsg);						\
    exit(1);								\
  }									\
}
