/***************************************************************************
 * @(#) diagsave.c 1.3 - last change made 08/07/92
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

/* routines for storing dialog described structures to a file */

#include <stdio.h>
#include <errno.h>
#include "def.h"
#include "objects.h"
#define NOT_OBTOR
#include "dialog.h"
#include "vline.h"
#include "objheader.h"
#include "objinfo.h"


/* #define OBJ_DEBUG */


/* routines in this file work on assumption that chars are 8 bits */
#define BYTE_SIZE	((unsigned short) 256)

/* locally global buffer */
static char buf[STRING_BUFFER_SIZE];



/* ========================= W R I T I N G ============================ */


write_to_descriptor(socknum, stuff, nbytes)
/* write something to a file descriptor, and check to make sure that
   all of the buffer was written */
int socknum;
char *stuff;
int nbytes;
{
  int	a, amount = 0;			/* amount written so far */
  char	*write_at;			/* point of next write */
  int	error_countdown = 50;		/* times we will try */

  /* set write_at pointer for the beginning of the data structure */
  write_at = stuff;

  /* write until we do it all */
  do {
  
    /* we note amount written */
    a = write(socknum, write_at, (nbytes - amount));

    /* increment amount written and write_at pointer */
    if (a != -1) {
      amount += a;
      write_at += a;
    }

   /* if we haven't written it all yet then signal warning */
    if (amount != nbytes) {
      error_countdown--;
      if (DEBUG) {
        printf("WARNING: %d bytes written so far out of %d\n", amount, nbytes);
        printf("WARNING: error number was %d\n", errno);
      }
    }
    
  } while (amount < nbytes &&  error_countdown > 0);

  return 0;
}



void write_diag_to_fd(diag, o, fd)
DialogList *diag;
char *o;
int fd;
/* writes a single object description to a file device.  Object is described
   be giving the id and value of all flags that are not FALSE or NULL in
   the object's property flag list.  Identifier of zero signifies the
   end of the record.  High order byte is written to file first. */
{
  int i, j, len, value;
  char id[2], *tmp, **deref;
  DialogList *ptr;

  i = 0;
  do {
    len = -1;
    ptr = &(diag[i++]);
    tmp = (((char *) o) + ptr->offset);

    if (!(ptr->type & OLDDIAG)) { /* doesn't write out-dated properties */
      switch (ptr->type) {
        /* length of character stream dependent on variable type */
	case BOLBOX:
	  value = *tmp;
	  if (value) len = 0;
	  break;
	case INTBOX:
	case LINKPIC:
	  value = (short) *((short *) tmp);
	  if (value) {
	    buf[0] = (unsigned char) (abs(value) / BYTE_SIZE);
	    buf[1] = (unsigned char) (abs(value) % BYTE_SIZE);
	    if (value < 0) buf[0] = buf[0] | (BYTE_SIZE / 2);
	    len = 2;
	  }
	  break;
	case LONGINT:
	  value = (long) *((long *) tmp);
	  if (value) {
	    buf[0] =
	      (unsigned char) (abs(value) /
			       (BYTE_SIZE * BYTE_SIZE * BYTE_SIZE));
	    buf[1] =
	      (unsigned char) ((abs(value) /
				(BYTE_SIZE * BYTE_SIZE)) % BYTE_SIZE);
	    buf[2] =
	      (unsigned char) ((abs(value) /
				BYTE_SIZE) % BYTE_SIZE);
	    buf[3] =
	      (unsigned char) (abs(value) % BYTE_SIZE);
	    if (value < 0) buf[0] = buf[0] | (BYTE_SIZE / 2);
	    len = 4;
	  }
	  break;
	case STRBOX:
	case LSTRBOX:
	case HIDSTR:
	  deref = (char **) tmp;
	  if (*deref) {
	    if (strlen(*deref)) {
  	      strcpy(buf, *deref);
	      len = strlen(*deref) + 1;
	    }
	  }
	  break;
	case ICONPIC:
	  value = 0;
	  for (j=0; (j<BITMAP_ARRAY_SIZE && !value); j++)
	    if (tmp[j]) value = 1;
	  if (value) {
	    for (j=0; (j<BITMAP_ARRAY_SIZE); j++) buf[j] = tmp[j];
	    len = BITMAP_ARRAY_SIZE;
	  }
	  break;
	case ENDLIST:
	  id[0] = 0;
	  id[1] = 0;
	  len = 0;
	case NEWCOL:
	case BLANKBOX:
	case TITLEBOX:
	case QUITBOX:
	case MARKSHO:
	case MARKHID:
	default:
	  break;
      }
    }
    if (len != -1) {
      if (ptr->type != ENDLIST) {
	id[0] = (unsigned char) (ptr->id / BYTE_SIZE);
	id[1] = (unsigned char) (ptr->id % BYTE_SIZE);
      }
      write_to_descriptor(fd, id, 2);
      if (len > 0) write_to_descriptor(fd, buf, len);
#ifdef OBJ_DEBUG
      printf("Wrote out id number %d (%s)\n",
	     (unsigned char) id[0] * 256 + (unsigned char) id[1], ptr->name);
      if (len > 0) printf("Wrote out %d additional bytes\n", len);
#endif
    }
  } while (ptr->type != ENDLIST);
}



void write_end_of_section(fd)
/* writes out the end of section id marker */
int fd;
{
  char id[2];

  id[0] = (unsigned char) (END_OF_SECTION_ID / BYTE_SIZE);
  id[1] = (unsigned char) (END_OF_SECTION_ID % BYTE_SIZE);
  write_to_descriptor(fd, id, 2);
}



/* ===================== R E A D I N G ================================= */



static DialogList *find_dialog_desc(diag, id)
/* looks through dialog list for the record that describes a property
   flag.  The property flag is specified by it's id number.  If the
   id number is not found in the list, then NULL is returned.
   If there are two properties with the same id number, then a warning
   message is generated. */
DialogList *diag;
int id;
{
  static DialogList *origlist = NULL, *ptr = NULL;
  DialogList *found = NULL;

  if (!origlist || diag != origlist) {
    origlist = diag;
    ptr = diag;
  }

  if (ptr->type == ENDLIST) ptr = origlist;
  while (ptr->type != ENDLIST) {
    if (ptr->id == id) {
      if (!found)
	found = ptr;
      else
	fprintf(stderr, "Warning: multiple properties using id %d\n", id);
    }
    ptr++;
  }

  return(found);
}



read_from_descriptor(socknum, stuff, nbytes)
int socknum;
char *stuff;
int nbytes;
/* read something from a file descriptor */
{
  int	a, amount = 0;			/* amount read so far */
  char	*read_at;			/* point of next read */
  int	error_countdown = 50;		/* times we will try */

  /* set read_at pointer for the beginning of the data structure */
  read_at = stuff;

  /* read until we get it all */
  do {
  
    /* read, note amount read */
    a = read(socknum, read_at, (nbytes - amount));

    /* increment amount read and read_at pointer */
    if (a != -1) {
      amount += a;
      read_at += a;
    }

   /* if we haven't read it all yet then signal warning */
    if (amount != nbytes) {
      error_countdown--;
      if (DEBUG) {
        printf("WARNING: %d bytes read so far out of %d\n", amount, nbytes);
        printf("WARNING: error number was %d\n", errno);
      }
    }
    
  } while (amount < nbytes &&  error_countdown > 0);

  return amount;
}



int read_diag_from_fd(diag, new, fd)
/* reads from a device the character stream that represents an object
   property description.  Returns TRUE if a new object was read,
   returns NULL if the end of the file has been reached and no object was
   readable, or if the special end-of-section marker is read.
   Caller is responsible for providing space in 'new'. */
DialogList *diag;
char *new;
int fd;
{
  int i, j, len, value, neg;
  char id[2], *tmp, **deref;
  DialogList *ptr;

  /* read the id number of next object property flag in stream */
  len = read_from_descriptor(fd, id, 2);
  if (len != 2) return(NULL);

  /* check for the special id signifying end of a section */
  value = ((short) ((unsigned char)id[0]) * BYTE_SIZE) + 
    (short) ((unsigned char)id[1]);
  if (value == END_OF_SECTION_ID) return(NULL);


  /* read each flag and place its value into the object structure */
   while ((len == 2) && (id[0] | id[1])) {

    value = ((short) ((unsigned char)id[0]) * BYTE_SIZE) + 
	    (short) ((unsigned char)id[1]);
    if (value == END_OF_SECTION_ID) {
      fprintf(stderr, "Warning: non-sensical end-of-section marker\n");
      continue;
    }

    ptr = find_dialog_desc(diag, value);

    if (ptr) {

#ifdef OBJ_DEBUG
      printf("%s: ", ptr->name);
#endif

      if (ptr->type & OLDDIAG)
        tmp = buf;	/* info on out-dated props goes in throw-away buf */
      else 
        tmp = (((char *) new) + ptr->offset);	/* real location */

      switch (ptr->type) {
	case BOLBOX:
	case OLDBOLBOX:
	  *tmp = TRUE;
#ifdef OBJ_DEBUG
	  printf("True\n");
#endif
	  break;
	case INTBOX:
	case OLDINTBOX:
	case LINKPIC:
	case OLDLINKPIC:
          len = read_from_descriptor(fd, buf, 2);
	  if (len == 2) {
	    short *stmp = (short *) tmp;
	    neg = buf[0] & (BYTE_SIZE / 2);	/* look for negative bit */
	    if (neg) buf[0] = buf[0] & (BYTE_SIZE / 2 - 1);
	    *stmp = (short)((unsigned char)buf[0]) * BYTE_SIZE +
		    (short)((unsigned char)buf[1]);
	    if (neg) *stmp = - (*stmp);
#ifdef OBJ_DEBUG
	    printf("%d\n", *stmp);
#endif
	  }
	  else
	    printf("Warning: can't read 2 byte number\n");
	  break;
	case LONGINT:
          len = read_from_descriptor(fd, buf, 4);
	  if (len == 4) {
	    long *stmp = (long *) tmp;
	    neg = buf[0] & (BYTE_SIZE / 2);	/* look for negative bit */
	    if (neg) buf[0] = buf[0] & (BYTE_SIZE / 2 - 1);
	    *stmp = ((long)((unsigned char)buf[0]) * BYTE_SIZE *
		     BYTE_SIZE * BYTE_SIZE) +
	            ((long)((unsigned char)buf[1]) * BYTE_SIZE *
		     BYTE_SIZE) +
		    ((long)((unsigned char)buf[2]) * BYTE_SIZE) +
		    ((long)((unsigned char)buf[3]));
	    if (neg) *stmp = - (*stmp);
#ifdef OBJ_DEBUG
	    printf("%d\n", *stmp);
#endif
	  }
	  else
	    printf("Warning: can't read 4 byte number\n");
	  break;
	case STRBOX:
	case OLDSTRBOX:
	case LSTRBOX:
	case OLDLSTRBOX:
	case HIDSTR:
	case OLDHIDSTR:
	  i=0;
          do {
  	    len = read_from_descriptor(fd, &(buf[i]), 1);
	    i++;
	  } while ((len == 1) && (buf[i-1]));
          buf[i] = '\0';
          if (!(ptr->type & OLDDIAG)) {
  	    deref = (char **) tmp;
	    if (strlen(buf)) {
  	      demand((*deref = (char *) malloc(strlen(buf) + 1)),
		     "no memory for string copy");
	      strcpy(*deref, buf);
#ifdef OBJ_DEBUG
	      printf("%s\n", *deref);
#endif
	    }
	    else *deref = NULL;
	  }
	  break;
	case ICONPIC:
	case OLDICONPIC:
#ifdef OBJ_DEBUG
	    printf("bitmap");
#endif
	  len = read_from_descriptor(fd, buf, BITMAP_ARRAY_SIZE);
	  for (i=0; i<len; i++) tmp[i] = buf[i];
	  if (len != BITMAP_ARRAY_SIZE)
	    printf("Warning: bad num bytes (%d) in bitmap array\n", len);
	  break;
	case ENDLIST:
	case NEWCOL:
	case OLDNEWCOL:
	case BLANKBOX:
	case OLDBLANKBOX:
	case TITLEBOX:
	case OLDTITLEBOX:
	case QUITBOX:
	case OLDQUITBOX:
	case MARKSHO:
	case OLDMARKSHO:
	case MARKHID:
	case OLDMARKHID:
	default:
	  printf("Property of type %d had id of %d\n", ptr->type, value);
	  break;
      }
    }
    else printf("Warning: unknown property id %d in read file\n", value);

    len = read_from_descriptor(fd, id, 2);
  };
  return(TRUE);
}
