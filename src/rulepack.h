/***************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * February 1991
 **************************************************************************/

/* Rules for use by ray.c */

typedef struct _rulesquare {
	int lr, mr;
} RuleSquare;

extern struct raypack raypack[ROOM_WIDTH+1][ROOM_HEIGHT+1];
extern RuleSquare rulepack[ROOM_WIDTH][ROOM_HEIGHT];


