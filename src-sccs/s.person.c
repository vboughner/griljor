h52023
s 00005/00010/01094
d D 1.8 92/08/28 11:32:45 vbo 8 7
c updated this files use of inventory and hand objects
e
s 00000/00000/01104
d D 1.7 92/08/07 01:03:10 vbo 7 6
c source copied to a separate tree for work on new map and object format
e
s 00009/00012/01095
d D 1.6 91/12/16 16:45:32 labc-4lc 6 5
c changed teams_represented() function
e
s 00067/00000/01040
d D 1.5 91/12/07 17:37:48 labc-4lc 5 4
c made error message window bigger and scrollable
e
s 00002/00001/01038
d D 1.4 91/09/03 21:39:11 labb-3li 4 3
c Trevor's misc changes - fixed missile heap
e
s 00002/00001/01037
d D 1.3 91/08/26 00:34:23 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00016/00000/01022
d D 1.2 91/08/04 21:34:43 labc-3id 2 1
c added code for many of the limitation properties
e
s 01022/00000/00000
d D 1.1 91/02/16 12:55:23 labc-3id 1 0
c date and time created 91/02/16 12:55:23 by labc-3id
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

/* Routines to handle various "persons" that make up a player or monster */

#define PERSON_MAIN

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
#include "level.h"


/* Global variables */

/* Persons defined under this player */
int	num_persons = 0;		/* how many persons in use */
Person	*person[MAX_PLAYER_SIZE];	/* storage of persons' information */

/* array of persons in the game, by game id number */
int		persons_in_game = 0;		/* how many persons in game */
GamePerson	*gameperson[MAX_GAME_SIZE];	/* everyone */

/* array of lists of persons in certain rooms */
PersonList	*room_persons;	/* an array with a list for each room */



/* variables for use in this file only */

char *default_bitmap, *default_mask;	/* default bitmap and mask */




/* ============== Persons under   P L A Y E R  ========================== */

/* initialize person pointer array to NULL's */

initialize_person_array()
{
  int i;

  for (i=0; i<MAX_PLAYER_SIZE; i++)
    person[i] = NULL;
}



/* initialize a given person's stats to zero */

initialize_person(num)
int num;
{
  /* see if there is already a record allocated for this person,
     if not, then allocate one */
  if (person[num] == NULL)  person[num] = allocate_person();

  person[num]->health = BASE_HEALTH;
  person[num]->power = BASE_POWER;
  person[num]->experience = BASE_EXP;
  person[num]->death_timer = -1;
  person[num]->fire_again = 0;
  person[num]->future_hand = -1;
I 4
  ClearDamageRecord(num);
E 4

  /* empty out inventory */
  empty_person_inventory(num);

  clear_person_messages(num);
I 5
  clear_person_error_messages(num);
E 5

  /* clear the packet queue of unread mail */
  person[num]->pqueue = NULL;
  person[num]->final = NULL;
}




I 5
/* clear the saved error message array for a person */

clear_person_error_messages(num)
int num;
{
  int i;

  for (i=0; i<MAX_ERROR_MESSAGES; i++) person[num]->errmsg[i] = NULL;
}



E 5
/* clear the saved message array for a person */

clear_person_messages(num)
int num;
{
  int i;

  for (i=0; i<MAX_MESSAGES; i++) person[num]->msg[i] = NULL;
}



D 8
/* empty a person's inventory */
E 8
I 8
/* empty a person's inventory and hands */
E 8

empty_person_inventory(num)
int num;
{
  int i;

D 8
  for (i=0; i<INV_SIZE; i++) {
      person[num]->inventory[i].type = 0;
      person[num]->inventory[i].record = NULL;
    }
E 8
I 8
  for (i=0; i<INV_SIZE; i++)
    person[num]->inventory_object[i] = NULL;
E 8

D 8
  /* empty out hands */
  for (i=0; i<2; i++) {
    person[num]->hand[i].type = 0;  
    person[num]->hand[i].record = NULL;
    }
E 8
I 8
  for (i=0; i<2; i++)
    person[num]->hand_object[i] = NULL;
E 8
}



/* clear the queue containing unread mail */

clear_person_packet_queue(num)
int num;
{
  QPacket *ptr, *new;

  ptr = person[num]->pqueue;

  while (ptr) {
    new = ptr->next;
    free(ptr);
    ptr = new;
  }

  person[num]->pqueue = NULL;
  person[num]->final = NULL;
}



/* allocate memory for a person record */

Person *allocate_person()
{
  Person *new;

  new = (Person *) malloc(sizeof(Person));
  if (new == NULL) Gerror("not enough memory for person record");

  return new;
}



I 5
/* add a message to the person's recent error message list, return the number
   of the message (0 to MAX_ERROR_MESSAGES-1) or return MAX_ERROR_MESSAGES
   if the list had to be scrolled to accomidate the additional message */

int add_person_error_message(num, s)
int num;
char *s;
{
  int  i, found = -1, result = -1;
  char *new;
  
  /* look for the first NULL pointer in the list as a blank spot to put this */
  for (i=0; i<MAX_ERROR_MESSAGES && found == -1; i++)
    if (person[num]->errmsg[i] == NULL) found = i;

  /* if we didn't find a blank spot then scroll all the others upwards */
  if (found == -1) {
    moveup_person_error_messages(num);
    found = MAX_ERROR_MESSAGES - 1;
    result = MAX_ERROR_MESSAGES;
  }
  else result = found;

  /* allocate room for the string and place it there */
  new = allocate_message_string(strlen(s) + 1);
  strcpy(new, s);
  person[num]->errmsg[found] = new;
  
  return result;
}



E 5
/* add a message to the person's recent message list, return the number
   of the message (0 to MAX_MESSAGES-1) or return MAX_MESSAGES if the
   list had to be scrolled to accomidate the additional message */

int add_person_message(num, s)
int num;
char *s;
{
  int  i, found = -1, result = -1;
  char *new;
  
  /* look for the first NULL pointer in the list as a blank spot to put this */
  for (i=0; i<MAX_MESSAGES && found == -1; i++)
    if (person[num]->msg[i] == NULL) found = i;

  /* if we didn't find a blank spot then scroll all the others upwards */
  if (found == -1) {
    moveup_person_messages(num);
    found = MAX_MESSAGES - 1;
    result = MAX_MESSAGES;
  }
  else result = found;

  /* allocate room for the string and place it there */
  new = allocate_message_string(strlen(s) + 1);
  strcpy(new, s);
  person[num]->msg[found] = new;
  
  return result;
}



/* allocate memory for a message string */

char *allocate_message_string(len)
int len;
{
  char *result;

  result = (char *) malloc(len);
  if (result == NULL) Gerror("no memory for message string");

  return result;
}
I 5



/* move all error messages up one space, freeing the top one */

moveup_person_error_messages(num)
int num;
{
  int i;
  
  /* free memory space taken by top most message */
  free(person[num]->errmsg[0]);

  /* move others up */
  for (i=1; i<MAX_ERROR_MESSAGES; i++)
    person[num]->errmsg[i-1] = person[num]->errmsg[i];

  /* clear the last message */
  person[num]->errmsg[MAX_ERROR_MESSAGES-1] = NULL;
}

E 5



/* move all messages up one space, freeing the top one */

moveup_person_messages(num)
int num;
{
  int i;
  
  /* free memory space taken by top most message */
  free(person[num]->msg[0]);

  /* move others up */
  for (i=1; i<MAX_MESSAGES; i++)
    person[num]->msg[i-1] = person[num]->msg[i];

  /* clear the last message */
  person[num]->msg[MAX_MESSAGES-1] = NULL;
}




/* get ready the driver info packet that a new player or monster is
   going to have to send to the driver */

prepare_driver_packet(player, pack)
PlayerInfo *player;
DriverPack *pack;
{
  register int i;
  
  /* load info from player record into driver pack */
  strcpy(pack->info.player_name, player->player_name);
  strcpy(pack->info.password, player->password);
  strcpy(pack->info.our_game, player->our_game);
  strcpy(pack->info.login, player->login);
  strcpy(pack->info.host, player->host);
  pack->info.what_am_i = what_am_i;
  
  /* load info about number of persons we are using into the pack */
  pack->info.num_persons = num_persons;

  /* load data about each person into driver pack */
  for (i=0; i<num_persons; i++)
    load_person_into_driver_pack(i, pack);

  /* if the player has a personalized bitmap, set the savebits flag so
     that driver saves the bitmap in the password file */
  if (am_player && (strcmp(DFLT_BITMAP, player->personal_bitmap) ||
		    strcmp(DFLT_MASK, player->bitmap_mask)))
	for (i=0; i<num_persons; i++)  pack->newperson[i].savebits = TRUE;
}



/* load one person's info into array in driver pack for persons */

load_person_into_driver_pack(num, pack)
int num;
DriverPack *pack;
{
  /* load the person's name into driver pack */
  strcpy(pack->newperson[num].name, gameperson[num]->name);

  /* load the person's bitmap */
  bitmapcpy(pack->newperson[num].bitmap, gameperson[num]->bitmap);

  /* load the person's mask */
  bitmapcpy(pack->newperson[num].mask, gameperson[num]->mask);

  /* blank out experience and rank variable and put in team number */
  pack->newperson[num].experience = BASE_EXP;
  strcpy(pack->newperson[num].rank, RANK_NAME[0]);
  pack->newperson[num].team = gameperson[num]->team;

  /* load default values for listing person in game and save file check */
  pack->newperson[num].deity = FALSE;
  pack->newperson[num].savebits = FALSE;
  pack->newperson[num].loadbits = FALSE;
  pack->newperson[num].listed = gameperson[num]->listed;
  pack->newperson[num].checksave = (am_player);
  pack->newperson[num].loaded = FALSE;	/* will be TRUE if save file loaded */
}



/* remove person from game area and restore his normal appearance */

dead_person_ritual(pnum)
int pnum;
{
  person[pnum]->health = max_person_health(pnum);
I 4
  ClearDamageRecord(pnum);
E 4
  person[pnum]->power = max_person_power(pnum);
  person[pnum]->death_timer = -1;
  gameperson[pnum]->appearance = APPEAR_NORMAL;
  move_person(pnum, -1, -1, -1, TRUE);
}
D 4

E 4


/* set the content of future variables for firing something in the future */

set_future_use(pnum, hand, x, y)
int pnum, hand, x, y;
{
  /* check hand number */
  if (hand < 0 || hand > 1) return;

  /* set the variables */
  person[pnum]->future_hand = hand;
  person[pnum]->future_room = gameperson[pnum]->room;
  person[pnum]->future_x = x;
  person[pnum]->future_y = y;
}


/* ====================== Persons in   G A M E   ======================== */


/* initialize game person pointer array to NULL's */

initialize_game_person_array()
{
  int i;
  int h, w, xh, yh, result;

  /* set all array pointers to NULL */
  for (i=0; i<MAX_GAME_SIZE; i++) gameperson[i] = NULL;


  /* load default bitmap pointers with the appropriate arrays */

  result = XXReadBitmapFile(DFLT_BITMAP, &w, &h, &default_bitmap, &xh, &yh);
  if (result != 1) Gerror("default bitmap not found, check DFLT_BITMAP");

  result = XXReadBitmapFile(DFLT_MASK, &w, &h, &default_mask, &xh, &yh);
  if (result != 1) Gerror("default mask not found, check DFLT_MASK");

}



/* initialize a given game person's stats to zero, and allocate a record
   if neccessary */

initialize_game_person(num)
int num;
{
  /* see if there is already a record allocated for this person,
     if not, then allocate one */
  if (gameperson[num] == NULL)  gameperson[num] = allocate_game_person();

  /* initialize all values to something safe and reasonable */
  gameperson[num]->id = -1;
  gameperson[num]->name[0] = '\0';
  strcpy(gameperson[num]->login, "nobody");
  strcpy(gameperson[num]->host, "nowhere");
  gameperson[num]->room = -1;
  gameperson[num]->x = -1;
  gameperson[num]->y = -1;
  gameperson[num]->ignore_walls = 0;
  gameperson[num]->moveq = NULL;
  gameperson[num]->movewait = CLICKS_PER_MOVE;
  gameperson[num]->lastmove = 0;
  strcpy(gameperson[num]->rank, "Unknown Rank");
  gameperson[num]->level = experience_level(BASE_EXP);
  gameperson[num]->deity = FALSE;
  gameperson[num]->kills = 0;
  gameperson[num]->losses = 0;
  gameperson[num]->games = 0;
  gameperson[num]->weight_limit = MAX_BURDEN;
  gameperson[num]->team = 0;
  gameperson[num]->listed = (am_player);
  gameperson[num]->appearance = APPEAR_NORMAL;
  bitmapcpy(gameperson[num]->bitmap, default_bitmap);
  bitmapcpy(gameperson[num]->mask, default_mask);

  /* assign rank only when there is a player person array present */
  if (num_persons > num) assign_rank(num);
}


/* get a gameperson record ready for restart in a new game, keeping a few
   of the old values like the bitmap and mask.  If there is no gameperson
   by this number then initialize one. */

restart_game_person(num)
int num;
{
  /* see if there is already a record allocated for this person,
     if not, then call initialize_game_person */
  if (gameperson[num] == NULL)  {
    initialize_game_person(num);
    return;
  }

  /* initialize all values that should be cleared with something safe */
  gameperson[num]->id = -1;
  gameperson[num]->room = -1;
  gameperson[num]->x = -1;
  gameperson[num]->y = -1;
  gameperson[num]->ignore_walls = 0;
  gameperson[num]->moveq = NULL;
  gameperson[num]->movewait = CLICKS_PER_MOVE;
  gameperson[num]->lastmove = 0;
  gameperson[num]->level = experience_level(BASE_EXP);
  gameperson[num]->kills = 0;
  gameperson[num]->losses = 0;
  gameperson[num]->weight_limit = MAX_BURDEN;
  gameperson[num]->games = 0;
  gameperson[num]->team = 0;
  gameperson[num]->listed = (am_player);
  gameperson[num]->appearance = APPEAR_NORMAL;

  /* assign rank only when there is a player person array present */
  if (num_persons > num) assign_rank(num);
}



/* assign rank to a gameperson according to number of experience points
   found in the player person record ----
   NOTE: experience points no longer determine rank, so this routine
   has been gutted. */

assign_rank(num)
int num;
{
/*******
  if (person[num]->experience > BASE_EXP)
    strcpy(gameperson[num]->rank, "Experienced");
  else
    strcpy(gameperson[num]->rank, "No rank");

  gameperson[num]->level = experience_level(person[num]->experience);
*******/
}



/* Load info from a driver pack person into the game person array, adding
   to the array.  This routine is used only by the driver for these are
   incomplete records. */

miniperson_to_gameperson(mini, pack)
MiniPerson *mini;	/* pointer to miniperson inside info pack */
DriverPack *pack;	/* the driver info pack this miniperson came from */
{
  int num;
  
  if (DEBUG) printf("Adding %s to gameperson array\n", mini->name);

  /* make a gameperson record for this person */
  num = persons_in_game;
  initialize_game_person(num);
  persons_in_game++;

  /* copy over the pertinent information */
  gameperson[num]->id = mini->id;
  gameperson[num]->parent = mini->parent;
  strcpy(gameperson[num]->name, mini->name);
  strcpy(gameperson[num]->rank, mini->rank);
  gameperson[num]->level = experience_level(mini->experience);
  gameperson[num]->team = mini->team;
  gameperson[num]->deity = mini->deity;
  strcpy(gameperson[num]->login, pack->info.login);
  strcpy(gameperson[num]->host, pack->info.host);

  /* copy over the bitmap and mask */
  bitmapcpy(gameperson[num]->bitmap, mini->bitmap);
  bitmapcpy(gameperson[num]->mask, mini->mask);

  /* copy over the persons network address */
  bcopy(&(pack->info.address), &(gameperson[num]->address), sizeof(Sockaddr));
}



/* given a pointer to a game person structure, add this person to the
   gameperson array by copying the data into an initialized gameperson
   array slot */

add_game_person(gp)
GamePerson *gp;
{
  int num;

  /* get a spot to place the new game person */
  num = persons_in_game;
  initialize_game_person(num);
  persons_in_game++;

  /* copy gameperson into initialized location */
  bcopy(gp, gameperson[num], sizeof(GamePerson));
}



/* given a gameperson number, delete him from our gameperson array.  To do
   this move all the the pointers in the array upward toward zero and free
   the deleted person's gameperson record */

delete_game_person(num)
int num;
{
  register int	i;
  
  /* check that it is a valid person to delete and not one of our own */
  if (DEBUG) printf("Deleting gameperson element %d person %d\n", num,
		    gameperson[num]->id);
  if (num < num_persons) {
    if (DEBUG) printf("Warning: tried to delete one of our own\n");
    return;
  }
  else if (num<0 || num >= persons_in_game) {
    if (DEBUG) printf("Warning: tried to delete non-existant person\n");
    return;
  }

  /* if we have a display, delete the person's pixmap from pixmap list */
  if (has_display) delete_person_pixmap(num);

  /* free the old game person record */
  free(gameperson[num]);

  /* copy everyone else upwards */
  for (i=num+1; i<persons_in_game; i++) gameperson[i-1] = gameperson[i];

  /* fix up the end of the array */
  gameperson[persons_in_game-1] = NULL;
  persons_in_game--;
  

  /* print new list of persons in game */
  if (DEBUG) print_all_game_persons();
}




/* allocate memory space for a game person array element */

GamePerson *allocate_game_person()
{
  GamePerson *new;

  new = (GamePerson *) malloc(sizeof(GamePerson));
  if (new == NULL) Gerror("not enough memory for person record");

  return new;
}



/* load the bitmap and mask for a game person into his record */

load_person_bitmap_and_mask(num, bitfile, maskfile)
int num;
char *bitfile, *maskfile;
{
  char *new;
  int h, w, xh, yh, result;
  
  result =XXReadBitmapFile(bitfile, &w, &h, &new, &xh, &yh);
  if (result != 1) Gerror("personal bitmap file not found");
  bitmapcpy(gameperson[num]->bitmap, new);
  free(new);

  result =XXReadBitmapFile(maskfile, &w, &h, &new, &xh, &yh);
  if (result != 1) Gerror("personal bitmap mask file not found");
  bitmapcpy(gameperson[num]->mask, new);
  free(new);
}



/* print contents of game person array element at num */

print_game_person(num)
int num;
{
  printf("==> %d \t%s \t%s\n", gameperson[num]->id, gameperson[num]->name,
	 gameperson[num]->rank);
  printf("    socket address %d at port %d\n",
	 gameperson[num]->address.sin_addr, gameperson[num]->address.sin_port);
  printf("    login:  %s@%s\n", gameperson[num]->login, gameperson[num]->host);
}



/* print contents of entire game person array */

print_all_game_persons()
{
  register int i;

  for (i=0; i<persons_in_game; i++) print_game_person(i);
}



/* compute the number of listed persons in this game at the moment */

int number_of_listed_persons()
{
  register int	i;
  int		result = 0;

  for (i=0; i<persons_in_game; i++)
    if (gameperson[i]->listed) result++;

  return result;
}



/* return TRUE if the given person id number is one of my persons */

int one_of_me(id)
int id;
{
  int result = FALSE;
  register int i;

  for (i=0; i<num_persons && !result; i++)
    if (gameperson[i]->id == id) result = TRUE;

  return result;
}



/* return TRUE if the given location is immediately next to this person */

int person_next_to(pnum, roomnum, x, y)
int pnum, roomnum, x, y;
{
  return ((roomnum == gameperson[pnum]->room) &&
	  (abs(x - gameperson[pnum]->x) < 2) &&
	  (abs(y - gameperson[pnum]->y) < 2));
}



/* ===================== P E R S O N S  in rooms ======================= */

/* Initialize the persons in room array now that we know how many rooms
   there are in the map (that information is in the global structure
   "mapstats") */

initialize_room_person_array()
{
  register int i;

  if (DEBUG) printf("Initializing room person array\n");

  /* allocate space for the array */
  room_persons = (PersonList *) malloc(sizeof(PersonList) * mapstats.rooms);
  if (room_persons == NULL) Gerror("no memory for room person array");

  /* initialize all next pointers to NULL as if there were no one in any
     of the rooms in the map (each array element is a dummy record that
     makes deletion more uniform) */
  for (i=0; i<mapstats.rooms; i++) room_persons[i].next = NULL;
}



/* Initialize the room_persons array to reflect the situation in the
   game.  Go through all of the persons in the game and put them into
   the right room.  This can be done anytime the array is not correct
   or has never been setup.  Also, free any records that are already
   in use in the array. */

assign_persons_to_rooms()
{
  register int	i;
  PersonList	*new;

  /* free the previous contents of the array */
  free_room_person_array();

  /* assign each person in game to their respective room */
  for (i=0; i<persons_in_game; i++)  assign_person(i);
}



/* free the previous contents of the room_person array (the array must have at
   least been initialized once before with initialize_room_person_array) */

free_room_person_array()
{
  register int	i;
  PersonList	*ptr, *last;

  /* notify user we are freeing the room person array */
  if (DEBUG) printf("Freeing old contents of room person array\n");

  /* attempt to free each list */
  for (i=0; i<mapstats.rooms; i++)
    for (ptr=room_persons[i].next; (ptr != NULL); ) {
      last = ptr;
      ptr = ptr->next;
      free(last);
    }
}



/* Assign a given person (by element number) to the room list where he
   should reside.  If there are no other siblings there (persons with same
   parent as he) then he should be marked as important. */

assign_person(num)
int num;
{
  int only_child;
  
  if (DEBUG) printf("Assigning person (id %d) to room %d\n",
		    gameperson[num]->id, gameperson[num]->room);

  /* if person's room number is -1 then he is not in a room, don't assign */
  if (gameperson[num]->room < 0) return;

  /* look in his room for siblings */
  only_child = !found_siblings(gameperson[num]->room, gameperson[num]->parent);
  
  /* add him to the room */
  add_person_to_room(gameperson[num]->room, num, only_child);
}



/* look for siblings in a room, given a parent and room to look in.  Return
   TRUE when any persons with this parent are found in the room. */

int found_siblings(room, parent)
int room, parent;
{
  PersonList	*ptr;
  int		found = FALSE;

  /* look through room for siblings */
  for (ptr = room_persons[room].next; (ptr && !found);  ptr = ptr->next)
    if (ptr->person->parent == parent) found = TRUE;

  return found;
}



/* add a person to a room given whether or not he should be marked as
   important or not */

add_person_to_room(room, num, of_importance)
int room, num, of_importance;
{
  PersonList	*new;

  /* allocate a new room person record */
  new = (PersonList *) malloc(sizeof(PersonList));
  if (new == NULL) Gerror("no memory for a room person record");

  /* set up the record */
  new->person = gameperson[num];
  new->important = of_importance;

  /* place the record at front of room list */
  new->next = room_persons[room].next;
  room_persons[room].next = new;
}



/* remove a persons record from a given room and free it.  Look for other
   siblings in the room and transfer importance marker if neccessary. */

remove_person_from_room(room, num)
int room, num;
{
  PersonList	*old, *ptr, *found = NULL;

  /* check for -1 room value */
  if (room < 0) return;

  /* look through room list for the person */
  old = &(room_persons[room]);
  for (ptr = old->next; (ptr && (!found)); old = ptr , ptr = ptr->next)
    /* if we find him then remove him from the list */
    if (ptr->person->id == gameperson[num]->id) {
      found = ptr;
      old->next = ptr->next;
    }

  if (found) {
    /* if he was important then get a sibling to take over importance */
    if (found->important)
      make_sibling_important(room, gameperson[num]->parent);

    /* free the record */
    free(found);
  }
}



/* set a sibling to be important in the given room */

make_sibling_important(roomnum, parent)
int roomnum, parent;
{
  PersonList *ptr, *found = NULL;

  /* make the first sibling we find important */
  for (ptr = room_persons[roomnum].next; (ptr && (!found)); ptr = ptr->next)
    if (ptr->person->parent == parent) {
      found = ptr;
      found->important = TRUE;
    }
}



/* return TRUE when the given person is marked as important in his room,
   or FALSE when not important or non-existent */

int is_marked_important(num)
int num;
{
  PersonList	*ptr;
  int		result = -1;

  for (ptr = room_persons[gameperson[num]->room].next;
       (ptr && result == -1); ptr = ptr->next)
	    if (ptr->person->id == gameperson[num]->id)
		result = ptr->important;

  if (result == -1) result = FALSE;
  return result;
}



/* when a person has changed rooms, call this procedure to update the
   room person array.  We shall use values of "oldroom" and "room" in
   the gameperson record to figure out what has gone on. */

person_has_changed_rooms(num)
int num;	/* element number */
{
  /* check if room has really changed */
  if (gameperson[num]->oldroom == gameperson[num]->room) return;
  
  /* delete this person from old room */
  remove_person_from_room(gameperson[num]->oldroom, num);

  /* assign the person to their new room */
  assign_person(num);
}



/* ========================= T E A M  stuff ============================== */

/* print out information on the teams in the game, excluding person
   given (give -1 if you don't want anyone excluded from count) */

print_teams(pnum)
int pnum;
{
  int i, team_total[NUM_OF_TEAMS + 1];
  char name[TEAM_NAME_LENGTH];

  /* count how many on each team */
  for (i=0; i<=NUM_OF_TEAMS; i++) team_total[i] = 0;
  for (i=0; i<persons_in_game; i++)
    if (gameperson[i]->team >= 0 && 
	gameperson[i]->team <= mapstats.teams_supported &&
	gameperson[i]->listed && i != pnum)
		team_total[gameperson[i]->team]++;

  /* print team names and number of players on each */
  printf("\n===#==Team Name================================================");
  printf("==========Size==\n");
  for (i=0; i<=mapstats.teams_supported; i++) {
    if (!i) strcpy(name, "neutral");
    else strcpy(name, mapstats.team_name[i-1]);

    printf("   %d  %s  %d\n", i, left_justify(name, 65), team_total[i]);
  }
  printf("===============================================================");
  printf("================\n\n");
}




/* return the number of listed players on a team */

int players_on_team(tnum)
int tnum;
{
D 6
  register int i;
E 6
I 6
  int i;
E 6
  int result = 0;

  for (i=0; i<persons_in_game; i++)
D 6
    if (gameperson[i]->team == tnum && 
	gameperson[i]->team <= mapstats.teams_supported &&
	gameperson[i]->listed) result++;
E 6
I 6
    if (gameperson[i]->team == tnum && gameperson[i]->listed) result++;
E 6

  return result;
}



I 2
D 6
/* return TRUE if there is a representative of each team in the game
   (except for neutral), return TRUE also when there is only one team. */
E 6
I 6
/* return a bitfield showing which teams are represented in the map.
   The '1' bit is for neutral teams, other bits follow for teams 1, 2, etc. */
E 6

D 6
int all_teams_represented()
E 6
I 6
int teams_represented()
E 6
{
D 6
  int i, result = TRUE;
E 6
I 6
  int i, result = 0;
E 6

D 6
  if (mapstats.teams_supported != 1) {
    for (i=1; i<=mapstats.teams_supported; i++)
    result = (result && players_on_team(i));
  }
E 6
I 6
  for (i=0; i<mapstats.teams_supported; i++)
    if (players_on_team(i)) result = (result | 1<<i);

E 6
  return result;
}



E 2
/* return a total of all listed players in the game */

int players_in_game()
{
  register int i;
  int result = 0;

  for (i=0; i<= mapstats.teams_supported; i++)
    result += players_on_team(i);

  return result;
}



/* get a team name */

D 3
char *get_team_name(int num)
E 3
I 3
char *get_team_name(num)
int num;
E 3
{
  static char *result;

  if (num < 0 || num > mapstats.teams_supported) result = "none";
  else if (num == 0) result = "neutral";
  else result = mapstats.team_name[num - 1];

  return result;
}



/* ====================== G A M E   R E S T A R T ===================== */

/* this routine frees the person arrays ('person' and 'gameperson' arrays)
   This is done in order to allow the game to be restarted.  It also clears
   out the room_person array as well.   It leaves just 'num' entries in
   the 'person' and 'gameperson' arrays intact: this player's person(s). */

trim_person_arrays(num)
int num;
{
  int i;
  PersonList *ptr, *erase;

  /* if player number is too big, error out */
  if (num > MAX_PLAYER_SIZE) Gerror("trim_person_arrays(): limit execeeded");

  /* be sure person arrays are filled up to 'num' and initialize */
  for (i=0; i<num; i++) {
    initialize_person(i);
    restart_game_person(i);
  }

  /* free all array contents no longer needed in person array */
  for (i=num; i<num_persons; i++)
    if (person[i]) {
      free(person[i]);
      person[i] = NULL;
    }

  /* free all array contents no longer needed in gameperson array */
  for (i=num; i<persons_in_game; i++)
    if (gameperson[i]) {
      free(gameperson[i]);
      gameperson[i] = NULL;
    }

  /* reset the global person counters */
  num_persons = num;
  persons_in_game = num;

  /* free the room person array and all its contents */
  for (i=0; i<mapstats.rooms; i++)
    for (ptr=room_persons[i].next; ptr; ) {
      erase = ptr;
      ptr = ptr->next;
      free(erase);
    }
  free(room_persons);	
  room_persons = NULL;
}



/* remove the gameperson arrays for the driver */

free_all_game_persons()
{
  int i;

  for (i=0; i<persons_in_game; i++)
    if (gameperson[i]) {
      free(gameperson[i]);
      gameperson[i] = NULL;
    }

  persons_in_game = NULL;
}
E 1
