/* routines in this file mainly by Albert C. Baker III */

#include <time.h>
#include <stdio.h>
#include "config.h"
#include "def.h"
#include "password.h"
#include "lib.h"
#define MAXSTRLEN 50
#define EVER (;;)



int ShowUsers(file, pause)
PersonLink *file;
int pause;
/* prints out users' names, with numbers. returns number printed */
{
  int i = 0;
  for (; file ; file = file->next, i++) {
    printf("%4d: %-25s %7d %s@%s %s\n",
	   i+1,
	   file->person->name,
	   file->person->ExpPts,
	   file->person->login,
	   file->person->host,
	   (file->person->deity) ? "[Deity]" : "");
    if (pause && (i % 22) == 21) getchar();
  }
  return i;
}



void DeleteUser(file, user)
PersonLink *file;
int user;
/* removes user (by number) from file list, note that because of how this
   is implemented, you cannot delete user #1 (the first one in the list) */
{
  PersonLink *i,*follow;
  int curnum = 1;
  demand((user != 1), "DeleteUser: bug: cannot delete user #1");

  for (i = file; i && user != curnum ; follow = i, i = i->next,curnum++);

  follow->next = i->next;
  free(i->person);
  free(i);
}



void GetString(prompt, target, length)
char *prompt, *target;
int length;
{
  char result[500];
  char *temp;
  for EVER {
    printf("%s",prompt);
    fgets(result,length-1,stdin);
    if (*result == '\0') 
      printf("\nBad String Input\n");
    else break;
  }
  strncpy(target,result,length);
  for(temp = target; *temp != '\n' ; temp++);
  *temp = '\0';
}



void GetNum(prompt, target)
char *prompt;
int *target;
{
  char result[MAXSTRLEN];
  for EVER {
    printf("%s",prompt);
    if (fgets(result,MAXSTRLEN-1,stdin)) break;
  }
    *target = atoi(result);
}



void DoBitMap(person, type)
PersonLink *person;
char type;
{
  char line[MAXSTRLEN];
  int width,height,x_hot,y_hot;
  char *data;
  char *pic = ((type == 'b') ? 
		person->person->bitmap : 
		person->person->mask);
  char *p1 = ((type == 'b') ?
		"\nIs this BitMap OK? (s to save it to disk) " :
		"\bIs this Mask OK? (s to save it to disk) ");
  char *p2 = ((type == 'b') ?
	      "\nBitMap File Name: " :
	      "\nMask File Name ");
  for EVER {
    if (person->person->bitmap_saved)
      textbit(32,32,pic);
    else
      printf("\nNo BitMap Specified\n");

    GetString(p1,line,MAXSTRLEN);
    if (*line == 'y' || *line == 'Y') break;
    if ((*line == 's' || *line == 'S') && person->person->bitmap_saved) {
      GetString(p2,line,MAXSTRLEN);
      if (XXWriteBitmapFile(line,person->person->name,32,32,pic,-1,-1))
         printf("Bitmap written to file %s\n",line);
      continue;
    }
    GetString(p2,line,MAXSTRLEN);
    if (XXReadBitmapFile(line,&width,&height,&data,&x_hot,&y_hot))
      if (width == (height == 32)) {
        bcopy(data,pic,BITMAP_ARRAY_SIZE);
        person->person->bitmap_saved = TRUE;
      }
      else 
        printf("\nBitmap not 32x32\n");
    else
      printf("\nerror in file-read\n");
  }
}


void EditUser(file, user)
PersonLink *file;
int user;
/* given user number user, prompts for editing of that person's
 * attributes and changes the information contained in 
 * the link. 
 * PC: user is a legal (in-bounds) user */
 
{
  char line[MAXSTRLEN];
  int curnum = 1;
  struct tm *tdate = localtime(&file->person->date);
  for (;file && (user != curnum) ; file = file->next,curnum++);

  for EVER {
    printf("login name: %s@%s\n",file->person->login,file->person->host);
    printf("last time used: %s\n", asctime(tdate));
    printf("1 name: %s\n",file->person->name);
    printf("2 password: %s\n" ,file->person->password);
    printf("3 Exp Points: %d\n",file->person->ExpPts);
    printf("4 Rank: %s\n",file->person->rank);
    printf("5 Kills: %d\n",file->person->kills);
    printf("6 Losses: %d\n",file->person->losses);
    printf("7 Games Won: %d\n",file->person->games);
    printf("8 Deity Mode: %s\n",file->person->deity ? "On" : "Off");
    printf("b change bitmap\n");
    printf("m change mask\n");
    printf("q quit to previous level\n");

    printf("\nFunction? > ");
    fgets(line,MAXSTRLEN-1,stdin);
    curnum = atoi(line);
    if (curnum == 0  &&
	*line != 'b' &&
	*line != 'm' &&
	*line != 'q') {
      printf("\nBad Command\n");
      continue;
    }
    if (*line == 'q') break;
    if (*line == 'm' || *line == 'b') {
      DoBitMap(file,*line);
      continue;
    }
    switch (curnum) {
      case 1: 
	GetString("New Name: ",file->person->name,NAME_LENGTH);
	break;
      case 2: 
	GetString("New Password: ",file->person->password,PASSWORD_LENGTH);
	break;
      case 3: 
	GetNum("Experience Points: ",&file->person->ExpPts);
	break;
      case 4: 
	GetString("New Rank: ",file->person->rank,RANK_LENGTH);
	break;
      case 5: 
	GetNum("Number of kills: ",&file->person->kills);
	break;
      case 6:
	GetNum("Number of losses: ",&file->person->losses);
	break;
      case 7: 
	GetNum("Number of games won: ",&file->person->games);
	break;
      case 8:
	file->person->deity = !(file->person->deity);
        printf("Deity Mode now: %s\n",file->person->deity ? "On" : "Off");
    }
  }
}



/* print out the user usage help for editpass */
int print_editpass_usage(argc, argv)
int argc;
char **argv;
{
  printf("\nUsage:  %s [-text] [filename]\n", argv[0]);
  printf("  [-text]      run text version rather than graphic version\n");
  printf("  [filename]   option filename to use rather than default\n");
  printf("\n");
  return 0;
}



/* this main() is called when the user opts to do password file editing
   in a textual way (ie. from a remote login etc.) */
int textual_main(argc, argv)
int argc;
char **argv;
{
  PersonLink *file, *end;
  char line[MAXSTRLEN];
  int MaxUsers,user;

  if (argc == 3) set_password_file(argv[2]);

  /* load the file without locking it */
  file = Load_File(&end, FALSE);
  demand (file,"File not found.");

  /* convert integer dates to strings and place into the records */
  update_all_string_dates(file);

  MaxUsers = ShowUsers(file, FALSE);

  for EVER {
    printf("\n(s)how users, (n)o pause display, (e)dit, (D)elete or (q)uit: ");
    fgets(line,MAXSTRLEN-1,stdin);
    if (*line == 'D') {
      printf("\nWhich person would you like to delete? ");
      fgets(line,MAXSTRLEN-1,stdin);
      user = atoi(line);
      if (user <= 0 || user > MaxUsers) {
        printf("Bad user number. Try again\n");
        continue;
      }
      DeleteUser(file,user);
    }
    else if (*line == 'e') {
      printf("\nWhich person would you like to edit? ");
      fgets(line,MAXSTRLEN-1,stdin);
      user = atoi(line);
      if (user <= 0 || user > MaxUsers) {
        printf("Bad user number. Try again\n");
        continue;
      }
      EditUser(file,user);
    }
    else if (*line == 'q') break;
    else if (*line == 's') MaxUsers = ShowUsers(file, TRUE);
    else if (*line == 'n') MaxUsers = ShowUsers(file, FALSE);
  }


  printf("\nmake changes permanent? ");
  fgets(line,MAXSTRLEN-1,stdin);
  if (*line == 'y' || *line == 'Y') Write_File(file);

  return 0;
}
