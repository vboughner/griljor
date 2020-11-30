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

/* Order to plot things when drawing a room */

int plotx[ROOM_WIDTH * ROOM_HEIGHT] = {
10, 14, 12, 5, 2, 8, 11, 10, 11, 12, 18, 9, 2, 6, 15, 15, 7, 18, 7, 4, 15, 
6, 5, 19, 10, 16, 2, 0, 13, 16, 14, 14, 18, 15, 10, 15, 2, 5, 8, 0, 8, 
17, 5, 0, 8, 5, 6, 9, 10, 11, 14, 19, 2, 15, 17, 18, 12, 7, 1, 5, 14, 
10, 15, 9, 2, 16, 12, 12, 12, 9, 18, 7, 9, 5, 7, 1, 4, 8, 5, 8, 4, 
18, 9, 19, 3, 19, 2, 18, 19, 19, 2, 18, 3, 10, 6, 19, 4, 5, 13, 9, 11, 
17, 3, 3, 11, 9, 1, 11, 19, 3, 14, 17, 15, 5, 18, 18, 17, 17, 16, 7, 13, 
11, 8, 3, 9, 8, 13, 3, 1, 1, 3, 12, 8, 13, 7, 3, 0, 4, 15, 19, 16, 
8, 14, 7, 5, 0, 1, 13, 0, 15, 8, 18, 7, 14, 11, 11, 18, 7, 15, 16, 14, 
12, 13, 1, 17, 13, 1, 0, 10, 9, 12, 2, 18, 8, 12, 19, 2, 6, 10, 1, 3, 
17, 8, 3, 7, 3, 17, 1, 0, 1, 6, 2, 11, 14, 13, 1, 10, 7, 13, 10, 9, 
7, 8, 11, 18, 3, 4, 9, 6, 9, 7, 13, 0, 2, 2, 4, 0, 16, 6, 13, 4, 
17, 6, 5, 16, 0, 8, 19, 19, 16, 3, 12, 5, 15, 6, 9, 4, 13, 6, 18, 2, 
13, 6, 16, 8, 9, 1, 11, 4, 10, 1, 10, 5, 16, 8, 10, 16, 7, 10, 6, 7, 
0, 16, 13, 12, 1, 14, 18, 6, 7, 2, 4, 17, 14, 1, 11, 19, 9, 12, 16, 0, 
15, 4, 9, 2, 17, 1, 11, 11, 19, 5, 3, 5, 16, 15, 4, 15, 19, 18, 12, 12, 
15, 19, 12, 0, 17, 4, 9, 16, 18, 8, 3, 10, 14, 15, 14, 12, 13, 6, 16, 2, 
0, 10, 14, 11, 14, 4, 6, 1, 16, 14, 0, 17, 3, 13, 0, 9, 11, 3, 4, 8, 
6, 5, 19, 5, 17, 17, 15, 15, 7, 15, 17, 18, 1, 6, 18, 2, 19, 5, 0, 14, 
13, 13, 5, 11, 6, 14, 3, 12, 4, 11, 3, 0, 19, 14, 17, 9, 4, 17, 11, 10, 
0, 13, 12, 6, 1, 7, 17, 4, 8, 7, 10, 4, 16, 12, 2, 16, 10, 2, 19 };

int ploty[ROOM_WIDTH * ROOM_HEIGHT] = {
16, 13, 2, 14, 12, 3, 4, 14, 1, 5, 9, 1, 13, 0, 13, 2, 8, 16, 13, 4, 16, 
10, 6, 19, 13, 6, 2, 0, 7, 7, 6, 18, 2, 15, 11, 1, 5, 10, 4, 12, 9, 
1, 13, 15, 19, 7, 8, 17, 5, 9, 19, 18, 16, 9, 6, 6, 17, 18, 0, 16, 17, 
3, 7, 16, 7, 0, 19, 1, 15, 4, 13, 12, 8, 11, 14, 6, 17, 0, 0, 16, 3, 
0, 13, 4, 16, 12, 11, 10, 3, 14, 19, 7, 7, 8, 4, 13, 5, 8, 11, 0, 7, 
16, 4, 2, 12, 19, 13, 3, 16, 1, 0, 13, 10, 3, 19, 4, 14, 7, 13, 1, 6, 
0, 18, 11, 12, 1, 12, 15, 4, 2, 18, 9, 11, 17, 19, 19, 14, 11, 19, 17, 19, 
7, 2, 11, 5, 8, 17, 18, 17, 0, 12, 3, 4, 7, 15, 2, 18, 5, 6, 8, 15, 
10, 5, 14, 10, 8, 8, 18, 7, 7, 14, 17, 15, 17, 4, 2, 15, 3, 2, 18, 8, 
19, 10, 13, 15, 9, 11, 10, 10, 16, 19, 8, 10, 10, 13, 12, 1, 9, 3, 0, 11, 
16, 13, 17, 5, 6, 9, 9, 17, 14, 7, 4, 9, 1, 4, 15, 5, 1, 18, 9, 6, 
18, 7, 9, 12, 2, 14, 7, 9, 16, 5, 0, 19, 14, 1, 2, 7, 0, 16, 14, 3, 
16, 15, 17, 15, 5, 3, 19, 12, 18, 9, 9, 12, 14, 2, 10, 4, 3, 4, 14, 10, 
19, 2, 1, 16, 19, 3, 8, 5, 0, 9, 0, 5, 14, 7, 18, 15, 18, 6, 9, 16, 
3, 18, 3, 18, 2, 11, 5, 8, 5, 17, 12, 15, 11, 17, 2, 8, 1, 12, 3, 11, 
18, 0, 8, 3, 12, 19, 10, 10, 1, 5, 17, 6, 1, 12, 9, 7, 2, 13, 18, 6, 
7, 17, 11, 6, 8, 16, 2, 5, 15, 16, 13, 9, 14, 15, 11, 15, 11, 0, 1, 6, 
12, 1, 8, 18, 17, 4, 11, 5, 2, 4, 8, 11, 1, 6, 17, 14, 10, 4, 4, 4, 
10, 19, 2, 16, 11, 12, 3, 13, 8, 14, 10, 1, 6, 5, 3, 6, 10, 0, 13, 19, 
6, 14, 18, 9, 15, 6, 15, 13, 8, 17, 15, 14, 5, 12, 10, 3, 12, 0, 11 };
 
