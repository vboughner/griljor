h50680
s 00019/00020/00270
d D 1.5 92/08/28 11:58:27 vbo 5 4
c changed manner of looking through map squares
e
s 00000/00000/00290
d D 1.4 92/08/07 01:01:19 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00008/00004/00282
d D 1.3 91/08/26 00:33:25 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00007/00008/00279
d D 1.2 91/07/07 20:02:05 labc-3id 2 1
c took out references to old target property flag
e
s 00287/00000/00000
d D 1.1 91/02/16 12:54:08 labc-3id 1 0
c date and time created 91/02/16 12:54:08 by labc-3id
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

D 2
/* Routines for handling accounting of flags and targets in the game.
   When your team has all of the flags it's supposed to have, and you've
   destroyed all of your assigned targets, you win. */
E 2
I 2
/* Routines for handling accounting of flags in the game.
   When your team has all of the flags it's supposed to have, you win. */
E 2

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
D 2
typedef struct _important {	/* flags and targets are "important" items */
E 2
I 2
typedef struct _important {	/* flags are "important" items */
E 2
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

D 3
int count_all_flags(int team)
E 3
I 3
int count_all_flags(team)
int team;
E 3
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

D 3
int count_aquired_flags(int team)
E 3
I 3
int count_aquired_flags(team)
int team;
E 3
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
D 5
  int r, x, y, z, type;
  MemObj *ptr;
E 5
I 5
  int r, x, y;
  SquareRec *square;
  ObjectInstance *o;
E 5

  /* make sure list starts as empty */
  imp = NULL;

  /* go through each room and add flags */
  for (r=0; r<mapstats.rooms; r++) {

      /* look on each square for flag objects */
      for (x=0; x<ROOM_WIDTH; x++)
D 5
        for (y=0; y<ROOM_HEIGHT; y++)
	  for (z=0; z<ROOM_DEPTH; z++) {
	    type = room[r].spot[x][y][z];
D 2
	    if (info[(uc)type]->flag || info[(uc)type]->target) {
	      /* add this flag or target to game list */
E 2
I 2
	    if (info[(uc)type]->flag) {
	      /* add this flag to game list */
E 2
	      add_imp(r, x, y, type);
	    }
	  }
  }
E 5
I 5
        for (y=0; y<ROOM_HEIGHT; y++) {
	  square = get_square(&mapstats, r, x, y);
	  for (o = square->first; o; o = o->next)
	    if (info[(uc)o->type]->flag) add_imp(r, x, y, o->type);
	}
    }
E 5
}



/* If you are the driver, you will call this routine to discover how
   many flags each team has to aquire to win the game */

count_all_game_flags()
{
D 5
  int team, r, x, y, z, type;
  MemObj *ptr;
E 5
I 5
  int team, r, x, y;
  SquareRec *square;
  ObjectInstance *o;
E 5

  /* clear all teams in game */
  for (team = 1; team <= mapstats.teams_supported; team++)
    flags_to_get[team - 1] = 0;

  /* go through each room and count flags */
  for (r=0; r<mapstats.rooms; r++) {

      /* look on each square for flag objects */
      for (x=0; x<ROOM_WIDTH; x++)
D 5
        for (y=0; y<ROOM_HEIGHT; y++)
	  for (z=0; z<ROOM_DEPTH; z++) {
	    type = room[r].spot[x][y][z];
	    if (info[type]->flag) {
E 5
I 5
        for (y=0; y<ROOM_HEIGHT; y++) {
	  square = get_square(&mapstats, r, x, y);
	  for (o = square->first; o; o = o->next)
	    if (info[(uc)o->type]->flag) {
E 5
	      /* add one to each team's total, for those that need this one */
	      for (team = 1; team <= mapstats.teams_supported; team++)
D 5
	        if (FOR_TEAM(team, type)) flags_to_get[team - 1]++;
E 5
I 5
	        if (FOR_TEAM(team, o->type)) flags_to_get[team - 1]++;
E 5
	    }
D 5
	  }
  }
E 5
I 5
	}
    }
E 5

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

D 3
int get_flag_requirement(int team)
E 3
I 3
int get_flag_requirement(team)
int team;
E 3
{
  return flags_to_get[team - 1];
}



/* If you are a player, you call this routine to see if your team has won
   the game (ie. your team has aquired the correct number of flags to
   win).  TRUE is returned if you have won, otherwise FALSE is returned. */

D 3
int team_has_won(int team)
E 3
I 3
int team_has_won(team)
int team;
E 3
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
D 2
  if (info[(uc)new]->flag || info[(uc)new]->target)
E 2
I 2
  if (info[(uc)new]->flag)
E 2
    add_imp(roomnum, x, y, new);

  /* if old important object is being removed the remove it from the list */
D 2
  if (info[(uc)old]->flag || info[(uc)old]->target)
E 2
I 2
  if (info[(uc)old]->flag)
E 2
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
E 1
