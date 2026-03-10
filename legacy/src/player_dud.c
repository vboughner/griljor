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

AutoMask()
{
}

SaveObject()
{
}

QuitObject()
{
}

calculate_rank()
{
}

EditHeaderSave()
{
}

EditHeaderQuit()
{
}
