/* Volvox Text Line function library - vline.c
 * Author: Van A. Boughner
 * January 1991
 * Purpose: This package manipulates strings and can form linked lists
 * of strings, most useful for representing lines of a text file.
 * The utility package vsys.c is needed to use this package.
 */

#include "vline.h"


/* ======================== String Functions ============================= */

/* given a string, allocate space in memory for the string, copy it into the
   new memory space and return a character pointer to the new duplicate copy.
   The old string remains unchanged.  An error will occur if no memory
   is available. */
char *dupstr(s)
char *s;
{
  char *result = NULL;

  if (s) {
    result = valloc(strlen(s) + 1);
    strcpy(result, s);
  }
  return result;
}


/* given a string, allocate space in memory for the string, copy it into the
   new memory space and return a character pointer to the new duplicate copy.
   The old string remains unchanged.  An error will occur if no memory
   is available. */
char *strdup(s)
char *s;
{
  char *result = NULL;

  if (s) {
    result = valloc(strlen(s) + 1);
    strcpy(result, s);
  }
  return result;
}


/* given a string, this procedure will search for the core part of the string
   that is without preceeding or trailing spaces (as defined by isspace()).
   It will allocate memory for a duplicate of that part of the string and
   return a pointer to it.  The caller is responsible for later freeing the
   string.  If the string pointer provided is NULL, then NULL is returned.
   In the case of s being an empty string, or there not being anything but
   spaces in it, NULL will be returned. */
char *trimstr(s)
char *s;
{
  char *result;
  int i, len, done;
  int first, last;	/* 'first' is first non-space index in the string,
			   'last' is the index of the first space (or NULL
			   terminator) that comes after last character. */

  if (!s) return NULL;
  len = strlen(s);
  for (i=0,done=0; (i<len && !done); i++)
    if (!isspace(s[i])) {
      first = i;
      done = 1;
    }

  if (!done) {
    /* the string was empty or all spaces */
    return NULL;
  }
  else {
    for (i=len-1,done=0; (i>=0 && !done); i--)
        if (!isspace(s[i])) {
        last = i + 1;
        done = 1;
      }
    result = valloc(last - first + 1);
    strncpy(result, (s + first), (last - first));
    result[(last - first)] = '\0';
    return result;
  }
}



/* look for a newline char at the end of a string, it there is one there,
   then replace it with a NULL terminator. */
void block_out_trailing_newline(s)
char *s;
{
  int len;
  demand(s, "block_out_trailing_newline: NULL string pointer passed in");

  len = strlen(s);
  if (s[len-1] == '\n') s[len-1] = '\0';
}



/* given a string, this routine will return the nth token in the string and
   also a pointer to the rest of the string after the token.  If the string
   given is not long enough, a NULL will be returned.  Spaces preceeding and
   trailing the token will be removed (possibly leaving an empty string if
   there were only spaces to begin with.  If an empty string is left after
   spaces are removed, then NULL will be returned).  The first token is
   number 0, an error will result if n is negative.  If 'rest' is passed as
   NULL, then no value will be returned through it.  By the way, the token
   returned will be a newly malloced duplicate that the caller should take
   responsiblity for freeing. If just_token is TRUE then only the token
   itself will be returned, if it is FALSE then the token and the rest
   of the line will all be returned as the result. */

char *string_separate_tokens(s, n, separators, just_token, rest)
char *s;
int n;
char *separators;
int just_token;
char **rest;
{
  int i;
  char *ptr, *last, *result, *trimmed;

  demand((n>=0), "error: n is negative in string_token_by_number()");
  if (!s) return NULL;

  /* look for separators until we find the right field */
  for (i=0,last=s; i<=n; i++) {
    ptr = strpbrk(last, separators);
    if (!ptr) {
      if (i < n) {
        if (rest) *rest = NULL;
        return NULL;	/* string not long enough to have an nth token */
      }
      else ptr = s + strlen(s);
    }
    if (i < n) last = ptr + 1;
  }

  if (ptr - last > 0) {
    if (just_token) {
      result = valloc(ptr - last + 1);
      strncpy(result, last, (ptr - last));
      result[(ptr - last)] = '\0';
    }
    else result = dupstr(last);
    trimmed = trimstr(result);
    free(result);
    if (rest) *rest = ptr;
    return trimmed;
  }
  else {
    if (rest) *rest = ptr;
    if (just_token) return NULL; /* token slot between separators was empty */
    else return trimstr(last);
  }
}



/* given a string, this routine will return the nth token in the string and
   also a pointer to the rest of the string after the token.  If the string
   given is not long enough, a NULL will be returned.  Spaces preceeding and
   trailing the token will be removed (possibly leaving an empty string if
   there were only spaces to begin with.  If an empty string is left after
   spaces are removed, then NULL will be returned).  The first token is
   number 0, an error will result if n is negative.  If 'rest' is passed as
   NULL, then no value will be returned through it.  By the way, the token
   returned will be a newly malloced duplicate that the caller should take
   responsiblity for freeing. */

char *string_token_by_number(s, n, rest, separators)
char *s;
int n;
char **rest;
char *separators;
{
  return string_separate_tokens(s, n, separators, TRUE, rest);
}



/* given a particular line of text, return not only the numbered token
   indicated, but the entire rest of the line following it regardless of any
   more separators being found.  For example, asking for token 0 would return
   the entire line, 1 would return all but the first token and separator.
   A token number of zero or a negative token number will result in an error.
   Giving a token number that is too large for the line will return NULL.
   An empty or NULL string will bring on a NULL result. */
char *get_token_to_end(s, number, separators)
char *s;
int number;
char *separators;
{
  return string_separate_tokens(s, number, separators, FALSE, NULL);
}



/* given the top of a list (or even some point in the middle), print a given
   number of lines in the list starting from that point.  If the number of
   lines requested is zero, then print all lines till the end.  If more
   lines are requested than exist in the list, then the rest of the list
   is printed.  If 'top' is NULL nothing is printed. */
void fprintf_lines(fp, top, num)
FILE *fp;
VLine *top;
int num;
{
  int i;
  VLine *ptr;

  if (!top) return;
  else ptr = top;

  for(i=0; ((i<num || num == 0) && ptr); i++) {
    if (ptr->line) fprintf(fp, "%s\n", ptr->line);
    else fprintf(fp, "\n");
    ptr = ptr->next;
  }
}



/* ======================== VLine Functions ============================= */

/* given a string, allocate space in memory for the string and also for a
   VLine structure, set the pointer in the structure to point to the new
   duplicate copy of the string, initialize the 'next' pointer to NULL
   and return a pointer to the new VLine structure.  An error
   will occur if there is insufficient memory. */
VLine *make_line(s)
char *s;
{
  VLine *result;

  result = (VLine *) valloc(sizeof(VLine));
  result->line = dupstr(s);
  result->prev = NULL;
  result->next = NULL;
  return result;
}



/* given the last line in a list of lines, and a string, add that string
   (encased in a VLine structure) to the end of the list after the given
   last line.  If the line pointer given is NULL, an error will occur.
   It returns a pointer to the new record which was added to the end. */
VLine *add_line(last, s)
VLine *last;
char *s;
{
  VLine *new;

  demand(last, "NULL given as last line in add_line()");
  new = make_line(s);
  last->next = new;
  new->prev = last;
  return new;
}



/* given a VLine structure, create a duplicate of it.  The caller is
   responsible for freeing the new copy allocated from memory.  The prev
   and next pointers of the new duplicate will both be initialized to NULL. */
VLine *dup_line(l)
VLine *l;
{
  return make_line(l->line);
}



/* free a VLine structure and the string within it */
void free_line(line)
VLine *line;
{
  if (line->line) free(line->line);
  free(line);
}



/* free an entire linked list of line structures, and all the strings within
   that follow the pointer given.  If the line pointer given is not the top
   of the linked list, this will effectively erase from that point to the
   end only.  To delete the entire list, use the pointer to the top.
   This function should not be used for routine manipulations unless the
   caller is willing to keep control of his own top and bottom of the
   list pointers. */
void free_list_to_end(top)
VLine *top;
{
  VLine *ptr, *last = NULL;

  if (!top) return;
  if (top->prev) top->prev->next = NULL;
  for (ptr=top; ptr; ptr = ptr->next) {
    if (last) free_line(last);
    last = ptr;
  }
  if (last) free_line(last);
}



/* given a pointer to a line, and access to the top of list and end of
   list pointers, this procedure deletes a line from the list.  If the
   line was at the top of the list, the top pointer will be changed for
   you.  If the line deleted was at the bottom of the list, the bottom
   pointer will be updated.  Should either top or bottom pointer reference
   be NULL, then there will be no updating for that one.  The line
   deleted in this fashion will not be freed.  It must be freed by
   the caller, using free_line() */
void remove_line(l, top, bottom)
VLine *l, **top, **bottom;
{
  if (l) {
    if (l->prev) l->prev->next = l->next;
    if (l->next) l->next->prev = l->prev;
    if (*top == l) *top = l->next;
    if (*bottom == l) *bottom = l->prev;
  }
}



/* this procedure is like the one above, except that it also frees the line
   that is removed. */
void delete_line(l, top, bottom)
VLine *l, **top, **bottom;
{
  if (l) {
    remove_line(l, top, bottom);
    free_line(l);
  }
}



/* this routine inserts a line at the beginning of a text list.  The top
   and bottom pointers are changed if neccessary.  The line given, needs
   to have been create some time previously with make_line() */
void insert_at_top(l, top, bottom)
VLine *l, **top, **bottom;
{
  demand(l, "insert_at_top: no line given to use in insertion");
  demand(top, "insert_at_top: pointer to top of list does not exist");
  demand(bottom, "insert_at_top: pointer to bottom of list does not exist");
  if (*top) {
    demand((*bottom), "insert_at_top: top pointer not NULL, but bottom was");
    (*top)->prev = l;
    l->next = (*top);
    l->prev = NULL;
    (*top) = l;
  }
  else {
    demand(!(*bottom), "insert_at_top: top pointer was NULL, bottom was not");
    *top = l;
    *bottom = l;
    l->next = NULL;
    l->prev = NULL;
  }
}



/* look for the last line in the text, and return a pointer to it,
   or return NULL if ptr is NULL.  The user should try to keep track of the
   end of the list himself while adding things to it, as this search is
   wasteful. */
VLine *seek_last(top)
VLine *top;
{
  VLine *ptr, *last = NULL;
  for (ptr=top; ptr; ptr = ptr->next) last = ptr;
  return last;
}



/* takes a line and appends it to a list, changing top and bottom pointers
   if neccessary.  If the line being appended was in some other list, then
   the responsibilty of making sure other list remains intact is caller's */
void append_line_to_list(line, top, bottom)
VLine *line, **top, **bottom;
{
  demand(line, "append_line_to_list: NULL line pointer given");
  if (*top) {
    demand((*bottom), "append_line_to_list: top had list, but bottom invalid");
    (*bottom)->next = line;
    line->prev = *bottom;
    line->next = NULL;
    *bottom = line;
  }
  else {
    *top = line;
    *bottom = line;
    line->prev = NULL;
    line->next = NULL;
  }
}



/* this routine takes the lines in the list between start and end (inclusive),
   makes copies of them and appends them to the list given */
void copy_and_append(start, end, first, last)
VLine *start, *end, **first, **last;
{
  VLine *ptr, *new;
  demand(start, "copy_and_append: start pointer was NULL, no list given");
  demand(end, "copy_and_append: end pointer was NULL, no end of list given");
  demand(first, "copy_and_append: first pointer was NULL, can't append");
  demand(last, "copy_and_append: last pointer was NULL, can't append");

  for (ptr=start; ptr; ptr=ptr->next) {
    new = dup_line(ptr);
    append_line_to_list(new, first, last);
    if (ptr == end) break;
  }
}



/* this routine takes the lines in the list between start and end (inclusive),
   and moves them to the end of the list given.  The previous list suffers
   orderly removal of these lines (unless you pass NULL's in these positions */
void transfer_and_append(prevtop, prevbot, start, end, first, last)
VLine **prevtop, **prevbot;	/* pointers to previous list */
VLine *start, *end, **first, **last;
{
  demand(start, "transfer_and_append: start pointer was NULL, no list given");
  demand(end, "transfer_and_append: end pointer was NULL, no end of list given");
  demand(first, "transfer_and_append: first pointer was NULL, can't append");
  demand(last, "transfer_and_append: last pointer was NULL, can't append");

  if (start->prev) start->prev->next = end->next;
  if (end->next) end->next->prev = start->prev;
  if (*prevtop == start) *prevtop = end->next;
  if (*prevbot == end) *prevbot = start->prev;
  if (*first) {
    (*last)->next = start;
    start->prev = *last;
    end->next = NULL;
    *last = end;
  }
  else {
    *first = start;
    start->prev = NULL;
    *last = end;
    end->next = NULL;
  }
}




/* this routine writes the contents of a line list to an open stream.
   The caller is responsible for opening and closing the stream himself.
   TRUE is returned when all went well, FALSE is returned if there were
   any errors. */
int write_list_to_stream(list, fp)
VLine *list;
FILE *fp;
{
  int status = !(EOF);
  VLine *ptr;

  for(ptr=list; (ptr && (status != EOF)); ptr=ptr->next) {
    if (ptr->line) status = fputs(ptr->line, fp);
    if (status != EOF) putc('\n', fp);
  }
  return (status != EOF);
}



/* given a file pointer to an open stream, read from it into a linked
   list of lines we create along the way until the stream is exhausted.
   NULL is returned if the file pointer was already at EOF. */
VLine *read_stream(fp)
FILE *fp;
{
  char *in, buffer[2048];
  VLine *top = NULL, *ptr;

  demand(fp, "read_stream: file pointer was NULL");
  while (in = fgets(buffer, 2048, fp)) {
    block_out_trailing_newline(in);
    if (top) ptr = add_line(ptr, in);
    else {
      top = make_line(in);
      ptr = top;
    }
  }
  return top;
}



/* look for the next line in the text that contains the given token as the
   nth token in the line.  Use the given separators to find tokens.
   Return NULL if no such line is found. */
VLine *seek_line_by_nth_token(top, token, n, separators)
VLine *top;
char *token;
int n;
char *separators;
{
  char *item;
  VLine *ptr, *loc = NULL;

  for (ptr = top;  (ptr && !loc);  ptr = ptr->next) {
    item = string_token_by_number(ptr->line, n, NULL, separators);
    if (item) 
      if (!strcmp(item, token)) loc = ptr;
    free(item);
  }

  return loc;
}



/* find the next line in the text that has a particular token as the first
   token in the line and return the "value" for that token, ie. the 2nd
   token and everything that follows it.  NULL will be returned if there
   is no matching first token found. */
char *get_value_by_key(top, token, separators)
VLine *top;
char *token, *separators;
{
  VLine *ptr;
  char *result = NULL;

  ptr = seek_line_by_nth_token(top, token, 0, separators);
  if (ptr) result = get_token_to_end(ptr->line, 1, separators);

  return result;
}



int is_blank_line(l)
VLine *l;
{
  demand(l, "is_blank_line: line pointer was NULL, can't check");

  if (l->line) {
    if (!strlen(l->line)) return TRUE;
    else return FALSE;
  }
  else return TRUE;
}



/* return a pointer to the next blank line we can find in the given list.
   An error will occur if no list pointer is given.  NULL will be returned
   if there are no blank lines in the text. */
VLine *next_blank_line(l)
VLine *l;
{
  VLine *ptr, *found = NULL;
  demand(l, "next_blank_line: list pointer is NULL, can't look for blanks");

  for (ptr=l; (ptr && !found); ptr=ptr->next)
    if (is_blank_line(ptr))
      found = ptr;

  return found;
}



/* looks at each of the lines between start and end (inclusive) and returns
    TRUE if they are all blank lines (NULL string ptr or empty string) */
int just_blank_lines(start, end)
VLine *start, *end;
{
  int result = TRUE;
  VLine *ptr;
  demand(start, "just_blank_lines: start of lines not defined, was NULL");
  demand(end, "just_blank_lines: end of lines not defined, was NULL");

  for (ptr=start; (ptr && result); ptr=ptr->next) {
    if (!is_blank_line(ptr)) result = FALSE;
    if (ptr == end) break;
  }
  demand((!result || ptr==end), 
	 "just_blank_lines: end signified was never found");
  return result;
}
