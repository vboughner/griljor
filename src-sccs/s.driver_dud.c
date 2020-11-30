h07167
s 00000/00000/00225
d D 1.9 92/08/07 01:00:51 vbo 9 8
c source copied to a separate tree for work on new map and object format
e
s 00008/00000/00217
d D 1.8 92/01/02 17:05:39 labc-4lc 8 7
c 
e
s 00002/00002/00215
d D 1.7 91/12/07 17:37:17 labc-4lc 7 6
c made error message window bigger and scrollable
e
s 00008/00000/00209
d D 1.6 91/12/05 12:09:13 labc-4lc 6 5
c fixed objheader stuff to compile with editmap
e
s 00024/00000/00185
d D 1.5 91/09/27 20:01:44 labc-4lc 5 4
c 
e
s 00004/00000/00181
d D 1.4 91/09/08 19:33:35 labc-4lc 4 3
c 
e
s 00012/00000/00169
d D 1.3 91/08/26 00:33:13 vanb 3 2
c fixed up procedure defs and other compatibilty problems
e
s 00004/00000/00165
d D 1.2 91/07/07 20:01:38 labc-3id 2 1
c added another dud
e
s 00165/00000/00000
d D 1.1 91/02/16 12:53:47 labc-3id 1 0
c date and time created 91/02/16 12:53:47 by labc-3id
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

D 7
redraw_error_window()
E 7
I 7
player_error_out_message()
E 7
{
D 7
  printf("Warning: monsters don't call redraw_error_window\n");
E 7
I 7
  printf("Warning: monsters don't call player_out_message\n");
E 7
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
I 2

ChangedRoomSquare()
{
}
I 3

AutoMask()
{
}

SaveObject()
{
}

QuitObject()
{
}
I 4

ClearDamageRecord()
{
}
I 5

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
I 6

EditHeaderSave()
{
}

EditHeaderQuit()
{
}
I 8

local_x()
{
}

local_y()
{
}
E 8
E 6
E 5
E 4
E 3
E 2
E 1
