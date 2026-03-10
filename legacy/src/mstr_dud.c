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

AutoMask()
{
}

SaveObject()
{
}

QuitObject()
{
}
