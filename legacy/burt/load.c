/* Personality loading routines
   Van Boughner   May 1989    */

#include <stdio.h>
#include <string.h>
#include "burt.h"
#include "extern.h"


FILE  *open_load_file();
char  *read_line();
char  *read_a_list();
char  *read_pronoun_list();
struct keyword_records	*get_a_record();


/* load the personality in given file, return TRUE if successful */

int load_personality(p, filename)
struct personalities *p;
char *filename;
{
  FILE *fp;
  char line[LINE_LENGTH];

  init_other_pointers(p);
  fp = fopen(filename, "r");
  if (fp == NULL) return FALSE;

  if (debug_mode) printf("\n===== Personality file\n");
  strcpy(line, read_line(fp));

  while (strlen(line) || !feof(fp)) {
    switch (line[0]) {
      case '#':
        strcpy(line, read_line(fp));
        break;
      case '@':
        strcpy(line, read_pronoun_list(fp, p));
	break;
      case '>':
        strcpy(line, read_a_list(fp, &p->main_list, line));
	break;
      case '?':
        strcpy(line, read_a_list(fp, &p->main_list, line));
	break;
      case '}':
        strcpy(line, read_a_list(fp, &p->sub_list, line));
	break;
      default:
        printf("burt: warning: misunderstood file line\n");
	strcpy(line, read_line(fp));
    };
  };

  if (debug_mode) printf("===== End of file.\n");
  fclose(fp);
  return TRUE;
}



init_other_pointers(p)
struct personalities *p;
{
  p->sub_list = NULL;
  p->main_list = NULL;
  p->history_list = NULL;
  p->pronoun_list = NULL;
}



/* Read a list of responses from the file after setting up the
   keyword.  Stop when a new command line is reached and return
   that line for the use of the calling routine.    */

char *read_a_list(fp, list, line)
FILE *fp;
struct keyword_records **list;
char *line;
{
  char s1[LINE_LENGTH];
  struct keyword_records *record;
  enum key_types kind_of_entry, what_type();

  record = get_a_record(list);       /* get a useable record        */
  kind_of_entry = what_type(line);   /* find out key type           */
  record->type = kind_of_entry;

  /* initialize record contents to NULLS */
  record->keyword_list = NULL;
  record->response_list = NULL;
  record->last_response = NULL;
  record->count = 0;

  implant_keywords(line, record);  /* place the keywords into it  */
  record->num_of_lines = 0;        /* initialize response counter */

  strcpy(s1, read_line(fp));
  while (!new_command_line(s1)) {
    add_response(record, s1);          /* add response to record  */
    strcpy(s1, read_line(fp));         /* get next line from file */
  }
  return s1;
}


enum key_types what_type(line)
char *line;
{
  if (line[0]=='>') return standard;
  if (line[0]=='?') return question;
  if (line[0]=='}') return substitution;
}


char *read_pronoun_list(fp, p)
FILE *fp;
struct personalities *p;
{
  char line[LINE_LENGTH];

  strcpy(line, read_line(fp));

  while (!new_command_line(line)) {
    add_pronoun(p, line);
    strcpy(line, read_line(fp));
  }
  if (debug_mode) printf("===== Pronouns read\n");
  return line;
}



/* This function returns 0 if the line given it does not have one of
   the special characters denoting a new command line at the front. */

new_command_line(line)
char *line;
{
  char front;
  int  i, found;
  char special[6];

  strcpy(special, "}>#?@");

  if ((strlen(line)==0)) return 1;
  front = line[0];
  for (i=0,found=0; (i<strlen(special)); i++)
    if (front==special[i]) found = 1;

  return found;
}
