h09346
s 00000/00000/00310
d D 1.17 92/08/07 21:47:14 vbo 17 16
c fixes for sun port merged in
e
s 00016/00318/00294
d D 1.16 92/02/20 22:04:56 vanb 16 15
c 
e
s 00012/00005/00600
d D 1.15 91/12/13 20:46:12 labc-4lc 15 14
c added capability to spot multiple properties with same id
e
s 00036/00013/00569
d D 1.14 91/12/13 19:33:50 labc-4lc 14 13
c fixed bug that made a completely blank object unreadable and made it possible to move object properties into an old/outdated state
e
s 00000/00004/00582
d D 1.13 91/12/07 14:18:06 labc-4lc 13 12
c moved a macro constant from objstore.c to def.h
e
s 00015/00006/00571
d D 1.12 91/12/03 17:29:54 labc-4lc 12 11
c implemented header items for object def files
e
s 00024/00007/00553
d D 1.11 91/11/27 13:41:40 labc-4lc 11 10
c fixed string in objects so space is malloced, not static
e
s 00001/00000/00559
d D 1.10 91/10/16 20:05:57 labc-4lc 10 9
c changed made for DecStations
e
s 00001/00006/00558
d D 1.9 91/08/28 01:54:10 vanb 9 8
c removed annoying debugging message from reader and writer routines
e
s 00015/00005/00549
d D 1.8 91/08/26 00:33:55 vanb 8 7
c fixed up procedure defs and other compatibilty problems
e
s 00096/00015/00458
d D 1.7 91/08/03 18:43:14 labc-3id 7 6
c converted read() and write() to read_from_descriptor(), etc.
e
s 00001/00001/00472
d D 1.6 91/07/15 01:44:00 labc-3id 6 5
c fixed problems with procedure call parameters in passing
e
s 00038/00027/00435
d D 1.5 91/07/14 14:01:27 labc-3id 5 4
c Added 'ObjDiag.t' code.
e
s 00003/00006/00459
d D 1.4 91/07/07 18:46:06 labc-3id 4 3
c took out message number zero warning line from file saving
e
s 00357/00014/00108
d D 1.3 91/07/07 17:22:41 labc-3id 3 2
c added/changed some flags, fixed bugs in definition file save
e
s 00020/00001/00102
d D 1.2 91/06/30 20:27:32 labc-3id 2 1
c fixed up editing order of properties for obtor
e
s 00103/00000/00000
d D 1.1 91/06/30 18:54:26 labc-3id 1 0
c date and time created 91/06/30 18:54:26 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
D 9
 * %Z% %M% %I% - last change made %G%
E 9
I 9
 * @(#) objstore.c 1.8 - last change made 08/26/91
E 9
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

D 3
/* routines for loading and storing of object files in the new format */
E 3
I 3
/* Routines for loading and storing of object files in the new format.
   The ability to load and store object files by means of a file
   descriptor is provided here in order to use this for socket pipes.
   A couple of convenience functions for disk files are here as well.
   Object files should not be loaded or stored using any other functions
   than these, in order to retain a standard data file structure. */
E 3

#include <stdio.h>
I 7
#include <errno.h>
E 7
#include "def.h"
#include "objects.h"
#define NOT_OBTOR
#include "dialog.h"
D 5
#include "ObjDiag.h"
E 5
I 3
#include "vline.h"
I 5
#include "objheader.h"
I 12
#include "objinfo.h"
E 12
E 5
E 3

I 12

E 12
I 5
/* #define OBJ_DEBUG */
E 5
I 3

I 16
/* prototypes */
DialogList *StaticObjectProperties();
E 16
I 5

I 11
D 13
/* buffer size of maximum size string that can be written out or read in
   for a textual property in an object */
#define STRING_BUFFER_SIZE	20000

E 13
E 11
E 5
E 3
D 16
/* routines in this file work on assumption that chars are 8 bits */
D 3
#define BYTE_SIZE	256
E 3
I 3
#define BYTE_SIZE	((unsigned short) 256)
E 16
E 3

I 3
D 16
/* locally global variable that points to top of dialog record search tree */
static DialogList *dialogdesc = NULL;
E 16
E 3

I 11
D 16
/* locally global buffer */
static char buf[STRING_BUFFER_SIZE];
E 11
I 3
D 5
/* #define OBJ_DEBUG */
E 5
E 3

D 2
int write_obj_to_fd(ObjInfo *o, int fd)
E 2
I 2
D 3
void write_obj_to_fd(ObjInfo *o, int fd)
E 3
I 3

E 16
I 5
/* ========================= W R I T I N G ============================ */
E 5

I 5

I 7
D 8
write_to_descriptor(int socknum, char *stuff, int nbytes)
E 8
I 8
D 16
write_to_descriptor(socknum, stuff, nbytes)
int socknum;
char *stuff;
int nbytes;
E 8
/* write something to a file descriptor, and check to make sure that
   all of the buffer was written */
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

D 9
  if (DEBUG)
    printf("%d bytes written when %d requested\n", amount, nbytes);
E 9
  return 0;
}



E 16
E 7
E 5
D 8
static void write_obj_to_fd(ObjInfo *o, int fd)
E 8
I 8
static void write_obj_to_fd(o, fd)
ObjInfo *o;
int fd;
E 8
E 3
E 2
/* writes a single object description to a file device.  Object is described
   be giving the id and value of all flags that are not FALSE or NULL in
   the object's property flag list.  Identifier of zero signifies the
   end of the record.  High order byte is written to file first. */
{
I 5
D 8
#include "ObjDiag.t"
E 8
I 8
D 16
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 8
E 5
  int i, j, len, value;
D 11
  char id[2], buf[BITMAP_ARRAY_SIZE + 10], *tmp;
E 11
I 11
  char id[2], *tmp, **deref;
E 11
  DialogList *ptr;

  i = 0;
  do {
    len = -1;
    ptr = &(ObjectData[i++]);
D 3
    id[0] = (ptr->id / BYTE_SIZE);
    id[1] = (ptr->id % BYTE_SIZE);
E 3
I 3
    id[0] = (unsigned char) (ptr->id / BYTE_SIZE);
    id[1] = (unsigned char) (ptr->id % BYTE_SIZE);
E 3
    tmp = (((char *) o) + ptr->offset);

D 14
    switch (ptr->type) {
E 14
I 14
    if (!(ptr->type & OLDDIAG)) { /* doesn't write out-dated properties */
      switch (ptr->type) {
        /* length of character stream dependent on variable type */
E 14
	case BOLBOX:
	  value = *tmp;
	  if (value) len = 0;
	  break;
	case INTBOX:
	case LINKPIC:
	  value = (short) *((short *) tmp);
	  if (value) {
D 3
	    buf[0] = (value / BYTE_SIZE) % BYTE_SIZE;
	    buf[1] = (value % BYTE_SIZE);
E 3
I 3
	    buf[0] = (unsigned char) (value / BYTE_SIZE);
	    buf[1] = (unsigned char) (value % BYTE_SIZE);
E 3
	    len = 2;
	  }
	  break;
	case STRBOX:
	case LSTRBOX:
D 11
	  if (strlen(tmp)) {
	    strcpy(buf, tmp);
D 3
	    len = strlen(tmp);
E 3
I 3
	    len = strlen(tmp) + 1;
E 11
I 11
	case HIDSTR:
	  deref = (char **) tmp;
	  if (*deref) {
	    if (strlen(*deref)) {
  	      strcpy(buf, *deref);
	      len = strlen(*deref) + 1;
	    }
E 11
E 3
	  }
	  break;
	case ICONPIC:
	  value = 0;
	  for (j=0; (j<BITMAP_ARRAY_SIZE && !value); j++)
D 3
	    if (tmp[j]) value= 1;
E 3
I 3
	    if (tmp[j]) value = 1;
E 3
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
I 14
      }
E 14
    }
D 14

E 14
    if (len != -1) {
D 7
      write(fd, id, 2);
      if (len > 0) write(fd, buf, len);
E 7
I 7
      write_to_descriptor(fd, id, 2);
      if (len > 0) write_to_descriptor(fd, buf, len);
E 7
I 3
#ifdef OBJ_DEBUG
      printf("Wrote out id number %d (%s)\n",
	     (unsigned char) id[0] * 256 + (unsigned char) id[1], ptr->name);
      if (len > 0) printf("Wrote out %d additional bytes\n", len);
#endif
E 3
    }
  } while (ptr->type != ENDLIST);
E 16
I 16
  static DialogList *ObjectData = NULL;
  if (!ObjectData) ObjectData = StaticObjectProperties();
  write_diag_to_fd(ObjectData, o, fd);
E 16
}
I 2



D 3
int read_obj_from_fd(ObjInfo *o, int fd)
E 3
I 3
D 5
void write_messages_to_fd(fd, msgs)
E 5
I 5
static void write_messages_to_fd(fd, msgs, count)
E 5
/* writes out all the messages to the file descriptor given.  This should
D 5
   be called before write_objects_to_fd().  If msgs is NULL, then only
   the end of messages line will be written out. */
E 5
I 5
   be called by write_objects_to_fd().  If msgs is NULL, then only
   a bare minimum of header information and the end of messages
   line will be written out.  Messages are descriptions of the various
   objects and so forth.  The message text lines also contain important
   pieces of information about the object file as well. */
E 5
int fd;
VLine *msgs;
I 5
int count;	/* required so it can be placed in header */
E 5
{
  char s[200];
  VLine *ptr;

I 5
  msgs = make_count_accurate(msgs, count);

E 5
D 4
  sprintf(s, "%s%d\n", MESSAGE_NUMBER_PRECURSOR, 0);
  write(fd, s, strlen(s));
  sprintf(s, "You shouldn't use message number zero.\n");
  write(fd, s, strlen(s));

  for (ptr = msgs;  ptr;  ptr = next_line(ptr))
E 4
I 4
  for (ptr = msgs;  ptr;  ptr = next_line(ptr)) {
E 4
D 7
    write(fd, line_text(ptr), strlen(line_text(ptr)));
I 4
    write(fd, "\n", 1);
E 7
I 7
    write_to_descriptor(fd, line_text(ptr), strlen(line_text(ptr)));
    write_to_descriptor(fd, "\n", 1);
E 7
  }
E 4

  sprintf(s, "%s\n", END_OF_MESSAGES);
D 7
  write(fd, s, strlen(s));  
E 7
I 7
  write_to_descriptor(fd, s, strlen(s));  
E 7
}



D 5
void write_objects_to_fd(fd, o, count)
E 5
I 5
D 12
void write_objects_to_fd(fd, msgs, o, count)
E 12
I 12
void write_objects_to_fd(fd, header, msgs, o, count)
E 12
E 5
/* writes out an entire object set to a file descriptor, object by object.
   Count needs to be set to be the number of objects to write out. */
int fd;
I 12
ObjHeader *header;
E 12
I 5
VLine *msgs;
E 5
ObjInfo **o;
int count;
{
  int i;

I 12
  insert_header_into_messages(header, &msgs);
E 12
D 5
  for (i=0; i<count; i++)
    write_obj_to_fd(o[i], fd);
E 5
I 5
  write_messages_to_fd(fd, msgs, count);
I 12

E 12
  for (i=0; i<count; i++) write_obj_to_fd(o[i], fd);
E 5
}



D 12
int write_objects_to_file(filename, msgs, o, count)
E 12
I 12
int write_objects_to_file(filename, header, msgs, o, count)
E 12
/* writes out an entire object set to a particular filename.  Returns
   TRUE if the file was written, FALSE if file could not be opened
   for writing. */
char *filename;
I 12
ObjHeader *header;
E 12
VLine *msgs;
ObjInfo **o;
int count;
{
  FILE *fp;

  fp = fopen(filename, "w");
  if (!fp) return FALSE;

D 5
  write_messages_to_fd(fileno(fp), msgs);
  write_objects_to_fd(fileno(fp), o, count);
E 5
I 5
D 12
  write_objects_to_fd(fileno(fp), msgs, o, count);
E 12
I 12
  write_objects_to_fd(fileno(fp), header, msgs, o, count);
E 12
E 5
  fclose(fp);

  return TRUE;
}



I 5
/* ===================== R E A D I N G ================================= */


E 5
D 16
static void init_dialog_find(ptr)
/* initializes the finding structure for the purposes of doing efficient
   searches in find_dialog_desc().  Requires a pointer to the statically
   allocated dialog record list that will be used in the searches.
   The setup stuff will only be done the first time this is called.  */
DialogList *ptr;
{
  /* for now, does nothing, current search is inefficient linear scan */
  if (!dialogdesc) dialogdesc = ptr;
}



static DialogList *find_dialog_desc(id)
/* looks through dialog list for the record that describes a property
   flag.  The property flag is specified by it's id number.  If the
D 15
   id number is not found in the list, then NULL is returned. */
E 15
I 15
   id number is not found in the list, then NULL is returned.
   If there are two properties with the same id number, then a warning
   message is generated. */
E 15
int id;
{
  DialogList *ptr, *found = NULL;

  if (dialogdesc) {
    ptr = dialogdesc;
D 15
    while ((ptr->type != ENDLIST) && !found)
      if (ptr->id == id) found = ptr;
      else ptr++;
E 15
I 15
    while (ptr->type != ENDLIST) {
      if (ptr->id == id) {
	if (!found)
	  found = ptr;
	else
	  fprintf(stderr, "Warning: multiple properties using id %d\n", id);
      }
      ptr++;
    }
E 15
  }
D 15

E 15
  return found;
}



I 7
D 8
read_from_descriptor(int socknum, char *stuff, int nbytes)
E 8
I 8
read_from_descriptor(socknum, stuff, nbytes)
int socknum;
char *stuff;
int nbytes;
E 8
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
D 9

  if (DEBUG)
    printf("%d bytes read when %d expected\n", amount, nbytes);
E 9

  return amount;
}



E 16
E 7
static ObjInfo *read_obj_from_fd(fd)
E 3
/* reads from a device the character stream that represents an object
D 3
   property description.  Clears the object structure passed to it via
   the pointer and places the flags in the stream into the object record.
   Returns TRUE if an object was read, returns FALSE if end of file reached */
E 3
I 3
   property description.  Returns the pointer to a newly allocated
   object structure if an object was read, returns NULL if the end of
   the file has been reached and no object was readable.
   The caller is responsible for freeing any object record returned. */
int fd;
E 3
{
I 5
D 8
#include "ObjDiag.t"
E 8
I 8
D 16
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 8
E 5
D 3
  i, j, len, value;
E 3
I 3
  ObjInfo *new = NULL;
  int i, j, len, value;
E 3
D 11
  char id[2], buf[BITMAP_ARRAY_SIZE + 10], *tmp;
E 11
I 11
  char id[2], *tmp, **deref;
E 11
  DialogList *ptr;
E 16
I 16
  ObjInfo *new;
  static DialogList *ObjectData = NULL;
  if (!ObjectData) ObjectData = StaticObjectProperties();
E 16

D 16
  /* read the id number of next object property flag in stream */
I 3
D 7
  len = read(fd, id, 2);
E 7
I 7
  len = read_from_descriptor(fd, id, 2);
E 7
  if (len != 2) return NULL;
E 3

I 3
  /* allocate and clear a new object record */
E 16
  new = allocate_obj();
  initialize_obj_record(new);
D 16
  init_dialog_find(ObjectData);
E 16
E 3

I 3
D 16
  /* read each flag and place its value into the object structure */
D 14
  do {
E 14
I 14
   while ((len == 2) && (id[0] | id[1])) {
E 14
    value = ((short) ((unsigned char)id[0]) * BYTE_SIZE) + 
	    (short) ((unsigned char)id[1]);
    ptr = find_dialog_desc(value);

    if (ptr) {
I 14

E 14
#ifdef OBJ_DEBUG
      printf("%s: ", ptr->name);
#endif
D 14
      tmp = (((char *) new) + ptr->offset);
E 14
I 14

      if (ptr->type & OLDDIAG)
        tmp = buf;	/* info on out-dated props goes in throw-away buf */
      else 
        tmp = (((char *) new) + ptr->offset);	/* real location */

E 14
      switch (ptr->type) {
	case BOLBOX:
I 14
	case OLDBOLBOX:
E 14
	  *tmp = TRUE;
#ifdef OBJ_DEBUG
	  printf("True\n");
#endif
	  break;
	case INTBOX:
I 14
	case OLDINTBOX:
E 14
	case LINKPIC:
I 14
	case OLDLINKPIC:
E 14
D 7
          len = read(fd, buf, 2);
E 7
I 7
          len = read_from_descriptor(fd, buf, 2);
E 7
	  if (len == 2) {
	    short *stmp = (short *) tmp;
	    *stmp = (short)((unsigned char)buf[0]) * BYTE_SIZE +
		    (short)((unsigned char)buf[1]);
#ifdef OBJ_DEBUG
	    printf("%d\n", *stmp);
#endif
	  }
	  else
	    printf("Warning: can't read 2 byte number\n");
	  break;
	case STRBOX:
I 14
	case OLDSTRBOX:
E 14
	case LSTRBOX:
I 14
	case OLDLSTRBOX:
E 14
I 11
	case HIDSTR:
I 14
	case OLDHIDSTR:
E 14
E 11
	  i=0;
          do {
D 7
  	    len = read(fd, &(buf[i]), 1);
E 7
I 7
  	    len = read_from_descriptor(fd, &(buf[i]), 1);
E 7
	    i++;
	  } while ((len == 1) && (buf[i-1]));
          buf[i] = '\0';
D 11
	  strcpy(tmp, buf);
E 11
I 11
D 14
	  deref = (char **) tmp;
	  if (strlen(buf)) {
  	    demand((*deref = (char *) malloc(strlen(buf) + 1)),
		   "no memory for string copy");
	    strcpy(*deref, buf);
E 14
I 14
          if (!(ptr->type & OLDDIAG)) {
  	    deref = (char **) tmp;
	    if (strlen(buf)) {
  	      demand((*deref = (char *) malloc(strlen(buf) + 1)),
		     "no memory for string copy");
	      strcpy(*deref, buf);
E 14
E 11
#ifdef OBJ_DEBUG
D 11
	  printf("%s\n", tmp);
E 11
I 11
D 14
	    printf("%s\n", *deref);
E 14
I 14
	      printf("%s\n", *deref);
E 14
E 11
#endif
I 14
	    }
	    else *deref = NULL;
E 14
I 11
	  }
D 14
	  else *deref = NULL;
E 14
E 11
	  break;
	case ICONPIC:
I 14
	case OLDICONPIC:
E 14
D 7
	  len = read(fd, buf, BITMAP_ARRAY_SIZE);
	  if (len == BITMAP_ARRAY_SIZE) {
	    for (i=0; i<BITMAP_ARRAY_SIZE; i++) tmp[i] = buf[i];
E 7
#ifdef OBJ_DEBUG
	    printf("bitmap");
#endif
D 7
	  }
	  else
E 7
I 7
	  len = read_from_descriptor(fd, buf, BITMAP_ARRAY_SIZE);
	  for (i=0; i<len; i++) tmp[i] = buf[i];
	  if (len != BITMAP_ARRAY_SIZE)
E 7
	    printf("Warning: bad num bytes (%d) in bitmap array\n", len);
	  break;
	case ENDLIST:
	case NEWCOL:
I 14
	case OLDNEWCOL:
E 14
	case BLANKBOX:
I 14
	case OLDBLANKBOX:
E 14
	case TITLEBOX:
I 14
	case OLDTITLEBOX:
E 14
	case QUITBOX:
I 14
	case OLDQUITBOX:
E 14
	case MARKSHO:
I 14
	case OLDMARKSHO:
E 14
	case MARKHID:
I 14
	case OLDMARKHID:
E 14
	default:
	  printf("Property of type %d had id of %d\n", ptr->type, value);
	  break;
      }
    }
D 14
    else printf("Warning: unknow property id %d in read file\n", value);
E 14
I 14
    else printf("Warning: unknown property id %d in read file\n", value);
E 14

D 7
    len = read(fd, id, 2);
E 7
I 7
    len = read_from_descriptor(fd, id, 2);
E 7
D 14
  } while ((len == 2) && (id[0] | id[1]));
E 14
I 14
  };
E 14

  return new;
E 16
I 16
  if (read_diag_from_fd(ObjectData, new, fd)) {
    return(new);
  }
  else {
    free(new);
    return(NULL);
  }
E 16
E 3
}

D 16
   
E 16
I 16

E 16
I 3

static char *read_to_newline(fd)
/* reads up to and including the next newline character.  Returns a newly
   allocated string that doesn't include the the newline character.
   returns NULL if there is nothing to read from the file descriptor */
int fd;
{
  int i, len, size;
  char c, *s;

  i = 0;
  size = 50;
  s = (char *) malloc(sizeof(char) * size);
  demand(s, "read_to_newline: out of memory");
  do {
D 7
    len = read(fd, &c, 1);
E 7
I 7
    len = read_from_descriptor(fd, &c, 1);
E 7
    if (len == 1) {
      if (i >= size) {
	size += 50;
        s = (char *) realloc(s, sizeof(char) * size);
	demand(s, "read_to_newline: can't realloc");
      }
      if (c != '\n') {
	s[i] = c;
	i++;
      }
    }
  } while ((len == 1) && (c != '\n'));

  if (len == 1)
    s[i] = '\0';
  else {
    free(s);
    s = NULL;
  }
  return s;
}



D 5
VLine *read_messages_from_fd(fd)
E 5
I 5
static VLine *read_messages_from_fd(fd)
E 5
/* reads in all the messages from the file descriptor given.  This should
D 5
   be called before read_objects_from_fd().  If return value is NULL, it
E 5
I 5
   be called by read_objects_from_fd().  If return value is NULL, it
E 5
   means there are no messages in the object file. */
int fd;
{
  int done = FALSE;
  char *nextline;
  VLine *top = NULL, *bottom = NULL, *new;

  do {
    nextline = read_to_newline(fd);
    if (nextline) {
      if (strcmp(nextline, END_OF_MESSAGES)) {
	new = make_line(nextline);
	append_line_to_list(new, &top, &bottom);
	free(nextline);
      }
      else done = TRUE;
    }
    else done = TRUE;
  } while (!done);
  
  return top;
}



D 5
ObjInfo **read_objects_from_fd(fd, count)
E 5
I 5
D 12
ObjInfo **read_objects_from_fd(fd, msgs, count)
E 12
I 12
ObjInfo **read_objects_from_fd(fd, header, msgs, count)
E 12
E 5
/* Reads in an entire object set from the file descriptor given and
   returns a pointer to a newly allocated array of the object definitions.
   The number of objects read is returned by reference in count.
   If NULL is returned, that means there were no objects to read.
   The caller is responsible for some day freeing the array of pointers
   and the object definition records. */
int fd;
I 12
ObjHeader **header;
E 12
I 5
VLine **msgs;
E 5
int *count;
{
  int i, size, done = FALSE;
  ObjInfo **new = NULL, *nextobj;

D 5
  size = 50;
E 5
I 5
  *msgs = read_messages_from_fd(fd);
I 12
  *header = extract_header_from_messages(*msgs);
E 12
D 6
  size = os_extract_size(msgs);
E 6
I 6
  size = os_extract_size(*msgs);
E 6

E 5
  new = (ObjInfo **) malloc(sizeof(ObjInfo *) * size);
  demand(new, "read_objects_from_fd: out of memory");

D 5
  for (i=0; (!done); i++) {
E 5
I 5
  for (i=0; (!done && i<size); i++) {
E 5
    nextobj = read_obj_from_fd(fd);
D 5
    if (nextobj) {
      if (i >= size) {
	size += 50;
	new = (ObjInfo **) realloc(new, sizeof(ObjInfo *) * size);
	demand(new, "read_objects_from_fd: can't reallocate space");
      }
      new[i] = nextobj;
    }
E 5
I 5
    if (nextobj) new[i] = nextobj;
E 5
    else done = TRUE;
  }

  *count = i - 1;
  if (*count)
    return new;
  else {
    if (new) free(new);
    return NULL;
  }
}



D 12
ObjInfo **read_objects_from_file(filename, msgs, count, newname)
E 12
I 12
ObjInfo **read_objects_from_file(filename, header, msgs, count, newname)
E 12
/* reads an object set in from a file, creating an array of pointers
   to the object definition records.  Caller is responsible for eventually
   freeing the space allocated.  Returns NULL if we could not open
   the file for reading.  If newname is a pointer to a (char *) variable,
   rather than NULL, we'll return the new name we may have had to use
   to find the file in the library directory (the string given is
   must also be freed someday).  Any messages in the object file will
   also be read in - if the msgs pointer is not NULL. */
char *filename;   /* if NULL, we'll use the default object filename */
I 12
ObjHeader **header;   /* return value is NULL if there is no header info */
E 12
VLine **msgs;	  /* return value is NULL if there are no messages */
int *count;	  /* returns by reference number of objects loaded */
char **newname;	  /* returns by reference a copy of libbed filename if used */
{
  FILE *fp;
I 5
  VLine *mtmp;
E 5
  ObjInfo **result;
I 10
  char *libbed_filename();
E 10
  demand(count, "read_objects_from_file: need a count pointer");
  *count = 0;

  /* open the file if possible */
  if (!filename) 
    filename = DEFAULT_OBJ_PATH;
  fp = fopen(filename, "r");
  if (!fp) {
    fp = fopen(libbed_filename(OBJ_LIB_DIR, filename), "r");
    if (fp && newname) 
      *newname = dupstr(libbed_filename(OBJ_LIB_DIR, filename));
  }
  if (!fp)
    return NULL;

  /* read messages and objects and close file */
D 5
  if (msgs) *msgs = read_messages_from_fd(fileno(fp));
  else (void) read_messages_from_fd(fileno(fp));
  result = read_objects_from_fd(fileno(fp), count);
  fclose(fp);
E 5
I 5
D 12
  result = read_objects_from_fd(fileno(fp), &mtmp, count);
E 12
I 12
  result = read_objects_from_fd(fileno(fp), header, &mtmp, count);
E 12
  if (msgs) *msgs = mtmp;
  else if (mtmp) free(mtmp);
E 5

I 5
  fclose(fp);
E 5
  return result;
}
E 3
E 2
E 1
