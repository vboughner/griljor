h56875
s 00000/00000/00237
d D 1.6 92/08/07 01:02:46 vbo 6 5
c source copied to a separate tree for work on new map and object format
e
s 00081/00001/00156
d D 1.5 91/12/03 17:29:28 labc-4lc 5 4
c implemented header items for object def files
e
s 00030/00002/00127
d D 1.4 91/09/04 21:16:47 labb-3li 4 3
c 
e
s 00001/00001/00128
d D 1.3 91/07/15 01:44:06 labc-3id 3 2
c fixed problems with procedure call parameters in passing
e
s 00046/00005/00083
d D 1.2 91/07/14 14:01:59 labc-3id 2 1
c Added 'ObjDiag.t' code.
e
s 00088/00000/00000
d D 1.1 91/07/13 17:25:22 labc-3id 1 0
c date and time created 91/07/13 17:25:22 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
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

/* Object header and message text dechipering */

/* This file is responsible for handling all the details of how messages
D 4
   are stored in the message text linked list and how header information
   is also placed there. */
E 4
I 4
   and other information are stored in the message text at the beginning
   of object set definition files.

   Messages are stored by placing a line with the MESSAGE_NUMBER_PRECURSOR
   and the message number in the file, followed by as many lines as are in
   the message.  The end of the message is the last line before another
   message number or the END_OF_MESSAGES string.  Example:

   ...
   ...
   GOM# 34
   This is the text of message number 34.  It may continue
   on as many lines as you like, until another message number.
   GOM# 35
   ...
   ...

   Other pieces of information, such as the number of objects in the set
   are stored on lines with a specific key, like OKN_SIZE, on it,
   followed by the number, eg:

   okn_object_set_size:244

   All these lines are found above the line with the END_OF_MESSAGES string
   on it.  After that point, the binary object definitions follow.
   The object files may be edited with a text editor, as long as everything
   after the END_OF_MESSAGES line is left untouched.

   The END_OF_MESSAGES macro is defined in objects.h
 */
E 4


#include <stdio.h>
#include "def.h"
#include "objects.h"
#define NOT_OBTOR
#include "dialog.h"
D 2
#include "ObjDiag.h"
E 2
#include "vline.h"

/* the separator that goes between keys and values */
#define SEPARATOR ":"

/* The first part of the key that appears before every text message */
#define MESSAGE_NUMBER_PRECURSOR "GOM# "

/* key names for header items in message lines of an object file */
#define OKN_NAME	"okn_object_set_name"
#define OKN_AUTHOR	"okn_object_set_author"
#define OKN_SIZE	"okn_object_set_size"
I 5
#define OKN_CODE_FILE	"okn_object_set_code_file"
E 5


D 5

E 5
D 2
static VLine *extract_value(top, key)
/* returns the value associated with a particular key in the text */
E 2
I 2
static char *extract_value(top, key)
/* returns the value associated with a particular key in the text,
   the caller is responsible for freeing the value returned.
   NULL is returned if there is no key by that name found. */
E 2
VLine *top;
char *key;
{
I 2
  return get_value_by_key(top, key, SEPARATOR);
E 2
}



static void set_value(top, key, value)
/* finds the current line that contains the key and replaces it's value.
   Will add a line with the key and value if there isn't one already.
   If value is NULL, then any line with the given key will be deleted. */
VLine **top;  /* may need to change the top level pointer */
char *key;
char *value;
{
I 2
  char *s = valloc(strlen(key) + strlen(value) + strlen(SEPARATOR) + 1);
D 3
  VLine *current = seek_line_by_nth_token(top, key, 0, SEPARATOR);
E 3
I 3
  VLine *current = seek_line_by_nth_token(*top, key, 0, SEPARATOR);
E 3
  if (key && value) sprintf(s, "%s%s%s", key, SEPARATOR, value);

  if (current) {
    if (value) {
      /* replace contents of line already there */
      if (current->line) free(current->line);
      current->line = s;
    }
    else {
      /* remove the reference to this key */
      if (current->prev) current->prev->next = current->next;
      if (current->next) current->next->prev = current->prev;
      if (*top == current) *top = current->next;
      free_line(current);
    }
  }
  else {
    if (value) {
      /* add a new reference to the text list */
      VLine *new;
      new = make_line(s);
      new->prev = NULL;
      new->next = *top;
      *top = new;
    }
  }
E 2
}



I 5
/* =================== H E A D E R  S E T T I N G ==================== */


E 5
VLine *make_count_accurate(msgs, count)
/* takes a current message list, or NULL, and returns a list with the
   accurate count number installed/replaced in it */
VLine *msgs;
int count;
{
D 2

E 2
I 2
  char value[50];
  sprintf(value, "%d", count);
  set_value(&msgs, OKN_SIZE, value);
E 2
  return msgs;
}



I 5
VLine *os_set_name(msgs, name)
/* sets the name of an object set into the message list, returns a pointer
   to a new list (if msgs was originally NULL) or the new list (if it had
   to be changed) */
VLine *msgs;
char *name;
{
  if (name) {
    set_value(&msgs, OKN_NAME, name);
  }
  return msgs;
}



VLine *os_set_author(msgs, name)
/* sets the name of the author into the message list, returns a pointer
   to a new list (if msgs was originally NULL) or the new list (if it had
   to be changed) */
VLine *msgs;
char *name;
{
  if (name) {
    set_value(&msgs, OKN_AUTHOR, name);
  }
  return msgs;
}



VLine *os_set_codefilename(msgs, name)
/* sets the name of the code file into the message list, returns a pointer
   to a new list (if msgs was originally NULL) or the new list (if it had
   to be changed) */
VLine *msgs;
char *name;
{
  if (name) {
    set_value(&msgs, OKN_CODE_FILE, name);
  }
  return msgs;
}



/* ================== H E A D E R  S E L E C T O R S ================ */

E 5
int os_extract_size(top)
/* figures out what the header says the size of the object set is */
VLine *top;
{
D 2
  return 0;
E 2
I 2
  int result = 0;
  char *s = extract_value(top, OKN_SIZE);

  if (s) {
    result = atoi(s);
    free(s);
  }
  else printf("Warning: object file doesn't have a size line\n");

  return result;
I 5
}



/* These routines return NULL or a newly allocated string that the call
   must eventually free. */

char *os_extract_name(top)
/* gets the name of the object set from the message list */
VLine *top;
{
  return(extract_value(top, OKN_NAME));
}



char *os_extract_author(top)
/* gets the name of the author from the message list */
VLine *top;
{
  return(extract_value(top, OKN_AUTHOR));
}



char *os_extract_codefilename(top)
/* gets the name of the code file from the message list */
VLine *top;
{
  return(extract_value(top, OKN_CODE_FILE));
E 5
E 2
}
E 1
