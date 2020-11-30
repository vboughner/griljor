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

/* Header file for handling player experience levels */

/* Definitions */

/* Everyone starts with something, and it can go up or down */
#define	BASE_EXP		1000	/* # of exp pts you start with */
#define	EXP_PER_LEVEL		1000	/* # of exp pts you need per level */
#define BASE_HEALTH		120	/* basic # of health points (level 1)*/
#define	HEALTH_PER_LEVEL	20	/* added health gained per level */
#define BASE_POWER		60	/* basic magic power (level 1)*/
#define POWER_PER_LEVEL		20	/* added magic power per level */
#define MAX_BURDEN		150	/* maximum weight level 1 may carry */
#define BURDEN_PER_LEVEL	10	/* added possible burden per level */
#define REVIVAL_TIME		1000	/* game clicks to regain health pt. */
#define REVIVE_BONUS_TIME 	1	/* how much faster for each level */
#define RECHARGE_TIME		1000	/* game clicks to regain magic power */
#define RECHARGE_BONUS_TIME	1	/* how much faster for each level */
#define DEMOTION_ALLOWED	FALSE	/* TRUE when you may be demoted */
#define	LEEWAY_POINTS		500	/* points you lose before demotion */
#define	LEVEL_LIMIT		3000	/* highest level attainable */
#define MIN_PERCENTAGE		65	/* low level players will be promoted
					   when their level is not equal to 
					   at least this % of average level */

/* When the game is over, you are rewarded according to how long you have been
   in the game and whether your time won.  You get a percentage according
   to "your time in game" divided by REQUIRED_TIME. */
#define	REQUIRED_TIME		2400	/* time (sec) in game for full bonus */
#define REQUIRED_PEOPLE		10	/* players in game for full bonus */
#define	BASE_GAME_REWARD	1000	/* basic reward for being in a game */
#define	BASE_TEAM_REWARD	1000	/* basic reward for winning team */
#define	BASE_WINNER_REWARD	1000	/* reward for player causing win */

/* When you kill someone, you get experience points based on your
   difference in levels. */
#define	BASE_KILL_REWARD	500	/* basic reward for killing someone */
#define	KILL_ADJUSTMENT		0.3	/* "per level" exp. reward adjustment*/

/* When you are killed by someone, you suffer a penalty to your experience
   point total, according to the level difference (like above).
   When low level people are killed by masters the penalty goes to zero. */
#define BASE_DIE_PENALTY	250	/* basic penalty */
#define	DIE_ADJUSTMENT		25	/* modifying amount */

/* Options that the driver looks at when starting up or renewing players'
   statistics from the password file */
#define LEVELS_SAVED		FALSE	/* is TRUE if the driver should save
					   player levels, FALSE when players
					   should start from 1 every time */
#define KILLS_SAVED		TRUE	/* is TRUE if driver should save
					   total number of kills player has
					   ever made, is FALSE if kill nums
					   only kept on per game basis */

/* Rank names */
#define NORANK                  "unranked"
#define TIME_TO_THROW_AWAY      ( 60 * 24 * 60 * 60 )
#define MIN_RANK_KILLS  20      /* have to have killed at least 5 people 
				   to be considered for ranking */
#define NUM_OF_RANKS	18	/* number of available ranks - this must
				   be equal to the number of strings in
				   the array below.  Be sure that rank names
				   are not longer than RANK_NAME_LENGTH.
				   There must always be at least one rank. */

static char *RANK_NAME[] = {
	"rank beginner", "mere fool", "foolish mortal",
	"flunkie", "expert flunkie", "rank sophomore", "mere sophomore",
	"sophomore", "student of war", "war studies B.S.",
	"war studies Ph.D.", "instructor of war", "professor of war",
	"dean of war", "war itself", "war incarnate", "God of War", "Jerk" };

/* don't forget to change the NUM_OF_RANKS constant above if you alter these */


/* Ranking constants */
#define MIN_GAME_TIME	  300	/* minimum amount of game seconds that must
				   elapse before any rankings are done */
#define MIN_GAME_PLAYERS  5	/* minimum number of players that must be
				   in the game for ranking to take place */
#define MIN_PLAYER_LEVEL  4	/* minimum level you must be for a re-
				   evaluation of rank to take place */
#define RANK_AT_LEVELS	  2	/* mod x at which ranking evaluations will
				   periodically take place */
#define GAME_COMPARISON	  30	/* how many games a player must win to get
				   the full games_won rank bonus */
#define BEGINNER_RUT	  30	/* a percent (0-100) of the ranking points
				   required for anything above beginner */
#define DEITY_FLAGGED	  TRUE	/* is TRUE if diety flagged players will be
				   given the following special rank name */
#define DEITY_RANK_NAME	  "Deity"

/* relative importance of different aspects of ranking, all of the
   aspects together should add up to 100 */
#define RANKASP_LEVEL	  35	/* your level / average level */
#define RANKASP_KILLS	  30	/* your kills / your losses */
#define RANKASP_TEAM	  15	/* other teams' size / your team size */
#define RANKASP_GAMES	  20	/* your games won / GAME_COMPARISON */

/* macros */
#define	max_person_health(pnum)	(BASE_HEALTH +		\
				((gameperson[pnum]->level - 1)  \
				* HEALTH_PER_LEVEL))
#define max_person_power(pnum)	(BASE_POWER +		\
				((gameperson[pnum]->level - 1) * \
				POWER_PER_LEVEL))
#define	experience_level(pts)	(((pts - BASE_EXP) >= 0) ? \
				 (((pts - BASE_EXP) / EXP_PER_LEVEL) + 1) : 0)
#define max_person_burden(pnum)	(MAX_BURDEN +		\
				((gameperson[pnum]->level - 1)  \
				* BURDEN_PER_LEVEL))
