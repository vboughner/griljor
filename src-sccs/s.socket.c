h35031
s 00015/00012/00682
d D 1.23 92/09/03 16:13:46 vbo 23 22
c made read and write of map and objects work compatibly
e
s 00012/00040/00682
d D 1.22 92/08/27 20:57:08 vbo 22 21
c changed map read/write functions to deal with new format
e
s 00000/00000/00722
d D 1.21 92/08/07 01:04:11 vbo 21 20
c source copied to a separate tree for work on new map and object format
e
s 00000/00000/00722
d D 1.20 92/01/20 16:30:45 labc-4lc 20 19
c blojo finished changes necesssary for network packets
e
s 00001/01378/00721
d D 1.19 91/12/07 21:40:35 labc-4lc 19 18
c split socket.c into three files and change DAMAGE_REPORT to REPORT
e
s 00007/00007/02092
d D 1.18 91/12/07 17:38:28 labc-4lc 18 17
c made error message window bigger and scrollable
e
s 00039/00026/02060
d D 1.17 91/11/26 14:31:41 labc-4lc 17 16
c fixed invisible/phasing player bug
e
s 00002/00002/02084
d D 1.16 91/10/16 20:06:33 labc-4lc 16 15
c changed made for DecStations
e
s 00003/00002/02083
d D 1.15 91/09/10 13:31:28 labc-4lc 15 14
c 
e
s 00003/00003/02082
d D 1.14 91/09/08 22:47:03 labc-4lc 14 13
c Modifications to ranking code.
e
s 00003/00003/02082
d D 1.13 91/09/08 21:54:05 labc-4lc 13 12
c Modifications for new ranking system.
e
s 00057/00047/02028
d D 1.12 91/09/06 00:35:55 labb-3li 12 11
c added reliability flag to all procedure called send_to_...()
e
s 00000/00000/02075
d D 1.11 91/08/26 00:34:50 vanb 11 10
c fixed up procedure defs and other compatibilty problems
e
s 00002/00001/02073
d D 1.10 91/08/04 16:18:22 labc-3id 10 9
c limit strcpy into message packets
e
s 00003/00055/02071
d D 1.9 91/07/14 14:21:58 labc-3id 9 8
c worked to make loads/saves/transferrals of objects 
e
s 00001/00001/02125
d D 1.8 91/07/07 20:02:10 labc-3id 8 7
c took out references to old target property flag
e
s 00004/00002/02122
d D 1.7 91/05/26 22:43:30 labc-3id 7 6
c Worked on drawing improvements
e
s 00001/00001/02123
d D 1.6 91/05/17 02:04:57 labc-3id 6 5
c Intermediate work on offscreen pixmap
e
s 00033/00002/02091
d D 1.5 91/05/15 05:07:25 labc-3id 5 4
c fixed initial player flicker in room
e
s 00011/00004/02082
d D 1.4 91/04/17 16:54:31 labc-3id 4 3
c made logging of messages store the player names
e
s 00000/00000/02086
d D 1.3 91/03/29 17:24:38 labc-3id 3 2
c 
e
s 00007/00005/02079
d D 1.2 91/03/29 14:07:28 labc-3id 2 1
c 
e
s 02084/00000/00000
d D 1.1 91/02/16 12:55:58 labc-3id 1 0
c date and time created 91/02/16 12:55:58 by labc-3id
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

/* Socket Procedures for all inter-player communications */

#define BOSS_PACKAGE	TRUE	/* this is a flag for use by "socket.h" */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gametime.h"
#include "level.h"
#include "password.h"

#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/time.h>
#include <signal.h>


/* global variables that are useable by the calling program */

D 16
int		Iamboss;	/* TRUE if I am the boss */
E 16
I 16
int		Iamboss;	/* TRUE if I am the driver/server */
E 16
Sockaddr	ownaddr;	/* my own network address */
int		thesocket;	/* my own main socket */
Sockaddr	recaddr;	/* address for receiving new players */
int		recsocket;	/* socket where receiving is done */
Sockaddr	driveraddr;	/* address of drivers datagram socket */
long		end_of_world = GAME_GOING;	/* countdown to game end */
long		last_io = 0;	/* last gameclick io was received on */
char		dmessage[MSG_LENGTH];	/* last message from driver */
I 19
short		game_password;		/* current game's packet password */
E 19

D 19

E 19
D 16
/* internal variable for this file only */
E 16
I 16
/* internal variables for this file only */
E 16
D 15
short		game_password;	/* current game's packet password */
int		initialized = FALSE;	/* TRUE once we have done selfInit */
E 15
I 15
D 19
static short	game_password;		/* current game's packet password */
E 19
static int	initialized = FALSE;	/* TRUE once we have done selfInit */

E 15


/* ===================== S T A R T U P  stuff ========================== */

/* ========================== P L A Y E R ============================== */

/* initialize this package, find the boss, get my own port, and try to
   join or start a game, return TRUE when join is successful.  We are to be
   given the exact location of the driver's new player socket. */

int initBossByAddr(where, with_what)
Sockaddr	*where;		/* which address to look for the boss by */
DriverPack	*with_what;	/* player information */
{
  Iamboss = FALSE;

  /* find my own address and port (if we have not done so earlier) */
  if (!initialized) selfInit();

  /* now that we have a datagram address, put that info into driver pack */
  bcopy(&ownaddr, &(with_what->info.address), sizeof(Sockaddr));

  /* get into the game */
  return join(where, with_what);
}



/* initialize and join game when only a reasonable host name is known,
   return TRUE when the join is successful.  We plan to ask the player/
   driver running on that host where the driver's new player socket is.  */

int initBossByHost(where, with_what)
char		*where;		/* which host to look for the boss on */
DriverPack	*with_what;	/* player information */
{
  Sockaddr	*addr_of_boss;

  Iamboss = FALSE;

  /* find my own address and port */
  if (!initialized) selfInit();

  /* now that we have a datagram address, put that info into driver pack */
  bcopy(&ownaddr, &(with_what->info.address), sizeof(Sockaddr));

  /* find the boss's host address */
  addr_of_boss = specifyBossHost(where);

  /* if boss couldn't be found then return FALSE */
  if (addr_of_boss == NULL) return FALSE;
  
  /* get into the game */
  return join(addr_of_boss, with_what);
}



/* specify the host name where the boss of this program is, and we'll try to
   find out where you can connect with the boss and return the address.
   We return NULL if we can't figure it out. */

Sockaddr *specifyBossHost(name)
char *name;
{
  static Sockaddr addr;

  resolve_inet_address(&addr, name, BOSS_REC_PORT);

  return &addr;
}



/* join an existing game, we know where the boss's new player socket is,
   request a connection and load up the game information, return TRUE if
   the join is successful */

int join(bossaddr, with_what)
Sockaddr	*bossaddr;	/* address of driver new player socket */
DriverPack	*with_what;	/* player information */
{
  int joinsock, result;

  /* get a socket on which to talk to driver */
  joinsock = make_driver_connection(bossaddr);
  if (joinsock < 0) {
    if (am_player) joiner_message("Couldn't make socket connection.");
    return FALSE;
  }

  /* commune with the driver, returns TRUE if we get to join the game */
  result = commune_with_driver(joinsock, with_what);

  /* close the socket connection with driver */
  close(joinsock);

  /* if we were able to join the game then we received addresses of old
     players in the game, we must let them know we are in */
  if (result) notify_old_players();

  return result;
}




/* Given the valid socket number and something to send, commune with the
   driver and get all needed info.  Return TRUE or FALSE about whether
   we will be admitted to the game. */

int commune_with_driver(socknum, info_pack)
int socknum;
DriverPack *info_pack;
{
  AcceptancePack acc_pack;
  
  /* give driver our special driver info package (contains name & password) */
  write_driver_pack(socknum, info_pack);
  
  /* read whether we've been accepted into the game or not */
  read_from_socket(socknum, (char *)&acc_pack, sizeof(AcceptancePack));
  strcpy(dmessage, acc_pack.msg);
  if (am_player) joiner_message(dmessage);

  /* return now if we weren't accepted */
  if (!acc_pack.accepted) return FALSE;

  /* load in the driver's datagram address */
  bcopy(&(acc_pack.address), &driveraddr, sizeof(Sockaddr));

  /* load in the datagram packet password for this game */
  game_password = acc_pack.password;

  /* get the driver info pack back from driver */
  read_driver_pack(socknum, info_pack);

  /* convert new info in driver pack to usable form in person arrays */
  move_pack_to_persons(info_pack);

D 23
  /* get the map */
  if (am_player) joiner_message("Reading and initializing map...");
E 23
I 23
  /* get the map and object definitions */
E 23
  read_map_from_socket(socknum);

D 23
  /* get the object definitions */
  if (am_player) joiner_message("Loading object definitions...");
D 9
  read_objects_from_socket(socknum);
E 9
I 9
  receive_obj_file(socknum);
E 9

E 23
  /* get info about other persons in the game */
  if (am_player) joiner_message("Opening communication with other players...");
  read_persons_in_game(socknum);

  /* get miscellaneous other info */
  read_miscellaneous(socknum);

  /* return TRUE for we were accepted into the game */
  IO_RECEIVED();
  return TRUE;
}



/* move the new contents in a game info pack into the player's person
   arrays (this reflect some of the new info given back by the driver,
   like id numbers, bitmaps, and masks)  The player's persons should
   also be loaded with the correct network address. */

move_pack_to_persons(pack)
DriverPack *pack;
{
  register int i;
  
  if (DEBUG) printf("Using new data about self from driver\n");

  /* go through each person in info pack and get the new data */
  for (i=0; i < pack->info.num_persons; i++) {
    /* get the driver-assigned id number for this person and parent */
    gameperson[i]->id = pack->newperson[i].id;
    gameperson[i]->parent = pack->newperson[0].id;

    /* copy the person's network address in */
    bcopy(&(pack->info.address), &(gameperson[i]->address), sizeof(Sockaddr));

    /* if driver managed to retrieve saved info on this person then load it */
    if (pack->newperson[i].loaded) {
      if (num_persons > i) {
        person[i]->experience = pack->newperson[i].experience;
        gameperson[i]->level = experience_level(person[i]->experience);
	gameperson[i]->weight_limit = max_person_burden(i);
      }
      strcpy(gameperson[i]->rank, pack->newperson[i].rank);
      gameperson[i]->deity = pack->newperson[i].deity;
      gameperson[i]->kills = pack->newperson[i].kills;
      gameperson[i]->losses = pack->newperson[i].losses;
      gameperson[i]->games = pack->newperson[i].games;
      if (pack->newperson[i].loadbits) {
        /* load the bitmap and mask driver has for us */
	bitmapcpy(gameperson[i]->bitmap, pack->newperson[i].bitmap);
	bitmapcpy(gameperson[i]->mask, pack->newperson[i].mask);
      }
    }
  }
}



/* ============================ D R I V E R ============================= */

/* this procedure is to be called only when you really should be the boss,
   it starts up your sockets and such, call only once. */

initSelfBoss()
{
  Iamboss = TRUE;

  /* find my own address and port */
  if (!initialized) {
    selfInit();	  /* initialized is set to TRUE in selfInit() */

    /* figure out what my address will be */
    resolve_inet_address(&recaddr, NULL, BOSS_REC_PORT);

    /* make myself a socket for accepting new players */
    recsocket = make_listen_socket(&recaddr);
    if (recsocket < 0) Berror("couldn't get BOSS_REC_PORT");

    /* choose a packet password for this driver's games */
    game_password = (short) lrand48();
  }
}



/* check for a new player, if there is one, connect with him and give him all
   data, otherwise just return.  When there is a new player, we must be
   sure to add him and his persons to our person array.  Return TRUE when
   we have received any new player's request. */

int checkNewPlayer()
{
  int		newsock, result;
  
  /* check to see if there is anything readable on the accepting socket */
  result = readable_on_socket(recsocket, DRIVER_SLEEP * 1000);

  /* if there is nothing readable then there is no new player waiting */
  if (!result) return FALSE;

  /* there was something readable, get new player request for game entry */
  newsock = make_new_player_connection(recsocket);

  /* if there was an error now then return quietly */
  if (newsock == -1) return FALSE;

  /* just before communing with new player, let's check last minute mail */
  BossIO();

  /* now welcome the new player into the game (if possible),
     give him game info and add him to our person array */
  commune_with_player(newsock);

  /* note how many persons are now in our game person array */
  if (DEBUG)
    printf("Currently there are %d persons in game\n", persons_in_game);

  /* close up our connection with the now possibly accepted new player */
  close(newsock);

  /* return TRUE because handled a new player request, accepted or not,
     at the same time, set io receipt flags */
  IO_RECEIVED();
  return TRUE;
}



/* exploit the connection made with a new player.  Get his personal info
   and tell him whether he can join the game.  Give him maps and players
   addresses. */

commune_with_player(socknum)
int socknum;
{
  DriverPack info_pack;
  AcceptancePack acc_pack;
  
  /* load the package of info on the player */
  read_driver_pack(socknum, &info_pack);
  if (DEBUG) printf("Communing with new player: %s\n",
		    info_pack.info.player_name);
  
  /* decide whether to let player into game */
  make_acceptance_pack(&info_pack, &acc_pack);

  /* place the game packet password into acceptance pack */
  acc_pack.password = game_password;

  /* send the acceptance/rejection packet to the player */
  write_to_socket(socknum, (char *)&acc_pack, sizeof(AcceptancePack));

  /* if he was rejected then return */
  if (!acc_pack.accepted) return;

I 13
D 14
  /* Calculte his overall rank */
  calculate_rank(&info_pack);

E 14
E 13
  /* add him to our game persons list, and fill in and send his info pack */
  integrate_into_game(socknum, &info_pack);

  /* give him the rest of the game info as well */
  send_game_info(socknum, &info_pack);
}
D 13



E 13

/* add this player's persons to our list and fill in his id numbers.  Send
   him the updated info package (updated from what he gave us) */

integrate_into_game(socknum, info_pack)
int socknum;
DriverPack *info_pack;
{
  if (DEBUG) printf("Give player his person id numbers\n");

  /* fill in his info pack with id numbers and any saved info about him */
  assign_person_ids(info_pack);

  /* add his persons to our array */
  add_persons_with_ids(info_pack);
  if (DEBUG) print_all_game_persons();
I 14

  /* Calculte his overall rank */
  calculate_rank(info_pack);
E 14

  /* send new player his updated info pack */
  write_driver_pack(socknum, info_pack);
}



/* give the persons assigned to a player some unique id numbers and
   assign their parent to be the person in the zeroth array element.
   Also look for saved data on this person and load it if possible. */

assign_person_ids(pack)
DriverPack *pack;
{
  register int i;

  for (i=0; i < pack->info.num_persons; i++) {
    /* assign id number and parent */
    pack->newperson[i].id = alloc_id();
    pack->newperson[i].parent = pack->newperson[0].id;
    pack->newperson[i].loaded = FALSE;

    /* pick up any saved data on this person */
    if (pack->newperson[i].checksave) {
      /* get saved data on person */
      pickup_saved_person_info(pack, i);
    }
  }
  
}



/* place persons with id numbers into our person array */

add_persons_with_ids(pack)
DriverPack *pack;
{
  register int i;

  /* add each person in turn to the driver's gameperson array */
  for (i=0; i < pack->info.num_persons; i++)
    miniperson_to_gameperson((MiniPerson *) &(pack->newperson[i]), pack);

  /* let's also make use of the info stored in the password files */
  copy_passinfo_into_person(pack);
}



/* send new player all pertinent information about this game: map, objects,
   other players, bitmaps etc. */

send_game_info(socknum, info_pack)
int socknum;
DriverPack *info_pack;
{
  /* just before send the map, let's check our mail once again */
  BossIO();

  /* send map */
  write_map_to_socket(socknum);

D 23
  /* send object definitions */
D 9
  write_objects_to_socket(socknum);
E 9
I 9
  send_obj_file(socknum);
E 9

E 23
  /* send other game person infomation (don't repeat his own) */
  write_persons_in_game(socknum, info_pack);

  /* send miscellaneous other game info */
  write_miscellaneous(socknum);
}



/* ================= C O M M O N  (driver and player) ===================== */

/* Start me out, get me a datagram socket, an address, and a port to be at. */

selfInit()
{
  /* figure out our network address */
  resolve_inet_address(&ownaddr, NULL, BOSS_PORT);

  /* get a socket on BOSS_PORT or the next avaliable port */
  thesocket = make_socket_next_port(&ownaddr, SOCK_DGRAM);
  initialized = TRUE;	/* a flag global to this file */

  /* remark on the size datagram packets will be */
  if (DEBUG)
    printf("Datagram sockets will be %d bytes in length\n", sizeof(Packet));
}




/* ================ DriverPack reading and writing ====================== */

/* report on contents of driver pack */

print_driver_pack(pack)
DriverPack *pack;
{
  int i;
  
  printf("Driver pack contains following info:\n");
  printf("Player name:          %s\n", pack->info.player_name);
  printf("Password:             %s\n", pack->info.password);
  printf("Our Game:             %s\n", pack->info.our_game);
  printf("Persons using:        %d\n", pack->info.num_persons);
  printf("Address:		%d\n", pack->info.address.sin_addr);
  printf("Port:			%d\n", ntohs(pack->info.address.sin_port));

  for (i=0; i < pack->info.num_persons; i++) {
    printf("Person %d\n", i);
    printf("  Name:             %s\n", pack->newperson[i].name);
    printf("  Id:               %d\n", pack->newperson[i].id);
    printf("  Parent:           %d\n", pack->newperson[i].parent);
    printf("  Experience:       %d\n", pack->newperson[i].experience);
    printf("  Rank:             %s\n", pack->newperson[i].rank);
  }
}



/* read from a stream socket the contents of a driver pack */

read_driver_pack(socknum, pack)
int socknum;
DriverPack *pack;
{
  register int i;
  
  /* read the information about the player */
  read_from_socket(socknum, (char *)(&(pack->info)), sizeof(MiniPlayer));

  /* read the information on each of the player's persons */
  for (i=0; i < pack->info.num_persons; i++)
    read_from_socket(socknum, (char *)(&(pack->newperson[i])),
		     sizeof(MiniPerson));
}



/* write to a stream socket the contents of a driver pack */

write_driver_pack(socknum, pack)
int socknum;
DriverPack *pack;
{
  register int i;

  /* write the information about the player */
  write_to_socket(socknum, (char *)(&(pack->info)), sizeof(MiniPlayer));

  /* write the information on each of the player's persons */
  for (i=0; i < pack->info.num_persons; i++)
    write_to_socket(socknum, (char *)(&(pack->newperson[i])),
		     sizeof(MiniPerson));
}




/* ================== GAME PERSONS reading and writing ================== */


/* read about persons in the game other than ourselves and add then to
   our game person array as we read them */

read_persons_in_game(socknum)
int socknum;
{
  register int	i;
  int		many_to_load;	/* how many we are going to get */
  GamePerson	gp;		/* holding area for game person info */
  
  /* find out from driver just how many he is going to send to us */
  if (DEBUG) printf("Read info on other players in game\n");
  read_from_socket(socknum, (char *)&many_to_load, sizeof(int));

  /* load each of the persons in game and add to our person array */
  for (i=0; i<many_to_load; i++) {
    /* read the gameperson record */
    read_from_socket(socknum, (char *)&gp, sizeof(GamePerson));

    /* store it into my game person array */
    add_game_person(&gp);
  }

  /* print info on all persons read */
  if (DEBUG) print_all_game_persons();
}



/* read in some miscellaneous other information driver has for us */

read_miscellaneous(socknum)
int socknum;
{
  int i, x;

  /* get the number of flags each team will need to win */
  for (i=1; i<=mapstats.teams_supported; i++) {
    read_from_socket(socknum, (char *) &x, sizeof(int));
    set_flag_requirement(i, x);
  }
}



/* tell the player about others in his game, omitting the persons that
   belong to him (the persons he requested were tacked on to the end
   of the gameperson array, so we just stop before we get there) */

write_persons_in_game(socknum, pack)
int socknum;
DriverPack *pack;
{
  register int	i;
  int		many_to_send;
  
  /* tell new player how many gameperson packets to expect */
  if (DEBUG) printf("Inform new player of others in game\n");
  many_to_send = persons_in_game - pack->info.num_persons;
  write_to_socket(socknum, (char *)&many_to_send, sizeof(int));

  /* send each gameperson record in turn */
  for (i=0; i<many_to_send; i++)
    write_to_socket(socknum, (char *)gameperson[i], sizeof(GamePerson));
}



/* write out some miscellaneous other information we have for player */

write_miscellaneous(socknum)
int socknum;
{
  int i, x;

  /* write out the number of flags the teams need to win */
  for (i=1; i<=mapstats.teams_supported; i++) {
    x = get_flag_requirement(i);
    write_to_socket(socknum, (char *) &x, sizeof(int));
  }
}




D 9
/* ================= MAP and OBJECTS reading and writing ================= */
E 9
I 9
/* ==================== MAP reading and writing ======================= */
E 9


D 22
/* Write current game map to given stream socket.  This procedure makes
   heavy use of global variables from map.c */
E 22
I 22
D 23
/* Write current game map to given stream socket */
E 23
I 23
/* Write current game map and objects to given stream socket */
E 23
E 22

write_map_to_socket(socknum)
int socknum;
{
D 22
  register int i, j;
  MemObj *ptr;
  
  /* send the mapstats info record */
E 22
  if (DEBUG) printf("Send current map over the socket\n");
D 22
  write_to_socket(socknum, (char *)&mapstats, sizeof(MapInfo));

  /* send each of the rooms */
  for (i=0; i<mapstats.rooms; i++) {
    room[i].numobjs = number_of_objs_here(i);
    write_to_socket(socknum, (char *) (&(room[i])), sizeof(RoomInfo));
    for (j=0,ptr=roomrecobj[i].next; j<room[i].numobjs; j++,ptr = ptr->next)
      write_to_socket(socknum, (char *) &(ptr->obj), sizeof(RecordedObj));
  }

  /* report how many rooms sent */
  if (DEBUG) printf("%d rooms sent\n", mapstats.rooms);
E 22
I 22
D 23
  lib_write_map_to_fd(&mapstats, socknum);
E 23
I 23
  lib_write_map_header_to_fd(&mapstats, socknum);

  /* send object definitions */
  send_obj_file(socknum);

  /* write out the object instances */
  lib_write_map_objects_to_fd(&mapstats, socknum);
E 23
E 22
}



D 9
/* write object definitions to given stream socket */

write_objects_to_socket(socknum)
int socknum;
{
  register int	i;
  int		how_many;
  
  /* find out how many object definitions to send */
  if (DEBUG) printf("Send object definitions over socket... ");
  how_many = objects;		/* objects is a global variable */
  write_to_socket(socknum, (char *)&how_many, sizeof(int));

  /* send each object in turn, getting it from object array */
  for (i=0; i<how_many; i++) {
    write_to_socket(socknum, (char *)info[(uc)i], sizeof(ObjInfo));
  }

  /* report how many objects sent */
  if (DEBUG) printf("%d sent\n", how_many);
}



E 9
/* Read current game map from given stream socket, allocating memory for
   it as we load it in.  This procedure makes heavy use of global variables
   from map.c */

read_map_from_socket(socknum)
int socknum;
{
D 22
  register int	i, j;
  int		how_many_rooms;
  RecordedObj	o;
  
E 22
I 22
  MapInfo *map = NULL;

E 22
  /* get the mapstats record with info about the map in it */
  if (DEBUG) printf("Load current map over the socket\n");
D 22
  read_from_socket(socknum, (char *)&mapstats, sizeof(MapInfo));
  how_many_rooms = mapstats.rooms;
E 22

D 22
  /* allocate a room array for the map */
  room = make_room_array(how_many_rooms);
E 22
D 23

E 23
D 22
  /* allocate the array of lists of recorded objs in the rooms and fill it */
  roomrecobj = make_room_obj_list_array(how_many_rooms);
  initialize_room_obj_list_array(roomrecobj, how_many_rooms);
E 22
I 22
  /* get the header information about map */
I 23
  if (am_player) joiner_message("Loading map header...");
E 23
  map = lib_read_map_header_from_fd(socknum);
  if (!map) Gerror("header is wrong in received map file");
  bcopy(map, &mapstats, sizeof(MapInfo));
  room = mapstats.room;
  free(map);
E 22

I 23
  /* get the object definitions */
  if (am_player) joiner_message("Loading object definitions...");
  receive_obj_file(socknum);

E 23
D 22
  /* read in all the rooms */
  for (i=0; i<how_many_rooms; i++) {
    read_from_socket(socknum, (char *)(&(room[i])), sizeof(RoomInfo));
    for (j=0; j<room[i].numobjs; j++) {
      read_from_socket(socknum, (char *)&o, sizeof(RecordedObj));
      add_file_record_to_mem(i, &o);
    }
  }

  /* report on what's been done here */
  if (DEBUG) printf("%d rooms loaded\n", how_many_rooms);
E 22
I 22
  /* now load object instances from the map */
I 23
  if (am_player) joiner_message("Loading object instances...");
E 23
  while (lib_read_mapobj_from_fd(&mapstats, socknum));
E 22
D 9
}



/* read object definitions from given stream socket and allocate space
   for them in memory as they are loaded */

read_objects_from_socket(socknum)
int socknum;
{
  register int	i;
  int		how_many;
  ObjInfo	*nextobj;
  
  /* find out how many object definitions to expect */
  if (DEBUG) printf("Load object definitions over socket... ");
  read_from_socket(socknum, (char *)&how_many, sizeof(int));

  /* load each object in turn, allocating memory and putting into obj array */
  for (i=0; i<how_many; i++) {
    nextobj = allocate_obj();
    read_from_socket(socknum, (char *)nextobj, sizeof(ObjInfo));
    info[(uc)i] = nextobj;
  }

  /* set global number of objects variable */
  objects = how_many;
  if (DEBUG) printf("%d loaded\n", how_many);
E 9
}



D 19
/* =================== D A T A G R A M  socket stuff =================== */

/* =========================== I N P U T =============================== */

/* This procedure waits for the specified timeout (in millisec) or
   until input is seen on the main datagram socket, whichever happens/
   completes first.  Returns TRUE when it found something readable. */

WaitForIO(ms)
int ms;
{
  return readable_on_socket(thesocket, ms);
}



/* This procedure should be called once in a while to check for input
   pending on the player's/driver's datagram socket.  When there is something
   to do, do what you can and place any special stuff on individual person's
   queues to be handled by other parts of the program */
   
BossIO()
{
  BossPacket		packet;
  struct timeval	timeout;
  Sockaddr		from;
  int			got_packet;

  /* set wait time for receiving a packet */     
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  /* read first of as many packets as there are right now */
  got_packet = recvPacket(&packet, &from, &timeout);

  while (got_packet) {

    /* call the routine for handling packets */
    if (am_monster) {
	/* monster_peek_packet returns TRUE if the packet should be passed
	   on for regular handling */
	if (monster_peek_packet(&packet)) handle_boss_packet(&packet, &from);
    }
    else handle_boss_packet(&packet, &from);

    /* read the next packet to handle */
    got_packet = recvPacket(&packet, &from, &timeout);
  }

}



/* Get a packet from our socket if there is anything at all to get, return
   TRUE if something was received, otherwise return FALSE.  You supply the
   amount of time to wait with a timeval structure pointer. */

int recvPacket(packet, from, wait)
BossPacket	*packet;
Sockaddr	*from;
struct timeval	*wait;
{
  int		ret, fromlen;

  /* if there is nothing to read then return FALSE to calling procedure */
  if (readable_on_socket(thesocket, 0) == FALSE) return FALSE;

  /* specify length of a network address */
  fromlen = sizeof(Sockaddr);

  /* pull in the packet from the socket */
  ret = recvfrom(thesocket, packet, sizeof(BossPacket), 0, from, &fromlen);

  /* on any kind of error but an interrupt error, crash and burn */
  if (ret < 0 && errno != EINTR) Berror("reception error while reading");
  else if (ret <= 0) return FALSE;	/* if nothing was read then return */

  /* let's check the packet's password */
  if (packet->password != BOSS_PASSWORD) {
      if (BOSS_DEBUG)
        printf("Socket Boss: warning: packet with bad password received\n");
      return FALSE;
  }

  /* everthing checks out, we can keep it, set receipt flags and return TRUE */
  IO_RECEIVED();
  return TRUE;
}




/* Add incoming packets to the correct person's queue or handle them ourself
   when we can do it without needing to inform any persons */

handle_boss_packet(packet, from)
BossPacket *packet;
Sockaddr   *from;
{
  int fromelement;
  
  if (DEBUG) printf("Received a packet of type %d\n", packet->type);

  /* if the packet is neither addressed to one of my persons, nor is
     addressed TO_ALL, then I should ignore it, it's probably spurious */
  if (am_driver && (packet->to != TO_GM && packet->to != TO_ALL)) return;
  else if (!am_driver)
         if (packet->to != TO_ALL && !one_of_me(packet->to)) return;

  /* if the packet is from an unknown person and is not MY_INFO, MY_BITMAP,
     or MY_MASK then ignore it and send REPEAT_INFO (unless it was a
     LEAVING_GAME or CHANGE_MAP packet) */

  fromelement = person_array(packet->from);	/* look in gameperson array */
  if (fromelement == -1 && (packet->from != FROM_GM))
    /* packet is from an unknown person */
    switch (packet->type) {
	case MY_INFO:
	case MY_BITMAP:
	case MY_MASK:		notice_new_person(packet, from);
				break;
	case CHANGE_MAP:	notice_map_change(packet);
				break;
	case LEAVING_GAME:	break;
	case WEAPON_FIRED:	break;
	default:		/* send a repeat info request */
				break;
    }
  else
    /* packet is from a known person or the game master */
    switch (packet->type) {
	/* new player info packets */
	case MY_INFO:
	case MY_BITMAP:
	case MY_MASK:		notice_added_info(packet);
				break;
	/* packets of knowledge for all persons */
	case WEAPON_FIRED:	add_missile(packet);
				break;
	case MULTI_FIRE:	notice_multi_fire(packet);
				break;
	case MY_LOCATION:	notice_person_location(packet);
				break;
	case CHANGE_MAP:	notice_map_change(packet);
				break;
	case DAMAGE_REPORT:	notice_damage_report(packet);
				break;
	case YOU_KILLED_ME:	notice_kill(packet);
				break;
	case ADD_EXPERIENCE:	if (!am_driver) notice_experience(packet);
				break;
	case LEAVING_GAME:	notice_person_leaving(packet);
				break;
	case SAVE_STATS:	notice_save_stats(packet);
				break;
	case GAME_OVER:		notice_game_over(packet);
				break;
	/* packets to put into individual person queues */
	case MESSAGE:
	case EXECUTE_COMMAND:	if (!am_driver) notice_queued_packet(packet);
				else try_to_log_packet(packet);
				break;
	default:		break;
  }
}



/* ============================ O U T P U T =============================== */


/* Send a packet to an address. This procedure will automatically
   fill in the BOSS_PASSWORD for you. (current game password) */

D 12
send_to_address(address, packet)
E 12
I 12
send_to_address(address, packet, reliable)
E 12
Sockaddr	*address;
BossPacket	*packet;
I 12
int		reliable;
E 12
{
  int		tolen, ret;
  
  packet->password = BOSS_PASSWORD;

  tolen = sizeof(Sockaddr);
  ret = sendto(thesocket, packet, sizeof(BossPacket), 0, address, tolen);

  if (ret == -1) {
    printf("Socket Boss: error num: %d\n", errno);
    Berror("failed on sendto other player");
  }
}



/* send the driver a packet (you shouldn't call this if you are the driver) */

D 12
send_to_driver(packet)
E 12
I 12
send_to_driver(packet, reliable)
E 12
BossPacket *packet;
I 12
int reliable;
E 12
{
  if (Iamboss && BOSS_DEBUG)
    fprintf(stderr, "Socket Boss: warning: send_to_driver called\n");

  /* send the packet to the boss */
  if (DEBUG) printf("Sending packet of type %d to driver\n", packet->type);
D 12
  if (!am_driver) send_to_address(&driveraddr, packet);
E 12
I 12
  if (!am_driver) send_to_address(&driveraddr, packet, reliable);
E 12
}



/* Send a packet to a particular person (in person array).  BOSS_PASSWORD
   will be filled in for you and the correct parent and address will be used,
   you still have to fill in the "to" variable in the packet yourself. */

D 12
send_to_person(pnum, packet)
E 12
I 12
send_to_person(pnum, packet, reliable)
E 12
int pnum;
BossPacket *packet;
I 12
int reliable;
E 12
{
  if (DEBUG)
    printf("Sending packet of type %d to person element %d\n", packet->type, pnum);

  /* check for out of bounds person */
  if (pnum >= persons_in_game || pnum < 0) {
    if (DEBUG) printf("No person %d exists yet\n", pnum);
    return;
  }

  /* send the packet to their parent if neccessary*/
  if (gameperson[pnum]->parent == gameperson[pnum]->id)
D 12
    send_to_address(&(gameperson[pnum]->address), packet);
E 12
I 12
    send_to_address(&(gameperson[pnum]->address), packet, reliable);
E 12
  else
D 12
    send_to_person(person_array(gameperson[pnum]->parent), packet);
E 12
I 12
    send_to_person(person_array(gameperson[pnum]->parent), packet, reliable);
E 12
}



/* send to packet to a particular person, that person being identified
   by his id number */

D 12
send_to_id(idnum, pack)
E 12
I 12
send_to_id(idnum, pack, reliable)
E 12
int idnum;
BossPacket *pack;
I 12
int reliable;
E 12
{
  int pnum;

  pnum = person_array(idnum);
  if (pnum == -1) return;

D 12
  send_to_person(pnum, pack);
E 12
I 12
  send_to_person(pnum, pack, reliable);
E 12
}



		    
/* Send a packet to all players in the game (that means only one packet per
   player process not to every person).  You are responsible for putting
   TO_ALL as the "to" variable inside the packet */

D 12
send_to_players(packet)
E 12
I 12
send_to_players(packet, reliable)
E 12
BossPacket *packet;
I 12
int reliable;
E 12
{
  register int i;
  
  /* we send the packet to all persons (except ours) who are own parents */
  for (i=num_persons; i<persons_in_game; i++)
D 12
    if (gameperson[i]->parent == gameperson[i]->id) send_to_person(i, packet);
E 12
I 12
    if (gameperson[i]->parent == gameperson[i]->id)
	send_to_person(i, packet, reliable);
E 12
}



/* Send a packet only to those players who have one of their persons in
   the given room.  As usual you should address the packet TO_ALL.  This
   routine relies on the fact that all persons store the network address
   of their parent inside their own structure. */

D 12
send_to_room(roomnum, packet)
E 12
I 12
send_to_room(roomnum, packet, reliable)
E 12
int roomnum;
BossPacket *packet;
I 12
int reliable;
E 12
{
  PersonList *ptr;
  
  /* check that it is a valid room */
  if (roomnum < 0  || roomnum >= mapstats.rooms) return;

  /* go through the persons in room list, sending this packet to all
     persons that are marked as important.  This will assure that each
     player who has one or more persons in this room will only be sent
     this packet once.  Do not send the packet to my own siblings
     in the room. */

  for (ptr=room_persons[roomnum].next; ptr; ptr = ptr->next)
    if (ptr->important && ptr->person->parent != gameperson[0]->parent)
D 12
      send_to_address(&(ptr->person->address), packet);
E 12
I 12
      send_to_address(&(ptr->person->address), packet, reliable);
E 12
}


I 5
D 17
/* Send a packet only to those players who do NOT have one of their persons
   in the given room.  As usual you should address the packet TO_ALL.  This
   routine relies on the fact that all persons store the network address
   of their parent inside their own structure. */
E 17
I 17
/* Send one packet only to those players who do NOT have one of their persons
   in the given room.  Send another packet to everyone else.
   As usual you should address the packet TO_ALL. */
E 17
E 5

I 5
D 12
send_out_of_room(roomnum, packet)
E 12
I 12
D 17
send_out_of_room(roomnum, packet, reliable)
E 17
I 17
send_in_and_out_of_room(roomnum, inner_packet, outer_packet, reliable)
E 17
E 12
int roomnum;
D 17
BossPacket *packet;
E 17
I 17
BossPacket *inner_packet, *outer_packet;
E 17
I 12
int reliable;
E 12
{
D 17
  int i;
E 17
I 17
  int i, *person_list;
E 17
  PersonList *ptr;
D 17

  /* check that it is a valid room, if not, send packet to everyone */
D 12
  if (roomnum < 0  || roomnum >= mapstats.rooms) send_to_players(packet);
E 12
I 12
  if (roomnum < 0  || roomnum >= mapstats.rooms)
	send_to_players(packet, reliable);
E 17
E 12
  
D 17
  /* go through all rooms except the one we want to skip */
  for (i=0; i<mapstats.rooms; i++)
    if (i != roomnum) {
      for (ptr=room_persons[i].next; ptr; ptr = ptr->next)
        if (ptr->important && ptr->person->parent != gameperson[0]->parent)
D 12
          send_to_address(&(ptr->person->address), packet);
E 12
I 12
          send_to_address(&(ptr->person->address), packet, reliable);
E 12
    }
E 17
I 17
  /* make a list for marking which persons got a copy of inner packet */
  person_list = (int *) malloc(sizeof(int) * persons_in_game);
  for (i=num_persons; i<persons_in_game; i++) person_list[i] = FALSE;

  /* go through the persons in room list, sending inner_packet to all
     persons that are marked as important in the room.  This will assure
     that each player who has one or more persons in this room will only be
     sent this packet once.  Do not send the packet to my own siblings in
     the room. */

  if (!(roomnum < 0  || roomnum >= mapstats.rooms)) {
    for (ptr=room_persons[roomnum].next; ptr; ptr = ptr->next)
      if (ptr->important && ptr->person->parent != gameperson[0]->parent) {
        send_to_address(&(ptr->person->address), inner_packet, reliable);
        person_list[person_array(ptr->person->parent)] = TRUE;
      }
  }

  /* send the other packet to all parents who did not receive first packet */

  for (i=num_persons; i<persons_in_game; i++)
   if ((gameperson[i]->id == gameperson[i]->parent) && !person_list[i])
     send_to_person(i, outer_packet, reliable);
E 17
}



E 5
/* Send a packet to everyone important (that means players and driver) */

D 12
send_to_important(packet)
E 12
I 12
send_to_important(packet, reliable)
E 12
BossPacket *packet;
{
D 12
  send_to_players(packet);
  send_to_driver(packet);
E 12
I 12
  send_to_players(packet, reliable);
  send_to_driver(packet, reliable);
E 12
}



/* Send a packet to a given string name, which could be a person name, could
   be "all", could be a team name, or could be GM_NAME.  Return the id number
   of the person who is going to get message (or return TO_ALL or TO_GM).
   Return -99 if no one resonable could be found. */

D 12
int send_to_name(fromid, name, pack)
E 12
I 12
int send_to_name(fromid, name, pack, reliable)
E 12
int fromid;
char *name;
Packet *pack;
I 12
int reliable;
E 12
{
  register int	i;
  int		result = -99, temp, pnum = -1, matchlen = 0;
  
  /* check for send to all */
  if (strmatch(name, "all")==3) {
    address_packet(pack, fromid, TO_ALL);
D 12
    send_to_players(pack);
E 12
I 12
    send_to_players(pack, reliable);
E 12
    result = TO_ALL;
  }

  /* check for send to everyone in room */
  if ((strmatch(name, "say")==3 && strlen(name)==3) ||
      (strmatch(name, "room")==4 && strlen(name)==4)) {
    address_packet(pack, fromid, TO_ALL);
D 12
    send_to_room(pack);
E 12
I 12
    send_to_room(pack, reliable);
E 12
    result = TO_ALL;
  }

  /* or perhaps to the game master */
  else if (strmatch(name, GM_NAME)==strlen(GM_NAME)) {
    address_packet(pack, fromid, TO_GM);
D 12
    send_to_driver(pack);
E 12
I 12
    send_to_driver(pack, reliable);
E 12
    result = TO_GM;
  }

  /* or see if it is an id number */
  else if (isdigit(name[0])) {
    pnum = person_array(atoi(name));
    if (pnum < 0  ||  pnum >= persons_in_game) return result;	/* bad id */
    address_packet(pack, fromid, gameperson[pnum]->id);
D 12
    send_to_person(pnum, pack);
E 12
I 12
    send_to_person(pnum, pack, reliable);
E 12
    result = gameperson[pnum]->id;
  }

  /* finally, check to see if we get any resonable match to any person name */
  else {
    for (i=num_persons; i<persons_in_game; i++) {
      temp = strmatch(name, gameperson[i]->name);
      if (temp>matchlen && (temp>2 || temp==strlen(gameperson[i]->name))) {
        matchlen = temp;
	pnum = i;
      }
    }
    if (pnum < 0) return result;  /* return if no matching name found */
    /* otherwise send off the packet */
    address_packet(pack, fromid, gameperson[pnum]->id);
D 12
    send_to_person(pnum, pack);
E 12
I 12
    send_to_person(pnum, pack, reliable);
E 12
    result = gameperson[pnum]->id;
  }

  return result;
}



/* ==================== O U T G O I N G  packets ========================= */

/* let old players know we are joining the game, send this info to the
   driver as well so that he will know what team we have chosen. */

notify_old_players()
{
  int i;
  Packet pack;
  char	 s[MSG_LENGTH];
  
  if (DEBUG) printf("Tell old players we are joining the game\n");

  /* send all players info packets for each of my persons */
  for (i=0; i<num_persons; i++) {

    /* send MY_INFO packet to all players */
    prepare_my_info(i, &pack);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
D 12
    send_to_important(&pack);
E 12
I 12
    send_to_important(&pack, TRUE);
E 12

    /* send MY_MASK packet to all players */
    prepare_my_mask(i, &pack);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
D 12
    send_to_players(&pack);
E 12
I 12
    send_to_players(&pack, TRUE);
E 12

    /* send MY_BITMAP packet to all players */
    prepare_my_bitmap(i, &pack);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
D 12
    send_to_players(&pack);
E 12
I 12
    send_to_players(&pack, TRUE);
E 12

    /* send SAVE_STATS packet so players know what I've got */
    notify_of_stats(i);

    /* send MESSAGE concerning my entry to all persons */
D 7
    sprintf(s, "%s->ALL %s (%d) joining game", GM_NAME, gameperson[i]->name,
	    gameperson[i]->id);
E 7
I 7
    sprintf(s, "%s->ALL %s (%d) joining %s", GM_NAME,
	    gameperson[i]->name,gameperson[i]->id,
	    get_team_name(gameperson[i]->team));

E 7
    prepare_message(&pack, s);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
D 12
    send_to_players(&pack);
E 12
I 12
    send_to_players(&pack, FALSE);
E 12
  }
}



/* making a change to the map, tell all players and the driver exactly
   what change has been made.  See the documentation on CHANGE_MAP in
   socket.h for explanations of the parameters. */

notify_all_of_map_change(roomnum, x, y, which, old, new, recobjptr)
int roomnum, x, y, which, old, new;
RecordedObj *recobjptr;
{
  Packet pack;
  
  /* prepare the packet */
  if (DEBUG) printf("Telling all of map change\n");
  pack.type = CHANGE_MAP;
  pack.info.map.room = roomnum;
  pack.info.map.x = x;
  pack.info.map.y = y;
  pack.info.map.which = which;
  pack.info.map.old = (uc) old;
  pack.info.map.new = (uc) new;
  if (recobjptr)
    bcopy(recobjptr, &(pack.info.map.recobj), sizeof(RecordedObj));

D 12
  /* send the packet anonymously to players and driver */
E 12
I 12
  /* send the packet to players and driver, anonymously */
E 12
  address_packet(&pack, -1, TO_ALL);
D 12
  send_to_important(&pack);
E 12
I 12
  send_to_important(&pack, TRUE);
E 12

  /* order the object's replacement for ourselves as well */
  change_map_square(roomnum, x, y, which, (uc)old, (uc)new, recobjptr);

  /* check to see if our team has won the game because of my change */
D 8
  if (info[(uc)new]->flag || info[(uc)new]->target) look_for_wins();
E 8
I 8
  if (info[(uc)new]->flag) look_for_wins();
E 8
}



D 5
/* let everyone in game know where person is (changing rooms) */
E 5
I 5
/* let everyone in game know where person is (changing rooms),
   players not in the target room only get a partial location (room) */
E 5

notify_all_of_location(pnum)
int pnum;
{
D 17
  Packet pack;
E 17
I 17
  Packet inner_pack, outer_pack;
E 17
  
D 17
  prepare_my_location(pnum, &pack);
  address_packet(&pack, gameperson[pnum]->id, TO_ALL);
D 5
  send_to_players(&pack);
E 5
I 5
D 12
  send_to_room(gameperson[pnum]->room, &pack);
E 12
I 12
  send_to_room(gameperson[pnum]->room, &pack, TRUE);
E 12

E 17
I 17
  prepare_my_location(pnum, &inner_pack);
  prepare_my_location(pnum, &outer_pack);
  address_packet(&inner_pack, gameperson[pnum]->id, TO_ALL);
  address_packet(&outer_pack, gameperson[pnum]->id, TO_ALL);
E 17
  /* block out detailed location for far away folks */
D 17
  pack.info.loc.x = -1;
  pack.info.loc.y = -1;
D 12
  send_out_of_room(gameperson[pnum]->room, &pack);
E 12
I 12
  send_out_of_room(gameperson[pnum]->room, &pack, TRUE);
E 17
I 17
  outer_pack.info.loc.x = -1;
  outer_pack.info.loc.y = -1;

  send_in_and_out_of_room(gameperson[pnum]->room, &inner_pack,
			  &outer_pack, TRUE);
E 17
E 12
E 5
}




/* let everyone this persons room know where he is (changing spots) */

notify_room_of_location(pnum)
int pnum;
{
  Packet pack;
  
  prepare_my_location(pnum, &pack);
  address_packet(&pack, gameperson[pnum]->id, TO_ALL);
D 12
  send_to_room(gameperson[pnum]->room, &pack);
E 12
I 12
  send_to_room(gameperson[pnum]->room, &pack, FALSE);
E 12
}




/* notify persons coming into a room one of my persons is in of the location
   of my persons in the room */

notify_incoming_of_locations(pnum)
int pnum;	/* incoming person array number */
{
  int		kids, important;
  PersonList	*ptr;

  /* check that it is a valid room number */
  if (gameperson[pnum]->room < 0 ||
      gameperson[pnum]->room >= mapstats.rooms)  return;

  /* check that it is a valid person number */
  if (pnum < 0 || pnum >= persons_in_game) return;
    
  /* check the room they are entering for any of my kids */
  kids = found_siblings(gameperson[pnum]->room, gameperson[0]->parent);

  /* check to see if incoming person has been marked as important */
  important = is_marked_important(pnum);

  /* if I have kids there and he is important then send location info */
  if (kids && important) {
    for (ptr = room_persons[gameperson[pnum]->room].next; ptr; ptr = ptr->next)
      if (ptr->person->parent == gameperson[0]->parent)
        notify_person_of_person_location(pnum, person_array(ptr->person->id));
  }
}



/* notify a particular person (probably a parent) of the locations of
   all of my persons */

notify_person_of_my_locations(pnum)
int pnum;
{
  register int	i;

  for (i=0; i<num_persons; i++)
    notify_person_of_person_location(pnum, i);
}



/* notify a particular person of the location of one of my persons */

notify_person_of_person_location(pnum, mine)
int pnum, mine;
{
  Packet pack;

  prepare_my_location(mine, &pack);
  address_packet(&pack, gameperson[mine]->id, gameperson[pnum]->id);
D 12
  send_to_person(pnum, &pack);
E 12
I 12
  send_to_person(pnum, &pack, FALSE);
E 12
}



/* notify the recipient(s) of a sent message, return id num of receiver */

int notify_of_message(fromid, to_name, msg)
int fromid;
char *to_name, *msg;
{
  Packet msgpack;

  /* prepare and send the packet */
  prepare_message(&msgpack, msg);
D 12
  return send_to_name(fromid, to_name, &msgpack);
E 12
I 12
  return send_to_name(fromid, to_name, &msgpack, FALSE);
E 12
}



/* notify the given person id of damage he has done */

notify_of_damage(fromid, to_id, msg)
int fromid, to_id;
char *msg;
{
  Packet dmgpack;

  /* prepare and send the packet */
  dmgpack.type = DAMAGE_REPORT;
  strcpy(dmgpack.info.msg, msg);
  address_packet(&dmgpack, fromid, to_id);
D 12
  send_to_id(to_id, &dmgpack);
E 12
I 12
  send_to_id(to_id, &dmgpack, FALSE);
E 12
}



/* Tell person about an experience point bonus and give it to him */

notify_of_experience(fromid, to_id, amount, msg)
int fromid, to_id, amount;
char *msg;
{
  Packet pack;

  /* prepare and send the packet */
  pack.type = ADD_EXPERIENCE;
  pack.info.add.id = to_id;
  pack.info.add.amount = amount;
  strcpy(pack.info.add.msg, msg);
  address_packet(&pack, fromid, to_id);
D 12
  send_to_id(to_id, &pack);
E 12
I 12
  send_to_id(to_id, &pack, FALSE);
E 12
}



/* change a person's bitmap and mask, telling everyone else about the
   change as well.  If this player has a display he'll need to call the
   pixmap updater right after this */

change_person_bitmaps(pnum, bitmap, mask)
int pnum;
char *bitmap, *mask;
{
  Packet bitpack, maskpack;

  /* prepare mask packet and send it first */
  maskpack.type = MY_MASK;
  bitmapcpy(maskpack.info.bit, mask);
  address_packet(&maskpack, gameperson[pnum]->id, TO_ALL);
D 12
  send_to_players(&maskpack);
E 12
I 12
  send_to_players(&maskpack, TRUE);
E 12

  /* prepare bitmap packet and send it */
  bitpack.type = MY_BITMAP;
  bitmapcpy(bitpack.info.bit, bitmap);
  address_packet(&bitpack, gameperson[pnum]->id, TO_ALL);
D 12
  send_to_players(&bitpack);
E 12
I 12
  send_to_players(&bitpack, TRUE);
E 12

  /* copy the new bitmaps into my gameperson structure */
  bitmapcpy(gameperson[pnum]->bitmap, bitmap);
  bitmapcpy(gameperson[pnum]->mask, mask);

  /* update my own pixmap of the person if neccessary */
  if (has_display) update_pixmaps(pnum);
}




/* let everyone know that the game is over, winner is the id number of
   person that finished off the game */

notify_of_game_over(winner, team)
int winner, team;
{
  Packet pack;

  /* don't do this if the game is already ending */
  if (IS_GAME_ENDING) return;

  /* prepare the packet */
  pack.type = GAME_OVER;
  pack.info.game.winner = winner;
  pack.info.game.team = team;

  /* send it to players and driver */
  address_packet(&pack, winner, TO_ALL);
D 12
  send_to_important(&pack);
E 12
I 12
  send_to_important(&pack, FALSE);
E 12

  /* send it again to be sure everyone gets it */
D 12
  send_to_important(&pack);
E 12
I 12
  send_to_important(&pack, TRUE);
E 12

  /* notice for myself that the game is over */
  notice_game_over(&pack);
}



/* notify the driver and players of one of my player person's stats,
   request that driver save the stats in the password file. */

notify_of_stats(pnum)
int pnum;
{
  Packet pack;

  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending SAVE_STATS about person %d\n", pnum);

  /* prepare packet and send to driver and players */
  pack.type = SAVE_STATS;
  pack.info.stat.id = gameperson[pnum]->id;
  pack.info.stat.ExpPts = person[pnum]->experience;
  strcpy(pack.info.stat.rank, gameperson[pnum]->rank);
  pack.info.stat.level = gameperson[pnum]->level;
  pack.info.stat.kills = gameperson[pnum]->kills;
  pack.info.stat.losses = gameperson[pnum]->losses;
  pack.info.stat.games = gameperson[pnum]->games;
  address_packet(&pack, gameperson[pnum]->id, TO_ALL);
D 12
  send_to_important(&pack);
E 12
I 12
  send_to_important(&pack, FALSE);
E 12
}



/* tell someone they killed me */

notify_of_kill(victim_id, killer_id)
{
  Packet pack;

  /* check that this is one of my own persons */
  if (person_array(victim_id) >= num_persons)
    printf("Warning: sending YOU_KILLED_ME about someone else");

  /* prepare packet and send it to person */
  pack.type = YOU_KILLED_ME;
  address_packet(&pack, victim_id, killer_id);
D 12
  send_to_id(killer_id, &pack);
E 12
I 12
  send_to_id(killer_id, &pack, FALSE);
E 12
}



/* prepare a MY_INFO packet for a certain person */

prepare_my_info(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_INFO about person %d\n", pnum);

  /* prepare packet */
  pack->type = MY_INFO;
  pack->info.data.parent = gameperson[pnum]->parent;
  strcpy(pack->info.data.name, gameperson[pnum]->name);
  strcpy(pack->info.data.login, gameperson[pnum]->login);
  strcpy(pack->info.data.host, gameperson[pnum]->host);
  strcpy(pack->info.data.rank, gameperson[pnum]->rank);
  pack->info.data.level = gameperson[pnum]->level;
  pack->info.data.team = gameperson[pnum]->team;
  pack->info.data.listed = gameperson[pnum]->listed;
  pack->info.data.deity = gameperson[pnum]->deity;
}


/* prepare a MY_BITMAP packet for a certain person */

prepare_my_bitmap(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_BITMAP about person %d\n", pnum);

  /* prepare packet */
  pack->type = MY_BITMAP;
  bitmapcpy(pack->info.bit, gameperson[pnum]->bitmap);
}


/* prepare a MY_MASK packet for a certain person */

prepare_my_mask(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_MASK about person %d\n", pnum);

  /* prepare packet */
  pack->type = MY_MASK;
  bitmapcpy(pack->info.bit, gameperson[pnum]->mask);
}


/* prepare a MY_LOCATION packet to all whom it may concern */

prepare_my_location(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_LOCATION about person %d\n", pnum);

  pack->type = MY_LOCATION;
  pack->info.loc.x = gameperson[pnum]->x;
  pack->info.loc.y = gameperson[pnum]->y;
  pack->info.loc.room = gameperson[pnum]->room;
  pack->info.loc.appear = gameperson[pnum]->appearance;
}



/* prepare a message packet */

prepare_message(pack, msg)
Packet *pack;
char   *msg;
{
  pack->type = MESSAGE;
D 10
  strcpy(pack->info.msg, msg);
E 10
I 10
  (void) strncpy(pack->info.msg, msg, MSG_LENGTH-1);
  pack->info.msg[MSG_LENGTH-1] = '\0';
E 10
}



/* address a packet by setting its contents according to parameters */

address_packet(pack, from, to)
Packet *pack;
int from, to;
{
  pack->from = from;
  pack->to = to;
}


/* ============================== M I S C ================================ */

/* persons leave game by sending everyone a LEAVING_GAME packet */

leaveGame(num)
int num;	/* number of element in person array */
{
  BossPacket msgpack, leavepack;
  char	     s[MSG_LENGTH];

  /* ask driver to save my stats */
  notify_of_stats(num);

  /* send message to all concerning leaving the game */
  sprintf(s, "%s->ALL %s (%d) leaving game", GM_NAME,
	  gameperson[num]->name, gameperson[num]->id);
  prepare_message(&msgpack, s);
  address_packet(&msgpack, gameperson[num]->id, TO_ALL);
D 12
  send_to_players(&msgpack);
E 12
I 12
  send_to_players(&msgpack, FALSE);
E 12

  /* prepare the LEAVING_GAME packet */
  leavepack.type = LEAVING_GAME;
  leavepack.from = gameperson[num]->id;
  leavepack.to = TO_ALL;

  /* send the packet off to everyone */
D 12
  send_to_important(&leavepack);
E 12
I 12
  send_to_important(&leavepack, TRUE);
E 12
}



/* for players, leave the game */

allLeaveGame()
{
  int i;

  /* notify other players and driver of leaving persons */
  if (!am_driver) for (i=0; i<num_persons; i++)  leaveGame(i);
}



E 19
/* Close up shop, this player or driver wants to quit */

endBoss()
{
  /* close the main socket */
  close(thesocket);

  /* the driver must also close his new player socket */
  if (Iamboss) close(recsocket);
}



D 19

E 19
/* general socket boss error routine */

Berror(s)
char *s;
{
  fprintf(stderr, "Griljor Socket Boss: %s\n", s);
  exit(1);
D 19
}



/* ====================== A U T O  packet handlers ====================== */


/* when a person wants to leave the game, see that his records are deleted */

notice_person_leaving(pack)
Packet *pack;
{
  int pnum;
  
  /* check that it is a valid person */
  if (DEBUG) printf("Person %d leaving the game\n", pack->from);
  pnum = person_array(pack->from);
  if (pnum == -1) {
    if (DEBUG) printf("Warning: non-existant person\n");
    return;
  }
  if (pnum < num_persons) return;

  /* move them off into infinity so that if they are in our room
     their image will be erased */
  if (!am_driver) move_person(pnum, -1, -1, -1, TRUE);

  /* delete their records from our game person array */
  delete_game_person(pnum);
}




/* someone new has joined the game, notice this and add them to our
   gameperson array */

notice_new_person(pack, from)
Packet *pack;
Sockaddr *from;
{
  int num, i;
  Packet infopack;
  
  /* make a gameperson array element for them */
  if (DEBUG) printf("New person %d in the game\n", pack->from);
  num = persons_in_game;
  initialize_game_person(num);
  persons_in_game++;

  /* put in the persons id number and socket address */
  gameperson[num]->id = pack->from;
  bcopy(from, &(gameperson[num]->address), sizeof(Sockaddr));

  /* put in the data given by this packet */
  notice_added_info(pack);

  /* if we are also new to the game and it is close to the time when we
     joined, then send him back our own info to insure that he finds out
     what team we are on (driver might not have known at time he joined) */
  if (!am_driver && gametime <= STARTUP_WAIT) {
    for (i=0; i<num_persons; i++) {
      prepare_my_info(i, &infopack);
      address_packet(&infopack, gameperson[i]->id, TO_ALL);
D 12
      send_to_id(pack->from, &infopack);
E 12
I 12
      send_to_id(pack->from, &infopack, TRUE);
E 12
    }
  }
}




/* handle MY_INFO, MY_BITMAP, and MY_MASK packets, adding info to array */

notice_added_info(pack)
Packet *pack;
{
  int pnum;	/* person array element number */

  /* find out array index */
  if (DEBUG) printf("Information on person %d\n", pack->from);
  pnum = person_array(pack->from);

  /* check for invalid array element */
  if (pnum == -1) {
    if (DEBUG) printf("unidentified person %d\n", pack->from);
    return;
  }

  /* figure out what kind of info it is and add it */
  switch (pack->type) {
  	case MY_INFO:	/* fill in gameperson record with info given */
			gameperson[pnum]->parent = pack->info.data.parent;
			strcpy(gameperson[pnum]->name, pack->info.data.name);
			strcpy(gameperson[pnum]->rank, pack->info.data.rank);
			strcpy(gameperson[pnum]->login, pack->info.data.login);
			strcpy(gameperson[pnum]->host, pack->info.data.host);
			gameperson[pnum]->level = pack->info.data.level;
			gameperson[pnum]->team = pack->info.data.team;
			gameperson[pnum]->deity = pack->info.data.deity;
			gameperson[pnum]->listed = pack->info.data.listed;
			/* let possible new player know where we are */
			if (gameperson[pnum]->id == gameperson[pnum]->parent &&
			    !am_driver) notify_person_of_my_locations(pnum);
			break;
	case MY_BITMAP:	if (!am_driver) {
			  bitmapcpy(gameperson[pnum]->bitmap, pack->info.bit);
			  /* update pixmap when bitmap is received */
			  if (has_display) update_pixmaps(pnum);
			}
			break;
	case MY_MASK:	if (!am_driver) {
			  bitmapcpy(gameperson[pnum]->mask, pack->info.bit);
			  /* don't update pixmap, wait for bitmap */
			}
			break;
  }

  /* print out new version of game person */
  if (DEBUG) print_game_person(pnum);
}




/* handle new info about a person's location */

notice_person_location(pack)
Packet *pack;
{
  int pnum;

  /* get array element number */
  if (DEBUG) printf("Handling location data from person %d\n", pack->from);
  pnum = person_array(pack->from);

  /* update person array as long as this isn't one of my own people */
  if (pnum >= num_persons) {
    gameperson[pnum]->appearance = pack->info.loc.appear;
    move_person(pnum, pack->info.loc.room, pack->info.loc.x,
		pack->info.loc.y, TRUE);
  }
}



/* notice a multi-missile firing packet.  Separate it into missiles
   and add each of those to my regular missile list */

notice_multi_fire(pack)
Packet *pack;
{
  add_multi_missiles(pack);
}



/* notice a damage report message, if we have a display, then put the
   message up in the error window.  Only messages for the current person
   are displayed (probably person 0) */

notice_damage_report(pack)
Packet *pack;
{
  /* MAY CHANGE: person zero considered current person */
  if (person_array(pack->to) == 0 && has_display)
D 18
    redraw_error_window(pack->info.msg);
E 18
I 18
    player_error_out_message(pack->info.msg);
E 18
}


/* notice when someone tells me I killed them, give my self some credit */

notice_kill(pack)
Packet *pack;
{
  int pnum = person_array(pack->to);

  /* check for bad person number */
  if (pnum < 0 || pnum >= num_persons) return;

  /* add a kill to his records, his stats will be updated to everyone else
     when the dying player sends the experience point packet */
  add_kill(pnum);
}



/* notice a person sending info to me to add something to one of my
   person's experience point total.  Negative adds are also possible. */

notice_experience(pack)
Packet *pack;
{
  int pnum = person_array(pack->info.add.id);
  
  alter_experience(pnum, pack->info.add.amount, pack->info.add.msg);
}



/* handle a CHANGE_MAP packet, changing the map if possible, and being
   careful not to be fooled by duplicate packets */

notice_map_change(pack)
Packet *pack;
{
  if (DEBUG) printf("Handling map change info from %d\n", pack->from);

  /* call for a change in the map */
  change_map_square(pack->info.map.room, pack->info.map.x, pack->info.map.y,
		    pack->info.map.which, (uc)pack->info.map.old,
		    (uc)pack->info.map.new, &(pack->info.map.recobj));
}



/* change the contents of a square according to the data given.  This is
   the routine called from the player and driver process when a CHANGE_MAP
   packet is received */

change_map_square(roomnum, x, y, which, old, new, recobjptr)
int roomnum, x, y, which, old, new;
RecordedObj *recobjptr;
{
  MemObj *ptr;
  
  /* check that these are valid values */
  if (DEBUG) printf("Changing map in room %d at %d %d\n", roomnum, x, y);
  if (roomnum < 0 || roomnum >= mapstats.rooms || x<0 || y<0 ||
      x >= ROOM_WIDTH || y >= ROOM_HEIGHT || which<0 ||
      which > ROOM_DEPTH || new<0 || new >= objects) return;

  /* if object is a recorded object then treat specially */
  if (which == ROOM_DEPTH) {
    /* check to see if we are required to replace with a recorded object
       but no recorded object record is given */
    if (info[(uc)new]->recorded && recobjptr == NULL) return;

    /* check that old object is of type claimed */
    ptr = what_recorded_obj_here(roomnum, x, y);

    if (!ptr) {	/* no recorded object here! */
      if (old) return; /* old thing not there */
    }
    else if (old != ptr->obj.objtype) return;  /* rec obj of different type */
    else {
      /* remove the old recorded object */
      remove_recorded_obj(roomnum, x, y);
    }

    /* try to bring on the new one (but only if it is a recorded object) */
    if (info[(uc)new]->recorded) {
      ptr = allocate_mem_obj();
      bcopy(recobjptr, &(ptr->obj), sizeof(RecordedObj));
      add_obj_to_room(roomnum, ptr);
    }
  }

  /* otherwise treat as a normal object */
  else {
    /* check to see if we are asked to put a recorded obj into normal slot */
    if (info[(uc)new]->recorded) return;

    /* check to see if old value is really as is claimed */
    if (room[roomnum].spot[x][y][which] != old) return;

    /* replace old value with new one */
    room[roomnum].spot[x][y][which] = new;
  }

  /* redraw the room square referred to */
D 6
  if (has_display) redraw_player_room_square(roomnum, x, y);
E 6
I 6
  if (has_display) ChangedRoomSquare(roomnum,x,y);
E 6

  /* tell the important object accounting system about this */
  flag_account_map_change(roomnum, x, y, old, new);
}



/* notice a message that the game is over */

notice_game_over(pack)
Packet *pack;
{
  char s[80];
  int quit_time;

  /* set the game end countdown timer (if game isn't already ending)
     and notify player of impending end of game */
  if (!IS_GAME_ENDING) {
    end_of_world = gametime + GAME_OVER_WAIT;

    /* figure out how long to game end (in seconds) */
    quit_time = (GAME_OVER_WAIT / CLICKS_PER_SEC);

    /* print messages about game end */
    if (has_display) {
D 18
      player_out_message("********* Game is over *****************************");
      player_out_message("*** Congratulations !!!");
E 18
I 18
      player_error_out_message("********* Game is over *****************************");
      player_error_out_message("*** Congratulations !!!");
E 18
      if (person_array(pack->info.game.winner) != -1) {
        sprintf(s, "*** Winning  player: %s", 
		gameperson[person_array(pack->info.game.winner)]->name);
D 18
        player_out_message(s);
E 18
I 18
        player_error_out_message(s);
E 18
	/* print out winning team name */
	if (pack->info.game.team > 0 &&
	    pack->info.game.team <= mapstats.teams_supported) {
          sprintf(s, "*** Victorious team: %s", 
		  mapstats.team_name[pack->info.game.team - 1]);
D 18
	  player_out_message(s);
E 18
I 18
	  player_error_out_message(s);
E 18
	}
        sprintf(s, "*** %d seconds until full game shutdown", quit_time);
D 18
        player_out_message(s);
E 18
I 18
        player_error_out_message(s);
E 18
      }
D 18
      player_out_message("****************************************************");
E 18
I 18
      player_error_out_message("****************************************************");
E 18
    }

    if (!am_driver) {
      show_player_ending_game(pack->info.game.team);
      end_game_experience(pack->info.game.team, pack->info.game.winner);
    }
  }
}



/* notice a player's request to save his stats before he leaves the game */

notice_save_stats(pack)
Packet *pack;
{
  int pnum;
  PersonRec *p;

  /* check for validity of person id number */
  if (DEBUG) printf("Noticing save stats request from %d\n", pack->from);
  pnum = person_array(pack->info.stat.id);
  if (pnum<0 || pnum>persons_in_game) {
    printf("Warning: unknow person's stats\n");
    return;
  }

  /* check the person's records, update ours */
  gameperson[pnum]->kills = pack->info.stat.kills;
  gameperson[pnum]->losses = pack->info.stat.losses;
  gameperson[pnum]->games = pack->info.stat.games;
  strcpy(gameperson[pnum]->rank, pack->info.stat.rank);
  gameperson[pnum]->level = pack->info.stat.level;

  if (am_driver) {
    /* get the person's record out of password file if there is one */
    p = get_player(gameperson[pnum]->name);

    /* if there wasn't one then don't try to modify anything, he must
       be one of those unrecorded monsters */
    if (!p) return;

    /* otherwise modify the record and save it */
    p->ExpPts = pack->info.stat.ExpPts;
    strcpy(p->rank, pack->info.stat.rank);
    strcpy(p->login, gameperson[pnum]->login);
    strcpy(p->host, gameperson[pnum]->host);
    p->kills = pack->info.stat.kills;
    p->losses = pack->info.stat.losses;
    p->games = pack->info.stat.games;
    modify_player(p);
    free(p);
    if (DEBUG) printf("Saved stats of person %d\n", pack->info.stat.id);
  }
}
 



/* notice a packet that is meant for person queues */

notice_queued_packet(pack)
Packet *pack;
{
  register int i;
  
  if (pack->to == TO_ALL)
    for (i=0; i<num_persons; i++) add_packet_to_queue(i, pack);
  else
    add_packet_to_queue(person_array(pack->to), pack);
}



/* add a queued packet to a person's queue */

add_packet_to_queue(pnum, pack)
int pnum;
Packet *pack;
{
  QPacket *new;
  
  /* check person number */
  if (pnum < 0 || pnum >= num_persons) {
    if (DEBUG) printf("Warning: bad person number for queuing packet\n");
    return;
  }

  /* duplicate packet */
  new = duplicate_packet(pack);

  /* add the packet to end of person's queue */
  if (person[pnum]->pqueue == NULL) person[pnum]->pqueue = new;
  else person[pnum]->final->next = new;

  person[pnum]->final = new;
}



/* duplicate a packet into a newly allocated packet queue record */

QPacket *duplicate_packet(pack)
Packet *pack;
{
  QPacket *new;
  Packet  *ppart;

  /* allocate queue packet and an area for a copy of the packet part */
  new = (QPacket *) malloc(sizeof(QPacket));
  ppart = (Packet *) malloc(sizeof(Packet));
  if (new == NULL || ppart == NULL)
    Gerror("out of memory making packet duplicate");

  /* copy the packet part into new packet structure */
  bcopy(pack, ppart, sizeof(Packet));

  /* link up the packet part to the queue packet structure */
  new->pack = ppart;
  new->next = NULL;

  /* return the new duplicate */
  return new;
}



D 2
/* try to log a packet to the disk log file, only the driver should be
   trying to do this */
E 2
I 2
/* try to log a packet to the disk log file, only the driver should ever
   try to do this.  A reply will to sent to the sender.  The reply will
   be generated by the Burt code. */
E 2

try_to_log_packet(packet)
Packet *packet;
{
  Packet msgpack;
D 2
  char query[300];
E 2
I 2
  char query[400];
E 2
  char *response;
I 4
  int sender = person_array(packet->from);
E 4
  
  if (DEBUG) printf("Driver logging packet to file\n");

  if (packet->type == MESSAGE) {
I 4
    char *name, s[250];

E 4
D 2
    /* try to log the message from player */
E 2
I 2
    /* try to place a copy of the message into the log file */
E 2
D 4
    log_message(packet->info.msg);
E 4
I 4
    if (sender >= 0) name = gameperson[sender]->name;
    else name = "unnamed";
    sprintf(s, "%-6s %s", name, packet->info.msg);
    log_message(s);
E 4

    /* set return message depending on gm's personality */
    first_and_rest(packet->info.msg, query);	/* skip from->to info */
D 4
    response = query_personality(0, query);
E 4
I 4
    response = query_personality(0, query);     /* get burt code response */
E 4
    if (response) {
D 2
      /* send a message to the person that sent us the log message */
E 2
I 2
      /* send a message to the person that sent us the log message
         and record our response in the log file */
E 2
      msgpack.type = MESSAGE;
      sprintf(msgpack.info.msg, "%s->%d %s", GM_NAME, packet->from, response);
      address_packet(&msgpack, FROM_GM, packet->from);
D 4
      send_to_person(person_array(packet->from), &msgpack);
      log_message(msgpack.info.msg);
E 4
I 4
D 12
      send_to_person(sender, &msgpack);
E 12
I 12
      send_to_person(sender, &msgpack, FALSE);
E 12
      sprintf(s, "%-6s %s", name, msgpack.info.msg);
      log_message(s);
E 4
    }
  }
E 19
}
E 1
