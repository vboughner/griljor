h08986
s 00000/00000/00580
d D 1.10 92/08/07 01:01:59 vbo 10 9
c source copied to a separate tree for work on new map and object format
e
s 00001/00001/00579
d D 1.9 91/12/07 17:37:21 labc-4lc 9 8
c made error message window bigger and scrollable
e
s 00002/00001/00578
d D 1.8 91/10/16 20:05:34 labc-4lc 8 7
c changed made for DecStations
e
s 00006/00002/00573
d D 1.7 91/09/08 21:56:31 labc-4lc 7 6
c Removed ranking system and moved to driver.
e
s 00045/00054/00530
d D 1.6 91/09/08 21:28:16 labc-4lc 6 5
c Adjusted code for awarding experience points.
e
s 00100/00049/00484
d D 1.5 91/09/03 21:38:50 labb-3li 5 4
c Trevor's misc changes - fixed missile heap
e
s 00013/00006/00520
d D 1.4 91/08/29 01:40:09 vanb 4 3
c fixed up more compatibility problems
e
s 00001/00001/00525
d D 1.3 91/08/04 21:34:51 labc-3id 3 2
c added code for many of the limitation properties
e
s 00024/00003/00502
d D 1.2 91/05/10 04:46:40 labc-3id 2 1
c fixed level death bug and no heal bug
e
s 00505/00000/00000
d D 1.1 91/02/16 12:54:33 labc-3id 1 0
c date and time created 91/02/16 12:54:33 by labc-3id
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

/* Routines for handling of experience points and levels */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "lib.h"
#include "map.h"
#include "objects.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gametime.h"
#include "level.h"
I 5
#include "math.h"
E 5

D 5

E 5
/* globals for this file only */

long	next_revival = 0;	/* next game time when revival takes place */
long	next_recharge = 0;	/* next game time when recharge takes place */
char	*rank_evaluation();

I 2
D 6

E 6
/* this needs to be called before starting another game */

reset_revivals()
{
  next_revival = 0;
  next_recharge = 0;
}

D 6


E 6
E 2
/* if it is time for another hit point or so, add one or more,
   also add to magic power if it is time to do so */

revive_persons()
{
  register int i;
D 2
  int bonus_mult = 1;	/* bonus multiplier */
E 2
I 2
  int bonus_mult;
E 2

  if (gametime >= next_revival) {
    /* if so then add health points to all persons */
    for (i=0; i<num_persons; i++) {
I 2

E 2
      if (person[i]->health < max_person_health(i)) {
        /* there is a bonus if player is not moving */
I 2
	bonus_mult = 1;
E 2
        if (!gameperson[i]->moveq) bonus_mult++;
I 2

E 2
        /* add the extra hit points he deserves */
D 5
        person[i]->health +=
          1 + bonus_mult * (gameperson[i]->level / REVIVE_BONUS_LEVEL);
E 5
I 5
        person[i]->health += bonus_mult;
E 5
        if (person[i]->health > max_person_health(i))
          person[i]->health = max_person_health(i);
I 2

E 2
        if (has_display && i == 0) redraw_player_health();
      }
    }
D 5
    next_revival = gametime + REVIVAL_TIME;
E 5
I 5

    if (gameperson[i])
D 6
      {
	next_revival = 
	  max(gametime,
	      gametime + REVIVAL_TIME - 
	      gameperson[i]->level*REVIVE_BONUS_TIME);
      }
    else
      next_revival = gametime;
    
E 6
I 6
      next_revival = gametime + 
	REVIVAL_TIME - gameperson[i]->level*REVIVE_BONUS_TIME;
E 6
E 5
  }

  if (gametime >= next_recharge) {
    /* if so then add magic points to all persons */
    for (i=0; i<num_persons; i++) {
I 2

E 2
      if (person[i]->power < max_person_power(i)) {
        /* there is a bonus if player is not moving */
I 2
	bonus_mult = 1;
E 2
        if (!gameperson[i]->moveq) bonus_mult++;
I 2

E 2
        /* add the extra power points he deserves */
D 5
        person[i]->power +=
          1 + bonus_mult * (gameperson[i]->level / RECHARGE_BONUS_LEVEL);
E 5
I 5
        person[i]->power += bonus_mult;
E 5
        if (person[i]->power > max_person_power(i))
          person[i]->power = max_person_power(i);
I 2

E 2
        if (has_display && i == 0) redraw_player_magic();
      }
    }
D 5
    next_recharge = gametime + RECHARGE_TIME;
E 5
I 5

    if (gameperson[i])
D 6
      {
	next_recharge = 
	  max(gametime,
	      gametime + RECHARGE_TIME -
	      gameperson[i]->level*RECHARGE_BONUS_TIME);
      }
    else
      next_recharge = gametime;
E 6
I 6
      next_recharge = 
	gametime + 
	  RECHARGE_TIME - gameperson[i]->level*RECHARGE_BONUS_TIME;
E 6
E 5
  }
}


D 6

E 6
/* add or subtract a given amount from one of my person's experience point
   totals.  If there is a display, update the exp pts. reading and deliver
   the given message */

D 4
alter_experience(int pnum, int amount, char *msg)
E 4
I 4
alter_experience(pnum, amount, msg)
int pnum, amount;
char *msg;
E 4
{
  char s[MSG_LENGTH];
  int level_change = 0;

  /* check that person to add it to is appropriate */
  if (pnum < 0 || pnum >= num_persons) {
    printf("Warning: trying to alter experience for non-existant person\n");
    return;
  }

  /* add the experience points */
  person[pnum]->experience += amount;
  if (person[pnum]->experience < 0) person[pnum]->experience = 0;

  /* if the player's points have gone below his level amount and demotions
     are not currently allowed, then restore his points back to the
     minimum allowed for his level */
  if (!(DEMOTION_ALLOWED)) {
    if (person[pnum]->experience < (BASE_EXP + (gameperson[pnum]->level - 1) *
				    EXP_PER_LEVEL))
	person[pnum]->experience = (BASE_EXP + (gameperson[pnum]->level - 1) *
				    EXP_PER_LEVEL);
  }

  /* if there is a display for this person, print message and update */
  if (has_display && pnum == 0) {
    redraw_player_experience();
D 3
    redraw_error_window(msg);
E 3
I 3
D 9
    if (msg) redraw_error_window(msg);
E 9
I 9
    if (msg) player_error_out_message(msg);
E 9
E 3
  }

  /* check for a level change in this person, ie. a promotion */
I 2
  /* note that new health points are given only to those still alive */
E 2
  if (experience_level(person[pnum]->experience) > gameperson[pnum]->level &&
      gameperson[pnum]->level < LEVEL_LIMIT) {
    gameperson[pnum]->level = experience_level(person[pnum]->experience);
D 2
    person[pnum]->health += HEALTH_PER_LEVEL;
E 2
I 2
    if (person[pnum]->health > 0) person[pnum]->health += HEALTH_PER_LEVEL;
E 2
    sprintf(s, "%s->ALL %s (%d) has been promoted to level %d", GM_NAME,
	    gameperson[pnum]->name, gameperson[pnum]->id,
	    gameperson[pnum]->level);
    level_change = TRUE;
  }
  /* check for a possible demotion, if demotions are active */
  else if ((person[pnum]->experience <
	    (BASE_EXP + ((gameperson[pnum]->level - 1) * EXP_PER_LEVEL) -
	     LEEWAY_POINTS)) && (DEMOTION_ALLOWED)) {
    gameperson[pnum]->level = experience_level(person[pnum]->experience);
    sprintf(s, "%s->ALL %s (%d) has been demomoted to level %d", GM_NAME,
	    gameperson[pnum]->name, gameperson[pnum]->id,
	    gameperson[pnum]->level);
    level_change = TRUE;
  }
  /* give mercy levels to those who can't keep up with average level */
I 2
  /* note that new health points are given only to those still alive */
E 2
  else if (gameperson[pnum]->level <
	   (((average_player_level() + 1) * MIN_PERCENTAGE + 50) / 100)) {
	gameperson[pnum]->level++;
D 2
	person[pnum]->health += HEALTH_PER_LEVEL;
E 2
I 2
	if (person[pnum]->health > 0) person[pnum]->health += HEALTH_PER_LEVEL;
E 2
	person[pnum]->experience = BASE_EXP + 
		(gameperson[pnum]->level - 1) * EXP_PER_LEVEL;
	sprintf(s, "%s->ALL %s (%d) has been charitied to level %d",
		GM_NAME, gameperson[pnum]->name, gameperson[pnum]->id,
		gameperson[pnum]->level);
	level_change = TRUE;
  }

  /* if there was a level change, then make changes and let everyone know */
  if (level_change) {
    char *newrank;
    if (gameperson[pnum]->listed) notify_of_message(FROM_GM, "all", s);
    enforce_level_limits(pnum);
D 7
    newrank = rank_evaluation(pnum);
    if (newrank) strcpy(gameperson[pnum]->rank, newrank);
E 7
I 7

   /*  Rank calculations moved to the driver. */
/*    newrank = rank_evaluation(pnum);
    if (newrank) strcpy(gameperson[pnum]->rank, newrank); */


E 7
    notify_of_stats(pnum);
    if (has_display && pnum == 0) {
      redraw_player_stat_window();
      player_out_message(s);
    }
  }
}



/* given a person number, enforce the level limits for his level.  That is,
   if he has more of a given thing than he should drop it down to
   acceptable level */

D 4
enforce_level_limits(int pnum)
E 4
I 4
enforce_level_limits(pnum)
int pnum;
E 4
{
  if (person[pnum]->health > max_person_health(pnum))
    person[pnum]->health = max_person_health(pnum);

  if (person[pnum]->power > max_person_power(pnum))
    person[pnum]->power = max_person_power(pnum);

  gameperson[pnum]->weight_limit = max_person_burden(pnum);
}



/* find the current average of all player levels in the game and return it */

int average_player_level()
{
  int i, num=0, sum=0, result=1;

  for (i=0; i<persons_in_game; i++) {
    if (gameperson[i]->listed) {
	num++;
	sum = sum + gameperson[i]->level;
    }
  }

  if (num > 0) result = sum / num;
  return result;
}



/* take care of all experience point changes when someone kills me.
   ie.  increase their total by sending them an ADD_EXPERIENCE packet,
   and decrease my own because I lost the fight. */

D 4
death_experience(int pnum, int killer_num)
E 4
I 4
D 5
death_experience(pnum, killer_num)
int pnum, killer_num;
E 5
I 5
death_experience(pnum)
int pnum;
E 5
E 4
{
  int winner_amount, loser_penalty, level_diff;
  char msg[MSG_LENGTH];
I 5
  int i;
  double tot,this;
E 5

D 5
  /* figure out how many levels below me they are */
  level_diff = gameperson[pnum]->level - gameperson[killer_num]->level;
E 5
I 5
  AddtoDamageRecord(pnum,0,0);  /* equalize all damage records for the given
				 time */
E 5
  
D 5
  /* figure out their total */
  winner_amount = BASE_KILL_REWARD + (KILL_ADJUSTMENT * level_diff);
E 5
I 5
  tot = 0.0;
  loser_penalty = 0;
  for (i = 0;i < persons_in_game;i++)
    tot += person[pnum]->damage.damage[i];
  
D 6
  for (i = 0;i <persons_in_game;i++)
    {
      /* figure out how many levels below me they are */
      level_diff = gameperson[pnum]->level - gameperson[i]->level;
      this = person[pnum]->damage.damage[i] / tot;
E 6
I 6
  if (tot > 0.0)
    for (i = 0;i <persons_in_game;i++)
      {
	/* figure out how many levels below me they are */
	level_diff = gameperson[pnum]->level - gameperson[i]->level;
	this = person[pnum]->damage.damage[i] / tot;
E 6
E 5

D 5
  /* if they are killing someone on their own team they get nothing */
  if ((gameperson[pnum]->team == gameperson[killer_num]->team &&
      gameperson[pnum]->team != 0 &&
      mapstats.teams_supported > 1) ||
      pnum == killer_num)	winner_amount = 0;
E 5
I 5
D 6
      /* figure out their total */
      winner_amount =
	floor(this * BASE_KILL_REWARD * 
	      exp2(((double) level_diff) * KILL_ADJUSTMENT) );
E 6
I 6
	/* figure out their total */
	winner_amount =
	  floor(this * (double) BASE_KILL_REWARD * 
D 8
		exp2(((double) level_diff) * KILL_ADJUSTMENT) );
E 8
I 8
		(int) pow(2.0, ((double) level_diff) *
			        (double) KILL_ADJUSTMENT) );
E 8
E 6

D 6
      /* if they are killing someone on their own team they lose experience */
      if ((gameperson[pnum]->team == gameperson[i]->team &&
	   gameperson[pnum]->team != 0 &&
	   mapstats.teams_supported > 1) ||
	  pnum == i)	
	winner_amount = -winner_amount;
E 5
      
D 5
  if (winner_amount <= 0) {
    winner_amount = 0;
    sprintf(msg, "You received no points for killing %s (%d)",
	    gameperson[pnum]->name, gameperson[pnum]->id);
  }
  else {
    sprintf(msg, "You received %d points for killing %s (%d)",
	    winner_amount, gameperson[pnum]->name, gameperson[pnum]->id);
  }
E 5
I 5
      loser_penalty += winner_amount;
      if (winner_amount != 0)
	{
	  sprintf(msg, "You received %d points for the death of %s (%d)",
		  winner_amount, gameperson[pnum]->name, gameperson[pnum]->id);
E 6
I 6
	/* if they are killing someone on their 
	   own team they lose experience */
	if (gameperson[pnum]->team == gameperson[i]->team &&
	    gameperson[pnum]->team != 0 &&
	    mapstats.teams_supported > 1)
	  winner_amount = -winner_amount;
	else
	  loser_penalty += winner_amount;
E 6
E 5

D 5
  /* let winner know about his experience point addition */
  notify_of_experience(gameperson[pnum]->id, gameperson[killer_num]->id,
		       winner_amount, msg);
E 5
I 5
D 6
	  /* let winner know about his experience point addition */
	  notify_of_experience(gameperson[pnum]->id,
			       gameperson[i]->id,
			       winner_amount, msg);
	}
      
    }
E 6
I 6
	if (pnum == i)
	  winner_amount = 0;
	
	if (winner_amount != 0)
	  {
	    sprintf(msg, "You received %d points for the death of %s (%d)",
		    winner_amount, gameperson[pnum]->name,
		    gameperson[pnum]->id);
	    
	    /* let winner know about his experience point addition */
	    notify_of_experience(gameperson[pnum]->id,
				 gameperson[i]->id,
				 winner_amount, msg);
	  }
      }
E 6
E 5

D 5
  /* figure out your penalty for being killed */
  loser_penalty = BASE_DIE_PENALTY + (DIE_ADJUSTMENT * level_diff);

  /* if they were killed by someone on their own team they lose nothing */
  if ((gameperson[pnum]->team == gameperson[killer_num]->team &&
      gameperson[pnum]->team != 0 &&
      mapstats.teams_supported > 1) ||
      pnum == killer_num)	loser_penalty = 0;

  if (loser_penalty <= 0) {
    loser_penalty = 0;
    sprintf(msg, "You receive no penalty for being killed by %s (%d)",
	    gameperson[killer_num]->name, gameperson[killer_num]->id);
  }
  else {
    sprintf(msg, "You received a %d point penalty when killed by %s (%d)",
	    loser_penalty, gameperson[killer_num]->name,
	    gameperson[killer_num]->id);
  }
E 5
I 5
  sprintf(msg, "You lost %d points for being killed.",loser_penalty);
E 5
  alter_experience(pnum, -(loser_penalty), msg);
}




/* add experience points to each person's total for the end of the game.
   The amount awarded should be dependent on whether they where on the
   winning team, whether they won the game for their team, and whether
   they have been in the game very long. */

D 4
end_game_experience(int winning_team, int winner)
E 4
I 4
end_game_experience(winning_team, winner)
int winning_team, winner;
E 4
{
  register int i;
  int amount;
  double time_percentage, people_percentage;
  char msg[MSG_LENGTH];

  /* figure out percentage of points you'll get based on time in game */
  time_percentage = ((double) (gametime / CLICKS_PER_SEC)) /
		    ((double) (REQUIRED_TIME));

  /* figure out percentage of points you'll get based on people in game */
  people_percentage = ((double) players_in_game()) /
		      ((double) REQUIRED_PEOPLE);

  /* add experience to each person */
  for (i=0; i<num_persons; i++) {
    amount = BASE_GAME_REWARD;
    if (winning_team == gameperson[i]->team) amount += BASE_TEAM_REWARD;
    if (winner == gameperson[i]->id) amount += BASE_WINNER_REWARD;

    if (winner == gameperson[i]->id && winning_team != gameperson[i]->team &&
      gameperson[i]->team != 0) {
      sprintf(msg, "You lose %d points for winning for the wrong team",
	      amount);
      amount = -amount;
    }
    else {
      amount = (int) ((double) amount * time_percentage);
      sprintf(msg, "You gained %d experience points for that game", amount);
    }
    /* if this person won the game then credit his game win count */
    if (gameperson[i]->id == winner) add_game_win(i);
    alter_experience(i, amount, msg);
  }
}




/* add a kill to person's total */

D 4
add_kill(int pnum)
E 4
I 4
add_kill(pnum)
int pnum;
E 4
{
  gameperson[pnum]->kills++;
  if (has_display && pnum == 0) redraw_player_kills();

  /* need to call notify_of_stats? It is currently only being called
     at game end */
}


/* add a loss to person's total */

D 4
add_loss(int pnum)
E 4
I 4
add_loss(pnum)
int pnum;
E 4
{
  gameperson[pnum]->losses++;
  if (has_display && pnum == 0) redraw_player_kills();

  /* need to call notify_of_stats? It is currently only being called
     at game end */
}


/* add a game win to player's total */

add_game_win(pnum)
{
  gameperson[pnum]->games++;
  if (has_display && pnum == 0) redraw_player_kills();

  /* need to call notify_of_stats? It is currently only being called
     at game end */
}



/* reset a player's stats to the initial values */

reset_player_stats(pnum)
{
  char s[80];

  strcpy(gameperson[pnum]->rank, RANK_NAME[0]);
  gameperson[pnum]->level = experience_level(BASE_EXP);
  gameperson[pnum]->kills = 0;
  gameperson[pnum]->losses = 0;
  gameperson[pnum]->games = 0;
  gameperson[pnum]->weight_limit = MAX_BURDEN;

  /* assign exp points */
  if (num_persons > pnum) {
    person[pnum]->experience = BASE_EXP;
    if (gameperson[pnum]->listed) {
      sprintf(s, "%s->ALL %s (%d) has reset player statistics", GM_NAME,
	      gameperson[pnum]->name, gameperson[pnum]->id);
      notify_of_message(FROM_GM, "all", s);
    }
    enforce_level_limits(pnum);
    notify_of_stats(pnum);
    if (has_display && pnum == 0) {
        redraw_player_stat_window();
        player_out_message(s);
    }
  }

}



/* return a pointer to read-only memory containing the rank name
   referred to by the given number.  If the number is too large or too
   small then give the highest or lowest rank respectively. */

char *rankname_by_num(num)
int num;
{
  if (num < 1) return RANK_NAME[0];
  else if (num > NUM_OF_RANKS) return RANK_NAME[NUM_OF_RANKS-1];
  else return RANK_NAME[num-1];
}



/* return the rank number that in referred to by a certain number of
   rank evaluation points (0-100) */

int ranknum_by_points(points)
int points;
{
  double result;

  if (points <= BEGINNER_RUT) result = 1.0;
  else if (points > 100) result = (double) NUM_OF_RANKS;
  else {
    result = ((((double) points - (double) BEGINNER_RUT) * NUM_OF_RANKS + 1) /
	      (max((100.0 - (double) BEGINNER_RUT), 1.0)));
  }
  
  return (int) result;
}



/* Evaluate the rank a player deserves and return a pointer to a read-only
   string containing the new rank.  Or return NULL if rank cannot or should
   not be re-evaluated at this time. */

char *rank_evaluation(pnum)
int pnum;
{
  int i, j, points = 0;
  double x, y;
  char *result;

  /* don't evaluate the rank of someone we dont have info for! */
  if (pnum >= num_persons) {
    printf("Warning: tried to evaluate rank of someone else\n");
    return NULL;
  }

  /* rank is only to be evaluated under certain conditions */
  if ((gametime < MIN_GAME_TIME) ||
      (players_in_game() < MIN_GAME_PLAYERS) ||
      (gameperson[pnum]->level < MIN_PLAYER_LEVEL)) return NULL;

  /* rank is easy to evaluate under certain conditions */
  if (DEITY_FLAGGED && gameperson[pnum]->deity) 
    return(DEITY_RANK_NAME);

  /* at this point lets start computing points toward rank,
     beginning with points accrued due to the player's level */
  x = (double) max(average_player_level(), 1);
  y = (gameperson[pnum]->level) / x * (double) RANKASP_LEVEL;
  y = max(y, ((double) RANKASP_LEVEL * 2.0));
  points = points + (int) y;

  /* now let's add points for their kill ratio */
  x = (double) max(gameperson[pnum]->losses, 1);
  y = (double) gameperson[pnum]->kills / x * (double) RANKASP_KILLS;
  y = max(y, ((double) RANKASP_KILLS * 2.0));
  points = points + (int) y;

  /* now give some points for unfair teams */
D 6
  if (gameperson[pnum]->team && mapstats.teams_supported > 1) {
E 6
I 6
  if (gameperson[pnum]->team && mapstats.teams_supported > 1) 
    {
E 6
      x = (double) max(players_on_team(gameperson[pnum]->team), 1);
      y = (double) (players_in_game() -
		    players_on_team(gameperson[pnum]->team)) / x *
D 6
	  (double) RANKASP_TEAM;
E 6
I 6
		      (double) RANKASP_TEAM;
E 6
      y = max(y, ((double) RANKASP_TEAM * 2.0));
      points = points + (int) y;
D 6
  }
  else {
E 6
I 6
    }
  else 
    {
E 6
      y = (double) players_in_game() / (double) MIN_GAME_PLAYERS;
      y = max(y, ((double) RANKASP_TEAM * 2.0));
      points = points + (int) y;
D 6
  }
E 6
I 6
    }
E 6

  /* and some points if they've won a lot of games */
  x = (double) max(GAME_COMPARISON, 1);
  y = (double) gameperson[pnum]->games / x * (double) RANKASP_GAMES;
  y = max(y, ((double) RANKASP_GAMES * 2.0));
  points = points + (int) y;

  /* now normalize their total points number so that it is between
     zero and one-hundred  */
  points = points * 100 / 200;

  /* compare their current rank with the calculated rank from this
     number and give them a promotion or demotion of one setting.
     Demotions are only given when the calculated level is 2 below their
     current level */
  if (ranknum_by_points(points) > ranknum_by_name(gameperson[pnum]->rank))
    result = rankname_by_num(ranknum_by_name(gameperson[pnum]->rank) + 1);
  else if (ranknum_by_points(points) <
	   ranknum_by_name(gameperson[pnum]->rank) - 1)
    result = rankname_by_num(ranknum_by_name(gameperson[pnum]->rank) - 1);
  else result = rankname_by_num(ranknum_by_points(points));

  return result;
}



/* determine the rank number (1 - NUM_OF_RANKS) given the name of the rank,
   or return 1 if the rank name cannot be found in the list */

int ranknum_by_name(name)
char *name;
{
  int i, result = 1, found = 0;

  for (i=0; (i<NUM_OF_RANKS && !found); i++)
    if (!strcmp(name, RANK_NAME[i])) {
      result = i + 1;
      found = 1;
    }

  return result;
}
I 5

/*****************************************************************/

ClearDamageRecord(pnum)
int pnum;
{
  int i;

  for (i = 0;i < MAX_GAME_SIZE;i++)
    person[pnum]->damage.damage[i] = 0.0;
}

/*****************************************************************/

AddtoDamageRecord(pnum,value,who)
int pnum,value,who;
{
  int timediff,rtime,i;
  double a;

  timediff = gametime - person[pnum]->damage.when;
  if (timediff >= 0)
    {
      rtime = (REVIVAL_TIME - gameperson[pnum]->level*REVIVE_BONUS_TIME);
      a = ((double) timediff)/((double) rtime) * 2.0;
      
      for (i = 0;i < MAX_GAME_SIZE;i++)
	person[pnum]->damage.damage[i] = 
	  max(0.0,(person[pnum]->damage.damage[i]-a));
    }

  person[pnum]->damage.when = gametime;
  person[pnum]->damage.damage[who] += (double) value;
}

/*****************************************************************/
E 5
E 1
