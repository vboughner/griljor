h23067
s 00000/00000/01478
d D 1.4 92/08/07 01:03:46 vbo 4 3
c source copied to a separate tree for work on new map and object format
e
s 00028/00027/01450
d D 1.3 91/05/02 16:52:11 labc-3id 3 2
c 
e
s 00001/00003/01476
d D 1.2 91/02/18 21:13:47 labc-3id 2 1
c 
e
s 01479/00000/00000
d D 1.1 91/02/16 12:55:51 labc-3id 1 0
c date and time created 91/02/16 12:55:51 by labc-3id
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
 * February 1991
 **************************************************************************/

/* code to handle line of sight vision for players */

#include <stdio.h>
#include "config.h"
#include "def.h"
#include "ray.h"
D 2
#include "rulepack.h"
E 2
#include "queued.h"

D 2

E 2
/* This is the sighting routine for unlit rooms.  Given the location
 * (room, x, y) of the Ego and the radius of his/her light source, it
 * will determine which squares need to be drawn and/or undrawn, and
 * drawn/undraw them.  It further updates the lighting/sighting arrays
 * for use by other ray procedures (i. e. DrawOthersLightRays) */

void DrawEgoRaysWithLight(room, x, y, light)
{  int i, j, mr, lr;
 
  /* ================== Handle Own Square ========================= */
 
  /* the player will always see his own square */
  if (!raypack[x][y].drawn) DrawOwnSquare(room, x, y);
D 2
 
E 2
I 2

E 2
  /* the player always casts ego rays */
  raypack[x][y].botego = 1;
  raypack[x][y].ritego = 1;
  raypack[x-1][y].ritego = 1;
  raypack[x][y+1].botego = 1;
 
  /* if the player has light, he casts light rays too. */
  if (light > 0) { 
    raypack[x][y].botlit |= 1;
    raypack[x][y].ritlit |= 1;
    raypack[x-1][y].ritlit |= 1;
    raypack[x][y+1].botlit |= 1;
  }
  else { /* no light */
    raypack[x][y].botlit &= ~1;
    raypack[x][y].ritlit &= ~1;
    raypack[x-1][y].ritlit &= ~1;
    raypack[x][y+1].botlit &= ~1;
  }
 
 
  /* =================== Handle Adjacent Squares ======================== */
 
  /* ---------------- vertical and horizantal first --------------------- */
 
  /* top */
  if(y+1 < ROOM_HEIGHT) { /* make sure this is on screen */
    if((raypack[x][y+1].botlit & 1) && /* did the light make it this far? */
       IsTransparent(room, x, y+1)) { /* can it pass through? */
      raypack[x][y+1].ritlit |= 1;
      raypack[x-1][y+1].ritlit |= 1;
      if (light > 2) { /* Is this the end of the range? */
        raypack[x][y+2].botlit |= 1;
      }
      else {
        raypack[x][y+2].botlit &= ~1;
      }
    }
    else {
      raypack[x][y+1].ritlit &= ~1;
      raypack[x-1][y+1].ritlit &= ~1;
      raypack[x][y+2].botlit &= ~1;
    }
 
    if(raypack[x][y+1].botego && /* did the ego rays make it this far? */
       IsTransparent(room, x, y+1)) { /* can they pass through? */
      raypack[x][y+1].ritego = 1;
      raypack[x-1][y+1].ritego = 1;
      raypack[x][y+2].botego = 1;
    }
    else {
      raypack[x][y+1].ritego = 0;
      raypack[x-1][y+1].ritego = 0;
      raypack[x][y+2].botego = 0;
    }
 
    if(raypack[x][y+1].botlit && raypack[x][y+1].botego) {
      if(!raypack[x][y+1].drawn) DrawUnconcealableSquare(room, x, y+1);
      if(HFLAG(room, x, y+1)) {
      	HSET(x+1, y+1);
      	HSET(x-1, y+1);
      }
    }
    else {
      if(raypack[x][y+1].drawn) EraseSquare(room, x, y+1);
      if(!HFLAG(room, x, y+1)) {
      	UNHSET(x+1, y+1);
      	UNHSET(x-1, y+1);
      }
    }
  }
 
  /* bottom */
  if(y-1 > -1) { /* make sure this is on screen */
    if((raypack[x][y].botlit & 1) && /* did the light make it this far? */
       IsTransparent(room, x, y-1)) { /* can it continue? */
      raypack[x][y-1].ritlit |= 1;
      raypack[x-1][y-1].ritlit |= 1;
      if (light > 2) { /* is this past the light range? */
        raypack[x][y-1].botlit |= 1;
      }
      else {
        raypack[x][y-1].botlit &= ~1;
      }
    }
    else {
      raypack[x][y-1].ritlit &= ~1;
      raypack[x-1][y-1].ritlit &= ~1;
      raypack[x][y-1].botlit &= ~1;
    }
 
    if((raypack[x][y].botego & 1) && /* did the ego rays make it this far? */
       IsTransparent(room, x, y-1)) { /* can it continue? */
      raypack[x][y-1].ritego = 1;
      raypack[x-1][y-1].ritego = 1;
      raypack[x][y-1].botego = 1;
    }
    else {
      raypack[x][y-1].ritego = 0;
      raypack[x-1][y-1].ritego = 0;
      raypack[x][y-1].botego = 0;
    }
 
    if(raypack[x][y].botlit && raypack[x][y].botego) {
      if(!raypack[x][y-1].drawn) DrawUnconcealableSquare(room, x, y-1);
      if(HFLAG(room, x, y-1)) {
      	HSET(x+1, y-1);
      	HSET(x-1, y-1);
      }
    }
    else {
      if(raypack[x][y-1].drawn) EraseSquare(room, x, y-1);
      if(!HFLAG(room, x, y-1)) {
      	UNHSET(x+1, y-1);
      	UNHSET(x-1, y-1);
      }
    }
  }
  
  /* left */
  if(x-1 > -1) { /* make sure this is on screen */
D 3
    if((raypack[x][y].ritlit & 1) && /* did the light make it this far? */
E 3
I 3
    if((raypack[x-1][y].ritlit & 1) && /* did the light make it this far? */
E 3
       IsTransparent(room, x-1, y)) { /* can it continue? */
      raypack[x-1][y].botlit |= 1;
      raypack[x-1][y+1].botlit |= 1;
      if (light > 2) { /* will the light make it farther? */
D 3
        raypack[x-1][y].ritlit |= 1;
E 3
I 3
        raypack[x-2][y].ritlit |= 1;
E 3
      }
      else {
D 3
        raypack[x-1][y].ritlit &= ~1;
E 3
I 3
        raypack[x-2][y].ritlit &= ~1;
E 3
      }
    }
    else {
      raypack[x-1][y].botlit &= ~1;
      raypack[x-1][y+1].botlit &= ~1;
D 3
      raypack[x-1][y].ritlit &= ~1;
E 3
I 3
      raypack[x-2][y].ritlit &= ~1;
E 3
    }
 
D 3
    if((raypack[x][y].ritego & 1) && /* did the ego rays make it this far? */
E 3
I 3
    if((raypack[x-1][y].ritego & 1) && /* did the ego rays make it this far? */
E 3
       IsTransparent(room, x-1, y)) { /* can it continue? */
      raypack[x-1][y].botego = 1;
      raypack[x-1][y+1].botego = 1;
D 3
      raypack[x-1][y].ritego = 1;
E 3
I 3
      raypack[x-2][y].ritego = 1;
E 3
    }
    else {
      raypack[x-1][y].botego = 0;
      raypack[x-1][y+1].botego = 0;
D 3
      raypack[x-1][y].ritego = 0;
E 3
I 3
      raypack[x-2][y].ritego = 0;
E 3
    }
 
    if(raypack[x-1][y].ritlit && raypack[x-1][y].ritego) {
      if(!raypack[x-1][y].drawn) DrawUnconcealableSquare(room, x-1, y);
      if(VFLAG(room, x-1,y)) {
      	VSET(x-1,y-1);
      	VSET(x-1,y+1);
      }
    }
    else {
      if(raypack[x-1][y].drawn) EraseSquare(room, x-1, y);
      if(!VFLAG(room, x-1,y)) {
      	UNVSET(x-1,y-1);
      	UNVSET(x-1,y+1);
      }
    }
  }
  
  /* right */
D 3
  if(x+1 > ROOM_WIDTH) { /* make sure this is on screen */
    if((raypack[x+1][y].ritlit & 1) && /* did the light make it this far? */
E 3
I 3
  if(x+1 < ROOM_WIDTH) { /* make sure this is on screen */
    if((raypack[x][y].ritlit & 1) && /* did the light make it this far? */
E 3
       IsTransparent(room, x+1, y)) { /* can it continue? */
      raypack[x+1][y].botlit |= 1;
      raypack[x+1][y+1].botlit |= 1;
      if (light > 2) { /* will the light make it farther? */
D 3
        raypack[x+2][y].ritlit |= 1;
E 3
I 3
        raypack[x+1][y].ritlit |= 1;
E 3
      }
      else {
D 3
        raypack[x+2][y].ritlit &= ~1;
E 3
I 3
        raypack[x+1][y].ritlit &= ~1;
E 3
      }
    }
    else {
      raypack[x+1][y].botlit &= ~1;
      raypack[x+1][y+1].botlit &= ~1;
D 3
      raypack[x+2][y].ritlit &= ~1;
E 3
I 3
      raypack[x+1][y].ritlit &= ~1;
E 3
    }
 
D 3
    if(raypack[x+1][y].ritego && /* did the ego rays make it this far? */
E 3
I 3
    if(raypack[x][y].ritego && /* did the ego rays make it this far? */
E 3
       IsTransparent(room, x+1, y)) { /* can it continue? */
      raypack[x+1][y].botego = 1;
      raypack[x+1][y+1].botego = 1;
D 3
      raypack[x+2][y].ritego = 1;
E 3
I 3
      raypack[x+1][y].ritego = 1;
E 3
    }
    else {
      raypack[x+1][y].botego = 0;
      raypack[x+1][y+1].botego = 0;
D 3
      raypack[x+2][y].ritego = 0;
E 3
I 3
      raypack[x+1][y].ritego = 0;
E 3
    }
 
D 3
    if(raypack[x+1][y].ritlit && raypack[x+1][y].ritego) {
E 3
I 3
    if(raypack[x][y].ritlit && raypack[x][y].ritego) {
E 3
      if(!raypack[x][y].drawn) DrawUnconcealableSquare(room, x+1, y);
      if(VFLAG(room, x+1,y)) {
      	VSET(x+1,y+1);
      	VSET(x+1,y-1);
      }
    }
    else {
      if(raypack[x][y].drawn) EraseSquare(room, x+1, y);
      if(!VFLAG(room, x+1,y)) {
      	UNVSET(x+1,y+1);
      	UNVSET(x+1,y-1);
      }
    }
  }

  /* --------------------------- diagonals ------------------------------- */
  /* left-up */
  if(y+1 < ROOM_HEIGHT && x-1 > -1) { /* make sure space is on screen */
    if(IsTransparent(room, x-1, y+1)) { /* should rays continue? */
      if(light > 1) { /* end of light range? */
      	if(raypack[x-1][y+1].ritlit & 1) {
      	  raypack[x-1][y+2].botlit |= 1;
      	  raypack[x-1][y+2].lrl = 1;
      	  raypack[x-1][y+2].mrl = 1;
	}
      	else raypack[x-1][y+2].botlit &= ~1;
        if(raypack[x-1][y+1].botlit) {
          raypack[x-2][y+1].ritlit |= 1;
          raypack[x-2][y+1].mrl = 1;
          raypack[x-2][y+1].lrl = 1;
        }
        else raypack[x-2][y+1].ritlit &= ~1;
      }
      if(raypack[x-1][y+1].ritego) {
      	raypack[x-1][y+2].botego = 1;
      	raypack[x-1][y+2].mre = 1;
      	raypack[x-1][y+2].lre = 1;
      }
      else raypack[x-1][y+2].botego = 0;
      if(raypack[x-1][y+1].botego) {
      	raypack[x-2][y+1].ritego = 1;
      	raypack[x-2][y+1].mre = 1;
      	raypack[x-2][y+1].lre = 1;
      }
      else raypack[x-2][y+1].ritego = 0;
    }
    if(raypack[x-1][y+1].botego && raypack[x-1][y+1].botlit) {
      if (!raypack[x-1][y+1].drawn) DrawUnconcealableSquare(room,x-1,y+1);
      if (HFLAG(room, x-1, y+1)) HSET(x-2, y+1);
    }
    else {
      if (raypack[x-1][y+1].drawn) EraseSquare(room,x-1,y+1);
      if (!HFLAG(room, x-1, y+1)) UNHSET(x-2, y+1);
    }
    if(raypack[x-1][y+1].ritego && raypack[x-1][y+1].ritlit) {
      if (!raypack[x-1][y+1].drawn) DrawUnconcealableSquare(room,x-1,y+1);
      if (VFLAG(room, x-1, y+1)) VSET(x-1, y+2);
    }
    else {
      if (raypack[x-1][y+1].drawn) EraseSquare(room,x-1,y+1);
      if (!VFLAG(room, x-1, y+1)) UNVSET(x-1, y+2);
    }
  }
 
  /* right up */
  if(y+1 < ROOM_HEIGHT && x+1 < ROOM_WIDTH) { /* make sure space is on screen */
    if(IsTransparent(room, x+1, y+1)) { /* should rays continue? */
      if(light > 1) { /* end of light range? */
      	if(raypack[x][y+1].ritlit & 1) {
      	  raypack[x+1][y+2].botlit |= 1;
      	  raypack[x+1][y+2].lrl = 1;
      	  raypack[x+1][y+2].mrl = 1;
	}
      	else raypack[x+1][y+2].botlit &= ~1;
        if(raypack[x+1][y+1].botlit) {
          raypack[x+1][y+1].ritlit |= 1;
          raypack[x+2][y+1].mrl = 1;
          raypack[x+2][y+1].lrl = 1;
        }
        else raypack[x+1][y+1].ritlit &= ~1;
      }
      if(raypack[x][y+1].ritego) {
      	raypack[x+1][y+2].botego = 1;
      	raypack[x+1][y+2].mre = 1;
      	raypack[x+1][y+2].lre = 1;
      }
      else raypack[x+1][y+2].botego = 0;
      if(raypack[x+1][y+1].botego) {
      	raypack[x+1][y+1].ritego = 1;
      	raypack[x+2][y+1].mre = 1;
      	raypack[x+2][y+1].lre = 1;
      }
      else raypack[x+1][y+1].ritego = 0;
    }
    if(raypack[x+1][y+1].botego && raypack[x+1][y+1].botlit) {
      if (!raypack[x+1][y+1].drawn) DrawUnconcealableSquare(room,x+1,y+1);
      if (HFLAG(room, x+1, y+1)) HSET(x+2, y+1);
    }
    else {
      if (raypack[x+1][y+1].drawn) EraseSquare(room,x+1,y+1);
      if (HFLAG(room, x+1, y+1)) HSET(x+2, y+1);
    }
    if(raypack[x][y+1].ritego && raypack[x][y+1].ritlit) {
      if (!raypack[x+1][y+1].drawn) DrawUnconcealableSquare(room,x+1,y+1);
      if (VFLAG(room, x+1, y+1)) VSET(x+1, y+2);
    }
    else {
      if (raypack[x+1][y+1].drawn) EraseSquare(room,x+1,y+1);
      if (VFLAG(room, x+1, y+1)) VSET(x+1, y+2);
    }
  }
 
  /*right down */
  if(y-1 > -1 && x+1 < ROOM_WIDTH) { /* make sure space is on screen */
    if(IsTransparent(room, x+1, y-1)) { /* should rays continue? */
      if(light > 1) { /* end of light range? */
      	if(raypack[x][y-1].ritlit & 1) {
      	  raypack[x+1][y-1].botlit |= 1;
      	  raypack[x+1][y-2].lrl = 1;
      	  raypack[x+1][y-2].mrl = 1;
	}
      	else raypack[x+1][y-1].botlit &= ~1;
        if(raypack[x+1][y].botlit) {
          raypack[x+1][y-1].ritlit |= 1;
          raypack[x+2][y-1].mrl = 1;
          raypack[x+2][y-1].lrl = 1;
        }
        else raypack[x+1][y-1].ritlit &= ~1;
      }
      if(raypack[x][y-1].ritego) {
      	raypack[x+1][y-1].botego = 1;
      	raypack[x+1][y-2].mre = 1;
      	raypack[x+1][y-2].lre = 1;
      }
      else raypack[x+1][y-1].botego = 0;
      if(raypack[x+1][y].botego) {
      	raypack[x+1][y-1].ritego = 1;
      	raypack[x+2][y-1].mre = 1;
      	raypack[x+2][y-1].lre = 1;
      }
      else raypack[x+1][y-1].ritego = 0;
    }
    if(raypack[x+1][y].botego && raypack[x+1][y].botlit) {
      if (!raypack[x+1][y-1].drawn) DrawUnconcealableSquare(room,x+1,y-1);
      if (HFLAG(room, x+1, y-1)) HSET(x+2, y-1);
    }
    else {
      if (raypack[x+1][y-1].drawn) EraseSquare(room,x+1,y-1);
      if (HFLAG(room, x+1, y-1)) HSET(x+2, y-1);
    }
    if(raypack[x][y-1].ritego && raypack[x][y-1].ritlit) {
      if (!raypack[x+1][y-1].drawn) DrawUnconcealableSquare(room,x+1,y-1);
      if (VFLAG(room, x+1, y-1)) VSET(x+1, y-2);
    }
    else {
      if (raypack[x+1][y-1].drawn) EraseSquare(room,x+1,y-1);
      if (VFLAG(room, x+1, y-1)) VSET(x+1, y-2);
    }
  }
 
  /* left down */
  if(y-1 > -1 && x-1 > -1) { /* make sure space is on screen */
    if(IsTransparent(room, x-1, y-1)) { /* should rays continue? */
      if(light > 1) { /* end of light range? */
      	if(raypack[x-1][y-1].ritlit & 1) {
      	  raypack[x-1][y-1].botlit |= 1;
      	  raypack[x-1][y-2].lrl = 1;
      	  raypack[x-1][y-2].mrl = 1;
	}
      	else raypack[x-1][y-1].botlit &= ~1;
        if(raypack[x-1][y].botlit) {
          raypack[x-2][y-1].ritlit |= 1;
          raypack[x-2][y-1].mrl = 1;
          raypack[x-2][y-1].lrl = 1;
        }
        else raypack[x-2][y-1].ritlit &= ~1;
      }
      if(raypack[x-1][y-1].ritego) {
      	raypack[x-1][y-1].botego = 1;
      	raypack[x-1][y-2].mre = 1;
      	raypack[x-1][y-2].lre = 1;
      }
      else raypack[x-1][y-1].botego = 0;
      if(raypack[x-1][y].botego) {
      	raypack[x-2][y-1].ritego = 1;
      	raypack[x-2][y-1].mre = 1;
      	raypack[x-2][y-1].lre = 1;
      }
      else raypack[x-2][y-1].ritego = 0;
    }
    if(raypack[x-1][y].botego && raypack[x-1][y].botlit) {
      if (!raypack[x-1][y-1].drawn) DrawUnconcealableSquare(room,x-1,y-1);
      if (HFLAG(room, x-1, y-1)) HSET(x-2, y-1);
    }
    else {
      if (raypack[x-1][y-1].drawn) EraseSquare(room,x-1,y-1);
      if (HFLAG(room, x-1, y-1)) HSET(x-2, y-1);
    }
    if(raypack[x-1][y-1].ritego && raypack[x-1][y-1].ritlit) {
      if (!raypack[x-1][y-1].drawn) DrawUnconcealableSquare(room,x-1,y-1);
      if (VFLAG(room, x-1, y-1)) VSET(x-1, y-2);
    }
    else {
      if (raypack[x-1][y-1].drawn) EraseSquare(room,x-1,y-1);
      if (VFLAG(room, x-1, y-1)) VSET(x-1, y-2);
    }
  }
  
  /* ==================== Handle Other Squares =========================== */
 
  /* ------ handle them in boxes radiating outward from the ego ---------- */
 
  for(i=2; i<19; i++) {
    /* ------------- rook move squares first ----------------------------- */

    /* top */
    if (i+y < ROOM_HEIGHT) { /* make sure this is not off screen */
      if(IsTransparent(room, x, y+i)) { /* can rays continue ? */
        if(raypack[x][y+i].botlit & 1) { /* did the light make it this far? */
          raypack[x][y+i].ritlit |= 1;
          raypack[x-1][y+i].ritlit |=1;
          raypack[x+1][y+i].mrl = 0;
          raypack[x-1][y+i].mrl = 0;
D 3
          if (light > i+i) { /* end of light range? */
E 3
I 3
          if (light > i+1) { /* end of light range? */
E 3
            raypack[x][y+i+1].botlit |= 1;
	  }
          else {
            raypack[x][y+i+1].botlit &= ~1;
	  }
	}
        else {
          raypack[x][y+i].ritlit &= ~1;
          raypack[x-1][y+i].ritlit &= ~1;
          raypack[x][y+i+1].botlit &= ~1;
	}
	if(raypack[x][y+i].botego) {
	  raypack[x][y+i].ritego |= 1;
          raypack[x-1][y+i].ritego |= 1;
          raypack[x+1][y+i].mre = 0;
          raypack[x-1][y+i].mre = 0;
	  raypack[x][y+i+1].botego |= 1;
	}
	else {
	  raypack[x][y+i].ritego &= ~1;
          raypack[x-1][y+i].ritego &= ~1;
	  raypack[x][y+i+1].botego &= ~1;
	}
      }
      else {
        raypack[x][y+i].ritlit &= ~1;
        raypack[x-1][y+i].ritlit &= ~1;
        raypack[x][y+i+1].botlit &= ~1;
        raypack[x][y+i].ritego &= ~1;
        raypack[x-1][y+i].ritego &= ~1;
        raypack[x][y+i+1].botego &= ~1;
      }

      if(raypack[x][y+i].botlit && raypack[x][y+i].botego) {
        if(!raypack[x][y+i].drawn) DrawUnconcealableSquare(room, x, y+i);
        if(HFLAG(room, x, y+i)) {
          HSET(x+1, y+i);
          HSET(x-1, y+i);
	}
      }
      else {
        if(raypack[x][y+i].drawn) EraseSquare(room, x, y+i);
        if(!HFLAG(room, x, y+i)) {
          UNHSET(x+1, y+i);
          UNHSET(x-1, y+i);
	}
      }
    }

    /* bottom */
    if (y - i > -1) { /* make sure this is not off screen */
      if(IsTransparent(room, x, y-i)) { /* can rays continue ? */
        if(raypack[x][y-i+1].botlit & 1) { /* did the light make it this far? */
          raypack[x][y-i].ritlit |= 1;
          raypack[x-1][y-i].ritlit |=1;
          raypack[x+1][y-i].mrl = 0;
          raypack[x-1][y-i].mrl = 0;
D 3
          if (light > i+i) { /* end of light range? */
E 3
I 3
          if (light > i+1) { /* end of light range? */
E 3
            raypack[x][y-i].botlit |= 1;
	  }
          else {
            raypack[x][y-i].botlit &= ~1;
	  }
	}
        else {
          raypack[x][y-i].ritlit &= ~1;
          raypack[x-1][y-i].ritlit &= ~1;
          raypack[x][y-i].botlit &= ~1;
	}
	if(raypack[x][y-i+1].botego) {
	  raypack[x][y-i].ritego |= 1;
          raypack[x-1][y-i].ritego |= 1;
          raypack[x+1][y-i].mre = 0;
          raypack[x-1][y-i].mre = 0;
	  raypack[x][y-i].botego |= 1;
	}
	else {
	  raypack[x][y-i].ritego &= ~1;
          raypack[x-1][y-i].ritego &= ~1;
	  raypack[x][y-i].botego &= ~1;
	}
      }
      else {
        raypack[x][y-i].ritlit &= ~1;
        raypack[x-1][y-i].ritlit &= ~1;
        raypack[x][y-i].botlit &= ~1;
        raypack[x][y-i].ritego &= ~1;
        raypack[x-1][y-i].ritego &= ~1;
        raypack[x][y-i].botego &= ~1;
      }
      
      if(raypack[x][y-i+1].botlit && raypack[x][y-i+1].botego) {
        if(!raypack[x][y-i].drawn) DrawUnconcealableSquare(room, x, y-i+1);
        if(HFLAG(room, x, y-i+1)) {
          HSET(x+1, y-i+1);
          HSET(x-1, y-i+1);
	}
      }
      else {
        if(raypack[x][y-i].drawn) EraseSquare(room, x, y-i+1);
        if(!HFLAG(room, x, y-i+1)) {
          UNHSET(x+1, y-i+1);
          UNHSET(x-1, y-i+1);
	}
      }
    }

    /* right */
    if (x+i < ROOM_WIDTH) { /* make sure this is not off screen */
      if(IsTransparent(room, x+i, y)) { /* can rays continue ? */
        if(raypack[x+i-1][y].ritlit & 1) { /* did the light make it this far? */
          raypack[x+i][y+1].botlit |= 1;
          raypack[x+i][y].botlit |=1;
          raypack[x+i][y+1].mrl = 0;
          raypack[x+i][y].mrl = 0;
D 3
          if (light > i+i) { /* end of light range? */
E 3
I 3
          if (light > i+1) { /* end of light range? */
E 3
            raypack[x+i][y].ritlit |= 1;
	  }
          else {
            raypack[x+i][y].ritlit &= ~1;
	  }
	}
        else {
          raypack[x+i][y+1].botlit &= ~1;
          raypack[x+i][y].botlit &= ~1;
          raypack[x+i][y+1].ritlit &= ~1;
	}
	if(raypack[x+i-1][y].ritego) {
	  raypack[x+i][y+1].botego |= 1;
          raypack[x+i][y].botego |= 1;
          raypack[x+i][y+1].mre = 0;
          raypack[x+i][y].mre = 0;
	  raypack[x+i][y].ritego |= 1;
	}
	else {
	  raypack[x+i][y+1].botego &= ~1;
          raypack[x+i][y].botego &= ~1;
	  raypack[x+i][y].ritego &= ~1;
	}
      }
      else {
        raypack[x+i][y+1].botlit &= ~1;
        raypack[x+i][y].botlit &= ~1;
        raypack[x+i][y].ritlit &= ~1;
        raypack[x+i][y+1].botego &= ~1;
        raypack[x+i][y].botego &= ~1;
        raypack[x+i][y].ritego &= ~1;
      }

      if(raypack[x+i-1][y].ritlit && raypack[x+i-1][y].ritego) {
        if(!raypack[x+i][y].drawn) DrawUnconcealableSquare(room, x+i, y);
        if(VFLAG(room, x+i, y)) {
          VSET(x+i, y+1);
          VSET(x+i, y-1);
	}
      }
      else {
        if(raypack[x+i][y].drawn) EraseSquare(room, x+i, y);
        if(!VFLAG(room, x+i, y)) {
          UNVSET(x+i, y+1);
          UNVSET(x+i, y-1);
	}
      }
    }

    /* left */
    if (x-i > -1) { /* make sure this is not off screen */
      if(IsTransparent(room, x-i, y)) { /* can rays continue ? */
        if(raypack[x-i][y].ritlit & 1) { /* did the light make it this far? */
          raypack[x-i][y+1].botlit |= 1;
          raypack[x-i][y].botlit |=1;
          raypack[x-i][y+1].mrl = 0;
          raypack[x-i][y].mrl = 0;
D 3
          if (light > i+i) { /* end of light range? */
E 3
I 3
          if (light > i+1) { /* end of light range? */
E 3
            raypack[x-i-1][y].ritlit |= 1;
	  }
          else {
            raypack[x-i-1][y].ritlit &= ~1;
	  }
	}
        else {
          raypack[x-i][y+1].botlit &= ~1;
          raypack[x-i][y].botlit &= ~1;
          raypack[x-i-1][y+1].ritlit &= ~1;
	}
	if(raypack[x-i][y].ritego) {
	  raypack[x-i][y+1].botego |= 1;
          raypack[x-i][y].botego |= 1;
          raypack[x-i][y+1].mre = 0;
          raypack[x-i][y].mre = 0;
	  raypack[x-i-1][y].ritego |= 1;
	}
	else {
	  raypack[x-i][y+1].botego &= ~1;
          raypack[x-i][y].botego &= ~1;
	  raypack[x-i-1][y].ritego &= ~1;
	}
      }
      else {
        raypack[x-i][y+1].botlit &= ~1;
        raypack[x-i][y].botlit &= ~1;
        raypack[x-i-1][y+1].ritlit &= ~1;
        raypack[x-i][y+1].botego &= ~1;
        raypack[x-i][y].botego &= ~1;
        raypack[x-i-1][y+1].ritego &= ~1;
      }

      if(raypack[x-i][y].ritlit && raypack[x-i][y].ritego) {
        if(!raypack[x-i][y].drawn) DrawUnconcealableSquare(room, x-i, y);
        if(VFLAG(room, x-i, y)) {
          VSET(x-i, y+1);
          VSET(x-i, y-1);
	}
      }
      else {
        if(raypack[x-i][y].drawn) EraseSquare(room, x-i, y);
        if(!VFLAG(room, x-i, y)) {
          UNVSET(x-i, y+1);
          UNVSET(x-i, y-1);
	}
      }
    }

    /* next non-cardinals */
    for(j=1; j<i; j++) {
      /* set rulers */
      mr = rulepack[i][j].mr;
      lr = rulepack[i][j].lr;

      /* up then right */
D 3
      if(y+i < ROOM_HEIGHT || x+j < ROOM_WIDTH) {
E 3
I 3
      if(y+i < ROOM_HEIGHT && x+j < ROOM_WIDTH) {
E 3
        if(IsTransparent(room, x+j, y+i)) {
          if(raypack[x+j-1][y+i].ritlit & 1) {
	    if (light > i) {
	      raypack[x+j][y+i+1].botlit |= 1;
	      raypack[x+j][y+i+1].lrl = raypack[x+j][y+i].lrl + 1;
	    }
          }
          if(raypack[x+j][y+i].botlit & 1) {
            if (raypack[x+j][y+i].lrl >= lr) {
	      raypack[x+j][y+i].ritlit |= 1;
	      raypack[x+j+1][y+i].mrl = 0;
	    }
	    else {
	      raypack[x+j][y+i].ritlit &= ~1;
	    }
	    if((light > i) && raypack[x+j][y+i].mrl <= mr) {
	      raypack[x+j][y+i+1].botlit |= 1;
	      raypack[x+j][y+i+1].lrl = raypack[x+j][y+i].lrl + 1;
	    }
	    else {
	      raypack[x+j][y+i+1].botlit &= ~1;
	    }
	  }
	  else {
	    raypack[x+j][y+i].ritlit &= ~1;
	    raypack[x+j][y+i+1].botlit &= ~1;
	  }
  
  
          if(raypack[x+j-1][y+i].ritego & 1) {
	    if (light > i) {
	      raypack[x+j][y+i+1].botego |= 1;
	      raypack[x+j][y+i+1].lre = raypack[x+j][y+i].lre + 1;
	    }
          }
          if(raypack[x+j][y+i].botego & 1) {
            if (raypack[x+j][y+i].lre >= lr) {
	      raypack[x+j][y+i].ritego |= 1;
	      raypack[x+j+1][y+i].mre = 0;
	    }
	    else {
	      raypack[x+j][y+i].ritego &= ~1;
	    }
	    if((light > i) && raypack[x+j][y+i].mre <= mr) {
	      raypack[x+j][y+i+1].botego |= 1;
	      raypack[x+j][y+i+1].lre = raypack[x+j][y+i].lre + 1;
	    }
	    else {
	      raypack[x+j][y+i+1].botego &= ~1;
	    }
	  }
	  else {
	    raypack[x+j][y+i].ritego &= ~1;
	    raypack[x+j][y+i+1].botego &= ~1;
	  }
D 3
        }
E 3
I 3
	}
E 3
        else {
            raypack[x+j][y+i].ritlit &= ~1;
            raypack[x+j][y+i+1].botlit &= ~1;
            raypack[x+j][y+i].ritego &= ~1;
            raypack[x+j][y+i+1].botego &= ~1;
D 3
        }
E 3
I 3
	}
      
E 3
  
        if ((raypack[x+j][y+i].botlit && raypack[x+j][y+i].botego) ||
            (raypack[x+j-1][y+i].ritlit && (raypack[x+j-1][y+i].ritego))) {
          if (!raypack[x+j][y+i].drawn) DrawConcealableSquare(room, x+j, y+i);
          if (HFLAG(room, x+j,y+i)) HSET(x+j+1,y+i);
        }
	else {
          if (raypack[x+j][y+i].drawn) EraseSquare(room, x+j, y+i);
          if (!HFLAG(room, x+j,y+i)) UNHSET(x+j+1,y+i);
	}
      }
	    
      /* up then left */
D 3
      if(y+i < ROOM_HEIGHT || x-j > -1) {
E 3
I 3
      if(y+i < ROOM_HEIGHT && x-j > -1) {
E 3
        if(IsTransparent(room, x-j, y+i)) {
          if(raypack[x-j][y+i].ritlit & 1) {
	    if (light > i) {
	      raypack[x-j][y+i+1].botlit |= 1;
	      raypack[x-j][y+i+1].lrl = raypack[x+j][y+i].lrl + 1;
	    }
          }
          if(raypack[x-j][y+i].botlit & 1) {
            if (raypack[x-j][y+i].lrl >= lr) {
	      raypack[x-j-1][y+i].ritlit |= 1;
	      raypack[x-j-1][y+i].mrl = 0;
	    }
	    else {
	      raypack[x-j-1][y+i].ritlit &= ~1;
	    }
	    if((light > i) && raypack[x-j][y+i].mrl <= mr) {
	      raypack[x-j][y+i+1].botlit |= 1;
	      raypack[x-j][y+i+1].lrl = raypack[x+j][y+i].lrl + 1;
	    }
	    else {
	      raypack[x-j][y+i+1].botlit &= ~1;
	    }
	  }
	  else {
	    raypack[x-j-1][y+i].ritlit &= ~1;
	    raypack[x-j][y+i+1].botlit &= ~1;
	  }
  
  
          if(raypack[x-j][y+i].ritego & 1) {
	    if (light > i) {
	      raypack[x-j][y+i+1].botego |= 1;
	      raypack[x-j][y+i+1].lre = raypack[x-j][y+i].lre + 1;
	    }
          }
          if(raypack[x-j][y+i].botego & 1) {
            if (raypack[x-j][y+i].lre >= lr) {
	      raypack[x-j-1][y+i].ritego |= 1;
	      raypack[x-j-1][y+i].mre = 0;
	    }
	    else {
	      raypack[x-j-1][y+i].ritego &= ~1;
	    }
	    if((light > i) && raypack[x-j][y+i].mre <= mr) {
	      raypack[x-j][y+i+1].botego |= 1;
	      raypack[x-j][y+i+1].lre = raypack[x-j][y+i].lre + 1;
	    }
	    else {
	      raypack[x-j][y+i+1].botego &= ~1;
	    }
	  }
	  else {
	    raypack[x-j][y+i].ritego &= ~1;
	    raypack[x-j][y+i+1].botego &= ~1;
	  }
        }
        else {
            raypack[x-j-1][y+i].ritlit &= ~1;
            raypack[x-j][y+i+1].botlit &= ~1;
            raypack[x-j][y+i].ritego &= ~1;
            raypack[x-j][y+i+1].botego &= ~1;
        }
        if ((raypack[x-j][y+i].botlit && raypack[x-j][y+i].botego) ||
            (raypack[x-j][y+i].ritlit && (raypack[x-j][y+i].ritego))) {
          if (!raypack[x-j][y+i].drawn) DrawConcealableSquare(room, x-j, y+i);
          if (HFLAG(room, x-j,y+i)) HSET(x-j-1,y+i);
        }
	else {
          if (raypack[x-j][y+i].drawn) EraseSquare(room, x-j, y+i);
          if (!HFLAG(room, x-j,y+i)) UNHSET(x-j-1,y+i);
	}
      }

      /* left then up */
      if(x-i > -1 && y+j < ROOM_HEIGHT) {
        if(IsTransparent(room, x-i, y+j)) {
          if(raypack[x-i][y+j].ritlit & 1) {
            if (raypack[x-i][y+j].lrl >= lr) {
	      raypack[x-i][y+j+1].botlit |= 1;
	      raypack[x-i][y+j+1].mrl = 0;
	    }
	    else {
	      raypack[x-i][y+j+1].botlit &= ~1;
	    }
	    if((light > i) && raypack[x-i][y+j].mrl <= mr) {
	      raypack[x-i-1][y+j].ritlit |= 1;
	      raypack[x-i-1][y+j].lrl = raypack[x-i][y+j].lrl + 1;
	    }
	    else {
	      raypack[x-i-1][y+j].ritlit &= ~1;
	    }
	  }
	  else {
	    raypack[x-i][y+j+1].botlit &= ~1;
	    raypack[x-i-1][y+j].ritlit &= ~1;
	  }
          if(raypack[x-i][y+j].botlit & 1) {
	    if (light > i) {
	      raypack[x-i-1][y+j].ritlit |= 1;
	      raypack[x-i-1][y+j].lrl = raypack[x-i][y+j].lrl + 1;
	    }
          }
  
          if(raypack[x-i][y+j].ritego & 1) {
            if (raypack[x-i][y+j].lre >= lr) {
	      raypack[x-i][y+j+1].botego |= 1;
	      raypack[x-i][y+j+1].mre = 0;
	    }
	    else {
	      raypack[x-i][y+j+1].botego &= ~1;
	    }
	    if((light > i) && raypack[x-i][y+j].mre <= mr) {
	      raypack[x-i-1][y+j].ritego |= 1;
	      raypack[x-i-1][y+j].lre = raypack[x+i][y+j].lre + 1;
	    }
	    else {
	      raypack[x-i-1][y+j].ritego &= ~1;
	    }
	  }
	  else {
	    raypack[x-i][y+j+1].botego &= ~1;
	    raypack[x-i-1][y+j].ritego &= ~1;
	  }
          if(raypack[x-i][y+j].botego & 1) {
	    if (light > i) {
	      raypack[x-i-1][y+j].ritego |= 1;
	      raypack[x-i-1][y+j].lre = raypack[x+i][y+j].lre + 1;
	    }
          }
        }
        else {
            raypack[x-i][y+j+1].botlit &= ~1;
            raypack[x-i-1][y+j].ritlit &= ~1;
            raypack[x-i][y+j+1].botego &= ~1;
            raypack[x-i-1][y+j].ritego &= ~1;
        }
        if ((raypack[x-i][y+j].botlit && raypack[x-i][y+j].botego) ||
            (raypack[x-i][y+j].ritlit && (raypack[x-i][y+j].ritego))) {
          if (!raypack[x-i][y+j].drawn) DrawConcealableSquare(room, x-i, y+j);
          if (VFLAG(room, x-i,y+j)) VSET(x-i,y+j+1);
        }
	else {
          if (raypack[x-i][y+j].drawn) EraseSquare(room, x-i, y+j);
          if (!VFLAG(room, x-i,y+j)) UNVSET(x-i,y+j+1);
	}
      }
	    
  
      /* right then up */
      if(x+i < ROOM_WIDTH && y+j < ROOM_HEIGHT) {
        if(IsTransparent(room, x+i, y+j)) {
          if(raypack[x+i-1][y+j].ritlit & 1) {
            if (raypack[x+i][y+j].lrl >= lr) {
	      raypack[x+i][y+j+1].botlit |= 1;
	      raypack[x+i][y+j+1].mrl = 0;
	    }
	    else {
	      raypack[x+i][y+j+1].botlit &= ~1;
	    }
	    if((light > i) && raypack[x+i][y+j].mrl <= mr) {
	      raypack[x+i][y+j].ritlit |= 1;
	      raypack[x+i+1][y+j].lrl = raypack[x+i][y+j].lrl + 1;
	    }
	    else {
	      raypack[x+i][y+j].ritlit &= ~1;
	    }
	  }
	  else {
	    raypack[x+i][y+j+1].botlit &= ~1;
	    raypack[x+i][y+j].ritlit &= ~1;
	  }
          if(raypack[x+i][y+j].botlit & 1) {
	    if (light > i) {
	      raypack[x+i][y+j].ritlit |= 1;
	      raypack[x+i+1][y+j].lrl = raypack[x+i][y+j].lrl + 1;
	    }
          }
  
          if(raypack[x+i-1][y+j].ritego & 1) {
            if (raypack[x+i][y+j].lre >= lr) {
	      raypack[x+i][y+j+1].botego |= 1;
	      raypack[x+i][y+j+1].mre = 0;
	    }
	    else {
	      raypack[x+i][y+j+1].botego &= ~1;
	    }
	    if((light > i) && raypack[x+i][y+j].mre <= mr) {
	      raypack[x+i][y+j].ritego |= 1;
	      raypack[x+i+1][y+j].lre = raypack[x+i][y+j].lre + 1;
	    }
	    else {
	      raypack[x+i][y+j].ritego &= ~1;
	    }
	  }
	  else {
	    raypack[x+i][y+j+1].botego &= ~1;
	    raypack[x+i][y+j].ritego &= ~1;
	  }
          if(raypack[x+i][y+j].botego & 1) {
	    if (light > i) {
	      raypack[x+i][y+j].ritego |= 1;
	      raypack[x+i+1][y+j].lre = raypack[x+i][y+j].lre + 1;
	    }
          }
        }
        else {
            raypack[x+i][y+j+1].botlit &= ~1;
            raypack[x+i][y+j].ritlit &= ~1;
            raypack[x+i][y+j+1].botego &= ~1;
            raypack[x+i][y+j].ritego &= ~1;
        }
        if ((raypack[x+i][y+j].botlit && raypack[x+i][y+j].botego) ||
            (raypack[x+i-1][y+j].ritlit && (raypack[x+i-1][y+j].ritego))) {
          if (!raypack[x+i][y+j].drawn) DrawConcealableSquare(room, x+i, y+j);
          if (VFLAG(room, x+i,y+i)) VSET(x+i,y+j+1);
        }
	else {
          if (raypack[x+i][y+j].drawn) EraseSquare(room, x+i, y+j);
          if (!VFLAG(room, x+i,y+i)) UNVSET(x+i,y+j+1);
	}
      }
  
      /* down then right */
      if(y-i > -1 && x+j < ROOM_WIDTH) {
        if(IsTransparent(room, x+j, y-i)) {
          if(raypack[x+j-1][y-i].ritlit & 1) {
	    if (light > i) {
	      raypack[x+j][y-i].botlit |= 1;
	      raypack[x+j][y-i-1].lrl = raypack[x+j][y-i].lrl + 1;
	    }
          }
          if(raypack[x+j][y-i+1].botlit & 1) {
            if (raypack[x+j][y-i].lrl >= lr) {
	      raypack[x+j][y-i].ritlit |= 1;
	      raypack[x+j+1][y-i].mrl = 0;
	    }
	    else {
	      raypack[x+j][y-i].ritlit &= ~1;
	    }
	    if((light > i) && raypack[x+j][y-i].mrl <= mr) {
	      raypack[x+j][y-i].botlit |= 1;
	      raypack[x+j][y-i-1].lrl = raypack[x+j][y-i].lrl + 1;
	    }
	    else {
	      raypack[x+j][y-i-1].botlit &= ~1;
	    }
	  }
	  else {
	    raypack[x+j][y-i].ritlit &= ~1;
	    raypack[x+j][y-i].botlit &= ~1;
	  }
  
          if(raypack[x+j-1][y-i].ritego & 1) {
	    if (light > i) {
	      raypack[x+j][y-i].botego |= 1;
	      raypack[x+j][y-i-1].lre = raypack[x+j][y-i].lre + 1;
	    }
          }
          if(raypack[x+j][y-i+1].botego & 1) {
            if (raypack[x+j][y-i].lre >= lr) {
	      raypack[x+j][y-i].ritego |= 1;
	      raypack[x+j+1][y-i].mre = 0;
	    }
	    else {
	      raypack[x+j][y-i].ritego &= ~1;
	    }
	    if((light > i) && raypack[x+j][y-i].mre <= mr) {
	      raypack[x+j][y-i].botego |= 1;
	      raypack[x+j][y-i-1].lre = raypack[x+j][y-i].lre + 1;
	    }
	    else {
	      raypack[x+j][y-i-1].botego &= ~1;
	    }
	  }
	  else {
	    raypack[x+j][y-i].ritego &= ~1;
	    raypack[x+j][y-i].botego &= ~1;
	  }
        }
        else {
            raypack[x+j][y-i].ritlit &= ~1;
            raypack[x+j][y-i].botlit &= ~1;
            raypack[x+j][y-i].ritego &= ~1;
            raypack[x+j][y-i].botego &= ~1;
        }
  
        if ((raypack[x+j][y-i+1].botlit && raypack[x+j][y-i+1].botego) ||
            (raypack[x+j-1][y-i].ritlit && (raypack[x+j-1][y-i].ritego))) {
          if (!raypack[x+j][y-i].drawn) DrawConcealableSquare(room, x+j, y-i);
          if (HFLAG(room, x+j,y-i)) HSET(x+j+1,y-i);
        }
	else {
          if (raypack[x+j][y-i].drawn) EraseSquare(room, x+j, y-i);
          if (!HFLAG(room, x+j,y-i)) UNHSET(x+j+1,y-i);
	}
      }
      
      /* down then left */
D 3
      if(x-j < -1 && y-i < -1) {
E 3
I 3
      if(x-j > -1 && y-i > -1) {
E 3
        if(IsTransparent(room, x-j, y-i)) {
          if(raypack[x-j][y-i].ritlit & 1) {
	    if (light > i) {
	      raypack[x-j][y-i].botlit |= 1;
	      raypack[x-j][y-i+1].lrl = raypack[x+j][y-i].lrl + 1;
	    }
          }
          if(raypack[x-j][y-i+1].botlit & 1) {
            if (raypack[x-j][y-i].lrl >= lr) {
	      raypack[x-j-1][y-i].ritlit |= 1;
	      raypack[x-j-1][y-i].mrl = 0;
	    }
	    else {
	      raypack[x-j-1][y-i].ritlit &= ~1;
	    }
	    if((light > i) && raypack[x-j][y-i].mrl <= mr) {
	      raypack[x-j][y-i].botlit |= 1;
	      raypack[x-j][y-i-1].lrl = raypack[x+j][y-i].lrl + 1;
	    }
	    else {
	      raypack[x-j][y-i].botlit &= ~1;
	    }
	  }
	  else {
	    raypack[x-j-1][y-i].ritlit &= ~1;
	    raypack[x-j][y-i].botlit &= ~1;
	  }
  
          if(raypack[x-j][y-i].ritego & 1) {
	    if (light > i) {
	      raypack[x-j][y-i].botego |= 1;
	      raypack[x-j][y-i-1].lre = raypack[x-j][y-i].lre + 1;
	    }
          }
          if(raypack[x-j][y-i+1].botego & 1) {
            if (raypack[x-j][y-i].lre >= lr) {
	      raypack[x-j-1][y-i].ritego |= 1;
	      raypack[x-j-1][y-i].mre = 0;
	    }
	    else {
	      raypack[x-j-1][y-i].ritego &= ~1;
	    }
	    if((light > i) && raypack[x-j][y-i].mre <= mr) {
	      raypack[x-j][y-i].botego |= 1;
	      raypack[x-j][y-i-1].lre = raypack[x-j][y-i].lre + 1;
	    }
	    else {
	      raypack[x-j][y-i].botego &= ~1;
	    }
	  }
	  else {
	    raypack[x-j][y-i].ritego &= ~1;
	    raypack[x-j][y-i].botego &= ~1;
	  }
        }
        else {
            raypack[x-j-1][y-i].ritlit &= ~1;
            raypack[x-j][y-i].botlit &= ~1;
            raypack[x-j][y-i].ritego &= ~1;
            raypack[x-j][y-i].botego &= ~1;
        }
        if ((raypack[x-j][y-i+1].botlit && raypack[x-j][y-i+1].botego) ||
            (raypack[x-j][y-i].ritlit && (raypack[x-j][y-i].ritego))) {
          if (!raypack[x-j][y-i].drawn) DrawConcealableSquare(room, x-j, y-i);
          if (HFLAG(room, x-j,y-i)) HSET(x-j-1,y-i);
        }
	else {
          if (raypack[x-j][y-i].drawn) EraseSquare(room, x-j, y-i);
          if (!HFLAG(room, x-j,y-i)) UNHSET(x-j-1,y-i);
	}
      }

      /* left then down */
D 3
      if(x-i < -1 && y-j < -1) {
E 3
I 3
      if(x-i > -1 && y-j > -1) {
E 3
        if(IsTransparent(room, x-i, y-j)) {
          if(raypack[x-i][y-j].ritlit & 1) {
            if (raypack[x-i][y-j].lrl >= lr) {
	      raypack[x-i][y-j].botlit |= 1;
	      raypack[x-i][y-j-1].mrl = 0;
	    }
	    else {
	      raypack[x-i][y-j].botlit &= ~1;
	    }
	    if((light > i) && raypack[x-i][y-j].mrl <= mr) {
	      raypack[x-i-1][y-j].ritlit |= 1;
	      raypack[x-i-1][y-j].lrl = raypack[x-i][y-j].lrl + 1;
	    }
	    else {
	      raypack[x-i-1][y-j].ritlit &= ~1;
	    }
	  }
	  else {
	    raypack[x-i][y-j].botlit &= ~1;
	    raypack[x-i-1][y-j].ritlit &= ~1;
	  }
          if(raypack[x-i][y-j+1].botlit & 1) {
	    if (light > i) {
	      raypack[x-i-1][y-j].ritlit |= 1;
	      raypack[x-i-1][y-j].lrl = raypack[x-i][y-j].lrl + 1;
	    }
          }
  
          if(raypack[x-i][y-j].ritego & 1) {
            if (raypack[x-i][y-j].lre >= lr) {
	      raypack[x-i][y-j].botego |= 1;
	      raypack[x-i][y-j-1].mre = 0;
	    }
	    else {
	      raypack[x-i][y-j].botego &= ~1;
	    }
	    if((light > i) && raypack[x-i][y-j].mre <= mr) {
	      raypack[x-i-1][y-j].ritego |= 1;
	      raypack[x-i-1][y-j].lre = raypack[x+i][y-j].lre + 1;
	    }
	    else {
	      raypack[x-i-1][y-j].ritego &= ~1;
	    }
	  }
	  else {
	    raypack[x-i][y-j].botego &= ~1;
	    raypack[x-i-1][y-j].ritego &= ~1;
	  }
          if(raypack[x-i][y-j+1].botego & 1) {
	    if (light > i) {
	      raypack[x-i-1][y-j].ritego |= 1;
	      raypack[x-i-1][y-j].lre = raypack[x+i][y-j].lre + 1;
	    }
          }
        }
        else {
            raypack[x-i][y-j].botlit &= ~1;
            raypack[x-i-1][y-j].ritlit &= ~1;
            raypack[x-i][y-j].botego &= ~1;
            raypack[x-i-1][y-j].ritego &= ~1;
        }
        if ((raypack[x-i][y-j+1].botlit && raypack[x-i][y-j+1].botego) ||
            (raypack[x-i][y-j].ritlit && (raypack[x-i][y-j].ritego))) {
          if (!raypack[x-i][y-j].drawn) DrawConcealableSquare(room, x-i, y-j);
          if (VFLAG(room, x-i,y-j)) VSET(x-i,y-j-1);
        }
	else {
          if (raypack[x-i][y-j].drawn) EraseSquare(room, x-i, y-j);
          if (!VFLAG(room, x-i,y-j)) UNVSET(x-i,y-j-1);
	}
      }

      /* right then down */
D 3
      if(x+i > ROOM_WIDTH && y-j < -1) {
E 3
I 3
      if(x+i < ROOM_WIDTH && y-j > -1) {
E 3
        if(IsTransparent(room, x+i, y-j)) {
          if(raypack[x+i-1][y-j].ritlit & 1) {
            if (raypack[x+i][y-j].lrl >= lr) {
	      raypack[x+i][y-j].botlit |= 1;
	      raypack[x+i][y-j].mrl = 0;
	    }
	    else {
	      raypack[x+i][y-j].botlit &= ~1;
	    }
	    if((light > i) && raypack[x+i][y-j].mrl <= mr) {
	      raypack[x+i][y-j].ritlit |= 1;
	      raypack[x+i+1][y-j].lrl = raypack[x+i][y-j].lrl + 1;
	    }
	    else {
	      raypack[x+i][y-j].ritlit &= ~1;
	    }
	  }
	  else {
	    raypack[x+i][y-j].botlit &= ~1;
	    raypack[x+i][y-j].ritlit &= ~1;
	  }
          if(raypack[x+i][y-j].botlit & 1) {
	    if (light > i) {
	      raypack[x+i][y-j].ritlit |= 1;
	      raypack[x+i+1][y-j].lrl = raypack[x+i][y-j].lrl + 1;
	    }
          }
  
          if(raypack[x+i-1][y-j].ritego & 1) {
            if (raypack[x+i][y-j].lre >= lr) {
	      raypack[x+i][y-j].botego |= 1;
	      raypack[x+i][y-j-1].mre = 0;
	    }
	    else {
	      raypack[x+i][y-j].botego &= ~1;
	    }
	    if((light > i) && raypack[x+i][y-j].mre <= mr) {
	      raypack[x+i][y-j].ritego |= 1;
	      raypack[x+i+1][y-j].lre = raypack[x+i][y-j].lre + 1;
	    }
	    else {
	      raypack[x+i][y-j].ritego &= ~1;
	    }
	  }
	  else {
	    raypack[x+i][y-j].botego &= ~1;
	    raypack[x+i][y-j].ritego &= ~1;
	  }
          if(raypack[x+i][y-j+1].botego & 1) {
	    if (light > i) {
	      raypack[x+i][y-j].ritego |= 1;
	      raypack[x+i+1][y-j].lre = raypack[x+i][y-j].lre + 1;
	    }
          }
        }
        else {
            raypack[x+i][y-j].botlit &= ~1;
            raypack[x+i][y-j].ritlit &= ~1;
            raypack[x+i][y-j].botego &= ~1;
            raypack[x+i][y-j].ritego &= ~1;
        }
        if ((raypack[x+i][y-j].botlit && raypack[x+i][y-j].botego) ||
            (raypack[x+i-1][y-j].ritlit && (raypack[x+i-1][y-j].ritego))) {
          if (!raypack[x+i][y-j].drawn) DrawConcealableSquare(room, x+i, y-j);
          if (VFLAG(room, x+i,y-i)) VSET(x+i,y-j-1);
        }
	else {
          if (raypack[x+i][y-j].drawn) EraseSquare(room, x+i, y-j);
          if (!VFLAG(room, x+i,y-i)) UNVSET(x+i,y-j-1);
	}
      }
    }

    /* finally the diagonals */
    /* left-up */
    if(y+i < ROOM_HEIGHT && x-i > -1) { /* make sure space is on screen */
      if(IsTransparent(room, x-i, y+i)) { /* should rays continue? */
        if(light > i) { /* end of light range? */
          if(raypack[x-i][y+i].ritlit & 1) {
            raypack[x-i][y+i+1].botlit |= 1;
            raypack[x-i][y+i+1].lrl = 1;
            raypack[x-i][y+i+1].mrl = 1;
	  }
          else raypack[x-i][y+i+1].botlit &= ~1;
          if(raypack[x-i][y+i].botlit) {
            raypack[x-i-1][y+i].ritlit |= 1;
            raypack[x-i-1][y+i].mrl = 1;
            raypack[x-i-1][y+i].lrl = 1;
  	  }
          else raypack[x-i-1][y+i].ritlit &= ~1;
        }
        if(raypack[x-i][y+i].ritego) {
          raypack[x-i][y+i+1].botego = 1;
          raypack[x-i][y+i+1].mre = 1;
          raypack[x-i][y+i+1].lre = 1;
        }
        else raypack[x-i][y+i+1].botego = 0;
        if(raypack[x-i][y+i].botego) {
          raypack[x-i-1][y+i].ritego = 1;
          raypack[x-i-1][y+i].mre = 1;
          raypack[x-i-1][y+i].lre = 1;
        }
        else raypack[x-i-1][y+i].ritego = 0;
      }
      if(raypack[x-i][y+i].botego && raypack[x-i][y+i].botlit) {
        if (!raypack[x-i][y+i].drawn) DrawUnconcealableSquare(room,x-i,y+i);
        if (HFLAG(room, x-i, y+i)) HSET(x-i-1, y+i);
      }
      else {
        if (raypack[x-i][y+i].drawn) EraseSquare(room,x-i,y+i);
        if (!HFLAG(room, x-i, y+i)) UNHSET(x-i-1, y+i);
      }
      if(raypack[x-i][y+i].ritego && raypack[x-i][y+i].ritlit) {
        if (!raypack[x-i][y+i].drawn) DrawUnconcealableSquare(room,x-i,y+i);
        if (VFLAG(room, x-i, y+i)) VSET(x-i, y+i+1);
      }
      else {
        if (raypack[x-i][y+i].drawn) EraseSquare(room,x-i,y+i);
        if (!VFLAG(room, x-i, y+i)) UNVSET(x-i, y+i+1);
      }
    }

    /* right up */
    if(y+i < ROOM_HEIGHT && x+i < ROOM_WIDTH) { /* make sure space is on screen */
      if(IsTransparent(room, x+i, y+i)) { /* should rays continue? */
        if(light > i) { /* end of light range? */
          if(raypack[x][y+i].ritlit & 1) {
            raypack[x+i][y+i+1].botlit |= 1;
            raypack[x+i][y+i+1].lrl = 1;
            raypack[x+i][y+i+1].mrl = 1;
  	}
          else raypack[x+i][y+i+1].botlit &= ~1;
          if(raypack[x+i][y+i].botlit) {
            raypack[x+i][y+i].ritlit |= 1;
            raypack[x+i+1][y+i].mrl = 1;
            raypack[x+i+1][y+i].lrl = 1;
  	}
          else raypack[x+i][y+i].ritlit &= ~1;
        }
        if(raypack[x][y+i].ritego) {
          raypack[x+i][y+i+1].botego = 1;
          raypack[x+i][y+i+1].mre = 1;
          raypack[x+i][y+i+1].lre = 1;
        }
        else raypack[x+i][y+i+1].botego = 0;
        if(raypack[x+i][y+i].botego) {
          raypack[x+i][y+i].ritego = 1;
          raypack[x+i+1][y+i].mre = 1;
          raypack[x+i+1][y+i].lre = 1;
        }
        else raypack[x+i][y+i].ritego = 0;
      }
      if(raypack[x+i][y+i].botego && raypack[x+i][y+i].botlit) {
        if (!raypack[x+i][y+i].drawn) DrawUnconcealableSquare(room,x+i,y+i);
        if (HFLAG(room, x+i, y+i)) HSET(x+i+1, y+i);
      }
      else {
        if (raypack[x+i][y+i].drawn) EraseSquare(room,x+i,y+i);
        if (!HFLAG(room, x+i, y+i)) UNHSET(x+i+1, y+i);
      }
      if(raypack[x][y+i].ritego && raypack[x][y+i].ritlit) {
        if (!raypack[x+i][y+i].drawn) DrawUnconcealableSquare(room,x+i,y+i);
        if (VFLAG(room, x+i, y+i)) VSET(x+i, y+i+1);
      }
      else {
        if (raypack[x+i][y+i].drawn) EraseSquare(room,x+i,y+i);
        if (!VFLAG(room, x+i, y+i)) UNVSET(x+i, y+i+1);
      }
    }

    /*right down */
    if(y-i > -1 && x+i < ROOM_WIDTH) { /* make sure space is on screen */
      if(IsTransparent(room, x+i, y-i)) { /* should rays continue? */
        if(light > i) { /* end of light range? */
          if(raypack[x][y-i].ritlit & 1) {
            raypack[x+i][y-i].botlit |= 1;
            raypack[x+i][y-2].lrl = 1;
            raypack[x+i][y-2].mrl = 1;
  	}
          else raypack[x+i][y-i].botlit &= ~1;
          if(raypack[x+i][y].botlit) {
            raypack[x+i][y-i].ritlit |= 1;
            raypack[x+i+1][y-i].mrl = 1;
            raypack[x+i+1][y-i].lrl = 1;
  	}
          else raypack[x+i][y-i].ritlit &= ~1;
        }
        if(raypack[x][y-i].ritego) {
          raypack[x+i][y-i].botego = 1;
          raypack[x+i][y-2].mre = 1;
          raypack[x+i][y-2].lre = 1;
        }
        else raypack[x+i][y-i].botego = 0;
        if(raypack[x+i][y].botego) {
          raypack[x+i][y-i].ritego = 1;
          raypack[x+i+1][y-i].mre = 1;
          raypack[x+i+1][y-i].lre = 1;
        }
        else raypack[x+i][y-i].ritego = 0;
      }
      if(raypack[x+i][y].botego && raypack[x+i][y].botlit) {
       if (!raypack[x+i][y-i].drawn) DrawUnconcealableSquare(room,x+i,y-i);
       if (HFLAG(room, x+i, y-i)) HSET(x+i+1, y-i);
      }
      else {
       if (raypack[x+i][y-i].drawn) EraseSquare(room,x+i,y-i);
       if (!HFLAG(room, x+i, y-i)) UNHSET(x+i+1, y-i);
      }
      if(raypack[x][y-i].ritego && raypack[x][y-i].ritlit) {
        if (!raypack[x+i][y-i].drawn) DrawUnconcealableSquare(room,x+i,y-i);
        if (VFLAG(room, x+i, y-i)) VSET(x+i, y-2);
      }
      else {
        if (raypack[x+i][y-i].drawn) EraseSquare(room,x+i,y-i);
        if (!VFLAG(room, x+i, y-i)) UNVSET(x+i, y-2);
      }
    }
  
    /* left down */
    if(y-i > -1 && x-i > -1) { /* make sure space is on screen */
      if(IsTransparent(room, x-i, y-i)) { /* should rays continue? */
        if(light > i) { /* end of light range? */
          if(raypack[x-i][y-i].ritlit & 1) {
            raypack[x-i][y-i].botlit |= 1;
            raypack[x-i][y-2].lrl = 1;
            raypack[x-i][y-2].mrl = 1;
	  }
          else raypack[x-i][y-i].botlit &= ~1;
          if(raypack[x-i][y].botlit) {
            raypack[x-2][y-i].ritlit |= 1;
            raypack[x-2][y-i].mrl = 1;
            raypack[x-2][y-i].lrl = 1;
	  }
          else raypack[x-2][y-i].ritlit &= ~1;
        }
        if(raypack[x-i][y-i].ritego) {
          raypack[x-i][y-i].botego = 1;
          raypack[x-i][y-2].mre = 1;
          raypack[x-i][y-2].lre = 1;
        }
        else raypack[x-i][y-i].botego = 0;
        if(raypack[x-i][y].botego) {
          raypack[x-2][y-i].ritego = 1;
          raypack[x-2][y-i].mre = 1;
          raypack[x-2][y-i].lre = 1;
        }
        else raypack[x-2][y-i].ritego = 0;
      }
      if(raypack[x-i][y].botego && raypack[x-i][y].botlit) {
        if (!raypack[x-i][y-i].drawn) DrawUnconcealableSquare(room,x-i,y-i);
        if (HFLAG(room, x-i, y-i)) HSET(x-2, y-i);
      }
      else {
        if (raypack[x-i][y-i].drawn) EraseSquare(room,x-i,y-i);
        if (!HFLAG(room, x-i, y-i)) UNHSET(x-2, y-i);
      }
      if(raypack[x-i][y-i].ritego && raypack[x-i][y-i].ritlit) {
        if (!raypack[x-i][y-i].drawn) DrawUnconcealableSquare(room,x-i,y-i);
        if (VFLAG(room, x-i, y-i)) VSET(x-i, y-2);
      }
      else {
        if (raypack[x-i][y-i].drawn) EraseSquare(room,x-i,y-i);
        if (!VFLAG(room, x-i, y-i)) UNVSET(x-i, y-2);
      }
    }
  }
}

E 1
