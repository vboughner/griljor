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

/* Routines for handling accounting of flags in the game.
   When your team has all of the flags it's supposed to have, you win. */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "lib.h"


/* types and variables for this file only */

/* for making a linked list of all the important objects in the game */
typedef struct _important {	/* flags are "important" items */
	int	roomnum, x, y;	/* the definitive location of this one */
	int	type;		/* it's object type number */
	struct _important *next;/* this is going to be a linked list */
} Important;


Important	*imp = NULL;	/* pointer to global linked list */
int		flags_to_get[NUM_OF_TEAMS];	/* how many to win */



/* team checking macros */
#define FOR_TEAM(teamnum, objnum)	((!info[(uc)(objnum)]->type) ||   \
					 ((info[(uc)(objnum)]->type) &    \
					  (1 << (teamnum - 1))))
#define IS_TEAM_ROOM(teamnum, roomnum)	((teamnum) == (room[(roomnum)].team))



/* add an important object to the list */

add_imp(roomnum, x, y, type)
int roomnum, x, y, type;
{
  Important *new;

  /* allocate a new object structure */
  demand(new = (Important *) malloc(sizeof(Important)),
	 "can't allocate an important object structure");

  /* place values into object */
  new->roomnum = roomnum;
  new->x = x;
  new->y = y;
  new->type = type;
  new->next = NULL;

  /* add object to the beginning of the list */
  if (imp) new->next = imp;
  imp = new;
}



/* delete on of the objects from the list */

delete_imp(roomnum, x, y, type)
int roomnum, x, y, type;
{
  Important *ptr, *last = NULL;
  int done;

  /* look for it, then delete it */
  for (ptr=imp,done=0; ptr && !done; ptr = ptr->next) {
    if (ptr->roomnum == roomnum && ptr->x == x &&
	ptr->y == y && ptr->type == type) {
		if (last) last->next = ptr->next;
		else imp = ptr->next;
		free(ptr);
		done = 1;
    }
    else last = ptr;
  }
}


/* free the entire list of important objects, deleting them all */

free_important_objects()
{
  Important *ptr, *each;

  for (ptr=imp; ptr; ) {
    each = ptr;
    ptr = ptr->next;
    free(each);
  }

  imp = NULL;
}


/* return the number of flags in the list that the given team needs to
   aquire, this is all of the flags, not just the ones they don't have yet.
   Team is given as a number between 0 and 4 */

int count_all_flags(team)
int team;
{
  Important *ptr;
  int result = 0;

  for (ptr=imp; ptr; ptr = ptr->next)
    if (FOR_TEAM(team, ptr->type)) result++;

  return result;
}



/* return the number of flags this team has aquired (placed into their own
   rooms).  The game will be ending when number aquired is equal to or
   greater than the number the driver said there was to get. */

int count_aquired_flags(team)
int team;
{
  Important *ptr;
  int result = 0;

  for (ptr=imp; ptr; ptr = ptr->next)
    if (FOR_TEAM(team, ptr->type))
      if (IS_TEAM_ROOM(team, ptr->roomnum)) result++;

  return result;
}



/* ======================= M A I N  routines ============================= */

/* initialize the important object list, by going through entire map and
   the linked list of all of them */

initialize_important_object_list()
{
  int r, x, y;
  SquareRec *square;
  ObjectInstance *o;

  /* make sure list starts as empty */
  imp = NULL;

  /* go through each room and add flags */
  for (r=0; r<mapstats.rooms; r++) {

      /* look on each square for flag objects */
      for (x=0; x<ROOM_WIDTH; x++)
        for (y=0; y<ROOM_HEIGHT; y++) {
	  square = get_square(&mapstats, r, x, y);
	  for (o = square->first; o; o = o->next)
	    if (info[(uc)o->type]->flag) add_imp(r, x, y, o->type);
	}
    }
}



/* If you are the driver, you will call this routine to discover how
   many flags each team has to aquire to win the game */

count_all_game_flags()
{
  int team, r, x, y;
  SquareRec *square;
  ObjectInstance *o;

  /* clear all teams in game */
  for (team = 1; team <= mapstats.teams_supported; team++)
    flags_to_get[team - 1] = 0;

  /* go through each room and count flags */
  for (r=0; r<mapstats.rooms; r++) {

      /* look on each square for flag objects */
      for (x=0; x<ROOM_WIDTH; x++)
        for (y=0; y<ROOM_HEIGHT; y++) {
	  square = get_square(&mapstats, r, x, y);
	  for (o = square->first; o; o = o->next)
	    if (info[(uc)o->type]->flag) {
	      /* add one to each team's total, for those that need this one */
	      for (team = 1; team <= mapstats.teams_supported; team++)
	        if (FOR_TEAM(team, o->type)) flags_to_get[team - 1]++;
	    }
	}
    }

  /* report how many flags each team must get */
  if (DEBUG) for (team = 1; team <= mapstats.teams_supported; team++)
    printf("Team %d must aquire %d flags to win\n",
	   team, flags_to_get[team - 1]);
}



/* If you are a player, you will set you own team's requirement for how many
   flags must be aquired */

set_flag_requirement(team, num)
int team, num;
{
  flags_to_get[team - 1] = num;
  if (DEBUG)
    printf("Setting that team %d must have %d flags to win\n", team, num);
}



/* return the number of flags the given team must aquire, according to
   the numbers calculated earlier with count_all_game_flags() */

int get_flag_requirement(team)
int team;
{
  return flags_to_get[team - 1];
}



/* If you are a player, you call this routine to see if your team has won
   the game (ie. your team has aquired the correct number of flags to
   win).  TRUE is returned if you have won, otherwise FALSE is returned. */

int team_has_won(team)
int team;
{
  return (flags_to_get[team - 1] <= count_aquired_flags(team));
}



/* register a change on a square.  If an important object is being removed
   or added, reflect change in important object list */

flag_account_map_change(roomnum, x, y, old, new)
int roomnum, x, y, old, new;
{
  Important *ptr;

  /* if two types are same then what is the use */
  if (old == new) return;

  /* if new important object is being added then do it */
  if (info[(uc)new]->flag)
    add_imp(roomnum, x, y, new);

  /* if old important object is being removed the remove it from the list */
  if (info[(uc)old]->flag)
    delete_imp(roomnum, x, y, old);
}



/* look for a winning condition for any team, and end the game
   if you find one.  If the game is won, I must have done it. */

look_for_wins()
{
  int i, won = -1, winner = -1;

  for (i=1; (i<=mapstats.teams_supported && won == -1); i++)
    if (team_has_won(i)) {
      /* if I made a team win, announce that fact to all */
      won = i;						/* winning team */
      if (!am_driver) winner = gameperson[0]->id;	/* current person */
      notify_of_game_over(winner, won);
    }
}
