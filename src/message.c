/***************************************************************************
 * @(#) message.c 1.5 - last change made 08/28/92
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

/* message creation routines (for messages passed around concerning events */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "objects.h"
#include "map.h"
#include "vline.h"
#include "message.h"


/* =========== I N F O R M A T I O N  G A T H E R I N G ============== */

MessageInfo *create_message_info()
/* creates a record message info stuff into which information may be
   planted.  All strings in message info structures will be the static
   pointers passed in.  No copies will be made of such strings. */
{
  MessageInfo *new = (MessageInfo *) malloc(sizeof(MessageInfo));
  demand(new, "no memory for message info record");
  bzero(new, sizeof(MessageInfo));
  return new;
}



void destroy_message_info(info)
/* frees a message info record */
MessageInfo *info;
{
  if (info) free(info);
}



/* ================= M E S S A G E  M A N G L I N G ================= */

char *create_message(s)
/* given one of the hidden message strings available in the object
   definitions, create a copy of the message for later editing and
   insertions by routines in this file */
char *s;
{
  char *result;
  if (!s) return NULL;
  demand((result = (char *) malloc(strlen(s) + 1)),
	 "no memory for message copy");
  strcpy(result, s);
  return result;
}



static char *itoa(num)
/* returns a statically allocated string representing the number given */
int num;
{
  static char result[15];
  sprintf(result, "%d", num);
  return result;
}



static char *substitute_word(s, variable, replacement)
/* goes through string s, substituting all occurance of string
   'variable' with string 'replacement'.  Returns pointer to a new
   string, frees the old string.  Might not work with strings that contain
   more than 20 occurances of variable string. */
char *s;
char *variable;
char *replacement;
{
  char *new, *ptr, *nptr, varlen, endlen, count;
  if (!s || !variable || !replacement) return s;
  new = (char *) malloc(strlen(s) + strlen(replacement) * 20 + 1);
  demand(new, "not enough memory for substitution string");

  nptr = new;
  varlen = strlen(variable);
  endlen = strlen(s) - varlen + 1;

  for (ptr=s,count=0; *ptr;) {
    if ((count < endlen) && (!strncmp(ptr, variable, varlen))) {
      strcpy(nptr, replacement);
      nptr += strlen(replacement);
      ptr += varlen;
      count += varlen;
    }
    else {
      *nptr = *ptr;
      ptr++;
      nptr++;
      count++;
    }
  }

  *nptr = '\0';
  free(s);
  return new;
}



char *substitute_in_message(s, info)
/* takes writeable string and substitutes the variables inside with
   the information found in info record */
char *s;
MessageInfo *info;
{
  /* full variable names */
  s = substitute_word(s, "$object_name", info->object_name);
  s = substitute_word(s, "$user_name", info->user_name);
  s = substitute_word(s, "$attacker_name", info->attacker_name);
  s = substitute_word(s, "$hitter_name", info->attacker_name);
  s = substitute_word(s, "$victim_name", info->victim_name);
  s = substitute_word(s, "$user_team_name", info->user_team_name);
  s = substitute_word(s, "$attacker_team_name", info->attacker_team_name);
  s = substitute_word(s, "$victim_team_name", info->victim_team_name);
  s = substitute_word(s, "$room_name", info->room_name);
  s = substitute_word(s, "$user_id", itoa(info->user_id));
  s = substitute_word(s, "$attacker_id", itoa(info->attacker_id));
  s = substitute_word(s, "$victim_id", itoa(info->victim_id));
  s = substitute_word(s, "$damage_amount", itoa(info->damage_amount));

  /* abbreviated variable names */
  s = substitute_word(s, "$on", info->object_name);
  s = substitute_word(s, "$un", info->user_name);
  s = substitute_word(s, "$an", info->attacker_name);
  s = substitute_word(s, "$hn", info->attacker_name);
  s = substitute_word(s, "$vn", info->victim_name);
  s = substitute_word(s, "$utn", info->user_team_name);
  s = substitute_word(s, "$atn", info->attacker_team_name);
  s = substitute_word(s, "$vtn", info->victim_team_name);
  s = substitute_word(s, "$rn", info->room_name);
  s = substitute_word(s, "$ui", itoa(info->user_id));
  s = substitute_word(s, "$ai", itoa(info->attacker_id));
  s = substitute_word(s, "$vi", itoa(info->victim_id));
  s = substitute_word(s, "$da", itoa(info->damage_amount));

  return s;
}



char *fill_message(s, column)
/* given a writeable string, change all new-lines to spaces then go through
   and put in new new-lines so that string is filled on the right to the
   indicated column.  Returns pointer to new string.  This doesn't work
   well for strings with words that are longer than 'column' characters. */
char *s;
int column;
{
  char *ptr, *last_space;
  int count;
  demand((column > 0), "column must be more than zero in fill_message()");
  if (!s) return NULL;

  for (ptr=s; *ptr; ptr++)
    if (*ptr == '\n') *ptr = ' ';

  count = 0;
  last_space = NULL;
  for (ptr=s; *ptr; ptr++) {
    if (*ptr == ' ') last_space = ptr;
    if (count >= column && last_space) {
	*last_space = '\n';
	count = ptr - last_space + 1;
	ptr = last_space + 1;
	last_space = NULL;
    }
    else count++;
  }

  return s;
}



char *create_finished_message(s, info, column)
/* perform all above functions on a message, returns pointer to finished
   message result. */
char *s;
MessageInfo *info;
int column;
{
  char *new = create_message(s);
  if (!new) return NULL;
  new = substitute_in_message(new, info);
  new = fill_message(new, column);
  return new;
}



void destroy_message(s)
/* free the copy of the message that was created earlier */
char *s;
{
  if (s) free(s);
}



/* ================ E C H O  M E S S A G E S ======================== */

static char *prepend_name(name, msg)
/* prepends the name on the message (if there is a name) and returns
   a new string pointing to it.  The string returned should be copied before
   the next call to this procedure. */
char *name;
char *msg;
{
  static char *prepended = NULL;
  demand(msg, "no msg to prepend_name()");
  if (prepended) free(prepended);

  if (name) {
    prepended = (char *) malloc(strlen(name) + strlen(msg) + 10);
    demand(prepended, "no memory for prepending");
    sprintf(prepended, "%s: %s", name, msg);
  }
  else {
    prepended = (char *) malloc(strlen(msg) + 1);
    demand(prepended, "no memory for prepending");
    strcpy(prepended, msg);
  }

  return prepended;
}



char *get_description(objnum, nearby, columns, msginfo)
/* returns a pointer to a message string you can send to the error
   reporting window */
int objnum;
int nearby;
int columns;
MessageInfo *msginfo;
{
  char *new;

  if (nearby && info[objnum]->examinemsg)
    new = prepend_name(info[objnum]->name, info[objnum]->examinemsg);
  else if (info[objnum]->lookmsg)
    new = prepend_name(info[objnum]->name, info[objnum]->lookmsg);
  else
    new = info[objnum]->name;

  if (new && (!strcmp(new, "no name"))) new = NULL;

  if (new) {
    new = create_finished_message(new, msginfo, columns);
  }
  return new;
}



void echo_description(objnum, nearby, columns, info)
/* echoes description of an object to the error report window */
int objnum;
int nearby;
int columns;
MessageInfo *info;
{
  char *new = get_description(objnum, nearby, columns, info);
  if (new) {
    player_error_out_message(new);
    destroy_message(new);
  }
}
