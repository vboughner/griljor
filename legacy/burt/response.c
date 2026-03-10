/* Burt's response mechanism
   Van Boughner   May 1989   */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "burt.h"
#include "extern.h"


char *make_response(s, p)
char *s;
struct personalities *p;
{
  struct keyword_records  *point;
  struct keywords	  *keys;
  char   keep_phrase[LINE_LENGTH], question_word[KEY_LENGTH];
  char   *question_word_grasp(), *no_key_found_response();
  char   *answer_question(), *respond_to_key();
  int	 found, question_phrase;

  if (strlen(s)==0) return "\0";
  if (p->main_list==NULL) return no_key_found_response(p);
  point = p->main_list;
  found = 0;

  while (point && !found) {
    keys = point->keyword_list;

    while (keys && !found) {
      found = phrase_grasp(s, keys->word, keep_phrase);
      if (debug_mode)
        printf("===== Phrase check: [%d] %s\n", found, keys->word);
      question_phrase = (point->type==question);
      if (question_phrase && found) {
        strcpy(question_word,
	       question_word_grasp(s, point->response_list));
	if (strlen(question_word)==0)  found = 0;
	if (debug_mode && found)
	  printf("===== Secondary phrase found: %s\n", question_word);
      }
    if (!found)  keys = keys->next;
    }

  if (!found) point = point->next;
  }

  if (!found)  return no_key_found_response(p);
  keys->count++;
  point->count++;

  if (question_phrase)
    return answer_question(p, point, question_word, keep_phrase);
  else
    return respond_to_key(p, point, keep_phrase);
}




/* This function searches through a sentence for a particular
   phrase.  It returns 1 if found and 0 if not found.  It ignores
   characters found in ignore[] in either string, matching things
   up regardless of the occurance of these.  It also allows space
   characters in the search phrase to be represented in the
   sentence by the punctuation marks found in space_subs[] or by
   line barriers on either side of the phrase.
   It returns the part of the sentence after the key phrase in the
   variable after_phrase.  */

phrase_grasp(sentence, phrase, after_phrase)
char *sentence, *phrase, *after_phrase;
{
  char space_subs[9];
  char ignore[2];
  char c1, c2;
  int i, i_hold, j, ok, length, found, end_of_phrase;

  strcpy(space_subs, " .,!()?/");
  strcpy(ignore, "'");

  length = strlen(sentence);
  for (i=0,found=0; (i<=length && !found); i++) {
    i_hold = i;
    for (j=0,ok=1; (j<strlen(phrase) && ok &&
                    i_hold<=strlen(sentence));) {
      c1 = sentence[i_hold];
      c2 = phrase[j];
      ok = no_care_match(c1, c2);
      if (!ok) {
        if (c2==' ' && (i_hold==0 || i_hold==strlen(sentence))) {
	  ok = 1;  j++;
	}
        if (is_element_of(space_subs, c1) && c2==' ')  {
	  ok = 1;  j++;  i_hold++;
	}
	else {
	  if (is_element_of(ignore, c1)) {
	    i_hold++;  ok = 1;
	  }
	  if (is_element_of(ignore, c2)) {
	    j++;  ok = 1;
	  }
	}
      }
      else {
        j++;
	i_hold++;
      }
    }
    if (ok &&
         (j==strlen(phrase) ||
         (j==strlen(phrase)-1 && phrase[j]==' '))) {
      found = 1;
      end_of_phrase = i_hold;
    }
  }
  if (found) strcpy(after_phrase, sentence+end_of_phrase);
  return found;
}



is_element_of(s, c)
char *s, c;
{
  int i, found;
  for (i=0,found=0; (i<strlen(s) && !found); i++)
    if (s[i]==c)  found = 1;
  return found;
}



/* This function returns 1 if the two characters match,
   regardless of case       */

no_care_match(c1, c2)
char c1, c2;
{
  int difference;

  if (c1==c2) return 1;
  difference = 'A' - 'a';
  if ((islower(c1) && isupper(c2) &&  c2 - c1 == difference) ||
      (islower(c2) && isupper(c1) &&  c1 - c2 == difference))
    return 1;
  else return 0;
}




/* This function returns either a valid question word or a null string.
   A question word is returned if the sentence contains one of the valid
   question words for this record  */

char *question_word_grasp(sentence, list)
char *sentence;
struct response_lines *list;
{
  struct response_lines *point;
  char *get_first_word();
  char word[KEY_LENGTH], keep_phrase[LINE_LENGTH];
  int  found = 0;

  point = list;
  while (point && !found) {
    strcpy(word, get_first_word(point->line));
    found = phrase_grasp(sentence, word, keep_phrase);
    if (!found) point = point->next;
  }
  if (found) return word;
  else return "\0";
}


char *get_first_word(s)
char *s;
{
  int i = 0;
  char line[LINE_LENGTH];

  while (s[i] != '=')  i++;
  if (i==0) return "\0";
  strncpy(line, s, i);
  line[i] = '\0';
  return line;
}


char *get_last_words(s)
char *s;
{
  int i;
  char word[KEY_LENGTH], result[LINE_LENGTH];
  char *get_first_word();

  strcpy(word, get_first_word(s));
  i = strlen(word);
  while (s[i]=='=')  i++;
  strcpy(result, s+i);
  return result;
}




char *no_key_found_response(p)
struct personalities *p;
{
  char answer[LINE_LENGTH];

  strcpy(answer, "{no-key-found}");
  make_all_substitutions(p, answer);
  return answer;
}




char *answer_question(p, record, question_word, kept_phrase)
struct personalities *p;
struct keyword_records *record;
char *question_word, *kept_phrase;
{
  struct response_lines *point;
  int found = 0;
  static char answer[LINE_LENGTH];
  char *get_last_words(), *get_first_word();

  point = record->response_list;
  while (point && !found) {
    found = !strcmp(question_word, get_first_word(point->line));
    if (!found)  point = point->next;
  }
  if (found)  strcpy(answer, get_last_words(point->line));
  else strcpy(answer, no_key_found_response(p));

  make_all_substitutions(p, answer);
  fill_in_star(p, answer, kept_phrase);
  return answer;
}




char *respond_to_key(p, record, kept_phrase)
struct personalities *p;
struct keyword_records *record;
char *kept_phrase;
{
  int  select, max_used_already, tries = 0;
  struct response_lines *response, *getnresponse();
  static char answer[LINE_LENGTH];

  if (record->response_list==NULL)  return no_key_found_response(p);
  max_used_already = record->count / record->num_of_lines;

  do {
    select = int_rand(record->num_of_lines);
    response = getnresponse(record, select);
    tries++;
  } while ((response->count > max_used_already) && (tries<15));

  strcpy(answer, response->line);
  response->count++;
  make_all_substitutions(p, answer);
  fill_in_star(p, answer, kept_phrase);
  return answer;
}



struct response_lines *getnresponse(record, n)
struct keyword_records *record;
int n;
{
  struct response_lines *point;
  int i;

  point = record->response_list;
  for (i=1; i<n; i++) {
    if (point==NULL) {
      printf("===== Error: program tried to grab non-existant response\n");
      exit(1);
    }
    point = point->next;
  }
  return point;
}



fill_in_star(p, s, keep_phrase)
struct personalities *p;
char *s, *keep_phrase;
{
  char new_keeper[LINE_LENGTH], after_pronoun[LINE_LENGTH];
  char result[LINE_LENGTH];
  char *trade_pronouns(), *remove_punct(), *remove_double_spaces();
  int  i, j, k, length;

  strcpy(new_keeper, remove_punct(keep_phrase));
  strcpy(after_pronoun, trade_pronouns(p, new_keeper));
  if (debug_mode && strlen(after_pronoun)>0)
    printf("===== Altered fragment: %s\n", after_pronoun);
  if (strlen(s) + strlen(after_pronoun) > LINE_LENGTH)
    printf("===== Error: resulting answer line is too long\n");
  length = strlen(s);
  for (i=0,k=0; i<length; i++) {
    if (s[i]=='*')
      for (j=0; j<strlen(after_pronoun); j++)
        result[k++] = after_pronoun[j];
    else  result[k++] = s[i];
  }
  result[k] = '\0';
  strcpy(s, remove_double_spaces(result));
}


char *trade_pronouns(p, s)
struct personalities *p;
char *s;
{
  struct pronoun_subs *point;
  char result[LINE_LENGTH];

  point = p->pronoun_list;
  strcpy(result, s);
  while (point) {
    double_replace(result, point->s1, point->s2);
    point = point->next;
  }
  return result;
}


double_replace(s, word1, word2)
char *s, *word1, *word2;
{
  int  master, new, barrier, length, skip;
  char result[LINE_LENGTH];

  length = strlen(s);
  result[0] = '\0';
  for (master=new=0; master<length;) {
    barrier = (master==0);
    skip = word_check(s+master, word1, barrier);
    if (skip) {
      strcat(result, word2);
      new = strlen(result);
      master += skip;
    }
    else {
      skip = word_check(s+master, word2, barrier);
      if (skip) {
        strcat(result, word1);
	new = strlen(result);
	master += skip;
      }
      else {
        result[new++] =  s[master++];
	result[new]   =  '\0';
      }
    }
  }
  strcpy(s, result);
}


word_check(s, word, barrier)
char *s, *word;
int  barrier;
{
  int  master, place, ok, slen, wlen;

  slen = strlen(s);
  wlen = strlen(word);
  for (master=place=0,ok=1; (master<=slen && place<wlen && ok);) {
    ok = no_care_match(s[master], word[place]);
    if (ok) {
      master++;
      place++;
    } else if (master==0 && barrier && word[place]==' ') {
             ok = 1;
	     place++;
    } else if (master==slen && word[place]==' ') {
             ok = 1;
	     place++;
    } else if (s[master]=='\'') {
	  ok = 1;
	  master++;
    } else if (word[place]=='\'') {
	  ok = 1;
	  place++;
    }
  }
  if (!ok)  return 0;
  else      return master;
}



char *remove_punct(s)
char *s;
{
  int i, j;
  char result[LINE_LENGTH];

  for (i=0,j=0; i<strlen(s); i++)
    if (!ispunct(s[i]) || s[i]=='\'') result[j++] = s[i];

  result[j] = '\0';
  return result;
}


char *remove_double_spaces(s)
char *s;
{
  char result[LINE_LENGTH];
  int  i, j, length;

  length = strlen(s);
  for (i=0,j=0; i<length; i++)
    if (s[i]!=' ' || s[i+1]!=' ') result[j++] = s[i];

  result[j] = '\0';
  return result;
}


make_all_substitutions(p, s)
struct personalities *p;
char   *s;
{
  while (make_a_substitution(p, s));
}



/* This function returns TRUE only when some substitution has
   been successfully made.  It returns FALSE otherwise and does
   not modify the supplied string.    */

make_a_substitution(p, s)
struct personalities *p;
char   *s;
{
  char sub_name[KEY_LENGTH], sub_phrase[LINE_LENGTH];
  char result[LINE_LENGTH];
  char *read_sub_name(), *get_sub_phrase();
  int  location, skip, found, length;

  length = strlen(s);
  for (location=0,found=0; (location<length && !found);)
    if (s[location]=='{')  found = 1;
    else  location++;

  if (!found)  return 0;

  strcpy(sub_name, read_sub_name(s+location+1));
  strcpy(sub_phrase, get_sub_phrase(p, sub_name));

  if (location) strncpy(result, s, location);
  result[location] = '\0';
  strcat(result, sub_phrase);
  skip = strlen(sub_name) + 2;
  if (location+skip <= length) strcat(result, s+location+skip);
  strcpy(s, result);
  return 1;
}


char *read_sub_name(s)
char *s;
{
  int  i, length, done;
  char name[KEY_LENGTH];

  length = strlen(s);
  for (i=done=0; (i<length && !done);) {
    if (s[i]=='}')  done = 1;
    else {
      name[i] = s[i];
      i++;
    }
  }
  name[i] = '\0';
  if (!done) {
    printf("===== Error: No closing } in response line\n");
    printf("===== Attempted sub name: {%s\n", s);
    exit(1);
  }
  return name;
}



char *get_sub_phrase(p, name)
struct personalities *p;
char *name;
{
  struct keyword_records *point;
  struct keywords        *key;
  int found;
  char *pick_sub();

  point = p->sub_list;
  found = 0;
  while (point && !found) {
    key = point->keyword_list;
    while (key && !found) {
      if (!strcmp(key->word, name)) {
        found = 1;
	key->count++;
      } else  key = key->next;
    }
    if (!found)  point = point->next;
  }
  if (!found) {
    if (!strcmp(name, "no-key-found")) {
      printf("===== Error: sub key {no-key-found} is missing\n");
      exit(1);
    }
    if (debug_mode)
      printf("===== Warning: {%s} sub list is missing\n", name);
    return get_sub_phrase(p, "no-key-found");
  }
  else {
    point->count++;
    if (debug_mode) printf("===== Substitution: %s\n", name);
    return pick_sub(point);
  }
}



char *pick_sub(record)
struct keyword_records *record;
{
  int select, max_used_already, tries = 0;
  struct response_lines *response, *getnresponse();
  char answer[LINE_LENGTH];

  if (record->response_list==NULL) return "{no-key-found}";
  max_used_already = record->count / record->num_of_lines;

  do {
    select = int_rand(record->num_of_lines);
    response = getnresponse(record, select);
    tries++;
  } while ((response->count > max_used_already) && (tries<15));

  strcpy(answer, response->line);
  response->count++;
  return answer;
}
