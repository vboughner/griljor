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

/* Game List File manipulation programming */


#include <stdio.h>
#include <string.h>
#include "config.h"
#include "def.h"
#include "extern.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "gamelist.h"
#include "lib.h"

/* Variables global inside this file only */

static GameInfo	*glist = NULL;		/* list of games */
static GameInfo	dummyrec;		/* header record in list */

#define GAME_LIST_EXISTS		(glist != NULL)



select_a_game(player)
/* allow a player to select the game he wantsat the keyboard */
PlayerInfo *player;
{
  printf("\n");
  print_list_of_games(player);

  if (!strcmp(player->our_game, UNKNOWN))
    strcpy(player->our_game, prompt_for_input("\nEnter Game: "));
  else
    printf("Enter Game: %s\n", player->our_game);
}



/* print out all of the games currently avaliable */

print_list_of_games(player)
PlayerInfo *player;
{
  GameInfo *record;
  int count = 0, i;

  printf("===Game======Location===========Map======================Size==");
  printf("Started=========\n");

  if (!GAME_LIST_EXISTS) read_game_list_file(player->gamelist_file);

  /* print contents of each game record */
  for (record = glist->next; record; record=record->next) {
	printf("   %s  %s  %s  %s  %s\n",      left_justify(record->name, 8),
	       left_justify(record->host, 17), left_justify(record->map, 23),
	       left_justify_number(record->size, 4),
	       left_justify(record->time, 14));
	/* print a little info on each of the players */
	for (i=0; i<record->size; i++) {
  	  printf("       ( %s  -  %s@%s )\n", record->player[i].name,
		 record->player[i].login, record->player[i].host);
	}
	count++;
  }

  printf("===============================================================");
  printf("================\n");

  if (!count) {
    printf("\nUse the driver program to start up a game.\n");
  }
}



/* return an array of all of the games currently avaliable, with the lines
   nice and formatted.  This result must be copied to preserve it, as it
   will be freed/overwritten on the next call.  The end of the list
   is denoted by a NULL pointer.  A call to this routine forces a re-reading
   of the list. */

char **return_list_of_games(player)
PlayerInfo *player;
{
  GameInfo *record;
  int count = 0;
  char s[250];
  static char **result = (char **) NULL;

  /* count the number of games in the list */
  read_game_list_file(player->gamelist_file);
  for (record=glist->next; record; record=record->next) count++;

  /* allocate an array for the lines of text */
  if (result) free(result);
  result = (char **) malloc((sizeof(char *) * count) + 3);
  if (!result) Gerror("not enough memory for list of games");
  
  /* print contents of list into strings and put into array */
  sprintf(s, "===Game======Location===========Map======================Size==Started=========");
  result[0] = create_string(s);
  count = 1;
  for (record = glist->next; record; record=record->next) {
	sprintf(s, "   %s  %s  %s  %s  %s", left_justify(record->name, 8),
	       left_justify(record->host, 17), left_justify(record->map, 23),
	       left_justify_number(record->size, 4),
	       left_justify(record->time, 14));
	result[count] = create_string(s);
	count++;
  }
  sprintf(s, "===============================================================================");
  result[count] = create_string(s);
  count++;
  result[count] = (char *) NULL;

  return result;
}



/* return a pointer to a game record given the name of the game we are
   looking for.  Return NULL if there is no game by that name.
   This does not cause gaame list to be re-read. */

GameInfo *get_game_by_name(player, name)
PlayerInfo *player;
char *name;
{
  GameInfo *record, *result = NULL;

  if (!GAME_LIST_EXISTS) read_game_list_file(player->gamelist_file);

  /* look through all games until we find one with given name */
  for (record=glist->next; record && !result; record = record->next)
    if (!strcmp(record->name, name)) {
      result = allocate_game_record();
      bcopy(record, result, sizeof(GameInfo));
    }

  return result;
}



/* return TRUE if a game by the given name exists already, does not call
   for a re-read of the gamelist. */

int game_exists(player, name)
PlayerInfo *player;
char *name;
{
  GameInfo	*record;
  int		exists = FALSE;

  if (!GAME_LIST_EXISTS) read_game_list_file(player->gamelist_file);

  /* look through all games until we find one with given name */
  for (record=glist->next; record && !exists; record = record->next)
    if (!strcmp(record->name, name)) exists = TRUE;

  return exists;
}



/* add a game to the file's list, given all the pertinent info about it in
   a GameInfo record, if there is a game by same name already, then return
   FALSE, otherwise, if addition was successful return TRUE.  Does re-read
   the gamelist file. */

int add_game(player, record)
PlayerInfo *player;
GameInfo *record;
{
  GameInfo *current, *new;

  /* if no game file exists at the moment, create it with our one record */
  if (!game_file_exists(player->gamelist_file)) {
    create_game_file(player, record);
    return TRUE;
  }

  /* load game list into memory */
  read_game_list_file(player->gamelist_file);

  /* look for a game by the same name already existing */
  if (game_exists(player, player->our_game)) return FALSE;

  /* make a copy of our game and add to the beginning */
  new = allocate_game_record();
  bcopy(record, new, sizeof(GameInfo));
  current = glist->next;
  glist->next = new;
  new->next = current;

  /* write out the new list to the file */
  write_game_list_file(player->gamelist_file);
  free_game_list();
  return TRUE;
}



/* remove our game from the file, we'll use its name to search for it,
   causes a re-reading of the gamelist file. */

remove_game(player)
PlayerInfo *player;
{
  GameInfo *current, *previous;
  
  read_game_list_file(player->gamelist_file);

  /* go through entire list, deleting all game records with our name */
  previous = glist;
  for (current=glist->next; current; current = current->next) {
    if (!strcmp(current->name, player->our_game)) {
      previous->next = current->next;
      free(current);
      current = previous;
    }
    else previous = previous->next;
  }

  /* write the updated list back out to file */
  write_game_list_file(player->gamelist_file);
  free_game_list();
}



/* update a game record in the file, we shall look for the record by its
   name, and if found will replace its record with the one we are given.
   Causes a re-read of the gamelist. */

update_game(player, record)
PlayerInfo *player;
GameInfo *record;
{
  GameInfo *current, *saveptr;
  
  if (GAME_LIST_EXISTS) free_game_list();
  read_game_list_file(player->gamelist_file);

  /* go through entire list until we find ours, then replace it */

  for (current = glist->next; current; current = current->next)
    if (!strcmp(current->name, record->name)) {
      saveptr = current->next;
      bcopy(record, current, sizeof(GameInfo));
      current->next = saveptr;
    }
      
  /* write out the updated list to file */
  write_game_list_file(player->gamelist_file);
  free_game_list();
}




/* ====================== Internally used procedures ================= */

/* return 1 if the game file exists */

int game_file_exists(filename)
char *filename;
{
  FILE *gfile;
  int result = 0;

  gfile = fopen(filename, "r");
  if (gfile != NULL) {
    result = 1;
    fclose(gfile);
  }

  return result;
}



read_game_list_file(name)
/* looks for a game list file from either disk or from a driver running
   on a certain host. */
char *name;
{
  if (strncmp(name, HOST_SIGNIFIER, strlen(HOST_SIGNIFIER))) {
    read_game_list_from_disk(name);
  }
  else {
    read_game_list_from_host((char *)(name + strlen(HOST_SIGNIFIER)));
  }
}



read_game_list_from_disk(filename)
/* open the game list file and read in all the games in the list, putting
   them into a linked list in memory */
char *filename;
{
  FILE *fp;
  GameInfo *record, *last;
  int i;

  /* initialize list header */
  if (GAME_LIST_EXISTS) free_game_list();
  glist = &dummyrec;
  dummyrec.next = NULL;
  last = &dummyrec;

  /* if there is no game file, there is no list */
  fp = fopen(filename, "r");
  if (fp == NULL) return;

  /* read all the records avaliable */
  do {
    record = allocate_game_record();
    i = fread(record, sizeof(GameInfo), 1, fp);

    if (i == 1) {
      last->next = record;
      record->next = NULL;
      last = record;
    }
    else free(record);
    
  } while (i == 1);

  /* close the file, we are done */
  fclose(fp);
}



read_game_list_from_host(hostname)
/* open a connection to driver on a given host and get the game list
   that driver knows about */
char *hostname;
{
  printf("get from host %s\n", hostname);
}



create_game_file(player, record)
/* create a game list file from scratch when it doesn't yet exist
   on the file system */
PlayerInfo *player;
GameInfo *record;
{
  FILE *fp;
  char s[250];

  fp = fopen(player->gamelist_file, "w");
  if (fp == NULL) {
    fprintf(stderr, "Warning: could not start game list file\n");
    return;
  }

  /* write out the one record we have */
  fwrite(record, sizeof(GameInfo), 1, fp);

  /* close file, we are done */
  fclose(fp);

  /* change files access mode to allow all to read it */
  sprintf(s, "chmod 644 %s", player->gamelist_file);
  system(s);
}



/* write the current gamelist file out to file */

write_game_list_file(filename)
char *filename;
{
  FILE *fp;
  GameInfo *record;
  char s[250];

  if (!GAME_LIST_EXISTS) {
	printf("Warning: tried to write invalid game list to file\n");
	return;
  }

  /* if we cannot create game file, then signal error */
  fp = fopen(filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "Warning: could not write game list file\n");
    return;
  }

  /* write all the records in the list (except dummy record) */
  for (record = glist->next; record != NULL; record = record->next)
    fwrite(record, sizeof(GameInfo), 1, fp);
    
  /* close the file, we are done */
  fclose(fp);

  /* change files access mode to allow all to read it */
  sprintf(s, "chmod 644 %s", filename);
  system(s);
}




/* free all the records used for storing the game list */

free_game_list()
{
  GameInfo *record, *previous = NULL;

  if (!glist) return;
  for (record = glist->next; record; record = record->next) {
    if (previous != NULL) free(previous);
    previous = record;
  }

  if (previous != NULL) free(previous);
  glist = NULL;
}

    

/* allocate in memory a place for a GameInfo record */

GameInfo *allocate_game_record()
{
  GameInfo *result;

  result = (GameInfo *) malloc(sizeof(GameInfo));
  if (result == NULL) Gerror("no room for game list record");

  return result;
}
