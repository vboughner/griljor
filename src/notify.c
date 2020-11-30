/***************************************************************************
 * @(#) notify.c 1.8 - last change made 08/07/92
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

/* datagram socket stuff for communication during game between players */

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

static unsigned int global_junk; /* Used in notify.c macros */

/*
 * OUTGOING DATAGRAM-STUFFER MACROS
 *
 * Translate data into network-transmittable form.
 */

CARD8	junkCARD8;
CARD16	junkCARD16;
CARD32	junkCARD32;
char *junkstring;

#define Add8(p,b)	(((p)->info[(p)->len] = b), (p)->len += 4)
#define Add16(p,s)	((*(CARD16 *)((p)->info+(p)->len) = htons(s)), (p)->len += 4)
#define Add32(p,l)	((*(CARD32 *)((p)->info+(p)->len) = htonl(l)), (p)->len += 4)
/*	(*(CARD16 *)((p)->info+(p)->len) = htons(global_junk));	\ */
#define AddString(p,s) if (1) {					\
	global_junk = strlen(s);				\
        bcopy(s, (p)->info+(p)->len, global_junk + 1);		\
	(p)->len += global_junk + 1;				\
	if ((p)->len & 3) (p)->len += 4 - (p)->len & 3;		\
}
#define AddArbitrary(p,d,l) if (1) {				\
	bcopy(d, (p)->info + (p)->len, l);			\
	(p)->len += l;						\
	if ((p)->len & 3) (p)->len += 4 - (p)->len & 3;		\
}
#define SetPacketFrom(p,f)	(*((CARD16 *)((p)->info + FROM_OFFSET)) = htons(f))
#define SetPacketTo(p,t)	(*((CARD16 *)((p)->info + TO_OFFSET)) = htons(t))
#define PacketInit(p,t) {			\
	((Packet *)p)->info[0] = t;		\
	((Packet *)p)->len = RANDOM_DATA_OFFSET;\
}
#define SetPassword(p,pw)	(*((CARD16 *)((p)->info + PASSWORD_OFFSET)) = htonl(pw))

/*
 * EVERYTHING BELOW THIS LINE HAS NOT YET BEEN COMPLETELY REVAMPED.
 *
 * I'm sorta working on it and moving pieces around as I figure out how to
 * optimize things and what exactly needs to be done and what can go away.
 * it's like playing tetris.
 *
 *														-blojo@soda
 */

/* =================== D A T A G R A M  socket stuff =================== */

/* ============================ O U T P U T =============================== */



/* Send a packet to an address. This procedure will automatically
   fill in the BOSS_PASSWORD for you. (current game password) */

send_to_address(address, packet, reliable)
Sockaddr	*address;
BossPacket	*packet;
int		reliable;
{
  int		ret;

  SetPassword(packet,BOSS_PASSWORD);

  if (DEBUG) slimy_debug_packet(packet);
  ret = sendto(thesocket, packet->info, packet->len, 0, address,
	       sizeof(Sockaddr));

  if (ret == -1) {
    printf("Socket Boss: error num: %d\n", errno);
    Berror("failed on sendto other player");
  }
}



/* send the driver a packet (you shouldn't call this if you are the driver) */

send_to_driver(packet, reliable)
BossPacket *packet;
int reliable;
{
  if (Iamboss && BOSS_DEBUG)
    fprintf(stderr, "Socket Boss: warning: send_to_driver called\n");

  /* send the packet to the boss */
  if (!am_driver) send_to_address(&driveraddr, packet, reliable);
}



/* Send a packet to a particular person (in person array).  BOSS_PASSWORD
   will be filled in for you and the correct parent and address will be used,
   you still have to fill in the "to" variable in the packet yourself. */

send_to_person(pnum, packet, reliable)
int pnum;
BossPacket *packet;
int reliable;
{
  /* check for out of bounds person */
  if (pnum >= persons_in_game || pnum < 0) {
    DPRINTF("No person %d exists yet\n", pnum);
    return;
  }

  /* send the packet to their parent if neccessary*/
  if (gameperson[pnum]->parent == gameperson[pnum]->id)
    send_to_address(&(gameperson[pnum]->address), packet, reliable);
  else
    send_to_person(person_array(gameperson[pnum]->parent), packet, reliable);
}



/* send to packet to a particular person, that person being identified
   by his id number */

send_to_id(idnum, pack, reliable)
int idnum;
BossPacket *pack;
int reliable;
{
  int pnum;

  pnum = person_array(idnum);
  if (pnum == -1) return;

  send_to_person(pnum, pack, reliable);
}



		    
/* Send a packet to all players in the game (that means only one packet per
   player process not to every person).  You are responsible for putting
   TO_ALL as the "to" variable inside the packet */

send_to_players(packet, reliable)
BossPacket *packet;
int reliable;
{
  register int i;
  
  /* we send the packet to all persons (except ours) who are own parents */
  for (i=num_persons; i<persons_in_game; i++)
    if (gameperson[i]->parent == gameperson[i]->id)
        send_to_person(i, packet, reliable);
}


/* Send a packet only to those players who have one of their persons in
   the given room.  As usual you should address the packet TO_ALL.  This
   routine relies on the fact that all persons store the network address
   of their parent inside their own structure. */

send_to_room(roomnum, packet, reliable)
int roomnum;
BossPacket *packet;
int reliable;
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
      send_to_address(&(ptr->person->address), packet, reliable);
}


/* Send one packet only to those players who do NOT have one of their persons
   in the given room.  Send another packet to everyone else. */

send_in_and_out_of_room(roomnum, inner_packet, outer_packet, reliable)
int roomnum;
BossPacket *inner_packet, *outer_packet;
int reliable;
{
  int i, *person_list;
  PersonList *ptr;
  
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
}



/* Send a packet to everyone important (that means players and driver) */

send_to_important(packet, reliable)
BossPacket *packet;
{
  send_to_players(packet, reliable);
  send_to_driver(packet, reliable);
}



/* Send a packet to a given string name, which could be a person name, could
   be "all", could be a team name, or could be GM_NAME.  Return the id number
   of the person who is going to get message (or return TO_ALL or TO_GM).
   Return -99 if no one resonable could be found. */

int send_to_name(fromid, name, pack, reliable)
int fromid;
char *name;
Packet *pack;
int reliable;
{
  register int	i;
  int		temp, pnum = -1, matchlen = 0;
# define BADRESULT -99
  
  /* check for send to all */
  if (strmatch(name, "all")==3) {
    address_packet(pack, fromid, TO_ALL);
    send_to_players(pack, reliable);
    return TO_ALL;
  }

  /* check for send to everyone in room */
  if ((strmatch(name, "say")==3 && strlen(name)==3) ||
      (strmatch(name, "room")==4 && strlen(name)==4)) {
    address_packet(pack, fromid, TO_ALL);
    send_to_room(pack, reliable);
    return TO_ALL;
  }

  /* or perhaps to the game master */
  else if (strmatch(name, GM_NAME)==strlen(GM_NAME)) {
    address_packet(pack, fromid, TO_GM);
    send_to_driver(pack, reliable);
    return TO_GM;
  }

  /* or see if it is an id number */
  else if (isdigit(name[0])) {
    pnum = person_array(atoi(name));
    if (pnum < 0  ||  pnum >= persons_in_game) return BADRESULT; /* bad id */
    address_packet(pack, fromid, gameperson[pnum]->id);
    send_to_person(pnum, pack, reliable);
    return gameperson[pnum]->id;
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
    if (pnum < 0) return BADRESULT;  /* return if no matching name found */
    /* otherwise send off the packet */
    address_packet(pack, fromid, gameperson[pnum]->id);
    send_to_person(pnum, pack, reliable);
    return gameperson[pnum]->id;
  }

  return BADRESULT;
}



/* ============== P A C K E T  P R E P A R A T I O N ============== */

/*
 * CARD8/CARD16/CARD32:		8-, 16-, or 32-byte cardinal
 * STRING:			A Pascal-style string which consists
 *				of a CARD16 describing the string's
 *				length, followed by that much string.
 */

#define FLAG_DEITY 1
#define FLAG_LISTED 2

/*
 * MY_INFO:
 *
 * CARD16	parent person's ID number
 * STRING	name
 * STRING	login
 * STRING	hostname
 * STRING	rank
 * CARD16	level
 * CARD8	team
 * CARD8	flags (deity, listed)
 */

prepare_my_info(pnum, pack)
int pnum;
Packet *pack;
{
  unsigned char junk;

  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_INFO about person %d\n", pnum);

  /* prepare packet */
  PacketInit(pack, MY_INFO);
  Add16(pack, gameperson[pnum]->parent);
  AddString(pack, gameperson[pnum]->name);
  AddString(pack, gameperson[pnum]->login);
  AddString(pack, gameperson[pnum]->host);
  AddString(pack, gameperson[pnum]->rank);
  Add16(pack, gameperson[pnum]->level);
  Add8(pack, gameperson[pnum]->team);
  junk = ((gameperson[pnum]->listed) ? (FLAG_LISTED) : (0));
  junk = junk | ((gameperson[pnum]->deity) ? (FLAG_DEITY) : (0));
  Add8(pack, junk);
}


/*
 * MY_BITMAP:
 *
 * CARD8*BITMAP_ARRAY_SIZE	bitmap data
 */

/* prepare a MY_BITMAP packet for a certain person */

prepare_my_bitmap(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_BITMAP about person %d\n", pnum);

  /* prepare packet */
  PacketInit(pack, MY_BITMAP);
  AddArbitrary(pack, gameperson[pnum]->bitmap, BITMAP_ARRAY_SIZE);
}


/*
 * MY_MASK:
 *
 * CARD8*BITMAP_ARRAY_SIZE	bitmap data
 */

/* prepare a MY_MASK packet for a certain person */

prepare_my_mask(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_MASK about person %d\n", pnum);

  /* prepare packet */
  PacketInit(pack, MY_MASK);
  AddArbitrary(pack, gameperson[pnum]->mask, BITMAP_ARRAY_SIZE);
}

/*
 * MY_LOCATION:
 *
 * CARD8	x
 * CARD8	y
 * CARD16	room
 * CARD16	appearance
 */

/* prepare a MY_LOCATION packet to all whom it may concern */

prepare_my_location(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_LOCATION about person %d\n", pnum);

  PacketInit(pack, MY_LOCATION);
  Add8(pack, gameperson[pnum]->x);
  Add8(pack, gameperson[pnum]->y);
  Add16(pack, gameperson[pnum]->room);
  Add16(pack, gameperson[pnum]->appearance);
}

prepare_far_my_location(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_LOCATION about person %d\n", pnum);

  PacketInit(pack, MY_LOCATION);
  Add8(pack, -1);
  Add8(pack, -1);
  Add16(pack, gameperson[pnum]->room);
  Add16(pack, gameperson[pnum]->appearance);
}

/*
 * MESSAGE:
 *
 * STRING	message
 */

/* prepare a message packet */

prepare_message(pack, msg)
Packet *pack;
char   *msg;
{
  PacketInit(pack, MESSAGE);
  AddString(pack, msg);
}

/*
 * REPORT:
 *
 * STRING	report
 */

prepare_report(pack, msg)
Packet *pack;
char *msg;
{
  PacketInit(pack, REPORT);
  AddString(pack, msg);
}

/*
 * WEAPON_FIRED
 *
 * CARD16	owner
 * CARD16	type
 * CARD8	hurts
 * CARD16	wait
 *   ^^ (prepare_missile_poop)
 * CARD16	room
 * CARD16	heading
 * CARD16	range
 * Arbitrary8	x[1],x[2],...x[range]
 * Arbitrary8	y[1],y[2],...y[range]
 *   ^^ (prepare_missile_course)
 * CARD8	direction
 *   ^^ (prepare_missile_direction)
 */

prepare_missile_packet(m, x, y)
Missile *m;
char *x, *y;
{
  Packet pack;
  PacketInit(&pack, WEAPON_FIRED);

  Add16(&pack, m->owner);
  Add16(&pack, m->type);
  Add8(&pack, m->hurts_owner);
  Add16(&pack, m->wait);

  Add16(&pack, m->room);
  Add16(&pack, m->heading);
  Add16(&pack, m->range);
  AddArbitrary(&pack, x, m->range);
  AddArbitrary(&pack, y, m->range);

  Add8(&pack, m->direction);

  address_packet(&pack, m->owner, TO_ALL);
  send_to_room(m->room, &pack, FALSE);
}


/*
 * MULTI_FIRE
 *
 * CARD16	type
 * CARD16	owner
 * CARD16	hurts
 * CARD16	room
 * CARD16	include_start
 * CARD16	number-of-missiles
 * Arbitrary8	x1[1],...x1[number-of-missiles]
 * Arbitrary8	x2[1],...x2[number-of-missiles]
 * Arbitrary8	y1[1],...y1[number-of-missiles]
 * Arbitrary8	y2[1],...y2[number-of-missiles]
 */

prepare_and_send_multimissile_packet(MultiPack *m)
{
  Packet p;

  PacketInit(&p, MULTI_FIRE);
  Add16(&p, m->miss.type);
  Add16(&p, m->miss.owner);
  Add16(&p, m->miss.hurts_owner);
  Add16(&p, m->miss.room);
  Add16(&p, m->include_start);
  Add16(&p, m->number);
  AddArbitrary(&p, m->x1, m->number);
  AddArbitrary(&p, m->x2, m->number);
  AddArbitrary(&p, m->y1, m->number);
  AddArbitrary(&p, m->y2, m->number);

  address_packet(&p, m->miss.owner, TO_ALL);
  send_to_room(m->miss.room, &p, FALSE);
}


/* address a packet by setting its contents according to parameters */

address_packet(pack, from, to)
Packet *pack;
int from, to;
{
  SetPacketFrom(pack,from);
  SetPacketTo(pack,to);
  pack->to = to;
}



/* ==================== O U T G O I N G  packets ========================= */

/* let old players know we are joining the game, send this info to the
   driver as well so that he will know what team we have chosen. */

notify_old_players()
{
  int i;
  Packet pack;
  char	 s[MSG_LENGTH];
  
  DPRINTF("Tell old players we are joining the game\n");

  /* send all players info packets for each of my persons */
  for (i=0; i<num_persons; i++) {

    /* send MY_INFO packet to all players */
    prepare_my_info(i, &pack);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
    send_to_important(&pack, TRUE);

    /* send MY_MASK packet to all players */
    prepare_my_mask(i, &pack);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
    send_to_players(&pack, TRUE);

    /* send MY_BITMAP packet to all players */
    prepare_my_bitmap(i, &pack);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
    send_to_players(&pack, TRUE);

    /* send SAVE_STATS packet so players know what I've got */
    notify_of_stats(i);

    /* send MESSAGE concerning my entry to all persons */
    sprintf(s, "%s->ALL %s (%d) joining %s", GM_NAME,
	    gameperson[i]->name,gameperson[i]->id,
	    get_team_name(gameperson[i]->team));

    prepare_message(&pack, s);
    address_packet(&pack, gameperson[i]->id, TO_ALL);
    send_to_players(&pack, FALSE);
  }
}



/* making a change to the map, tell all players and the driver exactly
   what change has been made.  See the documentation on CHANGE_MAP in
   socket.h for explanations of the parameters. */

/*
 * CHANGE_MAP  (This should probably go in a prepare_ function above.)
 *
 * CARD16		room
 * CARD8		x
 * CARD8		y
 * CARD16		which
 * CARD16		old
 * CARD16		new
 */

notify_all_of_map_change(roomnum, x, y, which, old, new, recobjptr)
int roomnum, x, y, which, old, new;
RecordedObj *recobjptr;
{
  Packet pack;
  
  /* prepare the packet */
  DPRINTF("Telling all of map change\n");
  PacketInit(&pack, CHANGE_MAP);
  Add16(&pack, roomnum);
  Add8(&pack, x);
  Add8(&pack, y);
  Add16(&pack, which);
  Add16(&pack, old);
  Add16(&pack, new);
  if (recobjptr) {
      Add8(&pack, recobjptr->x);
      Add8(&pack, recobjptr->y);
      Add8(&pack, recobjptr->objtype);
      Add16(&pack, recobjptr->detail);
      Add16(&pack, recobjptr->infox);
      Add16(&pack, recobjptr->infoy);
      Add16(&pack, recobjptr->zinger);
      Add16(&pack, recobjptr->extra[0]);
      Add16(&pack, recobjptr->extra[1]);
      Add16(&pack, recobjptr->extra[2]);
  }
  /* send the packet to players and driver, anonymously */
  address_packet(&pack, -1, TO_ALL);
  send_to_important(&pack, TRUE);

  /* order the object's replacement for ourselves as well */
  change_own_map_square(roomnum, x, y, which, (uc)old, (uc)new, recobjptr);

  /* check to see if our team has won the game because of my change */
  if (info[(uc)new]->flag) look_for_wins();
}



/* let everyone in game know where person is (changing rooms),
   players not in the target room only get a partial location (room) */

notify_all_of_location(pnum)
int pnum;
{
  Packet inner_pack, outer_pack;
  
  prepare_my_location(pnum, &inner_pack);
  prepare_far_my_location(pnum, &outer_pack);
  address_packet(&inner_pack, gameperson[pnum]->id, TO_ALL);
  address_packet(&outer_pack, gameperson[pnum]->id, TO_ALL);

  send_in_and_out_of_room(gameperson[pnum]->room, &inner_pack,
			  &outer_pack, TRUE);
}




/* let everyone in this persons room know where he is (changing spots) */

notify_room_of_location(pnum)
int pnum;
{
  Packet pack;
  
  prepare_my_location(pnum, &pack);
  address_packet(&pack, gameperson[pnum]->id, TO_ALL);
  send_to_room(gameperson[pnum]->room, &pack, FALSE);
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
  send_to_person(pnum, &pack, FALSE);
}



/* notify the recipient(s) of a sent message, return id num of receiver */

int notify_of_message(fromid, to_name, msg)
int fromid;
char *to_name, *msg;
{
  Packet msgpack;

  /* prepare and send the packet */
  prepare_message(&msgpack, msg);
  return send_to_name(fromid, to_name, &msgpack, FALSE);
}



/* notify the given person id of some information */

notify_of_report(fromid, to_id, msg)
int fromid, to_id;
char *msg;
{
  Packet rptpack;

  /* prepare and send the packet */

  PacketInit(&rptpack, REPORT);
  AddString(&rptpack, msg);
  address_packet(&rptpack, fromid, to_id);
  send_to_id(to_id, &rptpack, FALSE);
}



/* Tell person about an experience point bonus and give it to him */

/*
 * ADD_EXPERIENCE
 *
 * CARD16	to
 * CARD16	amount
 * STRING	message
 */

notify_of_experience(fromid, to_id, amount, msg)
int fromid, to_id, amount;
char *msg;
{
  Packet pack;

  /* prepare and send the packet */
  PacketInit(&pack,ADD_EXPERIENCE);
  Add16(&pack,to_id);
  Add16(&pack,amount);
  AddString(&pack,msg);
  address_packet(&pack, fromid, to_id);
  send_to_id(to_id, &pack, FALSE);
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
  PacketInit(&pack,GAME_OVER);
  Add16(&pack,winner);
  Add16(&pack,team);

  /* send it to players and driver */
  address_packet(&pack, winner, TO_ALL);
  send_to_important(&pack, FALSE);

  /* send it again to be sure everyone gets it */
  send_to_important(&pack, TRUE);

  /* notice for myself that the game is over */
  notice_game_over(&pack);
}



/* notify the driver and players of one of my player person's stats,
   request that driver save the stats in the password file. */

/*
 * SAVE_STATS
 * 
 * CARD16				id
 * CARD32				experience
 * STRING				rank
 * CARD16				level
 * CARD32				kills
 * CARD32				losses
 * CARD32				games
 */

notify_of_stats(pnum)
int pnum;
{
  Packet pack;

  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending SAVE_STATS about person %d\n", pnum);

  /* prepare packet and send to driver and players */
  PacketInit(&pack, SAVE_STATS);

  DPRINTF("Sending STATS id # of %d.\n", gameperson[pnum]->id);

  Add16(&pack, gameperson[pnum]->id);
  Add32(&pack, person[pnum]->experience);
  AddString(&pack, gameperson[pnum]->rank);
  Add16(&pack, gameperson[pnum]->level);
  Add32(&pack, gameperson[pnum]->kills);
  Add32(&pack, gameperson[pnum]->losses);
  Add32(&pack, gameperson[pnum]->games);
  address_packet(&pack, gameperson[pnum]->id, TO_ALL);
  send_to_important(&pack, FALSE);
}


/* tell someone they killed me */

/*
 * YOU_KILLED_ME
 */ 

notify_of_kill(victim_id, killer_id)
{
  Packet pack;

  /* check that this is one of my own persons */
  if (person_array(victim_id) >= num_persons)
    printf("Warning: sending YOU_KILLED_ME about someone else");

  /* prepare packet and send it to person */
  PacketInit(&pack, YOU_KILLED_ME);
  address_packet(&pack, victim_id, killer_id);
  send_to_id(killer_id, &pack, FALSE);
}



/* persons leave game by sending everyone a LEAVING_GAME packet */

/*
 * LEAVING_GAME
 */

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

  /* prepare the LEAVING_GAME packet */
  PacketInit(&leavepack, LEAVING_GAME);
  address_packet(&leavepack, gameperson[num]->id, TO_ALL);

  /* send the packet off to everyone */
  send_to_important(&leavepack, TRUE);
}



/* for players, leave the game */

allLeaveGame()
{
  int i;

  /* notify other players and driver of leaving persons */
  if (!am_driver) for (i=0; i<num_persons; i++)  leaveGame(i);
}
