h63327
s 00000/00000/00051
d D 1.4 92/08/07 01:04:20 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00011/00008/00040
d D 1.3 91/08/27 02:21:27 vanb 3 2
c fixed up a few compatibility problems
e
s 00029/00002/00019
d D 1.2 91/04/17 16:52:49 labc-3id 2 1
c added string to long time conversion
e
s 00021/00000/00000
d D 1.1 91/04/14 19:35:45 labc-3id 1 0
c date and time created 91/04/14 19:35:45 by labc-3id
e
u
U
f e 0
t
T
I 1
#include <stdio.h>
#include <time.h>
  
  /* This procedure takes the time on the system and puts it into the
     following format :
     
     DD/MM/YY HH:MM.SS  */
  
D 2
  char *get_current_time_string ()
E 2
I 2
char *get_current_time_string ()
E 2
{
  static char tme[20];
  struct tm *tp;
  long	blah;
  
D 3
  blah = time(NULL);
E 3
I 3
/*   blah = time(NULL);
E 3
  tp = localtime (&blah);
D 2
  strftime (tme, 20, "%d/%m/%y %H:%M.%S", tp);
E 2
I 2
  strftime (tme, 20, "%m/%d/%y %H:%M.%S", tp);
E 2
D 3
  return tme;
E 3
I 3
  return tme; */
  return "time unknown";
E 3
}


I 2
  /* This procedure takes the time given and puts it into the
     following format :
     
     DD/MM/YY HH:MM.SS  */
  
char *get_time_string (t)
     time_t t;
{
  static char tme[20];
  struct tm *tp;
  
D 3
  tp = localtime (&t);
E 3
I 3
/*   tp = localtime (&t);
E 3
  strftime (tme, 20, "%m/%d/%y %H:%M.%S", tp);
D 3
  return tme;
E 3
I 3
  return tme; */
  return "time unknown";
E 3
}


D 3
time_t evaluate_time_string(char *s)
E 3
I 3
time_t evaluate_time_string(s)
char *s;
E 3
{
  time_t result;
  struct tm tp;

D 3
  (void) strptime(s, "%d/%m/%y %H:%M.%S", &tp);
E 3
I 3
/*   (void) strptime(s, "%d/%m/%y %H:%M.%S", &tp);
E 3
  result = timelocal(&tp);
D 3

  return result;
E 3
I 3
  return result; */
  return time(NULL);
E 3
}
E 2
E 1
