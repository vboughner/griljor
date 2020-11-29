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

/* Procedures to provide a nice programming language for monsters */

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
#include "gamelist.h"
#include "gametime.h"


/* definition of a person */

typedef struct _init_person {
	char	name[NAME_LENGTH];	/* this monster's name */
	char	bitmap[BITMAP_ARRAY_SIZE];
	char	mask[BITMAP_ARRAY_SIZE];
	int	alive;			/* is TRUE when monster is in game */
	int	listed;			/* TRUE if we are to be listed */
	int	level;			/* the level of the monster */
} InitPerson;



/* global variables for this file */

PlayerInfo *m_info = NULL;	/* pointer to info record of this monster */
InitPerson *monster[MAX_PLAYER_SIZE]; /* list of persons programmer wants */
int	   num_monsters = 0;	/* number of persons he has asked for */
int	   game_started = 0;	/* becomes positive once game starts */


/* macros and constants */

#define BAD_NUM(num)	((num) < 0  ||  (num) >= num_monsters)
#define	TIMEOUT_MS	200	/* timeout for monster mail check */


/* =================== G R I L J O R  internal routines ================== */

/* main monster program passes monster info record to us at the beginning */
pass_info_record(PlayerInfo *ptr)
{
  m_info = ptr;
}


/* load the monster names into their new person records */

load_monsters_into_persons()
{
  register int i;

  for (i=0; i<num_monsters; i++) {
    strcpy(gameperson[i]->name, monster[i]->name);
    strcpy(gameperson[i]->login, m_info->login);
    strcpy(gameperson[i]->host, m_info->host);
    gameperson[i]->team = m_info->team;
    gameperson[i]->level = monster[i]->level;
    gameperson[i]->listed = monster[i]->listed;
    bitmapcpy(gameperson[i]->bitmap, monster[i]->bitmap);
    bitmapcpy(gameperson[i]->mask, monster[i]->mask);
  }
}


/* check to see if any of our monsters has become either dead or alive since
   the last time we checked them, if so then update the dead/alive flag,
   or in the case of death, see that they look like a tombstone for a while */

check_monsters_breathing()
{
  int i;

  for (i=0; i<num_monsters; i++)
    if (monster[i]->alive && person[i]->health <= 0) {
      MDeath(i);
    }
    else if (!monster[i]->alive && person[i]->health > 0) {
      monster[i]->alive = TRUE;
    }
}


/* ================== Monster  L A N G U A G E  routines ================= */

/* The point of these routines is that the makers of monster programs
   will not need to ever call any routines inside of any of the other
   griljor source code.  The routines that follow are the only ones
   they should ever call. */

/* The programmer must call this once for each of the persons he wishes
   to be allocated for him during the game.  He must finish all his
   calls to this procedure before calling MEnterGame().
   Any one of the parameters may be passed as NULL, and the default
   will be used, or as name, the name given to the monster originally. */

MInit(char *name, int level, int listed, char *bitmap, char *mask)
{
  InitPerson *new;
  
  if (num_monsters >= MAX_PLAYER_SIZE)
    Gerror("defining too many monsters, change MAX_PLAYER_SIZE");

  if (game_started)
    Gerror("all monsters must be initialized before entering game");

  new = (InitPerson *) malloc(sizeof(InitPerson));
  demand(new, "out of memory initializing person");

  /* initialize the values of new person record */
  if (name) strcpy(new->name, name);
  else strcpy(new->name, m_info->player_name);
  new->level = level;
  new->listed = listed;
  if (bitmap) bitmapcpy(new->bitmap, bitmap);
  else load_bitmap(m_info->personal_bitmap, new->bitmap);
  if (mask) bitmapcpy(new->mask, mask);
  else if (bitmap) bitmapcpy(new->mask, make_mask(bitmap));
  else load_bitmap(m_info->bitmap_mask, new->mask);
  new->alive = FALSE;

  /* add new person to the end of the list */
  monster[num_monsters] = new;
  num_monsters++;
}



/* Once the programmer has initialized his persons, he must enter the game
   by calling this procedure, which communicates with the driver and does
   most of the setup */

MEnterGame()
{
  int got_in;

  if (!num_monsters) Gerror("attempt to start game with no monsters");
  if (game_started) Gerror("attempt to enter game a second time");

  /* startup vital arrays for future book-keeping */
  startup_player_persons(num_monsters);
  load_monsters_into_persons();

  /* try to get into the game command line specified for us */
  ignore_termination();
  got_in = try_to_access_game(m_info);

  /* if we didn't get in then we can go ahead and give up */
  if (got_in == -2) Gerror("driver rejected monster game entry");
  if (got_in < 0) Gerror("specify correct game monster is supposed to join");

  /* initialize some more vital book-keeping arrays */
  safety_termination(NULL);
  initialize_room_person_array();
  assign_persons_to_rooms();
  initialize_missile_array();
  initialize_hit_list(mapstats.rooms);
  initialize_important_object_list();

  /* now would be the time to select a team, we'll just leave it at default */

  /* set up the windows if this monster is to be viewed */
  if (has_display) {
    start_player_windows();
    setup_player_variables();
    /* note that visibility stuff will have to be moved out of setup_pl.. */
    get_game_news();
  }

  /* startup timer */
  init_game_time();
  game_started = TRUE;
}



/* Pick a random starting location for a monster, and bring him
   to life, this can only be done when the monster is not currently alive */

MBirth(int num)
{
  if (BAD_NUM(num)) Gerror("bad monster number to MBirth");
  if (monster[num]->alive) Gerror("attempt to use MBirth on living monster");
  if (!game_started) Gerror("must enter a game before using MBirth");

  select_person_place(num);
  monster[num]->alive = TRUE;
}


/* Kill this monster, this routine will need to be called when the monster
   is supposed to die, it will start the 4-5 second wait while the
   monster's tombstone hangs around */

MDeath(int num)
{
  if (BAD_NUM(num)) Gerror("bad monster number to MDeath");
  if (!monster[num]->alive) Gerror("used MDeath on already dead monster");
  if (!game_started) Gerror("must enter a game before using MDeath");

  setup_death(num);
  monster[num]->alive = FALSE;
}


/* This routine needs to be called as often as possible by the programmer's
   monster, in order to read current mail, update the X window if there
   is one, and keep abreast of game events as they happen.  Normally
   returns FALSE, but returns TRUE when the game is over. */

MUpdate()
{
  register int i;
  int done = FALSE;

  if (!game_started) Gerror("game must be entered before calling MUpdate");

  /* update the time counter */
  update_game_time();

  /* check for game over countdown expiration */
  if (IS_GAME_OVER || NOT_RECEIVING) done = TRUE;

  if (!done) {
    /* handle the movement all the persons controlled by this player */
    move_persons();

    /* handle the movement of all missiles in the room */
    update_missiles();

    /* remove old hits from display */
    remove_old_hits();

    /* load any network packet mail that has come in recently */
    /* wait for some mail to come in before doing anything */
    if (WaitForIO(TIMEOUT_MS)) BossIO();

    /* update the time counter */
    update_game_time();

    /* have each person handle their own mail */
    for (i=0; i<num_persons; i++) handle_person_packet_queue(i);

    /* now look for screen refresh events and such, if we are supporting
       a display that humans can watch their monster on */
    if (has_display) player_get_input();

    /* check for object use commands from the past */
    for (i=0; i<num_persons; i++) use_if_ready(i);

    /* check for futurized person events */
    check_person_events();

    /* check to see if any of our persons has become dead or alive */
    check_monsters_breathing();
  }

  /* update the time counter */
  update_game_time();

  return done;
}



/* Once the programmer has determined he wants to leave the game, he
   should call this procedure.  In many cases this procedure will be
   called for him, eg.  when the game ends due to somebody winning,
   all monsters will auto quit */

MLeaveGame()
{
  ignore_termination();
  endBoss();
  game_started = FALSE;
}



/* returns TRUE if game is in progress */

int MQGameGoing()
{
  return (game_started);
}


/* =========================== M O V E M E N T =========================== */

/* PRIVILEDGED: This movement routine can be used for initial placement of
   monster on the map at a chosen point, or it may be used to teleport
   him around.  If called with invalid coordinates, the monster's
   position will still change, but it will mean that he will no longer
   really be in the realms of the game. */

MTeleportMove(int num, int roomnum, int x, int y)
{
  if (!game_started) Gerror("must enter a game before using MTeleportMove");
  if (BAD_NUM(num)) Gerror("bad monster number to MTeleportMove");

  move_person(num, roomnum, x, y, TRUE);
}



/* This is the normal straight line movement routine.  You specify a
   location in the same room you wish monster to move toward.
   If you specify a spot out of range you will be able to exit the room. */

MLineMove(int num, int x, int y)
{
  if (!game_started) Gerror("must enter a game before using MLineMove");
  if (BAD_NUM(num)) Gerror("bad monster number to MLineMove");
  if (!monster[num]->alive) {
    printf("Warning: attempt to move a dead monster\n");
    return;
  }

  request_movement(num, x, y, FALSE);
}



/* This procedure returns TRUE when your previously requested movement
   has come to a conclusion, either by you hitting an obstacle,
   dying, or ending up at the right location.  It returns FALSE if this
   monster is still attempting to get there. */

int MQMoveDone(int num)
{
  if (!game_started) Gerror("must enter a game before using MMoveDone");
  if (BAD_NUM(num)) Gerror("bad monster number to MQMoveDone()");
  return (gameperson[num]->moveq == NULL);
}



/* These routines return the position of given monster */

int MQRoom(int num)
{
  if (BAD_NUM(num)) Gerror("bad monster number to MQRoom()");
  return (gameperson[num]->room);
}

int MQX(int num)
{
  if (BAD_NUM(num)) Gerror("bad monster number to MQX()");
  return (gameperson[num]->x);
}

int MQY(int num)
{
  if (BAD_NUM(num)) Gerror("bad monster number to MQY()");
  return (gameperson[num]->y);
}


/* These routines return the nth next position of the given monster.
   ie. with n=1 they return very next position, with n=2 they return
       the position monster will go to after that, and so forth. 
   -100 is returned when there is no planned move that far ahead. */

int MQNextX(int num, int n)
{
  int i, result;
  Location *ptr;

  if (BAD_NUM(num)) Gerror("bad monster number to MQNextX()");
  if (n < 1 || n > 500) Gerror("bad n to  MQNextX");

  for (ptr=gameperson[num]->moveq,i=1; (ptr && i<n); ptr=ptr->next,i++);
  if (!ptr) result = -100;
  else result = ptr->x;

  return result;
}

int MQNextY(int num, int n)
{
  int i, result;
  Location *ptr;

  if (BAD_NUM(num)) Gerror("bad monster number to MQNextY()");
  if (n < 1 || n > 500) Gerror("bad n to  MQNextY");

  for (ptr=gameperson[num]->moveq,i=1; (ptr && i<n); ptr=ptr->next,i++);
  if (!ptr) result = -100;
  else result = ptr->y;

  return result;
}


/* =========================== C O M B A T ============================== */

/* returns TRUE if a particular square has a missile headed for it, that we
   know about.  Will return FALSE if there is no missile headed there, or
   we have no knowledge of any missiles in the given room.  Range is the
   distance you wish to be knowledgeable about.  Eg.  A range of 5 will
   cause this routine to return FALSE when the only missile that will hit
   the square is 6 squares away or more.  A range of 0 signifies a desire
   to know about ANY missile, no matter how far.  A negtive range is an
   error. */

MQBangSquare(int roomnum, int x, int y, int range)
{
  if (!game_started) Gerror("must enter a game before using MBangSquare");
  if (OUT_OF_BOUNDS(roomnum, x, y)) return FALSE;
  if (range < 0) Gerror("negative range fed to MQBangSquare()");
  if (!range) range = max(ROOM_WIDTH, ROOM_HEIGHT);

  return targeted_square(roomnum, x, y, range);
}



/* return the person id if there is a person next to this spot, otherwise
   returns -1.  The x and y location of the person are transferred to the
   passed variables if there was someone.  The spot itself is not checked.
   If you don't want the return values for x and y, just pass NULL as the
   parameters in the first place. */

int MQWhoNextToSpot(int roomnum, int x, int y, int *nearx, int *neary)
{
  PersonList	*ptr;
  int result = -1, done = FALSE;

  if (!game_started) Gerror("must enter a game before using MQWhoNextToSpot");
  if (OUT_OF_BOUNDS(roomnum, x, y)) return -1;

  /* go through all persons in room, looking for those near this spot */
  for (ptr = room_persons[roomnum].next; (ptr && !done); ptr = ptr->next)
    if (ptr->person->room == roomnum && abs(ptr->person->x - x) < 2 &&
	abs(ptr->person->y - y) < 2 && ptr->person->x != x &&
	ptr->person->y != y) {
		result = ptr->person->id;
		if (nearx) *nearx = ptr->person->x;
		if (neary) *neary = ptr->person->y;
		done = TRUE;
    }

  return result;
}


/* returns the person id of the person on the given square, or -1 if there
   is no one there */

int MQWhoHere(int roomnum, int x, int y)
{
  PersonList	*ptr;
  int result = -1, done = FALSE;

  if (!game_started) Gerror("must enter a game before using MQWhoHere");
  if (OUT_OF_BOUNDS(roomnum, x, y)) return -1;

  /* go through all persons in room, looking for those on this spot */
  for (ptr = room_persons[roomnum].next; (ptr && !done); ptr = ptr->next)
    if (ptr->person->room == roomnum && ptr->person->x == x &&
	ptr->person->y == y) {
		result = ptr->person->id;
		done = TRUE;
    }

  return result;
}



/* PRIVILEDGED: make an explosion occur on a certain square.  FALSE is
   returned if explosion can't be done, TRUE is returned upon success.  You
   need to give the following information:
	num:		number of the monster calling for explosion
	roomnum, x, y:	location of the explosion
	radius:		radius of the explosion (0-20 or so)
			note: a large radius will signifigantly reduce game
			      performance while explosion is drawn
	type:		the type of explosion (0-2 right now) 
	time:		number of game clicks (millisec) until explosion
			occurs (0 to any reasonable number) */

int MExplosion(int num, int roomnum, int x, int y,
	       int radius, int type, long time)
{
  if (!game_started) Gerror("must enter a game before using MExplosion");
  if (BAD_NUM(num)) Gerror("bad monster number to MExplosion()");
  if (OUT_OF_BOUNDS(roomnum, x, y)) return FALSE;
  if (radius < 0) Gerror("radius given to MExplosion must be non-negative");
  if (type < 0) Gerror("explosion type to MExplosion must be non-negative");

/* *** these calls are out of date, will no longer work ***  if (time < 1)
    explode_at_square(gameperson[num]->id, roomnum, x, y, radius, type);
  else
    setup_explosion(num, roomnum, x, y, radius, type, gametime + time); */
}


/* ==================== C O N S T A N T S  etc. ========================= */

/* this returns the standard room width */

int MQRoomWidth()
{
  return ROOM_WIDTH;
}



/* this returns the standard room height */

int MQRoomHeight()
{
  return ROOM_HEIGHT;
}



/* this returns a random valid room x coordinate with chance to leave room */

int MQRandX()
{
  return ((lrand48() % (ROOM_WIDTH + 2)) - 1);
}



/* this returns a random valid room y coordinate with chance to leave room */

int MQRandY()
{
  return ((lrand48() % (ROOM_HEIGHT + 2)) - 1);
}
