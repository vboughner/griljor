/***************************************************************************
 * @(#) notice.c 1.9 - last change made 08/07/92
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

#define Get8(p)		((p)->info[(p)->len += 4])
#define Get16(p)	ntohs(*((CARD16 *)((p)->info + ((p)->len += 4))))
#define Get32(p)	ntohl(*((CARD32 *)((p)->info + ((p)->len += 4))))
char   *GetString(p)
Packet *p;
{
	char *ret;
	int l;
	ret = (char *)(p->info + p->len + 4);
	l = strlen(ret) + 1;
	if (l & 3) l += 4 - (l & 3);
	p->len += l;
	return ret;
}

GetArbitrary(s,d,l)
Packet *s;
char *d;
int l;
{
	bcopy(s->info + s->len + 4, d, l);
	if (l & 3) l += 4 - (l & 3);
	s->len += l;
}
	

/* #define GetType(p)		((CARD8)((p)->info[TYPE_OFFSET])) */
/* #define GetTo(p)		(ntohs(*((CARD16 *)((p)->info + TO_OFFSET)))) */
/* Defined in socket.h for some ungodly reason.		  */
#define GetFrom(p)		(ntohs(*((CARD16 *)((p)->info + FROM_OFFSET))))
#define GetPassword(p)		(ntohl(*((CARD16 *)((p)->info + PASSWORD_OFFSET))))


/* =================== D A T A G R A M  socket stuff =================== */

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
    handle_boss_packet(&packet, &from);

    /* read the next packet to handle */
    got_packet = recvPacket(&packet, &from, &timeout);
  }

}

/* Get a packet from our socket if there is anything at all to get, return
   TRUE if something was received, otherwise return FALSE.  You supply the
   amount of time to wait with a timeval structure pointer. */

int recvPacket(packet, from, wait)
Packet		*packet;
Sockaddr	*from;
struct timeval	*wait;
{
  int		ret, fromlen;

  /* if there is nothing to read then return FALSE to calling procedure */
  if (readable_on_socket(thesocket, 0) == FALSE) return FALSE;

  /* specify length of a network address */
  fromlen = sizeof(Sockaddr);

  /* pull in the packet from the socket */
  ret = recvfrom(thesocket, packet->info, MAXPACKETLEN, 0, from, &fromlen);

  /* on any kind of error but an interrupt error, crash and burn */
  if (ret < 0 && errno != EINTR) Berror("reception error while reading");
  else if (ret <= 0) return FALSE;	/* if nothing was read then return */

  /* let's check the packet's password */
  if (GetPassword(packet) != BOSS_PASSWORD) {
      if (BOSS_DEBUG)
        printf("Socket Boss: warning: packet with bad password received\n");
      return FALSE;
  }
  packet->len = RANDOM_DATA_OFFSET - 4;
  /* everthing checks out, we can keep it, set receipt flags and return TRUE */
  IO_RECEIVED();
  return TRUE;
}


/* Add incoming packets to the correct person's queue or handle them ourself
   when we can do it without needing to inform any persons */

handle_boss_packet(packet, from)
Packet *packet;
Sockaddr   *from;
{
  int fromelement;
  
  DPRINTF("Received a packet of type %d\n", GetType(packet));
  if (DEBUG) slimy_debug_incoming_packet(packet);

  /* if the packet is neither addressed to one of my persons, nor is
     addressed TO_ALL, then I should ignore it, it's probably spurious */
  if (am_driver && (GetTo(packet) != TO_GM && GetTo(packet) != TO_ALL)) return;
  else if (!am_driver)
         if (GetTo(packet) != TO_ALL && !one_of_me(GetTo(packet))) return;

  /* if the packet is from an unknown person and is not MY_INFO, MY_BITMAP,
     or MY_MASK then ignore it and send REPEAT_INFO (unless it was a
     LEAVING_GAME or CHANGE_MAP packet) */

  fromelement = person_array(GetFrom(packet));	/* look in gameperson array */
  if (fromelement == -1 && (GetFrom(packet) != FROM_GM))
    /* packet is from an unknown person */
    switch (GetType(packet)) {
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
    switch (GetType(packet)) {
	/* new player info packets */
	case MY_INFO:
	case MY_BITMAP:
	case MY_MASK:		notice_added_info(packet);
				break;
	/* packets of knowledge for all persons */
	case WEAPON_FIRED:	notice_missile(packet);
				break;
	case MULTI_FIRE:	notice_multi_fire(packet);
				break;
	case MY_LOCATION:	notice_person_location(packet);
				break;
	case CHANGE_MAP:	notice_map_change(packet);
				break;
	case REPORT:		notice_report(packet);
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
				else try_to_log_message(packet);
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
  DPRINTF("Person %d leaving the game\n", GetFrom(pack));
  pnum = person_array(GetFrom(pack));
  if (pnum == -1) {
    DPRINTF("Warning: non-existant person\n");
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
  DPRINTF("New person %d in the game\n", GetFrom(pack));
  num = persons_in_game;
  initialize_game_person(num);
  persons_in_game++;

  /* put in the persons id number and socket address */
  gameperson[num]->id = GetFrom(pack);
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
      send_to_id(GetFrom(pack), &infopack, TRUE);
    }
  }
}




/* handle MY_INFO, MY_BITMAP, and MY_MASK packets, adding info to array */

notice_added_info(pack)
Packet *pack;
{
  int pnum;	/* person array element number */
  unsigned char junk;

  /* find out array index */
  DPRINTF("Information on person %d\n", GetFrom(pack));
  pnum = person_array(GetFrom(pack));

  /* check for invalid array element */
  if (pnum == -1) {
    DPRINTF("unidentified person %d\n", GetFrom(pack));
    return;
  }

  /* figure out what kind of info it is and add it */
  switch (GetType(pack)) {
  	case MY_INFO:	/* fill in gameperson record with info given */
			gameperson[pnum]->parent = Get16(pack);
			strcpy(gameperson[pnum]->name, GetString(pack));
			strcpy(gameperson[pnum]->login, GetString(pack));
			strcpy(gameperson[pnum]->host, GetString(pack));
			strcpy(gameperson[pnum]->rank, GetString(pack));
			gameperson[pnum]->level = Get16(pack);
			gameperson[pnum]->team = Get8(pack);
			junk = Get8(pack);
			gameperson[pnum]->deity = junk & FLAG_DEITY;
			gameperson[pnum]->listed = junk & FLAG_LISTED;
			/* let possible new player know where we are */
			if (gameperson[pnum]->id == gameperson[pnum]->parent &&
			    !am_driver) notify_person_of_my_locations(pnum);
#define uu gameperson[pnum]
			DPRINTF("GOT DOODER %d, %s, %s, %s, %s, %d, %d\n",
			       uu->parent, uu->name, uu->login, uu->host,
			       uu->rank, uu->level, uu->team);
			break;
	case MY_BITMAP:	if (!am_driver) {
			  GetArbitrary(pack, gameperson[pnum]->bitmap,
				       BITMAP_ARRAY_SIZE);
			  /* update pixmap when bitmap is received */
			  if (has_display) update_pixmaps(pnum);
			}
			break;
	case MY_MASK:	if (!am_driver) {
			  GetArbitrary(pack, gameperson[pnum]->mask,
				       BITMAP_ARRAY_SIZE);
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
  unsigned int x, y, room;
  /* get array element number */
  DPRINTF("Handling location data from person %d\n", GetFrom(pack));
  pnum = person_array(GetFrom(pack));

  /* update person array as long as this isn't one of my own people */
  if (pnum >= num_persons) {
    x = Get8(pack);
    y = Get8(pack);
    room = Get16(pack);
    gameperson[pnum]->appearance = Get16(pack);
    move_person(pnum, room, x, y, TRUE);
  }
}



/* notice a multi-missile firing packet.  Separate it into missiles
   and add each of those to my regular missile list */

/* MULTI_FIRE */

notice_multi_fire(p)
Packet *p;
{
  MultiPack m;
  int len;

  m.miss.type = Get16(p);
  m.miss.owner = Get16(p);
  m.miss.hurts_owner = Get16(p);
  m.miss.room = Get16(p);
  m.include_start = Get16(p);
  m.number = Get16(p);

  GetArbitrary(p, m.x1, m.number);
  GetArbitrary(p, m.x2, m.number);
  GetArbitrary(p, m.y1, m.number);
  GetArbitrary(p, m.y2, m.number);

  add_multi_missiles(&m);
}


extern char local_x[], local_y[];

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
  GetArbitrary(p, local_x, m->range);
  GetArbitrary(p, local_y, m->range);
  m->direction = Get8(p);

  add_missile(m);
}


/* notice a report message, if we have a display, then put the
   Packet up in the error window.  Only messages for the current person
   are displayed (probably person 0) */

notice_report(pack)
Packet *pack;
{
  if ((GetTo(pack) == TO_ALL || person_array(GetTo(pack)) == 0) && has_display)
    player_error_out_message(GetString(pack));
}


/* notice when someone tells me I killed them, give myself some credit */

notice_kill(pack)
Packet *pack;
{
  int pnum = person_array(GetTo(pack));

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
  int amount;
  int pnum = person_array(Get16(pack));
  amount = Get16(pack);
  alter_experience(pnum, amount, GetString(pack));
}



/* handle a CHANGE_MAP packet, changing the map if possible, and being
   careful not to be fooled by duplicate packets */

notice_map_change(pack)

Packet *pack;
{
  int room, x, y, which, old, new;
  DPRINTF("Handling map change info from %d\n", GetFrom(pack));
  /* call for a change in the map */
  room = Get16(pack);
  x = Get8(pack);
  y = Get8(pack);
  which = Get16(pack);
  old = Get16(pack);
  new = Get16(pack);
  change_map_square(room, x, y, which, old, new, pack);
}



/* change the contents of a square according to the data given.  This is
   the routine called from the player and driver process when a CHANGE_MAP
   packet is received */

/* front end to change a pack into a recobjptr so we can call
 * change_own_map_square.
 */
change_map_square(roomnum, x, y, which, old, new, pack)
int roomnum, x, y, which, old, new;
Packet *pack;
{
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
    change_own_map_square(roomnum, x, y, which, (uc)old, (uc)new, pobj);
}

change_own_map_square(roomnum, x, y, which, old, new, obj)
int roomnum, x, y, which, old, new;
RecordedObj *obj;
{
  MemObj *ptr;
  
  /* check that these are valid values */
  DPRINTF("Changing map in room %d at %d %d\n", roomnum, x, y);
  if (roomnum < 0 || roomnum >= mapstats.rooms || x<0 || y<0 ||
      x >= ROOM_WIDTH || y >= ROOM_HEIGHT || which<0 ||
      which > ROOM_DEPTH || new<0 || new >= objects) return;

  /* if object is a recorded object then treat specially */
  if (which == ROOM_DEPTH) {
    /* check to see if we are required to replace with a recorded object
       but no recorded object record is given */
    if (info[(uc)new]->recorded && obj == NULL) return;

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
      bcopy(obj, &(ptr->obj), sizeof(RecordedObj));
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
  int winner, team;
  /* set the game end countdown timer (if game isn't already ending)
     and notify player of impending end of game */
  if (!IS_GAME_ENDING) {
    end_of_world = gametime + GAME_OVER_WAIT;

    /* figure out how long to game end (in seconds) */
    quit_time = (GAME_OVER_WAIT / CLICKS_PER_SEC);

    winner = Get16(pack);
    team = Get16(pack);
    /* print messages about game end */
    if (has_display) {
      player_error_out_message("********* Game is over *****************************");
      player_error_out_message("*** Congratulations !!!");
      if (person_array(winner) != -1) {
        sprintf(s, "*** Winning  player: %s", 
		gameperson[person_array(winner)]->name);
        player_error_out_message(s);
	/* print out winning team name */
	if (team > 0 && team <= mapstats.teams_supported) {
          sprintf(s, "*** Victorious team: %s", 
		  mapstats.team_name[team - 1]);
	  player_error_out_message(s);
	}
        sprintf(s, "*** %d seconds until full game shutdown", quit_time);
        player_error_out_message(s);
      }
      player_error_out_message("****************************************************");
    }

    if (!am_driver) {
      show_player_ending_game(team);
      end_game_experience(team, winner);
    }
  }
}



/* notice a player's request to save his stats before he leaves the game */

notice_save_stats(pack)
Packet *pack;
{
  int pnum;
  PersonRec *p;
  int experience;
  int junk;

  /* check for validity of person id number */
  DPRINTF("Noticing save stats request from %d\n", GetFrom(pack));
  pnum = person_array(junk = Get16(pack));
  DPRINTF("JUNK is %d.  pnum is %d.\n", junk, pnum);

  if (pnum<0 || pnum>persons_in_game) {
    printf("Warning: unknow person's stats\n");
    return;
  }

  /* check the person's records, update ours */
  experience = Get32(pack);
  strcpy(gameperson[pnum]->rank, GetString(pack));
  gameperson[pnum]->level = Get16(pack);
  gameperson[pnum]->kills = Get32(pack);
  gameperson[pnum]->losses = Get32(pack);
  gameperson[pnum]->games = Get32(pack);

  if (am_driver) {
    /* get the person's record out of password file if there is one */
    p = get_player(gameperson[pnum]->name);

    /* if there wasn't one then don't try to modify anything, he must
       be one of those unrecorded monsters */
    if (!p) return;

    /* otherwise modify the record and save it */
    p->ExpPts = experience;
    strcpy(p->rank, gameperson[pnum]->rank);
    strcpy(p->login, gameperson[pnum]->login);
    strcpy(p->host, gameperson[pnum]->host);
    p->kills = gameperson[pnum]->kills;
    p->losses = gameperson[pnum]->losses;
    p->games = gameperson[pnum]->games;
    modify_player(p);
    free(p);
    DPRINTF("Saved stats of person %d\n", pnum);
  }
}
 



/* notice a packet that is meant for person queues */

notice_queued_packet(pack)
Packet *pack;
{
  register int i;
  
  if (GetTo(pack) == TO_ALL)
    for (i=0; i<num_persons; i++) add_packet_to_queue(i, pack);
  else
    add_packet_to_queue(person_array(GetTo(pack)), pack);
}



/* add a queued packet to a person's queue */

add_packet_to_queue(pnum, pack)
int pnum;
Packet *pack;
{
  QPacket *new;
  
  /* check person number */
  if (pnum < 0 || pnum >= num_persons) {
    DPRINTF("Warning: bad person number for queuing packet\n");
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

try_to_log_message(packet)
Packet *packet;
{
  Packet msgpack;
  char query[400];
  char *msg, *response;
  int sender = person_array(GetFrom(packet));
  
  DPRINTF("Driver logging packet to file\n");

  if (GetType(packet) == MESSAGE) {
    char *name, s[250];

    /* try to place a copy of the message into the log file */
    if (sender >= 0) name = gameperson[sender]->name;
	else name = "unnamed";
    msg = GetString(packet);
    sprintf(s, "%-6s %s", name, msg);
    log_message(s);

    /* set return message depending on gm's personality */
    first_and_rest(msg, query);			/* skip from->to info */
    response = query_personality(0, query);     /* get burt code response */
    if (response) {
      /* send a message to the person that sent us the log Packet
         and record our response in the log file */
      sprintf(s, "%s->%d %s", GM_NAME, GetFrom(packet), response);
      prepare_message(&msgpack, s);
      address_packet(&msgpack, FROM_GM, GetFrom(packet));
      send_to_person(sender, &msgpack, FALSE);
      sprintf(s, "%-6s %s", name, msg);
      log_message(s);
    }
  }
}
