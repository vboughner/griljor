h06069
s 00000/00000/00354
d D 1.9 92/08/07 01:02:08 vbo 9 8
c source copied to a separate tree for work on new map and object format
e
s 00005/00005/00349
d D 1.8 91/12/07 17:37:25 labc-4lc 8 7
c made error message window bigger and scrollable
e
s 00009/00004/00345
d D 1.7 91/08/29 01:40:16 vanb 7 6
c fixed up more compatibility problems
e
s 00005/00001/00344
d D 1.6 91/08/04 16:18:00 labc-3id 6 5
c fixed teleport bug - use while moving
e
s 00001/00001/00344
d D 1.5 91/06/30 16:57:18 labc-3id 5 4
c removed all traces of old explosion 'special' types
e
s 00006/00003/00339
d D 1.4 91/05/16 01:23:53 labc-3id 4 3
c a couple more fixes to recent additions
e
s 00113/00015/00229
d D 1.3 91/05/16 00:32:27 labc-3id 3 2
c added more teleporter types
e
s 00056/00007/00188
d D 1.2 91/05/15 05:06:08 labc-3id 2 1
c fixed lots of small bugs so old code would work
e
s 00195/00000/00000
d D 1.1 91/02/16 12:54:39 labc-3id 1 0
c date and time created 91/02/16 12:54:39 by labc-3id
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

/* Routines for handling magic items or powers */

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
#include "gametime.h"
#include "lib.h"
#include "level.h"
#include "magic.h"


/* Main magical item use routine, returns TRUE if the item was used */

int use_magical(pnum, which_hand, x, y)
int pnum, which_hand, x, y;
{
  register int i, j;
D 2
  int type, used = FALSE;
E 2
I 2
  int type, used = 0;
E 2

  /* check that this object is recorded and has a record */
D 4
  type = person[pnum]->hand[which_hand].type;
E 4
I 4
  type = (uc) person[pnum]->hand[which_hand].type;
E 4
  if (!info[(uc)type]->recorded ||
      !(person[pnum]->hand[which_hand].record)) return;

  /* go through each possible magic property slot, trying to execute it */
  for (i=1,j=0; j<MAGICAL_SLOTS; i*=2,j++) {
    if (info[(uc)type]->magic & i)
D 2
      used = used || use_specific_magical(pnum, which_hand, x, y, i);
  }
E 2
I 2
      used += use_specific_magical(pnum, which_hand, x, y, i);
    }
E 2

D 2
  return used;
E 2
I 2
  return (used > 0);
E 2
}



I 2
/* checks for contraints on health and magic points due to possible
   subtractions by certain magical properties, returns TRUE
   only if those contraints are met */

D 7
int meets_magical_constraints(int pnum, int type, MemObj *record)
E 7
I 7
int meets_magical_constraints(pnum, type, record)
int pnum, type;
MemObj *record;
E 7
{
  int i, j, meets = TRUE;

  if (!record || 
      !info[(uc)type]->recorded ||
      !info[(uc)type]->magic)   return TRUE;

  /* go through all magical slots looking for health or magic drain */
  for (i=1,j=0; j<MAGICAL_SLOTS; i*=2,j++) {
    if (info[(uc)type]->magic & i) {
      if (get_rec_value(record, i) == MA_HEALTH) {
        int amount = get_rec_value(record, SL_X);
	if (person[pnum]->health + amount < 0) {
	  meets = FALSE;
D 8
	  redraw_error_window("You don't have enough health points");
E 8
I 8
	  player_error_out_message("You don't have enough health points");
E 8
	}
      }
      if (get_rec_value(record, i) == MA_MAGIC) {
        int amount = get_rec_value(record, SL_X);
	if (person[pnum]->power + amount < 0) {
	  meets = FALSE;
D 8
	  redraw_error_window("You don't have enough power points");
E 8
I 8
	  player_error_out_message("You don't have enough power points");
E 8
	}
      }
    }
  }
  return meets;
}


E 2
/* use the magical property denoted by the value in the recorded object's
   record.  The variable to use in the record is given to us in variable
   'which_slot', return TRUE if some action was taken. */

int use_specific_magical(pnum, which_hand, x, y, which_slot)
int pnum, which_hand, x, y, which_slot;
{
D 3
  int property, used = TRUE;
E 3
I 3
  int type, property, used = FALSE;
  MemObj *rec = person[pnum]->hand[which_hand].record;
  int temp1, temp2;
E 3

  /* branch according to the magical property */
I 3
D 4
  type = person[pnum]->hand[which_hand].type;
E 4
I 4
  type = (uc) person[pnum]->hand[which_hand].type;
E 4
E 3
  property = get_rec_value(person[pnum]->hand[which_hand].record, which_slot);
  switch (property) {
	case MA_UNDEFINED:
D 3
	case MA_NOTHING:	used = FALSE;
				break;
	case MA_HEALTH:		used = ma_change_health(pnum, which_hand);
				break;
	case MA_MAGIC:		used = ma_change_magic(pnum, which_hand);
				break;
D 2
	case MA_RANDPORT:	used = ma_random_teleport(pnum);
E 2
I 2
	case MA_OWNPORT:	used = ma_own_teleport(pnum);
E 2
				break;
E 3
I 3
	case MA_NOTHING:
	  used = FALSE;
          break;
	case MA_HEALTH:
 	  used = ma_change_health(pnum, which_hand);
	  break;
	case MA_MAGIC:
	  used = ma_change_magic(pnum, which_hand);
	  break;
	case MA_OWNPORT:
	  used = ma_teleport(pnum, gameperson[pnum]->team, -1, -1, -1);
	  break;
	case MA_TEAMPORT:
	  used = ma_teleport(pnum, get_rec_value(rec, SL_EXTRA1), -1, -1, -1);
	  break;
	case MA_ANYPORT:
	  used = ma_teleport(pnum, -1, -1, -1, -1);
	  break;
	case MA_ROOMPORT:
	  used = ma_teleport(pnum, -1, get_rec_value(rec, SL_EXTRA1), -1, -1);
	  break;
	case MA_POINTPORT:
	  used = ma_teleport(pnum, -1, get_rec_value(rec, SL_EXTRA1),
			     get_rec_value(rec, SL_EXTRA2),
			     get_rec_value(rec, SL_EXTRA3));
	  break;
	case MA_SAMEPORT:
	  used = ma_teleport(pnum, -1, gameperson[pnum]->room, -1, -1);
	  break;
	case MA_OFFPORT:
          temp1 = gameperson[pnum]->x + get_rec_value(rec, SL_EXTRA1);
	  while (temp1 < 0) temp1 = temp1 + ROOM_WIDTH;
	  while (temp1 >= ROOM_WIDTH) temp1 = temp1 - ROOM_WIDTH;
	  temp2 = gameperson[pnum]->y + get_rec_value(rec, SL_EXTRA2);
	  while (temp2 < 0) temp2 = temp2 + ROOM_HEIGHT;
	  while (temp2 >= ROOM_HEIGHT) temp2 = temp2 - ROOM_HEIGHT;
	  used = ma_teleport(pnum, -1, gameperson[pnum]->room, temp1, temp2);
	  break;
	case MA_SPOTPORT:
D 4
	  used = ma_teleport(pnum, -1, gameperson[pnum]->room, x, y);
E 4
I 4
	  if (x < 0 || x >= ROOM_WIDTH || y < 0 || y >= ROOM_HEIGHT)
D 8
	    redraw_error_window("Cursor is in an illegal location.");
E 8
I 8
	    player_error_out_message("Cursor is in an illegal location.");
E 8
	  else
  	    used = ma_teleport(pnum, -1, gameperson[pnum]->room, x, y);
E 4
	  break;
	case MA_RADIUSPORT:
	  printf("radius teleport not yet supported\n");
	  break;
E 3
I 2
	case MA_FISTS:
D 3
          used = ma_fists_of_death(pnum,
	    get_rec_value(person[pnum]->hand[which_hand].record, SL_ZINGER));
E 3
I 3
          used = ma_fists_of_death(pnum, type);
E 3
          break;
E 2
  }

  return used;
}



/* figure out the value in a recorded object record variable, and return it */

int get_rec_value(record, slotnum)
MemObj *record;
int slotnum;
{
  int result = -1;

  /* if no record here then return -1 as the value */
  if (!record) return result;

  switch (slotnum) {
	case SL_DETAIL:		result = record->obj.detail;
				break;
	case SL_X:		result = record->obj.infox;
				break;
	case SL_Y:		result = record->obj.infoy;
				break;
	case SL_ZINGER:		result = record->obj.zinger;
				break;
	case SL_EXTRA1:		result = record->obj.extra[0];
				break;
	case SL_EXTRA2:		result = record->obj.extra[1];
				break;
	case SL_EXTRA3:		result = record->obj.extra[2];
				break;
  }

  return result;
}



/* use magical item that changes your health point level, return TRUE
   if the object was of use to you */

int ma_change_health(pnum, which_hand)
{
  int amount;

  /* find out how much health should change */
  amount = get_rec_value(person[pnum]->hand[which_hand].record, SL_X);

  /* if amount is 0 then nothing happens */
  if (!amount) return FALSE;

  /* check to see if this amount will not affect player's current health */
  if ((amount > 0 && person[pnum]->health == max_person_health(pnum)) ||
      (amount < 0 && person[pnum]->health <= 0)) {
	if (has_display && pnum == 0)
D 8
	  redraw_error_window("Your health is already at maximum");
E 8
I 8
	  player_error_out_message("Your health is already at maximum");
E 8
	return FALSE;
  }
  
  /* change this person's health */
  person[pnum]->health += amount;

  /* look for out of bounds values */
  if (person[pnum]->health < 0) person[pnum]->health = 0;
  if (person[pnum]->health > max_person_health(pnum))
    person[pnum]->health = max_person_health(pnum);

  /* if there is a display then redraw the health readout */
  if (has_display) redraw_player_health();

D 3
  return TRUE;		/* the item was used */
E 3
I 3
  return TRUE;
E 3
}



/* use magical item that changes your magic point level, return TRUE
   if the object was of use to you */

int ma_change_magic(pnum, which_hand)
{
  int amount;

  /* find out how much magic should change */
  amount = get_rec_value(person[pnum]->hand[which_hand].record, SL_X);

  /* if amount is 0 then nothing happens */
  if (!amount) return FALSE;

  /* check to see if this amount will not affect player's current magic */
  if ((amount > 0 && person[pnum]->power == max_person_power(pnum)) ||
      (amount < 0 && person[pnum]->power <= 0)) {
	if (has_display && pnum == 0)
D 8
	  redraw_error_window("Your magic power is at maximum");
E 8
I 8
	  player_error_out_message("Your magic power is at maximum");
E 8
	return FALSE;
  }
  
  /* change this person's magic power */
  person[pnum]->power += amount;

  /* look for out of bounds values */
  if (person[pnum]->power < 0) person[pnum]->power = 0;
  if (person[pnum]->power > max_person_power(pnum))
    person[pnum]->power = max_person_power(pnum);

  /* if there is a display then redraw the magic readout */
  if (has_display) redraw_player_magic();

  return TRUE;		/* the item was used */
}


I 3
/* teleport a person to a given set of random locales, each setting of team,
   roomnum, x, y, helps determine the scope of the teleportation.  Any setting
   that is -1 or a non-legal value, tells that that item should be randomized,
   otherwise it is taken as a limiting contraint.  If room and team are both
   not -1, then the room number os used, not the team number.
   If room number is SAME_ROOM, that means the same room as player is in. */
D 7
int ma_teleport(int pnum, int team, int roomnum, int x, int y)
E 7
I 7
int ma_teleport(pnum, team, roomnum, x, y)
int pnum, team, roomnum, x, y;
E 7
{
  int result_x, result_y;
  int rt = team, rr = roomnum;
  int team_me, tries = 30, okay = 0;

  if (pnum < 0 || pnum >= num_persons)
    Gerror("Tried to teleport someone else");

  /* figure out whether this is a room or team teleport */
  if (roomnum == SAME_ROOM) roomnum = gameperson[pnum]->room;
  team_me = !(roomnum >= 0 && roomnum < mapstats.rooms);
    
  do {
    /* if it is a team teleport but team is not valid, put -1 there */
    if (team_me && (team < 0 || team > mapstats.teams_supported)) rt = -1;
    else rt = team;

    /* if the room is not valid, pick one */
    if (roomnum < 0 || roomnum >= mapstats.rooms) {
      rr = random_team_room(rt);
      /* if room still not valid (due to bad team number), look for any room */
      if (rr < 0 || rr >= mapstats.rooms) {
        rr = random_team_room(-1);
        if (rr < 0 || rr >= mapstats.rooms)
	  Gerror("Can't find a room to teleport to");
      }
    }
    else rr = roomnum;

    /* get a spot in the room */
    tries--;
    okay = random_square_in_room(rr, &result_x, &result_y, 
				 FALSE, PLACE_DEFAULT);

    /* if x and y were defined, then set them */
    if (x >=0 && x < ROOM_WIDTH) result_x = x;
    if (y >=0 && y < ROOM_HEIGHT) result_y = y;

  } while (!okay && tries > 0);

D 6
  if (okay) move_person(pnum, rr, result_x, result_y, TRUE);
E 6
I 6
  if (okay) {
    /* clear the movement queue and teleport the person */
    if (gameperson[pnum]->moveq) free_move_queue(&(gameperson[pnum]->moveq));
    move_person(pnum, rr, result_x, result_y, TRUE);
  }
E 6
  return okay;
}



E 3
D 2
/* randomly teleport this person to another square on the map somewhere */
E 2
I 2
/* randomly teleport this person to another square on the map someplace
   that belongs to his own team */
E 2

D 2
int ma_random_teleport(int pnum)
E 2
I 2
D 7
int ma_own_teleport(int pnum)
E 7
I 7
int ma_own_teleport(pnum)
int pnum;
E 7
E 2
{
  /* teleport one of my own persons */
  select_person_place(pnum);
  return TRUE;
I 2
}



D 3
/* ran fist of death (or whatever the object given isin zinger) */
E 3
I 3
/* rain fist of death (or whatever the object given is fires) */
E 3

D 3
int ma_fists_of_death(int pnum, int objnum)
E 3
I 3
D 7
int ma_fists_of_death(int pnum, int firetype)
E 7
I 7
int ma_fists_of_death(pnum, firetype)
int pnum, firetype;
E 7
E 3
{
D 3
  printf("fists of death, person %s, object %s\n", pnum, objnum);
E 3
I 3
  if (firetype < 0 || firetype >= objects) firetype = 0;
  else if (!info[firetype]->weapon) firetype = 0;

  if (info[firetype]->weapon) {
    random_missile_barrage(pnum, gameperson[pnum]->id,
D 5
			   gameperson[pnum]->room, FALSE,
E 5
I 5
			   gameperson[pnum]->room,
E 5
			   firetype, info[firetype]->range);
  }
E 3
E 2
}
E 1
