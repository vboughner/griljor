h59220
s 00000/00000/00020
d D 1.2 92/08/07 01:00:34 vbo 2 1
c source copied to a separate tree for work on new map and object format
e
s 00020/00000/00000
d D 1.1 91/02/16 13:00:20 labc-3id 1 0
c date and time created 91/02/16 13:00:20 by labc-3id
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

/* Header for stream connection routines for Griljor driver and new players */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* new data type definitions that connector uses */

typedef struct sockaddr_in	Sockaddr;
E 1
