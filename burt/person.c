/* File containing pointer routines for Burt
   Van Boughner   May 1989     */

#include <stdio.h>
#include <string.h>
#include "burt.h"
#include "extern.h"


struct keyword_records	*alloc_record();
struct pronoun_subs	*alloc_pronoun();
struct pronoun_subs	*make_pronoun_record();
struct keywords		*alloc_one_keyword();
char *first_key(), *remove_special_char();



implant_keywords(line, record)
char *line;
struct keyword_records *record;
{
  char temp[LINE_LENGTH], first[KEY_LENGTH], rest[KEY_LENGTH];

  strcpy(temp, remove_special_char(line));
  if (debug_mode) printf("===== Adding keyword(s): %s\n", temp);

  while (strlen(temp)>0) {
    strcpy(first, first_key(temp, rest));
    if (strlen(first)>0) add_one_keyword(first, record);
    strcpy(temp, rest);
  }
}



add_one_keyword(keyword, record)
char *keyword;
struct keyword_records *record;
{
  struct keywords *point, *last;

  if (!record->keyword_list) {
    point = alloc_one_keyword();
    record->keyword_list = point;
  }
  else {
    point = record->keyword_list;
    while (point) {
      last = point;
      point = point->next;
    }
    point = alloc_one_keyword();
    last->next = point;
  }

  strcpy(point->word, keyword);
  point->count = 0;
  point->next = NULL;
}



struct keywords *alloc_one_keyword()
{
  struct keywords *p;

  p = (struct keywords *)
            malloc(sizeof(struct keywords));
  if (!p) {
    printf("===== ERROR:  memory allocation fault\n");
    exit(1);
  }
  return p;
}



struct keyword_records *get_a_record(list)
struct keyword_records **list;
{
  struct keyword_records *point, *last;

  point = *list;
  if (!point) {
    *list = alloc_record();
    return *list;
  }
  else {
    while (point) {
      last = point;
      point = point->next;
    }
    point = alloc_record();
    last->next = point;
    return point;
  }
}



struct keyword_records *alloc_record()
{
  struct keyword_records *p;

  p = (struct keyword_records *)
            malloc(sizeof(struct keyword_records));
  if (!p) {
    printf("===== ERROR:  memory allocation fault\n");
    exit(1);
  }

  /* initialize a few things to NULL */
  p->keyword_list = NULL;
  p->response_list = NULL;
  p->last_response = NULL;
  p->num_of_lines = 0;
  p->count = 0;
  p->next = NULL;

  return p;
}



add_response(record, line)
struct keyword_records *record;
char *line;
{
  char *s;
  int  i;
  struct response_lines *response;


  /* Allocate space for the text line  */

  i = strlen(line) + 1;
  s = (char *) malloc(i);
  if (!s) {
    printf("===== ERROR: memory allocation fault\n");
    exit(1);
  }
  strcpy(s, line);


  /* Allocate space for the response record */

  response = (struct response_lines *)
                malloc(sizeof(struct response_lines));
  if (!response) {
    printf("===== ERROR: memory allocation falut\n");
    exit(1);
  }
  response->line = s;


  /* Find the right place to put the response record */

  if (!record->response_list) {
    record->response_list = response;
    record->last_response = response;
    response->next = NULL;
  }
  else {
    record->last_response->next = response;
    record->last_response = response;
    response->next = NULL;
  }
  record->num_of_lines++;
}



struct pronoun_subs *alloc_pronoun()
{
  struct pronoun_subs *p;

  p = (struct pronoun_subs *)
            malloc(sizeof(struct pronoun_subs));
  if (!p) {
    printf("===== ERROR:  memory allocation fault\n");
    exit(1);
  }

  return p;
}



add_pronoun(p, line)
struct personalities *p;
char *line;
{
  struct pronoun_subs *point;
  struct pronoun_subs *last;

  point = p->pronoun_list;

  if (!point) {
    point = make_pronoun_record(line);
    point->next = NULL;
    p->pronoun_list = point;
  }
  else {
    while (point) {
      last = point;
      point = point->next;
    }
    point = make_pronoun_record(line);
    point->next = NULL;
    last->next = point;
  }
}



struct pronoun_subs *make_pronoun_record(line)
char *line;
{
  struct pronoun_subs *x;
  char first_pronoun[PRONOUN_LENGTH], second_pronoun[PRONOUN_LENGTH];

  x = alloc_pronoun();
  strcpy(first_pronoun, first_key(line, second_pronoun));

  strcpy(x->s1, first_pronoun);
  strcpy(x->s2, second_pronoun);
  x->next = NULL;

  return x;
}



/* This function returns the first keyword in a given line of the
   format.    Example:    I can_/I may_/I will  (etc.)
              Returns:    I can          (with a trailing space)
   The rest of the line is returned in the variable "rest"
   For the above example value of rest is: I may_/I will     */

char *first_key(line, rest)
char *line;
char *rest;
{
  int i, slash;
  char result[LINE_LENGTH];

  /* Convert '_' char to ' ' in the string */
  for (i=0; i<strlen(line); i++)
    if (line[i]=='_')  line[i] = ' ';

  /* Find location of first '/' character */
  for (i=0,slash = -1; (i<strlen(line) && slash == -1); i++)
    if (line[i]=='/') slash = i;

  /* Assign values of result and rest accordingly */
  if (slash==-1) {
    strcpy(result, line);
    rest[0] = '\0';
  }
  else {
    strcpy(result, line);
    result[slash] = '\0';
    strcpy(rest, line+slash+1);
  }
  return result;
}



char *remove_special_char(line)
char *line;
{
  char temp[LINE_LENGTH], remove[7];
  int  j, bad, place;
  strcpy(remove, "}>#?@ ");

  place = 0;
  do {
    bad = 0;
    for (j=0; j<strlen(remove); j++)
      if (line[place]==remove[j]) bad = 1;
    if (bad)  place++;
  } while (bad);

  strcpy(temp, line+place);
  return temp;
}
