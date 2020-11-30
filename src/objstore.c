/***************************************************************************
 * @(#) objstore.c 1.8 - last change made 08/26/91
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

/* Routines for loading and storing of object files in the new format.
   The ability to load and store object files by means of a file
   descriptor is provided here in order to use this for socket pipes.
   A couple of convenience functions for disk files are here as well.
   Object files should not be loaded or stored using any other functions
   than these, in order to retain a standard data file structure. */

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

/* locally global variable that points to top of dialog record search tree */
static DialogList *dialogdesc = NULL;

/* locally global buffer */
static char buf[STRING_BUFFER_SIZE];


/* ========================= W R I T I N G ============================ */


write_to_descriptor(socknum, stuff, nbytes)
int socknum;
char *stuff;
int nbytes;
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

  return 0;
}



static void write_obj_to_fd(o, fd)
ObjInfo *o;
int fd;
/* writes a single object description to a file device.  Object is described
   be giving the id and value of all flags that are not FALSE or NULL in
   the object's property flag list.  Identifier of zero signifies the
   end of the record.  High order byte is written to file first. */
{
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
  int i, j, len, value;
  char id[2], *tmp, **deref;
  DialogList *ptr;

  i = 0;
  do {
    len = -1;
    ptr = &(ObjectData[i++]);
    id[0] = (unsigned char) (ptr->id / BYTE_SIZE);
    id[1] = (unsigned char) (ptr->id % BYTE_SIZE);
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
	    buf[0] = (unsigned char) (value / BYTE_SIZE);
	    buf[1] = (unsigned char) (value % BYTE_SIZE);
	    len = 2;
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



static void write_messages_to_fd(fd, msgs, count)
/* writes out all the messages to the file descriptor given.  This should
   be called by write_objects_to_fd().  If msgs is NULL, then only
   a bare minimum of header information and the end of messages
   line will be written out.  Messages are descriptions of the various
   objects and so forth.  The message text lines also contain important
   pieces of information about the object file as well. */
int fd;
VLine *msgs;
int count;	/* required so it can be placed in header */
{
  char s[200];
  VLine *ptr;

  msgs = make_count_accurate(msgs, count);

  for (ptr = msgs;  ptr;  ptr = next_line(ptr)) {
    write_to_descriptor(fd, line_text(ptr), strlen(line_text(ptr)));
    write_to_descriptor(fd, "\n", 1);
  }

  sprintf(s, "%s\n", END_OF_MESSAGES);
  write_to_descriptor(fd, s, strlen(s));  
}



void write_objects_to_fd(fd, header, msgs, o, count)
/* writes out an entire object set to a file descriptor, object by object.
   Count needs to be set to be the number of objects to write out. */
int fd;
ObjHeader *header;
VLine *msgs;
ObjInfo **o;
int count;
{
  int i;

  insert_header_into_messages(header, &msgs);
  write_messages_to_fd(fd, msgs, count);

  for (i=0; i<count; i++) write_obj_to_fd(o[i], fd);
}



int write_objects_to_file(filename, header, msgs, o, count)
/* writes out an entire object set to a particular filename.  Returns
   TRUE if the file was written, FALSE if file could not be opened
   for writing. */
char *filename;
ObjHeader *header;
VLine *msgs;
ObjInfo **o;
int count;
{
  FILE *fp;

  fp = fopen(filename, "w");
  if (!fp) return FALSE;

  write_objects_to_fd(fileno(fp), header, msgs, o, count);
  fclose(fp);

  return TRUE;
}



/* ===================== R E A D I N G ================================= */


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
   id number is not found in the list, then NULL is returned.
   If there are two properties with the same id number, then a warning
   message is generated. */
int id;
{
  DialogList *ptr, *found = NULL;

  if (dialogdesc) {
    ptr = dialogdesc;
    while (ptr->type != ENDLIST) {
      if (ptr->id == id) {
	if (!found)
	  found = ptr;
	else
	  fprintf(stderr, "Warning: multiple properties using id %d\n", id);
      }
      ptr++;
    }
  }
  return found;
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



static ObjInfo *read_obj_from_fd(fd)
/* reads from a device the character stream that represents an object
   property description.  Returns the pointer to a newly allocated
   object structure if an object was read, returns NULL if the end of
   the file has been reached and no object was readable.
   The caller is responsible for freeing any object record returned. */
int fd;
{
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
  ObjInfo *new = NULL;
  int i, j, len, value;
  char id[2], *tmp, **deref;
  DialogList *ptr;

  /* read the id number of next object property flag in stream */
  len = read_from_descriptor(fd, id, 2);
  if (len != 2) return NULL;

  /* allocate and clear a new object record */
  new = allocate_obj();
  initialize_obj_record(new);
  init_dialog_find(ObjectData);

  /* read each flag and place its value into the object structure */
   while ((len == 2) && (id[0] | id[1])) {
    value = ((short) ((unsigned char)id[0]) * BYTE_SIZE) + 
	    (short) ((unsigned char)id[1]);
    ptr = find_dialog_desc(value);

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

  return new;
}

   

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
    len = read_from_descriptor(fd, &c, 1);
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



static VLine *read_messages_from_fd(fd)
/* reads in all the messages from the file descriptor given.  This should
   be called by read_objects_from_fd().  If return value is NULL, it
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



ObjInfo **read_objects_from_fd(fd, header, msgs, count)
/* Reads in an entire object set from the file descriptor given and
   returns a pointer to a newly allocated array of the object definitions.
   The number of objects read is returned by reference in count.
   If NULL is returned, that means there were no objects to read.
   The caller is responsible for some day freeing the array of pointers
   and the object definition records. */
int fd;
ObjHeader **header;
VLine **msgs;
int *count;
{
  int i, size, done = FALSE;
  ObjInfo **new = NULL, *nextobj;

  *msgs = read_messages_from_fd(fd);
  *header = extract_header_from_messages(*msgs);
  size = os_extract_size(*msgs);

  new = (ObjInfo **) malloc(sizeof(ObjInfo *) * size);
  demand(new, "read_objects_from_fd: out of memory");

  for (i=0; (!done && i<size); i++) {
    nextobj = read_obj_from_fd(fd);
    if (nextobj) new[i] = nextobj;
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



ObjInfo **read_objects_from_file(filename, header, msgs, count, newname)
/* reads an object set in from a file, creating an array of pointers
   to the object definition records.  Caller is responsible for eventually
   freeing the space allocated.  Returns NULL if we could not open
   the file for reading.  If newname is a pointer to a (char *) variable,
   rather than NULL, we'll return the new name we may have had to use
   to find the file in the library directory (the string given is
   must also be freed someday).  Any messages in the object file will
   also be read in - if the msgs pointer is not NULL. */
char *filename;   /* if NULL, we'll use the default object filename */
ObjHeader **header;   /* return value is NULL if there is no header info */
VLine **msgs;	  /* return value is NULL if there are no messages */
int *count;	  /* returns by reference number of objects loaded */
char **newname;	  /* returns by reference a copy of libbed filename if used */
{
  FILE *fp;
  VLine *mtmp;
  ObjInfo **result;
  char *libbed_filename();
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
  result = read_objects_from_fd(fileno(fp), header, &mtmp, count);
  if (msgs) *msgs = mtmp;
  else if (mtmp) free(mtmp);

  fclose(fp);
  return result;
}
