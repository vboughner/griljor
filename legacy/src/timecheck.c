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

static char *day[7] = {"Sunday:    ", "Monday:    ", "Tuesday:   ",
		       "Wednesday: ", "Thursday:  ", "Friday:    ",
		       "Saturday:  " };


/* Loads the time resistrictions into 7 element integer arrays given.
   The file should contain the seven start times for each day of the week
   and the seven end times for when you can play each day.  Should the file
   be found not to exist, game play will be allowed at all times.  Times
   are given as integers between 0 and 24 (24 hour clock).
   The first day in the file is considered to be Sunday. */

static void get_time_data(start, end)
int start[7], end[7];
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
    char buff[300];
    do {
      fgets(buff, 300, fp);
    } while (!feof(fp) && (buff[0] == '#' || strlen(buff)==0));
    sscanf(buff,"%d %d %d %d %d %d %d\n",
	 &start[0],
	 &start[1],
	 &start[2],
	 &start[3],
	 &start[4],
	 &start[5],
	 &start[6]);
    do {
      fgets(buff, 300, fp);
    } while (!feof(fp) && (buff[0] == '#' || strlen(buff)==0));
    sscanf(buff,"%d %d %d %d %d %d %d\n",
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



/* print out the schedule of times when this program IS available */
static void print_schedule(start, end)
int start[7], end[7];
{
  int i;
  char s[150];

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



/* if, on a given day (0 = sunday, 1 = monday, etc.) it is before the
 * hour given by start, or after the hour given by end, this procedure
 * will exit, with the proper error message.  Otherwise the procedure
 * will return. */
  
static void timecheck(start, end)
int start[7], end[7];
{
  struct tm *now;
  int temp;
  temp = time(NULL);
  now = localtime(&temp);
    

  if (!now) {
    fprintf(stderr,"Warning: system time not available\n");
    return;
  }

  if (start[now->tm_wday] < end[now->tm_wday]) {
    if ((now->tm_hour < start[now->tm_wday])||(now->tm_hour >= end[now->tm_wday])) {
      if (now->tm_hour < start[now->tm_wday]) {
        fprintf(stderr,
		"Sorry, this game cannot be accessed until %d00 hours.\n",
		start[now->tm_wday]);
	print_schedule(start, end);
        exit(8);
      }
      else {
        fprintf(stderr,
		"Sorry, this game cannot be accessed until %d00 hours tommorrow\n",
		start[(now->tm_wday +1) % 7]);
	print_schedule(start, end);
        exit(8);
      }
    }
  }
  else if (start[now->tm_wday] == end[now->tm_wday]) {
    fprintf(stderr,"Sorry, this game is not accessable today.\n");
    print_schedule(start, end);
    exit(8);
  }
  else {
    if ((now->tm_hour >= end[now->tm_wday]) && (now->tm_hour < start[now->tm_wday])) {
      fprintf(stderr,
	      "Sorry, this game cannot be accessed until %d00 hours.\n",
	      start[now->tm_wday]);
      print_schedule(start, end);
      exit(8);
    }
  }
}



/* loads time restrictions and returns only if game is allowed to be played */

int exit_upon_time_restriction()
{
  int go_time[7], stop_time[7];

  if (DEBUG) printf("Looking for timecheck file %s\n", TIME_FILE);
  get_time_data(go_time, stop_time);
  if (DEBUG) printf("Finished reading time file\n");
  timecheck(go_time, stop_time);
  if (DEBUG) printf("Finished checking times\n");
}
