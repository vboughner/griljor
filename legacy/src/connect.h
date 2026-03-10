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
