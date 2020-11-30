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
/* To contain procedures used by players */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "mapfunc.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "level.h"
#include "gamelist.h"
#include "gametime.h"
#include "lib.h"
#include "message.h"
#include "inventory.h"


/* minimum number of pixels players must use in a bitmap */
#define	PIXEL_MINIMUM	15


void startup_player_persons(num)
/* set up the person array for as many persons as this player needs,
   also supervise the start up of the game person info array.
   Signifigant changes here should also be reflected by comparable
   changes in the trim_person_arrays() procedure. */
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



load_player_into_person(player, num)
/* load the info from a player structure into an already initialized
   person record */
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


static char *caterrmsg(s1, s2)
/* concatenates error messages line, puts result into locally allocd string.
   caller should not free the returned string, nor depend on it value
   staying the same next time he calls this routine. */
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



static void adderrmsg(holder, msg)
/* handles the adding of an error message, if the holder given is not
   NULL.  Used for updating the value of the holder variable,
   by adding another line of textand making it pointer to a new string. */
char **holder;
char *msg;
{
  if (holder) *holder = caterrmsg(*holder, msg);
}


/* *** STOPPED HERE *** */


static int bound_by_limitation(pnum, type, which_hand, msg)
/* check the various limitations on use of certain objects, return TRUE
   if there is some limitation that should keep player from using this
   object.  If the object number is illegal, TRUE is always returned.
   If msg is not NULL, then a pointer to a static error message string
   will be returned, containing as many errors as occurred.
   String should be copied if you want to keep it before calling this again. */
int pnum;
int type;
int which_hand;
char **msg;
{
  char s[200];
  int players = FALSE, representation = FALSE, inout = FALSE;
  int roomage = FALSE, teamage = FALSE, class = FALSE, level = FALSE;
  int expts = FALSE, powerperc = FALSE, mana = FALSE, strength = FALSE;
  int health = FALSE, another = FALSE, deity = FALSE;
  if (type < 0 || type >= objects) return TRUE;
  if (msg) *msg = NULL;

  /* check the number of players, min and max */
  if (info[type]->minplayers || info[type]->maxplayers) {
    int numplayers = players_in_game();
    players =
      ((info[type]->maxplayers && info[type]->maxplayers < players) ||
       (info[type]->minplayers > players));
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
  }

  /* check whether all teams must be represented */
  if (info[type]->represented && mapstats.teams_supported > 1) {
    representation = ((info[type]->represented & teams_represented()) ==
		      info[type]->represented);
    if (representation)
      adderrmsg(msg, "use: the right teams aren't represented yet");
  }

  /* NOT DONE: insert check for insideness and outsideness here */

  /* check that player is in the right team's room */
  if (info[type]->rooms) {
    roomage = (!(info[type]->rooms & (1<<room[gameperson[pnum]->room].team)));
    if (roomage)
      adderrmsg(msg, "use: you can't use that in this room");
  }

  /* check that player belongs to the right team to use this object */
  if (info[type]->teams) {
    teamage = (!(info[type]->teams & (1<<gameperson[pnum]->team)));
    if (teamage)
      adderrmsg(msg, "use: you don't belong to the right team");
  }

  /* NOT DONE: insert check for class type here */

  /* check for the player's level being appropriate */
  if (info[type]->minlevel || info[type]->maxlevel) {
    level = ((info[type]->maxlevel &&
	      gameperson[pnum]->level > info[type]->maxlevel) ||
	     (gameperson[pnum]->level < info[type]->minlevel));
    if (level && msg) {
      if (gameperson[pnum]->level < info[type]->minlevel)
        sprintf(s, "use: you need to be level %d to use this",
		info[type]->minlevel);
      else
        sprintf(s, "use: this can only be used through level %d",
		info[type]->maxlevel);
      adderrmsg(msg, s);
    }
  }

  /* check for an appropriate number of experience points */
  expts = (person[pnum]->experience < info[type]->experience);
  if (expts && msg) {
    sprintf(msg, "use: you don't have the %d experience points required",
	    info[type]->experience);
    adderrmsg(msg, s);
  }

  /* look for the right percentage of power remaining */
  if (info[type]->power) { 
    int minimal = max_person_power(pnum) * info[type]->power / 100;
    powerperc = (person[pnum]->power < minimal);
    if (powerperc && msg)  {
      sprintf(s, "use: you've got to have %d%% of your power points", minimal);
      adderrmsg(msg, s);
    }
  }

  /* check for the right amount of available mana */
  mana = (info[type]->mana > person[pnum]->power);
  if (mana && msg) {
    sprintf(msg, "use: you don't have the %d power points required",
	    info[type]->mana);
    adderrmsg(msg, s);
  }

  /* look for the right percentage of health remaining */
  if (info[type]->strength) {
    int minimal = max_person_health(pnum) * info[type]->strength / 100;
    strength = (person[pnum]->health < minimal);
    if (strength && msg)  {
      sprintf(s, "use: you've got to have %d%% of your health points", minimal);
      adderrmsg(msg, s);
    }
  }

  /* make sure person has enough health points */
  health = (info[type]->health > person[pnum]->health);
  if (health && msg) {
    sprintf(msg, "use: you don't have the %d health points required",
	    info[type]->health);
    adderrmsg(msg, s);
  }

  /* check object in other hand */
  if (info[type]->needanother) {
    int otherhand = (which_hand ? 0 : 1);
    another = (!(person[pnum]->hand_object[otherhand]) ||
	       (person[pnum]->hand_object[otherhand]->type !=
		info[type]->otherobj));
    if (another) {
      if (info[type]->otherobj)
        adderrmsg(msg, "use: you don't have the right thing in the other hand");
      else
	adderrmsg(msg, "use: your other hand must be empty");
    }
  }

  /* check whether player needs to be a deity */
  deity = (info[type]->deity && !gameperson[pnum]->deity);
  if (deity)
    adderrmsg(msg, "use: you have to be a deity to use this object");

  return (players || representation || inout || roomage || teamage ||
	  class || level || expts || powerperc || mana || strength ||
	  health || another || deity);
}



static void reduce_player_commodities(pnum, type)
/* check to see if the object is supposed to use up some of the player's
   magic power points, health, or experience points */
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
      player_error_out_message(msg);
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
      player_error_out_message(msg);
    }
  }
}



static void send_out_the_use_messages(pnum, objnum, personal_msg,
				      room_msg, all_msg)
/* sends out by socket mail the appropriate messages given as args. */
int pnum, objnum;
char *personal_msg, *room_msg, *all_msg;
{
  Packet pack, pack2;

  if (personal_msg || room_msg || all_msg) {
    char *msg;
    MessageInfo *msginfo = create_message_info();
    MI_USAGE_INFO(msginfo, info[objnum]->name, gameperson[pnum]->name,
		  gameperson[pnum]->id,
		  mapstats.team_name[gameperson[pnum]->team],
		  room[gameperson[pnum]->room].name);

    if (personal_msg) {
      msg = create_finished_message(personal_msg, msginfo,
				    PLAYER_MESSAGE_COLUMNS);
      player_error_out_message(msg);
      destroy_message(msg);
    }

    if (room_msg) {
      msg = create_finished_message(room_msg, msginfo, PLAYER_MESSAGE_COLUMNS);
      prepare_report(&pack, msg);
      address_packet(&pack, gameperson[pnum]->id, TO_ALL);
      destroy_message(msg);

      if (all_msg) {
        if (all_msg != room_msg) {
	  msg = create_finished_message(all_msg, msginfo,
					PLAYER_MESSAGE_COLUMNS);
    	  prepare_report(&pack2, msg);
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



static void send_out_any_use_messages(pnum, objnum)
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

  send_out_the_use_messages(pnum, objnum, personal_msg, room_msg, all_msg);
}



static int use_weapon(pnum, which_hand, x, y)
/* fire a weapon, given the same neccessary info required in above procedure.
   Return TRUE if it was fired, FALSE if it was not. */
int pnum, which_hand, x, y;
{
  OI *missile_id = NULL;
  int type;

  /* if this is directed to person's own square then forget it */
  if (gameperson[pnum]->x == x && gameperson[pnum]->y == y) return FALSE;

  /* if there is nothing in the hand, then forget it */
  if (!person[pnum]->hand_object[which_hand]->type) return(FALSE);

  /* create a copy of the fired object to send as the missile */
  missile_id = copy_fired_object(pnum, which_hand);
  
  type = person[pnum]->hand_object[which_hand]->type;
  /* send off the missile info packet */
  fire_missile(gameperson[pnum]->id, missile_id, gameperson[pnum]->room,
	       gameperson[pnum]->x, gameperson[pnum]->y, x, y, type);
  return TRUE;
}



static int use_charger(charger, chargee)
/* use a charging item, try to charge the thing given, return the number
   of charges uses, but don't remove them inside this procedure. */
OI *charger, *chargee;
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



static int use_opener(pnum, which_hand, x, y)
/* try to use the given opening item toward square xy.  Return TRUE if
   the item was used, FALSE if it could not be used. */
int pnum, which_hand, x, y;
{
  int type = 0, id = 0, used = FALSE;
  OI *obj;
  
  /* forget trying to open anything we are not next to */
  if (!person_next_to(pnum, gameperson[pnum]->room, x, y)) return(FALSE);

  /* forget trying to open anything with an empty hand */
  if (!person[pnum]->hand_object[which_hand]) return(FALSE);

  /* we don't do out of bounds opening either */
  if (OUT_OF_BOUNDS(gameperson[pnum]->room, x, y)) return(FALSE);

  /* nor do we allow you to open/close something you are standing on */
  if (gameperson[pnum]->x == x && gameperson[pnum]->y == y) {
    if (has_display)
      player_error_out_message("Stand next to it, not on top of it.");
    return(FALSE);
  }

  /* figure out this object's opening type and id number */
  obj = person[pnum]->hand_object[which_hand];
  type = info[(uc)obj->type]->opens;
  if (info[(uc)obj->type]->recorded && info[(uc)obj->type]->id)
    id = get_record(obj, REC_INFOX);

  /* try to open something on the given square */
  used = open_something_on_square(gameperson[pnum]->room, x, y, type, id);

  /* return whether the object was used or not */
  return used;
}




int open_something_on_square(roomnum, x, y, type, id)
/* check through objects on given square for something openable by the
   given typed and id'd opener.  Open all openable objects.
   Return TRUE if something was opened. */
int roomnum, x, y, type, id;
{
  int obj_id, found = 0;
  OI *o;

  /* go through objects on square looking for openable object */
  for (o = first_obj_here(&mapstats, roomnum, x, y); o; o = o->next) {
    if (info[(uc)o->type]->swings) {
      obj_id = get_record(o, REC_INFOX);
      found++;

      /* check if type and id are right to make this object alternate */
      if (types_match(type, info[(uc)o->type]->type) && 
          ids_compatible(id, obj_id)) {
	/* tell everyone about the successful change to the map */
	if (info[(uc)o->type]->recorded) {
	  o->type = info[(uc)o->type]->alternate;
	}

/*	notify_all_of_map_change(roomnum, x, y, found, here[found],
				 info[(uc)here[found]]->alternate, new); */
      }
    }
  }

  /* if nothing openable was found return FALSE */
  return(found >= 0);
}



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

  /* if there is nothing in the hand, return, doing nothing */
  if (!person[pnum]->hand_object[which_hand]) return;

  /* find out what is in the hand */
  type = person[pnum]->hand_object[which_hand]->type;

  /* use the object only if it is not 'empty' */
  if (has_uses_left(person[pnum]->hand_object[which_hand])) {

    /* check the limititations on use for this kind of item */
    if (!bound_by_limitation(pnum, type, which_hand, &errmsg)) {

      /* if it is a weapon fire it */
      if (info[(uc)type]->weapon) used += use_weapon(pnum, which_hand, x, y);

      /* if it charges another item, then try it */
      if (info[(uc)type]->charges) {
	charges_lost =
	  use_charger(person[pnum]->hand_object[which_hand],
		      person[pnum]->hand_object[OTHER(which_hand)]);
	if (charges_lost) redraw_hand_contents(OTHER(which_hand));
      }

      /* if it opens something, attempt it */
      if (info[(uc)type]->opens)
	used += use_opener(pnum, which_hand, x, y);

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
	  if (used && !charges_lost) charges_lost = 1;
	  reduce_count(person[pnum]->hand_object[which_hand],
		       charges_lost);
          if (has_display) redraw_hand_contents(which_hand);
	}
      }

      /* when used up, remove object from hand, try to reload w/similar */
      if (info[(uc)type]->lost && 
	  !query_count(person[pnum]->hand_object[which_hand])) {
	slot = remove_with_reload(pnum, which_hand);
	if (has_display && slot != -1)  {
	  redraw_inv_square(slot);
	  redraw_player_weight();
	}
	redraw_hand_contents(which_hand);
      }

      /* update the person variable that tells us when he can fire again */
      person[pnum]->fire_again = gametime + firing_wait(type);
    }
  }
  else {
    /* object was bound by some limitation, produce error messages */
    if (has_display && errmsg) {
      player_error_out_message(errmsg);
    }
  }
}




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
    sprintf(s, "Quitting in %d seconds... standby...", i);
    player_error_out_message(s);
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
    sprintf(s, "Quitting aborted... the slaughter continues");
    player_error_out_message(s);
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
  int i;

  drop_everything(pnum);
  if (has_display && pnum == 0) {
    for (i=0; i<NUMBER_OF_HANDS; i++)
      redraw_hand_contents(i);
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
    if (line[0] != EOF) player_error_out_message(line);
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
