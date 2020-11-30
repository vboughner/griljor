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
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gametime.h"
#include "level.h"
#include "math.h"

/* globals for this file only */

long	next_revival = 0;	/* next game time when revival takes place */
long	next_recharge = 0;	/* next game time when recharge takes place */
char	*rank_evaluation();

/* this needs to be called before starting another game */

reset_revivals()
{
  next_revival = 0;
  next_recharge = 0;
}

/* if it is time for another hit point or so, add one or more,
   also add to magic power if it is time to do so */

revive_persons()
{
  register int i;
  int bonus_mult;

  if (gametime >= next_revival) {
    /* if so then add health points to all persons */
    for (i=0; i<num_persons; i++) {

      if (person[i]->health < max_person_health(i)) {
        /* there is a bonus if player is not moving */
	bonus_mult = 1;
        if (!gameperson[i]->moveq) bonus_mult++;

        /* add the extra hit points he deserves */
        person[i]->health += bonus_mult;
        if (person[i]->health > max_person_health(i))
          person[i]->health = max_person_health(i);

        if (has_display && i == 0) redraw_player_health();
      }
    }

    if (gameperson[i])
      next_revival = gametime + 
	REVIVAL_TIME - gameperson[i]->level*REVIVE_BONUS_TIME;
  }

  if (gametime >= next_recharge) {
    /* if so then add magic points to all persons */
    for (i=0; i<num_persons; i++) {

      if (person[i]->power < max_person_power(i)) {
        /* there is a bonus if player is not moving */
	bonus_mult = 1;
        if (!gameperson[i]->moveq) bonus_mult++;

        /* add the extra power points he deserves */
        person[i]->power += bonus_mult;
        if (person[i]->power > max_person_power(i))
          person[i]->power = max_person_power(i);

        if (has_display && i == 0) redraw_player_magic();
      }
    }

    if (gameperson[i])
      next_recharge = 
	gametime + 
	  RECHARGE_TIME - gameperson[i]->level*RECHARGE_BONUS_TIME;
  }
}


/* add or subtract a given amount from one of my person's experience point
   totals.  If there is a display, update the exp pts. reading and deliver
   the given message */

alter_experience(pnum, amount, msg)
int pnum, amount;
char *msg;
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
    if (msg) player_error_out_message(msg);
  }

  /* check for a level change in this person, ie. a promotion */
  /* note that new health points are given only to those still alive */
  if (experience_level(person[pnum]->experience) > gameperson[pnum]->level &&
      gameperson[pnum]->level < LEVEL_LIMIT) {
    gameperson[pnum]->level = experience_level(person[pnum]->experience);
    if (person[pnum]->health > 0) person[pnum]->health += HEALTH_PER_LEVEL;
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
  /* note that new health points are given only to those still alive */
  else if (gameperson[pnum]->level <
	   (((average_player_level() + 1) * MIN_PERCENTAGE + 50) / 100)) {
	gameperson[pnum]->level++;
	if (person[pnum]->health > 0) person[pnum]->health += HEALTH_PER_LEVEL;
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

   /*  Rank calculations moved to the driver. */
/*    newrank = rank_evaluation(pnum);
    if (newrank) strcpy(gameperson[pnum]->rank, newrank); */


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

enforce_level_limits(pnum)
int pnum;
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

death_experience(pnum)
int pnum;
{
  int winner_amount, loser_penalty, level_diff;
  char msg[MSG_LENGTH];
  int i;
  double tot,this;

  AddtoDamageRecord(pnum,0,0);  /* equalize all damage records for the given
				 time */
  
  tot = 0.0;
  loser_penalty = 0;
  for (i = 0;i < persons_in_game;i++)
    tot += person[pnum]->damage.damage[i];
  
  if (tot > 0.0)
    for (i = 0;i <persons_in_game;i++)
      {
	/* figure out how many levels below me they are */
	level_diff = gameperson[pnum]->level - gameperson[i]->level;
	this = person[pnum]->damage.damage[i] / tot;

	/* figure out their total */
	winner_amount =
	  floor(this * (double) BASE_KILL_REWARD * 
		(int) pow(2.0, ((double) level_diff) *
			        (double) KILL_ADJUSTMENT) );

	/* if they are killing someone on their 
	   own team they lose experience */
	if (gameperson[pnum]->team == gameperson[i]->team &&
	    gameperson[pnum]->team != 0 &&
	    mapstats.teams_supported > 1)
	  winner_amount = -winner_amount;
	else
	  loser_penalty += winner_amount;

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

  sprintf(msg, "You lost %d points for being killed.",loser_penalty);
  alter_experience(pnum, -(loser_penalty), msg);
}




/* add experience points to each person's total for the end of the game.
   The amount awarded should be dependent on whether they where on the
   winning team, whether they won the game for their team, and whether
   they have been in the game very long. */

end_game_experience(winning_team, winner)
int winning_team, winner;
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

add_kill(pnum)
int pnum;
{
  gameperson[pnum]->kills++;
  if (has_display && pnum == 0) redraw_player_kills();

  /* need to call notify_of_stats? It is currently only being called
     at game end */
}


/* add a loss to person's total */

add_loss(pnum)
int pnum;
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
  if (gameperson[pnum]->team && mapstats.teams_supported > 1) 
    {
      x = (double) max(players_on_team(gameperson[pnum]->team), 1);
      y = (double) (players_in_game() -
		    players_on_team(gameperson[pnum]->team)) / x *
		      (double) RANKASP_TEAM;
      y = max(y, ((double) RANKASP_TEAM * 2.0));
      points = points + (int) y;
    }
  else 
    {
      y = (double) players_in_game() / (double) MIN_GAME_PLAYERS;
      y = max(y, ((double) RANKASP_TEAM * 2.0));
      points = points + (int) y;
    }

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
