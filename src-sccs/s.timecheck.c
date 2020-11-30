h59682
s 00000/00000/00168
d D 1.6 92/08/07 01:04:18 vbo 6 5
c source copied to a separate tree for work on new map and object format
e
s 00010/00005/00158
d D 1.5 91/08/26 00:35:12 vanb 5 4
c fixed up procedure defs and other compatibilty problems
e
s 00003/00000/00160
d D 1.4 91/03/30 22:24:43 labc-3id 4 3
c added print_schedule() and debugged it for good
e
s 00045/00017/00115
d D 1.3 91/03/30 19:53:01 labc-3id 3 2
c added print_schedule()
e
s 00009/00002/00123
d D 1.2 91/03/29 17:24:55 labc-3id 2 1
c 
e
s 00125/00000/00000
d D 1.1 91/02/16 12:56:06 labc-3id 1 0
c date and time created 91/02/16 12:56:06 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 **************************************************************************/

/* Routines to load and enforce game time restrictions */

#include <stdio.h>
#include <time.h>
#include "config.h"

#define FNAME	TIME_FILE

I 5
static char *day[7] = {"Sunday:    ", "Monday:    ", "Tuesday:   ",
		       "Wednesday: ", "Thursday:  ", "Friday:    ",
		       "Saturday:  " };
E 5

I 5

E 5
D 3
/* loads time restrictions and returns only if game is allowed to be played */

exit_upon_time_restriction()
{
  int go_time[7], stop_time[7];

  get_time_data(go_time, stop_time);
  timecheck(go_time, stop_time);
}



E 3
/* Loads the time resistrictions into 7 element integer arrays given.
   The file should contain the seven start times for each day of the week
   and the seven end times for when you can play each day.  Should the file
   be found not to exist, game play will be allowed at all times.  Times
   are given as integers between 0 and 24 (24 hour clock).
   The first day in the file is considered to be Sunday. */

D 3
get_time_data(int start[7], int end[7])
E 3
I 3
D 5
static void get_time_data(int start[7], int end[7])
E 5
I 5
static void get_time_data(start, end)
int start[7], end[7];
E 5
E 3
{
  int i;
  FILE *fp;

  /* look for time file */
  fp = fopen(FNAME,"r");
  if (!fp) {
	/* when there is no time file, all times are permissible */
	for (i=0; i<7; i++) {
	  start[i] = 0;
	  end[i] = 24;
	}
  }
  else {
D 2
    fscanf(fp,"%d %d %d %d %d %d %d\n",
E 2
I 2
    char buff[300];
    do {
      fgets(buff, 300, fp);
    } while (!feof(fp) && (buff[0] == '#' || strlen(buff)==0));
    sscanf(buff,"%d %d %d %d %d %d %d\n",
E 2
	 &start[0],
	 &start[1],
	 &start[2],
	 &start[3],
	 &start[4],
	 &start[5],
	 &start[6]);
D 2
    fscanf(fp,"%d %d %d %d %d %d %d\n",
E 2
I 2
    do {
      fgets(buff, 300, fp);
    } while (!feof(fp) && (buff[0] == '#' || strlen(buff)==0));
    sscanf(buff,"%d %d %d %d %d %d %d\n",
E 2
	 &end[0],
	 &end[1],
	 &end[2],
	 &end[3],
	 &end[4],
	 &end[5],
	 &end[6]);
    fclose(fp);
  }
}



I 3
/* print out the schedule of times when this program IS available */
D 5
static void print_schedule(int start[7], int end[7])
E 5
I 5
static void print_schedule(start, end)
int start[7], end[7];
E 5
{
  int i;
D 5
  char *day[] = {"Sunday:    ", "Monday:    ", "Tuesday:   ", "Wednesday: ", 
		 "Thursday:  ", "Friday:    ", "Saturday:  " };
E 5
  char s[150];
E 3

I 3
  fprintf(stderr, "\nRunning of this program limited on day of week basis:\n");
  for (i=0; i<7; i++) {
    if (end[i] == start[i])
      sprintf(s, "no access allowed at all");
    else if (start[i] == 0 && end[i] == 24)
      sprintf(s, "play allowed all day");
    else if (end[i] < start[i])
      sprintf(s, "play allowed until %d00 and again after %d00",
	      end[i], start[i]);
    else
      sprintf(s, "play allowed between %d00 and %d00", start[i], end[i]);

    fprintf(stderr, "%s %s\n", day[i], s);
  }
}



E 3
/* if, on a given day (0 = sunday, 1 = monday, etc.) it is before the
 * hour given by start, or after the hour given by end, this procedure
 * will exit, with the proper error message.  Otherwise the procedure
 * will return. */
  
D 3
int timecheck(int start[7], int end[7])
E 3
I 3
D 5
static void timecheck(int start[7], int end[7])
E 5
I 5
static void timecheck(start, end)
int start[7], end[7];
E 5
E 3
{
  struct tm *now;
  int temp;
  temp = time(NULL);
  now = localtime(&temp);
    

  if (!now) {
    fprintf(stderr,"Warning: system time not available\n");
D 3
    return 1;
E 3
I 3
    return;
E 3
  }

  if (start[now->tm_wday] < end[now->tm_wday]) {
    if ((now->tm_hour < start[now->tm_wday])||(now->tm_hour >= end[now->tm_wday])) {
      if (now->tm_hour < start[now->tm_wday]) {
        fprintf(stderr,
		"Sorry, this game cannot be accessed until %d00 hours.\n",
		start[now->tm_wday]);
I 3
	print_schedule(start, end);
E 3
        exit(8);
      }
      else {
        fprintf(stderr,
		"Sorry, this game cannot be accessed until %d00 hours tommorrow\n",
		start[(now->tm_wday +1) % 7]);
I 3
	print_schedule(start, end);
E 3
        exit(8);
      }
    }
  }
  else if (start[now->tm_wday] == end[now->tm_wday]) {
D 3
    fprintf(stderr,"Sorry, this game is not accessable today.  Sorry.\n");
E 3
I 3
    fprintf(stderr,"Sorry, this game is not accessable today.\n");
    print_schedule(start, end);
E 3
    exit(8);
  }
  else {
    if ((now->tm_hour >= end[now->tm_wday]) && (now->tm_hour < start[now->tm_wday])) {
      fprintf(stderr,
	      "Sorry, this game cannot be accessed until %d00 hours.\n",
	      start[now->tm_wday]);
I 3
      print_schedule(start, end);
E 3
      exit(8);
    }
  }
D 3
  return 0;
E 3
I 3
}



/* loads time restrictions and returns only if game is allowed to be played */

int exit_upon_time_restriction()
{
  int go_time[7], stop_time[7];

I 4
  if (DEBUG) printf("Looking for timecheck file %s\n", TIME_FILE);
E 4
  get_time_data(go_time, stop_time);
I 4
  if (DEBUG) printf("Finished reading time file\n");
E 4
  timecheck(go_time, stop_time);
I 4
  if (DEBUG) printf("Finished checking times\n");
E 4
E 3
}
E 1
