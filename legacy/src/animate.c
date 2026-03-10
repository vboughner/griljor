#include <stdio.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "credits.h"
#include "animate.h"
#include "free.h"
#include "bitmaps/tombbit"
#include "bitmaps/tombmask"
#include "bitmaps/skullbit"
#include "bitmaps/skullmask"

extern Pixmap background;
extern GC drawGC;
extern GC maskGC;

static Pixmap tmask = NULL;
static Pixmap tbit = NULL;

static Pixmap smask = NULL;
static Pixmap sbit = NULL;

static int mymap[40][20] = { 
  {2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, /* col0 */
  {2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, /* col1 */
  {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3}, /* col2 */
  {2, 2, 2, 2, 2, 2, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3}, /* col3 */
  {0, 0, 0, 2, 2, 2, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3}, /* col4 */
  {0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3}, /* col5 */
  {0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3}, /* col6 */
  {2, 0, 0, 1, 0, 0, 0, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3}, /* col7 */
  {2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 3, 3, 3, 3, 3, 3, 3, 1, 3}, /* col8 */
  {2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 4, 4, 3, 3, 3, 3, 3, 3, 1, 3}, /* col9 */
  {2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 3, 3, 3, 3, 3, 1, 3}, /* col10 */
  {2, 2, 2, 1, 2, 2, 2, 2, 0, 0, 0, 0, 4, 3, 3, 3, 3, 3, 1, 2}, /* col11 */
  {2, 2, 2, 1, 2, 2, 2, 4, 0, 0, 0, 0, 4, 3, 3, 3, 3, 3, 1, 2}, /* col12 */
  {2, 2, 1, 1, 2, 2, 4, 4, 4, 0, 0, 0, 4, 3, 3, 3, 3, 2, 1, 2}, /* col13 */
  {2, 2, 1, 2, 2, 4, 0, 0, 0, 0, 0, 0, 4, 3, 2, 2, 2, 2, 1, 2}, /* col14 */
  {2, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 2, 2, 2, 2, 1, 2}, /* col15 */
  {2, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 2, 2, 2, 2, 1, 2}, /* col16 */
  {3, 3, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 2, 2, 2, 2, 1, 2}, /* col17 */
  {3, 3, 1, 4, 0, 0, 0, 0, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2}, /* col18 */
  {3, 3, 1, 4, 0, 0, 0, 0, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2}, /* col19 */
  {3, 3, 1, 4, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2}, /* col20 */
  {3, 3, 1, 4, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2}, /* col21 */
  {3, 3, 1, 4, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2}, /* col22 */
  {3, 3, 1, 4, 0, 0, 0, 0, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 2}, /* col23 */
  {3, 3, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 2, 2, 2, 1, 2}, /* col24 */
  {3, 3, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 1, 2}, /* col25 */
  {3, 3, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 2}, /* col26 */
  {3, 3, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2, 2}, /* col27 */
  {3, 3, 1, 4, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 2, 1, 2, 2, 2}, /* col28 */
  {3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 4, 0, 0, 0, 0, 0, 1, 2, 2, 2}, /* col29 */
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 2}, /* col30 */
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0}, /* col31 */
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 4, 4, 0, 0, 0, 0, 1, 0, 0, 0}, /* col32 */
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 4, 4, 4, 4, 0, 0, 1, 0, 0, 0}, /* col33 */
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}, /* col34 */
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, /* col35 */
  {3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, /* col36 */
  {3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, /* col37 */
  {3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, /* col38 */
  {3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}  /* col39 */
};

static Player *mypos[40][20];

int randNum(range)
     int range;
{
  int trevRand;
  static int seeded = 0;

  if(!seeded) {
    srand48(time(NULL));
    seeded = 1;
  }

  trevRand = (int)lrand48();
  return trevRand % range;
}



void getRandPos(player)
     Player *player;
{
  int x, y;

  do {
     x = randNum(40);
     y = randNum(20);
   } while(mypos[x][y] || !mymap[x][y]);

  player->curx = x;
  player->cury = y;
  player->direction = 0;
  player->dead = 0;
  mypos[x][y] = player;
}



void getSpeed(player)
     Player *player;
{
  int val;

  val = player->direction;
  if(player->dead)
    return;

  switch(val) {
  case 0:
    mypos[player->curx][player->cury] = player;
    break;
  case 1:
    mypos[player->curx][player->cury] = NULL;
    (player->curx)++;
    mypos[player->curx][player->cury] = player;
    break;
  case -1:
    mypos[player->curx][player->cury] = NULL;
    (player->curx)--;
    mypos[player->curx][player->cury] = player;
    break;
  case 2:
    mypos[player->curx][player->cury] = NULL;
    (player->cury)++;
    mypos[player->curx][player->cury] = player;
    break;
  case -2:
    mypos[player->curx][player->cury] = NULL;
    (player->cury)--;
    mypos[player->curx][player->cury] = player;
    break;
  default:
    printf("getSpeed: Unknown direction %d for %s\n", val, player->name);
    break;
  }

  val = mymap[player->curx][player->cury];

  switch(val) {
  case 1:
    player->inc = 8;
    player->changeme = 4;
    break;
  case 2:
  case 3:
    player->inc = 4;
    player->changeme = 8;
    break;
  case 4:
    player->inc = 2;
    player->changeme = 16;
    break;
  default:
    printf("getSpeed: Invalid movement %d for %s\n", val, player->name);
    exit(1);
  }
}



void getNewDirection(player)
     Player *player;
{
  int val, num, num2, inc;
  int dirs[4];
  int nummatch = 0;
  int done = 0;

  val = player->direction;
  num = randNum(6);
  if(!num) {
    player->direction = 0;
    return;
  }

  if(mymap[player->curx+1][player->cury] == 1 &&
     player->direction != -1) {
    dirs[nummatch] = 0;
    nummatch++;
  }
  if(mymap[player->curx-1][player->cury] == 1 &&
     player->direction != 1) {
    dirs[nummatch] = 1;
    nummatch++;
  }
  if(mymap[player->curx][player->cury+1] == 1 &&
     player->direction != -2) {
    dirs[nummatch] = 2;
    nummatch++;
  }
  if(mymap[player->curx][player->cury-1] == 1 &&
     player->direction != 2) {
    dirs[nummatch] = 3;
    nummatch++;
  }

  if(nummatch) {
    num = dirs[randNum(nummatch)];
  }
  else {
    num = randNum(4);
  }

  if(player->curx+1 < 40 &&
     mypos[player->curx+1][player->cury]) {
    (mypos[player->curx+1][player->cury])->dead = 1;
    mypos[player->curx+1][player->cury] = NULL;
  }
  if(player->curx-1 >= 0 &&
     mypos[player->curx-1][player->cury]) {
    (mypos[player->curx-1][player->cury])->dead = 1;
    mypos[player->curx-1][player->cury] = NULL;
  }
  if(player->cury+1 < 20 &&
     mypos[player->curx][player->cury+1]) {
    (mypos[player->curx][player->cury+1])->dead = 1;
    mypos[player->curx][player->cury+1] = NULL;
  }
  if(player->cury-1 >= 0 &&
     mypos[player->curx][player->cury-1]) {
    (mypos[player->curx][player->cury-1])->dead = 1;
    mypos[player->curx][player->cury-1] = NULL;
  }

  inc = 4;
  while(inc-- && !done) {
    switch(num) {
    case 0:
      if(val == 1)
	break;
      if(val == -1) {
	num2 = randNum(5);
	if(num2)
	  break;
      }
      if(player->curx+1 < 40 &&
	 mymap[player->curx+1][player->cury] &&
	 !(mypos[player->curx+1][player->cury])) {
	player->direction = 1;
	done = 1;
      }
      break;
    case 1:
      if(val == -1)
	break;
      if(val == 1) {
	num2 = randNum(5);
	if(num2)
	  break;
      }
      if(player->curx-1 >= 0 &&
	 mymap[player->curx-1][player->cury] &&
	 !(mypos[player->curx-1][player->cury])) {
	player->direction = -1;
	done = 1;
      }
      break;
    case 2:
      if(val == 2)
	break;
      if(val == -2) {
	num2 = randNum(5);
	if(num2)
	  break;
      }
      if(player->cury+1 < 20 &&
	 mymap[player->curx][player->cury+1] &&
	 !(mypos[player->curx][player->cury+1])) {
	player->direction = 2;
	done = 1;
      }
      break;
    case 3:
      if(val == -2)
	break;
      if(val == 2) {
	num2 = randNum(5);
	if(num2)
	  break;
      }
      if(player->cury-1 >= 0 &&
	 mymap[player->curx][player->cury-1] &&
	 !(mypos[player->curx][player->cury-1])) {
	player->direction = -2;
	done = 1;
      }
      break;
    default:
      printf("Oops\n");
      break;
    }
    num++;
    if(num > 3)
      num = 0;
    if(!inc && !done) {
      player->direction = 0;
    }
  }
}
      

void getDirection(player)
     Player *player;
{
  int val, num;

  val = player->direction;

  if(player->dead == 1) {
    if(player->drawdead == 1) {
      player->drawdead = 0;
      player->dead = 2;
      player->changeme = 64;
      player->direction = 0;
      player->inc = 0;
    }
    else {
      player->changeme = 4;
      player->direction = 0;
      player->inc = 0;
    }
  }
  else {
    if(player->dead == 2)
      getRandPos(player);

    switch(val) {
    case 0:
      num = randNum(6);
      if(num) {
	getNewDirection(player);
      }
      break;
    case 1:
    case -1:
      if(player->curx + val > 39 || player->curx + val < 0) {
	getNewDirection(player);
      }
      else if(!(mymap[player->curx + val][player->cury])) {
	getNewDirection(player);
      }
      else if(mypos[player->curx + val][player->cury]) {
	(mypos[player->curx + val][player->cury])->dead = 1;
	getNewDirection(player);
	mypos[player->curx + val][player->cury] = NULL;
      }
      else if(mymap[player->curx][player->cury] !=
	      mymap[player->curx + val][player->cury]) {
	if(mymap[player->curx][player->cury] == 1) {
	  num = 0;
	}
	else {
	  num = randNum(3);
	}
	if(!num) {
	  getNewDirection(player);
	}
      }
      else {
	if(mymap[player->curx+val][player->cury] == 1)
	  num = randNum(12);
	else
	  num = randNum(6);
	if(!num) {
	  getNewDirection(player);
	}
      }
      break;
    case 2:
    case -2:
      val = val/2;
      if(player->cury + val > 19 || player->cury + val < 0) {
	getNewDirection(player);
      }
      else if(!(mymap[player->curx][player->cury+val])) {
	getNewDirection(player);
      }
      else if(mypos[player->curx][player->cury + val]) {
	(mypos[player->curx][player->cury + val])->dead = 1;
	getNewDirection(player);
	mypos[player->curx][player->cury + val] = NULL;
      }
      else if(mymap[player->curx][player->cury] !=
	      mymap[player->curx][player->cury + val]) {
	if(mymap[player->curx][player->cury] == 1) {
	  num = 0;
	}
	else {
	  num = randNum(3);
	}
	if(!num) {
	  getNewDirection(player);
	}
      }
      else {
	if(mymap[player->curx][player->cury+val] == 1)
	  num = randNum(12);
	else
	  num = randNum(8);
	if(!num) {
	  getNewDirection(player);
	}
      }
      break;
    default:
      printf("get Direction: Unknown direction %d for %s\n", val, player->name);
      exit(1);
    }
  }
  getSpeed(player);
}



Player *createPlayers(display, mainWindow, depth, names, numPlayers, fg, bg)
     Display *display;
     Window mainWindow;
     int depth;
     Credits *names;
     int numPlayers;
     int fg, bg;
{
  int i;
  int x, y;
  Player *start;
  Player *players;

  tbit = MyXCreatePixmapFromBitmapData(display, mainWindow, tombbit_bits, 
				       tombbit_width, tombbit_height,
				       fg, bg, depth);
  tmask = MyXCreatePixmapFromBitmapData(display, mainWindow, tombmask_bits, 
					tombmask_width, tombmask_height,
					fg, bg, depth);
  sbit = MyXCreatePixmapFromBitmapData(display, mainWindow, skullbit_bits, 
				       skullbit_width, skullbit_height,
				       fg, bg, depth);
  smask = MyXCreatePixmapFromBitmapData(display, mainWindow, skullmask_bits, 
					skullmask_width, skullmask_height,
					fg, bg, depth);

  for(x=0; x<40; x++) {
    for(y=0; y<20; y++) {
      mypos[x][y] = NULL;
    }
  }

  start = players = (Player *)mymalloc(sizeof(Player));
  i = numPlayers;

  while(i-- && names) {
    players->name = names->name;
    players->bits = names->face;
    players->mask = names->mask;
    players->restore = MyXCreatePixmap(display, mainWindow, 32, 32, depth);
    getRandPos(players);
    getDirection(players);
    players->drawdead = 0;
    names = names->next;
    if(i && names) {
      players->next = (Player *)mymalloc(sizeof(Player));
      players = players->next;
    }
    else {
      players->next = NULL;
    }
  }

  return start;
}



void doMovement(display, context, players)
     Display *display;
     GC context;
     Player *players;
{
  Player *start;
  static int first = 1;

  start = players;

  if(!first) {
    while(players) {
      XCopyArea(display, players->restore, background, context,
		0, 0, 32, 32, players->locx, players->locy);

      if(!(players->changeme)) {
	getDirection(players);
      }

      switch(players->direction) {
      case 0:
	players->locx = 32*(players->curx);
	players->locy = 32*(players->cury);
	break;
      case 1:
	players->locx = 32*(players->curx) - 
	  (players->changeme)*(players->inc);
	players->locy = 32*(players->cury);
	break;
      case -1:
	players->locx = 32*(players->curx) +
	  (players->changeme)*(players->inc);
	players->locy = 32*(players->cury);
	break;
      case 2:
	players->locx = 32*(players->curx);
	players->locy = 32*(players->cury) - 
	  (players->changeme)*(players->inc);
	break;
      case -2:
	players->locx = 32*(players->curx);
	players->locy = 32*(players->cury) +
	  (players->changeme)*(players->inc);
	break;
      default:
	break;
      }
      (players->changeme)--;
      players = players->next;
    }
  }
  players = start;
  while(players) {
    XCopyArea(display, background, players->restore, context, 
	      players->locx, players->locy, 32, 32, 0, 0);
    players = players->next;
  }

  players = start;
  while(players) {
    if(players->dead == 2) {
      XCopyArea(display, tmask, background, maskGC, 
		0, 0, 32, 32, players->locx, players->locy);
      XCopyArea(display, tbit, background, drawGC, 
		0, 0, 32, 32, players->locx, players->locy);
    }
    else {
      XCopyArea(display, players->mask, background, maskGC, 
		0, 0, 32, 32, players->locx, players->locy);
      XCopyArea(display, players->bits, background, drawGC, 
		0, 0, 32, 32, players->locx, players->locy);
      if(players->dead == 1) {
	players->drawdead = 1;
	XCopyArea(display, smask, background, maskGC, 
		  0, 0, 32, 32, players->locx, players->locy);
	XCopyArea(display, sbit, background, drawGC, 
		  0, 0, 32, 32, players->locx, players->locy);
      }
    }
      
    players = players->next;
  }
  first = 0;
}  
