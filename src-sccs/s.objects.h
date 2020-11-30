h47377
s 00000/00000/00330
d D 1.12 92/08/07 01:02:44 vbo 12 11
c source copied to a separate tree for work on new map and object format
e
s 00035/00176/00295
d D 1.11 91/12/13 23:21:37 labc-4lc 11 10
c added and deleted a lot of object properties
e
s 00007/00006/00464
d D 1.10 91/11/29 16:18:47 labc-4lc 10 9
c changed a few comments
e
s 00011/00043/00459
d D 1.9 91/11/27 15:07:12 labc-4lc 9 8
c changed message properties to hidden strings
e
s 00002/00002/00500
d D 1.8 91/11/27 13:41:23 labc-4lc 8 7
c fixed string in objects so space is malloced, not static
e
s 00006/00004/00496
d D 1.7 91/08/04 21:35:04 labc-3id 7 6
c added code for many of the limitation properties
e
s 00003/00004/00497
d D 1.6 91/07/14 14:22:09 labc-3id 6 5
c worked to make loads/saves/transferrals of objects 
e
s 00002/00001/00499
d D 1.5 91/07/07 20:03:01 labc-3id 5 4
c cleaned out a bunch of old stuff no longer in use
e
s 00050/00040/00450
d D 1.4 91/07/07 17:22:48 labc-3id 4 3
c added/changed some flags, fixed bugs in definition file save
e
s 00444/00066/00046
d D 1.3 91/06/30 18:39:59 labc-3id 3 2
c halfway done fixing up new object structure in griljor
e
s 00007/00001/00105
d D 1.2 91/02/23 01:49:29 labc-3id 2 1
c 
e
s 00106/00000/00000
d D 1.1 91/02/16 13:01:02 labc-3id 1 0
c date and time created 91/02/16 13:01:02 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
D 3
 * The War of Griljor
E 3
I 3
 * %Z% %M% %I% - last change made %G%
E 3
 *
D 3
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
E 3
I 3
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
E 3
 *
D 3
 * Students of the University of California at Berkeley
 * October 1989
E 3
I 3
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
E 3
 **************************************************************************/

/* Definition File for Objects */

D 3
/* Storage record for the master definition of each object in the game */
E 3
I 3
#define COLOR_NAME_LENGTH 40
D 6
#define MESSAGE_NUMBER_PRECURSOR "GOM# "
E 6
#define END_OF_MESSAGES "-end of object messages (do not remove this line)-"
E 3

I 3


/* Each object has a definition.  This structure represents all of the
   possible elements in that definition.  An array of these structures will
   be held in memory during the game from which information about the
   objects will be taken when it is needed. */

E 3
typedef struct _obj_info {
D 3
	char	name[OBJ_NAME_LENGTH];
E 3

D 3
	char	masked;		/* TRUE if should use a mask when plotting */
	char	recorded;	/* TRUE if this object is to be carried on
				   the room list and not on the map grid */
E 3
I 3
	/* === Name and type information === */
D 8
	char	name[OBJ_NAME_LENGTH];    /* human-readable name of object */
	char	color[COLOR_NAME_LENGTH]; /* X color name for this object */
E 8
I 8
	char	*name;		/* human-readable name of object */
	char	*color;		/* X color name for this object */
E 8
	short	type;		/* non-zero for typed objects, a type is one
				   of 1, 2, 4, 8, 16, 32, etc. */
	char	masked;		/* TRUE if a masking bitmap is defined  */
	char	recorded;	/* TRUE if this object has special info
				   allocated for each instance of it */
E 3

D 3
	char	transparent;	/* TRUE if you can see through this object */
	char	glows;		/* TRUE if obj can be seen in the dark */
	char	flashlight;	/* TRUE if obj lights up dark rooms for you */
E 3

D 3
	char	move;		/* TRUE if you can move onto its square */
	short	movement;	/* the speed at which you may move in square */
	short	override;	/* non-zero if object like a stepping stone */
	char	concealing;	/* TRUE if you are not seen when on it */
	char	permeable;	/* TRUE if you may fire through the square */
	char	exit;		/* TRUE if it is an exit object */
E 3
I 3
	/* === Description messages === */
D 9
	/* Messages are stored in a list of numbered messages at the end
	   of the object file.  Zero in a message field means that there is
	   no message for the given flag below.  A positive number denotes the
	   number of the message to be used.  A negative number signifies
	   that the number of the message is stored in one of the object's
	   recorded-object variable fields.  The absolute value of the
	   negative number given is used as a bitmask to tell which of the
	   fields contains a message number for this action.  There may be more
	   than one.  A zero or negative number in the recorded object
	   variable field will halt the lookup with a 'no message' result.
	   Message numbers in recorded object fields refer to messages
	   stored with the mapfile, not the objectfile */
	short	lookmsg;	/* if non-zero, is the number of a descriptive
				   message in the message file, that
				   a player gets when looking at an object
E 9
I 9
D 10
	char	*lookmsg;	/* message that player gets when looking at an object
E 9
				   from a distance */
D 9
	short	examinemsg;	/* if non-zero, is the number of a descriptive
				   message in the message file, that
				   user sees when 'examining' the object
E 9
I 9
	char	*examinemsg;	/* message that user sees when 'examining' the object
E 9
				   when it is in his possession */
E 10
I 10
	char	*lookmsg;	/* message that player gets when looking
				   at an object from a distance, or from
				   closeup when there is no examinemsg. */
	char	*examinemsg;	/* message that user sees when 'examining'
				   the object when it is in his possession */
E 10
D 11
	char	hideprops;	/* TRUE if the user will not be allowed to
				   get property statistics on this object
				   (a different feature than examination) */
E 11
E 3

D 3
	char	pushable;	/* TRUE if you can push the object around */
E 3

I 3
	/* === Use messages === */
D 9
	short	usemsg;		/* if non-zero, is the number of message in
				   message file sent to player as a message
				   when he uses the object, (or on autoact) */
	short	roommsg;	/* if non-zero, is the number of message in
				   message file sent to other players in the
E 9
I 9
D 10
	char	*usemsg;	/* message sent to player when he uses the object,
				   (or on autoact) */
E 10
I 10
	char	*usemsg;	/* message sent to player when he uses the
				   object */
E 10
	char	*roommsg;	/* message sent to other players in the
E 9
				   room when player uses the object */
D 9
	short	allmsg;		/* if non-zero, is the number of message in
				   message file sent to other players who
E 9
I 9
	char	*allmsg;	/* message sent to other players who
E 9
				   aren't in the same room as user, people
				   in room don't get it if there is a room
				   message.  User doesn't get it if there
				   is a user message. */
D 9
	char	alerter;	/* is TRUE if this item sends out messages
				   to each team when the item is used or
				   autoactivated.  Messages nums for neutral
				   team and each of the 4 others are kept
				   in y, zinger, extra1, extra2, and extra3 */
E 9


	/* === Damage messages === */
D 9
	short	hitmsg;		/* if non-zero, is the number of message in
				   message file the firer gets if this object,
E 9
I 9
	char	*hitmsg;	/* message the firer gets if this object,
E 9
				   as a missile, hits someone. */ 
D 9
	short	victimmsg;	/* if non-zero, is the number of message in
				   message file sent to the player hit by
E 9
I 9
	char	*victimmsg;	/* message sent to the player hit by
E 9
				   this object when used as a missile. */


D 11
	/* === Alteration messages === */
D 9
	short	usealtermsg;	/* if non-zero, is the number of message in
				   message file sent to player when he uses
E 9
I 9
	char	*usealtermsg;	/* message sent to player when he uses
E 9
				   something to alter this object */
D 9
	short	roomaltermsg;	/* if non-zero, is the number of message in
				   message file sent to other players in
E 9
I 9
	char	*roomaltermsg;	/* message sent to other players in
E 9
				   in the room when someone uses
				   something to alter this object */
D 9
	short	allaltermsg;	/* if non-zero, is the number of message in
				   message file sent to all other players
E 9
I 9
	char	*allaltermsg;	/* message sent to all other players
E 9
				   when someone alters this object */


E 11
	/* === Usage limitations and benefits === */
D 4
	short	gamesize;	/* is the minimum number of listed players
E 4
I 4
	short	minplayers;	/* is the minimum number of listed players
E 4
				   that need to be in the game before you
				   are allowed to use this object */
I 4
	short	maxplayers;	/* is the maximum number of players that
				   can be in the game and have this object
D 7
				   still work. */
E 7
I 7
				   still work.  If zero, then no maximum
				   is placed on players in game and use. */
E 7
E 4
D 11
	char	notoutside;	/* TRUE if this object cannot be used on
				   the scale defined in rooms outside */
	char	notinside;	/* TRUE if this object cannot be used on
				   the scale inherent in 'inside' rooms */
	char	represented;	/* TRUE if all teams need to be represented
				   before you can use this object */
E 11
I 11
	short	scalebits;	/* is a bitmask of the different map
				   sizing scales you are allowed to use the
				   object in.  A value of zero signifies
				   useability on all scales. */
	short	represented;	/* is a bitfield of which teams must be
				   in game before you can use this object.
				   Zero signifies no limit, '1' bit is
				   for neutral team, others follow. */
E 11
	short	rooms;		/* is a bitfield of which teams' rooms this
				   object can be used in.  Zero signifies
D 7
				   useability in all rooms.  64 bit is the
				   one to add for use in neutral rooms */
E 7
I 7
				   useability in all rooms.  The '1' bit is
				   one to add for use in neutral rooms.
				   Other teams bits follow that. */
E 7
	short	teams;		/* bitfield telling which teams can use it,
				   zero means all can use it, and neutrals
D 7
				   are assigned the 64 bit (the 6th bit).
E 7
I 7
				   are assigned the '1' bit (the 1st bit).
E 7
				   Limits who can auto-activate as well */
	short	class;		/* bitfield telling which classes can use it,
D 11
				   zero means all can use it.  This limits
				   who can autoactivate it, of course. */
E 11
I 11
				   zero means all can use it. */
E 11
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
D 11
	short	delay;		/* number of clicks (ms) of delay that occur
				   after you try to use an object and before
				   it takes effect */
	char	mustwait;	/* TRUE if you are forced to wait until a
				   delay time is over.  You can't move right
				   after using this object */
E 11


	/* === Object possession === */
E 3
	char	takeable;	/* TRUE if a player may pick it up */
	short	weight;		/* weight of the obj when carried */
I 3
D 11
	char	intohand;	/* TRUE if item taken into hand, not inv. */
	short	whichhand;	/* is 0 or 1, depending on which hand you'd
				   like the object taken into */
E 11
	char	lost;		/* TRUE if item lost from person when used,
				   or lost from gun when fired */
E 3

I 3

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
D 11
				   object.  Value specifies group it's in */
	char	connectsup;	/* TRUE if this object connects upward to
				   others in its group (like walls do) */
	char	connectsdown;	/* TRUE if this objects connects downward */
	char	connectsleft;	/* TRUE if this objects connects to left */
	char	connectsright;	/* TRUE if this objects connects to right */
	short	connectsother;	/* bitmask for other kinds of connections */
E 11
I 11
				   object.  Value specifies group it's in
				   (a bitmask, actually) */
	short	connectbits;	/* bitmask of the directions this wall
				   object connects to */
E 11


D 11
	/* === Vehicles === */
	/* Vehicles must be recorded objects and they shouldn't be takeable.
	   Detail variable must hold the room number of one of the rooms
	   that makes up the vehicle, the one that people enter in.  People
	   enter the vehicle on one of the entry squares denoted in that
	   room closest to the edge that corresponds to which side they
	   approached the vehicle.  The player standing on the driver square
	   is the one that controls the vehicle.  Access to cab could easily
	   be limited with a key.  If the vehicle can move, then the driver
	   sees the outside of the vehicle and can move the vehicles position
	   on the map.  Any exit square that doesn't point to a valid
	   map location can be used for exiting the vehicle in the direction
	   off the nearest edge.  Edges that don't point to valid rooms can
	   be used for exiting too.  Vehicles inside of vehicles shall be
	   allowed. */
	short	vehicle;	/* non-zero if this object is a vehicle,
				   its value is the max number of occupants */
	char	firein;		/* TRUE if shots fired toward the vehicle
				   are allowed to enter edges and entry
				   squares */
	short	drivespeed;	/* (0-9+) speed of the vehicle, may be more
				   than 9, due to fact that vehicles can be
				   faster than people, check override flags
				   for kinds of terrain this vehicle allows
				   players to overcome */
	/* these flags are used for a few objects inside the vehicle */
	char	entry;		/* TRUE if this obj denotes an entry square */
	char	driver;		/* TRUE if this obj denotes driver square */
	char	window;		/* TRUE if this is a window square.  Player
				   on the square can opt to get a look
				   outside the vehicle */
	char	fireout;	/* TRUE if players at this kind of window
				   can fire out of the vehicle through it */


E 11
	/* === Weapons === */
E 3
	char	weapon;		/* TRUE if this object is a weapon/throwable */
D 3
	short	damage;		/* amount of damage it can do to someone */
	short	range;		/* how far the object can be thrown/fired */
	short	speed;		/* how fast the obj moves when thrown/fired */
	short	movingobj;	/* the obj it appears as when thrown */
	char	stop;		/* TRUE if missiles stop at target point */
	short	explodes;	/* non-zero if object causes explosions */
	short	flamable;	/* non-zero if it is triggered by explosions */
	short	charges;	/* non-zero signifies what it will charge up */
	char	numbered;	/* TRUE if charges left are counted (detail) */
	char	lost;		/* TRUE if item lost from person when used */
	char	thrown;		/* TRUE if item placed on map after use */
E 3
I 3
	short	range;		/* how far the weapon fires something */
D 4
	short   arc;            /* how wide of an arc the weapn will make. 
E 4
I 4
	short   arc;            /* how wide of an arc the weapon will make. 
E 4
				   0 = straight line or 360 degrees. */
	short   spread;         /* number of missiles to fire in arc
				   0 = as many as it takes */
	short   fan;            /* if non-zero, is random degree of fanning */
	short	movingobj;	/* the obj that acts as its fired missile */
	char	stop;		/* TRUE if weapon fire stops at target point,
				   rather than taking it only as direction */
E 3
	short	refire;		/* (-5 to +5) speed adj. for refiring */
I 3
	char	numbered;	/* TRUE if charges left are counted (detail) */
E 3
	short	capacity;	/* max number of charges object can store */
I 3


	/* === Ammo === */
	short	charges;	/* non-zero signifies what it will charge up,
				   it's a bitfield compared to types */


	/* === Missiles === */
E 3
	char	directional;	/* TRUE if there different bitmaps for 
D 3
				   each direction it may be thrown/fired */
E 3
I 3
				   each direction it may be thrown/fired
				   at the end of travel as a missile */
	short	speed;		/* how fast obj is as missile or explosion */
	short	damage;		/* amount of damage it does as a missile */
D 11
	char	vampire;	/* TRUE if this missile causes a bestowal
				   the hitpoints on the firer that the victim
				   loses in a hit */
E 11
	char	mowdown;	/* TRUE if this missile goes through people,
				   wounding them and going on */
D 11
	short	putsdown;	/* bitmask of types of missile objects that
				   this object, as a missile, could block */
E 11
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
E 3

D 3
	char	armor;		/* TRUE if it protects you */
	short	defense;	/* added percent chance you will be missed */
	short	absorb;		/* number damage points it will absorb */
	short	wearable;	/* non-zero when you can wear this thing */
E 3

D 3
	char	swings;		/* TRUE if object may swing open or closed */
	short	alternate;	/* the character it changes to when swinging */
	char	id;		/* TRUE if object has an id number (infox) */
	short	type;		/* non-zero for typed objects (chargees) */
E 3
I 3
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
E 3
	short	opens;		/* non-zero if object can open something,
D 3
				   it is equal to object number it works on */
E 3
I 3
				   is a bitfield that may match any bits in
				   the target object's type number */
	char	swings;		/* TRUE if object may swing open or closed,
				   we know from type number what 'opens' it */
	short	alternate;	/* number of object it becomes when swings */
	char	id;		/* TRUE if object has an id number stored in
				   the 'x' field of recorded object record.
				   It is checked when using 'opens' feature */
E 3

D 3
	char	vulnerable;	/* TRUE for objs destroyed by an explosion */
	short	destroyed;	/* the obj number this becomes when blown up */
	char	restorable;	/* TRUE if this object can be repaired */
	short	restored;	/* what the object will be when repaired */
E 3
I 3

D 11
	/* === Clothing and armor === */
E 11
I 11
	/* === Clothing === */
E 11
	short	wearable;	/* non-zero when you can wear this thing,
				   is a bit in a field of things worn, eg.
				   so that you can only wear one 'helmet' */
D 11
	short	weartime;	/* time, in clicks (ms), it takes to put on
				   or take off this wearable item */
	short	deflect;	/* bitmask of which types of missiles
				   are completely deflected by this armor */
	short	defense;	/* percent chance you will be missed,
				   if wearing this thing */
	short	absorb;		/* percentage of damage points it will absorb,
				   if you are wearing it */
E 11


	/* === Winning the game === */
E 3
	char	flag;		/* TRUE if this is a game winning "flag" */
D 3
	char	target;		/* TRUE if this is a game winning "target" */
E 3
I 3
	short	flagteams;	/* is a bitmask of which teams need the
				   the flag to win */
D 11
	short	winner;		/* is non-zero if one can win game simply by
				   'using' an object like this the number
				   of times equal to the value of this var */
E 11
	char	important;	/* TRUE if driver pays attention to this
				   kind of object, and keeps account of them */
E 3

D 3
	short	magic;		/* non-zero tells where property is stored:
E 3
I 3

	/* === Special and magical properties */
D 11
	short	carrybits;	/* is a bitfield of bits that this object
				   causes to be set in your carrybits
				   register as seen by other players, when
				   you are carrying it, and not wearing
				   or holding it */
	short	wearbits;	/* is a bitfield of bits that this object
				   causes to be set in your wearbits
				   register as seen by other players, when
				   you are wearing it */
	short	holdbits;	/* is a bitfield of bits that this object
				   causes to be set in your holdbits
				   register as seen by other players, when
				   you are holding it */
	short	theft;		/* if non-zero, specifies a percentage chance
				   that you steal something from a person
				   that you 'hit' with this object.  Bonuses
				   are given for your level and penalties are
				   taken according to your victims level. */
	char	secure;		/* TRUE if this obj can't be stolen from you */
E 11
I 11
	char	secret;		/* is TRUE when this object is not apparent
				   as one of your possessions when you are
				   viewed by other players */
E 11
	short	magic;		/* non-zero tells where magic property ids are:
E 3
				   1: detail;  2: X;  4: Y;  8: zinger etc. */
D 3
	char	set[7];		/* are TRUE when following defaults used */
E 3
I 3
D 4
	short	protection;	/* non-zero tells where magic property ids
				   are, like above, except this list is of
				   things this object protects you from if
				   you are wearing or holding the object */
E 4
	char	set[7];		/* are TRUE when the defaults stored in
				   'def' array are used to initialize this
				   objects recorded object instances */
E 3
	short	def[7];		/* default values for recorded obj vars */
D 3
	char	ignoring;	/* TRUE when missile goes through anything */
	char	piercing;	/* TRUE when missile only goes through non-
				   permeable spaces that are vulnerable */
	char	destroys;	/* TRUE if missile destroys vulnerable walls */
	short	boombit;	/* # of the special missile when exploding */
E 3
I 3
D 9
	short	specialmsg;	/* is the message number of text that
				   contains user defined property/value
				   pairs (for very new features) */
E 9

E 3
I 2
        
D 3
	short   arc;            /* how wide of an arc the weapn will make. 
				   0 = straight line or 360 degrees. */
	short   spread;         /* number of missiles to fire in arc
				   0 = as many as it takes */
	short   fan;            /* random degree fan to each missile */
E 3
I 3
D 11
	/* === Auto-activation === */
	char	autoactor;	/* TRUE if object activiates when you on it */
	short	autodelay;	/* number of clicks before action happens */
	short	autorepeat;	/* number of clicks before it repeats if
				   you are still standing on the square */
	char	autotaken;	/* If the item is takeable, this dictates
				   that if TRUE, the item will be auto-taken
				   by person there instead of auto-acting */
E 3
E 2

D 3
	char	pad1[5];	/* padding so we can add more flags */
D 2
	short	pad2[7];	/* more padding */
E 2
I 2
	short	pad2[4];	/* more padding */
E 3
E 2

E 11
I 3
	/* === Breakage === */
	short	breaks;		/* percentage chance of breaking each use,
				   units used are 100ths of a percent */
	short	broken;		/* object it becomes when broken */


	/* === Nullifying other objects === */
D 4
	short	override;	/* is a bit-field telling which recorded
				   object fields contain the numbers of
				   objects this object can override.  Should
				   this object override another, the object
				   overridden will not do it's normal things,
				   nor affect players' movement, etc.
				   Other bits in the field might mean special
				   things about how override is done */
E 4
I 4
D 11
	short	override;	/* is the number of the kind of object
				   this object will override.  Should this
				   object be capable of overriding another,
E 11
I 11
	short	overridden;	/* is a bitfield that needs to match with
				   any overriding objects override bits. */
	short	overrider;	/* is a bitfield that should match with
				   overridden bits of victim object. If this
				   object is capable of overriding another,
E 11
				   the other flags in this section dictate
				   what is overridden. */
	char	ignoremove;	/* causes the overridden object's movement
				   flag to be ignored during the figuring
				   of player movement on the square */
	char	ignoretrans;	/* causes the overridden object's
				   transparency to be ignored for sighting */
	char	ignoreperm;	/* causes the overridden object's
				   permeability to be ignored for firing */
D 11
	char	ignoreauto;	/* causes the overridden object's auto-
				   activation to be ignored */
	char	pullcharges;	/* causes the number of charges in the
				   overriding objects to be reduced each
				   time an override is neccessary.  When
				   charges are gone, it could disappear. */
E 11
E 4


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
D 4
	short	dropper;	/* is a bit-field of recorded fields which
				   contain objects that are dropped/
				   removed from a player's inventory when
				   he/she uses this object (even if those
				   objects are 'unremoveable' etc.)
				   The 512 bit, when set, signifies that
				   the stuff will be dropped in random
				   map locations rather than nearby. */
	short	destroyer;	/* is a bit-field of recorded fields which
				   contain objects that are destroyed and
				   removed from a player's inventory when
				   he/she uses this object (even if those
				   objects are 'unremoveable' etc.) */
	short	creator;	/* is a bit-field of recorded fields which
				   contain objects that are created and
				   added to a player's inventory when
				   he/she uses this object (even if those
				   objects are 'unremoveable' etc.)  Objects
				   are in priority order in case there isn't
				   room in player's inventory for all */
E 4
I 4
D 11
	short	dropper;	/* is the number of the object that you
				   are made to drop when you use this
				   object.  This will override the
				   undroppable setting of the object, and
				   will cause you to drop one of the copies
				   of the object nearby.  If you don't
				   have the specified kind of object in
				   you inventory, nothing happens. */
	short	fardropper;	/* is like the previous flag, but makes
				   the dropped object appear in a different
				   room rather than nearby. */
	short	destroyer;	/* is the number of the kind of object
				   this will cause to be destroyed in your
				   inventory.  Only one copy of whatever
				   kind of object it is will be destroyed */
	short	creator;	/* is the number of the kind of object this
				   object will cause to be created in your
				   inventory each time you use it.  If
				   you don't have room in your inventor
				   for the object, it will be dropped on
				   the floor nearby. */
E 11
E 4


D 11
	/* === Object chaining === */
I 4
	short	multiple;	/* is the number of extra times this
				   object calls its execution routines.
				   Can be used to make objects that
				   create 5 of something in your inventory,
				   for example.  If the value is 0 the object
				   will trigger just once, like normal. */
E 4
	char	chains;		/* TRUE if this object, once used, calls upon
				   a generic instance of another kind of
				   object to act in the same way it just did */
	short	chainobj;	/* the number of object that will be chained
				   to if the above flag was TRUE.  A temporary
				   copy will be made of new object.  No loops
				   are permitted. */


	/* === Miscellaneous === */
	char	makesmove;	/* TRUE if this object forces player to
				   begin moving in direction this object
				   is 'fired' in, such an object could replace
				   the movement icon in right button box */
	char	swapper;	/* TRUE if this object causes the contents
				   on two different squares on the map to
				   be swapped.  First location is in detail,
				   x, y, and the second is in extra1, extra2,
				   and extra3 */
D 4


	/* === Future additions === */
	char	pad1[100];	/* padding so we can add more flags */
	short	pad2[100];	/* more padding.  All pads have been
				   initialized to zeros upon creation. */
E 4


E 11
	/* === Appearance === */
E 3
	char	bitmap[BITMAP_ARRAY_SIZE];	/* the bit array for object */
	char	mask[BITMAP_ARRAY_SIZE];	/* the masking bitmap */
I 11


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
E 11
I 3

E 3
} ObjInfo;



#ifndef OBJ_MAIN
D 3

E 3
/* external definitions for programs other than objects.c */
D 3

E 3
D 5
extern ObjInfo	*info[MAX_OBJECTS];	/* array of pointers to obj info */
E 5
I 5
D 6
extern ObjInfo	**info;			/* array of pointers to obj info */
extern void	*obj_messages;		/* linked list of object messages */
E 5
extern int	objects;		/* how many objects are defined */
E 6
I 6
extern ObjInfo		**info;		/* array of pointers to obj info */
extern void		*obj_messages;	/* linked list of object messages */
extern int		objects;	/* how many objects are defined */
E 6
D 3

E 3
#endif


D 3
/* procedures in objects.c */

E 3
I 3
/* protypes for procedures in objects.c */
E 3
ObjInfo *allocate_obj();
I 3

E 3
E 1
