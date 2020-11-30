h36625
s 00000/00000/00026
d D 1.4 92/08/07 01:02:31 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00008/00000/00018
d D 1.3 91/08/29 01:40:40 vanb 3 2
c fixed up more compatibility problems
e
s 00001/00001/00017
d D 1.2 91/05/26 22:43:47 labc-3id 2 1
c Worked on drawing improvements
e
s 00018/00000/00000
d D 1.1 91/02/16 13:01:00 labc-3id 1 0
c date and time created 91/02/16 13:01:00 by labc-3id
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

/* Procedure declarations for movement routines */

I 3
/* *****
E 3
Location *block_movement_list(char x1, char y1, char x2, char y2);
D 2
Location *diag_movement_list(char x1, char y1, char x2, char y2);
E 2
I 2
Location *diag_movement_list(char x1, char y1, char x2, char y2, int overlap);
E 2
Location *movement_list(char x1, char y1, char x2, char y2);
Location *makelocation(char x, char y, Location *next);
I 3
MemObj	 *exit_char_on_square();
**** */

Location *block_movement_list();
Location *diag_movement_list();
Location *movement_list();
Location *makelocation();
E 3
MemObj	 *exit_char_on_square();
E 1
