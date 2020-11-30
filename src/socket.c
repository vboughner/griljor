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

int		Iamboss;	/* TRUE if I am the driver/server */
Sockaddr	ownaddr;	/* my own network address */
int		thesocket;	/* my own main socket */
Sockaddr	recaddr;	/* address for receiving new players */
int		recsocket;	/* socket where receiving is done */
Sockaddr	driveraddr;	/* address of drivers datagram socket */
long		end_of_world = GAME_GOING;	/* countdown to game end */
long		last_io = 0;	/* last gameclick io was received on */
char		dmessage[MSG_LENGTH];	/* last message from driver */
short		game_password;		/* current game's packet password */

/* internal variables for this file only */
static int	initialized = FALSE;	/* TRUE once we have done selfInit */



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

  /* get the map */
  if (am_player) joiner_message("Reading and initializing map...");
  read_map_from_socket(socknum);

  /* get the object definitions */
  if (am_player) joiner_message("Loading object definitions...");
  receive_obj_file(socknum);

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

  /* add him to our game persons list, and fill in and send his info pack */
  integrate_into_game(socknum, &info_pack);

  /* give him the rest of the game info as well */
  send_game_info(socknum, &info_pack);
}

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

  /* Calculte his overall rank */
  calculate_rank(info_pack);

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

  /* send object definitions */
  send_obj_file(socknum);

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




/* ==================== MAP reading and writing ======================= */


/* Write current game map to given stream socket.  This procedure makes
   heavy use of global variables from map.c */

write_map_to_socket(socknum)
int socknum;
{
  register int i, j;
  MemObj *ptr;
  
  /* send the mapstats info record */
  if (DEBUG) printf("Send current map over the socket\n");
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
}



/* Read current game map from given stream socket, allocating memory for
   it as we load it in.  This procedure makes heavy use of global variables
   from map.c */

read_map_from_socket(socknum)
int socknum;
{
  register int	i, j;
  int		how_many_rooms;
  RecordedObj	o;
  
  /* get the mapstats record with info about the map in it */
  if (DEBUG) printf("Load current map over the socket\n");
  read_from_socket(socknum, (char *)&mapstats, sizeof(MapInfo));
  how_many_rooms = mapstats.rooms;

  /* allocate a room array for the map */
  room = make_room_array(how_many_rooms);

  /* allocate the array of lists of recorded objs in the rooms and fill it */
  roomrecobj = make_room_obj_list_array(how_many_rooms);
  initialize_room_obj_list_array(roomrecobj, how_many_rooms);

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
}



/* Close up shop, this player or driver wants to quit */

endBoss()
{
  /* close the main socket */
  close(thesocket);

  /* the driver must also close his new player socket */
  if (Iamboss) close(recsocket);
}



/* general socket boss error routine */

Berror(s)
char *s;
{
  fprintf(stderr, "Griljor Socket Boss: %s\n", s);
  exit(1);
}
