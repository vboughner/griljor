h12660
s 00000/00000/00178
d D 1.5 92/08/07 01:00:07 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00008/00005/00170
d D 1.4 91/09/05 21:28:31 labb-3li 4 3
c made obtor use the new objprops.c code for dialog lists
e
s 00004/00004/00171
d D 1.3 91/07/14 14:01:41 labc-3id 3 2
c Added 'ObjDiag.t' code.
e
s 00025/00000/00150
d D 1.2 91/07/07 18:47:10 labc-3id 2 1
c made messages from object files be loaded and saved right
e
s 00150/00000/00000
d D 1.1 91/05/10 04:48:49 labc-3id 1 0
c date and time created 91/05/10 04:48:49 by labc-3id
e
u
U
f e 0
t
T
I 2
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
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

/* Adjusting object numbers inside property field list.
   Get out a bottle of aspirin before tackling this file. */

E 2
I 1
#include "stdio.h"
#include "obtor.h"

/*****************************************************************/

typedef struct _foolist
{
  int data;
  int aswhat;
  struct _foolist *next;
} IntList;

/*****************************************************************/

recursive_transfer(int fromgroup,int fromobj,int togroup)
{
D 3
  #include "ObjDiag.h"
E 3
I 3
D 4
  #include "ObjDiag.t"
E 4
I 4
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 4
E 3

  IntList *killlist = NULL,*temp;
  
  recursive_transfer_inner(fromgroup,fromobj,togroup,&killlist,ObjectData);

  while(killlist)
    {
      temp = killlist;
      killlist = killlist->next;
      free(temp);
    }
}

/*****************************************************************/

recursive_transfer_inner(int fromgroup,int fromobj,int togroup,
			 IntList **copied,DialogList *ObjectData)
{
  int toobj,j;
  IntList *traverse;
  short *objptr;

  /* don't copy object #0 unless it is the first object */
  if ((fromobj == 0)&&(*copied != NULL))
    return(0);

  for(traverse = *copied;traverse;traverse = traverse->next)
    if (traverse->data == fromobj)
      return(traverse->aswhat);
  
  traverse = (IntList *) malloc(sizeof(IntList));
  traverse->next = *copied;
  *copied = traverse;
  
  toobj = num_objects[togroup];
  copy_obj_to_group(objdef[fromgroup][fromobj],togroup,toobj);
  traverse->data  = fromobj;
  traverse->aswhat = toobj;

  for (j = 0;ObjectData[j].name;j++)
    if (ObjectData[j].type == LINKPIC)
      {
	objptr =
	  (short *) (((char *) objdef[togroup][toobj]) + ObjectData[j].offset);
	*objptr = recursive_transfer_inner(fromgroup,*objptr,togroup,
					   copied,ObjectData);
      }
  return(toobj);
}

/*****************************************************************/
  
adj_depend(group,from,to,delta)
int group,from,to,delta;
{
D 3
  #include "ObjDiag.h"
E 3
I 3
D 4
  #include "ObjDiag.t"
E 3

E 4
I 4
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 4
  int i,maxnum,j;

  maxnum = num_objects[group]-1;

  for (i = 0;i <= maxnum;i++)
    for (j = 0;ObjectData[j].name;j++)
      if (ObjectData[j].type == LINKPIC)
	adj_field(from,to,delta,maxnum, (short *)
		  (((char *) objdef[group][i]) + ObjectData[j].offset));

  FileChanged(group,True);
}

/*****************************************************************/
I 2

E 2

adj_switch_all(group,one,two)
int one,two;
{
D 3
  #include "ObjDiag.h"
E 3
I 3
D 4
  #include "ObjDiag.t"
E 4
I 4
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 4
E 3
  int i,j,maxnum;

  maxnum = num_objects[group]-1;
  for (i = 0;i <= maxnum;i++)
    for (j = 0;ObjectData[j].name;j++)
      if (ObjectData[j].type == LINKPIC)
	adj_switch_field(one,two, (short *) 
			 (((char *) objdef[group][i]) + ObjectData[j].offset));
  
  FileChanged(group,True);
}

/*****************************************************************/

adj_clear(group,i)
int group,i;
{
D 3
  #include "ObjDiag.h"
E 3
I 3
D 4
  #include "ObjDiag.t"
E 4
I 4
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
E 4
E 3
  int j;

  for (j = 0;ObjectData[j].name;j++)
    if (ObjectData[j].type == LINKPIC)
      *((short *) (((char *) objdef[group][i]) + ObjectData[j].offset)) = 0;
  
  FileChanged(group,True);
}

/*****************************************************************/

adj_switch_field(one,two,value)
int one,two;
short *value;
{
  if (*value == one)
    *value = two;
  else if (*value == two)
    *value = one;
}

/*****************************************************************/

adj_field(from,to,delta,maxnum,value)
int from,to,delta,maxnum;
short *value;
{
  if ((*value == from)&&(delta < 0))
    *value = 0;
  else if ((*value >= from)&&(*value <= to))
    *value += delta;

  if (*value < 0) 
    *value = 0;
  if (*value > maxnum)
    *value = 0;
}

/*****************************************************************/
E 1
