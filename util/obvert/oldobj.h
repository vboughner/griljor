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

/* Definition File for Objects */

/* STRIPPED DOWN and CHANGED for use in obvert */

/* Storage record for the master definition of each object in the game */

typedef struct _old_obj_info {
	char	name[OBJ_NAME_LENGTH];

	char	masked;		/* TRUE if should use a mask when plotting */
	char	recorded;	/* TRUE if this object is to be carried on
				   the room list and not on the map grid */

	char	transparent;	/* TRUE if you can see through this object */
	char	glows;		/* TRUE if obj can be seen in the dark */
	char	flashlight;	/* TRUE if obj lights up dark rooms for you */

	char	move;		/* TRUE if you can move onto its square */
	short	movement;	/* the speed at which you may move in square */
	short	override;	/* non-zero if object like a stepping stone */
	char	concealing;	/* TRUE if you are not seen when on it */
	char	permeable;	/* TRUE if you may fire through the square */
	char	exit;		/* TRUE if it is an exit object */

	char	pushable;	/* TRUE if you can push the object around */

	char	takeable;	/* TRUE if a player may pick it up */
	short	weight;		/* weight of the obj when carried */

	char	weapon;		/* TRUE if this object is a weapon/throwable */
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
	short	refire;		/* (-5 to +5) speed adj. for refiring */
	short	capacity;	/* max number of charges object can store */
	char	directional;	/* TRUE if there different bitmaps for 
				   each direction it may be thrown/fired */

	char	armor;		/* TRUE if it protects you */
	short	defense;	/* added percent chance you will be missed */
	short	absorb;		/* number damage points it will absorb */
	short	wearable;	/* non-zero when you can wear this thing */

	char	swings;		/* TRUE if object may swing open or closed */
	short	alternate;	/* the character it changes to when swinging */
	char	id;		/* TRUE if object has an id number (infox) */
	short	type;		/* non-zero for typed objects (chargees) */
	short	opens;		/* non-zero if object can open something,
				   it is equal to object number it works on */

	char	vulnerable;	/* TRUE for objs destroyed by an explosion */
	short	destroyed;	/* the obj number this becomes when blown up */
	char	restorable;	/* TRUE if this object can be repaired */
	short	restored;	/* what the object will be when repaired */
	char	flag;		/* TRUE if this is a game winning "flag" */
	char	target;		/* TRUE if this is a game winning "target" */

	short	magic;		/* non-zero tells where property is stored:
				   1: detail;  2: X;  4: Y;  8: zinger etc. */
	char	set[7];		/* are TRUE when following defaults used */
	short	def[7];		/* default values for recorded obj vars */
	char	ignoring;	/* TRUE when missile goes through anything */
	char	piercing;	/* TRUE when missile only goes through non-
				   permeable spaces that are vulnerable */
	char	destroys;	/* TRUE if missile destroys vulnerable walls */
	short	boombit;	/* # of the special missile when exploding */
        
	short   arc;            /* how wide of an arc the weapn will make. 
				   0 = straight line or 360 degrees. */
	short   spread;         /* number of missiles to fire in arc
				   0 = as many as it takes */
	short   fan;            /* random degree fan to each missile */

	char	pad1[5];	/* padding so we can add more flags */
	short	pad2[4];	/* more padding */

	char	bitmap[BITMAP_ARRAY_SIZE];	/* the bit array for object */
	char	mask[BITMAP_ARRAY_SIZE];	/* the masking bitmap */
} OldObjInfo;
