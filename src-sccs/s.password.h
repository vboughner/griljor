h57556
s 00000/00000/00052
d D 1.4 92/08/07 01:03:08 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00003/00003/00049
d D 1.3 91/08/26 00:34:15 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00005/00001/00047
d D 1.2 91/04/17 16:53:29 labc-3id 2 1
c  changed person->pad to person->strdate for date storage
e
s 00048/00000/00000
d D 1.1 91/02/16 13:01:04 labc-3id 1 0
c date and time created 91/02/16 13:01:04 by labc-3id
e
u
U
f e 0
t
T
I 1
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
I 2
/* "date" field holds dates as they are placed in there by the driver
   when a player uses the account.  "strdate" field stores a date there
   computed from "date" field or input of user in editpass program. */
E 2

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
D 2
  char pad[70];				/* space for future stuff */
E 2
I 2
  char strdate[20];			/* string-date account last used */
  char pad[50];				/* space for future stuff */
E 2
} PersonRec;


typedef struct _personlink *lptr;

typedef struct _personlink {
  PersonRec *person;
  lptr next;
} PersonLink;



/* procedure declarations */

D 3
PersonRec  *GetRec(FILE *iptr);
PersonRec  *get_player(char *name);
PersonLink *Load_File(PersonLink **end, int locking);
E 3
I 3
PersonRec  *GetRec();
PersonRec  *get_player();
PersonLink *Load_File();
E 3
E 1
