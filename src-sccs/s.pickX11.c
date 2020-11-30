h58513
s 00001/00002/01009
d D 1.6 92/09/03 16:55:16 vbo 6 5
c neutrals team is now named in the team array
e
s 00000/00000/01011
d D 1.5 92/08/07 01:03:13 vbo 5 4
c source copied to a separate tree for work on new map and object format
e
s 00002/00002/01009
d D 1.4 91/11/25 20:44:22 labc-4lc 4 3
c minor changes
e
s 00022/00006/00989
d D 1.3 91/09/27 20:02:20 labc-4lc 3 2
c added the connect to specif host feature
e
s 00001/00001/00994
d D 1.2 91/08/30 01:32:02 vanb 2 1
c made windows code color compatible
e
s 00995/00000/00000
d D 1.1 91/02/16 12:55:26 labc-3id 1 0
c date and time created 91/02/16 12:55:26 by labc-3id
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
 * January 1991
 **************************************************************************/

/* X routines used by player program for selecting games */

#include "windowsX11.h"
#include "playerX11.h"
#include "menusX11.h"
#include "def.h"
#include "extern.h"
#include "missile.h"
#include "socket.h"
#include "person.h"
#include "gamelist.h"

#include "bit/logo"
#include "bit/border"

/* variables global to this file only */

Help	*namerec = NULL, *listrec = NULL, *helprec = NULL;
char	**gamelist;	/* current list of displayed games */
Window	logo_window = NULL, setup_griljor_logo();

/* global variables for this file having to do with team selection windows */

Help	*teamtop = NULL;		/* team selection help window */
Help	*joinrec = NULL;		/* help window for game join reports */
Window	teamwin[NUM_OF_TEAMS+1];	/* windows to contain team lists */
int	win_exists[NUM_OF_TEAMS+1];	/* is TRUE for each team window up */
int tx[NUM_OF_TEAMS+1], ty[NUM_OF_TEAMS+1];
int twidth[NUM_OF_TEAMS+1], theight[NUM_OF_TEAMS+1];
int num_windows;	/* number of team windows to be made */
int localmode = TRUE;	/* TRUE when we search for local games only */
D 3
char *mode_msg = "extend game search";	/* what to display on network
					   menu line in namerec window */
E 3
I 3
char *mode_msg = "connect to specific host";	/* what to display on network
						menu line in namerec window */
E 3

/* macros concerning the game selection window, these refer to the x and
   y locations of the various menus in the window */
#define GAME_LOGO_X	20	/* Griljor logo in the corner */
#define GAME_LOGO_Y	20
#define GAME_NAME_X	GAME_LIST_X	/* Player's name etc */
#define GAME_NAME_Y	20
#define GAME_NAME_W	(NAME_LENGTH + 7)
#define GAME_NAME_H	5
#define GAME_NAME_FONT	(bigfont)
#define GAME_LIST_X	(GAME_LOGO_X + logo_width + 45)
#define GAME_LIST_Y	(GAME_NAME_Y + char_height(GAME_NAME_FONT) * \
			 GAME_NAME_H + 30)
#define GAME_HELP_X	(GAME_NAME_X + char_width(GAME_NAME_FONT) * \
			 GAME_NAME_W + 30)
#define GAME_HELP_Y	GAME_LOGO_Y
#define GAME_HELP_COLS	((PLAYER_MAIN_W - GAME_HELP_X - GAME_LOGO_X) / \
			 char_width(GAME_HELP_FONT)) /* Size of help window */
#define GAME_HELP_LINES	((GAME_NAME_H * char_height(GAME_NAME_FONT)) / \
			 (char_height(GAME_HELP_FONT)))
#define GAME_HELP_FONT	(regfont)
#define LOCAL_GAME_RECHECK 5	/* seconds between local list rechecks */


/* location and size of team selection help windows and inside contents */
#define THELP_WIN_X	((PLAYER_MAIN_W - (THELP_COLS *  \
					   char_width(THELP_FONT)) \
			  - WINDOW_BORDERS * 2) / 2)
#define THELP_WIN_Y	(GAME_LOGO_Y)
#define THELP_COLS	80
#define THELP_LINES	8	/* number of lines in team help window */
#define THELP_FONT	regfont

/* set the location of the top of the team windows, and how wide they are */
#define TEAM_WIN_Y	(THELP_WIN_Y + char_height(THELP_FONT) * \
			 THELP_LINES + WINDOW_BORDERS * 2 + 6)
#define TEAM_WIN_COLS	50
#define TEAM_WIN_FONT	tinyfont
#define TEAM_WIN_TITLE_FONT	bigfont
#define TEAM_WIN_UNDER_FONT	tinyfont
#define TEAM_WIN_W	(TEAM_WIN_COLS * char_width(TEAM_WIN_FONT) + \
			 WINDOW_BORDERS * 2)
#define TEAM_WIN_PAD	20	/* space between team windows */
#define TEAM_TITLE_H	(char_height(TEAM_WIN_TITLE_FONT) + \
			 char_height(TEAM_WIN_UNDER_FONT))/* height of title */
#define PLAYER_HEIGHT	(max((char_height(TEAM_WIN_FONT)*3), BITMAP_HEIGHT+2))
#define TEAM_WIN_HEIGHT(num)	(TEAM_TITLE_H + PLAYER_HEIGHT * num)
#define PLAYER_TOP_Y(num)	(TEAM_TITLE_H + PLAYER_HEIGHT * num)
#define PLAYER_BIT_X	2
#define PLAYER_TEXT_X	(PLAYER_BIT_X*2 + BITMAP_WIDTH)
#define JOIN_COLS	65
#define JOIN_LINES	9

/* definitions of the lower bounds of all map sizes */
#define SMALL_MAP	3
#define MEDIUM_MAP	8
#define LARGE_MAP	15
#define HUGE_MAP	30


/* ==================== G A M E   S E L E C T I O N ==================== */


/* get player to select a game from those that are going on, if a game
   was selected on the command line then try that one first, before asking.
   Return TRUE if the player selects a game, FALSE if he wants to quit. */

player_select_game(player)
PlayerInfo *player;
{
  /* initialize values in player structure */

  /* make sure that if they want to press ctrl-c, they may do so */
  comply_with_termination();

  /* select a game */
  return player_game_selection_menu(player);
}



/* handle the screen setup and input for selecting a game, return TRUE
   when the player has selected a game, or return FALSE if the player
   has decided to quit. */

int player_game_selection_menu(player)
PlayerInfo *player;
{
  Help *setup_misc_help_menu(), *setup_selection_helps();
  int redraw_game_selections();
  char **setup_game_listing();

  /* setup the help windows needed for game selection */
  GlobalRedraw = redraw_game_selections;
  gamelist = setup_game_listing(player, &listrec);
  setup_name_menu(player, &namerec);
  if (!helprec)
    helprec = setup_selection_helps(mainwin, GAME_HELP_X, GAME_HELP_Y, regfont);
  if (!logo_window) {
    logo_window = setup_griljor_logo(mainwin, GAME_LOGO_X, GAME_LOGO_Y);
    XMapWindow(display, logo_window);
    init_input_lines(logo_window);
  }
  remap_game_select_windows();

  /* get input, enter game, etc. */
  return player_game_selection_input(player);
}



/* get the and process input received in the game selection window, return
   TRUE when the player has selected a game or FALSE when he wants to quit. */

int player_game_selection_input(player)
PlayerInfo *player;
{
  int   i, x, y, detail, not_quit = TRUE;
  char  c, **setup_game_listing();
  Window happenwin;
  unsigned long t;
  static unsigned long last = 0;
  
  while (not_quit == TRUE) {
    i = get_input(mainwin, &c, &x, &y, &detail, &happenwin);

    /* handle input according to its type */
    switch (i) {
      case NOTHING:	break;
      case KEYBOARD:	break;
      case EXPOSED:	redraw_game_selections(happenwin);
			break;
      case MOUSE:	/* not_quit might be returned as GAME_JOINED here */
			not_quit =
			  handle_game_select_mouse(player, happenwin, x, y);
			break;
      case ENTER:	if (PLAYERWIN_AUTORAISE)
			  XRaiseWindow(display, mainwin);
			break;
      default:		break;
    }

    if (localmode && not_quit != GAME_JOINED) {
      t = time(NULL);
      if (t > (last + LOCAL_GAME_RECHECK)) {
        gamelist = setup_game_listing(player, &listrec);
	show_help_window(listrec);
	last = t;
      }
    }

  }
  return (not_quit == GAME_JOINED);
}



/* redraw the contents of the game selection window */
redraw_game_selections(win)
Window win;
{
  check_help_expose(win);
}


/* set up the Griljor logo in the corner */
Window setup_griljor_logo(win, x, y)
Window win;
int x, y;
{
  Window logowin;

  logowin = XCreateSimpleWindow(display, win,  x, y, logo_width, logo_height,
				WINDOW_BORDERS, fgcolor, bgcolor);
  give_window_background_bitmap(logowin, logo_bits, logo_width, logo_height);
  give_window_border_bitmap(logowin, border_bits, border_width, border_height);

  return logowin;
}


/* if *rec is NULL then set up the game listing menu for the
   first time.  If it is not NULL, then update it and redraw it,
   returning, by reference a pointer to the possibly new help win record.
   This routine returns the current game list. */

char **setup_game_listing(player, rec)
PlayerInfo *player;
Help **rec;
{
  char **list;
  int i, lines;
  
  /* get a listing of the games being played */
  list = return_list_of_games(player);
  for (lines=0,i=0; list[i]; i++) lines++;

  /* [destroy and] create the help window object to hold game listing */
  if (*rec) destroy_help_window(*rec);
  *rec = NULL;
  *rec = make_help_window(mainwin, GAME_LIST_X, GAME_LIST_Y,
			  80, lines, regfont);
  init_input_lines((*rec)->win);
  give_window_border_bitmap((*rec)->win, border_bits, border_width, border_height);

  /* load the game list lines into the new help window */
  for (i=0; i<lines; i++) set_line(*rec, i, list[i]);
}



/* if *rec is NULL then set up the name menu for the
   first time.  If it is not NULL, then update it and redraw it,
   returning, by reference a pointer to the possibly new help win record. */

setup_name_menu(player, rec)
PlayerInfo *player;
Help **rec;
{
  char s[NAME_LENGTH+40];

  /* create a help window object to contain this stuff */
  if (*rec == NULL) {
    *rec = make_help_window(mainwin, GAME_NAME_X, GAME_NAME_Y,
			    GAME_NAME_W, GAME_NAME_H, GAME_NAME_FONT);
    init_input_lines((*rec)->win);
    give_window_border_bitmap((*rec)->win, border_bits, border_width, border_height);
  }
  else XClearWindow(display, (*rec)->win);

  sprintf(s, "name: %s", player->player_name);
  set_line(*rec, 0, s);
  set_line(*rec, 1, "confirm password");
  set_line(*rec, 2, mode_msg);
  set_line(*rec, 3, "start a new game");
  set_line(*rec, 4, "exit");
}



/* set up the help window for the player picking a game.  Return the help
   record pointer created. */

Help *setup_selection_helps(win, x, y, fi)
Window win;
int x, y;
XFontStruct *fi;
{
  Help *rec;

  /* make a help window and load it with the game selection help text */
  rec = make_help_window(mainwin, x, y, GAME_HELP_COLS, GAME_HELP_LINES, fi);
  init_input_lines(rec->win);
  load_help_window_contents(rec, GAME_HELP_FILE);
  give_window_border_bitmap(rec->win, border_bits, border_width, border_height);

  return rec;
}



/* handle the mouse selections that might be made on the game selection
   screen, returns FALSE if the player has selected "quit", TRUE if he
   has selected anything else, and GAME_JOINED if he has selected and
   joined a game */

int handle_game_select_mouse(player, win, x, y)
PlayerInfo *player;
Window win;
int x, y;
{
  int i, j, not_quit = TRUE;
  char s[80], *help_window_input_line();
I 3
  static char *old_gamelist_file = NULL;
E 3
  
  if (win == namerec->win) {
    flash_line_in_help_window(namerec, which_line_selected(namerec, y));
    switch (which_line_selected(namerec, y)) {
	case 0:	strcpy(player->player_name,
		       help_window_input_line(mainwin, GAME_NAME_X + 10,
				      GAME_NAME_Y + 10, bigfont,
				      "name: ", NAME_LENGTH - 1, TRUE));
		strcpy(player->password,
		       help_window_input_line(mainwin, GAME_NAME_X + 10,
				GAME_NAME_Y + 10 , bigfont, "password: ",
				PASSWORD_LENGTH - 1, FALSE));
		setup_name_menu(player, &namerec);
		break;
        case 1: confirm_player_password(player, mainwin, GAME_NAME_X + 10,
					GAME_NAME_Y + 10);
		break;
	case 2: i = GAME_NAME_X + 10;
		j = GAME_NAME_Y + char_height(bigfont) * 3 + 10;
D 3
		strcpy(s, "Net option not yet available, press any key...");
		(void) placed_want_query(mainwin, i, j,
				i + char_width(bigfont) * (strlen(s) + 2),
				j + char_height(bigfont) * 2, s);
E 3
I 3
		strcpy(s, help_window_input_line(mainwin, i, j, bigfont,
						 "Hostname: ", 40, TRUE));
		if (strlen(s)) {
		  if (!old_gamelist_file) {
		    /* save the player's original filename for later */
		    old_gamelist_file =
		      (char *) malloc(strlen(player->gamelist_file) + 1);
		    demand(old_gamelist_file, "no memory for filename copy");
		    strcpy(old_gamelist_file, player->gamelist_file);
		  }
		  /* give the player the hostname as gamelist file pathway */
 		  strcpy(player->gamelist_file, HOST_SIGNIFIER);
		  strcat(player->gamelist_file, s);
		}
		else {
		  /* restore the player's old gamelist file pathway */
		  if (old_gamelist_file)
		    strcpy(player->gamelist_file, old_gamelist_file);
		}
E 3
		break;
	case 3: /* i = GAME_NAME_X + 10;
		j = GAME_NAME_Y + char_height(bigfont) * 3 + 10;
		strcpy(s, "Game starting option not yet available, press any key...");
		(void) placed_want_query(mainwin, i, j,
				i + char_width(bigfont) * (strlen(s) + 2),
				j + char_height(bigfont) * 2, s);
		*/
		start_a_driver(player);
		break;
	case 4: /* i = GAME_NAME_X + 10;
		   j = GAME_NAME_Y + char_height(bigfont) * 3 + 10;
		   strcpy(s, "Press Y to confirm quitting...");
		   not_quit = !placed_want_query(mainwin, i, j,
				i + char_width(bigfont) * (strlen(s) + 2),
				j + char_height(bigfont) * 2, s);
                 */
		not_quit = FALSE;
		break;
    }
  }
  else if (win == listrec->win) {
    i = which_line_selected(listrec, y);
    if (i > 0  &&  i < (listrec->height-1)) {
      /* a game has been selected from the list */
      flash_line_in_help_window(listrec, i);
      unmap_game_select_windows();
      if (make_attempt_to_join_game(player, listrec, i))
        not_quit = GAME_JOINED;
      else {
        gamelist = setup_game_listing(player, &listrec);
	close_joiner();
        remap_game_select_windows();
      }
    }
  }
      
  return not_quit;
}



/* let a player type in their password again to confirm that it is as they
   typed it */

confirm_player_password(player, win, x, y)
PlayerInfo *player;
Window win;
int x, y;
{
  int w, h;
  char s[PASSWORD_LENGTH], *answer;

  /* get a password entry and compare with old password */
  strcpy(s, help_window_input_line(win, x, y, bigfont, "password: ",
				   PASSWORD_LENGTH - 1, FALSE));
  write_help_window_contents(namerec);
  if (strcmp(s, player->password))
    answer = "You may have erred, your re-entry does not match.  Press a key...";
  else
    answer = "Your password has been entered correctly.  Press a key...";

  /* tell them about it */
  w = char_width(bigfont) * (strlen(answer) + 2);
  h = char_height(bigfont) * 2;
  (void) placed_want_query(win, x, y, x+w, y+h, answer);
}



/* a game was selected from the list, show the info about the game and
   confirm that player wishes to join it, then do so.  Return FALSE if the
   game was not joined, TRUE if it was */

int make_attempt_to_join_game(player, rec, line)
PlayerInfo *player;
Help *rec;
int line;
{
  int i, j, got_in = -1;
  char s[100], *extract_from_help_line();

  /* get the name of the game we are joining */
  strcpy(player->our_game,
	 extract_from_help_line(rec, line, 3, GAME_NAME_LENGTH + 1));

  /* set up and go into input loop for team selection or abort request */
  if (do_team_pick_windows(player)) {
      /* at last moment now, put player's name, password and team in record */
      load_player_into_person(player, 0);

      /* start up a window to show game joining progress */
      joiner_message("Waiting for connection with server...");

      /* team was selected, now make the attempt to join */
      ignore_termination();
      got_in = try_to_access_game(player);

      /* if we couldn't get into the game, then hold so player sees message */
      if (got_in <= 0) {
	i = GAME_NAME_X;
	j = PLAYER_MAIN_H / 2;
	strcpy(s, "Press any key to return to list of games...");
	(void) placed_want_query(mainwin, i, j,
				i + char_width(bigfont) * (strlen(s) + 2),
				j + char_height(bigfont) * 2, s);
      }

      /* set the termination traps for safe termination if were are in a game */
      if (got_in > 0) safety_termination(NULL);
      else comply_with_termination();
  }

  return (got_in > 0);
}



/* set up the windows that will display who's on which teams and how the
   player can join one of them.  Return TRUE if the player picked a
   team to join.  Return FALSE if the player opted to quit looking at
   this game.  (The player may opt to look at a different game in the list,
   and thus gradually look through all the games.) */

do_team_pick_windows(player)
PlayerInfo *player;
{
  int got_in = FALSE, quit = FALSE;
  GameInfo *game;

  do {
    /* get the information about the game */
    game = get_game_by_name(player, player->our_game);

    if (game) {
      /* set up windows for the currently selected game */
      setup_team_windows(game);
      setup_teamtop(game);

      /* get input on the windows */
      quit = !get_team_select_input(player, game);
      got_in = !quit;

      /* get rid of the team windows */
      destroy_team_windows();
      destroy_help_window(teamtop);
    }
    else {
      /* let player know that game is not in the list */
      char *s = "Game selected is no longer active, press any key...";
      int x, y, w, h;
      w = char_width(bigfont) * (strlen(s) + 2);
      h = char_height(bigfont) * 2;
      x = (PLAYER_MAIN_W - w) / 2;
      y = (PLAYER_MAIN_H - h) / 2;
      (void) placed_want_query(mainwin, x, y, x+w, y+h, s);
      quit = TRUE;
      got_in = FALSE;
    }

  } while (!got_in && !quit);

  return (got_in);
}



/* set up the windows (ie. create them) that will contain the pictures
   of players on each team in the given game. */

setup_team_windows(game)
GameInfo *game;
{
  int i, taken, separation, outsides, septotal, start;

  if (game->teams == 0) Gerror("game selected has no teams");
  if (game->teams > NUM_OF_TEAMS) Gerror("number of teams is too high");
  for (i=0; (i <= NUM_OF_TEAMS); i++) win_exists[i] = FALSE;

  /* figure out which windows exist for team display */
  if (game->teams == 1) {
    num_windows = 1;
    win_exists[1] = TRUE;
  }
  else {
    num_windows = game->teams + 1;
    for (i=0; (i <= game->teams); i++) win_exists[i] = TRUE;
    if (!game->neutrals_allowed) {
      win_exists[0] = FALSE;
      num_windows--;
    }
  }

  /* determine where the windows must be placed, in accordance to how many
     of them there is to be */

  taken = num_windows * TEAM_WIN_W;   /* width needed for these windows */
  septotal = (num_windows - 1) * TEAM_WIN_PAD;	/* padding space between */
  if ((taken + septotal) > PLAYER_MAIN_W) {
    /* there isn't proper room for the windows, so spread as best you can */
    outsides = (PLAYER_MAIN_W - taken) / 2;
    separation = 0;
  }
  else {
    /* there is ample room for the windows, so place them with equal spacing */
    separation = TEAM_WIN_PAD;
    outsides = (PLAYER_MAIN_W - (taken + septotal)) / 2;
  }

  /* place the x and y coords of upper left corners in the location array */
  start = outsides;
  for (i=0; i <= NUM_OF_TEAMS; i++)
    if (win_exists[i]) {
      tx[i] = start;
      ty[i] = TEAM_WIN_Y;
      twidth[i] = TEAM_WIN_W;
      theight[i] = TEAM_WIN_HEIGHT(count_team_members(game, i));
      start = start + TEAM_WIN_W + separation;
    }

  /* create the windows and map them */
  for (i=0; i <= NUM_OF_TEAMS; i++)
    if (win_exists[i]) {
      teamwin[i] = XCreateSimpleWindow(display, mainwin, tx[i], ty[i],
				twidth[i], theight[i], WINDOW_BORDERS,
				fgcolor, bgcolor);
      init_input_lines(teamwin[i]);
      give_window_border_bitmap(teamwin[i], border_bits, border_width,
				border_height);
      XMapWindow(display, teamwin[i]);
    }
}



destroy_team_windows()
{
  int i;

  for (i=0; i <= NUM_OF_TEAMS; i++)
    if (win_exists[i])
      XDestroyWindow(display, teamwin[i]);
}



unmap_game_select_windows()
{
  hide_help_window(namerec);
  hide_help_window(listrec);
  hide_help_window(helprec);
}


remap_game_select_windows()
{
  show_help_window(helprec);
  show_help_window(namerec);
  show_help_window(listrec);
}



/* set up the the help window at the top of team selection screen,
   make sure that THELP_LINES defines as many lines as you wish to
   use for displaying information in this window. */

setup_teamtop(game)
GameInfo *game;
{
  char s[200], *t;

  /* define the general size of the map in this game */
  if (game->rooms < SMALL_MAP)       t = "tiny";
  else if (game->rooms < MEDIUM_MAP) t = "small";
  else if (game->rooms < LARGE_MAP)  t = "medium-sized";
  else if (game->rooms < HUGE_MAP)   t = "large";
  else t = "huge";

  /* create the team selection help window */
  teamtop = make_help_window(mainwin, THELP_WIN_X, THELP_WIN_Y,
			     THELP_COLS, THELP_LINES, THELP_FONT);
  give_window_border_bitmap(teamtop->win, border_bits,
			    border_width, border_height);
  init_input_lines(teamtop->win);

  /* place the important info into the help window and show it */
  set_line(teamtop, 0, game->map);

  if (game->size == 0)
    sprintf(s, "This is a %s map, and there is currently no one in the game.", t);
  else if (game->size == 1)
    sprintf(s, "This is a %s map, currently containing just one player.", t);
  else
    sprintf(s, "This is a %s map, currently containing %d players.", t, game->size);
  set_line(teamtop, 2, s);

  sprintf(s, "It has been running on %s since %s.", game->host, game->time);
  set_line(teamtop, 3, s);

  if (game->teams == 1)
    sprintf(s, "There are no teams in this map, it's every man for himself.");
  else if (!game->neutrals_allowed)
    sprintf(s, "%d teams play against each other, and no neutrals are permitted.", game->teams);
  else
    sprintf(s, "%d teams play against each other.  Neutrality is also an option.", game->teams);
  set_line(teamtop, 4, s);

  if (num_windows == 1)
    set_line(teamtop, 6, "Press the mouse button in the box below to join the game");
  else
    set_line(teamtop, 6, "Press the mouse button on the team below you wish to join");

  set_line(teamtop, 7, "or press any key to avoid joining this game and look at another");
  show_help_window(teamtop);
}



/* get and process input received in the team selection windows, return
   TRUE when the player has selected the game or FALSE when he quits. */

int get_team_select_input(player, game)
PlayerInfo *player;
GameInfo *game;
{
  int   i, x, y, detail, quit = FALSE, joined = FALSE;
  char  c;
  Window happenwin;
  
  while (!quit && !joined) {
    i = get_input(mainwin, &c, &x, &y, &detail, &happenwin);

    /* handle input according to its type */
    switch (i) {
      case KEYBOARD:	quit = TRUE;
			break;
      case EXPOSED:	handle_team_exposure(happenwin, game);
			break;
      case MOUSE:	joined = handle_team_select_mouse(player, happenwin);
			break;
      case ENTER:	if (PLAYERWIN_AUTORAISE)
			  XRaiseWindow(display, mainwin);
      default:		break;
    }

  }

  return(joined);
}



/* handle an expose event in one of the windows */

handle_team_exposure(win, game)
Window win;
GameInfo *game;
{
  int i, found = FALSE;

  for (i=0; ((i <= NUM_OF_TEAMS) && !found); i++)
    if (win_exists[i])
      if (teamwin[i] == win) {
        redraw_a_team_window(game, i);
        found = TRUE;
      }

  if (!found) check_help_expose(win);
}



/* redraw the contents of one of the team windows, according to the
   information given in the game structure */

redraw_a_team_window(game, n)
GameInfo *game;
int n;
{
  int i, y = PLAYER_TOP_Y(0), center;
  char *name, s[200];

  if (!win_exists[n]) Gerror("team window doesn't exist");

  /* write in the name of the team */
D 6
  if (n == 0) name = "Neutral";
  else name = game->team_name[n-1];
E 6
I 6
  name = game->team_name[n];
E 6
  text_write(teamwin[n], TEAM_WIN_TITLE_FONT, 0, 0, 1, 0, name);
  for (i=0; i<TEAM_WIN_COLS; i++)
    text_write(teamwin[n], TEAM_WIN_UNDER_FONT, 0,
	       char_height(TEAM_WIN_TITLE_FONT), i, 0, "-");

  /* place the information on each person on the team */
  center = (PLAYER_HEIGHT - char_height(TEAM_WIN_FONT)*3) / 2;
  for (i=0; (i < game->size); i++)
    if (game->player[i].team == n) {
      draw_player(teamwin[n], PLAYER_BIT_X, y, game->player[i].bitmap,
		  game->player[i].mask);
      text_write(teamwin[n], TEAM_WIN_FONT, PLAYER_TEXT_X, y+center, 0, 0,
		 game->player[i].name);
      text_write(teamwin[n], TEAM_WIN_FONT, PLAYER_TEXT_X, y+center, 0, 1,
		 game->player[i].rank);
      sprintf(s, "%s@%s", game->player[i].login, game->player[i].host);
      text_write(teamwin[n], TEAM_WIN_FONT, PLAYER_TEXT_X, y+center, 0, 2, s);
      y = y + PLAYER_HEIGHT;
    }
}



/* this routines determines which team window the mouse was pressed in.
   It returns TRUE if a team was selected and places the team number
   selected into the player structure.  It returns FALSE if the mouse
   was pressed somewhere other than in a team window. */

int handle_team_select_mouse(player, win)
PlayerInfo *player;
Window win;
{
  int i, found = FALSE;

  for (i=0; ((i <= NUM_OF_TEAMS) && !found); i++)
    if (win_exists[i])
      if (teamwin[i] == win) {
        player->team = i;
        found = TRUE;
      }

  return found;
}




/* draw the image of a player, using his mask and bitmap arrays.  This
   creates temporary pixmaps for the purpose and immediately frees them,
   not great for speed but good on memory storage requirements */

draw_player(win, x, y, draw_bits, mask_bits)
Window win;
int x, y;
char *draw_bits, *mask_bits;
{
  create_and_draw_pixmap(display, win, x, y, mask_bits, BITMAP_WIDTH,
			 BITMAP_HEIGHT, maskGC, fgcolor, bgcolor);
  create_and_draw_pixmap(display, win, x, y, draw_bits, BITMAP_WIDTH,
			 BITMAP_HEIGHT, drawGC, fgcolor, bgcolor);
}



/* create and draw a pixmap, given bitmap data array */

create_and_draw_pixmap(display, win, x, y, bits, width, height, gc, fg, bg)
Display *display;
Window win;
int x, y;
char *bits;
int width, height;
GC gc;
int fg, bg;
{
  Pixmap pix;

  pix = XCreatePixmapFromBitmapData(display,
		RootWindow(display, screen), bits,
D 2
		width, height, fg, bg, 1);
E 2
I 2
		width, height, fg, bg, depth);
E 2
  XCopyArea(display, pix, win, mainGC, 0, 0,
	        width, height, x, y);
  XFreePixmap(display, pix);
}



int count_team_members(game, team)
GameInfo *game;
int team;
{
  int i, count = 0;

  for (i=0; (i < game->size); i++)
    if (game->player[i].team == team) count++;

  return count;
}



/* add a line to the join report help window.  If it hasn't been created,
   then make it. */

joiner_message(s)
char *s;
{
  static int linenum = 0;

  /* if the help record isn't created then make it */
  if (!joinrec) {
      int x, y, w, h;
      w = char_width(bigfont) * JOIN_COLS;
      h = char_height(bigfont) * JOIN_LINES;
      x = (PLAYER_MAIN_W - w) / 2;
      y = (PLAYER_MAIN_H - h) / 2;
      linenum = 0;
      joinrec = make_help_window(mainwin, x, y, JOIN_COLS, JOIN_LINES, bigfont);
      init_input_lines(joinrec->win);
      give_window_border_bitmap(joinrec->win, border_bits,
				border_width, border_height);
      show_help_window(joinrec);
  }

  /* add a line at the current line number */
  if (linenum >= JOIN_LINES) linenum = 0;
  set_line(joinrec, linenum, s);
  write_help_window_contents(joinrec);
  XFlush(display);
  linenum++;
}



/* close up the join report window */

close_joiner()
{
  if (joinrec) {
    sleep(1);
    destroy_help_window(joinrec);
    joinrec = NULL;
  }
}



/* start up a driver program from here inside the player process.  This
   routine and the ones associated with it may some day have 
   portablility problems, so watch out. */

start_a_driver(player)
PlayerInfo *player;
{
  char host[120], map_filename[PATH_LENGTH], s[120], *choose_map_name();


  /* change help window contents concerning this event */
  XClearWindow(display, helprec->win);
  clear_help_window(helprec);
  load_help_window_contents(helprec, HOST_HELP_FILE);
  write_help_window_contents(helprec);

  /* get the name of a local host to run the driver on */
  strcpy(host,
	 help_window_input_line(mainwin, GAME_NAME_X, PLAYER_MAIN_H -
				char_height(bigfont) * 6, bigfont, 
				"machine: ", HOST_LENGTH - 1, TRUE));

  /* get a map to use */
  strcpy(map_filename, choose_map_name());

  /* start the driver with given host and map file (if a map file was given) */
  if (strlen(map_filename)) {
    system_driver_startup(host, map_filename);
  }
  else {
  strcpy(s, "You gave no map name, so no game will be started. Press a key...");
  (void) placed_want_query(mainwin, GAME_LIST_X, PLAYER_MAIN_H -
			   char_height(bigfont) * 5, GAME_LIST_X +
			   char_width(bigfont) * (strlen(s) + 2),
			   PLAYER_MAIN_H - char_height(bigfont) * 3, s);
  }

  /* change help window contents back again */
  XClearWindow(display, helprec->win);
  clear_help_window(helprec);
  load_help_window_contents(helprec, GAME_HELP_FILE);
  write_help_window_contents(helprec);
}



/* choose a map from those in the map list file */

char *choose_map_name()
{
  int w;
  char *result;
  Help *rec = NULL;

  /* put the map selection help file into a window */
  rec = make_help_from_loading_file(mainwin, GAME_LIST_X, GAME_LIST_Y,
				    PLAYER_MAIN_W - GAME_LIST_X - 15,
				    PLAYER_MAIN_H - GAME_LIST_Y -
D 4
				    char_height(bigfont) * 6 - 15,
E 4
I 4
				    char_height(bigfont) * 3 - 15,
E 4
				    regfont, MAP_LIST_FILE, MAP_LIB_DIR);
  if (rec) {
    give_window_border_bitmap(rec->win, border_bits, border_width, border_height);
    show_help_window(rec);
    init_input_lines(rec->win);
    write_help_window_contents(rec);
  }

  w = (PLAYER_MAIN_W - GAME_NAME_X - char_width(bigfont) *
       strlen("map filename: ") - 20) / char_width(bigfont);

  result = help_window_input_line(mainwin, GAME_NAME_X, PLAYER_MAIN_H -
				  char_height(bigfont) * 6, bigfont, 
				  "map filename: ", min(w, PATH_LENGTH-1),
				  TRUE);

  if (rec) destroy_help_window(rec);
  return result;
}




/* start up a driver in the background, given a host to start it at and
   a map file to look for */

system_driver_startup(host, filename)
char *host, *filename;
{
  Help *rec;
  char s[500];

  /* create a help window for messages */
  sprintf(s, "Standby, attempting to start driver on %s ...", host);
  rec = make_help_window(mainwin, GAME_LIST_X, PLAYER_MAIN_H -
			 char_height(bigfont) * 6, strlen(s) + 1, 3, bigfont);
  init_input_lines(rec->win);
  give_window_border_bitmap(rec->win, border_bits, border_width, border_height);
  set_line(rec, 1, s);
  show_help_window(rec);
  write_help_window_contents(rec);
  XFlush(display);

  /* do the driver startup here - WARNING: this part may need to be changed
     in order to work on some systems.  We do not claim that the next few
     lines are portable. */
  if (strlen(host))
    sprintf(s, "rsh %s -n %s -m %s &", host, DRIVER_RUN_FILE, filename);
  else
D 4
    sprintf(s, "%s -m %s &", DRIVER_RUN_FILE, filename);
E 4
I 4
    sprintf(s, "nohup %s -m %s &", DRIVER_RUN_FILE, filename);
E 4
  system(s);

  /* warn player of inconclusive results */
  destroy_help_window(rec);
  strcpy(s, "New game will appear within 2 minutes, if it worked. Press a key...");
  (void) placed_want_query(mainwin, GAME_LIST_X, PLAYER_MAIN_H -
			   char_height(bigfont) * 5, GAME_LIST_X +
			   char_width(bigfont) * (strlen(s) + 2),
			   PLAYER_MAIN_H - char_height(bigfont) * 3, s);
  write_help_window_contents(listrec);
}
E 1
