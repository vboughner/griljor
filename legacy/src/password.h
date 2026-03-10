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

/* Header file for player info storage routines */


/* data structure to hold information about a person */
/* "date" field holds dates as they are placed in there by the driver
   when a player uses the account.  "strdate" field stores a date there
   computed from "date" field or input of user in editpass program. */

typedef struct _personrec {
  char name[NAME_LENGTH];		/* player's name */
  char password[PASSWORD_LENGTH];	/* password (encripted when in file) */
  long date;				/* date account last used */
  int ExpPts;				/* experience accumulated */
  char rank[RANK_LENGTH];		/* string representing rank */
  int bitmap_saved;			/* is TRUE when his image is saved */
  char bitmap[BITMAP_ARRAY_SIZE];	/* player's bitmap image */
  char mask[BITMAP_ARRAY_SIZE];		/* mask for behind the image */
  int kills, losses;			/* stats on number of kills he has */
  int games;				/* number of games he has won */
  int deity;				/* TRUE if person is a deity */
  char login[LOGIN_LENGTH];		/* player's last login name */
  char host[HOST_LENGTH];		/* host that player last played from */
  char strdate[20];			/* string-date account last used */
  char pad[50];				/* space for future stuff */
} PersonRec;


typedef struct _personlink *lptr;

typedef struct _personlink {
  PersonRec *person;
  lptr next;
} PersonLink;



/* procedure declarations */

PersonRec  *GetRec();
PersonRec  *get_player();
PersonLink *Load_File();
