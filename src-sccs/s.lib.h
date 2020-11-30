h33282
s 00000/00000/00026
d D 1.4 92/08/07 01:02:05 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00001/00000/00025
d D 1.3 91/11/29 16:09:24 labc-4lc 3 2
c finished making entry of hidden text properties in obtor
e
s 00002/00003/00023
d D 1.2 91/07/07 20:05:56 labc-3id 2 1
c added #ifndef around max and min definitions
e
s 00026/00000/00000
d D 1.1 91/02/16 13:00:51 labc-3id 1 0
c date and time created 91/02/16 13:00:51 by labc-3id
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

/* Header for generic library routines */

char *read_line(), *prompt_for_input(), *get_next_word(), *first_and_rest();
char *allocate_string(), *create_string(), *libbed_filename();
char *left_justify(), *left_justify_number();
char *string_time(), *twostud(), *limited_prompt(), *spaces();
char get_nth_bit(), getBit();
I 3
char *read_string_from_file();
E 3
FILE *fopen_locking();


D 2
/* find minimum */
E 2
I 2
#ifndef min
E 2
#define min(a, b)	(((a) > (b)) ? (b) : (a))
D 2

/* find maximum */
E 2
#define max(a, b)	(((a) < (b)) ? (b) : (a))
I 2
#endif /* min */
E 2
E 1
