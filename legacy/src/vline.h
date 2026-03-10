/* Volvox Text Line function library - vline.h
 * Header file containing type definitions and procedure declarations
 * Author: Van A. Boughner
 * January 1991
 */

#include "vsys.h"


/* The structure used to store one line in a doubly linked list of 
   text lines.  The end of the list is signified by a NULL pointer in next. */

typedef struct _vline {
	char *line;
	struct _vline *prev, *next;
} VLine;



/* selectors for those who want them, their use is not mandatory, for
   the structure above is not going to be changed. */
#define previous_line(l)	((l)->prev)
#define next_line(l)		((l)->next)
#define line_text(l)		((l)->line)


#define line_token_by_number(l, n, rest, separators) \
		string_token_by_number((l)->line, n, rest, separators)
#define line_token_to_end(l, n, separators) \
		get_token_to_end((l)->line, n, separators)
#define print_lines(l, num) \
		fprintf_lines(stdout, l, num)
#define no_null(s) \
		((s) ? (s) : "")


/* Prototypes */
void block_out_trailing_newline();
void free_line();
void free_entire_list();
void fprintf_lines();
void remove_line();
void delete_line();
char *dupstr();
char *strdup();
char *trimstr();
char *separate_string_tokens();
char *string_token_by_number();
char *get_token_to_end();
VLine *make_line();
VLine *add_line();
VLine *dup_line();
VLine *seek_last();
void append_line_to_list();
void copy_and_append();
void transfer_and_append();
void insert_at_top();
VLine *read_stream();
VLine *seek_line_by_nth_token();
char *get_value_by_key();
VLine *next_blank_line();
int is_blank_line();
int just_blank_lines();
int write_list_to_stream();
