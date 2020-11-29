/* Burt, an Eliza-like programming project in C
   This particular implementation is by Van Boughner    May 1989    */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "burt.h"


/* Global variable */
int debug_mode = 0;

/* global in this file only */
struct personalities	*burt_person[MAX_PERSONALITIES];



/* Given a personality slot to load into, load a given file as personality.
   Return TRUE only if everything went successfully, return FALSE otherwise. */

set_personality(num, filename)
int num;
char *filename;
{
  int succeeded;

  /* check for bad personality number */
  if (num < 0 || num >= MAX_PERSONALITIES) return FALSE;

  /* set up random generator */
  start_random_generator();

  /* attempt to load the personality and return success flag */
  burt_person[num] = allocate_personality();
  succeeded = load_personality(burt_person[num], filename);
  if (!succeeded) {
    free(burt_person[num]);
    burt_person[num] = NULL;
  }
  return succeeded;
}




/* Given a personality number and an input string, find a response.  If
   there is an error, return NULL */

char *query_personality(num, input_line)
int num;
char *input_line;
{
  static char response[LINE_LENGTH];

  /* check for bad personality number */
  if (num < 0 || num >= MAX_PERSONALITIES) return NULL;
  if (burt_person[num] == NULL) return NULL;

  /* get the appropriate response */
  strcpy(response, make_response(input_line, burt_person[num]));

  /* clean up the response a little */
  remove_first_space(response);
  make_first_char_capital(response);
  remove_twiddles(response);
  /* make_into_lines(response); */ /* We don't want lines separated */

  /* return the final version of response */
  return response;
}



/* allocate memory for a personality */

struct personalities *allocate_personality()
{
  struct personalities *new;

  new = (struct personalities *) malloc(sizeof(struct personalities));
  if (new == NULL) Gerror("no memory for personality load");

  return new;
}



start_random_generator()
{
  unsigned int	stime;
  long		ltime;

  ltime = time(NULL);
  stime = (unsigned int) ltime/2;
  srand(stime);
}



int_rand(i)
int i;
{
  int result;

  result = (rand() % i) + 1;
  return result;
}
