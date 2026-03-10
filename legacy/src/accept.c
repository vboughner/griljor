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

/* Driver routines to check whether he accepts a new player */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "def.h"
#include "objects.h"
#include "map.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "password.h"
#include "level.h"

/* global variables for this file only */
PersonRec	*stored_person[MAX_GAME_SIZE];/* info about persons */
int		persons_loaded = FALSE;		/* TRUE when info loaded */

/* procedure declarations */
PersonRec *alloc_person_record();



/* decide whether player should be allowed to play in this game */

make_acceptance_pack(info_pack, acc_pack)
DriverPack *info_pack;
AcceptancePack *acc_pack;
{
  char msg[MSG_LENGTH];
  int a;

  /* place driver's datagram socket address into the packet */
  bcopy(&ownaddr, &(acc_pack->address), sizeof(Sockaddr));

  /* if this game is over or ending, tell him so */
  if (IS_GAME_ENDING) {
    sprintf(msg, "%s server: sorry - game just ended, restart in progress", PROGRAM_NAME);
    strcpy(acc_pack->msg, msg);
    acc_pack->accepted = FALSE;
    return;
  }

  /* if this is a monster, accept him without question */
  if (info_pack->info.what_am_i == MONSTER) {
    sprintf(msg, "%s server recoginizes monster '%s'", PROGRAM_NAME,
	    info_pack->info.player_name);
    strcpy(acc_pack->msg, msg);
    acc_pack->accepted = TRUE;
    return;
  }

  /* load the stored versions of persons for this player */
  load_stored_persons(info_pack);

  /* if passwords are bad, say so */
  if ((a = check_passwords(info_pack)) == FALSE) {
    sprintf(msg, "%s server: sorry - incorrect password", PROGRAM_NAME);
    strcpy(acc_pack->msg, msg);
    acc_pack->accepted = FALSE;
  }
  else {
    /* otherwise place welcome message into packet */
    if (a == 2) 
      sprintf(msg, "%s server creating new character '%s'", PROGRAM_NAME,
	      info_pack->info.player_name);
    else
      sprintf(msg, "%s server recognizes '%s'", PROGRAM_NAME,
	      info_pack->info.player_name);
    strcpy(acc_pack->msg, msg);
    acc_pack->accepted = TRUE;

    /* add new persons to password file if there are any */
    add_new_persons(info_pack);
  }
}

/*****************************************************************/
/* read the password file and find out the persons relative rank */

calculate_rank(info)
DriverPack *info;
{
  int total,better,index,i;
  double ratio,their;
  PersonLink *end,*loop,*data = Load_File(&end,TRUE);

  for (i = 0; i < info->info.num_persons;i++)
    if (info->newperson[i].listed)
      {
	if ((info->newperson[i].losses == 0)||
	    (info->newperson[i].kills < MIN_RANK_KILLS))
	  strcpy(info->newperson[i].rank,NORANK);
	else
	  {
	    ratio = ((double) info->newperson[i].kills) /
	      ((double) info->newperson[i].losses);
	    total = 0;
	    better = 0;
	    loop = data;
	    while(loop)
	      {
		if ((loop->person->losses > 0)&&
		    (loop->person->kills >= MIN_RANK_KILLS))
		  if (loop->person->date > (time(NULL) - TIME_TO_THROW_AWAY))
		    {
		      their = ((double) loop->person->kills) / 
			((double) loop->person->losses);
		      
		      if (their > ratio)
			better++;
		      total++;
		    }
		loop = loop->next;
	      }
	    
	    if (total == 0)
	      total = 1;
	    
	    ratio = (double) (total-better) / (double) total;
	    index = floor((double) (NUM_OF_RANKS-1) * ratio);

	    sprintf(info->newperson[i].rank,"%s %d/%d",RANK_NAME[index],
		    (total-better),total);
	  }
      }
  free_link(data);
}

/*****************************************************************/

/* pick up any saved information given person */

pickup_saved_person_info(pack, pnum)
DriverPack *pack;
int pnum;
{
  if (stored_person[pnum] == NULL)
    pack->newperson[pnum].loaded = FALSE;
  else {
    if (stored_person[pnum]->bitmap_saved) {
      /* if there is a stored bitmap then load it in */
      pack->newperson[pnum].loadbits = TRUE;
      bitmapcpy(pack->newperson[pnum].bitmap, stored_person[pnum]->bitmap);
      bitmapcpy(pack->newperson[pnum].mask, stored_person[pnum]->mask);
    }
    if (LEVELS_SAVED) 
      pack->newperson[pnum].experience = stored_person[pnum]->ExpPts;
    else
      pack->newperson[pnum].experience = BASE_EXP;

    strcpy(pack->newperson[pnum].rank, stored_person[pnum]->rank);
    pack->newperson[pnum].deity = stored_person[pnum]->deity;
    pack->newperson[pnum].loaded = TRUE;
  }

  return;
}

/*****************************************************************/

/* load the information on this player's persons */

load_stored_persons(pack)
DriverPack *pack;
{
  register int i;

  for (i=0; i < pack->info.num_persons; i++)
    stored_person[i] = get_player(pack->newperson[i].name);
}



/* Check the players given password against passwords on file.  If the
   password on file is of length zero, then any password will do.
   Return FALSE if there are any discrepencies, 1 if everything
   is fine, or 2 if a new person is being created.
   For as yet undefined persons, any password is fine. */

int check_passwords(pack)
DriverPack *pack;
{
  register int	i;
  int		okay = 1, new = 0;

  for (i=0; i < pack->info.num_persons; i++)
    if (stored_person[i]) {
      if (strlen(stored_person[i]->password))
        if (strcmp(stored_person[i]->password, pack->info.password))
	  okay = 0;
    }
    else new = 1;

  if (okay) return (okay + new);
  else return FALSE;
}




/* add any new persons to the password file, now that we know what password
   they are going to want to use */

add_new_persons(pack)
DriverPack *pack;
{
  PersonRec *new;
  register int i;

  for (i=0; i < pack->info.num_persons; i++)
    if (stored_person[i] == NULL) {
      new = alloc_person_record();

      /* initialize a new person for the password file */
      strcpy(new->name, pack->newperson[i].name);
      strcpy(new->password, pack->info.password);
      strcpy(new->login, pack->info.login);
      strcpy(new->host, pack->info.host);
      new->date = time(NULL);
      new->ExpPts = BASE_EXP;
      strcpy(new->rank, RANK_NAME[0]);
      new->bitmap_saved = pack->newperson[i].savebits;
      if (new->bitmap_saved) {
        bitmapcpy(new->bitmap, pack->newperson[i].bitmap);
	bitmapcpy(new->mask, pack->newperson[i].mask);
      }
      else {
        bzero(new->bitmap, BITMAP_ARRAY_SIZE * sizeof(char));
        bzero(new->mask, BITMAP_ARRAY_SIZE * sizeof(char));
      }
      new->kills = 0;
      new->losses = 0;
      new->games = 0;
      new->deity = FALSE;

      /* add person to the password file, and store for our own records */
      add_player(new);
      stored_person[i] = new;
    }
    else {
      /* check for person having new bitmaps for us to save */
      if (pack->newperson[i].savebits) {
        bitmapcpy(stored_person[i]->bitmap, pack->newperson[i].bitmap);
	bitmapcpy(stored_person[i]->mask, pack->newperson[i].mask);
	stored_person[i]->bitmap_saved = TRUE;
	modify_player(stored_person[i]);
      }
    }
}




/* given person id numbers now, we can place the password file info
   into the game person array */

copy_passinfo_into_person(pack)
DriverPack *pack;
{
  int i, pnum;

  for (i=0; i < pack->info.num_persons; i++)
    if (stored_person[i]) {
      pnum = person_array(pack->newperson[i].id);
      if (KILLS_SAVED) {
        gameperson[pnum]->kills =
          pack->newperson[i].kills = stored_person[i]->kills;
        gameperson[pnum]->losses =
          pack->newperson[i].losses = stored_person[i]->losses;
      }
      else {
        gameperson[pnum]->kills = 0;
        gameperson[pnum]->losses = 0;
      }
      if (LEVELS_SAVED)
        gameperson[pnum]->level = experience_level(stored_person[i]->ExpPts);
      else 
        gameperson[pnum]->level = experience_level(BASE_EXP);

      gameperson[pnum]->games =
        pack->newperson[i].games = stored_person[i]->games;
      gameperson[pnum]->deity = stored_person[i]->deity;
    }
}



/* allocate a person record */

PersonRec *alloc_person_record()
{
  PersonRec *new;

  new = (PersonRec *) malloc(sizeof(PersonRec));
  if (new == NULL) Gerror("no memory for personal statistics");

  bzero(new, sizeof(PersonRec));
  return new;
}
