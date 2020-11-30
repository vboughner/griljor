h29277
s 00000/00000/00821
d D 1.9 92/08/07 01:05:00 vbo 9 8
c source copied to a separate tree for work on new map and object format
e
s 00013/00012/00808
d D 1.8 92/01/22 21:32:56 labc-4lc 8 7
c fixed bugs in map updates and missile noticing (minor changes)
e
s 00003/00003/00817
d D 1.7 92/01/20 17:41:45 labc-4lc 7 6
c fixed a couple places where GetTo() FLAG_LISTED were needed
e
s 00129/00055/00691
d D 1.6 92/01/20 16:30:57 labc-4lc 6 5
c blojo finished changes necesssary for network packets
e
s 00028/00019/00718
d D 1.5 91/12/15 23:06:46 labc-4lc 5 4
c Fixing datagram incompatibilities.   -bloo
e
s 00147/00097/00590
d D 1.4 91/12/15 15:09:08 labc-4lc 4 3
c blojo's datagram mungification, attempt #1
e
s 00001/00001/00686
d D 1.3 91/12/08 03:33:56 labc-4lc 3 2
c implemented usage messages for objects
e
s 00003/00004/00684
d D 1.2 91/12/07 21:41:01 labc-4lc 2 1
c split socket.c into three files and change DAMAGE_REPORT to REPORT
e
s 00688/00000/00000
d D 1.1 91/12/07 20:57:20 labc-4lc 1 0
c date and time created 91/12/07 20:57:20 by labc-4lc
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

/* routines for noticing packet that come in as datagrams and acting
   on their contents in an intelligent manner */

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

I 4
D 5
#define Get8(p,b)	((junkCARD8 = (p)->info[(p)->len]), (p)->len += 4, junkCARD8)
#define Get16(p,s)	((junkCARD16 = ntohs(*((CARD16 *)((p) + len)))), (p)->len += 4, junkCARD16)
#define Get32(p,l)	((junkCARD32 = ntohl(*((CARD32 *)((p) + len)))), p->len += 4, junkCARD32)
char   *GetString(p,s)
E 5
I 5
D 6
#define Get8(p)		((junkCARD8 = (p)->info[(p)->len]), (p)->len += 4, junkCARD8)
#define Get16(p)	((junkCARD16 = ntohs(*((CARD16 *)((p) + (p)->len)))), (p)->len += 4, junkCARD16)
#define Get32(p)	((junkCARD32 = ntohl(*((CARD32 *)((p) + (p)->len)))), (p)->len += 4, junkCARD32)
E 6
I 6
#define Get8(p)		((p)->info[(p)->len += 4])
#define Get16(p)	ntohs(*((CARD16 *)((p)->info + ((p)->len += 4))))
#define Get32(p)	ntohl(*((CARD32 *)((p)->info + ((p)->len += 4))))
E 6
char   *GetString(p)
E 5
Packet *p;
D 5
char *s;
E 5
{
D 5
	char *ret = (char *)(p + len + 4);
	int l = ntohs(*((CARD16 *)(p + len)));
E 5
I 5
D 6
	char *ret = (char *)(p + p->len + 4);
	int l = ntohs(*((CARD16 *)(p + p->len)));
E 6
I 6
	char *ret;
	int l;
	ret = (char *)(p->info + p->len + 4);
	l = strlen(ret) + 1;
E 6
E 5
	if (l & 3) l += 4 - (l & 3);
	p->len += l;
	return ret;
}
D 6
#define GetArbitrary(s,d,l) bcopy(s, d, l)
E 6
E 4

I 4
D 5
#define GetType(p)	((CARD8)((p)[TYPE_OFFSET]))
#define GetFrom(p)	(ntohs(*((CARD16 *)((p) + FROM_OFFSET))))
#define GetTo(p)	(ntohs(*((CARD16 *)((p) + TO_OFFSET))))
E 5
I 5
D 6
#define GetType(p)	((CARD8)((p)->info[TYPE_OFFSET]))
#define GetFrom(p)	(ntohs(*((CARD16 *)((p)->info + FROM_OFFSET))))
#define GetTo(p)	(ntohs(*((CARD16 *)((p)->info + TO_OFFSET))))
E 6
I 6
GetArbitrary(s,d,l)
Packet *s;
char *d;
int l;
{
	bcopy(s->info + s->len + 4, d, l);
	if (l & 3) l += 4 - (l & 3);
	s->len += l;
}
	
E 6
E 5

I 6
/* #define GetType(p)		((CARD8)((p)->info[TYPE_OFFSET])) */
/* #define GetTo(p)		(ntohs(*((CARD16 *)((p)->info + TO_OFFSET)))) */
/* Defined in socket.h for some ungodly reason.		  */
#define GetFrom(p)		(ntohs(*((CARD16 *)((p)->info + FROM_OFFSET))))
#define GetPassword(p)		(ntohl(*((CARD16 *)((p)->info + PASSWORD_OFFSET))))


E 6
E 4
/* =================== D A T A G R A M  socket stuff =================== */

I 6
void slimy_debug_packet(packet)
Packet *packet;
{
  int to = GetTo(packet);
  int p = GetPassword(packet);
  int f = GetFrom(packet);
  int l = packet->len;
  int t = GetType(packet);

  printf("SENDING PACKET of TYPE %d, to %d from %d, length %d, passwd %d.\n",
	 t, to, f, l, p);
}

void slimy_debug_incoming_packet(packet)
Packet *packet;
{
  int to = GetTo(packet);
  int p = GetPassword(packet);
  int f = GetFrom(packet);
  int l = packet->len;
  int t = GetType(packet);

  printf("RECEIVING PACKET of TYPE %d, to %d from %d, length %d, passwd %d.\n",
	 t, to, f, l, p);
}
E 6
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
D 4
    if (am_monster) {
	/* monster_peek_packet returns TRUE if the packet should be passed
	   on for regular handling */
	if (monster_peek_packet(&packet)) handle_boss_packet(&packet, &from);
    }
    else handle_boss_packet(&packet, &from);
E 4
I 4
    handle_boss_packet(&packet, &from);
E 4

    /* read the next packet to handle */
    got_packet = recvPacket(&packet, &from, &timeout);
  }

}
I 4
D 6
/*
 * NOTE
 * 
 * The following code was taken out of the above routine because I don't
 * know what the hell to do w/ it.
 *
 *    -blojo
 */
E 6
E 4

I 4
D 6
/*    if (am_monster) { */
	/* monster_peek_packet returns TRUE if the packet should be passed
	   on for regular handling */
/*	if (monster_peek_packet(&packet)) handle_boss_packet(&packet, &from);
 *   }
 *   else 
 */
E 4


I 4

E 6
E 4
/* Get a packet from our socket if there is anything at all to get, return
   TRUE if something was received, otherwise return FALSE.  You supply the
   amount of time to wait with a timeval structure pointer. */

int recvPacket(packet, from, wait)
D 4
BossPacket	*packet;
E 4
I 4
Packet		*packet;
E 4
Sockaddr	*from;
struct timeval	*wait;
{
  int		ret, fromlen;

  /* if there is nothing to read then return FALSE to calling procedure */
  if (readable_on_socket(thesocket, 0) == FALSE) return FALSE;

  /* specify length of a network address */
  fromlen = sizeof(Sockaddr);

  /* pull in the packet from the socket */
D 4
  ret = recvfrom(thesocket, packet, sizeof(BossPacket), 0, from, &fromlen);
E 4
I 4
  ret = recvfrom(thesocket, packet->info, MAXPACKETLEN, 0, from, &fromlen);
E 4

  /* on any kind of error but an interrupt error, crash and burn */
  if (ret < 0 && errno != EINTR) Berror("reception error while reading");
  else if (ret <= 0) return FALSE;	/* if nothing was read then return */

  /* let's check the packet's password */
D 4
  if (packet->password != BOSS_PASSWORD) {
E 4
I 4
  if (GetPassword(packet) != BOSS_PASSWORD) {
E 4
      if (BOSS_DEBUG)
        printf("Socket Boss: warning: packet with bad password received\n");
      return FALSE;
  }
D 4

E 4
I 4
D 6
  packet->len = RANDOM_DATA_OFFSET;
E 6
I 6
  packet->len = RANDOM_DATA_OFFSET - 4;
E 6
E 4
  /* everthing checks out, we can keep it, set receipt flags and return TRUE */
  IO_RECEIVED();
  return TRUE;
}


D 4


E 4
/* Add incoming packets to the correct person's queue or handle them ourself
   when we can do it without needing to inform any persons */

handle_boss_packet(packet, from)
D 4
BossPacket *packet;
E 4
I 4
Packet *packet;
E 4
Sockaddr   *from;
{
  int fromelement;
  
D 4
  if (DEBUG) printf("Received a packet of type %d\n", packet->type);
E 4
I 4
  DPRINTF("Received a packet of type %d\n", GetType(packet));
I 6
D 8
  slimy_debug_incoming_packet(packet);
E 8
I 8
  if (DEBUG) slimy_debug_incoming_packet(packet);
E 8
E 6
E 4

  /* if the packet is neither addressed to one of my persons, nor is
     addressed TO_ALL, then I should ignore it, it's probably spurious */
D 5
  if (am_driver && (packet->to != TO_GM && packet->to != TO_ALL)) return;
E 5
I 5
  if (am_driver && (GetTo(packet) != TO_GM && GetTo(packet) != TO_ALL)) return;
E 5
  else if (!am_driver)
D 4
         if (packet->to != TO_ALL && !one_of_me(packet->to)) return;
E 4
I 4
         if (GetTo(packet) != TO_ALL && !one_of_me(GetTo(packet))) return;
E 4

  /* if the packet is from an unknown person and is not MY_INFO, MY_BITMAP,
     or MY_MASK then ignore it and send REPEAT_INFO (unless it was a
     LEAVING_GAME or CHANGE_MAP packet) */

D 4
  fromelement = person_array(packet->from);	/* look in gameperson array */
  if (fromelement == -1 && (packet->from != FROM_GM))
E 4
I 4
  fromelement = person_array(GetFrom(packet));	/* look in gameperson array */
  if (fromelement == -1 && (GetFrom(packet) != FROM_GM))
E 4
    /* packet is from an unknown person */
D 4
    switch (packet->type) {
E 4
I 4
    switch (GetType(packet)) {
E 4
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
D 4
    switch (packet->type) {
E 4
I 4
    switch (GetType(packet)) {
E 4
	/* new player info packets */
	case MY_INFO:
	case MY_BITMAP:
	case MY_MASK:		notice_added_info(packet);
				break;
	/* packets of knowledge for all persons */
D 6
	case WEAPON_FIRED:	add_missile(packet);
E 6
I 6
	case WEAPON_FIRED:	notice_missile(packet);
E 6
				break;
	case MULTI_FIRE:	notice_multi_fire(packet);
				break;
	case MY_LOCATION:	notice_person_location(packet);
				break;
	case CHANGE_MAP:	notice_map_change(packet);
				break;
D 2
	case DAMAGE_REPORT:	notice_damage_report(packet);
E 2
I 2
	case REPORT:		notice_report(packet);
E 2
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
D 4
				else try_to_log_packet(packet);
E 4
I 4
				else try_to_log_message(packet);
E 4
				break;
	default:		break;
  }
}



/* ======== I N D I V I D U A L  P A C K E T  H A N D L I N G ======== */

/* when a person wants to leave the game, see that his records are deleted */

notice_person_leaving(pack)
Packet *pack;
{
  int pnum;
  
  /* check that it is a valid person */
D 4
  if (DEBUG) printf("Person %d leaving the game\n", pack->from);
  pnum = person_array(pack->from);
E 4
I 4
  DPRINTF("Person %d leaving the game\n", GetFrom(pack));
  pnum = person_array(GetFrom(pack));
E 4
  if (pnum == -1) {
D 4
    if (DEBUG) printf("Warning: non-existant person\n");
E 4
I 4
    DPRINTF("Warning: non-existant person\n");
E 4
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
D 4
  if (DEBUG) printf("New person %d in the game\n", pack->from);
E 4
I 4
D 6
  DPRINTF("New person %d in the game\n", GetFrom(pack));
E 6
I 6
D 8
  printf("New person %d in the game\n", GetFrom(pack)); /* PLUGH */
E 8
I 8
  DPRINTF("New person %d in the game\n", GetFrom(pack));
E 8
E 6
E 4
  num = persons_in_game;
  initialize_game_person(num);
  persons_in_game++;

  /* put in the persons id number and socket address */
D 4
  gameperson[num]->id = pack->from;
E 4
I 4
  gameperson[num]->id = GetFrom(pack);
E 4
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
D 4
      send_to_id(pack->from, &infopack, TRUE);
E 4
I 4
      send_to_id(GetFrom(pack), &infopack, TRUE);
E 4
    }
  }
}




/* handle MY_INFO, MY_BITMAP, and MY_MASK packets, adding info to array */

notice_added_info(pack)
Packet *pack;
{
  int pnum;	/* person array element number */
I 4
  unsigned char junk;
E 4

  /* find out array index */
D 4
  if (DEBUG) printf("Information on person %d\n", pack->from);
  pnum = person_array(pack->from);
E 4
I 4
D 5
  DPRINTF("Information on person %d\n", pack->from);
E 5
I 5
  DPRINTF("Information on person %d\n", GetFrom(pack));
E 5
  pnum = person_array(GetFrom(pack));
E 4

  /* check for invalid array element */
  if (pnum == -1) {
D 4
    if (DEBUG) printf("unidentified person %d\n", pack->from);
E 4
I 4
D 5
    DPRINTF("unidentified person %d\n", pack->from);
E 5
I 5
    DPRINTF("unidentified person %d\n", GetFrom(pack));
E 5
E 4
    return;
  }

  /* figure out what kind of info it is and add it */
D 4
  switch (pack->type) {
E 4
I 4
  switch (GetType(pack)) {
E 4
  	case MY_INFO:	/* fill in gameperson record with info given */
D 4
			gameperson[pnum]->parent = pack->info.data.parent;
			strcpy(gameperson[pnum]->name, pack->info.data.name);
			strcpy(gameperson[pnum]->rank, pack->info.data.rank);
			strcpy(gameperson[pnum]->login, pack->info.data.login);
			strcpy(gameperson[pnum]->host, pack->info.data.host);
			gameperson[pnum]->level = pack->info.data.level;
			gameperson[pnum]->team = pack->info.data.team;
			gameperson[pnum]->deity = pack->info.data.deity;
			gameperson[pnum]->listed = pack->info.data.listed;
E 4
I 4
			gameperson[pnum]->parent = Get16(pack);
			strcpy(gameperson[pnum]->name, GetString(pack));
			strcpy(gameperson[pnum]->login, GetString(pack));
			strcpy(gameperson[pnum]->host, GetString(pack));
			strcpy(gameperson[pnum]->rank, GetString(pack));
			gameperson[pnum]->level = Get16(pack);
			gameperson[pnum]->team = Get8(pack);
I 6
D 8
#define uu gameperson[pnum]
printf("GOT DOODER %d, %s, %s, %s, %s, %d, %d\n",
       uu->parent, uu->name, uu->login, uu->host, uu->rank,
       uu->level, uu->team);
E 8
E 6
			junk = Get8(pack);
			gameperson[pnum]->deity = junk & FLAG_DEITY;
			gameperson[pnum]->listed = junk & FLAG_LISTED;
E 4
			/* let possible new player know where we are */
			if (gameperson[pnum]->id == gameperson[pnum]->parent &&
			    !am_driver) notify_person_of_my_locations(pnum);
I 8
#define uu gameperson[pnum]
			DPRINTF("GOT DOODER %d, %s, %s, %s, %s, %d, %d\n",
			       uu->parent, uu->name, uu->login, uu->host,
			       uu->rank, uu->level, uu->team);
E 8
			break;
	case MY_BITMAP:	if (!am_driver) {
D 4
			  bitmapcpy(gameperson[pnum]->bitmap, pack->info.bit);
E 4
I 4
			  GetArbitrary(pack, gameperson[pnum]->bitmap,
				       BITMAP_ARRAY_SIZE);
E 4
			  /* update pixmap when bitmap is received */
			  if (has_display) update_pixmaps(pnum);
			}
			break;
	case MY_MASK:	if (!am_driver) {
D 4
			  bitmapcpy(gameperson[pnum]->mask, pack->info.bit);
E 4
I 4
			  GetArbitrary(pack, gameperson[pnum]->mask,
				       BITMAP_ARRAY_SIZE);
E 4
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
D 4

E 4
I 4
  unsigned int x, y, room;
E 4
  /* get array element number */
D 4
  if (DEBUG) printf("Handling location data from person %d\n", pack->from);
  pnum = person_array(pack->from);
E 4
I 4
D 5
  DPRINTF("Handling location data from person %d\n", pack->from);
E 5
I 5
  DPRINTF("Handling location data from person %d\n", GetFrom(pack));
E 5
  pnum = person_array(GetFrom(pack));
E 4

  /* update person array as long as this isn't one of my own people */
  if (pnum >= num_persons) {
D 4
    gameperson[pnum]->appearance = pack->info.loc.appear;
    move_person(pnum, pack->info.loc.room, pack->info.loc.x,
		pack->info.loc.y, TRUE);
E 4
I 4
    x = Get8(pack);
    y = Get8(pack);
    room = Get16(pack);
    gameperson[pnum]->appearance = Get16(pack);
    move_person(pnum, room, x, y, TRUE);
E 4
  }
}



/* notice a multi-missile firing packet.  Separate it into missiles
   and add each of those to my regular missile list */

D 6
notice_multi_fire(pack)
Packet *pack;
E 6
I 6
/* MULTI_FIRE */

notice_multi_fire(p)
Packet *p;
E 6
{
D 6
  add_multi_missiles(pack);
E 6
I 6
  MultiPack m;
  int len;

I 8
  m.miss.type = Get16(p);
E 8
  m.miss.owner = Get16(p);
  m.miss.hurts_owner = Get16(p);
  m.miss.room = Get16(p);
  m.include_start = Get16(p);
  m.number = Get16(p);

  GetArbitrary(p, m.x1, m.number);
  GetArbitrary(p, m.x2, m.number);
  GetArbitrary(p, m.y1, m.number);
  GetArbitrary(p, m.y2, m.number);

D 8
  add_multi_missiles(m);
E 8
I 8
  add_multi_missiles(&m);
E 8
E 6
}

I 6
D 8
/* #ifndef DRIVER */
E 8
I 8

E 8
extern char local_x[], local_y[];
E 6

I 6
notice_missile(p)
Packet *p;
{
  Missile *m = (Missile *)malloc(sizeof(Missile));
  if (m == NULL) return;
  m->owner = Get16(p);
  m->type = Get16(p);
  m->hurts_owner = Get8(p);
  m->wait = Get16(p);
  m->room = Get16(p);
  m->heading = Get16(p);
  m->range = Get16(p);
E 6
D 8

E 8
I 6
  GetArbitrary(p, local_x, m->range);
  GetArbitrary(p, local_y, m->range);
I 8
  m->direction = Get8(p);
E 8

  add_missile(m);
}
D 8
/* #endif */ /* DRIVER */
E 8

I 8

E 8
E 6
D 2
/* notice a damage report message, if we have a display, then put the
E 2
I 2
/* notice a report message, if we have a display, then put the
E 2
D 4
   message up in the error window.  Only messages for the current person
E 4
I 4
   Packet up in the error window.  Only messages for the current person
E 4
   are displayed (probably person 0) */

D 2
notice_damage_report(pack)
E 2
I 2
notice_report(pack)
E 2
Packet *pack;
{
D 2
  /* MAY CHANGE: person zero considered current person */
E 2
D 3
  if (person_array(pack->to) == 0 && has_display)
E 3
I 3
D 4
  if ((pack->to == TO_ALL || person_array(pack->to) == 0) && has_display)
E 3
    player_error_out_message(pack->info.msg);
E 4
I 4
D 7
  if ((pack->to == TO_ALL || person_array(GetTo(pack)) == 0) && has_display)
E 7
I 7
  if ((GetTo(pack) == TO_ALL || person_array(GetTo(pack)) == 0) && has_display)
E 7
    player_error_out_message(GetString(pack));
E 4
}


D 4
/* notice when someone tells me I killed them, give my self some credit */
E 4
I 4
/* notice when someone tells me I killed them, give myself some credit */
E 4

notice_kill(pack)
Packet *pack;
{
D 4
  int pnum = person_array(pack->to);
E 4
I 4
  int pnum = person_array(GetTo(pack));
E 4

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
D 4
  int pnum = person_array(pack->info.add.id);
  
  alter_experience(pnum, pack->info.add.amount, pack->info.add.msg);
E 4
I 4
  int amount;
  int pnum = person_array(Get16(pack));
  amount = Get16(pack);
  alter_experience(pnum, amount, GetString(pack));
E 4
}



/* handle a CHANGE_MAP packet, changing the map if possible, and being
   careful not to be fooled by duplicate packets */

notice_map_change(pack)
I 4

E 4
Packet *pack;
{
D 4
  if (DEBUG) printf("Handling map change info from %d\n", pack->from);

E 4
I 4
  int room, x, y, which, old, new;
D 5
  DPRINTF("Handling map change info from %d\n", pack->from);
E 5
I 5
  DPRINTF("Handling map change info from %d\n", GetFrom(pack));
E 5
E 4
  /* call for a change in the map */
D 4
  change_map_square(pack->info.map.room, pack->info.map.x, pack->info.map.y,
		    pack->info.map.which, (uc)pack->info.map.old,
		    (uc)pack->info.map.new, &(pack->info.map.recobj));
E 4
I 4
  room = Get16(pack);
  x = Get8(pack);
  y = Get8(pack);
  which = Get16(pack);
  old = Get16(pack);
  new = Get16(pack);
  change_map_square(room, x, y, which, old, new, pack);
E 4
}



/* change the contents of a square according to the data given.  This is
   the routine called from the player and driver process when a CHANGE_MAP
   packet is received */

I 6
/* front end to change a pack into a recobjptr so we can call
 * change_own_map_square.
 */
E 6
D 4
change_map_square(roomnum, x, y, which, old, new, recobjptr)
E 4
I 4
change_map_square(roomnum, x, y, which, old, new, pack)
E 4
int roomnum, x, y, which, old, new;
D 4
RecordedObj *recobjptr;
E 4
I 4
D 6
/*RecordedObj *recobjptr;*/
E 6
Packet *pack;
E 4
{
I 6
    RecordedObj obj, *pobj;

    if (info[(uc)new]->recorded) {
      obj.x = Get8(pack);
      obj.y = Get8(pack);
      obj.objtype = Get8(pack);
      obj.detail = Get16(pack);
      obj.infox = Get16(pack);
      obj.infoy = Get16(pack);
      obj.zinger = Get16(pack);
      obj.extra[0] = Get16(pack);
      obj.extra[1] = Get16(pack);
      obj.extra[2] = Get16(pack);
      pobj = &obj;
    } else pobj = NULL;
D 8
    change_own_map_square(roomnum, x, y, which, old, new, pobj);
E 8
I 8
    change_own_map_square(roomnum, x, y, which, (uc)old, (uc)new, pobj);
E 8
}

change_own_map_square(roomnum, x, y, which, old, new, obj)
int roomnum, x, y, which, old, new;
RecordedObj *obj;
{
E 6
  MemObj *ptr;
  
  /* check that these are valid values */
D 4
  if (DEBUG) printf("Changing map in room %d at %d %d\n", roomnum, x, y);
E 4
I 4
  DPRINTF("Changing map in room %d at %d %d\n", roomnum, x, y);
E 4
  if (roomnum < 0 || roomnum >= mapstats.rooms || x<0 || y<0 ||
      x >= ROOM_WIDTH || y >= ROOM_HEIGHT || which<0 ||
      which > ROOM_DEPTH || new<0 || new >= objects) return;

  /* if object is a recorded object then treat specially */
  if (which == ROOM_DEPTH) {
    /* check to see if we are required to replace with a recorded object
       but no recorded object record is given */
D 5
    if (info[(uc)new]->recorded && recobjptr == NULL) return;
E 5
I 5
D 6
/*    if (info[(uc)new]->recorded && recobjptr == NULL) return;*/
E 6
I 6
    if (info[(uc)new]->recorded && obj == NULL) return;
E 6
E 5

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
D 5
      bcopy(recobjptr, &(ptr->obj), sizeof(RecordedObj));
E 5
I 5
D 6
      ptr->obj.x = Get8(pack);
      ptr->obj.y = Get8(pack);
      ptr->obj.objtype = Get8(pack);
      ptr->obj.detail = Get16(pack);
      ptr->obj.infox = Get16(pack);
      ptr->obj.infoy = Get16(pack);
      ptr->obj.zinger = Get16(pack);
      ptr->obj.extra[0] = Get16(pack);
      ptr->obj.extra[1] = Get16(pack);
      ptr->obj.extra[2] = Get16(pack);
/*      bcopy(recobjptr, &(ptr->obj), sizeof(RecordedObj));*/
E 6
I 6
      bcopy(obj, &(ptr->obj), sizeof(RecordedObj));
E 6
E 5
      add_obj_to_room(roomnum, ptr);
    }
  }
D 6

I 4
/*
 * THE ABOVE STUFF WITH RECOBJPTR IS REALLY FUCKED.
 * FIGURE OUT HOW TO DO IT RIGHT WHEN YOU'RE AWAKE.
 */

E 6
E 4
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
  if (has_display) ChangedRoomSquare(roomnum,x,y);

  /* tell the important object accounting system about this */
  flag_account_map_change(roomnum, x, y, old, new);
}



/* notice a message that the game is over */

notice_game_over(pack)
Packet *pack;
{
  char s[80];
  int quit_time;
D 4

E 4
I 4
  int winner, team;
E 4
  /* set the game end countdown timer (if game isn't already ending)
     and notify player of impending end of game */
  if (!IS_GAME_ENDING) {
    end_of_world = gametime + GAME_OVER_WAIT;

    /* figure out how long to game end (in seconds) */
    quit_time = (GAME_OVER_WAIT / CLICKS_PER_SEC);

I 4
    winner = Get16(pack);
    team = Get16(pack);
E 4
    /* print messages about game end */
    if (has_display) {
      player_error_out_message("********* Game is over *****************************");
      player_error_out_message("*** Congratulations !!!");
D 4
      if (person_array(pack->info.game.winner) != -1) {
E 4
I 4
      if (person_array(winner) != -1) {
E 4
        sprintf(s, "*** Winning  player: %s", 
D 4
		gameperson[person_array(pack->info.game.winner)]->name);
E 4
I 4
		gameperson[person_array(winner)]->name);
E 4
        player_error_out_message(s);
	/* print out winning team name */
D 4
	if (pack->info.game.team > 0 &&
	    pack->info.game.team <= mapstats.teams_supported) {
E 4
I 4
	if (team > 0 && team <= mapstats.teams_supported) {
E 4
          sprintf(s, "*** Victorious team: %s", 
D 4
		  mapstats.team_name[pack->info.game.team - 1]);
E 4
I 4
		  mapstats.team_name[team - 1]);
E 4
	  player_error_out_message(s);
	}
        sprintf(s, "*** %d seconds until full game shutdown", quit_time);
        player_error_out_message(s);
      }
      player_error_out_message("****************************************************");
    }

    if (!am_driver) {
D 4
      show_player_ending_game(pack->info.game.team);
      end_game_experience(pack->info.game.team, pack->info.game.winner);
E 4
I 4
      show_player_ending_game(team);
      end_game_experience(team, winner);
E 4
    }
  }
}



/* notice a player's request to save his stats before he leaves the game */

notice_save_stats(pack)
Packet *pack;
{
  int pnum;
  PersonRec *p;
I 4
  int experience;
I 6
  int junk;
E 6
E 4

  /* check for validity of person id number */
D 4
  if (DEBUG) printf("Noticing save stats request from %d\n", pack->from);
  pnum = person_array(pack->info.stat.id);
E 4
I 4
D 5
  DPRINTF("Noticing save stats request from %d\n", pack->from);
E 5
I 5
  DPRINTF("Noticing save stats request from %d\n", GetFrom(pack));
E 5
D 6
  pnum = person_array(Get16(pack));
E 6
I 6
  pnum = person_array(junk = Get16(pack));
D 8
  printf("JUNK is %d.  pnum is %d.\n", junk, pnum);
E 8
I 8
  DPRINTF("JUNK is %d.  pnum is %d.\n", junk, pnum);
E 8

E 6
E 4
  if (pnum<0 || pnum>persons_in_game) {
    printf("Warning: unknow person's stats\n");
    return;
  }

  /* check the person's records, update ours */
D 4
  gameperson[pnum]->kills = pack->info.stat.kills;
  gameperson[pnum]->losses = pack->info.stat.losses;
  gameperson[pnum]->games = pack->info.stat.games;
  strcpy(gameperson[pnum]->rank, pack->info.stat.rank);
  gameperson[pnum]->level = pack->info.stat.level;
E 4
I 4
  experience = Get32(pack);
  strcpy(gameperson[pnum]->rank, GetString(pack));
  gameperson[pnum]->level = Get16(pack);
  gameperson[pnum]->kills = Get32(pack);
  gameperson[pnum]->losses = Get32(pack);
  gameperson[pnum]->games = Get32(pack);
E 4

  if (am_driver) {
    /* get the person's record out of password file if there is one */
    p = get_player(gameperson[pnum]->name);

    /* if there wasn't one then don't try to modify anything, he must
       be one of those unrecorded monsters */
    if (!p) return;

    /* otherwise modify the record and save it */
D 4
    p->ExpPts = pack->info.stat.ExpPts;
    strcpy(p->rank, pack->info.stat.rank);
E 4
I 4
    p->ExpPts = experience;
    strcpy(p->rank, gameperson[pnum]->rank);
E 4
    strcpy(p->login, gameperson[pnum]->login);
    strcpy(p->host, gameperson[pnum]->host);
D 4
    p->kills = pack->info.stat.kills;
    p->losses = pack->info.stat.losses;
    p->games = pack->info.stat.games;
E 4
I 4
    p->kills = gameperson[pnum]->kills;
    p->losses = gameperson[pnum]->losses;
    p->games = gameperson[pnum]->games;
E 4
    modify_player(p);
    free(p);
D 4
    if (DEBUG) printf("Saved stats of person %d\n", pack->info.stat.id);
E 4
I 4
    DPRINTF("Saved stats of person %d\n", pnum);
E 4
  }
}
 



/* notice a packet that is meant for person queues */

notice_queued_packet(pack)
Packet *pack;
{
  register int i;
  
D 7
  if (pack->to == TO_ALL)
E 7
I 7
  if (GetTo(pack) == TO_ALL)
E 7
    for (i=0; i<num_persons; i++) add_packet_to_queue(i, pack);
  else
D 7
    add_packet_to_queue(person_array(pack->to), pack);
E 7
I 7
    add_packet_to_queue(person_array(GetTo(pack)), pack);
E 7
}



/* add a queued packet to a person's queue */

add_packet_to_queue(pnum, pack)
int pnum;
Packet *pack;
{
  QPacket *new;
  
  /* check person number */
  if (pnum < 0 || pnum >= num_persons) {
D 4
    if (DEBUG) printf("Warning: bad person number for queuing packet\n");
E 4
I 4
    DPRINTF("Warning: bad person number for queuing packet\n");
E 4
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



/* try to log a packet to the disk log file, only the driver should ever
   try to do this.  A reply will to sent to the sender.  The reply will
   be generated by the Burt code. */

D 4
try_to_log_packet(packet)
E 4
I 4
try_to_log_message(packet)
E 4
Packet *packet;
{
  Packet msgpack;
  char query[400];
D 4
  char *response;
E 4
I 4
  char *msg, *response;
E 4
D 5
  int sender = person_array(packet->from);
E 5
I 5
  int sender = person_array(GetFrom(packet));
E 5
  
D 4
  if (DEBUG) printf("Driver logging packet to file\n");
E 4
I 4
  DPRINTF("Driver logging packet to file\n");
E 4

D 4
  if (packet->type == MESSAGE) {
E 4
I 4
  if (GetType(packet) == MESSAGE) {
E 4
    char *name, s[250];

    /* try to place a copy of the message into the log file */
    if (sender >= 0) name = gameperson[sender]->name;
D 4
    else name = "unnamed";
    sprintf(s, "%-6s %s", name, packet->info.msg);
E 4
I 4
	else name = "unnamed";
    msg = GetString(packet);
    sprintf(s, "%-6s %s", name, msg);
E 4
    log_message(s);

    /* set return message depending on gm's personality */
D 4
    first_and_rest(packet->info.msg, query);	/* skip from->to info */
E 4
I 4
    first_and_rest(msg, query);			/* skip from->to info */
E 4
    response = query_personality(0, query);     /* get burt code response */
    if (response) {
D 4
      /* send a message to the person that sent us the log message
E 4
I 4
      /* send a message to the person that sent us the log Packet
E 4
         and record our response in the log file */
D 4
      msgpack.type = MESSAGE;
      sprintf(msgpack.info.msg, "%s->%d %s", GM_NAME, packet->from, response);
      address_packet(&msgpack, FROM_GM, packet->from);
E 4
I 4
D 6
      PacketInit(&msgpack, MESSAGE);
E 6
      sprintf(s, "%s->%d %s", GM_NAME, GetFrom(packet), response);
D 6
      AddString(&msgpack, s);
E 6
I 6
      prepare_message(&msgpack, s);
E 6
      address_packet(&msgpack, FROM_GM, GetFrom(packet));
E 4
      send_to_person(sender, &msgpack, FALSE);
D 4
      sprintf(s, "%-6s %s", name, msgpack.info.msg);
E 4
I 4
      sprintf(s, "%-6s %s", name, msg);
E 4
      log_message(s);
    }
  }
}
E 1
