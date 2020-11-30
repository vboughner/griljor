h43114
s 00000/00000/00136
d D 1.11 92/08/07 21:49:42 vbo 11 10
c fixes for sun port merged in
e
s 00011/00002/00125
d D 1.10 92/02/12 00:32:02 vanb 10 9
c added free_objects() to objects.c file where it belongs
e
s 00008/00004/00119
d D 1.9 91/12/03 17:29:20 labc-4lc 9 8
c implemented header items for object def files
e
s 00001/00003/00122
d D 1.8 91/11/27 13:41:17 labc-4lc 8 7
c fixed string in objects so space is malloced, not static
e
s 00000/00000/00125
d D 1.7 91/09/04 21:16:44 labb-3li 7 6
c 
e
s 00024/00004/00101
d D 1.6 91/07/14 14:21:50 labc-3id 6 5
c worked to make loads/saves/transferrals of objects 
e
s 00029/00377/00076
d D 1.5 91/07/07 20:02:56 labc-3id 5 4
c cleaned out a bunch of old stuff no longer in use
e
s 00007/00041/00446
d D 1.4 91/06/30 18:38:55 labc-3id 4 3
c took out references to now out-dated object property flags
e
s 00000/00000/00487
d D 1.3 91/03/24 20:17:22 labc-3id 3 2
c 
e
s 00009/00008/00478
d D 1.2 91/03/24 17:55:01 labc-3id 2 1
c 
e
s 00486/00000/00000
d D 1.1 91/02/16 12:55:13 labc-3id 1 0
c date and time created 91/02/16 12:55:13 by labc-3id
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

/* Object Handling routines */


#define OBJ_MAIN	/* will tell objects.h not to define externals */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "def.h"
I 5
D 6
#include "vline.h"
E 6
E 5
#include "objects.h"
#include "lib.h"
I 9
#include "objinfo.h"
E 9


/* Global Variables */

D 5
ObjInfo		*info[MAX_OBJECTS];	/* array of pointers to obj info */
E 5
I 5
ObjInfo		**info;			/* array of pointers to obj info */
void		*obj_messages;		/* linked list of object messages */
I 9
ObjHeader	*obj_header;		/* header information for obj set */
E 9
E 5
int		objects = 0;		/* how many objects are defined */


D 5
/* ================ R E C O R D S  to strings ============================ */
E 5

D 5
/* makes a string containing information about an object's characteristics,
   this string is suitable as the flag line in an object info file */

char *object_info_line(record)
ObjInfo *record;
{
  int i, j;
  char tmp[80];
  static char result[850];

  *result = '\0';
  if (record->masked) strcat(result, "mask ");
  if (record->recorded) strcat(result, "recorded ");
  if (record->transparent) strcat(result, "transparent ");
  if (record->glows) strcat(result, "glows ");
  if (record->flashlight) strcat(result, "flashlight ");
D 4
  if (record->move) {
    sprintf(tmp, "move %d ", record->movement);
    strcat(result, tmp);
  }
  if (record->override) {
    sprintf(tmp, "override %d ", record->override);
    strcat(result, tmp);
  }
E 4
I 4
  sprintf(tmp, "move %d ", record->movement);
  strcat(result, tmp);
E 4
  if (record->concealing) strcat(result, "concealing ");
  if (record->permeable) strcat(result, "permeable ");
  if (record->exit) strcat(result, "exit ");
D 4
  if (record->pushable) strcat(result, "pushable ");
E 4
  if (record->takeable) {
    sprintf(tmp, "takeable %d ", record->weight);
    strcat(result, tmp);
  }
  if (record->weapon) {
    sprintf(tmp, "weapon %d %d %d %d ", record->damage, record->range,
	    record->speed, record->movingobj);
    strcat(result, tmp);
  }
  if (record->stop) strcat(result, "stop ");
  if (record->explodes) {
    sprintf(tmp, "explodes %d ", record->explodes);
    strcat(result, tmp);
  }
D 4
  if (record->flamable) {
    sprintf(tmp, "flamable %d ", record->flamable);
E 4
I 4
  if (record->flammable) {
    sprintf(tmp, "flammable %d ", record->flammable);
E 4
    strcat(result, tmp);
  }
  if (record->charges) {
    sprintf(tmp, "charges %d ", record->charges);
    strcat(result, tmp);
  }
  if (record->numbered) strcat(result, "numbered ");
  if (record->lost) strcat(result, "lost ");
D 4
  if (record->thrown) strcat(result, "thrown ");
E 4
  if (record->refire) {
    sprintf(tmp, "refire %d ", record->refire);
    strcat(result, tmp);
  }
  if (record->capacity) {
    sprintf(tmp, "capacity %d ", record->capacity);
    strcat(result, tmp);
  }
  if (record->directional) strcat(result, "directional ");
D 4
  if (record->armor) {
    sprintf(tmp, "armor %d %d ", record->defense, record->absorb);
    strcat(result, tmp);
  }
E 4
  if (record->wearable) strcat(result, "wearable ");
  if (record->swings) {
    sprintf(tmp, "swings %d ", record->alternate);
    strcat(result, tmp);
  }
  if (record->id) strcat(result, "id ");
  if (record->type) {
    sprintf(tmp, "type %d ", record->type);
    strcat(result, tmp);
  }
  if (record->opens) {
    sprintf(tmp, "opens %d ", record->opens);
    strcat(result, tmp);
  }
  if (record->vulnerable) {
    sprintf(tmp, "vulnerable %d ", record->destroyed);
    strcat(result, tmp);
  }
D 4
  if (record->restorable) {
    sprintf(tmp, "restorable %d ", record->restored);
    strcat(result, tmp);
  }
E 4
  if (record->flag) strcat(result, "flag ");
D 4
  if (record->target) strcat(result, "target ");
E 4
  if (record->magic) {
    sprintf(tmp, "magic %d ", record->magic);
    strcat(result, tmp);
  }
  for (i=1,j=0; j<7; i*=2,j++)
    if (record->set[j]) {
      sprintf(tmp, "set %d %d ", i, record->def[j]);
      strcat(result, tmp);
    }
  if (record->ignoring) strcat(result, "ignoring ");
  if (record->piercing) strcat(result, "piercing ");
  if (record->destroys) strcat(result, "destroys ");
  if (record->explodes) {
    sprintf(tmp, "boombit %d ", record->boombit);
    strcat(result, tmp);
  }
  return result;
}
  

/* ================= S T R I N G S  to records =========================== */

/* get flags out of a line of info */

handle_obj_flags(record, line)
ObjInfo *record;
char    *line;
{
  char temp[120], word[50];
  int i, j;
  
  strcpy(temp, line);

  while (strlen(temp)>0) {
    strcpy(word, get_next_word(temp));

    if      (!strcmp(word, "recorded")) record->recorded = TRUE;
    else if (!strcmp(word, "masked")) record->masked = TRUE;
    else if (!strcmp(word, "mask")) record->masked = TRUE;
    else if (!strcmp(word, "transparent")) record->transparent = TRUE;
    else if (!strcmp(word, "glows")) record->glows = TRUE;
    else if (!strcmp(word, "flashlight")) record->flashlight = TRUE;
    else if (!strcmp(word, "concealing")) record->concealing = TRUE;
    else if (!strcmp(word, "permeable")) record->permeable = TRUE;
    else if (!strcmp(word, "exit")) record->exit = TRUE;
D 4
    else if (!strcmp(word, "pushable")) record->pushable = TRUE;
E 4
    else if (!strcmp(word, "stop")) record->stop = TRUE;
    else if (!strcmp(word, "numbered")) record->numbered = TRUE;
    else if (!strcmp(word, "lost")) record->lost = TRUE;
D 4
    else if (!strcmp(word, "thrown")) record->thrown = TRUE;
E 4
    else if (!strcmp(word, "directional")) record->directional = TRUE;
D 4
    else if (!strcmp(word, "wearable")) record->wearable = TRUE;
E 4
    else if (!strcmp(word, "id")) record->id = TRUE;
    else if (!strcmp(word, "flag")) record->flag = TRUE;
D 4
    else if (!strcmp(word, "target")) record->target = TRUE;
    else if (!strcmp(word, "move")) {
        record->move = TRUE;
        record->movement = atoi(get_next_word(temp));
    } else if (!strcmp(word, "override")) {
E 4
I 4
    else if (!strcmp(word, "override")) {
E 4
        record->override = atoi(get_next_word(temp));
    } else if (!strcmp(word, "takeable")) {
        record->takeable = TRUE;
        record->weight = atoi(get_next_word(temp));
    } else if (!strcmp(word, "weapon")) {
        record->weapon = TRUE;
        record->damage = atoi(get_next_word(temp));
        record->range = atoi(get_next_word(temp));
        record->speed = atoi(get_next_word(temp));
	record->movingobj = atoi(get_next_word(temp));
    } else if (!strcmp(word, "explodes")) {
        record->explodes = atoi(get_next_word(temp));
D 4
    } else if (!strcmp(word, "flamable")) {
        record->flamable = atoi(get_next_word(temp));
E 4
    } else if (!strcmp(word, "charges")) {
        record->charges = atoi(get_next_word(temp));
    } else if (!strcmp(word, "refire")) {
        record->refire = atoi(get_next_word(temp));
    } else if (!strcmp(word, "capacity")) {
        record->capacity = atoi(get_next_word(temp));
D 4
    } else if (!strcmp(word, "armor")) {
        record->armor = TRUE;
        record->defense = atoi(get_next_word(temp));
        record->absorb = atoi(get_next_word(temp));
E 4
    } else if (!strcmp(word, "swings")) {
        record->swings = TRUE;
        record->alternate = atoi(get_next_word(temp));
    } else if (!strcmp(word, "type")) {
        record->type = atoi(get_next_word(temp));
    } else if (!strcmp(word, "opens")) {
        record->opens = atoi(get_next_word(temp));
    } else if (!strcmp(word, "vulnerable")) {
        record->vulnerable = TRUE;
        record->destroyed = atoi(get_next_word(temp));
D 4
    } else if (!strcmp(word, "restorable")) {
        record->restorable = TRUE;
        record->restored = atoi(get_next_word(temp));
E 4
    } else if (!strcmp(word, "magic")) {
        record->magic = atoi(get_next_word(temp));
    } else if (!strcmp(word, "set")) {
D 4
	i = atoi(get_next_word(temp));	/* which var to set */
	for (j=0; i>1; j++,i /= 2);	/* which element in default array */
E 4
I 4
	i = atoi(get_next_word(temp));
	for (j=0; i>1; j++,i /= 2);
E 4
	record->set[j] = TRUE;
	record->def[j] = atoi(get_next_word(temp));
    } else if (!strcmp(word, "item")) {
        record->masked = TRUE;
D 4
	record->move = TRUE;
E 4
	record->movement = 9;
	record->transparent = TRUE;
	record->permeable = TRUE;
    } else if (!strcmp(word, "ignoring")) record->ignoring = TRUE;
    else if (!strcmp(word, "piercing")) record->piercing = TRUE;
    else if (!strcmp(word, "destroys")) record->destroys = TRUE;
    else if (!strcmp(word, "boombit")) {
        record->boombit = atoi(get_next_word(temp));
    } else printf("  unknown keyword: %s\n", word);
  }

}


/* =============== L O A D I N G  and  S A V I N G ======================= */

/* load the objects for a file into the default record array */

load_obj_file(filename)
char *filename;
{
  load_file_into_obj_array(filename, info, &objects);
}



/* load the objects from a file into an object record array, and set count
   to be the number of objects loaded, quit with an error when object file
   cannot be read from either the given name or a file of the same name
   in the object library directory. */

load_file_into_obj_array(filename, o, count)
char *filename;
ObjInfo *o[];
int *count;
{
  FILE *fp;
  register int i;
  ObjInfo *nextobj;
  
  fp = fopen(filename, "r");
  if (DEBUG) printf("Loading objects from %s\n", filename);
  
  if (fp == NULL) {
    fp = fopen(libbed_filename(OBJ_LIB_DIR, filename), "r");
    if (DEBUG) printf("Loading objects from %s\n", filename);
    if (!fp) {
	printf("Griljor: was searching for object def file %s\n", filename);
	Gerror("could not find object definition file");
    }
  }

  for (i=0; i<MAX_OBJECTS && !feof(fp); i++) {
    nextobj = allocate_obj();
    fread((char *)nextobj, sizeof(ObjInfo), 1, fp);
D 2
    o[(uc)i] = nextobj;
E 2
I 2
    o[i] = nextobj;
E 2
  }

  fclose(fp);
  *count = i - 1;
}



/* save the objects into a file from an object record array, 
D 2
   quit with an error when object file cannot be written */
E 2
I 2
   return FALSE when object file cannot be written */
E 2

save_file_from_obj_array(filename, o, count)
char *filename;
ObjInfo *o[];
int count;
{
  FILE *fp;
  register int i;
  ObjInfo *nextobj;
  
  fp = fopen(filename, "w");
  if (DEBUG) printf("Saving objects from %s\n", filename);
  
D 2
  if (fp == NULL) {
    printf("Griljor: opening file %s\n", filename);
    Gerror("could not write object definition file");
  }
E 2
I 2
  if (fp == NULL) 
    {
      return(FALSE);
    }
E 2

D 2
  for (i=0; i<MAX_OBJECTS && i<count; i++) {
    fwrite((char *)o[(uc)i], sizeof(ObjInfo), 1, fp);
E 2
I 2
  for (i=0; i<count; i++) {
    fwrite((char *)o[i], sizeof(ObjInfo), 1, fp);
E 2
  }

  fclose(fp);
I 2
  return(TRUE);
E 2
}



E 5
/* ============================ M I S C ================================= */

I 5

E 5
ObjInfo *allocate_obj()
{
  ObjInfo *result;
D 10

E 10
  result = (ObjInfo *) malloc(sizeof(ObjInfo));
D 10

E 10
  if (result == NULL) Gerror("out of memory while loading object info");

  return result;
}


D 5
/* initialize the object to make all flags false */
E 5

initialize_obj_record(record)
I 5
D 8
/* initialize the object to make all flags false, or empty,
   excepting the name, which will be "no name" */
E 8
I 8
/* initialize the object to make all flags false, or empty */
E 8
E 5
ObjInfo *record;
{
  char *p;
  int  i;
  
  for (i=0,p=(char *) record; i<sizeof(ObjInfo); i++,p++) *p = 0;
I 10
}



void free_objects()
/* free the objects */
{
  int i;

  for (i=0; i<objects; i++) free(info[(uc)i]);
  objects = 0;
E 10
D 8
  strcpy(record->name, "no name");
E 8
}


D 5
/* =================== L O A D  info file into record =================== */
E 5

D 5
/* Take an info file that contains all relevant data on generic object types
   and overwrite a set of object definition records with its contents.
   Return TRUE if there were no errors.  Records that do not or should not
   contain definitions, should have a pointer set to NULL in the record
   pointer array. */
E 5
I 5
/* ================ R E C O R D S  to strings ============================ */
E 5

D 5
int overwrite_record_info(recordset, readfile)
ObjInfo *recordset[];
char *readfile;
{
  char		*read_line();
  char		filename[120], line[120];
  ObjInfo	obj;
  int		i = 0;
  FILE	*infile;
E 5

D 5
  /* skip over the base filenames for the bitmap files */
  infile = fopen(readfile, "r");
  if (!infile) return FALSE;
  read_line(infile);
  read_line(infile);

  /* let user know what we are doing */
  printf("reading %s\n", readfile);

  /* now read about objects one by one until there are no more */
  strcpy(line, read_line(infile));
  while (!feof(infile)) {

    /* read about this kind of object, saving observation in obj record */
    if (recordset[i]) {
      clear_all_but_bitmaps(recordset[i]);
      put_in_obj_name(recordset[i], line, i);
    }

    strcpy(line, read_line(infile));
    while (!isdigit(*line) && !feof(infile)) {
      if (recordset[i]) handle_obj_flags(recordset[i], line);
      strcpy(line, read_line(infile));
    }

    /* increment the counter */
    i++;
  }

  fclose(infile);
  return TRUE;
}



/* get the name out of the first line of an object's information area */

put_in_obj_name(record, line, num)
E 5
I 5
char *object_info_line(record)
/* makes a string containing information about an object's characteristics,
   this string is suitable as the flag line in an object info file */
E 5
ObjInfo *record;
D 5
char    *line;
int	num;
E 5
{
D 5
  char first[50], rest[120];
E 5
I 5
  static char *result = "New obj info under construction";
E 5

D 5
  strcpy(first, first_and_rest(line, rest));
E 5
I 5
  return result;
}
E 5
  
D 5
  printf("Object #%s \t%s\n", first, rest);
  strcpy(record->name, rest);
E 5

D 5
  if (num != atoi(first)) Gerror("object descriptions out of order");
}
E 5

I 5
/* =============== L O A D I N G  and  S A V I N G ======================= */
E 5

D 5
/* clear a given record of all but its bitmap and mask */
E 5

D 5
clear_all_but_bitmaps(record)
ObjInfo *record;
E 5
I 5
load_obj_file(filename)
/* load the objects for a file into the global info array */
char *filename;
E 5
{
D 5
  char bsave[BITMAP_ARRAY_SIZE];
  char msave[BITMAP_ARRAY_SIZE];
E 5
I 5
  ObjInfo **read_objects_from_file();
E 5

D 5
  bitmapcpy(bsave, record->bitmap);
  bitmapcpy(msave, record->mask);
  bzero(record, sizeof(ObjInfo));
  bitmapcpy(record->bitmap, bsave);
  bitmapcpy(record->mask, msave);
E 5
I 5
D 6
  info = read_objects_from_file(filename, (VLine **)&obj_messages, 
				&objects, NULL);
E 6
I 6
D 9
  info = read_objects_from_file(filename, &obj_messages, &objects, NULL);
E 9
I 9
  info = read_objects_from_file(filename, &obj_header, &obj_messages,
				&objects, NULL);
E 9
E 6
  if (!info) {
    printf("Griljor: could not find file %s\n", filename);
    Gerror("could not find object definition file");
  }
E 5
}


D 5
/* =================== S A V E  info into a file ======================== */
E 5

D 5
/* Given a record of objects, and how many objects there are in it,
   create a text info file of the object definitions.  This is the exact
   reverse of the procedure overwrite_record_info().  Return TRUE if there
   were no errors, otherwise return false.  Records that do not or should not
   contain definitions, should have a pointer set to NULL in the record
   pointer array. */

int write_out_record_info(recordset, numrecs, readfile)
ObjInfo *recordset[];
int numrecs;
char *readfile;
E 5
I 5
save_obj_file(filename)
/* saves the default object file from the global info array,
   return FALSE when object file cannot be written. */
char *filename;
E 5
{
D 5
  register int i;
  FILE *fp;

  fp = fopen(readfile, "w");
  if (!fp) return FALSE;

  fprintf(fp, "./bitmap_base\n");
  fprintf(fp, "./mask_base\n");

  for (i=0; i<numrecs; i++)
    write_object_info_as_text(recordset[i], i, fp);

  fclose(fp);
  return TRUE;
}



/* Given the pointer to an object info record and an object number to
   label it with, write out a couple text lines to the given file that
   cover the properties of the object.  If the object pointer is NULL,
   then put out a blank definition. */

write_object_info_as_text(record, number, fp)
ObjInfo *record;
int number;
FILE *fp;
{
  if (record) {
    fprintf(fp, "%d %s\n", number, record->name);
    fprintf(fp, "%s\n", object_info_line(record));
  }
  else {
    fprintf(fp, "%d undefined object\n", number);
  }
E 5
I 5
D 6
  return write_objects_to_file(filename, (VLine *)obj_messages, info, objects);
E 6
I 6
D 9
  return write_objects_to_file(filename, obj_messages, info, objects);
E 9
I 9
  return write_objects_to_file(filename, obj_header, obj_messages,
			       info, objects);
E 9
}



/* ================= S O C K E T  T R A N S F E R R A L ================== */


void receive_obj_file(socknum)
/* read the object file from the given file device number */
int socknum;
{
  ObjInfo **read_objects_from_fd();
D 9
  info = read_objects_from_fd(socknum, &obj_messages, &objects);
E 9
I 9
  info = read_objects_from_fd(socknum, &obj_header, &obj_messages, &objects);
E 9
}



void send_obj_file(socknum)
/* transmit the object file over the given stream socket number */
int socknum;
{
D 9
  write_objects_to_fd(socknum, obj_messages, info, objects);
E 9
I 9
  write_objects_to_fd(socknum, obj_header, obj_messages, info, objects);
E 9
E 6
E 5
}
E 1
