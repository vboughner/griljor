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
#include "queued.h"
#include "ray.h"

/* ================================================================== */

/* This is the sighting routine for unlit rooms.  Given the location
 * (room, x, y) of the Ego and the radius of his/her light source, it
 * will determine which squares need to be drawn and/or undrawn, and
 * drawn/undraw them.  It further updates the lighting/sighting arrays
 * for use by other ray procedures (i. e. DrawOthersLightRays) */

void DrawEgoRaysWithoutLight(room, x, y)
{  int i, j, mr, lr;
 
  /* ================== Handle Own Square ========================= */
 
  /* the player will always see his own square */
  if (!raypack[x][y].drawn) DrawOwnSquare(room, x, y);

  /* the player always casts ego rays */
  raypack[x][y].botego = 1;
  raypack[x][y].ritego = 1;
  raypack[x-1][y].ritego = 1;
  raypack[x][y+1].botego = 1;
 
  /* =================== Handle Adjacent Squares ======================== */
 
  /* ---------------- vertical and horizantal first --------------------- */
 
  /* top */
  if(y+1 < ROOM_HEIGHT) { /* make sure this is on screen */
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
 
    if(raypack[x][y+1].botego) {
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
 
    if(raypack[x][y].botego) {
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
    if((raypack[x-1][y].ritego & 1) && /* did the ego rays make it this far? */
       IsTransparent(room, x-1, y)) { /* can it continue? */
      raypack[x-1][y].botego = 1;
      raypack[x-1][y+1].botego = 1;
      raypack[x-2][y].ritego = 1;
    }
    else {
      raypack[x-1][y].botego = 0;
      raypack[x-1][y+1].botego = 0;
      raypack[x-2][y].ritego = 0;
    }
 
    if(raypack[x-2][y].ritego) {
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
  if(x+1 < ROOM_WIDTH) { /* make sure this is on screen */
    if(raypack[x][y].ritego && /* did the ego rays make it this far? */
       IsTransparent(room, x+1, y)) { /* can it continue? */
      raypack[x+1][y].botego = 1;
      raypack[x+1][y+1].botego = 1;
      raypack[x+1][y].ritego = 1;
    }
    else {
      raypack[x+1][y].botego = 0;
      raypack[x+1][y+1].botego = 0;
      raypack[x+1][y].ritego = 0;
    }
 
    if(raypack[x][y].ritego) {
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
    if(raypack[x-1][y+1].botego) {
      if (!raypack[x-1][y+1].drawn) DrawUnconcealableSquare(room,x-1,y+1);
      if (HFLAG(room, x-1, y+1)) HSET(x-2, y+1);
    }
    else {
      if (raypack[x-1][y+1].drawn) EraseSquare(room,x-1,y+1);
      if (!HFLAG(room, x-1, y+1)) UNHSET(x-2, y+1);
    }
    if(raypack[x-1][y+1].ritego) {
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
    if(raypack[x+1][y+1].botego) {
      if (!raypack[x+1][y+1].drawn) DrawUnconcealableSquare(room,x+1,y+1);
      if (HFLAG(room, x+1, y+1)) HSET(x+2, y+1);
    }
    else {
      if (raypack[x+1][y+1].drawn) EraseSquare(room,x+1,y+1);
      if (HFLAG(room, x+1, y+1)) HSET(x+2, y+1);
    }
    if(raypack[x][y+1].ritego) {
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
    if(raypack[x+1][y].botego) {
      if (!raypack[x+1][y-1].drawn) DrawUnconcealableSquare(room,x+1,y-1);
      if (HFLAG(room, x+1, y-1)) HSET(x+2, y-1);
    }
    else {
      if (raypack[x+1][y-1].drawn) EraseSquare(room,x+1,y-1);
      if (HFLAG(room, x+1, y-1)) HSET(x+2, y-1);
    }
    if(raypack[x][y-1].ritego) {
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
    if(raypack[x-1][y].botego) {
      if (!raypack[x-1][y-1].drawn) DrawUnconcealableSquare(room,x-1,y-1);
      if (HFLAG(room, x-1, y-1)) HSET(x-2, y-1);
    }
    else {
      if (raypack[x-1][y-1].drawn) EraseSquare(room,x-1,y-1);
      if (HFLAG(room, x-1, y-1)) HSET(x-2, y-1);
    }
    if(raypack[x-1][y-1].ritego) {
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
        raypack[x][y+i].ritego &= ~1;
        raypack[x-1][y+i].ritego &= ~1;
        raypack[x][y+i+1].botego &= ~1;
      }

      if(raypack[x][y+i].botego) {
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
        raypack[x][y-i].ritego &= ~1;
        raypack[x-1][y-i].ritego &= ~1;
        raypack[x][y-i].botego &= ~1;
      }
      
      if(raypack[x][y-i+1].botego) {
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
        raypack[x+i][y+1].botego &= ~1;
        raypack[x+i][y].botego &= ~1;
        raypack[x+i][y].ritego &= ~1;
      }

      if(raypack[x+i-1][y].ritego) {
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
        raypack[x-i][y+1].botego &= ~1;
        raypack[x-i][y].botego &= ~1;
        raypack[x-i-1][y+1].ritego &= ~1;
      }

      if(raypack[x-i][y].ritego) {
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
      if(y+i < ROOM_HEIGHT && x+j < ROOM_WIDTH) {
        if(IsTransparent(room, x+j, y+i)) {
          if(raypack[x+j-1][y+i].ritego & 1) {
	      raypack[x+j][y+i+1].botego |= 1;
	      raypack[x+j][y+i+1].lre = raypack[x+j][y+i].lre + 1;
          }
          if(raypack[x+j][y+i].botego & 1) {
            if (raypack[x+j][y+i].lre >= lr) {
	      raypack[x+j][y+i].ritego |= 1;
	      raypack[x+j+1][y+i].mre = 0;
	    }
	    else {
	      raypack[x+j][y+i].ritego &= ~1;
	    }
	    if(raypack[x+j][y+i].mre <= mr) {
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
        }
        else {
            raypack[x+j][y+i].ritego &= ~1;
            raypack[x+j][y+i+1].botego &= ~1;
        }
  
        if ((raypack[x+j][y+i].botego) ||
            (raypack[x+j-1][y+i].ritego)) {
          if (!raypack[x+j][y+i].drawn) DrawConcealableSquare(room, x+j, y+i);
          if (HFLAG(room, x+j,y+i)) HSET(x+j+1,y+i);
        }
	else {
          if (raypack[x+j][y+i].drawn) EraseSquare(room, x+j, y+i);
          if (!HFLAG(room, x+j,y+i)) UNHSET(x+j+1,y+i);
	}
      }
	    
      /* up then left */
      if(y+i < ROOM_HEIGHT && x-j > -1) {
        if(IsTransparent(room, x-j, y+i)) {
          if(raypack[x-j][y+i].ritego & 1) {
	      raypack[x-j][y+i+1].botego |= 1;
	      raypack[x-j][y+i+1].lre = raypack[x-j][y+i].lre + 1;
          }
          if(raypack[x-j][y+i].botego & 1) {
            if (raypack[x-j][y+i].lre >= lr) {
	      raypack[x-j-1][y+i].ritego |= 1;
	      raypack[x-j-1][y+i].mre = 0;
	    }
	    else {
	      raypack[x-j-1][y+i].ritego &= ~1;
	    }
	    if(raypack[x-j][y+i].mre <= mr) {
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
            raypack[x-j][y+i].ritego &= ~1;
            raypack[x-j][y+i+1].botego &= ~1;
        }
        if ((raypack[x-j][y+i].botego) ||
            ((raypack[x-j][y+i].ritego))) {
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
          if(raypack[x-i][y+j].ritego & 1) {
            if (raypack[x-i][y+j].lre >= lr) {
	      raypack[x-i][y+j+1].botego |= 1;
	      raypack[x-i][y+j+1].mre = 0;
	    }
	    else {
	      raypack[x-i][y+j+1].botego &= ~1;
	    }
	    if(raypack[x-i][y+j].mre <= mr) {
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
	      raypack[x-i-1][y+j].ritego |= 1;
	      raypack[x-i-1][y+j].lre = raypack[x+i][y+j].lre + 1;
          }
        }
        else {
            raypack[x-i][y+j+1].botego &= ~1;
            raypack[x-i-1][y+j].ritego &= ~1;
        }
        if ((raypack[x-i][y+j].botego) ||
            ((raypack[x-i][y+j].ritego))) {
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
          if(raypack[x+i-1][y+j].ritego & 1) {
            if (raypack[x+i][y+j].lre >= lr) {
	      raypack[x+i][y+j+1].botego |= 1;
	      raypack[x+i][y+j+1].mre = 0;
	    }
	    else {
	      raypack[x+i][y+j+1].botego &= ~1;
	    }
	    if(raypack[x+i][y+j].mre <= mr) {
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
	      raypack[x+i][y+j].ritego |= 1;
	      raypack[x+i+1][y+j].lre = raypack[x+i][y+j].lre + 1;
          }
        }
        else {
            raypack[x+i][y+j+1].botego &= ~1;
            raypack[x+i][y+j].ritego &= ~1;
        }
        if ((raypack[x+i][y+j].botego) ||
            ((raypack[x+i-1][y+j].ritego))) {
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
          if(raypack[x+j-1][y-i].ritego & 1) {
	      raypack[x+j][y-i].botego |= 1;
	      raypack[x+j][y-i-1].lre = raypack[x+j][y-i].lre + 1;
          }
          if(raypack[x+j][y-i+1].botego & 1) {
            if (raypack[x+j][y-i].lre >= lr) {
	      raypack[x+j][y-i].ritego |= 1;
	      raypack[x+j+1][y-i].mre = 0;
	    }
	    else {
	      raypack[x+j][y-i].ritego &= ~1;
	    }
	    if(raypack[x+j][y-i].mre <= mr) {
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
            raypack[x+j][y-i].ritego &= ~1;
            raypack[x+j][y-i].botego &= ~1;
        }
  
        if ((raypack[x+j][y-i+1].botego) ||
            ((raypack[x+j-1][y-i].ritego))) {
          if (!raypack[x+j][y-i].drawn) DrawConcealableSquare(room, x+j, y-i);
          if (HFLAG(room, x+j,y-i)) HSET(x+j+1,y-i);
        }
	else {
          if (raypack[x+j][y-i].drawn) EraseSquare(room, x+j, y-i);
          if (!HFLAG(room, x+j,y-i)) UNHSET(x+j+1,y-i);
	}
      }
      
      /* down then left */
      if(x-j > -1 && y-i > -1) {
        if(IsTransparent(room, x-j, y-i)) {
          if(raypack[x-j][y-i].ritego & 1) {
	      raypack[x-j][y-i].botego |= 1;
	      raypack[x-j][y-i-1].lre = raypack[x-j][y-i].lre + 1;
          }
          if(raypack[x-j][y-i+1].botego & 1) {
            if (raypack[x-j][y-i].lre >= lr) {
	      raypack[x-j-1][y-i].ritego |= 1;
	      raypack[x-j-1][y-i].mre = 0;
	    }
	    else {
	      raypack[x-j-1][y-i].ritego &= ~1;
	    }
	    if(raypack[x-j][y-i].mre <= mr) {
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
            raypack[x-j][y-i].ritego &= ~1;
            raypack[x-j][y-i].botego &= ~1;
        }
        if ((raypack[x-j][y-i+1].botego) ||
            ((raypack[x-j][y-i].ritego))) {
          if (!raypack[x-j][y-i].drawn) DrawConcealableSquare(room, x-j, y-i);
          if (HFLAG(room, x-j,y-i)) HSET(x-j-1,y-i);
        }
	else {
          if (raypack[x-j][y-i].drawn) EraseSquare(room, x-j, y-i);
          if (!HFLAG(room, x-j,y-i)) UNHSET(x-j-1,y-i);
	}
      }

      /* left then down */
      if(x-i > -1 && y-j > -1) {
        if(IsTransparent(room, x-i, y-j)) {
          if(raypack[x-i][y-j].ritego & 1) {
            if (raypack[x-i][y-j].lre >= lr) {
	      raypack[x-i][y-j].botego |= 1;
	      raypack[x-i][y-j-1].mre = 0;
	    }
	    else {
	      raypack[x-i][y-j].botego &= ~1;
	    }
	    if(raypack[x-i][y-j].mre <= mr) {
	      raypack[x-i-1][y-j].ritego |= 1;
	      raypack[x-i-1][y-j].lre = raypack[x-i][y-j].lre + 1;
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
	      raypack[x-i-1][y-j].ritego |= 1;
	      raypack[x-i-1][y-j].lre = raypack[x-i][y-j].lre + 1;
          }
        }
        else {
            raypack[x-i][y-j].botego &= ~1;
            raypack[x-i-1][y-j].ritego &= ~1;
        }
        if ((raypack[x-i][y-j+1].botego) ||
            ((raypack[x-i][y-j].ritego))) {
          if (!raypack[x-i][y-j].drawn) DrawConcealableSquare(room, x-i, y-j);
          if (VFLAG(room, x-i,y-j)) VSET(x-i,y-j-1);
        }
	else {
          if (raypack[x-i][y-j].drawn) EraseSquare(room, x-i, y-j);
          if (!VFLAG(room, x-i,y-j)) UNVSET(x-i,y-j-1);
	}
      }

      /* right then down */
      if(x+i < ROOM_WIDTH && y-j > -1) {
        if(IsTransparent(room, x+i, y-j)) {
          if(raypack[x+i-1][y-j].ritego & 1) {
            if (raypack[x+i][y-j].lre >= lr) {
	      raypack[x+i][y-j].botego |= 1;
	      raypack[x+i][y-j-1].mre = 0;
	    }
	    else {
	      raypack[x+i][y-j].botego &= ~1;
	    }
	    if(raypack[x+i][y-j].mre <= mr) {
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
	      raypack[x+i][y-j].ritego |= 1;
	      raypack[x+i+1][y-j].lre = raypack[x+i][y-j].lre + 1;
          }
        }
        else {
            raypack[x+i][y-j].botego &= ~1;
            raypack[x+i][y-j].ritego &= ~1;
        }
        if ((raypack[x+i][y-j].botego) ||
            ((raypack[x+i-1][y-j].ritego))) {
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
      if(raypack[x-i][y+i].botego) {
        if (!raypack[x-i][y+i].drawn) DrawUnconcealableSquare(room,x-i,y+i);
        if (HFLAG(room, x-i, y+i)) HSET(x-i-1, y+i);
      }
      else {
        if (raypack[x-i][y+i].drawn) EraseSquare(room,x-i,y+i);
        if (!HFLAG(room, x-i, y+i)) UNHSET(x-i-1, y+i);
      }
      if(raypack[x-i][y+i].ritego) {
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
      if(raypack[x+i][y+i].botego) {
        if (!raypack[x+i][y+i].drawn) DrawUnconcealableSquare(room,x+i,y+i);
        if (HFLAG(room, x+i, y+i)) HSET(x+i+1, y+i);
      }
      else {
        if (raypack[x+i][y+i].drawn) EraseSquare(room,x+i,y+i);
        if (!HFLAG(room, x+i, y+i)) UNHSET(x+i+1, y+i);
      }
      if(raypack[x][y+i].ritego) {
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
      if(raypack[x+i][y].botego) {
       if (!raypack[x+i][y-i].drawn) DrawUnconcealableSquare(room,x+i,y-i);
       if (HFLAG(room, x+i, y-i)) HSET(x+i+1, y-i);
      }
      else {
       if (raypack[x+i][y-i].drawn) EraseSquare(room,x+i,y-i);
       if (!HFLAG(room, x+i, y-i)) UNHSET(x+i+1, y-i);
      }
      if(raypack[x][y-i].ritego) {
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
      if(raypack[x-i][y].botego) {
        if (!raypack[x-i][y-i].drawn) DrawUnconcealableSquare(room,x-i,y-i);
        if (HFLAG(room, x-i, y-i)) HSET(x-2, y-i);
      }
      else {
        if (raypack[x-i][y-i].drawn) EraseSquare(room,x-i,y-i);
        if (!HFLAG(room, x-i, y-i)) UNHSET(x-2, y-i);
      }
      if(raypack[x-i][y-i].ritego) {
        if (!raypack[x-i][y-i].drawn) DrawUnconcealableSquare(room,x-i,y-i);
        if (VFLAG(room, x-i, y-i)) VSET(x-i, y-2);
      }
      else {
        if (raypack[x-i][y-i].drawn) EraseSquare(room,x-i,y-i);
        if (!VFLAG(room, x-i, y-i)) UNVSET(x-i, y-2);
      }
    }

 }
  



EraseSquare(int room, int x, int y)
{
  raypack[x][y].drawn = 0;
  QDraw_request(room, x, y, BLACKED);
}

DrawOwnSquare(int room, int x, int y)
{
  raypack[x][y].drawn = 1;
  QDraw_request(room, x, y, EVERYTHING);
}

DrawUnconcealableSquare(int room, int x, int y)
{
  raypack[x][y].drawn = 1;
  QDraw_request(room, x, y, EVERYTHING);
}

DrawConcealableSquare(int room, int x, int y)
{
  raypack[x][y].drawn = 1;
  QDraw_request(room, x, y, CONCEALED);
}

EraseOwnSquare(int room, int x, int y)
{
  raypack[x][y].drawn = 0;
  QDraw_priority_request(room, x, y, BLACKED);
}

InitRaypack(int oldroom)
{
  int i,j;

  for(i=0; i<ROOM_WIDTH; i++) {
    for(j=0; j<ROOM_HEIGHT; j++) {
      EraseSquare(oldroom, i, j);
    }
  }
}

