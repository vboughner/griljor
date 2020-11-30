h02066
s 00000/00000/00088
d D 1.6 92/08/07 21:59:26 vbo 6 5
c fixes for sun port merged in
e
s 00001/00003/00087
d D 1.5 92/02/10 21:07:54 vanb 5 4
c 
e
s 00000/00013/00090
d D 1.4 91/12/16 16:23:21 labc-4lc 4 3
c removed altering object messages and auto-actor stuff
e
s 00036/00036/00067
d D 1.3 91/12/08 18:41:17 labc-4lc 3 2
c added messages for victims of hits
e
s 00024/00004/00079
d D 1.2 91/12/08 01:44:55 labc-4lc 2 1
c added description code for info on objects
e
s 00083/00000/00000
d D 1.1 91/12/07 20:37:53 labc-4lc 1 0
c date and time created 91/12/07 20:37:53 by labc-4lc
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


typedef struct _messageinfo {
/* information record for filling in variable substitution data */
	char *user_name;
	char *attacker_name;
	char *victim_name;
	char *user_team_name;
	char *attacker_team_name;
	char *victim_team_name;
	char *room_name;
I 2
	char *object_name;
E 2
D 4
	char *altering_object_name;
E 4
	int user_id;
	int attacker_id;
	int victim_id;
	int damage_amount;
} MessageInfo;


/* macros for setting information in a message info record */

D 3
#define MI_user_name(info, x)		((info)->user_name = (x))
#define MI_attacker_name(info, x)	((info)->attacker_name = (x))
#define MI_victim_name(info, x)		((info)->victim_name = (x))
#define MI_user_team_name(info, x)	((info)->user_team_name = (x))
#define MI_attacker_team_name(info, x)	((info)->attacker_team_name = (x))
#define MI_victim_team_name(info, x)	((info)->victim_team_name = (x))
#define MI_room_name(info, x)		((info)->room_name = (x))
I 2
#define MI_object_name(info,x)		((info)->object_name = (x))
E 2
#define MI_altering_object_name(info,x) ((info)->altering_object_name = (x))
#define MI_user_id(info, x)		((info)->user_id = (x))
#define MI_attacker_id(info, x)		((info)->attacker_id = (x))
#define MI_victim_id(info, x)		((info)->victim_id = (x))
#define MI_damage_amount(info, x)	((info)->damage_amount = (x))
E 3
I 3
#define MI_user_name(minfo, x)		((minfo)->user_name = (x))
#define MI_attacker_name(minfo, x)	((minfo)->attacker_name = (x))
#define MI_victim_name(minfo, x)	((minfo)->victim_name = (x))
#define MI_user_team_name(minfo, x)	((minfo)->user_team_name = (x))
#define MI_attacker_team_name(minfo, x)	((minfo)->attacker_team_name = (x))
#define MI_victim_team_name(minfo, x)	((minfo)->victim_team_name = (x))
#define MI_room_name(minfo, x)		((minfo)->room_name = (x))
#define MI_object_name(minfo,x)		((minfo)->object_name = (x))
D 4
#define MI_altering_object_name(minfo,x) ((minfo)->altering_object_name = (x))
E 4
#define MI_user_id(minfo, x)		((minfo)->user_id = (x))
#define MI_attacker_id(minfo, x)	((minfo)->attacker_id = (x))
#define MI_victim_id(minfo, x)		((minfo)->victim_id = (x))
#define MI_damage_amount(minfo, x)	((minfo)->damage_amount = (x))
E 3

D 2
#define MI_USAGE_INFO(info, username, userid, userteam, roomname) \
E 2
I 2
D 3
#define MI_USAGE_INFO(info, objname, username, userid, userteam, roomname) \
E 3
I 3
#define MI_USAGE_INFO(minfo, objname, username, userid, userteam, roomname) \
E 3
E 2
	{							\
I 2
D 3
		MI_object_name((info), (objname));		\
E 2
		MI_user_name((info), (username));		\
		MI_user_id((info), (userid));			\
		MI_user_team_name((info), (userteam));		\
		MI_room_name((info), (roomname));		\
E 3
I 3
		MI_object_name((minfo), (objname));		\
		MI_user_name((minfo), (username));		\
		MI_user_id((minfo), (userid));			\
		MI_user_team_name((minfo), (userteam));		\
		MI_room_name((minfo), (roomname));		\
E 3
D 4
	}

D 2
#define MI_ALTER_INFO(info, username, userid, userteam, roomname, altering) \
E 2
I 2
D 3
#define MI_ALTER_INFO(info, objname, username, userid,		\
E 3
I 3
#define MI_ALTER_INFO(minfo, objname, username, userid,		\
E 3
		      userteam, roomname, altering)		\
E 2
	{							\
I 2
D 3
		MI_object_name((info), (objname));		\
E 2
		MI_user_name((info), (username));		\
		MI_user_id((info), (userid));			\
		MI_user_team_name((info), (userteam));		\
		MI_room_name((info), (roomname));		\
		MI_altering_object_name((info), (altering));	\
E 3
I 3
		MI_object_name((minfo), (objname));		\
		MI_user_name((minfo), (username));		\
		MI_user_id((minfo), (userid));			\
		MI_user_team_name((minfo), (userteam));		\
		MI_room_name((minfo), (roomname));		\
		MI_altering_object_name((minfo), (altering));	\
E 4
E 3
	}

D 2
#define MI_DAMAGE_INFO(info, attackername, attackerid, attackerteam,	\
		       victimname, victimid, victimteam, roomname, damage) \
E 2
I 2
D 3
#define MI_DAMAGE_INFO(info, objname, attackername, attackerid,		\
E 3
I 3
D 5
#define MI_DAMAGE_INFO(minfo, objname, attackername, attackerid,	\
E 3
		       attackerteam, victimname, victimid, victimteam,	\
		       roomname, damage)				\
E 5
I 5
#define MI_DAMAGE_INFO(minfo, objname, attackername, attackerid, attackerteam, victimname, victimid, victimteam,  roomname, damage) \
E 5
E 2
	{								\
I 2
D 3
		MI_object_name((info), (objname));			\
E 2
		MI_attacker_name((info), (attackername));		\
		MI_attacker_id((info), (attackerid));			\
		MI_attacker_team_name((info), (attackerteam));		\
		MI_victim_name((info), (victimname));			\
		MI_victim_id((info), (victimid));			\
		MI_victim_team_name((info), (victimteam));		\
		MI_room_name((info), (roomname));			\
		MI_damage_amount((info), (damage));			\
E 3
I 3
		MI_object_name((minfo), (objname));			\
		MI_attacker_name((minfo), (attackername));		\
		MI_attacker_id((minfo), (attackerid));			\
		MI_attacker_team_name((minfo), (attackerteam));		\
		MI_victim_name((minfo), (victimname));			\
		MI_victim_id((minfo), (victimid));			\
		MI_victim_team_name((minfo), (victimteam));		\
		MI_room_name((minfo), (roomname));			\
		MI_damage_amount((minfo), (damage));			\
E 3
	}
I 2


/* prototypes */

MessageInfo *create_message_info();
void destroy_message_info();
char *create_message();
char *substitute_in_message();
char *fill_message();
char *create_finished_message();
void destroy_message();
char *get_description();
void echo_description();
E 2
E 1
