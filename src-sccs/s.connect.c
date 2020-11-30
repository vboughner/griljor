h16262
s 00000/00000/00325
d D 1.4 92/08/07 01:00:33 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00028/00010/00297
d D 1.3 91/08/26 00:33:07 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00002/00001/00305
d D 1.2 91/08/03 18:43:10 labc-3id 2 1
c converted read() and write() to read_from_descriptor(), etc.
e
s 00306/00000/00000
d D 1.1 91/02/16 12:53:45 labc-3id 1 0
c date and time created 91/02/16 12:53:45 by labc-3id
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

/* Stream connection routines for Griljor driver and new players */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include "config.h"
#include "connect.h"

#define CONNECT_DEBUG	0	/* is 1 when we want read/write byte counts */


/* Given a target address.  Return the socket
   number (and address by reference) or return -1 if no socket could be
   made. */

D 3
int make_listen_socket(Sockaddr *addr)
E 3
I 3
int make_listen_socket(addr)
Sockaddr *addr;
E 3
{
  int result;

  /* get a socket */
  result = make_socket_next_port(addr, SOCK_STREAM);

  /* start listening on that socket */
  if (result != -1) listen(result, 5);

  return result;
}



/* Given a target address.  Return the socket
   number (and address by reference) or return -1 if no socket could be
   made. */

D 3
int make_socket(Sockaddr *addr, int socktype)
E 3
I 3
int make_socket(addr, socktype)
Sockaddr *addr;
int socktype;
E 3
{
  int temp, err;
  
  if (DEBUG)
    printf("Making socket on address %d and port %d\n",
	   addr->sin_addr, addr->sin_port);

  /* create a stream socket */
  temp = socket(addr->sin_family, socktype, 0);
  if (temp < 0) return -1;

  /* bind socket to given address */
  err = bind(temp, (struct sockaddr *)addr, sizeof(*addr));
  if (err < 0)  return -1;

  /* return socket number */
  return temp;
}



/* Given a starting port address, look at succeeding ports until we find a
   free one to use for ourself, then bind a socket to the given address
   and return the socket number */

D 3
int make_socket_next_port(Sockaddr *addr, int socktype)
E 3
I 3
int make_socket_next_port(addr, socktype)
Sockaddr *addr;
int socktype;
E 3
{
  int port, return_socket;

  /* get starting port number */
  port = ntohs(addr->sin_port);

  /* look for an open port to take on this machine */
  for (;; port++) {

    /* try this one */
    addr->sin_port = htons(port);
    return_socket = make_socket(addr, socktype);

    if (return_socket < 0) {
          /* if the error is "socket in use", continue loop and try next one */
          if (errno != EADDRINUSE)  Berror("socket binding");
          else continue;
      }
      else break;
  }

  return return_socket;
}



/* Given an address, make a connection with the driver at that address and
   return a socket by which we can communicate with the driver.  Return
   -1 when we cannot make that connection. */

D 3
int make_driver_connection(Sockaddr *addr)
E 3
I 3
int make_driver_connection(addr)
Sockaddr *addr;
E 3
{
  int joinsock, err;
  
  if (DEBUG)
    printf("Making driver connection to address %d and port %d\n",
	   addr->sin_addr, addr->sin_port);

  /* get ourselves a socket to use */
  joinsock = socket(AF_INET, SOCK_STREAM, 0);
  if (joinsock < 0) Cerror("couldn't make connection socket");

  /* connect this socket to the driver */
  err = connect(joinsock, (struct sockaddr *)addr, sizeof(*addr));
  if (err < 0) {
    close(joinsock);
    return -1;	/* cannot connect with requested driver */
  }

  /* return the socket number to use */
  return joinsock;
}



/* Given a hostname and port number, return (by reference) the full
   inet address derived from that information.  Set address pointer to
   NULL if no host entry could be found for the host name given.  If we
   are given a NULL pointer for the hostname, then we will figure it
   out for ourself (get this machine's hostname and use it instead). */

D 3
resolve_inet_address(Sockaddr *addr, char *host, int port)
E 3
I 3
resolve_inet_address(addr, host, port)
Sockaddr *addr;
char *host;
int port;
E 3
{
  struct hostent	*hp;
  char			myhost[96];
  
  /* if we were given a null host pointer then find this machines hostname */
  if (host == NULL) {
    gethostname(myhost, 96);
    host = myhost;
  }

  /* look up the host entry */
  hp = gethostbyname(host);
  if (hp == NULL) {
    printf("Warning: no host entry found\n");
    return;
  }

  /* copy host address into socket address record */
  bzero((char *)addr, sizeof(*addr));
  bcopy(hp->h_addr, (char *)&(addr->sin_addr), hp->h_length);
  addr->sin_family = hp->h_addrtype;

  /* put in the port number */
  addr->sin_port = htons(port);
}



/* Given a socket that has been bound and set to listening mode, accept
   a connection on that socket and return a new socket bearing the 
   connection.  Return -1 when there is any kind of error. */

D 3
make_new_player_connection(int listensock)
E 3
I 3
make_new_player_connection(listensock)
int listensock;
E 3
{
  struct sockaddr from;
  int result, fromlen;
  
  if (DEBUG) printf("Serving a connection on socket %d\n", listensock);

  /* Wait for a connection request */
  fromlen = sizeof(from);
  result = accept(listensock, &from, &fromlen);
  if (result < 0) Cerror("problem accepting new connections");

  /* return the socket that connects us with new player */
  return result;
}



/* write something to a socket, and check to make sure that all was written */

D 3
write_to_socket(int socknum, char *stuff, int nbytes)
E 3
I 3
write_to_socket(socknum, stuff, nbytes)
int socknum;
char *stuff;
int nbytes;
E 3
{
  int	a, amount = 0, flags = 0;	/* amount written so far */
  char	*write_at;			/* point of next write */
  int	error_countdown = 50;		/* times we will try */

  /* set write_at pointer for the beginning of the data structure */
  write_at = stuff;

  /* write until we do it all */
  do {
  
    /* write to socket, note amount written */
    a = send(socknum, write_at, (nbytes - amount), flags);

    /* increment amount written and write_at pointer */
    if (a != -1) {
      amount += a;
      write_at += a;
    }

   /* if we haven't written it all yet then signal warning */
    if (amount != nbytes) {
      error_countdown--;
      if (CONNECT_DEBUG) {
        printf("WARNING: %d bytes written so far out of %d\n", amount, nbytes);
        printf("WARNING: error number was %d\n", errno);
      }
    }
    
  } while (amount < nbytes &&  error_countdown > 0);

  if (CONNECT_DEBUG)
    printf("%d bytes written when %d requested\n", amount, nbytes);
}



/* read something from a socket */

D 3
read_from_socket(int socknum, char *stuff, int nbytes)
E 3
I 3
read_from_socket(socknum, stuff, nbytes)
int socknum;
char *stuff;
int nbytes;
E 3
{
  int	a, amount = 0, flags = 0;	/* amount read so far */
  char	*read_at;			/* point of next read */
  int	error_countdown = 50;		/* times we will try */

  /* set read_at pointer for the beginning of the data structure */
  read_at = stuff;

  /* read until we get it all */
  do {
  
    /* read from socket, note amount read */
    a = recv(socknum, read_at, (nbytes - amount), flags);

    /* increment amount read and read_at pointer */
    if (a != -1) {
      amount += a;
      read_at += a;
    }

   /* if we haven't read it all yet then signal warning */
    if (amount != nbytes) {
      error_countdown--;
      if (CONNECT_DEBUG) {
        printf("WARNING: %d bytes read so far out of %d\n", amount, nbytes);
        printf("WARNING: error number was %d\n", errno);
      }
    }
    
  } while (amount < nbytes &&  error_countdown > 0);

  if (CONNECT_DEBUG)
    printf("%d bytes read when %d expected\n", amount, nbytes);
D 2
  
E 2
I 2

  return amount;
E 2
}




/* Check for anything to read on a certain socket, when there is something
   there, return TRUE, otherwise return FALSE, howlong is in millisec. */

D 3
int readable_on_socket(int socknum, int how_long)
E 3
I 3
int readable_on_socket(socknum, how_long)
int socknum, how_long;
E 3
{
  int			read_template;	/* tells which sockets to check */
  struct timeval	wait;		/* tells how long to wait */
  int			err;

  /* set wait time to seconds equal to how_long */
  wait.tv_sec = how_long / 1000;
  wait.tv_usec = how_long % 1000;

  /* clear file descriptor template and select this socket for querying */
  read_template = 0;
  read_template = (1<<socknum);

  /* find out if there is anything to read there */
  err = select(32, &read_template, NULL, NULL, &wait);

  /* if there was an error then report it */
  if (err < 0) Cerror("couldn't poll socket for readability");

  /* return whether this socket is readable */
  return (read_template != 0);
}




D 3
Cerror(char *msg)
E 3
I 3
Cerror(msg)
char *msg;
E 3
{
  char newmsg[200];

  sprintf(newmsg, "connection: %s", msg);
  
  Berror(newmsg);
}
E 1
