h29158
s 00000/00000/00056
d D 1.3 92/08/07 01:02:35 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00012/00000/00044
d D 1.2 91/08/26 00:33:44 vanb 2 1
c fixed up procedure defs and other compatibilty problems
e
s 00044/00000/00000
d D 1.1 91/02/16 12:55:00 labc-3id 1 0
c date and time created 91/02/16 12:55:00 by labc-3id
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

/* Dud routine definitions for the sake of the monster program.  Some
   routines that the player and the monster share make references to 
   procedures that aren't avaliable to the monster program (it isn't 
   compiled with those files) so these are provided just so that gcc
   doesn't squawk about procedures being undefined */



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
E 2
E 1
