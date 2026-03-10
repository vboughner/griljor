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

/* prototypes */
DialogList *StaticObjectProperties();



/* ========================= W R I T I N G ============================ */


static void write_obj_to_fd(o, fd)
ObjInfo *o;
int fd;
/* writes a single object description to a file device.  Object is described
   be giving the id and value of all flags that are not FALSE or NULL in
   the object's property flag list.  Identifier of zero signifies the
   end of the record.  High order byte is written to file first. */
{
  static DialogList *ObjectData = NULL;
  if (!ObjectData) ObjectData = StaticObjectProperties();
  write_diag_to_fd(ObjectData, o, fd);
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


static ObjInfo *read_obj_from_fd(fd)
/* reads from a device the character stream that represents an object
   property description.  Returns the pointer to a newly allocated
   object structure if an object was read, returns NULL if the end of
   the file has been reached and no object was readable.
   The caller is responsible for freeing any object record returned. */
int fd;
{
  ObjInfo *new;
  static DialogList *ObjectData = NULL;
  if (!ObjectData) ObjectData = StaticObjectProperties();

  new = allocate_obj();
  initialize_obj_record(new);

  if (read_diag_from_fd(ObjectData, new, fd)) {
    return(new);
  }
  else {
    free(new);
    return(NULL);
  }
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
