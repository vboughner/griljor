h26187
s 00000/00000/00113
d D 1.7 92/08/07 01:03:28 vbo 7 6
c source copied to a separate tree for work on new map and object format
e
s 00008/00000/00105
d D 1.6 91/12/05 12:09:25 labc-4lc 6 5
c fixed objheader stuff to compile with editmap
e
s 00000/00002/00105
d D 1.5 91/09/27 12:10:12 labc-4lc 5 4
c 
e
s 00026/00000/00081
d D 1.4 91/09/27 11:39:46 labc-4lc 4 3
c loser is Van!
e
s 00004/00000/00077
d D 1.3 91/09/08 21:53:31 labc-4lc 3 2
c Added stub for rank checking.
e
s 00012/00000/00065
d D 1.2 91/08/26 00:34:36 vanb 2 1
c fixed up procedure defs and other compatibilty problems
e
s 00065/00000/00000
d D 1.1 91/02/16 12:55:36 labc-3id 1 0
c date and time created 91/02/16 12:55:36 by labc-3id
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

/* Dud routine definitions for the sake of the player program.  Some
   routines that the player and the monster share make references to 
   procedures that aren't avaliable to the player program (it isn't 
   compiled with those files) so these are provided just so that gcc
   doesn't squawk about procedures being undefined */

I 4
D 5
/* Van is a loser! */

E 5
EditMapInfoSave()
{
}

EditMapInfoQuit()
{
}

EditRoomInfoSave()
{
}

EditRoomInfoQuit()
{
}

EditRecObjSave()
{
}

EditRecObjQuit()
{
}

E 4

/* burt */

set_personality()
{
  printf("Warning: player shouldn't call burt routines\n");
}


query_personality()
{
  printf("Warning: player shouldn't call burt routines\n");
}

pickup_saved_person_info()
{
}


make_acceptance_pack()
{
}


get_player()
{
}


modify_player()
{
}

copy_passinfo_into_person()
{
}

redraw_an_obtor_window()
{
}

monster_peek_packet()
{
}

write_dialog_window_contents()
{
}
I 2

AutoMask()
{
}

SaveObject()
{
}

QuitObject()
{
}
I 3

calculate_rank()
{
}
I 6

EditHeaderSave()
{
}

EditHeaderQuit()
{
}
E 6
E 3
E 2
E 1
