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
	char *object_name;
	int user_id;
	int attacker_id;
	int victim_id;
	int damage_amount;
} MessageInfo;


/* macros for setting information in a message info record */

#define MI_user_name(minfo, x)		((minfo)->user_name = (x))
#define MI_attacker_name(minfo, x)	((minfo)->attacker_name = (x))
#define MI_victim_name(minfo, x)	((minfo)->victim_name = (x))
#define MI_user_team_name(minfo, x)	((minfo)->user_team_name = (x))
#define MI_attacker_team_name(minfo, x)	((minfo)->attacker_team_name = (x))
#define MI_victim_team_name(minfo, x)	((minfo)->victim_team_name = (x))
#define MI_room_name(minfo, x)		((minfo)->room_name = (x))
#define MI_object_name(minfo,x)		((minfo)->object_name = (x))
#define MI_user_id(minfo, x)		((minfo)->user_id = (x))
#define MI_attacker_id(minfo, x)	((minfo)->attacker_id = (x))
#define MI_victim_id(minfo, x)		((minfo)->victim_id = (x))
#define MI_damage_amount(minfo, x)	((minfo)->damage_amount = (x))

#define MI_USAGE_INFO(minfo, objname, username, userid, userteam, roomname) \
	{							\
		MI_object_name((minfo), (objname));		\
		MI_user_name((minfo), (username));		\
		MI_user_id((minfo), (userid));			\
		MI_user_team_name((minfo), (userteam));		\
		MI_room_name((minfo), (roomname));		\
	}

#define MI_DAMAGE_INFO(minfo, objname, attackername, attackerid, attackerteam, victimname, victimid, victimteam, roomname, damage)	\
	{								\
		MI_object_name((minfo), (objname));			\
		MI_attacker_name((minfo), (attackername));		\
		MI_attacker_id((minfo), (attackerid));			\
		MI_attacker_team_name((minfo), (attackerteam));		\
		MI_victim_name((minfo), (victimname));			\
		MI_victim_id((minfo), (victimid));			\
		MI_victim_team_name((minfo), (victimteam));		\
		MI_room_name((minfo), (roomname));			\
		MI_damage_amount((minfo), (damage));			\
	}


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
