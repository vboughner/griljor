h32617
s 00017/00007/01020
d D 1.17 92/09/05 01:22:24 vbo 17 16
c fixed some problems with object pointer no being NULL sometimes
e
s 00087/00096/00940
d D 1.16 92/09/02 17:18:44 vbo 16 15
c changed InvObj and MemObj to ObjectInstances, removed magic use
e
s 00000/00000/01036
d D 1.15 92/08/07 01:03:17 vbo 15 14
c source copied to a separate tree for work on new map and object format
e
s 00135/00153/00901
d D 1.14 91/12/17 19:32:56 labc-4lc 14 13
c made some procedures static
e
s 00001/00001/01053
d D 1.13 91/12/16 18:37:17 labc-4lc 13 12
c spelling error
e
s 00118/00021/00936
d D 1.12 91/12/16 17:55:02 labc-4lc 12 11
c added error messages for limitations
e
s 00000/00030/00957
d D 1.11 91/12/16 16:23:38 labc-4lc 11 10
c removed altering object messages and auto-actor stuff
e
s 00002/00000/00985
d D 1.10 91/12/16 15:59:38 labc-4lc 10 9
c jove multifire.c
e
s 00004/00004/00981
d D 1.9 91/12/15 23:06:59 labc-4lc 9 8
c Fixing datagram incompatibilities.   -bloo
e
s 00040/00006/00945
d D 1.8 91/12/09 00:29:15 labc-4lc 8 7
c added alteration messages
e
s 00084/00003/00867
d D 1.7 91/12/08 03:33:47 labc-4lc 7 6
c implemented usage messages for objects
e
s 00006/00006/00864
d D 1.6 91/12/07 17:37:57 labc-4lc 6 5
c made error message window bigger and scrollable
e
s 00150/00002/00720
d D 1.5 91/08/04 21:34:57 labc-3id 5 4
c added code for many of the limitation properties
e
s 00001/00001/00721
d D 1.4 91/06/30 16:57:37 labc-3id 4 3
c removed all traces of old explosion 'special' types
e
s 00017/00015/00705
d D 1.3 91/05/16 01:24:12 labc-3id 3 2
c fixed redrawing of hand contents
e
s 00045/00036/00675
d D 1.2 91/05/15 05:06:40 labc-3id 2 1
c made magic work
e
s 00711/00000/00000
d D 1.1 91/02/16 12:55:29 labc-3id 1 0
c date and time created 91/02/16 12:55:29 by labc-3id
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

/* Main Game Play File */
D 14
/* To contain procedures usable by both players and monsters */
E 14
I 14
/* To contain procedures used by players */
E 14

D 16
/* inclusions */

E 16
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
I 16
#include "mapfunc.h"
E 16
#include "missile.h"
#include "socket.h"
#include "person.h"
I 5
#include "level.h"
E 5
#include "gamelist.h"
#include "gametime.h"
#include "lib.h"
I 7
#include "message.h"
I 16
#include "inventory.h"
E 16
E 7

I 16

E 16
D 7

E 7
/* minimum number of pixels players must use in a bitmap */
#define	PIXEL_MINIMUM	15


D 14

/* Give a nice title and welcome to entering player */

welcome_player()
{
  printf("\nWelcome to %s  Version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
  printf("1990 By Van A. Boughner, Mel Nicholson, ");
  printf("and Albert C. Baker III\n\n");
}

  

E 14
I 14
void startup_player_persons(num)
E 14
/* set up the person array for as many persons as this player needs,
   also supervise the start up of the game person info array.
   Signifigant changes here should also be reflected by comparable
   changes in the trim_person_arrays() procedure. */
D 14

startup_player_persons(num)
E 14
{
int i;

  /* if player tries to be too big, error out */
  if (num > MAX_PLAYER_SIZE) Gerror("limit execeeded, change MAX_PLAYER_SIZE");
  
  /* person for this player */
  num_persons = num;

  /* init the list of persons and define the person records */
  initialize_person_array();
  for (i=0; i<num; i++)  initialize_person(i);

  /* initialize the array that will hold info on all players */
  initialize_game_person_array();

  /* initialize enough elements to hold this player's persons */
  persons_in_game = num;
  for (i=0; i<num; i++)  initialize_game_person(i);
}



I 14
load_player_into_person(player, num)
E 14
/* load the info from a player structure into an already initialized
   person record */
D 14

load_player_into_person(player, num)
E 14
PlayerInfo *player;
int	   num;
{
  strcpy(gameperson[num]->name, player->player_name);
  strcpy(gameperson[num]->login, player->login);
  strcpy(gameperson[num]->host, player->host);
  gameperson[num]->team = player->team;  /* this team selection thing
					    is also done later under
					    player_select_game() */
}



/* =================== U S I N G  objects ============================== */

D 14
/* attempt to use a certain object in a particular hand towards a given
   location */
E 14

D 14
use_object(pnum, which_hand, x, y)
{
I 12
  char *errmsg = NULL;
E 12
D 2
  int i, type, slot, used = FALSE, charges_lost = 0;
E 2
I 2
  int i, type, slot, used = 0, charges_lost = 0;
E 2
  
  /* check that this is one of my persons */
  if (pnum < 0 || pnum >= num_persons) return;

  /* if this person shouldn't be allowed to use anything yet, then don't 
     let him do it yet, just put it off. */
  if (gametime < person[pnum]->fire_again) {
	set_future_use(pnum, which_hand, x, y);
	return;
  }

  /* find out what is in the hand */
  type = (unsigned char) (person[pnum]->hand[which_hand].type);

D 2
  /* if this object is empty then don't try to use it */
  if (!has_uses_left(&(person[pnum]->hand[which_hand]))) return;
E 2
I 2
  /* use the object only if it is not 'empty' */
  if (has_uses_left(&(person[pnum]->hand[which_hand]))) {
E 2

D 2
  /* if it is a weapon fire it */
  if (info[(uc)type]->weapon) used = use_weapon(pnum, which_hand, x, y);
E 2
I 2
D 5
    /* if the object has magical constraints in use, check them */
    if (meets_magical_constraints(pnum, type,
E 5
I 5
    /* check the limititations on use for this kind of item */
D 12
    if (!bound_by_limitation(pnum, type, which_hand)) {
E 12
I 12
    if (!bound_by_limitation(pnum, type, which_hand, &errmsg)) {
E 12

      /* if the object has magical constraints in use, check them */
      if (meets_magical_constraints(pnum, type,
E 5
				 person[pnum]->hand[which_hand].record)) {
E 2

D 2
  /* if it charges another item, then try it */
  if (info[(uc)type]->charges)
    charges_lost = use_charger(&(person[pnum]->hand[which_hand]),
			       &(person[pnum]->hand[OTHER(which_hand)]));
E 2
I 2
	/* if it is a weapon fire it */
	if (info[(uc)type]->weapon) used += use_weapon(pnum, which_hand, x, y);
E 2

D 2
  /* if it opens something, attempt it */
  if (info[(uc)type]->opens) used = used || use_opener(pnum, which_hand, x, y);
E 2
I 2
	/* if it charges another item, then try it */
D 3
	if (info[(uc)type]->charges)
E 3
I 3
	if (info[(uc)type]->charges) {
E 3
	  charges_lost = use_charger(&(person[pnum]->hand[which_hand]),
				     &(person[pnum]->hand[OTHER(which_hand)]));
I 3
	  if (charges_lost) redraw_hand_contents(OTHER(which_hand));
	}
E 3
E 2

D 2
  /* if it is a magical item, try it */
  if (info[(uc)type]->magic) used = used || use_magical(pnum, which_hand, x, y);
E 2
I 2
	/* if it opens something, attempt it */
	if (info[(uc)type]->opens)
	  used += use_opener(pnum, which_hand, x, y);
E 2

D 2
  /* or etc. */
E 2
I 2
	/* if it is a magical item, try it */
	if (info[(uc)type]->magic)
	  used += use_magical(pnum, which_hand, x, y);
E 2

D 2
  if (used && !charges_lost) charges_lost = 1;
E 2
I 2
	/* or etc. */
E 2

I 5
D 7
        /* if the object was used, reduce player commodities if neccessary */
        if (used || charges_lost)
E 7
I 7
        /* if the object was used, reduce player commodities if neccessary,
           and send out messages to other players if neccessary. */
        if (used || charges_lost) {
E 7
	  reduce_player_commodities(pnum, type);
I 7
	  send_out_any_use_messages(pnum, type);
	}
E 7

E 5
D 2
  /* now spend time reducing ammo or removing the object from hand if used */
  if (used || charges_lost) {
    /* reduce charges on charged objects by given amount, unless the object
       is a weapon, then remove charges only if missile object is "lost" */
    if (info[(uc)type]->numbered &&
	!(info[(uc)type]->weapon &&
	  !info[(uc)(info[(uc)type]->movingobj)]->lost))
      reduce_count(&(person[pnum]->hand[which_hand]), charges_lost);

    /* when used up, remove object from hand, try to reload w/similar object */
    if (info[(uc)type]->lost && !query_count(&(person[pnum]->hand[which_hand]))) {
      slot = remove_with_reload(pnum, which_hand);
      if (has_display && slot != -1)  {
        redraw_inv_square(slot);
	redraw_player_weight();
      }
E 2
I 2
D 3
	/* now spend time reducing ammo or removing the object from hand if used */
E 3
I 3
	/* now spend time reducing ammo or removing the object if used */
E 3
	if (used || charges_lost) {
	  /* reduce charges on charged objects by given amount,
	     unless the object is a weapon, then remove charges
	     only if missile object is "lost" */
	  if (info[(uc)type]->numbered &&
	      !(info[(uc)type]->weapon &&
		!info[(uc)(info[(uc)type]->movingobj)]->lost)) {
	  if (used && !charges_lost) charges_lost = 1;
		reduce_count(&(person[pnum]->hand[which_hand]), charges_lost);
          if (has_display) redraw_hand_contents(which_hand);
	  }
	}
E 2
D 3
    }
I 2
  }
E 3
E 2

D 2
    /* update the person variable that tells us when to let him fire again */
    person[pnum]->fire_again = gametime + firing_wait(type);

    /* redraw hand contents (numbers might have changed) */
    if (has_display) for (i=0; i<2; i++) redraw_hand_contents(i);
E 2
I 2
D 3
  /* when used up, remove object from hand, try to reload w/similar object */
  if (info[(uc)type]->lost && 
      !query_count(&(person[pnum]->hand[which_hand]))) {
    slot = remove_with_reload(pnum, which_hand);
    if (has_display && slot != -1)  {
      redraw_inv_square(slot);
      redraw_player_weight();
E 3
I 3
	/* when used up, remove object from hand, try to reload w/similar */
	if (info[(uc)type]->lost && 
	  !query_count(&(person[pnum]->hand[which_hand]))) {
	    slot = remove_with_reload(pnum, which_hand);
	    if (has_display && slot != -1)  {
	      redraw_inv_square(slot);
	      redraw_player_weight();
	    }
	redraw_hand_contents(which_hand);
	}

        /* update the person variable that tells us when he can fire again */
        person[pnum]->fire_again = gametime + firing_wait(type);
I 5
      }
    }
I 12
    else {
      /* object was bound by some limitation, produce error messages */
      if (has_display && errmsg) {
	player_error_out_message(errmsg);
      }
    }
E 12
  }
}



E 14
I 14
static char *caterrmsg(s1, s2)
E 14
I 12
/* concatenates error messages line, puts result into locally allocd string.
   caller should not free the returned string, nor depend on it value
   staying the same next time he calls this routine. */
D 14
static char *caterrmsg(s1, s2)
E 14
char *s1, *s2;
{
  static char *oldls = NULL;
  static char *ls = NULL;

  if (!s1 && !s2) return NULL;
  if (!s1) return s2;
  if (!s2) return s1;
  if (oldls) free(oldls);
  oldls = ls;
  ls = (char *) malloc(strlen(s1) + strlen(s2) + 2);
  demand(ls, "no memory for catting error messages");
  sprintf(ls, "%s\n%s", s1, s2);
  return ls;
}



I 14
static void adderrmsg(holder, msg)
E 14
/* handles the adding of an error message, if the holder given is not
   NULL.  Used for updating the value of the holder variable,
   by adding another line of textand making it pointer to a new string. */
D 14
static void adderrmsg(holder, msg)
E 14
char **holder;
char *msg;
{
  if (holder) *holder = caterrmsg(*holder, msg);
}


I 16
/* *** STOPPED HERE *** */
E 16

I 16

E 16
I 14
static int bound_by_limitation(pnum, type, which_hand, msg)
E 14
E 12
/* check the various limitations on use of certain objects, return TRUE
   if there is some limitation that should keep player from using this
D 12
   object.  If the object number is illegal, TRUE is always returned. */
int bound_by_limitation(pnum, type, which_hand)
E 12
I 12
   object.  If the object number is illegal, TRUE is always returned.
   If msg is not NULL, then a pointer to a static error message string
   will be returned, containing as many errors as occurred.
   String should be copied if you want to keep it before calling this again. */
D 14
int bound_by_limitation(pnum, type, which_hand, msg)
E 14
E 12
int pnum;
int type;
int which_hand;
I 12
char **msg;
E 12
{
I 12
  char s[200];
E 12
  int players = FALSE, representation = FALSE, inout = FALSE;
  int roomage = FALSE, teamage = FALSE, class = FALSE, level = FALSE;
  int expts = FALSE, powerperc = FALSE, mana = FALSE, strength = FALSE;
  int health = FALSE, another = FALSE, deity = FALSE;
  if (type < 0 || type >= objects) return TRUE;
I 12
  if (msg) *msg = NULL;
E 12

  /* check the number of players, min and max */
  if (info[type]->minplayers || info[type]->maxplayers) {
    int numplayers = players_in_game();
    players =
      ((info[type]->maxplayers && info[type]->maxplayers < players) ||
       (info[type]->minplayers > players));
I 12
    if (players && msg) {
        if (info[type]->minplayers > players)
	  sprintf(s, "use: must be %d player(s) in game to use object",
		  info[type]->minplayers);
	else
	  sprintf(s, "use: must not be more than %d player(s) in game to use",
		  info[type]->maxplayers);
	adderrmsg(msg, s);
	sprintf(s, "use: there currently are/is %d player(s) in the game");
	adderrmsg(msg, s);
    }
E 12
  }

  /* check whether all teams must be represented */
D 12
  if (info[type]->represented) {
    representation = !all_teams_represented();
E 12
I 12
  if (info[type]->represented && mapstats.teams_supported > 1) {
    representation = ((info[type]->represented & teams_represented()) ==
		      info[type]->represented);
    if (representation)
      adderrmsg(msg, "use: the right teams aren't represented yet");
E 12
  }

  /* NOT DONE: insert check for insideness and outsideness here */

  /* check that player is in the right team's room */
  if (info[type]->rooms) {
    roomage = (!(info[type]->rooms & (1<<room[gameperson[pnum]->room].team)));
I 12
    if (roomage)
      adderrmsg(msg, "use: you can't use that in this room");
E 12
  }

  /* check that player belongs to the right team to use this object */
  if (info[type]->teams) {
    teamage = (!(info[type]->teams & (1<<gameperson[pnum]->team)));
I 12
    if (teamage)
      adderrmsg(msg, "use: you don't belong to the right team");
E 12
  }

  /* NOT DONE: insert check for class type here */

  /* check for the player's level being appropriate */
  if (info[type]->minlevel || info[type]->maxlevel) {
    level = ((info[type]->maxlevel &&
	      gameperson[pnum]->level > info[type]->maxlevel) ||
	     (gameperson[pnum]->level < info[type]->minlevel));
I 12
    if (level && msg) {
      if (gameperson[pnum]->level < info[type]->minlevel)
        sprintf(s, "use: you need to be level %d to use this",
		info[type]->minlevel);
      else
        sprintf(s, "use: this can only be used through level %d",
		info[type]->maxlevel);
      adderrmsg(msg, s);
    }
E 12
  }

  /* check for an appropriate number of experience points */
  expts = (person[pnum]->experience < info[type]->experience);
I 12
  if (expts && msg) {
    sprintf(msg, "use: you don't have the %d experience points required",
	    info[type]->experience);
    adderrmsg(msg, s);
  }
E 12

  /* look for the right percentage of power remaining */
  if (info[type]->power) { 
    int minimal = max_person_power(pnum) * info[type]->power / 100;
    powerperc = (person[pnum]->power < minimal);
I 12
    if (powerperc && msg)  {
      sprintf(s, "use: you've got to have %d%% of your power points", minimal);
      adderrmsg(msg, s);
    }
E 12
  }

  /* check for the right amount of available mana */
  mana = (info[type]->mana > person[pnum]->power);
I 12
  if (mana && msg) {
    sprintf(msg, "use: you don't have the %d power points required",
	    info[type]->mana);
    adderrmsg(msg, s);
  }
E 12

  /* look for the right percentage of health remaining */
  if (info[type]->strength) {
    int minimal = max_person_health(pnum) * info[type]->strength / 100;
    strength = (person[pnum]->health < minimal);
I 12
    if (strength && msg)  {
      sprintf(s, "use: you've got to have %d%% of your health points", minimal);
      adderrmsg(msg, s);
    }
E 12
  }

  /* make sure person has enough health points */
  health = (info[type]->health > person[pnum]->health);
I 12
  if (health && msg) {
    sprintf(msg, "use: you don't have the %d health points required",
	    info[type]->health);
    adderrmsg(msg, s);
  }
E 12

  /* check object in other hand */
  if (info[type]->needanother) {
    int otherhand = (which_hand ? 0 : 1);
D 16
    another = (person[pnum]->hand[otherhand].type != info[type]->otherobj);
E 16
I 16
D 17
    another = (person[pnum]->hand_object[otherhand]->type !=
	       info[type]->otherobj);
E 17
I 17
    another = (!(person[pnum]->hand_object[otherhand]) ||
	       (person[pnum]->hand_object[otherhand]->type !=
		info[type]->otherobj));
E 17
E 16
I 12
    if (another) {
      if (info[type]->otherobj)
        adderrmsg(msg, "use: you don't have the right thing in the other hand");
      else
	adderrmsg(msg, "use: your other hand must be empty");
    }
E 12
  }

  /* check whether player needs to be a deity */
  deity = (info[type]->deity && !gameperson[pnum]->deity);
I 12
  if (deity)
    adderrmsg(msg, "use: you have to be a deity to use this object");
E 12

  return (players || representation || inout || roomage || teamage ||
	  class || level || expts || powerperc || mana || strength ||
	  health || another || deity);
}



I 14
static void reduce_player_commodities(pnum, type)
E 14
/* check to see if the object is supposed to use up some of the player's
   magic power points, health, or experience points */
D 14
reduce_player_commodities(pnum, type)
E 14
int pnum, type;
{
  char msg[200], *name;
  if (info[type]->name) name = info[type]->name;
  else name = "object";
  if (type < 0 || type >= objects) return;

  if (info[type]->experience) {
    if (info[type]->experience > 0)
      sprintf(msg, "The %s took %d from your experience.", name,
	      info[type]->experience);
    else
      sprintf(msg, "The %s gave you %d experience.", name,
	      -(info[type]->experience));
    alter_experience(pnum, -(info[type]->experience), msg);
  }

  if (info[type]->mana) {
    if (info[type]->mana > 0)
      sprintf(msg, "The %s took %d from your power.", name, info[type]->mana);
    else
      sprintf(msg, "The %s gave you %d power.", name, -(info[type]->mana));
    person[pnum]->power -= info[type]->mana;
    if (person[pnum]->power < 0) person[pnum]->power = 0;
    if (person[pnum]->power > max_person_power(pnum))
      person[pnum]->power = max_person_power(pnum);
    if (has_display && pnum == 0) {
      redraw_player_magic();
D 6
      redraw_error_window(msg);
E 6
I 6
      player_error_out_message(msg);
E 6
    }
  }

  if (info[type]->health) {
    if (info[type]->health > 0)
      sprintf(msg, "The %s took %d from your health.", name,
	      info[type]->health);
    else
      sprintf(msg, "The %s gave you %d health.", name, -(info[type]->health));
    person[pnum]->health -= info[type]->health;
    if (person[pnum]->health < 0) person[pnum]->health = 0;
    if (person[pnum]->health > max_person_health(pnum))
      person[pnum]->health = max_person_health(pnum);
    if (has_display && pnum == 0) {
      redraw_player_health();
D 6
      redraw_error_window(msg);
E 6
I 6
      player_error_out_message(msg);
E 6
E 5
E 3
    }
D 3
    redraw_hand_contents(which_hand);
E 3
E 2
  }
I 7
}



D 8
send_out_messages(pnum, objnum, personal_msg, room_msg, all_msg)
/* sends out by socket mail the appropriate messages given as args */
int pnum, objnum;
E 8
I 8
D 12
send_out_alteration_messages(pnum, alterer, objnum, personal_msg,
			     room_msg, all_msg)
/* sends out by socket mail the appropriate messages given as args,
   this routine does double duty, serving for both usage messages and
   alteration notification messages.  alterer should be zero if this
   is being used for usage messages. */
int pnum, alterer, objnum;
E 12
I 12
D 14
send_out_the_use_messages(pnum, objnum, personal_msg, room_msg, all_msg)
E 14
I 14
static void send_out_the_use_messages(pnum, objnum, personal_msg,
				      room_msg, all_msg)
E 14
/* sends out by socket mail the appropriate messages given as args. */
int pnum, objnum;
E 12
E 8
char *personal_msg, *room_msg, *all_msg;
{
  Packet pack, pack2;

  if (personal_msg || room_msg || all_msg) {
    char *msg;
    MessageInfo *msginfo = create_message_info();
D 8
    MI_USAGE_INFO(msginfo, info[objnum]->name, gameperson[pnum]->name,
E 8
I 8
D 12
    MI_ALTER_INFO(msginfo, info[objnum]->name, gameperson[pnum]->name,
E 12
I 12
D 13
    MI_USE_INFO(msginfo, info[objnum]->name, gameperson[pnum]->name,
E 13
I 13
    MI_USAGE_INFO(msginfo, info[objnum]->name, gameperson[pnum]->name,
E 13
E 12
E 8
		  gameperson[pnum]->id,
		  mapstats.team_name[gameperson[pnum]->team],
D 8
		  room[gameperson[pnum]->room].name);
E 8
I 8
D 12
		  room[gameperson[pnum]->room].name,
		  info[alterer]->name);
E 12
I 12
		  room[gameperson[pnum]->room].name);
E 12
E 8

    if (personal_msg) {
      msg = create_finished_message(personal_msg, msginfo,
				    PLAYER_MESSAGE_COLUMNS);
      player_error_out_message(msg);
      destroy_message(msg);
    }

    if (room_msg) {
      msg = create_finished_message(room_msg, msginfo, PLAYER_MESSAGE_COLUMNS);
D 9
      pack.type = REPORT;
      strcpy(pack.info.msg, msg);  /* WARNING: string size problem */
E 9
I 9
D 12
      PacketInit(&pack, REPORT);
      AddString(&pack, msg);
E 12
I 12
      prepare_report(&pack, msg);
E 12
E 9
      address_packet(&pack, gameperson[pnum]->id, TO_ALL);
      destroy_message(msg);

      if (all_msg) {
        if (all_msg != room_msg) {
	  msg = create_finished_message(all_msg, msginfo,
					PLAYER_MESSAGE_COLUMNS);
D 9
    	  pack2.type = REPORT;
	  strcpy(pack2.info.msg, msg); /* WARNING */
E 9
I 9
D 12
    	  PacketInit(&pack2, REPORT);
	  AddSTring(&pack2, msg);
E 12
I 12
    	  prepare_report(&pack2, msg);
E 12
E 9
          address_packet(&pack2, gameperson[pnum]->id, TO_ALL);
	  destroy_message(msg);
	  send_in_and_out_of_room(gameperson[pnum]->room, &pack, &pack2,
				  FALSE);
	}
        else {
          /* allmsg and roommsg are same */
          send_to_players(&pack, FALSE);
        }
      }

      else {
        /* there is a roommsg, but no allmsg */
        send_to_room(gameperson[pnum]->room, &pack, FALSE);
      }

    }
    destroy_message_info(msginfo);
  }
}



D 14
send_out_any_use_messages(pnum, objnum)
E 14
I 14
static void send_out_any_use_messages(pnum, objnum)
E 14
/* send out messages if directed to do so in the object definition */
int pnum, objnum;
{
  char *personal_msg, *room_msg, *all_msg;

  if (info[objnum]->usemsg) personal_msg = info[objnum]->usemsg;
  else if (info[objnum]->roommsg) personal_msg = info[objnum]->roommsg;
  else personal_msg = info[objnum]->allmsg;

  if (info[objnum]->roommsg) room_msg = info[objnum]->roommsg;
  else room_msg = info[objnum]->allmsg;

  all_msg = info[objnum]->allmsg;

D 8
  send_out_messages(pnum, objnum, personal_msg, room_msg, all_msg);
E 8
I 8
D 12
  send_out_alteration_messages(pnum, 0, objnum, personal_msg,
			       room_msg, all_msg);
E 12
I 12
  send_out_the_use_messages(pnum, objnum, personal_msg, room_msg, all_msg);
E 12
E 8
E 7
I 2
D 3

  /* update the person variable that tells us when to let him fire again */
  person[pnum]->fire_again = gametime + firing_wait(type);
E 3
E 2
}



D 14
/* if the future time has come and this person previously put off the usage
   of an object, then use it now that we are ready */

use_if_ready(pnum)
int pnum;
{
  /* maybe this person is not ready yet, if not then do nothing */
  if (gametime < person[pnum]->fire_again ||
      gameperson[pnum]->appearance == APPEAR_DYING) return;

  /* otherwise, if he is still in the same room as when he requested this
     object usage, use the object toward requested square */
  if (person[pnum]->future_hand >= 0)
    if (person[pnum]->future_room == gameperson[pnum]->room) {
      use_object(pnum, person[pnum]->future_hand,
		 person[pnum]->future_x, person[pnum]->future_y);
      person[pnum]->future_hand = -1;
    }
}



E 14
I 14
static int use_weapon(pnum, which_hand, x, y)
E 14
/* fire a weapon, given the same neccessary info required in above procedure.
   Return TRUE if it was fired, FALSE if it was not. */
D 14

use_weapon(pnum, which_hand, x, y)
E 14
int pnum, which_hand, x, y;
{
D 16
  InvObj *missile_id = NULL;
  InvObj *copy_fired_object();
E 16
I 16
  OI *missile_id = NULL;
E 16
  int type;

  /* if this is directed to person's own square then forget it */
  if (gameperson[pnum]->x == x && gameperson[pnum]->y == y) return FALSE;

I 17
  /* if there is nothing in the hand, then forget it */
  if (!person[pnum]->hand_object[which_hand]->type) return(FALSE);

E 17
  /* create a copy of the fired object to send as the missile */
  missile_id = copy_fired_object(pnum, which_hand);
  
D 16
  type = (unsigned char) (person[pnum]->hand[which_hand].type);
E 16
I 16
D 17
  type = (unsigned char) (person[pnum]->hand_object[which_hand]->type);
E 17
I 17
  type = person[pnum]->hand_object[which_hand]->type;
E 17
E 16
  /* send off the missile info packet */
  fire_missile(gameperson[pnum]->id, missile_id, gameperson[pnum]->room,
D 4
	       gameperson[pnum]->x, gameperson[pnum]->y, x, y, FALSE,type);
E 4
I 4
	       gameperson[pnum]->x, gameperson[pnum]->y, x, y, type);
E 4
  return TRUE;
}


I 14

static int use_charger(charger, chargee)
E 14
/* use a charging item, try to charge the thing given, return the number
   of charges uses, but don't remove them inside this procedure. */
D 14

int use_charger(charger, chargee)
E 14
D 16
InvObj *charger, *chargee;
E 16
I 16
OI *charger, *chargee;
E 16
{
  int avaliable, usable, transfer;
  
  /* check that this object is really a charger */
  if (!info[(uc)charger->type]->charges) return 0;

  /* check that this is really a chargee */
  if (!info[(uc)chargee->type]->recorded || 
      !info[(uc)chargee->type]->numbered) return 0;

  /* check that they have compatible types */
  if (!types_match(info[(uc)charger->type]->charges,
		   info[(uc)chargee->type]->type)) return 0;

  /* find out how much charge is avaliable */
  avaliable = query_count(charger);
  if (!avaliable) avaliable = 1;

  /* find out how much charge can be accepted */
  if (!info[(uc)chargee->type]->capacity)
    usable = avaliable;
  else
    usable = info[(uc)chargee->type]->capacity - query_count(chargee);

  /* give as much as can be taken */
  transfer = min(usable, avaliable);
  if (transfer) increase_count(chargee, transfer);

  return transfer;
}



I 14
static int use_opener(pnum, which_hand, x, y)
E 14
/* try to use the given opening item toward square xy.  Return TRUE if
   the item was used, FALSE if it could not be used. */
D 14

int use_opener(pnum, which_hand, x, y)
E 14
int pnum, which_hand, x, y;
{
  int type = 0, id = 0, used = FALSE;
D 16
  InvObj *obj;
E 16
I 16
  OI *obj;
E 16
  
  /* forget trying to open anything we are not next to */
D 17
  if (!person_next_to(pnum, gameperson[pnum]->room, x, y)) return;
E 17
I 17
  if (!person_next_to(pnum, gameperson[pnum]->room, x, y)) return(FALSE);
E 17

I 17
  /* forget trying to open anything with an empty hand */
  if (!person[pnum]->hand_object[which_hand]) return(FALSE);

E 17
  /* we don't do out of bounds opening either */
D 17
  if (OUT_OF_BOUNDS(gameperson[pnum]->room, x, y)) return;
E 17
I 17
  if (OUT_OF_BOUNDS(gameperson[pnum]->room, x, y)) return(FALSE);
E 17

  /* nor do we allow you to open/close something you are standing on */
  if (gameperson[pnum]->x == x && gameperson[pnum]->y == y) {
    if (has_display)
D 6
      redraw_error_window("Stand next to it, not on top of it.");
E 6
I 6
      player_error_out_message("Stand next to it, not on top of it.");
E 6
D 17
    return;
E 17
I 17
    return(FALSE);
E 17
  }

  /* figure out this object's opening type and id number */
D 16
  obj = &(person[pnum]->hand[which_hand]);
E 16
I 16
  obj = person[pnum]->hand_object[which_hand];
E 16
  type = info[(uc)obj->type]->opens;
D 16
  if (info[(uc)obj->type]->recorded && info[(uc)obj->type]->id && obj->record)
    id = obj->record->obj.infox;
E 16
I 16
  if (info[(uc)obj->type]->recorded && info[(uc)obj->type]->id)
    id = get_record(obj, REC_INFOX);
E 16

  /* try to open something on the given square */
  used = open_something_on_square(gameperson[pnum]->room, x, y, type, id);

  /* return whether the object was used or not */
  return used;
}



I 8
D 11
tell_others_about_alteration(pnum, alterer, altered)
/* tell others in the game about the altering of an object, if the object
   has alteration messages defined for it. */
int pnum, alterer, altered;
{
  char *personal_msg, *room_msg, *all_msg;

I 10
#ifdef PLUGH
E 10
  if (info[altered]->usealtermsg)
	personal_msg = info[altered]->usealtermsg;
  else if (info[altered]->roomaltermsg)
	personal_msg = info[altered]->roomaltermsg;
  else
	personal_msg = info[altered]->allaltermsg;

  if (info[altered]->roomaltermsg)
	room_msg = info[altered]->roomaltermsg;
  else
	room_msg = info[altered]->allaltermsg;

  all_msg = info[altered]->allaltermsg;

  send_out_alteration_messages(pnum, alterer, altered, personal_msg,
			       room_msg, all_msg);
I 10
#endif /* PLUGH */
E 10
}



E 11
E 8
D 14
/* check through objects on given square for something openable by the
   given typed and id'd opener.  Return TRUE if something was opened. */
E 14

int open_something_on_square(roomnum, x, y, type, id)
I 14
/* check through objects on given square for something openable by the
D 16
   given typed and id'd opener.  Return TRUE if something was opened. */
E 16
I 16
   given typed and id'd opener.  Open all openable objects.
   Return TRUE if something was opened. */
E 16
E 14
int roomnum, x, y, type, id;
{
D 16
  unsigned char *here;
  int i, obj_id = 0, found = -1;
  MemObj *obj = NULL, *new = NULL;
  
  /* find out what kinds of items are on the square */
  here = whats_on_square(roomnum, x, y);
E 16
I 16
  int obj_id, found = 0;
  OI *o;
E 16

D 16
  /* look for an alternating type object */
  for (i=0; i<=ROOM_DEPTH; i++)
    if (info[(uc)here[i]]->swings) {
      found = i;
E 16
I 16
  /* go through objects on square looking for openable object */
  for (o = first_obj_here(&mapstats, roomnum, x, y); o; o = o->next) {
    if (info[(uc)o->type]->swings) {
      obj_id = get_record(o, REC_INFOX);
      found++;
E 16

D 16
      /* figure out id of the object here, if it has one */
      if (found == ROOM_DEPTH) {
        obj = what_recorded_obj_here(roomnum, x, y);
        if (obj) obj_id = obj->obj.infox;
      }

E 16
      /* check if type and id are right to make this object alternate */
D 16
      if (types_match(type, info[(uc)here[found]]->type) && 
E 16
I 16
      if (types_match(type, info[(uc)o->type]->type) && 
E 16
          ids_compatible(id, obj_id)) {
D 16
	    /* tell everyone about the successful change to the map */
            if (info[(uc)here[found]]->recorded && obj) {
   	      new = allocate_mem_obj();
	      bcopy(obj, new, sizeof(MemObj));
	      new->obj.objtype = info[(uc)here[found]]->alternate;
 	    }
	    notify_all_of_map_change(roomnum, x, y, found, here[found],
				 info[(uc)here[found]]->alternate, new);
I 8
D 11
	    tell_others_about_alteration(0, type, (int)here[found]);
E 11
E 8
	    if (new) free(new);
E 16
I 16
	/* tell everyone about the successful change to the map */
	if (info[(uc)o->type]->recorded) {
	  o->type = info[(uc)o->type]->alternate;
	}

/*	notify_all_of_map_change(roomnum, x, y, found, here[found],
				 info[(uc)here[found]]->alternate, new); */
E 16
      }
    }
I 16
  }

E 16
  /* if nothing openable was found return FALSE */
D 16
  return (found >= 0);
E 16
I 16
  return(found >= 0);
E 16
}



I 14
void use_object(pnum, which_hand, x, y)
/* attempt to use object in a particular hand toward x,y location */
{
  char *errmsg = NULL;
  int i, type, slot, used = 0, charges_lost = 0;
  
  /* check that this is one of my persons */
  if (pnum < 0 || pnum >= num_persons) return;

  /* if this person shouldn't be allowed to use anything yet, then don't 
     let him do it yet, just put it off. */
  if (gametime < person[pnum]->fire_again) {
	set_future_use(pnum, which_hand, x, y);
	return;
  }

I 17
  /* if there is nothing in the hand, return, doing nothing */
  if (!person[pnum]->hand_object[which_hand]) return;

E 17
  /* find out what is in the hand */
D 16
  type = (unsigned char) (person[pnum]->hand[which_hand].type);
E 16
I 16
D 17
  type = (unsigned char) (person[pnum]->hand_object[which_hand]->type);
E 17
I 17
  type = person[pnum]->hand_object[which_hand]->type;
E 17
E 16

  /* use the object only if it is not 'empty' */
D 16
  if (has_uses_left(&(person[pnum]->hand[which_hand]))) {
E 16
I 16
  if (has_uses_left(person[pnum]->hand_object[which_hand])) {
E 16

    /* check the limititations on use for this kind of item */
    if (!bound_by_limitation(pnum, type, which_hand, &errmsg)) {

D 16
      /* if the object has magical constraints in use, check them */
      if (meets_magical_constraints(pnum, type,
				 person[pnum]->hand[which_hand].record)) {
E 16
I 16
      /* if it is a weapon fire it */
      if (info[(uc)type]->weapon) used += use_weapon(pnum, which_hand, x, y);
E 16

D 16
	/* if it is a weapon fire it */
	if (info[(uc)type]->weapon) used += use_weapon(pnum, which_hand, x, y);
E 16
I 16
      /* if it charges another item, then try it */
      if (info[(uc)type]->charges) {
	charges_lost =
	  use_charger(person[pnum]->hand_object[which_hand],
		      person[pnum]->hand_object[OTHER(which_hand)]);
	if (charges_lost) redraw_hand_contents(OTHER(which_hand));
      }
E 16

D 16
	/* if it charges another item, then try it */
	if (info[(uc)type]->charges) {
	  charges_lost = use_charger(&(person[pnum]->hand[which_hand]),
				     &(person[pnum]->hand[OTHER(which_hand)]));
	  if (charges_lost) redraw_hand_contents(OTHER(which_hand));
	}
E 16
I 16
      /* if it opens something, attempt it */
      if (info[(uc)type]->opens)
	used += use_opener(pnum, which_hand, x, y);
E 16

D 16
	/* if it opens something, attempt it */
	if (info[(uc)type]->opens)
	  used += use_opener(pnum, which_hand, x, y);
E 16
I 16
      /* or etc. */
E 16

D 16
	/* if it is a magical item, try it */
	if (info[(uc)type]->magic)
	  used += use_magical(pnum, which_hand, x, y);
E 16
I 16
      /* if the object was used, reduce player commodities if neccessary,
	 and send out messages to other players if neccessary. */
      if (used || charges_lost) {
	reduce_player_commodities(pnum, type);
	send_out_any_use_messages(pnum, type);
      }
E 16

D 16
	/* or etc. */

        /* if the object was used, reduce player commodities if neccessary,
           and send out messages to other players if neccessary. */
        if (used || charges_lost) {
	  reduce_player_commodities(pnum, type);
	  send_out_any_use_messages(pnum, type);
	}

	/* now spend time reducing ammo or removing the object if used */
	if (used || charges_lost) {
	  /* reduce charges on charged objects by given amount,
	     unless the object is a weapon, then remove charges
	     only if missile object is "lost" */
	  if (info[(uc)type]->numbered &&
	      !(info[(uc)type]->weapon &&
		!info[(uc)(info[(uc)type]->movingobj)]->lost)) {
E 16
I 16
      /* now spend time reducing ammo or removing the object if used */
      if (used || charges_lost) {
	/* reduce charges on charged objects by given amount,
	   unless the object is a weapon, then remove charges
	   only if missile object is "lost" */
	if (info[(uc)type]->numbered &&
	    !(info[(uc)type]->weapon &&
	      !info[(uc)(info[(uc)type]->movingobj)]->lost)) {
E 16
	  if (used && !charges_lost) charges_lost = 1;
D 16
		reduce_count(&(person[pnum]->hand[which_hand]), charges_lost);
E 16
I 16
	  reduce_count(person[pnum]->hand_object[which_hand],
		       charges_lost);
E 16
          if (has_display) redraw_hand_contents(which_hand);
D 16
	  }
E 16
	}
I 16
      }
E 16

D 16
	/* when used up, remove object from hand, try to reload w/similar */
	if (info[(uc)type]->lost && 
	  !query_count(&(person[pnum]->hand[which_hand]))) {
	    slot = remove_with_reload(pnum, which_hand);
	    if (has_display && slot != -1)  {
	      redraw_inv_square(slot);
	      redraw_player_weight();
	    }
	redraw_hand_contents(which_hand);
E 16
I 16
      /* when used up, remove object from hand, try to reload w/similar */
      if (info[(uc)type]->lost && 
	  !query_count(person[pnum]->hand_object[which_hand])) {
	slot = remove_with_reload(pnum, which_hand);
	if (has_display && slot != -1)  {
	  redraw_inv_square(slot);
	  redraw_player_weight();
E 16
	}
D 16

        /* update the person variable that tells us when he can fire again */
        person[pnum]->fire_again = gametime + firing_wait(type);
E 16
I 16
	redraw_hand_contents(which_hand);
E 16
      }
I 16

      /* update the person variable that tells us when he can fire again */
      person[pnum]->fire_again = gametime + firing_wait(type);
E 16
    }
D 16
    else {
      /* object was bound by some limitation, produce error messages */
      if (has_display && errmsg) {
	player_error_out_message(errmsg);
      }
E 16
I 16
  }
  else {
    /* object was bound by some limitation, produce error messages */
    if (has_display && errmsg) {
      player_error_out_message(errmsg);
E 16
    }
  }
}



I 16

E 16
void use_if_ready(pnum)
/* if the future time has come and this person previously put off the usage
   of an object, then use it now that we are ready */
int pnum;
{
  /* maybe this person is not ready yet, if not then do nothing */
  if (gametime < person[pnum]->fire_again ||
      gameperson[pnum]->appearance == APPEAR_DYING) return;

  /* otherwise, if he is still in the same room as when he requested this
     object usage, use the object toward requested square */
  if (person[pnum]->future_hand >= 0)
    if (person[pnum]->future_room == gameperson[pnum]->room) {
      use_object(pnum, person[pnum]->future_hand,
		 person[pnum]->future_x, person[pnum]->future_y);
      person[pnum]->future_hand = -1;
    }
}



E 14
/* ======================== Q U I T T I N G ============================== */

/* show other players that this player is quitting by changing his picture
   the the little "poof" bitmap */

show_player_quitting()
{
  register int i;
  char s[MSG_LENGTH];

  for (i=0; i<num_persons; i++) show_person_quitting(i);

  /* wait a little so everyone can see it */
  set_quit_mode(QUITTING);
  if (has_display) {
    i = (CLICKS_TO_QUIT / CLICKS_PER_SEC);
D 14
    sprintf(s, "QUITTING IN %d SECONDS... STANDBY...", i);
E 14
I 14
    sprintf(s, "Quitting in %d seconds... standby...", i);
E 14
D 6
    redraw_error_window(s);
E 6
I 6
    player_error_out_message(s);
E 6
  }
}



/* show an individual person quitting, provide a visual effect for other
   players */

show_person_quitting(pnum)
int pnum;
{
  /* change our appearance to show that we are quitting and send to all */
  gameperson[pnum]->appearance = APPEAR_QUITTING;
  move_in_place(pnum);
}



/* give up quitting for now, restore appearances back to normal */

abort_player_quit()
{
  register int i;
  char s[MSG_LENGTH];

  /* defuse mode variable */
  what_mode = NORMAL;

  /* reset appearance back to normal */
  for (i=0; i<num_persons; i++) show_person_normal_again(i);
  if (has_display) {
D 14
    sprintf(s, "QUITTING ABORTED... game continues");
E 14
I 14
    sprintf(s, "Quitting aborted... the slaughter continues");
E 14
D 6
    redraw_error_window(s);
E 6
I 6
    player_error_out_message(s);
E 6
  }
}



show_person_normal_again(pnum)
int pnum;
{
  gameperson[pnum]->appearance = APPEAR_NORMAL;
  move_in_place(pnum);
}



/* show all other players that someone has died */

show_person_dying(pnum)
int pnum;
{
  /* free the persons movement queue so that he stops moving around */
  free_move_queue(&(gameperson[pnum]->moveq));

  /* make this person appear dead, ie. tombstone */
  gameperson[pnum]->appearance = APPEAR_DYING;
  move_in_place(pnum);

  /* set timer for later removal of tombstone */
  person[pnum]->death_timer = gametime + CLICKS_TO_DIE;
}



/* set quit/die mode timer */

set_quit_mode(mode)
int mode;
{
  /* check for valid mode */
  if (mode != QUITTING && mode != DYING) {
    if (DEBUG) printf("Warning: bad mode call\n");
    return;
  }

  /* set mode variable and time left counter */
  what_mode = mode;
  time_left = gametime +
	      ((what_mode == QUITTING) ? CLICKS_TO_QUIT : CLICKS_TO_DIE);
}
D 14

E 14



/* when a player wants to quit the game, he should give up his possessions */

player_quitting_ritual()
{
  register int i;

  for (i=0; i<num_persons; i++) disperse_belongings(i);
}



/* disperse person's belongs around the room nearby where he was */

disperse_belongings(pnum)
int pnum;
{
I 16
  int i;

E 16
  drop_everything(pnum);
  if (has_display && pnum == 0) {
D 16
    redraw_hand_contents(0);
    redraw_hand_contents(1);
E 16
I 16
    for (i=0; i<NUMBER_OF_HANDS; i++)
      redraw_hand_contents(i);
E 16
    redraw_inventory_window();
    redraw_player_weight();
  }
}



/* show persons of this player as "lost" or "won" when game is ending,
   given the number of the winning team */

show_player_ending_game(team)
int team;
{
  int i;

  for (i=0; i<num_persons; i++) {
    if (gameperson[i]->team == team)
      gameperson[i]->appearance = APPEAR_WON;
    else
      gameperson[i]->appearance = APPEAR_LOST;
    move_in_place(i);
  }
}



/* ============================== M I S C =============================== */

/* open the player's display and load a few things we know he'll need */

open_player_display()
{
  /* open the display for use */
  open_display();

  /* load the fonts we will use */
  if (DEBUG) printf("Load all fonts\n");
  load_all_fonts();
}



/* start up the windows the player needs */

start_player_windows()
{
  /* load the object pixmaps */
  if (DEBUG) printf("Load object pixmaps\n");
  define_object_pixmaps();

  /* define all pixmaps of persons in the game */
  if (DEBUG) printf("Load person pixmaps\n");
  define_all_person_pixmaps();

  /* have window system make the windows */
  if (DEBUG) printf("Open windows\n");
  player_window_setup();

  /* start input on those windows */
  if (DEBUG) printf("Let input come in\n");
  startup_player_input();
  
  /* have window system get windows to actually appear on screen */
  map_player_windows();
}

  

/* get latest news and place in message window */

get_game_news()
{
  FILE *fp;
  char *line;

  fp = fopen(NEWS_FILE, "r");

  /* if NULL then there is no news file */
  if (fp==NULL) return;

  /* read each line from news file, placing it in message window */
  while (!feof(fp)) {
    line = read_line(fp);
D 6
    if (line[0] != EOF) player_out_message(line);
E 6
I 6
    if (line[0] != EOF) player_error_out_message(line);
E 6
  }

  /* close the news file */
  fclose(fp);
}




/* try to get into the game selected, and load all of the initial
   information we will need, such as:  the map, the id numbers, inet
   addresses, and bitmaps of all the players, and the datagram socket
   address of the driver.  Returns 1 if we succeeded in joining a game,
   returns a negative number if we could not join the game requested.
   If this is a player process, and not a monster, then don't print out
   the messages. */

int try_to_access_game(player)
PlayerInfo *player;
{
  char s[120];
  GameInfo *game;
  DriverPack drvpack;
  int	   accepted;
  
  /* get the game record for our game */
  game = get_game_by_name(player, player->our_game);

  /* if there was no such game, then return -1 */
  if (game == NULL) return -1;

  /* prepare packet to tell driver about us */
  prepare_driver_packet(player, &drvpack);

  /* if there was then connect with the driver and load game info */
  if (!am_player) printf("Connecting with %s ...\n", game->host);
  accepted = initBossByAddr(&(game->address), &drvpack);

  /* if we were rejected for some reason then return -2 */
  if (!accepted) {
    sprintf(s, "%s not accepted into %s@%s",
	     player->player_name, game->name, game->host);
    if (!am_player) printf("%s\n", s);
    else joiner_message(s);
    free(game);
    return -2;
  }
  else {
    /* otherwise print acceptance message */
    sprintf(s, "%s now entered into %s@%s", player->player_name,
	    game->name, game->host);
    if (!am_player) printf("%s\n", s);
    else joiner_message(s);
    free(game);
    return TRUE;
  }
}




/* select starting places for my persons and notify all of my new 
   locations on the map */

select_starting_places()
{
  register int	i;

  for (i=0; i<num_persons; i++) select_person_place(i);
}



/* start out a person in a new place */

select_person_place(pnum)
int pnum;
{
  int roomnum, x, y, okay = 0;

  do {
    /* find a good room number for this player */
    roomnum = random_team_room(gameperson[pnum]->team);

    /* if no team room was found, look for any room */
    if (roomnum == -1) roomnum = random_team_room(-1);

    /* if no room was yet found then it's a big error! */
    if (roomnum == -1) Gerror("No starting room could be found");

    /* try for a good random location in the given room */
    okay = random_square_in_room(roomnum, &x, &y, FALSE, PLACE_DEFAULT);
  } while (!okay);

  /* move the person there to start */
  move_person(pnum, roomnum, x, y, TRUE);
}



/* get a good random spot in a given room */

select_locale_in_room(roomnum, x, y)
int roomnum, *x, *y;
{
  int okay = FALSE;
  
  do {
    /* start by getting a random location in room */
    *x = lrand48() % ROOM_WIDTH;
    *y = lrand48() % ROOM_HEIGHT;

    /* make sure that moving onto it is allowed */
    okay = max_move_on_square(roomnum, *x, *y);
  } while (!okay);
}



/* ======================== D U M P  variables ============================ */

/* print out to stdout the player's variables */

dump_player_variables()
{
  Location *ptr;

  /* print player's movement queue */
  if (gameperson[0]->moveq) {
    printf("Movement queue:\n");
    for (ptr=gameperson[0]->moveq; ptr; ptr = ptr->next)
      printf("Next move to %d %d\n", ptr->x, ptr->y);
  }
  else printf("No movement queue\n");
}



/* ====================== B I T M A P  checking ========================= */

/* check that the bitmap files player has asked for exist and are good */

check_player_bitmaps(player)
PlayerInfo *player;
{
  /* check that the files exist */
  if (!file_exists(player->personal_bitmap) ||
      !file_exists(player->bitmap_mask))
	Gerror("Couldn't access bitmap file(s)");

  /* check that players bitmap is good enough */
  if (bad_bitmap(player->personal_bitmap))
    Gerror("You must have more set pixels in your bitmap");
}



/* checks for bitmaps that don't have enough pixels set, returns TRUE
   if the bitmap is unacceptable */

int bad_bitmap(filename)
char *filename;
{
  short *data;
  int w, h, x, y, status;

  /* load the bitmap */
  status = XXReadBitmapFile(filename, &w, &h, &data, &x, &y);

  /* if there was an error */
  if (status != 1) Gerror("Error reading player bitmap");

  /* count bits set in the bitmap */
  return (!bitok(data, PIXEL_MINIMUM));
}
E 1
