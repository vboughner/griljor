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

/* Header for generic library routines */

char *read_line(), *prompt_for_input(), *get_next_word(), *first_and_rest();
char *allocate_string(), *create_string(), *libbed_filename();
char *left_justify(), *left_justify_number();
char *string_time(), *twostud(), *limited_prompt(), *spaces();
char get_nth_bit(), getBit();
char *read_string_from_file();
FILE *fopen_locking();


#ifndef min
#define min(a, b)	(((a) > (b)) ? (b) : (a))
#define max(a, b)	(((a) < (b)) ? (b) : (a))
#endif /* min */
