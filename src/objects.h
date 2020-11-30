/***************************************************************************
 * @(#) objects.h 1.12 - last change made 08/07/92
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

/* Definition File for Objects */

#define COLOR_NAME_LENGTH 40
#define END_OF_MESSAGES "-end of object messages (do not remove this line)-"



/* Each object has a definition.  This structure represents all of the
   possible elements in that definition.  An array of these structures will
   be held in memory during the game from which information about the
   objects will be taken when it is needed. */

typedef struct _obj_info {

	/* === Name and type information === */
	char	*name;		/* human-readable name of object */
	char	*color;		/* X color name for this object */
	short	type;		/* non-zero for typed objects, a type is one
				   of 1, 2, 4, 8, 16, 32, etc. */
	char	masked;		/* TRUE if a masking bitmap is defined  */
	char	recorded;	/* TRUE if this object has special info
				   allocated for each instance of it */


	/* === Description messages === */
	char	*lookmsg;	/* message that player gets when looking
				   at an object from a distance, or from
				   closeup when there is no examinemsg. */
	char	*examinemsg;	/* message that user sees when 'examining'
				   the object when it is in his possession */


	/* === Use messages === */
	char	*usemsg;	/* message sent to player when he uses the
				   object */
	char	*roommsg;	/* message sent to other players in the
				   room when player uses the object */
	char	*allmsg;	/* message sent to other players who
				   aren't in the same room as user, people
				   in room don't get it if there is a room
				   message.  User doesn't get it if there
				   is a user message. */


	/* === Damage messages === */
	char	*hitmsg;	/* message the firer gets if this object,
				   as a missile, hits someone. */ 
	char	*victimmsg;	/* message sent to the player hit by
				   this object when used as a missile. */


	/* === Usage limitations and benefits === */
	short	minplayers;	/* is the minimum number of listed players
				   that need to be in the game before you
				   are allowed to use this object */
	short	maxplayers;	/* is the maximum number of players that
				   can be in the game and have this object
				   still work.  If zero, then no maximum
				   is placed on players in game and use. */
	short	scalebits;	/* is a bitmask of the different map
				   sizing scales you are allowed to use the
				   object in.  A value of zero signifies
				   useability on all scales. */
	short	represented;	/* is a bitfield of which teams must be
				   in game before you can use this object.
				   Zero signifies no limit, '1' bit is
				   for neutral team, others follow. */
	short	rooms;		/* is a bitfield of which teams' rooms this
				   object can be used in.  Zero signifies
				   useability in all rooms.  The '1' bit is
				   one to add for use in neutral rooms.
				   Other teams bits follow that. */
	short	teams;		/* bitfield telling which teams can use it,
				   zero means all can use it, and neutrals
				   are assigned the '1' bit (the 1st bit).
				   Limits who can auto-activate as well */
	short	class;		/* bitfield telling which classes can use it,
				   zero means all can use it. */
	short	minlevel;	/* minimum level player must have in order
				   to use this object */
	short	maxlevel;	/* the last level during which a player may
				   use this object.  Zero means that there
				   is no upper limit on level for usage. */
	short	experience;	/* number of experience points this takes
				   from you per use.  Negative number
				   signifies that it gives you points */
	short	power;		/* percentage of mana points person must
				   have in order to use this object */
	short	mana;		/* number of magic points object requires
				   per use.  Could be negative, meaning
				   that object gives player power pts. */
	short	strength;	/* percentage of health points person must
				   have left in order to use this object */
	short	health;		/* number of health points object requires
				   per use.  Could be negative, signifying
				   that object's use give player health pts */
	char	needanother;	/* TRUE if you must have a particular object
				   in your other hand in order to use this */
	short	otherobj;	/* The object you must have in other hand */
	char	deity;		/* TRUE if this object can only be used by
				   those who have deity mode priviledges */


	/* === Object possession === */
	char	takeable;	/* TRUE if a player may pick it up */
	short	weight;		/* weight of the obj when carried */
	char	lost;		/* TRUE if item lost from person when used,
				   or lost from gun when fired */


	/* === General properties === */
	char	exit;		/* TRUE if it is an exit object.  Destination
				   is stored in detail, x, and y recorded
				   object variables.  -1 values for x and/or
				   y provide randomness in the room.  -1 thru
				   -4 for room specify a random team room.
				   -10 signifies one's own team, -20 is for
				   a random enemy's team room, -30 for a
				   neutral room and anything other negative
				   means any room that allows appearances.
				   The zinger variable may contain the
				   number of clicks of delay before an exit
				   that you are standing on whisks you away.
				   -1 in zinger means 'movement' is used */
	short	movement;	/* max speed (0-9) you may move over it */
	char	permeable;	/* TRUE if you may fire through the square */
	char	transparent;	/* TRUE if you can see through this object */
	char	concealing;	/* TRUE if you are not seen when on it */
	char	glows;		/* TRUE if obj can be seen in the dark */
	short	flashlight;	/* if non-zero, is radius that this
				   object lights up in dark rooms for you */
	char	invisible;	/* TRUE if object is not seen when on floor
				   or in air, but is seen when in your inv. */
	short	connectgroup;	/* non-zero if this object is a wall or some
				   other type of map-editor random placeable
				   object.  Value specifies group it's in
				   (a bitmask, actually) */
	short	connectbits;	/* bitmask of the directions this wall
				   object connects to */


	/* === Weapons === */
	char	weapon;		/* TRUE if this object is a weapon/throwable */
	short	range;		/* how far the weapon fires something */
	short   arc;            /* how wide of an arc the weapon will make. 
				   0 = straight line or 360 degrees. */
	short   spread;         /* number of missiles to fire in arc
				   0 = as many as it takes */
	short   fan;            /* if non-zero, is random degree of fanning */
	short	movingobj;	/* the obj that acts as its fired missile */
	char	stop;		/* TRUE if weapon fire stops at target point,
				   rather than taking it only as direction */
	short	refire;		/* (-5 to +5) speed adj. for refiring */
	char	numbered;	/* TRUE if charges left are counted (detail) */
	short	capacity;	/* max number of charges object can store */


	/* === Ammo === */
	short	charges;	/* non-zero signifies what it will charge up,
				   it's a bitfield compared to types */


	/* === Missiles === */
	char	directional;	/* TRUE if there different bitmaps for 
				   each direction it may be thrown/fired
				   at the end of travel as a missile */
	short	speed;		/* how fast obj is as missile or explosion */
	short	damage;		/* amount of damage it does as a missile */
	char	mowdown;	/* TRUE if this missile goes through people,
				   wounding them and going on */
	short	explodes;	/* non-zero if object causes explosions after
				   it gets to end of its useful missile life */
	char	snuffs;		/* TRUE if this missile will NOT explode
				   upon hitting a wall or person, etc. */
	short	boombit;	/* the obj this one explodes into */
	short	ignoring;	/* is a bitmask of the types of the
				   walls/other objects this missile completely
				   ignores for the purposes of permeability */
	short	piercing;	/* is a bitmask of the types of walls that
				   this missile may ignore in cases where
				   those walls are also vulnerable */
	short	destroys;	/* is a bitmask of the types of walls/other
				   objects this missile destroys if those
				   objects are vulnerable */


	/* === Destruction === */
	short	flammable;	/* non-zero if can be a secondary explosion,
				   number is radius of explosion  Uses
				   'boombit' for explosion object.
				   Objects that set it off are ones that
				   would destroy it (see 'destroys') */
	char	vulnerable;	/* TRUE for objs destroyed by an explosion,
				   the objects type number will be used
				   to know which missiles can destroy it */
	short	destroyed;	/* the obj number this becomes when blown up */


	/* === Alteration === */
	short	opens;		/* non-zero if object can open something,
				   is a bitfield that may match any bits in
				   the target object's type number */
	char	swings;		/* TRUE if object may swing open or closed,
				   we know from type number what 'opens' it */
	short	alternate;	/* number of object it becomes when swings */
	char	id;		/* TRUE if object has an id number stored in
				   the 'x' field of recorded object record.
				   It is checked when using 'opens' feature */


	/* === Clothing === */
	short	wearable;	/* non-zero when you can wear this thing,
				   is a bit in a field of things worn, eg.
				   so that you can only wear one 'helmet' */


	/* === Winning the game === */
	char	flag;		/* TRUE if this is a game winning "flag" */
	short	flagteams;	/* is a bitmask of which teams need the
				   the flag to win */
	char	important;	/* TRUE if driver pays attention to this
				   kind of object, and keeps account of them */


	/* === Special and magical properties */
	char	secret;		/* is TRUE when this object is not apparent
				   as one of your possessions when you are
				   viewed by other players */
	short	magic;		/* non-zero tells where magic property ids are:
				   1: detail;  2: X;  4: Y;  8: zinger etc. */
	char	set[7];		/* are TRUE when the defaults stored in
				   'def' array are used to initialize this
				   objects recorded object instances */
	short	def[7];		/* default values for recorded obj vars */

        
	/* === Breakage === */
	short	breaks;		/* percentage chance of breaking each use,
				   units used are 100ths of a percent */
	short	broken;		/* object it becomes when broken */


	/* === Nullifying other objects === */
	short	overridden;	/* is a bitfield that needs to match with
				   any overriding objects override bits. */
	short	overrider;	/* is a bitfield that should match with
				   overridden bits of victim object. If this
				   object is capable of overriding another,
				   the other flags in this section dictate
				   what is overridden. */
	char	ignoremove;	/* causes the overridden object's movement
				   flag to be ignored during the figuring
				   of player movement on the square */
	char	ignoretrans;	/* causes the overridden object's
				   transparency to be ignored for sighting */
	char	ignoreperm;	/* causes the overridden object's
				   permeability to be ignored for firing */


	/* === Cursed/permanently held objects === */
	short	bornwith;	/* non-zero if this is an object you
				   should come alive with already in your
				   possession.  Value is the number of these
				   you should get.  Level, class, team, diety
				   values must be correct for you to get it. */
	char	unremoveable;	/* TRUE for objects you can't take off,
				   (except in death or dropper item use) */
	char	undroppable;	/* TRUE for objects you can't drop,
				   (except in death or dropper item use) */
	char	unswitchable;	/* TRUE for objects you can't take from
				   your hand once there. */
	char	nodeathdrop;	/* TRUE if this object is not dropped upon
				   your death.  Used often with 'bornwith'
				   and 'undroppable'.  'dropper' objects
				   may still affect it, however */


	/* === Appearance === */
	char	bitmap[BITMAP_ARRAY_SIZE];	/* the bit array for object */
	char	mask[BITMAP_ARRAY_SIZE];	/* the masking bitmap */


	/* === Forth Code === */
	char	*code;		/* pointer to object's Forth code text */
	short	varspace;	/* how many 4-byte pointers worth of
				   space will be allocated in the monster
				   server as local variable storage space */
	short	object1;	/* number of another object referenced
				   somewhere in the Forth code */
	short	object2;	/* number of another object referenced
				   somewhere in the Forth code */
	short	object3;	/* number of another object referenced
				   somewhere in the Forth code */

} ObjInfo;



#ifndef OBJ_MAIN
/* external definitions for programs other than objects.c */
extern ObjInfo		**info;		/* array of pointers to obj info */
extern void		*obj_messages;	/* linked list of object messages */
extern int		objects;	/* how many objects are defined */
#endif


/* protypes for procedures in objects.c */
ObjInfo *allocate_obj();

