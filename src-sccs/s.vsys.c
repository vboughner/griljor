h59709
s 00000/00000/00241
d D 1.3 92/08/07 01:04:30 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00005/00005/00236
d D 1.2 91/07/04 13:43:47 labc-3id 2 1
c changed names of string_time and twostud functions
e
s 00241/00000/00000
d D 1.1 91/07/04 11:59:07 labc-3id 1 0
c date and time created 91/07/04 11:59:07 by labc-3id
e
u
U
f e 0
t
T
I 1
/* vsys.c
 * Volvox Software Group
 * February 1991
 * Specs: Van A. Boughner
 * Author: Van A. Boughner
 * Purpose: This assorted routines do various system and miscellaneous
 * functions and save a little time in writing new programs that use
 * files and do other system-type maneuvers.
 * This package does not depend on the existance of any others.
 */

#include "vsys.h"
#include <time.h>
#include <pwd.h>


/* function to place zeros in l consecutive bytes of memory, starting at s */
void zero_memory(s, l)
char *s;
int l;
{
  int i;
  for (i=0; i<l; i++) *(s+i) = 0;
}


/* allocate l bytes of memory, initialize all of it to zero, and return a
   pointer.  If there is insufficient memory, quit with an error message. */
char *valloc(l)
int l;
{
  char *p;

  demand((p = (char *) malloc(l)), "error: not enough memory in valloc()");
  zero_memory(p, l);
  return p;
}


/* take a number and return a string containing a zero on the left if the
   number is less than 10.  If number is more than 100, use only two right
   most digits.  Caller may free the result, if he wishes (memory use
   is small in this case). */
D 2
char *twostud(num)
E 2
I 2
char *v_twostud(num)
E 2
int num;
{
  char *result;

  num = num % 100;
  result = valloc(3);
  sprintf(result, "%2d", num);
  if (num < 10) result[0] = '0';

  return result;
}



/* returns a string representation of the current time.  The caller must
   free the result himself.  May return NULL when there is no memory left. */
D 2
char *string_time()
E 2
I 2
char *v_string_time()
E 2
{
  char *result[40];
  long long_time;
  struct tm *current_time;

  /* get the current time if it is avaliable */
  long_time = time(NULL);
  if (long_time == -1) {
    fprintf(stderr, "Warning: system does not keep time\n");
    return strdup("unknown");
  }

  current_time = localtime(&long_time);
D 2
  sprintf(result, "%s:%s %s/%s/%s", twostud(current_time->tm_hour),
          twostud(current_time->tm_min), twostud(current_time->tm_mon + 1),
          twostud(current_time->tm_mday), twostud(current_time->tm_year));
E 2
I 2
  sprintf(result, "%s:%s %s/%s/%s", v_twostud(current_time->tm_hour),
          v_twostud(current_time->tm_min), v_twostud(current_time->tm_mon + 1),
          v_twostud(current_time->tm_mday), v_twostud(current_time->tm_year));
E 2

  return strdup(result);
}


/* return the user login name of the owner of this process, or NULL
   if it cannot be determined.  Caller is responsible for freeing the
   string returned. */
char *username()
{
  char *result = NULL;
  struct passwd *pw_ent;

  pw_ent = getpwuid(getuid());
  if (pw_ent) {
    result = valloc(strlen(pw_ent->pw_name) + 1);
    strcpy(result, pw_ent->pw_name);
  }
  return result;
}



/* return the user login name concatenated with the hostname of the
   machine this process is running on.  Caller must free this memory
   himself someday. */
char *user_and_host()
{
  char *login, *host[500], *result;

  login = username();
  (void) gethostname(host, 500);
  result = valloc(strlen(login) + strlen(host) + 2);
  sprintf(result, "%s@%s", login, host);
  free(login);

  return result;
}



/* this routine looks in the effective user's password file entry and
   returns the pathname of his home directory.  The caller must free the
   string returned.  NULL is returned if there is any problem getting it.
   You must pass either the login name of user or NULL to look up
   current effective user's home directory. */
char *home_dir(user)
char *user;
{
  char *result = NULL;
  struct passwd *pw_ent;

  if (user)
    pw_ent = getpwnam(user);
  else
    pw_ent = getpwuid(getuid());

  if (pw_ent) {
    if (pw_ent->pw_dir) {
      result = valloc(strlen(pw_ent->pw_dir) + 1);
      strcpy(result, pw_ent->pw_dir);
    }
  }
  return result;
}



/* this routine takes a directory name and a filename and puts them together.
   The directory name may have a '/' on the end of it, or not.  Both cases
   are handled.  Caller must free the string returned when done with it.
   If 'dir' is NULL, just the filename is returned.  If 'filename' is
   NULL an error occurs. */
char *concat_filename(dir, filename)
char *dir, *filename;
{
  char *result;
  demand(filename, "concat_filename: no filename given, pointer is NULL");

  if (dir) {
    int dlen = strlen(dir);
    char *ndir = strdup(dir);
    demand(ndir, "out of memory in concat_filename()");
    if (ndir[dlen-1] == '/') ndir[dlen-1] = '\0';
    result = valloc(strlen(ndir) + strlen(filename) + 2);
    sprintf(result, "%s/%s", ndir, filename);
    free(ndir);
  }
  else {
    result = strdup(filename);
    demand(result, "out of memory in concat_filename()");
  }

  return result;
}



/* this routine takes a directory path that contains a possible twiddle (~)
   character and converts it to an absolute pathname.  Examples:
	~/bin		~c60c-3xx/games
   It returns a pointer to a newly allocated string that the user
   should free when he's finished with it.  If there was no twiddle in the
   input string, a copy is simply made of the input string and the pointer
   to it returned.  In either case, the return value should be freed if it
   is not NULL.  NULL will only be returned if the in parameter was NULL
   or the user name specified next to the twiddle was invalid. */
char *expand_twiddle(dir)
char *dir;
{
  char *result = NULL;

  if (dir) {
    int len;
    char *home, *twiddlestr;
    twiddlestr = strdup(dir);
    demand(twiddlestr, "out of memory in expand_twiddle()");
    len = strcspn(twiddlestr, "/");
    if (len && twiddlestr[0] == '~') {
      twiddlestr[len] = '\0';
      if (len > 1) home = home_dir(twiddlestr+1);
      else home = home_dir(NULL);
      if (home) {
        if (len >= strlen(dir))
          result = strdup(home);
        else
          result = concat_filename(home, dir+len+1);
        free(home);
      }
      else result = NULL;
    }
    else {
      result = strdup(dir);
      demand(result, "out of memory in expand_twiddle()");
    }
    free(twiddlestr);
  }

  return result;
}



/* given a filename to try to open, look for it first in the current
   directory (just use the name, if it happens to be a full pathname it will
   work), and then in the user's home directory (the one who owns this
   process).  If it can be opened (for whichever purpose they ask) then do
   so and return the file pointer.  If it cannot be opened, then return
   NULL. */
FILE *open_local_file(filename, way)
char *filename, *way;	/* way is one of "r", "w", "a", etc. */
{
  char *name;
  FILE *result;

  if (!(result = fopen(filename, way))) {
    name = concat_filename(home_dir(NULL), filename);
    if (name) {
      result = fopen(name, way);
      free(name);
    }
  }
  return result;
}
E 1
