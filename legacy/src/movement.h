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

/* *****
Location *block_movement_list(char x1, char y1, char x2, char y2);
Location *diag_movement_list(char x1, char y1, char x2, char y2, int overlap);
Location *movement_list(char x1, char y1, char x2, char y2);
Location *makelocation(char x, char y, Location *next);
MemObj	 *exit_char_on_square();
**** */

Location *block_movement_list();
Location *diag_movement_list();
Location *movement_list();
Location *makelocation();
ObjectInstance *exit_char_on_square();
