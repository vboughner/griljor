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

/* Dud routines for driver who doesn't need some of these */


int move_person()
{
  printf("Warning: driver shouldn't be calling move_person()\n");
}



/* in socket.c */

add_missile()
{
  printf("Warning: driver shouldn't call add_missile\n");
}


/* In playerX10.c */

redraw_game_person(num)
int num;
{
  printf("Warning: monster program shouldn't call redraw_game_person\n");
}



/* In playerX10.c */

erase_old_game_person(num)
int num;
{
  printf("Warning: monster program shouldn't call erase_old_game_person\n");
}


/* In playerX10.c */

update_pixmaps(num)
int num;
{
  printf("Warning: monster program shouldn't call update_pixmaps\n");
}



/* In playerX10.c */

redraw_player_room_square()
{
  printf(
  "Warning: monster program shouldn't call redraw_player_room_square\n");
}


/* In playerX10.c */

player_error_out_message()
{
  printf("Warning: monsters don't call player_out_message\n");
}


/* In playerX10.c */

delete_person_pixmap()
{
  printf("Warning: monsters don't call delete_person_pixmap\n");
}


/* In playerX10.c */

redraw_persons_here()
{
  printf(
  "Warning: monster program shouldn't call redraw_persons_here\n");
}


redraw_missile()
{
  printf("Warning: mosnter shouldn't call missile routines\n");
}



redraw_player_health()
{
  printf("Warning: monster shouldn't call redraw_player_health\n");
}


player_out_message()
{
  printf("Warning: monster shouldn't call player_out_message\n");
}

redraw_hand_contents()
{
}

redraw_inventory_window()
{
}

flag_visibility()
{
}

flag_all_visibilities()
{
}

redraw_newly_visible_persons()
{
}

draw_hit()
{
}

show_player_ending_game()
{
}

alter_experience()
{
}

end_game_experience()
{
}

add_kill()
{
}

add_multi_missiles()
{
}

draw_room_square()
{
}

monster_peek_packet()
{
}

joiner_message()
{
}

ChangedRoomSquare()
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

ClearDamageRecord()
{
}

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

EditHeaderSave()
{
}

EditHeaderQuit()
{
}

local_x()
{
}

local_y()
{
}
