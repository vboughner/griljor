h48289
s 00000/00000/00372
d D 1.4 92/08/07 01:03:06 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00029/00013/00343
d D 1.3 91/08/26 00:34:08 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00000/00000/00356
d D 1.2 91/03/23 00:44:49 labc-3id 2 1
c 
e
s 00356/00000/00000
d D 1.1 91/02/16 12:55:21 labc-3id 1 0
c date and time created 91/02/16 12:55:21 by labc-3id
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

/* Routines for saving and retrieving player information */
/* For use of the driver/server program only */

#include <stdio.h>
#include <time.h>
#include "config.h"
#include "def.h"
#include "lib.h"
#include "password.h"


/* storage of password file's name */
char		*password_filename = DFLT_PASS_FILE;
#define NAME	(password_filename)

/* storage of file pointer */
FILE		*passfp = NULL;

/* error spotting and reporting macro */
#define demand(test,errmsg)						\
{									\
  if (!(test)) {							\
    printf("%s\n",errmsg);						\
    exit(1);								\
  }									\
}



/* routines for encoding and decoding of strings so that passwords will
   not be discovered simply with a text editor */

D 3
encode(char *string)
E 3
I 3
encode(string)
char *string;
E 3
{
  char *i = string;
  int pos = 0;
  for (; *i != '\0' ; i++, pos++) {
    *i = *i + 5 - (pos%10);
  }
}

D 3
decode(char *string)
E 3
I 3
decode(string)
char *string;
E 3
{
  char *i = string;
  int pos = 0;
  for (; *i != '\0' ; i++,pos++) {
    *i = *i - 5 + (pos%10);
  }
}




/* get record from file and decode the player's password */

D 3
PersonRec *GetRec(FILE *iptr)
E 3
I 3
PersonRec *GetRec(iptr)
FILE *iptr;
E 3
{
  PersonRec *result;
  char passtemp[PASSWORD_LENGTH];
  result = (PersonRec *)malloc(sizeof(PersonRec));
  demand (result,"Error:  malloc failed in GetRec");

  if (fread(result,sizeof(PersonRec),1,iptr) == 0) return NULL;

  decode(result->password);

  return result;
}
  


/* encode the player's password and write record to file */

D 3
PutRec(PersonRec *person,FILE *optr)
E 3
I 3
PutRec(person, optr)
PersonRec *person;
FILE *optr;
E 3
{
  encode(person->password);
  fwrite(person,sizeof(PersonRec),1,optr);
}



/* load in entire password file and return a pointer to the contents,
   if 'locking' is set to TRUE then the file is opened with a lock on it
   and it is left open at the end of this read.  You are expected to
   call Write_File in the very near future to write, unlock, and close the
   file, or very bad things could happen.  If you instead send 'locking'
   as FALSE, the file will be opened, read, and closed. */

D 3
PersonLink *Load_File(PersonLink **end, int locking)
E 3
I 3
PersonLink *Load_File(end, locking)
PersonLink **end;
int locking;
E 3
{
  PersonLink *result, *temp, *prev;
  PersonRec *record;

  temp = (PersonLink *)malloc(sizeof(PersonLink));
  demand(temp,"Error: malloc failed in Load_File");

  temp->next = NULL;
  prev = temp;
  result  = temp;
  *end = temp;

  /* open file for read, lock it if requested */
  if (locking) 
    passfp = fopen_locking(NAME, "r+", 10);
  else
    passfp = fopen(NAME, "r");

  /* if file-read error, return NULL */
  if (!passfp) return NULL;

  /* if file is empty, return NULL */
  if ((record = GetRec(passfp)) == NULL) return NULL;

  temp->person = record;

  /* otherwise, make the link-list */
  while (record = GetRec(passfp)) {
    temp = (PersonLink *)malloc(sizeof(PersonLink));
    demand(temp,"Error: malloc failed in Load_File");

    temp->next = NULL;
    temp->person = record;
    *end = temp;
    prev->next = temp;
    prev = temp;
  }

  /* return pointer to first element of link-list, close file if not locked */
  if (!locking) {
    fclose(passfp);
    passfp = NULL;
  }
  return result;
}




/* given pointer to contents, write out the password file */

D 3
int Write_File(PersonLink *data)
E 3
I 3
int Write_File(data)
PersonLink *data;
E 3
{
  PersonLink *orig = data;

  /* if file does not exist, create it, chmod it, and close it */
  if (!file_exists(NAME)) {
    char s[120];
    FILE *fp = fopen(NAME, "w");
    if (!fp) Gerror("could not create password file\n");
    fclose(fp);
    chmod(NAME, "600");
    sprintf(s, "chmod 600 %s", NAME);
    system(s);
  }

  /* if file is not open, open it */
  if (!passfp) {
    passfp = fopen_locking(NAME, "w", 10);
    if (!passfp) return 1;
  }
  else rewind(passfp);

  while (data) {
    PutRec(data->person,passfp);
    data = data->next;
  }

  fclose_unlocking(passfp);
  passfp = NULL;
  free_link(orig);
  return 0;
}




/* free memory of the password file contents */

D 3
free_link(PersonLink *data)
E 3
I 3
free_link(data)
PersonLink *data;
E 3
{
  if (data) {
    if (data->next) 
      free_link(data->next);
    free(data->person);
   free(data);
  }
}



/* free memory of password file contents except for one of the person records,
   don't free that one, we are going to want it for something */

D 3
free_link_except(PersonLink *data,PersonRec *exception)
E 3
I 3
free_link_except(data, exception)
PersonLink *data;
PersonRec *exception;
E 3
{
  if (data) {
    if (data->next) free_link_except(data->next,exception);
    if (data->person != exception) free (data->person);
    free(data);
  }
}




/* ====================== G E N E R A L  routines ======================= */

/* These routines are the ones for use by the world */


/* Set the filename that player data records will be found in */

D 3
set_password_file(char *name)
E 3
I 3
set_password_file(name)
char *name;
E 3
{
  password_filename = (char *) malloc(strlen(name)+1);
  demand(password_filename,"Malloc failed in set_password_file.");
  strcpy(password_filename,name);
}



/* Add a player to the password file, given pointer to record describing him,
   Return TRUE if file was updated, return FALSE if player already existed
   or file could not be written */

D 3
int add_player(PersonRec *person) 
E 3
I 3
int add_player(person)
PersonRec *person;
E 3
{

  PersonLink *i, *end, *new;
  PersonLink *data = Load_File(&end, TRUE);
  PersonRec  *new_person;

  for ( i = data ; i != NULL ; i = i->next) {
    if (!strcmp(i->person->name,person->name))
      return FALSE;
  }
  
  new = (PersonLink *)malloc(sizeof(PersonLink));
  demand(new,"Error: Malloc failed in add_player");

  new_person = (PersonRec *)malloc(sizeof(PersonRec));
  demand(new_person,"Error: Malloc failed in add_player");

  bcopy(person, new_person, sizeof(PersonRec));
  new->person = new_person;
  new->next = NULL;
  new->person->date = time(NULL);
  if (data)
    end->next = new;
  else 
    data = new;

  return Write_File(data);
}

  


/* Given name a player, look for his description record in the password file.
   If he is not to be found there, return NULL.  If he is found then
   a pointer to a description record is passed to you.  You must later free
   the description record on your own. */

D 3
PersonRec *get_player(char *name)
E 3
I 3
PersonRec *get_player(name)
char *name;
E 3
{
  PersonLink *end, *i;
  PersonLink *data = Load_File(&end, FALSE); 
  PersonRec *result;

  for (i = data ; (i != NULL) && strcmp(i->person->name,name) ; i = i->next) ;

  if (!i) {
    free_link(data);
    return NULL;
  }
  result = i->person;
  free_link_except(data,result);
  return result;
}




/* given a pointer to a new and improved person record, update the old
   one to agree with this one */

D 3
int modify_player(PersonRec *person)
E 3
I 3
int modify_player(person)
PersonRec *person;
E 3
{
  PersonLink *end, *i;
  PersonLink *data = Load_File(&end, TRUE); 

  for (i = data ; (i != NULL) && strcmp(i->person->name,
					person->name) ; i = i->next) ;

  if (!i) {
    free_link(data);
    return FALSE;
  }
  bcopy(person, i->person, sizeof(PersonRec));
  i->person->date = time(NULL);
  return Write_File(data);
}



/* given the name of a person, delete his record from the file if it exists */

D 3
int delete_player(char *name)
E 3
I 3
int delete_player(name)
char *name;
E 3
{
  PersonLink *end, *i,*follow;
  PersonLink *data = Load_File(&end, TRUE); 
  PersonRec result;

  for (i = data ; (i != NULL) && strcmp(i->person->name,name) ; 
       follow = i , i = i->next) ;

  if (!i) {
    free_link(data);
    return FALSE;
  }

  follow->next = i->next;
  free(i->person);
  free(i);
  return Write_File(data);
}



/* print all the names and passwords */

print_password_file()
{
  PersonLink *i, *end;
  PersonLink *data = Load_File(&end, FALSE);
  char *name;

  /* go through each record in turn */
  for (i=data; i; i = i->next) {
    name = left_justify(i->person->name, NAME_LENGTH);
    printf("%s  %s\n", name, i->person->password);
  }

  free_link(data);
}
E 1
