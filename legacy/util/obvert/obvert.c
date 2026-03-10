/***************************************************************************
 * %Z% %M% %G% - last change made %I%
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

/* program which converts a file containing OldObjInfo records to one
   that contains the newer ObjInfo records (once their size has been
   modified) */

#include <stdio.h>
#include "def.h"
#include "oldobj.h"
#include "objects.h"


typedef struct _objlist {
/* type definition for holding a linked list of these object records */
	OldObjInfo	*old;
	ObjInfo		*new;
	struct _objlist *next;
} ObjList;




static char *get_filename_from_options(argc, argv)
/* looks for single option in command line which contains the a filename,
   returns NULL if there isn't one the item on command line begins with a
   '-' character rather than the kind of thing you would expect to be
   the first letter in a filename */
int argc;
char *argv[];
{
  if (argc != 2) return NULL;
  else if (*argv[1] == '-') return NULL;
  else return argv[1];
}



static ObjList *load_old_file_contents(filename)
/* creates a linked list of objects it reads from the old object file.
   returns NULL if there is no such file, or returns a pointer to the linked
   list if the file is there and we read it successfully */
char *filename;
{
  int numread;
  FILE *fp;
  ObjList *top = NULL, *last, *ptr;
  
  fp = fopen(filename, "r");
  if (!fp) return NULL;

  while (!feof(fp)) {
    ptr = (ObjList *) malloc(sizeof(ObjList));
    demand(ptr, "load_old_file_contents: no memory for new ObjList record");
    ptr->new = NULL;
    ptr->next = NULL;
    ptr->old = (OldObjInfo *) malloc(sizeof(OldObjInfo));
    demand(ptr->old, "load_old_file_contents: no memory for old obj record");
    numread = fread((char *)ptr->old, sizeof(OldObjInfo), 1, fp);
    if (numread == 1) {
      if (!top) top = ptr;
      else last->next = ptr;
      last = ptr;
    }
    else {
      free(ptr->old);
      free(ptr);
    }
  }

  fclose(fp);
  return top;
}



static convert_objects(list)
/* takes data in the old object of each list element, creates a new
   object and transfers the data into it.  This basically entails clearing
   the the new record to all zeros and then copying over the elements of
   the old structure with the same name as an element in the new structure.
   Elements of the old structure with no counterpart are not copied over. */
ObjList *list;
{
  int i;
  char *tmp;
  OldObjInfo *old;
  ObjInfo *new;
  ObjList *ptr = list;

  while (ptr) {
    /* create new object record */
    new = (ObjInfo *) malloc(sizeof(ObjInfo));
    demand(new, "convert_objects: no memory for new obj record");
    tmp = (char *) new;
    for (i=0; (i < sizeof(ObjInfo)); i++,tmp++) *tmp = 0;
    
    /* copy over equivalent elements (old => new) */
    old = ptr->old;
    strcpy(new->name, old->name);
    new->masked = old->masked;
    new->recorded = old->recorded;
    new->transparent = old->transparent;
    new->glows = old->glows;
    new->flashlight = (short) old->flashlight;
    new->movement = old->movement;
    new->override = old->override;
    new->concealing = old->concealing;
    new->permeable = old->permeable;
    new->exit = old->exit;
    new->takeable = old->takeable;
    new->weight = old->weight;
    new->weapon = old->weapon;
    new->damage = old->damage;
    new->range = old->range;
    new->speed = old->speed;
    new->movingobj = old->movingobj;
    new->stop = old->stop;
    new->explodes = old->explodes;
    new->charges = old->charges;
    new->numbered = old->numbered;
    new->lost = old->lost;
    new->refire = old->refire;
    new->capacity = old->capacity;
    new->directional = old->directional;
    new->defense = old->defense;
    new->absorb = old->absorb;
    new->wearable = old->wearable;
    new->swings = old->swings;
    new->alternate = old->alternate;
    new->id = old->id;
    new->type = old->type;
    new->opens = old->opens;
    new->vulnerable = old->vulnerable;
    new->destroyed = old->destroyed;
    new->magic = old->magic;
    new->ignoring = (short) old->ignoring;
    new->piercing = (short) old->piercing;
    new->destroys = (short) old->destroys;
    new->boombit = old->boombit;
    new->arc = old->arc;
    new->spread = old->spread;
    new->fan = old->fan;
    for (i=0; i<7; i++) new->set[i] = old->set[i];
    for (i=0; i<7; i++) new->def[i] = old->def[i];
    for (i=0; i<BITMAP_ARRAY_SIZE; i++) new->bitmap[i] = old->bitmap[i];
    for (i=0; i<BITMAP_ARRAY_SIZE; i++) new->mask[i] = old->mask[i];
    /* copy over elements with differing names or arrangement */
    if (old->flag) {
	new->flag = TRUE;
	new->important = TRUE;
	new->flagteams = old->type;
    }
    new->unswitchable = old->pushable;
    new->flammable = old->flamable;
    new->intohand = old->thrown;
    new->autotaken = old->armor;
    new->autoactor = old->restorable;
    new->autodelay = old->restored;

    ptr->new = new;
    ptr = ptr->next;
  }
}



static void store_objects(list, filename)
/* stores the new objects out to the file.  The conversion routine must
   have already been run on the objects in the list. */
ObjList *list;
char *filename;
{
  int i, count, okay;
  ObjInfo **o;
  ObjList *ptr;

  /* create an object list usable by the save routines */
  for (ptr=list,count=0; ptr; ptr=ptr->next) count++;
  o = (ObjInfo **) malloc(sizeof(ObjInfo *) * count);
  demand(o, "store_objects: out of memory");
  for (i=0,ptr=list; i<count; i++,ptr=ptr->next) o[i] = ptr->new;

  /* save the new object list, and no messages are to be in new file */
  okay = write_objects_to_file(filename, NULL, o, count);
  demand(okay, "store_objects: could not open file for output");
  free(o);
}



int main(argc, argv)
int argc;
char *argv[];
{
  char *filename;
  ObjList *ptr;

  filename = get_filename_from_options(argc, argv);
  demand(filename, "\nUsage:	obvert filename\n");

  ptr = load_old_file_contents(filename);
  demand(ptr, "main: file error")

  convert_objects(ptr);
  store_objects(ptr, filename);
  exit(0);
}

