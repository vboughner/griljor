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

/* Error Handling Program File */

#include <stdio.h>
#include <signal.h>
#include "config.h"
#include "def.h"
#include "extern.h"


/* signal error handling routine declarations */
void handle_sigpipe();
void handle_sigsegv();
void handle_sigterm();

/* internal variable(s) for this file's use only */
PlayerInfo *save_info = NULL;


Gerror(s)
char *s;
{
  fprintf(stderr, "%s: %s\n", PROGRAM_NAME, s);
  exit(1);
}



/* trap signals and learn to avoid errors */

avoid_errors()
{
  if (DEBUG) printf("Setup error trapping\n");

  /* set myself to catch SIGPIPE signals */
  signal(SIGPIPE, handle_sigpipe);

  /* set myself to catch SIGSEGV signals */
  signal(SIGSEGV, handle_sigsegv);
}



/* arrange for complying with ctrl-C requests and other terminations */

comply_with_termination()
{
  signal(SIGTERM, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
}



/* arrange to ignore future termination requests */

ignore_termination()
{
  signal(SIGTERM, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
}



/* arrange to quit safely upon a termination request.  Send the driver's
   info structure (as a pointer) or NULL for players.  The info structure
   is needed for the driver's successful safe termination. */

safety_termination(info)
PlayerInfo *info;
{
  /* store info pointer away safely */
  save_info = info;

  /* send future termination signals to handler routine */
  signal(SIGTERM, handle_sigterm);
  signal(SIGHUP, handle_sigterm);
  signal(SIGINT, handle_sigterm);
  signal(SIGQUIT, handle_sigterm);
}



/* notify player of a SIGPIPE error if in debug mode */

void handle_sigpipe(sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
  if (DEBUG) printf("Warning: SIGPIPE signal received - code %d\n", code);
}



/* notify player of a SIGSEGV error if in debug mode */

void handle_sigsegv(sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
  if (DEBUG) {
    printf("Warning: SIGSEGV signal received - code %d\n", code);
    sleep(1);	/* pause so user can press ctrl-C if he wants to */
  }
}



/* handle a termination signal so that player or driver can quit safely */

void handle_sigterm(sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
  /* we don't want any more interruptions now that we are quitting */
  ignore_termination();

  /* if we are driver then try to remove game record */
  if (am_driver && save_info) remove_game(save_info);

  /* tell everyone I'm leaving */
  if (!am_driver) allLeaveGame();

  /* close up our game connections */
  endBoss();

  /* exit now that it is safe to do so */
  printf("\nEmergency Termination.\n");
  exit(1);
}
