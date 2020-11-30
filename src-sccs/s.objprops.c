h40669
s 00000/00000/00614
d D 1.14 92/08/07 01:02:49 vbo 14 13
c source copied to a separate tree for work on new map and object format
e
s 00084/00080/00530
d D 1.13 91/12/13 23:21:43 labc-4lc 13 12
c added and deleted a lot of object properties
e
s 00022/00003/00588
d D 1.12 91/12/13 19:33:45 labc-4lc 12 11
c fixed bug that made a completely blank object unreadable and made it possible to move object properties into an old/outdated state
e
s 00050/00000/00541
d D 1.11 91/12/03 17:29:42 labc-4lc 11 10
c implemented header items for object def files
e
s 00012/00012/00529
d D 1.10 91/11/27 15:07:19 labc-4lc 10 9
c changed message properties to hidden strings
e
s 00005/00002/00536
d D 1.9 91/11/27 13:41:31 labc-4lc 9 8
c fixed string in objects so space is malloced, not static
e
s 00001/00000/00537
d D 1.8 91/10/16 20:05:52 labc-4lc 8 7
c changed made for DecStations
e
s 00021/00005/00516
d D 1.7 91/09/16 22:20:27 labc-4lc 7 6
c made room and recobj dialog edit windows work for editmap
e
s 00012/00000/00509
d D 1.6 91/09/15 23:33:37 labc-4lc 6 5
c made mapinfo dialog editing window work
e
s 00158/00010/00351
d D 1.5 91/09/15 16:15:42 labc-4lc 5 4
c added property list defs for maps, rooms, and recobjects
e
s 00001/00001/00360
d D 1.4 91/09/04 21:16:50 labb-3li 4 3
c 
e
s 00002/00002/00359
d D 1.3 91/08/27 01:56:42 vanb 3 2
c fixed a bug in add_property()
e
s 00361/00000/00000
d D 1.2 91/08/26 00:33:49 vanb 2 1
c fixed up procedure defs and other compatibilty problems
e
s 00000/00000/00000
d D 1.1 91/08/25 18:40:27 vanb 1 0
c date and time created 91/08/25 18:40:27 by vanb
e
u
U
t
T
I 2
/***************************************************************************
 * %Z% %M% %I% - last change made %G%
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

D 5
/* File which contains the definition of object property flags */
E 5
I 5
/* File which contains the definition of object property flags,
   and also the definition of properties for maps and rooms in maps. */
E 5

#include <stdio.h>
#include "def.h"
#include "objects.h"
I 5
#include "map.h"
E 5
#define NOT_OBTOR
#include "dialog.h"
I 11
#include "objinfo.h"
E 11

I 11

E 11
#define SPACE_SEGMENTS 20



static DialogList *static_record(name, type, offset, size, call, data, id)
/* creates a record from the arguments in a statically allocated area
   of memory.  Caller must copy the result if he wants to keep it after
   a second call to this routine. */
char *name;
int offset, size;
int (*call)();
long data;
int id;
{
  static DialogList rec;
  rec.name = name;
  rec.type = type;
  rec.offset = offset;
  rec.size = size;
  rec.call = call;
  rec.data = data;
  rec.id = id;
  return &rec;
}



static DialogList *add_property(list, space, size, record)
DialogList *list;
int *space;   /* amount of space allocated in the array so far */
int *size;    /* number of array elements actually filled */
DialogList *record;    /* copy of the record we should place in array */
{
  if (*size == *space) {
    *space = *space + SPACE_SEGMENTS;
    if (list)
D 3
      list = (DialogList *) realloc(*list, sizeof(DialogList) * *space);
E 3
I 3
      list = (DialogList *) realloc(list, sizeof(DialogList) * *space);
E 3
    else
      list = (DialogList *) malloc(sizeof(DialogList) * *space);
  }
I 8
  demand(list, "ran out of memory making property list");
E 8
D 3
  bcopy(record, list[*size], sizeof(DialogList));
E 3
I 3
  bcopy(record, (DialogList *)(list + *size), sizeof(DialogList));
E 3
  *size = *size + 1;
  return list;
}


I 5
/* ==================== M A C R O S ===================================== */
E 5

D 5
DialogList *AllocObjectProperties()
/* returns a pointer to a newly allocated array of the properties an object
   can have.  Caller must free the array when done with it. */
{
  int space = 0, size = 0;
  DialogList *new = NULL;
  ObjInfo tobj;
  int SaveObject(),QuitObject(),AutoMask();

E 5
#define NULLID  0
#define BITMAPFIELD  1
#define MASKFIELD    2
#define MASKEDFIELD  3
#define OFF(var)  (((char *) &(tobj.var)) - ((char *) &(tobj)))
#define ARR(var)  (((char *)  (tobj.var)) - ((char *) &(tobj)))/* for arrays */
#define SIZ(var)  (sizeof(tobj.var))
#define ADD(name, type, offset, len, call, data, id) \
  new = add_property(new, &space, &size, \
		     static_record(name, type, offset, len, call, data, id))
I 12
#define OLD(name, type, offset, len, call, data, id) \
  ADD((name), ((type) | OLDDIAG), 0, 0, 0, 0, id)
E 12

I 5

D 12

E 12
/* ===================== O B J E C T S ================================== */


DialogList *AllocObjectProperties()
/* returns a pointer to a newly allocated array of the properties an object
   can have.  Caller must free the array when done with it. */
{
  int space = 0, size = 0;
  DialogList *new = NULL;
  ObjInfo tobj;
  int SaveObject(),QuitObject(),AutoMask();

E 5
  ADD("Object set", MARKSHO, 0, 0, NULL, NULL, NULLID);
  ADD("Object num", MARKSHO, 1, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Bitmap", ICONPIC, ARR(bitmap), SIZ(bitmap), NULL, BITMAPFIELD, 100);
  ADD("  Mask", ICONPIC, ARR(mask), SIZ(mask), NULL, MASKFIELD, 110);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("AutoMask", QUITBOX , 0, 0, AutoMask, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Save changes", QUITBOX , 0, 0, SaveObject, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Quit", QUITBOX , 0, 0, QuitObject, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
   
  ADD("Class:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 4
  ADD("Has mask", BOLBOX, OFF(masked), SIZ(masked), NULL, NULL, 130);
E 4
I 4
  ADD("Has mask", BOLBOX, OFF(masked), SIZ(masked), NULL, MASKEDFIELD, 130);
E 4
  ADD("Has record", BOLBOX, OFF(recorded), SIZ(recorded), NULL, NULL, 140);
  ADD("Type", INTBOX, OFF(type), SIZ(type), NULL, NULL, 120);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

I 10
D 12
  /* Do not use property id's 150, 160, 180, 190, 200, 210, 220, 230, 240, 250, 260 */
  /* These property id's were once used with now non existent property types */
E 12
E 10
  ADD("Descriptions:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 10
  ADD("From afar", INTBOX, OFF(lookmsg), SIZ(lookmsg), NULL, NULL, 150);
  ADD("Examination", INTBOX, OFF(examinemsg), SIZ(examinemsg), NULL, NULL, 160);
E 10
I 10
  ADD("From afar", HIDSTR, OFF(lookmsg), SIZ(lookmsg), NULL, NULL, 151);
  ADD("Examination", HIDSTR, OFF(examinemsg), SIZ(examinemsg), NULL, NULL, 161);
E 10
D 13
  ADD("Hide properties", BOLBOX, OFF(hideprops), SIZ(hideprops), NULL, NULL, 170);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Use messages:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 10
  ADD("Personal", INTBOX, OFF(usemsg), SIZ(usemsg), NULL, NULL, 180);
  ADD("Within room", INTBOX, OFF(roommsg), SIZ(roommsg), NULL, NULL, 190);
  ADD("Everyone", INTBOX, OFF(allmsg), SIZ(allmsg), NULL, NULL, 200);
  ADD("Alerter", BOLBOX, OFF(alerter), SIZ(alerter), NULL, NULL, 210);
E 10
I 10
  ADD("Personal", HIDSTR, OFF(usemsg), SIZ(usemsg), NULL, NULL, 181);
  ADD("Within room", HIDSTR, OFF(roommsg), SIZ(roommsg), NULL, NULL, 191);
  ADD("Everyone", HIDSTR, OFF(allmsg), SIZ(allmsg), NULL, NULL, 201);
E 10
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

D 13
  ADD("Alteration msgs:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 10
  ADD("Personal", INTBOX, OFF(usealtermsg), SIZ(usealtermsg), NULL, NULL, 220);
  ADD("Within room", INTBOX, OFF(roomaltermsg), SIZ(roomaltermsg), NULL, NULL, 230);
  ADD("Everyone", INTBOX, OFF(allaltermsg), SIZ(allaltermsg), NULL, NULL, 240);
E 10
I 10
  ADD("Personal", HIDSTR, OFF(usealtermsg), SIZ(usealtermsg), NULL, NULL, 221);
  ADD("Within room", HIDSTR, OFF(roomaltermsg), SIZ(roomaltermsg), NULL, NULL, 231);
  ADD("Everyone", HIDSTR, OFF(allaltermsg), SIZ(allaltermsg), NULL, NULL, 241);
E 10
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

E 13
  ADD("Damage msgs:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 10
  ADD("For hitter", INTBOX, OFF(hitmsg), SIZ(hitmsg), NULL, NULL, 250);
  ADD("For victim", INTBOX, OFF(victimmsg), SIZ(victimmsg), NULL, NULL, 260);
E 10
I 10
  ADD("For hitter", HIDSTR, OFF(hitmsg), SIZ(hitmsg), NULL, NULL, 251);
  ADD("For victim", HIDSTR, OFF(victimmsg), SIZ(victimmsg), NULL, NULL, 261);
E 10
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

D 13
  ADD("Vehicle:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Num occupants", INTBOX, OFF(vehicle), SIZ(vehicle), NULL, NULL, 640);
  ADD("Drivespeed", INTBOX, OFF(drivespeed), SIZ(drivespeed), NULL, NULL, 660);
  ADD("May fire in", BOLBOX, OFF(firein), SIZ(firein), NULL, NULL, 650);
  ADD("May fire out", BOLBOX, OFF(fireout), SIZ(fireout), NULL, NULL, 700);
  ADD("Is entry square", BOLBOX, OFF(entry), SIZ(entry), NULL, NULL, 670);
  ADD("Is driver square", BOLBOX, OFF(driver), SIZ(driver), NULL, NULL, 680);
  ADD("Is window square", BOLBOX, OFF(window), SIZ(window), NULL, NULL, 690);
E 13
I 13
  ADD("Possession:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Takeable", BOLBOX, OFF(takeable), SIZ(takeable), NULL, NULL, 450);
  ADD("Weight", INTBOX, OFF(weight), SIZ(weight), NULL, NULL, 460);
  ADD("Lost when used", BOLBOX, OFF(lost), SIZ(lost), NULL, NULL, 490);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
D 9
  ADD("Name", LSTRBOX, ARR(name), SIZ(name), NULL, NULL, 270);
  ADD("Color", LSTRBOX, ARR(color), SIZ(color), NULL, NULL, 280);
E 9
I 9
  ADD("Name", LSTRBOX, OFF(name), SIZ(name), NULL, NULL, 270);
D 13
  ADD("Color", LSTRBOX, OFF(color), SIZ(color), NULL, NULL, 280);
E 13
I 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
E 13
E 9

  ADD("Limitations:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 13
  ADD("Not outside", BOLBOX, OFF(notoutside), SIZ(notoutside), NULL, NULL, 300);
  ADD("Not inside", BOLBOX, OFF(notinside), SIZ(notinside), NULL, NULL, 310);
  ADD("All teams in", BOLBOX, OFF(represented), SIZ(represented), NULL, NULL, 320);
E 13
  ADD("Classes", INTBOX, OFF(class), SIZ(class), NULL, NULL, 330);
I 13
  ADD("Room scales", INTBOX, OFF(scalebits), SIZ(scalebits), NULL, NULL, 318);
  ADD("Teams in", INTBOX, OFF(represented), SIZ(represented), NULL, NULL, 319);
E 13
  ADD("Team rooms", INTBOX, OFF(rooms), SIZ(rooms), NULL, NULL, 332);
  ADD("Team members", INTBOX, OFF(teams), SIZ(teams), NULL, NULL, 334);
  ADD("Min players", INTBOX, OFF(minplayers), SIZ(minplayers), NULL, NULL, 290);
  ADD("Max players", INTBOX, OFF(maxplayers), SIZ(maxplayers), NULL, NULL, 292);
  ADD("Min level", INTBOX, OFF(minlevel), SIZ(minlevel), NULL, NULL, 340);
  ADD("Max level", INTBOX, OFF(maxlevel), SIZ(maxlevel), NULL, NULL, 350);
  ADD("Power percent", INTBOX, OFF(power), SIZ(power), NULL, NULL, 370);
  ADD("Health percent", INTBOX, OFF(strength), SIZ(strength), NULL, NULL, 385);
D 13
  ADD("Mana pts", INTBOX, OFF(mana), SIZ(mana), NULL, NULL, 380);
  ADD("Health pts", INTBOX, OFF(health), SIZ(health), NULL, NULL, 390);
  ADD("Experience pts", INTBOX, OFF(experience), SIZ(experience), NULL, NULL, 360);
  ADD("Delay(ms)", INTBOX, OFF(delay), SIZ(delay), NULL, NULL, 430);
  ADD("Frozen in delay", BOLBOX, OFF(mustwait), SIZ(mustwait), NULL, NULL, 440);
E 13
I 13
  ADD("Takes power", INTBOX, OFF(mana), SIZ(mana), NULL, NULL, 380);
  ADD("Takes health", INTBOX, OFF(health), SIZ(health), NULL, NULL, 390);
  ADD("Takes experience", INTBOX, OFF(experience), SIZ(experience), NULL, NULL, 360);
E 13
  ADD("Must be deity", BOLBOX, OFF(deity), SIZ(deity), NULL, NULL, 420);
  ADD("Need other obj", BOLBOX, OFF(needanother), SIZ(needanother), NULL, NULL, 400);
  ADD("Other object", LINKPIC, OFF(otherobj), SIZ(otherobj), NULL, NULL, 410);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

D 13
  ADD("Possession:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Takeable", BOLBOX, OFF(takeable), SIZ(takeable), NULL, NULL, 450);
  ADD("Weight", INTBOX, OFF(weight), SIZ(weight), NULL, NULL, 460);
  ADD("Into hand", BOLBOX, OFF(intohand), SIZ(intohand), NULL, NULL, 470);
  ADD("Which hand", INTBOX, OFF(whichhand), SIZ(whichhand), NULL, NULL, 480);
  ADD("Lost when used", BOLBOX, OFF(lost), SIZ(lost), NULL, NULL, 490);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

E 13
  ADD("Properties:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Movement over", INTBOX, OFF(movement), SIZ(movement), NULL, NULL, 510);
  ADD("May fire over", BOLBOX, OFF(permeable), SIZ(permeable), NULL, NULL, 520);
  ADD("May see over", BOLBOX, OFF(transparent), SIZ(transparent), NULL, NULL, 530);
  ADD("Is exit", BOLBOX, OFF(exit), SIZ(exit), NULL, NULL, 500);
  ADD("Conceals people", BOLBOX, OFF(concealing), SIZ(concealing), NULL, NULL, 540);
  ADD("Glows in dark", BOLBOX, OFF(glows), SIZ(glows), NULL, NULL, 550);
  ADD("Is invisible", BOLBOX, OFF(invisible), SIZ(invisible), NULL, NULL, 570);
  ADD("Provides light", INTBOX, OFF(flashlight), SIZ(flashlight), NULL, NULL, 560);
  ADD("Connect group", INTBOX, OFF(connectgroup), SIZ(connectgroup), NULL, NULL, 580);
D 13
  ADD("Connects up", BOLBOX, OFF(connectsup), SIZ(connectsup), NULL, NULL, 590);
  ADD("Connects down", BOLBOX, OFF(connectsdown), SIZ(connectsdown), NULL, NULL, 600);
  ADD("Connects left", BOLBOX, OFF(connectsleft), SIZ(connectsleft), NULL, NULL, 610);
  ADD("Connects right", BOLBOX, OFF(connectsright), SIZ(connectsright), NULL, NULL, 620);
  ADD("Connects other", INTBOX, OFF(connectsother), SIZ(connectsother), NULL, NULL, 630);
E 13
I 13
  ADD("Connect bits", INTBOX, OFF(connectbits), SIZ(connectbits), NULL, NULL, 582);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Weaponry:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Is weapon", BOLBOX, OFF(weapon), SIZ(weapon), NULL, NULL, 710);
  ADD("Range of fire", INTBOX, OFF(range), SIZ(range), NULL, NULL, 720);
  ADD("Degs from center", INTBOX, OFF(arc), SIZ(arc), NULL, NULL, 730);
  ADD("Spread number", INTBOX, OFF(spread), SIZ(spread), NULL, NULL, 740);
  ADD("Random factor", INTBOX, OFF(fan), SIZ(fan), NULL, NULL, 750);
  ADD("Refire adjustment", INTBOX, OFF(refire), SIZ(refire), NULL, NULL, 760);
  ADD("Stops at targ pt", BOLBOX, OFF(stop), SIZ(stop), NULL, NULL, 770);
  ADD("Has charges", BOLBOX, OFF(numbered), SIZ(numbered), NULL, NULL, 780);
  ADD("Charge capacity", INTBOX, OFF(capacity), SIZ(capacity), NULL, NULL, 790);
  ADD("Missile fired", LINKPIC, OFF(movingobj), SIZ(movingobj), NULL, NULL, 800);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Ammo:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Charges type", INTBOX, OFF(charges), SIZ(charges), NULL, NULL, 810);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Missiles:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Is directional", BOLBOX, OFF(directional), SIZ(directional), NULL, NULL, 820);
  ADD("Speed (1-9)", INTBOX, OFF(speed), SIZ(speed), NULL, NULL, 830);
  ADD("Damage", INTBOX, OFF(damage), SIZ(damage), NULL, NULL, 840);
D 13
  ADD("Does vampire", BOLBOX, OFF(vampire), SIZ(vampire), NULL, NULL, 850);
E 13
  ADD("Goes thru people", BOLBOX, OFF(mowdown), SIZ(mowdown), NULL, NULL, 860);
  ADD("Ignore bits", INTBOX, OFF(ignoring), SIZ(ignoring), NULL, NULL, 870);
  ADD("Piercing bits", INTBOX, OFF(piercing), SIZ(piercing), NULL, NULL, 880);
  ADD("Destroy bits", INTBOX, OFF(destroys), SIZ(destroys), NULL, NULL, 890);
D 13
  ADD("Putdown bits", INTBOX, OFF(putsdown), SIZ(putsdown), NULL, NULL, 900);
E 13
  ADD("Explosion radius", INTBOX, OFF(explodes), SIZ(explodes), NULL, NULL, 910);
  ADD("Snuffed explode", BOLBOX, OFF(snuffs), SIZ(snuffs), NULL, NULL, 920);
  ADD("Explosion object", LINKPIC, OFF(boombit), SIZ(boombit), NULL, NULL, 930);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Destruction:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Is vulnerable", BOLBOX, OFF(vulnerable), SIZ(vulnerable), NULL, NULL, 940);
  ADD("Flammable radius", INTBOX, OFF(flammable), SIZ(flammable), NULL, NULL, 950);
  ADD("When destroyed", LINKPIC, OFF(destroyed), SIZ(destroyed), NULL, NULL, 960);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

D 13
  ADD("Chaining:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Extra calls:", INTBOX, OFF(multiple), SIZ(multiple), NULL, NULL, 1465);
  ADD("Calls another", BOLBOX, OFF(chains), SIZ(chains), NULL, NULL, 1470);
  ADD("Object called", LINKPIC, OFF(chainobj), SIZ(chainobj), NULL, NULL, 1480);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

E 13
  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
I 13
  ADD("Color", LSTRBOX, OFF(color), SIZ(color), NULL, NULL, 280);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
D 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
E 13

  ADD("Alteration:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Will cause alter", INTBOX, OFF(opens), SIZ(opens), NULL, NULL, 970);
  ADD("Has an id (x)", BOLBOX, OFF(id), SIZ(id), NULL, NULL, 975);
  ADD("Is alterable", BOLBOX, OFF(swings), SIZ(swings), NULL, NULL, 980);
  ADD("Becomes", LINKPIC, OFF(alternate), SIZ(alternate), NULL, NULL, 990);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Clothing:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Location bits", INTBOX, OFF(wearable), SIZ(wearable), NULL, NULL, 1010);
D 13
  ADD("Removal time(ms)", INTBOX, OFF(weartime), SIZ(weartime), NULL, NULL, 1020);
  ADD("Shots miss at", INTBOX, OFF(defense), SIZ(defense), NULL, NULL, 1030);
  ADD("Points absorbed", INTBOX, OFF(absorb), SIZ(absorb), NULL, NULL, 1040);
  ADD("Fully deflects", INTBOX, OFF(deflect), SIZ(deflect), NULL, NULL, 1050);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

D 13
  ADD("Special/Magical:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Carrybits set", INTBOX, OFF(carrybits), SIZ(carrybits), NULL, NULL, 1100);
  ADD("Wearbits set", INTBOX, OFF(wearbits), SIZ(wearbits), NULL, NULL, 1110);
  ADD("Holdbits set", INTBOX, OFF(holdbits), SIZ(holdbits), NULL, NULL, 1120);
  ADD("Steals at", INTBOX, OFF(theft), SIZ(theft), NULL, NULL, 1130);
  ADD("Magic id ptrs", INTBOX, OFF(magic), SIZ(magic), NULL, NULL, 1140);
D 10
  ADD("Property text", INTBOX, OFF(specialmsg), SIZ(specialmsg), NULL, NULL, 1170);
E 10
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

E 13
  ADD("Override object:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 13
  ADD("Use up charges", BOLBOX, OFF(pullcharges), SIZ(pullcharges), NULL, NULL, 1150);
E 13
I 13
  ADD("Override bits", INTBOX, OFF(overrider), SIZ(overrider), NULL, NULL, 1167);
  ADD("Overridden bits", INTBOX, OFF(overridden), SIZ(overridden), NULL, NULL, 1163);
E 13
  ADD("Force move over", BOLBOX, OFF(ignoremove), SIZ(ignoremove), NULL, NULL, 1152);
  ADD("Force see through", BOLBOX, OFF(ignoretrans), SIZ(ignoretrans), NULL, NULL, 1154);
  ADD("Force fire over", BOLBOX, OFF(ignoreperm), SIZ(ignoreperm), NULL, NULL, 1156);
D 13
  ADD("Supress autoexec", BOLBOX, OFF(ignoreauto), SIZ(ignoreauto), NULL, NULL, 1158);
  ADD("Affected object", LINKPIC, OFF(override), SIZ(override), NULL, NULL, 1160);
E 13
I 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
E 13

  ADD("Record defaults:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Enforce", BOLBOX, OFF(set[0]), SIZ(set[0]), NULL, NULL, 1180);
  ADD("  detail", INTBOX, OFF(def[0]), SIZ(def[0]), NULL, NULL, 1190);
  ADD("Enforce", BOLBOX, OFF(set[1]), SIZ(set[1]), NULL, NULL, 1200);
  ADD("  xcoord", INTBOX, OFF(def[1]), SIZ(def[1]), NULL, NULL, 1210);
  ADD("Enforce", BOLBOX, OFF(set[2]), SIZ(set[2]), NULL, NULL, 1220);
  ADD("  ycoord", INTBOX, OFF(def[2]), SIZ(def[2]), NULL, NULL, 1230);
  ADD("Enforce", BOLBOX, OFF(set[3]), SIZ(set[3]), NULL, NULL, 1240);
  ADD("  zinger", INTBOX, OFF(def[3]), SIZ(def[3]), NULL, NULL, 1250);
  ADD("Enforce", BOLBOX, OFF(set[4]), SIZ(set[4]), NULL, NULL, 1260);
  ADD("  extra1", INTBOX, OFF(def[4]), SIZ(def[4]), NULL, NULL, 1270);
  ADD("Enforce", BOLBOX, OFF(set[5]), SIZ(set[5]), NULL, NULL, 1280);
  ADD("  extra2", INTBOX, OFF(def[5]), SIZ(def[5]), NULL, NULL, 1290);
  ADD("Enforce", BOLBOX, OFF(set[6]), SIZ(set[6]), NULL, NULL, 1300);
  ADD("  extra3", INTBOX, OFF(def[6]), SIZ(def[6]), NULL, NULL, 1310);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

D 13
  ADD("Auto activation:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Is auto-actor", BOLBOX, OFF(autoactor), SIZ(autoactor), NULL, NULL, 1320);
  ADD("Is auto-taken", BOLBOX, OFF(autotaken), SIZ(autotaken), NULL, NULL, 1330);
  ADD("Act delay(ms)", INTBOX, OFF(autodelay), SIZ(autodelay), NULL, NULL, 1340);
  ADD("Repeating(ms)", INTBOX, OFF(autorepeat), SIZ(autorepeat), NULL, NULL, 1350);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

E 13
  ADD("Breakage:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Breaks at", INTBOX, OFF(breaks), SIZ(breaks), NULL, NULL, 1360);
  ADD("When broken", LINKPIC, OFF(broken), SIZ(broken), NULL, NULL, 1370);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Inventory:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Number born with", INTBOX, OFF(bornwith), SIZ(bornwith), NULL, NULL, 1380);
D 13
  ADD("Unstealable", BOLBOX, OFF(secure), SIZ(secure), NULL, NULL, 1390);
  ADD("Unswitchable", BOLBOX, OFF(unswitchable), SIZ(unswitchable), NULL, NULL, 1400);
E 13
  ADD("Unremoveable", BOLBOX, OFF(unremoveable), SIZ(unremoveable), NULL, NULL, 1410);
I 13
  ADD("Unswitchable", BOLBOX, OFF(unswitchable), SIZ(unswitchable), NULL, NULL, 1400);
E 13
  ADD("Undroppable", BOLBOX, OFF(undroppable), SIZ(undroppable), NULL, NULL, 1420);
  ADD("No death drop", BOLBOX, OFF(nodeathdrop), SIZ(nodeathdrop), NULL, NULL, 1430);
D 13
  ADD("Drops from Inv", LINKPIC, OFF(dropper), SIZ(dropper), NULL, NULL, 1440);
  ADD("Far drops Inv", LINKPIC, OFF(fardropper), SIZ(fardropper), NULL, NULL, 1442);
  ADD("Destroys in Inv", LINKPIC, OFF(destroyer), SIZ(destroyer), NULL, NULL, 1450);
  ADD("Creates in Inv", LINKPIC, OFF(creator), SIZ(creator), NULL, NULL, 1460);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Miscellaneous:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
D 13
  ADD("Makes user move", BOLBOX, OFF(makesmove), SIZ(makesmove), NULL, NULL, 1490);
  ADD("Swaps map squares", BOLBOX, OFF(swapper), SIZ(swapper), NULL, NULL, 1500);
E 13
I 13
  ADD("Secretly carried", BOLBOX, OFF(secret), SIZ(secret), NULL, NULL, 1125);
  ADD("Magic id ptrs", INTBOX, OFF(magic), SIZ(magic), NULL, NULL, 1140);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Winning:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Is important", BOLBOX, OFF(important), SIZ(important), NULL, NULL, 1060);
  ADD("Is a flag", BOLBOX, OFF(flag), SIZ(flag), NULL, NULL, 1070);
  ADD("Needy teams", INTBOX, OFF(flagteams), SIZ(flagteams), NULL, NULL, 1080);
D 13
  ADD("Uses to win", INTBOX, OFF(winner), SIZ(winner), NULL, NULL, 1090);
E 13
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
I 12

D 13
  /* Do not use property listed in properties below */
  /* These property id's were once used with now non existent property types */
E 13
I 13
  ADD("Forth:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Specific code", HIDSTR, OFF(code), SIZ(code), NULL, NULL, 1500);
  ADD("Variable space", INTBOX, OFF(varspace), SIZ(varspace), NULL, NULL, 1510);
  ADD("Linked object A", LINKPIC, OFF(object1), SIZ(object1), NULL, NULL, 1520);
  ADD("Linked object B", LINKPIC, OFF(object2), SIZ(object2), NULL, NULL, 1530);
  ADD("Linked object C", LINKPIC, OFF(object3), SIZ(object3), NULL, NULL, 1540);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  /* Not using properties listed below */
  /* These property id's were once used with now non-existent property types */
E 13
  /* Lines below are for the retirement of old object properties no longer
D 13
     in use.  You may never reuse these object property numbers.
E 13
I 13
     in use.  You may never reuse their object property numbers.
E 13
     They are included below so that files containing them may still be
     understood (as far as lengths of the pieces of data are concerned.) */
  OLD("From afar", INTBOX, OFF(lookmsg), SIZ(lookmsg), NULL, NULL, 150);
  OLD("Examination", INTBOX, OFF(examinemsg), SIZ(examinemsg), NULL, NULL, 160);
  OLD("Personal", INTBOX, OFF(usemsg), SIZ(usemsg), NULL, NULL, 180);
  OLD("Within room", INTBOX, OFF(roommsg), SIZ(roommsg), NULL, NULL, 190);
  OLD("Everyone", INTBOX, OFF(allmsg), SIZ(allmsg), NULL, NULL, 200);
  OLD("Alerter", BOLBOX, OFF(alerter), SIZ(alerter), NULL, NULL, 210);
  OLD("Personal", INTBOX, OFF(usealtermsg), SIZ(usealtermsg), NULL, NULL, 220);
  OLD("Within room", INTBOX, OFF(roomaltermsg), SIZ(roomaltermsg), NULL, NULL, 230);
  OLD("Everyone", INTBOX, OFF(allaltermsg), SIZ(allaltermsg), NULL, NULL, 240);
  OLD("For hitter", INTBOX, OFF(hitmsg), SIZ(hitmsg), NULL, NULL, 250);
  OLD("For victim", INTBOX, OFF(victimmsg), SIZ(victimmsg), NULL, NULL, 260);
  OLD("Property text", INTBOX, OFF(specialmsg), SIZ(specialmsg), NULL, NULL, 1170);
I 13
  OLD("Not outside", BOLBOX, OFF(notoutside), SIZ(notoutside), NULL, NULL, 300);
  OLD("Not inside", BOLBOX, OFF(notinside), SIZ(notinside), NULL, NULL, 310);
  OLD("All teams in", BOLBOX, OFF(represented), SIZ(represented), NULL, NULL, 320);
  OLD("Delay(ms)", INTBOX, OFF(delay), SIZ(delay), NULL, NULL, 430);
  OLD("Frozen in delay", BOLBOX, OFF(mustwait), SIZ(mustwait), NULL, NULL, 440);
  OLD("Vehicle:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  OLD("Num occupants", INTBOX, OFF(vehicle), SIZ(vehicle), NULL, NULL, 640);
  OLD("Drivespeed", INTBOX, OFF(drivespeed), SIZ(drivespeed), NULL, NULL, 660);
  OLD("May fire in", BOLBOX, OFF(firein), SIZ(firein), NULL, NULL, 650);
  OLD("May fire out", BOLBOX, OFF(fireout), SIZ(fireout), NULL, NULL, 700);
  OLD("Is entry square", BOLBOX, OFF(entry), SIZ(entry), NULL, NULL, 670);
  OLD("Is driver square", BOLBOX, OFF(driver), SIZ(driver), NULL, NULL, 680);
  OLD("Is window square", BOLBOX, OFF(window), SIZ(window), NULL, NULL, 690);
  OLD("Removal time(ms)", INTBOX, OFF(weartime), SIZ(weartime), NULL, NULL, 1020);
  OLD("Uses to win", INTBOX, OFF(winner), SIZ(winner), NULL, NULL, 1090);
  OLD("Carrybits set", INTBOX, OFF(carrybits), SIZ(carrybits), NULL, NULL, 1100);
  OLD("Wearbits set", INTBOX, OFF(wearbits), SIZ(wearbits), NULL, NULL, 1110);
  OLD("Holdbits set", INTBOX, OFF(holdbits), SIZ(holdbits), NULL, NULL, 1120);
  OLD("Auto activation:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  OLD("Is auto-actor", BOLBOX, OFF(autoactor), SIZ(autoactor), NULL, NULL, 1320);
  OLD("Is auto-taken", BOLBOX, OFF(autotaken), SIZ(autotaken), NULL, NULL, 1330);
  OLD("Act delay(ms)", INTBOX, OFF(autodelay), SIZ(autodelay), NULL, NULL, 1340);
  OLD("Repeating(ms)", INTBOX, OFF(autorepeat), SIZ(autorepeat), NULL, NULL, 1350);
  OLD("Use up charges", BOLBOX, OFF(pullcharges), SIZ(pullcharges), NULL, NULL, 1150);
  OLD("Supress autoexec", BOLBOX, OFF(ignoreauto), SIZ(ignoreauto), NULL, NULL, 1158);
  OLD("Affected object", LINKPIC, OFF(override), SIZ(override), NULL, NULL, 1160);
  OLD("Drops from Inv", LINKPIC, OFF(dropper), SIZ(dropper), NULL, NULL, 1440);
  OLD("Far drops Inv", LINKPIC, OFF(fardropper), SIZ(fardropper), NULL, NULL, 1442);
  OLD("Destroys in Inv", LINKPIC, OFF(destroyer), SIZ(destroyer), NULL, NULL, 1450);
  OLD("Creates in Inv", LINKPIC, OFF(creator), SIZ(creator), NULL, NULL, 1460);
  OLD("Swaps map squares", BOLBOX, OFF(swapper), SIZ(swapper), NULL, NULL, 1500);
  OLD("Steals at", INTBOX, OFF(theft), SIZ(theft), NULL, NULL, 1130);
  OLD("Unstealable", BOLBOX, OFF(secure), SIZ(secure), NULL, NULL, 1390);
  OLD("Alteration msgs:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  OLD("Personal", HIDSTR, OFF(usealtermsg), SIZ(usealtermsg), NULL, NULL, 221);
  OLD("Within room", HIDSTR, OFF(roomaltermsg), SIZ(roomaltermsg), NULL, NULL, 231);
  OLD("Everyone", HIDSTR, OFF(allaltermsg), SIZ(allaltermsg), NULL, NULL, 241);
  OLD("Chaining:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  OLD("Extra calls:", INTBOX, OFF(multiple), SIZ(multiple), NULL, NULL, 1465);
  OLD("Calls another", BOLBOX, OFF(chains), SIZ(chains), NULL, NULL, 1470);
  OLD("Object called", LINKPIC, OFF(chainobj), SIZ(chainobj), NULL, NULL, 1480);
  OLD("Makes user move", BOLBOX, OFF(makesmove), SIZ(makesmove), NULL, NULL, 1490);
  OLD("Into hand", BOLBOX, OFF(intohand), SIZ(intohand), NULL, NULL, 470);
  OLD("Which hand", INTBOX, OFF(whichhand), SIZ(whichhand), NULL, NULL, 480);
  OLD("Shots miss at", INTBOX, OFF(defense), SIZ(defense), NULL, NULL, 1030);
  OLD("Points absorbed", INTBOX, OFF(absorb), SIZ(absorb), NULL, NULL, 1040);
  OLD("Fully deflects", INTBOX, OFF(deflect), SIZ(deflect), NULL, NULL, 1050);
  OLD("Connects other", INTBOX, OFF(connectsother), SIZ(connectsother), NULL, NULL, 630);
  OLD("Connects up", BOLBOX, OFF(connectsup), SIZ(connectsup), NULL, NULL, 590);
  OLD("Connects down", BOLBOX, OFF(connectsdown), SIZ(connectsdown), NULL, NULL, 600);
  OLD("Connects left", BOLBOX, OFF(connectsleft), SIZ(connectsleft), NULL, NULL, 610);
  OLD("Connects right", BOLBOX, OFF(connectsright), SIZ(connectsright), NULL, NULL, 620);
  OLD("Putdown bits", INTBOX, OFF(putsdown), SIZ(putsdown), NULL, NULL, 900);
  OLD("Does vampire", BOLBOX, OFF(vampire), SIZ(vampire), NULL, NULL, 850);
  OLD("Hide properties", BOLBOX, OFF(hideprops), SIZ(hideprops), NULL, NULL, 170);
E 13
  /* end of retired properties list */
E 12

   /* Obtor uses NULL name to see end, load/store routines use ENDLIST */
  ADD(NULL, ENDLIST, 0, 0, NULL, NULL, NULLID);

  return new;
}



DialogList *StaticObjectProperties()
/* returns a pointer to the array that describes the properties that
   any particular object can be composed of.  This list must not be freed. */
{
  static DialogList *dialist = NULL;

  if (!dialist) dialist = AllocObjectProperties();
I 5
  return dialist;
}



/* =========================== M A P S ================================= */

I 9

/* WARNING: this needs to be fixed before adding rest of map stuff.
   LSTRBOXes are now different than they used to be. */
E 9

DialogList *AllocMapProperties()
/* returns a pointer to a newly allocated array of the properties a map
   can have.  Caller must free the array when done with it. */
{
  int space = 0, size = 0;
  DialogList *new = NULL;
  MapInfo tobj;
I 6
  int EditMapInfoSave(), EditMapInfoQuit();
E 6

  ADD("Map specifics:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Map name", LSTRBOX, ARR(name), SIZ(name), NULL, NULL, 100);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Team name 1", LSTRBOX,
      (((char *) (tobj.team_name[0])) - ((char *) &(tobj))),
      sizeof(tobj.team_name[0]), NULL, NULL, 110);
  ADD("Team name 2", LSTRBOX,
      (((char *) (tobj.team_name[1])) - ((char *) &(tobj))),
      sizeof(tobj.team_name[1]), NULL, NULL, 120);
  ADD("Team name 3", LSTRBOX,
      (((char *) (tobj.team_name[2])) - ((char *) &(tobj))),
      sizeof(tobj.team_name[2]), NULL, NULL, 130);
  ADD("Team name 4", LSTRBOX,
      (((char *) (tobj.team_name[3])) - ((char *) &(tobj))),
      sizeof(tobj.team_name[3]), NULL, NULL, 140);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Number of teams", INTBOX, OFF(teams_supported), SIZ(teams_supported),
      NULL, NULL, 150);
  ADD("Are neutrals allowed", BOLBOX, OFF(neutrals_allowed), SIZ(neutrals_allowed),
      NULL, NULL, 155);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Object definitions file", LSTRBOX, ARR(objfilename), SIZ(objfilename),
      NULL, NULL, 160);
  ADD("Execution file", LSTRBOX, ARR(execute_file), SIZ(execute_file),
      NULL, NULL, 170);
  ADD("Startup file", LSTRBOX, ARR(startup_file), SIZ(startup_file),
      NULL, NULL, 180);
  ADD("Placement file", LSTRBOX, ARR(placement_file), SIZ(placement_file),
      NULL, NULL, 190);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

I 6
  ADD("Save Changes", QUITBOX, 0, 0, EditMapInfoSave, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Quit", QUITBOX, 0, 0, EditMapInfoQuit, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
I 7

E 7
  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD(NULL, ENDLIST, 0, 0, NULL, NULL, NULLID);

E 6
  return new;
}



DialogList *StaticMapProperties()
/* returns a pointer to the array that describes the properties that
   any particular map can be composed of.  This list must not be freed. */
{
  static DialogList *dialist = NULL;

  if (!dialist) dialist = AllocMapProperties();
  return dialist;
}



/* =========================== R O O M S ================================ */


DialogList *AllocRoomProperties()
/* returns a pointer to a newly allocated array of the properties a room
   can have.  Caller must free the array when done with it. */
{
  int space = 0, size = 0;
  DialogList *new = NULL;
  RoomInfo tobj;
I 7
  int EditRoomInfoSave(), EditRoomInfoQuit();
E 7

  ADD("Room specifics:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Name", LSTRBOX, ARR(name), SIZ(name), NULL, NULL, 100);
D 7
  ADD("Is dark", BOLBOX, ARR(dark), SIZ(dark), NULL, NULL, 110);
E 7
I 7
  ADD("Is dark", BOLBOX, OFF(dark), SIZ(dark), NULL, NULL, 110);
E 7
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
I 7

  ADD("Save Changes", QUITBOX, 0, 0, EditRoomInfoSave, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Quit", QUITBOX, 0, 0, EditRoomInfoQuit, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
E 7
I 6
  ADD(NULL, ENDLIST, 0, 0, NULL, NULL, NULLID);
E 6

  return new;
}



DialogList *StaticRoomProperties()
/* returns a pointer to the array that describes the properties that
   any particular room can be composed of.  This list must not be freed. */
{
  static DialogList *dialist = NULL;

D 7
  if (!dialist) dialist = AllocMapProperties();
E 7
I 7
  if (!dialist) dialist = AllocRoomProperties();
E 7
  return dialist;
}



/* =================== R E C O R D E D  O B J E C T S =================== */


DialogList *AllocRecObjProperties()
/* returns a pointer to a newly allocated array of the properties a
   recorded object can have.  Caller must free the array when done with it. */
{
  int space = 0, size = 0;
  DialogList *new = NULL;
  RecordedObj tobj;
I 7
  int EditRecObjSave(), EditRecObjQuit();
E 7

  ADD("Record object fields:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("detail", INTBOX, OFF(detail), SIZ(detail), NULL, NULL, 120);
D 7
  ADD("x", INTBOX, OFF(infox), SIZ(infox), NULL, NULL, 130);
  ADD("y", INTBOX, OFF(infoy), SIZ(infoy), NULL, NULL, 140);
E 7
I 7
  ADD("     x", INTBOX, OFF(infox), SIZ(infox), NULL, NULL, 130);
  ADD("     y", INTBOX, OFF(infoy), SIZ(infoy), NULL, NULL, 140);
E 7
  ADD("zinger", INTBOX, OFF(zinger), SIZ(zinger), NULL, NULL, 150);
  ADD("extra1", INTBOX, OFF(extra[0]), SIZ(extra[0]), NULL, NULL, 160);
  ADD("extra2", INTBOX, OFF(extra[1]), SIZ(extra[1]), NULL, NULL, 170);
  ADD("extra3", INTBOX, OFF(extra[2]), SIZ(extra[2]), NULL, NULL, 180);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
I 7

  ADD("Save Changes", QUITBOX, 0, 0, EditRecObjSave, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Quit", QUITBOX, 0, 0, EditRecObjQuit, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
E 7
I 6
  ADD(NULL, ENDLIST, 0, 0, NULL, NULL, NULLID);
E 6

  return new;
}



DialogList *StaticRecObjProperties()
/* returns a pointer to the array that describes the properties that
   any particular recorded object can be composed of.
   This list must not be freed. */
{
  static DialogList *dialist = NULL;

D 7
  if (!dialist) dialist = AllocMapProperties();
E 7
I 7
  if (!dialist) dialist = AllocRecObjProperties();
I 11
  return dialist;
}



/* ================ O B J E C T  F I L E  H E A D E R ================== */

DialogList *AllocHeaderProperties()
/* returns a pointer to a newly allocated array of the properties in
   the header for an object definition file */
{
  int space = 0, size = 0;
  DialogList *new = NULL;
  ObjHeader tobj;
  int EditHeaderSave(), EditHeaderQuit();

  ADD("Object File Information:", TITLEBOX, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Object set", MARKSHO, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Set Name", LSTRBOX, OFF(name), SIZ(name), NULL, NULL, 100);
  ADD("Author's Name", LSTRBOX, OFF(author), SIZ(author), NULL, NULL, 110);
  ADD("Code filename", LSTRBOX, OFF(codefile), SIZ(codefile), NULL, NULL, 120);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("Save Changes", QUITBOX, 0, 0, EditHeaderSave, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD("Quit", QUITBOX, 0, 0, EditHeaderQuit, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);

  ADD("", NEWCOL, 0, 0, NULL, NULL, NULLID);
  ADD("", BLANKBOX, 0, 0, NULL, NULL, NULLID);
  ADD(NULL, ENDLIST, 0, 0, NULL, NULL, NULLID);

  return new;
}



DialogList *StaticHeaderProperties()
/* returns a pointer to the array that describes the properties that
   any particular header object can be composed of.
   This list must not be freed. */
{
  static DialogList *dialist = NULL;

  if (!dialist) dialist = AllocHeaderProperties();
E 11
E 7
E 5
  return dialist;
}
E 2
I 1
E 1
