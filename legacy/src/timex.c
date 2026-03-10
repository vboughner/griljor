#include <stdio.h>
#include <time.h>
  
  /* This procedure takes the time on the system and puts it into the
     following format :
     
     DD/MM/YY HH:MM.SS  */
  
char *get_current_time_string ()
{
  static char tme[20];
  struct tm *tp;
  long	blah;
  
/*   blah = time(NULL);
  tp = localtime (&blah);
  strftime (tme, 20, "%m/%d/%y %H:%M.%S", tp);
  return tme; */
  return "time unknown";
}


  /* This procedure takes the time given and puts it into the
     following format :
     
     DD/MM/YY HH:MM.SS  */
  
char *get_time_string (t)
     time_t t;
{
  static char tme[20];
  struct tm *tp;
  
/*   tp = localtime (&t);
  strftime (tme, 20, "%m/%d/%y %H:%M.%S", tp);
  return tme; */
  return "time unknown";
}


time_t evaluate_time_string(s)
char *s;
{
  time_t result;
  struct tm tp;

/*   (void) strptime(s, "%d/%m/%y %H:%M.%S", &tp);
  result = timelocal(&tp);
  return result; */
  return time(NULL);
}
