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

/* Special game person event queue header file */

/* These future events will be stored on a queue awaiting execution.
   The list will ordered by the execution date of the commands, so that
   only the first command in the list need be chacked against the current
   time to know whether any of the commands should be executed. */

typedef struct _person_event {
	int	id;	/* id number of person affected */
	int	cmd;	/* command number of the event to take place */
	long	time;	/* gametime click number when it should happen */
	int	z[40];	/* information concerning the command */
	char	s[2][120];	/* further information for the command */
	int	die;	/* TRUE when if should erased upon person's death */
	struct _person_event *next;
} PersonEvent;



/* List of the available commands for this person event queue */

#define	PEQ_NULL	0	/* no action to be taken */
#define	PEQ_APPEAR	1	/* change the appearance flag of the person:
				   z[0] contains appearance number */
#define	PEQ_INPUT	2	/* change whether person takes user input:
				   z[0] contains TRUE or FALSE */
				/* NOT YET IMPLEMENTED */
#define	PEQ_DEATH	3	/* go through person death routine,
				   restarts person immediately elsewhere */
#define	PEQ_QUIT	4	/* go through person quit routine, starts
				   only the 4-5 second countdown */
#define	PEQ_VISIBLE	5	/* determine whether person is visible:
				   z[0] contains TRUE or FALSE */
				/* NOT YET IMPLEMENTED */
#define PEQ_ALLSEE	6	/* determines whether he sees all in room:
				   z[0] contains TRUE or FALSE */
				/* NOT YET IMPLEMENTED */
#define PEQ_EXPLOSION	7	/* dictates location and time of a future
				   explosion (useful for mines) */

#define PEQ_HIGHESTCMD	7	/* holds number of last legal command,
				   you must change this when adding commands */
