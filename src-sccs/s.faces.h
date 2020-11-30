h55031
s 00000/00000/00187
d D 1.3 92/08/07 01:01:18 vbo 3 2
c source copied to a separate tree for work on new map and object format
e
s 00003/00003/00184
d D 1.2 91/05/17 06:44:39 labc-3id 2 1
c 
e
s 00187/00000/00000
d D 1.1 91/05/17 04:16:23 labc-3id 1 0
c date and time created 91/05/17 04:16:23 by labc-3id
e
u
U
f e 0
t
T
I 1
/***************************************************************************
 * %Z% %M% %G% - last change made %I%
 *
 * Copyright 1991 Van A. Boughner, Mel Nicholson, and Albert C. Baker III
 * All Rights Reserved.
 *
 * Griljor by Van A. Boughner
 *            Mel Nicholson
 *            Albert C. Baker III
 *	      Trevor Pering
 *	      Eric van Bezooijen
 *
 * Copyright information is in the README file.  Note that this source code
 * may not be altered by anyone but the authors, except under the conditions
 * outlined in the copyright notice.
 *
 * Released under the supervision of the Volvox Software Group.
 * Many thanks to all the CS undergrads at U.C. Berkeley that helped us out.
 *
 **************************************************************************/

/* ===== First Grouping of Faces for Credits ===== */

#define GROUP1_SIZE 7

#include "facebits/vanbit"
#include "facebits/vanmask"
char *van_name = "Van A. Boughner";
char *van_con1 = "Wrote the large initial chunk of code.";
char *van_con2 = "Organized the collective work at the end.";

#include "facebits/melbit"
#include "facebits/melmask"
static char *mel_name = "Mel Nicholson";
D 2
static char *mel_con1 = "Originated many of the ideas, wrote";
static char *mel_con2 = "some code, participated from beginning.";
E 2
I 2
static char *mel_con1 = "Originated many of the basic concepts.";
static char *mel_con2 = "Coded the sighting, monsters, missiles.";
E 2

#include "facebits/albertbit"
#include "facebits/albertmask"
static char *albert_name = "Albert C. Baker III";
static char *albert_con1 = "Wrote textual password file editor,";
static char *albert_con2 = "object placement code, time restrictor.";

#include "facebits/trevorbit"
#include "facebits/trevormask"
static char *trevor_name = "Trevor Pering";
static char *trevor_con1 = "Wrote object editor, edited graphics";
static char *trevor_con2 = "routines, increased efficiency of code.";

#include "facebits/ericbit"
#include "facebits/ericmask"
static char *eric_name = "Eric van Bezooijen";
static char *eric_con1 = "Wrote graphics version of password file";
static char *eric_con2 = "editor, many maps, and did lots of testing.";

#include "facebits/spookbit"
#include "facebits/spookmask"
D 2
static char *spook_name = "Doug R. Stein";
E 2
I 2
static char *spook_name = "Douglas R. Stein";
E 2
static char *spook_con1 = "Wrote code for this title screen,";
static char *spook_con2 = "helped coallate the various credits.";

#include "facebits/stefanbit"
#include "facebits/stefanmask"
static char *stefan_name = "Stefan M. Sortland";
static char *stefan_con1 = "Created the good bitmaps in the standard";
static char *stefan_con2 = "object definitions and this title screen.";


/* ===== Second Grouping of Faces for Credits ===== */

#define GROUP2_SIZE 5

#include "facebits/bhbit"
#include "facebits/bhmask"
static char *bh_name = "Brian Harvey";
static char *bh_con1 = "The coolest CS instructor on campus.";
static char *bh_con2 = "Inspired many of us to continue trying.";

#include "facebits/gmbit"
#include "facebits/gmmask"
static char *gm_name = "Burt (game master)";
static char *gm_con1 = "Sometimes seen in the game, can always";
static char *gm_con2 = "be reached by sending message to 'gm'";

#include "facebits/moronusbit"
#include "facebits/moronusmask"
static char *moronus_name = "Boyd Montgomery";
static char *moronus_con1 = "A very demanding playtester...";
static char *moronus_con2 = "'Look what happens when I press ctrl-D'";

#include "facebits/olliebit"
#include "facebits/olliemask"
static char *ollie_name = "Ollie Juang";
static char *ollie_con1 = "An excellent player who's been with us";
static char *ollie_con2 = "for a long time.  Put litter in its place.";

#include "facebits/savakibit"
#include "facebits/savakimask"
static char *savaki_name = "Matt Ho";
static char *savaki_con1 = "A dabbler in many things.  Helped us";
static char *savaki_con2 = "out by playing and providing suggestions.";


/* ===== Third Grouping of Faces for Credits ===== */
/* Random sampling of these will be displayed if there isn't room for all */

#define GROUP3_SIZE 13

#include "facebits/stinglaibit"
#include "facebits/stinglaimask"
static char *stinglai_name = "Phillip 'Edward' Nunez";
static char *stinglai_con1 = "Thanks to this player who";
static char *stinglai_con2 = "occasionally stops by for a battle.";

#include "facebits/aaronbit"
#include "facebits/aaronmask"
static char *aaron_name = "Aaron";
static char *aaron_con1 = "Name unknown.  Spent many hours testing/";
static char *aaron_con2 = "playing some of the experimental versions.";

#include "facebits/aragornbit"
#include "facebits/aragornmask"
static char *aragorn_name = "Jack Hsu";
static char *aragorn_con1 = "Has been seen helping";
static char *aragorn_con2 = "test the game by playing it.";

#include "facebits/crescendobit"
#include "facebits/crescendomask"
static char *crescendo_name = "Crescendo";
static char *crescendo_con1 = "We really haven't a clue who this is.";
static char *crescendo_con2 = "They've played the game, that's for sure.";

#include "facebits/mahatmabit"
#include "facebits/mahatmamask"
static char *mahatma_name = "Michael Melo";
static char *mahatma_con1 = "Many thanks to this player with";
static char *mahatma_con2 = "an unusual but interesting bitmap.  ";

#include "facebits/crombit"
#include "facebits/crommask"
static char *crom_name = "Crom";
static char *crom_con1 = "Name unknown.  Another player entity.";
static char *crom_con2 = "We've had some good games with this fellow.";

#include "facebits/drustanbit"
#include "facebits/drustanmask"
static char *drustan_name = "Drustan";
static char *drustan_con1 = "Name unknown.  A good player who has";
static char *drustan_con2 = "devoted a good portion of time to this.";

#include "facebits/duelbit"
#include "facebits/duelmask"
static char *duel_name = "Duel";
static char *duel_con1 = "Name unknown.  Someone from the past.";
static char *duel_con2 = "Played often in the beginning stages.";

#include "facebits/mcelhoebit"
#include "facebits/mcelhoemask"
static char *mcelhoe_name = "Glenn McElhoe";
static char *mcelhoe_con1 = "Someone who evidently played for";
static char *mcelhoe_con2 = "a while, but we haven't seen lately.";

#include "facebits/mikebit"
#include "facebits/mikemask"
static char *mike_name = "Mike Wahle";
static char *mike_con1 = "Another helpful playtester.";
static char *mike_con2 = "We can't ever have too many of these.";

#include "facebits/mikeybit"
#include "facebits/mikeymask"
static char *mikey_name = "Mikey";
static char *mikey_con1 = "A very helpful playtester in the early";
static char *mikey_con2 = "stages.  The Simpsons were popular then...";

#include "facebits/adrianabit"
#include "facebits/adrianamask"
static char *adriana_name = "Adriana Nicholson";
static char *adriana_con1 = "Helped test this game through many";
static char *adriana_con2 = "hours of play.  Reported various bugs.";

#include "facebits/avatarbit"
#include "facebits/avatarmask"
static char *avatar_name = "(Tom) Chen-Ming Lee";
static char *avatar_con1 = "Has been caught playing griljor a";
static char *avatar_con2 = "good deal, helping to find bugs.";

/* If you add more here, don't forget to change GROUP3_SIZE definition */
E 1
