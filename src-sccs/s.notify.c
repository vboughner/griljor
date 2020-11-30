h39698
s 00000/00000/00981
d D 1.8 92/08/07 01:05:02 vbo 8 7
c source copied to a separate tree for work on new map and object format
e
s 00004/00003/00977
d D 1.7 92/01/22 21:32:49 labc-4lc 7 6
c fixed bugs in map updates and missile noticing (minor changes)
e
s 00005/00001/00975
d D 1.6 92/01/20 17:41:51 labc-4lc 6 5
c fixed a couple places where GetTo() FLAG_LISTED were needed
e
s 00104/00013/00872
d D 1.5 92/01/20 16:31:03 labc-4lc 5 4
c blojo finished changes necesssary for network packets
e
s 00027/00052/00858
d D 1.4 91/12/15 23:06:53 labc-4lc 4 3
c Fixing datagram incompatibilities.   -bloo
e
s 00251/00116/00659
d D 1.3 91/12/15 15:09:31 labc-4lc 3 2
c blojo's datagram mungification, attempt #1
e
s 00007/00007/00768
d D 1.2 91/12/07 21:41:07 labc-4lc 2 1
c split socket.c into three files and change DAMAGE_REPORT to REPORT
e
s 00775/00000/00000
d D 1.1 91/12/07 20:57:22 labc-4lc 1 0
c date and time created 91/12/07 20:57:22 by labc-4lc
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
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

I 3
D 4
#define DPRINTF if (DEBUG) printf
E 3

E 4
I 3
static unsigned int global_junk; /* Used in notify.c macros */
E 3

I 3
/*
D 4
 * MACHINE-SPECIFIC TYPEDEFS
 *
 * These are to insure proper translation of data for network transmission.
 * They will soon be moved to a config.h file or somesimilar.
 */

#define SUN3

#ifdef SUN3
#	define USUAL
#endif SUN3

#ifdef DECSTATION
#	define USUAL
#endif /* DECSTATION */

#ifdef USUAL
	typedef unsigned char		CARD8;
	typedef unsigned short		CARD16;
	typedef unsigned int		CARD32;
#endif /* USUAL */

#ifdef HAILMARY
	typedef unsigned : 8		CARD8;
	typedef unsigned : 16		CARD16;
	typedef unsigned : 32		CARD32;
#endif /* HAILMARY */

/*
E 4
 * OUTGOING DATAGRAM-STUFFER MACROS
 *
 * Translate data into network-transmittable form.
 */

CARD8	junkCARD8;
CARD16	junkCARD16;
CARD32	junkCARD32;
char *junkstring;

D 4
#define Add8(p,b)	(((p)->data+(p)->len = b), (p)->len += 4)
#define Add16(p,s)	((*(CARD16 *)((p)->data+(p)->len) = htons(s)), (p)->len += 4)
#define Add32(p,l)	((*(CARD32 *)((p)->data+(p)->len) = htonl(l)), (p)->len += 4)
#define AddString(p,s) {					\
E 4
I 4
#define Add8(p,b)	(((p)->info[(p)->len] = b), (p)->len += 4)
#define Add16(p,s)	((*(CARD16 *)((p)->info+(p)->len) = htons(s)), (p)->len += 4)
#define Add32(p,l)	((*(CARD32 *)((p)->info+(p)->len) = htonl(l)), (p)->len += 4)
I 5
/*	(*(CARD16 *)((p)->info+(p)->len) = htons(global_junk));	\ */
E 5
#define AddString(p,s) if (1) {					\
E 4
	global_junk = strlen(s);				\
D 4
	(*(CARD16 *)((p)->data+(p)->len) = htons(global_junk));	\
        bcopy(s, (p)->len + 4, global_junk + 1);		\
E 4
I 4
D 5
	(*(CARD16 *)((p)->info+(p)->len) = htons(global_junk));	\
        bcopy(s, (p)->info+(p)->len + 4, global_junk + 1);	\
E 4
	(p)->len += global_junk + 4;				\
E 5
I 5
        bcopy(s, (p)->info+(p)->len, global_junk + 1);		\
	(p)->len += global_junk + 1;				\
E 5
D 4
	if (p)->len & 3) (p)->len += 4 - (p)->len & 3;		\
E 4
I 4
	if ((p)->len & 3) (p)->len += 4 - (p)->len & 3;		\
E 4
}
D 4
#define AddArbitrary(p,d,l) {					\
	bcopy(d, (p)->data + (p)->len, l);			\
E 4
I 4
#define AddArbitrary(p,d,l) if (1) {				\
	bcopy(d, (p)->info + (p)->len, l);			\
E 4
	(p)->len += l;						\
D 4
	if (p)->len & 3) (p)->len += 4 - (p)->len & 3;		\
E 4
I 4
	if ((p)->len & 3) (p)->len += 4 - (p)->len & 3;		\
E 4
}
D 5
#define SetPacketFrom(p,f)	(*((CARD16 *)((p) + FROM_OFFSET)) = htons(f))
#define SetPacketTo(p,t)	(*((CARD16 *)((p) + TO_OFFSET)) = htons(t))
E 5
I 5
#define SetPacketFrom(p,f)	(*((CARD16 *)((p)->info + FROM_OFFSET)) = htons(f))
#define SetPacketTo(p,t)	(*((CARD16 *)((p)->info + TO_OFFSET)) = htons(t))
E 5
#define PacketInit(p,t) {			\
D 4
	p->info[0] = t;				\
	p->len = 9;				\
E 4
I 4
	((Packet *)p)->info[0] = t;		\
D 5
	((Packet *)p)->len = 9;			\
E 5
I 5
	((Packet *)p)->len = RANDOM_DATA_OFFSET;\
E 5
E 4
}
I 5
#define SetPassword(p,pw)	(*((CARD16 *)((p)->info + PASSWORD_OFFSET)) = htonl(pw))
E 5

/*
 * EVERYTHING BELOW THIS LINE HAS NOT YET BEEN COMPLETELY REVAMPED.
 *
 * I'm sorta working on it and moving pieces around as I figure out how to
 * optimize things and what exactly needs to be done and what can go away.
 * it's like playing tetris.
 *
 *														-blojo@soda
 */

E 3
/* =================== D A T A G R A M  socket stuff =================== */

/* ============================ O U T P U T =============================== */


I 5

E 5
/* Send a packet to an address. This procedure will automatically
   fill in the BOSS_PASSWORD for you. (current game password) */

send_to_address(address, packet, reliable)
Sockaddr	*address;
BossPacket	*packet;
int		reliable;
{
D 4
  int		tolen, ret;
E 4
I 4
  int		ret;
E 4
D 5
  
E 5
I 5

E 5
D 3
  packet->password = BOSS_PASSWORD;
E 3
I 3
  SetPassword(packet,BOSS_PASSWORD);
E 3

I 5
D 7
  slimy_debug_packet(packet);
E 7
I 7
  if (DEBUG) slimy_debug_packet(packet);
E 7
E 5
D 4
  tolen = sizeof(Sockaddr);
D 3
  ret = sendto(thesocket, packet, sizeof(BossPacket), 0, address, tolen);
E 3
I 3
  ret = sendto(thesocket, packet->info, packet->len, 0, address, tolen);
E 4
I 4
  ret = sendto(thesocket, packet->info, packet->len, 0, address,
D 5
	       sizeof(*address));
E 5
I 5
	       sizeof(Sockaddr));
E 5
E 4
E 3

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
D 3
  if (DEBUG) printf("Sending packet of type %d to driver\n", packet->type);
E 3
I 3
D 4
  DPRINTF("Sending packet of type %d to driver\n", packet->type);
E 4
E 3
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
D 3
  if (DEBUG)
    printf("Sending packet of type %d to person element %d\n", packet->type, pnum);
E 3
I 3
D 4
  DPRINTF("Sending packet of type %d to person element %d\n", packet->type, pnum);
E 3

E 4
  /* check for out of bounds person */
  if (pnum >= persons_in_game || pnum < 0) {
D 3
    if (DEBUG) printf("No person %d exists yet\n", pnum);
E 3
I 3
    DPRINTF("No person %d exists yet\n", pnum);
E 3
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
D 3
	send_to_person(i, packet, reliable);
E 3
I 3
        send_to_person(i, packet, reliable);
E 3
}


D 3

E 3
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
D 3
   in the given room.  Send another packet to everyone else.
   As usual you should address the packet TO_ALL. */
E 3
I 3
   in the given room.  Send another packet to everyone else. */
E 3

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
D 3
  int		result = -99, temp, pnum = -1, matchlen = 0;
E 3
I 3
  int		temp, pnum = -1, matchlen = 0;
# define BADRESULT -99
E 3
  
  /* check for send to all */
  if (strmatch(name, "all")==3) {
    address_packet(pack, fromid, TO_ALL);
    send_to_players(pack, reliable);
D 3
    result = TO_ALL;
E 3
I 3
    return TO_ALL;
E 3
  }

  /* check for send to everyone in room */
  if ((strmatch(name, "say")==3 && strlen(name)==3) ||
      (strmatch(name, "room")==4 && strlen(name)==4)) {
    address_packet(pack, fromid, TO_ALL);
    send_to_room(pack, reliable);
D 3
    result = TO_ALL;
E 3
I 3
    return TO_ALL;
E 3
  }

  /* or perhaps to the game master */
  else if (strmatch(name, GM_NAME)==strlen(GM_NAME)) {
    address_packet(pack, fromid, TO_GM);
    send_to_driver(pack, reliable);
D 3
    result = TO_GM;
E 3
I 3
    return TO_GM;
E 3
  }

  /* or see if it is an id number */
  else if (isdigit(name[0])) {
    pnum = person_array(atoi(name));
D 3
    if (pnum < 0  ||  pnum >= persons_in_game) return result;	/* bad id */
E 3
I 3
    if (pnum < 0  ||  pnum >= persons_in_game) return BADRESULT; /* bad id */
E 3
    address_packet(pack, fromid, gameperson[pnum]->id);
    send_to_person(pnum, pack, reliable);
D 3
    result = gameperson[pnum]->id;
E 3
I 3
    return gameperson[pnum]->id;
E 3
  }

  /* finally, check to see if we get any resonable match to any person name */
  else {
    for (i=num_persons; i<persons_in_game; i++) {
      temp = strmatch(name, gameperson[i]->name);
      if (temp>matchlen && (temp>2 || temp==strlen(gameperson[i]->name))) {
        matchlen = temp;
D 3
	pnum = i;
E 3
I 3
		pnum = i;
E 3
      }
    }
D 3
    if (pnum < 0) return result;  /* return if no matching name found */
E 3
I 3
    if (pnum < 0) return BADRESULT;  /* return if no matching name found */
E 3
    /* otherwise send off the packet */
    address_packet(pack, fromid, gameperson[pnum]->id);
    send_to_person(pnum, pack, reliable);
D 3
    result = gameperson[pnum]->id;
E 3
I 3
    return gameperson[pnum]->id;
E 3
  }

D 3
  return result;
E 3
I 3
  return BADRESULT;
E 3
}



/* ============== P A C K E T  P R E P A R A T I O N ============== */

D 5
/* prepare a MY_INFO packet for a certain person */

I 3

E 5
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

E 3
prepare_my_info(pnum, pack)
int pnum;
Packet *pack;
{
I 6
  unsigned char junk;

E 6
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_INFO about person %d\n", pnum);

  /* prepare packet */
D 3
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
E 3
I 3
  PacketInit(pack, MY_INFO);
  Add16(pack, gameperson[pnum]->parent);
  AddString(pack, gameperson[pnum]->name);
  AddString(pack, gameperson[pnum]->login);
  AddString(pack, gameperson[pnum]->host);
  AddString(pack, gameperson[pnum]->rank);
  Add16(pack, gameperson[pnum]->level);
  Add8(pack, gameperson[pnum]->team);
D 6
  Add8(pack, gameperson[pnum]->listed | (gameperson[pnum]->deity << 1));
E 6
I 6
  junk = ((gameperson[pnum]->listed) ? (FLAG_LISTED) : (0));
  junk = junk | ((gameperson[pnum]->deity) ? (FLAG_DEITY) : (0));
  Add8(pack, junk);
E 6
E 3
}


I 3
/*
 * MY_BITMAP:
 *
 * CARD8*BITMAP_ARRAY_SIZE	bitmap data
 */

E 3
/* prepare a MY_BITMAP packet for a certain person */

prepare_my_bitmap(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_BITMAP about person %d\n", pnum);

  /* prepare packet */
D 3
  pack->type = MY_BITMAP;
  bitmapcpy(pack->info.bit, gameperson[pnum]->bitmap);
E 3
I 3
  PacketInit(pack, MY_BITMAP);
  AddArbitrary(pack, gameperson[pnum]->bitmap, BITMAP_ARRAY_SIZE);
D 7
/*  bitmapcpy(pack->info.bit, gameperson[pnum]->bitmap);*/
E 7
E 3
}


I 3
/*
 * MY_MASK:
 *
 * CARD8*BITMAP_ARRAY_SIZE	bitmap data
 */

E 3
/* prepare a MY_MASK packet for a certain person */

prepare_my_mask(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_MASK about person %d\n", pnum);

  /* prepare packet */
D 3
  pack->type = MY_MASK;
  bitmapcpy(pack->info.bit, gameperson[pnum]->mask);
E 3
I 3
  PacketInit(pack, MY_MASK);
  AddArbitrary(pack, gameperson[pnum]->mask, BITMAP_ARRAY_SIZE);
E 3
}

I 3
/*
 * MY_LOCATION:
 *
 * CARD8	x
 * CARD8	y
 * CARD16	room
 * CARD16	appearance
 */
E 3

/* prepare a MY_LOCATION packet to all whom it may concern */

prepare_my_location(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_LOCATION about person %d\n", pnum);

D 3
  pack->type = MY_LOCATION;
  pack->info.loc.x = gameperson[pnum]->x;
  pack->info.loc.y = gameperson[pnum]->y;
  pack->info.loc.room = gameperson[pnum]->room;
  pack->info.loc.appear = gameperson[pnum]->appearance;
E 3
I 3
  PacketInit(pack, MY_LOCATION);
  Add8(pack, gameperson[pnum]->x);
  Add8(pack, gameperson[pnum]->y);
  Add16(pack, gameperson[pnum]->room);
  Add16(pack, gameperson[pnum]->appearance);
E 3
}

I 3
prepare_far_my_location(pnum, pack)
int pnum;
Packet *pack;
{
  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending MY_LOCATION about person %d\n", pnum);
E 3

I 3
  PacketInit(pack, MY_LOCATION);
  Add8(pack, -1);
  Add8(pack, -1);
  Add16(pack, gameperson[pnum]->room);
  Add16(pack, gameperson[pnum]->appearance);
}
E 3

I 3
/*
 * MESSAGE:
 *
 * STRING	message
 */

E 3
/* prepare a message packet */

prepare_message(pack, msg)
Packet *pack;
char   *msg;
{
D 3
  pack->type = MESSAGE;
  (void) strncpy(pack->info.msg, msg, MSG_LENGTH-1);
  pack->info.msg[MSG_LENGTH-1] = '\0';
E 3
I 3
D 5
  Add8(pack, MESSAGE);
E 5
I 5
  PacketInit(pack, MESSAGE);
E 5
  AddString(pack, msg);
E 3
}

I 5
/*
 * REPORT:
 *
 * STRING	report
 */
E 5

I 5
prepare_report(pack, msg)
Packet *pack;
char *msg;
{
  PacketInit(pack, REPORT);
  AddString(pack, msg);
}
E 5

I 5
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
I 7
 * CARD16	type
E 7
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
I 7
  Add16(&p, m->miss.type);
E 7
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


E 5
/* address a packet by setting its contents according to parameters */

address_packet(pack, from, to)
Packet *pack;
int from, to;
{
D 3
  pack->from = from;
E 3
I 3
  SetPacketFrom(pack,from);
  SetPacketTo(pack,to);
E 3
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
  
D 3
  if (DEBUG) printf("Tell old players we are joining the game\n");
E 3
I 3
  DPRINTF("Tell old players we are joining the game\n");
E 3

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

I 3
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

E 3
notify_all_of_map_change(roomnum, x, y, which, old, new, recobjptr)
int roomnum, x, y, which, old, new;
RecordedObj *recobjptr;
{
  Packet pack;
  
  /* prepare the packet */
D 3
  if (DEBUG) printf("Telling all of map change\n");
  pack.type = CHANGE_MAP;
  pack.info.map.room = roomnum;
  pack.info.map.x = x;
  pack.info.map.y = y;
  pack.info.map.which = which;
  pack.info.map.old = (uc) old;
  pack.info.map.new = (uc) new;
E 3
I 3
  DPRINTF("Telling all of map change\n");
  PacketInit(&pack, CHANGE_MAP);
  Add16(&pack, roomnum);
  Add8(&pack, x);
  Add8(&pack, y);
  Add16(&pack, which);
  Add16(&pack, old);
  Add16(&pack, new);
E 3
D 4
  if (recobjptr)
D 3
    bcopy(recobjptr, &(pack.info.map.recobj), sizeof(RecordedObj));
E 3
I 3
    AddArbitrary(&pack, &(pack.info.map.recobj), sizeof(RecordedObj));
E 3

E 4
I 4
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
E 4
  /* send the packet to players and driver, anonymously */
  address_packet(&pack, -1, TO_ALL);
  send_to_important(&pack, TRUE);

  /* order the object's replacement for ourselves as well */
D 5
  change_map_square(roomnum, x, y, which, (uc)old, (uc)new, recobjptr);
E 5
I 5
  change_own_map_square(roomnum, x, y, which, (uc)old, (uc)new, recobjptr);
E 5

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
D 3
  prepare_my_location(pnum, &outer_pack);
E 3
I 3
  prepare_far_my_location(pnum, &outer_pack);
E 3
  address_packet(&inner_pack, gameperson[pnum]->id, TO_ALL);
  address_packet(&outer_pack, gameperson[pnum]->id, TO_ALL);
D 3
  /* block out detailed location for far away folks */
  outer_pack.info.loc.x = -1;
  outer_pack.info.loc.y = -1;
E 3

  send_in_and_out_of_room(gameperson[pnum]->room, &inner_pack,
			  &outer_pack, TRUE);
}




D 3
/* let everyone this persons room know where he is (changing spots) */
E 3
I 3
/* let everyone in this persons room know where he is (changing spots) */
E 3

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



D 2
/* notify the given person id of damage he has done */
E 2
I 2
/* notify the given person id of some information */
E 2

D 2
notify_of_damage(fromid, to_id, msg)
E 2
I 2
notify_of_report(fromid, to_id, msg)
E 2
int fromid, to_id;
char *msg;
{
D 2
  Packet dmgpack;
E 2
I 2
  Packet rptpack;
E 2

  /* prepare and send the packet */
D 2
  dmgpack.type = DAMAGE_REPORT;
  strcpy(dmgpack.info.msg, msg);
  address_packet(&dmgpack, fromid, to_id);
  send_to_id(to_id, &dmgpack, FALSE);
E 2
I 2
D 3
  rptpack.type = REPORT;
  strcpy(rptpack.info.msg, msg);
E 3
I 3

  PacketInit(&rptpack, REPORT);
  AddString(&rptpack, msg);
E 3
  address_packet(&rptpack, fromid, to_id);
  send_to_id(to_id, &rptpack, FALSE);
E 2
}



/* Tell person about an experience point bonus and give it to him */

I 3
/*
 * ADD_EXPERIENCE
 *
 * CARD16	to
 * CARD16	amount
 * STRING	message
 */

E 3
notify_of_experience(fromid, to_id, amount, msg)
int fromid, to_id, amount;
char *msg;
{
  Packet pack;

  /* prepare and send the packet */
D 3
  pack.type = ADD_EXPERIENCE;
  pack.info.add.id = to_id;
  pack.info.add.amount = amount;
  strcpy(pack.info.add.msg, msg);
E 3
I 3
  PacketInit(&pack,ADD_EXPERIENCE);
  Add16(&pack,to_id);
  Add16(&pack,amount);
  AddString(&pack,msg);
E 3
  address_packet(&pack, fromid, to_id);
  send_to_id(to_id, &pack, FALSE);
}

D 3


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
  send_to_players(&maskpack, TRUE);

  /* prepare bitmap packet and send it */
  bitpack.type = MY_BITMAP;
  bitmapcpy(bitpack.info.bit, bitmap);
  address_packet(&bitpack, gameperson[pnum]->id, TO_ALL);
  send_to_players(&bitpack, TRUE);

  /* copy the new bitmaps into my gameperson structure */
  bitmapcpy(gameperson[pnum]->bitmap, bitmap);
  bitmapcpy(gameperson[pnum]->mask, mask);

  /* update my own pixmap of the person if neccessary */
  if (has_display) update_pixmaps(pnum);
}




E 3
/* let everyone know that the game is over, winner is the id number of
   person that finished off the game */

notify_of_game_over(winner, team)
int winner, team;
{
  Packet pack;

  /* don't do this if the game is already ending */
  if (IS_GAME_ENDING) return;

  /* prepare the packet */
D 3
  pack.type = GAME_OVER;
  pack.info.game.winner = winner;
  pack.info.game.team = team;
E 3
I 3
  PacketInit(&pack,GAME_OVER);
  Add16(&pack,winner);
  Add16(&pack,team);
E 3

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

I 3
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

E 3
notify_of_stats(pnum)
int pnum;
{
  Packet pack;

  /* check that this is one of my own persons */
  if (pnum >= num_persons)
    printf("Warning: sending SAVE_STATS about person %d\n", pnum);

  /* prepare packet and send to driver and players */
D 3
  pack.type = SAVE_STATS;
  pack.info.stat.id = gameperson[pnum]->id;
  pack.info.stat.ExpPts = person[pnum]->experience;
  strcpy(pack.info.stat.rank, gameperson[pnum]->rank);
  pack.info.stat.level = gameperson[pnum]->level;
  pack.info.stat.kills = gameperson[pnum]->kills;
  pack.info.stat.losses = gameperson[pnum]->losses;
  pack.info.stat.games = gameperson[pnum]->games;
E 3
I 3
  PacketInit(&pack, SAVE_STATS);
I 5

D 7
  printf("Sending STATS id # of %d.\n", gameperson[pnum]->id); /* PLUGH */
E 7
I 7
  DPRINTF("Sending STATS id # of %d.\n", gameperson[pnum]->id);
E 7
E 5

  Add16(&pack, gameperson[pnum]->id);
  Add32(&pack, person[pnum]->experience);
  AddString(&pack, gameperson[pnum]->rank);
  Add16(&pack, gameperson[pnum]->level);
  Add32(&pack, gameperson[pnum]->kills);
  Add32(&pack, gameperson[pnum]->losses);
  Add32(&pack, gameperson[pnum]->games);
E 3
  address_packet(&pack, gameperson[pnum]->id, TO_ALL);
  send_to_important(&pack, FALSE);
}


D 3

E 3
/* tell someone they killed me */

I 3
/*
 * YOU_KILLED_ME
 */ 

E 3
notify_of_kill(victim_id, killer_id)
{
  Packet pack;

  /* check that this is one of my own persons */
  if (person_array(victim_id) >= num_persons)
    printf("Warning: sending YOU_KILLED_ME about someone else");

  /* prepare packet and send it to person */
D 3
  pack.type = YOU_KILLED_ME;
E 3
I 3
  PacketInit(&pack, YOU_KILLED_ME);
E 3
  address_packet(&pack, victim_id, killer_id);
  send_to_id(killer_id, &pack, FALSE);
}



/* persons leave game by sending everyone a LEAVING_GAME packet */

I 3
/*
 * LEAVING_GAME
 */

E 3
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
D 3
  send_to_players(&msgpack, FALSE);
E 3

  /* prepare the LEAVING_GAME packet */
D 3
  leavepack.type = LEAVING_GAME;
  leavepack.from = gameperson[num]->id;
  leavepack.to = TO_ALL;
E 3
I 3
  PacketInit(&leavepack, LEAVING_GAME);
  address_packet(&leavepack, gameperson[num]->id, TO_ALL);
E 3

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
E 1
