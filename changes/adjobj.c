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

recursive_transfer(fromgroup,fromobj,togroup)
int fromgroup;
int fromobj;
int togroup;
{
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();

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

recursive_transfer_inner(fromgroup,fromobj,togroup,copied,ObjectData)
int fromgroup;
int fromobj;
int togroup;
IntList **copied;
DialogList *ObjectData;
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
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
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


adj_switch_all(group,one,two)
int one,two;
{
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
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
  DialogList *StaticObjectProperties();
  DialogList *ObjectData = StaticObjectProperties();
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
