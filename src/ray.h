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

#define HFLAG(room, x, y) 1
#define VFLAG(room, x, y) 1
#define HSET(x, y) 0
#define VSET(x, y) 0
#define UNHSET(x, y) 0
#define UNVSET(x, y) 0


/* header file for code to handle line of sight vision for players */
/* you gotta love it! */
/* comments you say?  You're lucky even to have this code! */

struct raypack {
  int botego;
  int ritego;
  int botlit;
  int ritlit;
  int drawn;
  int lre;
  int lrl;
  int mre;
  int mrl;
  int hflag;
  int vflag;
};

typedef struct _rulesquare {
	int lr, mr;
} RuleSquare;

extern struct raypack **raypack;
extern RuleSquare rulepack[ROOM_WIDTH][ROOM_HEIGHT];
