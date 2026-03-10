#include <stdio.h> 
#include <X11/Xlib.h> 
#include <X11/Xutil.h>

static Pixmap beach[3];
static Pixmap path[6];
static Pixmap forest;
static Pixmap sand;
static Pixmap river[21];
static Pixmap mask[21];

/* The creator */

static int bpoints[20][2] = {17, 39, 17, 39, 30, 39, 30, 38, 30, 37,
			     30, 37, 30, 37, 30, 36, 30, 35, 0, 4,
			     0, 7, 0, 8, 0, 9, 0, 14, 0, 13,
			     0, 13, 0, 13, 0, 12, 0, 1, 0, 10 };
static int fpoints1[5][2] = {0, 13, 0, 12, 0, 11, 0, 10, 0, 9 };
static int fpoints2[12][2] = {20, 22, 19, 22, 18, 23, 18, 23, 18, 23,
			      15, 23, 14, 23, 14, 33, 14, 39, 13, 39,
			      27, 39, 11, 39 };
static int fpoints3[14][2] = {39, 39, 38, 39, 38, 39, 38, 39, 37, 39, 
			      36, 39, 35, 39, 35, 39, 35, 39, 35, 39, 
			      35, 39, 35, 39, 35, 39, 35, 39 };
static int spoints1[10][2] = {17, 28, 17, 28, 14, 28, 13, 28, 12, 28,
			     11, 19, 8, 18, 8, 17, 9, 17, 10, 17 };
static int spoints2[7][2] = {23, 28, 23, 28, 24, 33, 24, 33, 24, 33,
			    24, 33, 24, 33 };
static int hpoints[6][3] = {2, 14, 28, 3, 6, 12, 6, 3, 4,
			    9, 30, 33, 16, 27, 33, 18, 2, 25 };
static int vpoints[5][3] = {2, 7, 18, 5, 4, 5, 26, 17, 17,
			    29, 3, 8, 34, 10, 15 };
static int lrpoints[4][2] = {13, 2, 5, 3, 2, 6, 26, 16 };
static int llpoints[2][2] = {29, 2, 34, 9};
static int urpoints[2][2] = {2, 18, 29, 9};
static int ulpoints[4][2] = {5, 6, 13, 3, 26, 18, 34, 16};
static int r0points[39] = {39, 6, 2, 5, 2, 9, 9, 11, 11, 12, 11, 13, 11,
			     14, 11, 15, 11, 16, 11, 18, 7, 19, 7, 20, 7, 
			     21, 7, 22, 7, 23, 7, 24, 7, 26, 14,
			     27, 14, 28, 14};
static int r2points[37] = {37, 7, 1, 9, 4, 11, 8, 13, 9, 16, 4, 17, 4, 18, 4,
			     19, 4, 20, 4, 21, 4, 22, 4, 23, 4, 24, 4, 
			     25, 4, 26, 4, 28, 11, 29, 11, 30, 11};
static int r4points[33] = {33, 6, 0, 8, 2, 10, 5, 10, 6, 10, 7, 17, 8, 
			     17, 9, 17, 10, 28, 6, 28, 7, 28, 8, 27, 10, 
			     32, 13, 33, 15, 34, 18, 34, 19};
static int r6points[33] = {33, 4, 0, 4, 1, 7, 4, 7, 5, 8, 7, 8, 8, 10, 10,
			     14, 7, 14, 8, 15, 5, 24, 9, 24, 10, 24, 11, 
			     25, 13, 30, 17, 31, 19};
static int r8points[7] = {7, 14, 6, 15, 4, 24, 8};
static int r9points[19] = {19, 8, 1, 10, 4, 12, 8, 27, 4, 28, 5, 31, 11,
			     32, 12, 33, 14, 34, 17};
static int r10points[17] = {17, 4, 2, 7, 6, 8, 9, 10, 11, 24, 12, 25, 14, 
			     29, 15, 30, 18};
static int r11points[5] = {5, 17, 11, 28, 9};
static int r12points[5] = {5, 17, 7, 27, 9};
static int r13points[17] = {17, 7, 2, 8, 6, 10, 9, 25, 12, 25, 7, 29, 14, 
			    30, 15, 31, 18};
static int r14points[19] = {19, 6, 1, 8, 4, 10, 8, 27, 5, 27, 11, 31, 12,
			    32, 14, 33, 17, 12, 9};
static int r15points[7] = {7, 14, 9, 15, 6, 25, 8};
static int r16points[123] = {123, 5, 0, 5, 1, 8, 5, 9, 5, 9, 6, 9, 7, 9, 8,
			     11, 9, 11, 10, 12, 10, 13, 10, 14, 10, 15, 10,
			     16, 10, 15, 7, 15, 8, 15, 9, 16, 5, 16, 6, 
			     16, 7, 16, 8, 16, 9, 26, 5, 26, 6, 26, 7,
			     26, 8, 26, 9, 26, 10, 26, 11, 26, 12, 26, 13,
			     26, 6, 26, 7, 26, 8, 27, 6, 27, 7, 27, 8,
			     25, 9, 25, 10, 25, 11, 26, 12, 27, 12,
			     28, 12, 29, 12, 30, 12, 26, 13, 27, 13, 
			     28, 13, 29, 13, 30, 13, 31, 13, 31, 15, 
			     30, 14, 31, 14, 32, 15, 31, 17, 32, 17, 
			     32, 18, 32, 19, 33, 18, 33, 19};


void drawScene(display, window, background, context, maskGC, drawGC, 
	       fg, bg, depth)
     Display *display;
     Window window;
     Pixmap background;
     GC context;
     GC maskGC;
     GC drawGC;
     int fg, bg;
     int depth;
{
  int x, y, i, num, inc;

  /*     
   * Set up the main animation loop
   */  

  /* use 1 for the depth, and invert the bitmaps (by switching bg and fg) */
  readSceneBitmaps(display, window, bg, fg, 1, river, mask,
		   beach, path, &forest, &sand); 

  /*  Beach Loop */
  inc = 0;
  for(y=0; y<=19*32; y+=32) {
    for(x=bpoints[inc][0]*32; x<=bpoints[inc][1]*32; x+=32) {
      num = randNum(3);
      XCopyPlane(display, beach[num], background, context, 
		0, 0, 32, 32, x, y,1);
    }
    inc++;
  }

  /*  Forest Loop 0 */
  for(x=0; x<=16*32; x+=32) {
    for(y=0; y<=4*32; y+=32) {
      XCopyPlane(display, forest, background, context, 
		0, 0, 32, 32, x, y,1);
    }
  }
  /* Forest Loop 1 */
  inc = 0;
  for(y=5*32; y<=9*32; y+=32) {
    for(x=fpoints1[inc][0]*32; x<=fpoints1[inc][1]*32; x+=32) {
      XCopyPlane(display, forest, background, context, 
		0, 0, 32, 32, x, y,1);
    }
    inc++;
  }
  /* Forest Loop 2 */
  inc = 0;
  for(y=8*32; y<=19*32; y+=32) {
    for(x=fpoints2[inc][0]*32; x<=fpoints2[inc][1]*32; x+=32) {
      XCopyPlane(display, forest, background, context, 
		0, 0, 32, 32, x, y,1);
    }
    inc++;
  }
  /* Forest Loop 3 */
  inc = 0;
  for(y=3*32; y<=16*32; y+=32) {
    for(x=fpoints3[inc][0]*32; x<=fpoints3[inc][1]*32; x+=32) {
      XCopyPlane(display, forest, background, context, 
		0, 0, 32, 32, x, y,1);
    }
 inc++;
  }

  /* Sand Loop 1 */
  inc=0;
  for(y=3*32; y<=12*32; y+=32) {
    for(x=spoints1[inc][0]*32; x<=spoints1[inc][1]*32; x+=32) {
      XCopyPlane(display, sand, background, context, 
		0, 0, 32, 32, x, y,1);
    }
    inc++;
  }
  
  /* Sand Loop 2 */
  inc=0;
  for(y=8*32; y<=14*32; y+=32) {
    for(x=spoints2[inc][0]*32; x<=spoints2[inc][1]*32; x+=32) {
      XCopyPlane(display, sand, background, context, 
		0, 0, 32, 32, x, y,1);
    }
    inc++;
  }

  /* Path Loops */
  for(inc=0; inc<6; inc++) {
    for(x=hpoints[inc][1]*32; x<=hpoints[inc][2]*32; x+=32) {
      y=hpoints[inc][0]*32;
      XCopyPlane(display, path[0], background, context, 
		0, 0, 32, 32, x, y,1);
    }
  }
  for(inc=0; inc<5; inc++) {
    for(y=vpoints[inc][1]*32; y<=vpoints[inc][2]*32; y+=32) {
      x=vpoints[inc][0]*32;
      XCopyPlane(display, path[1], background, context, 
		0, 0, 32, 32, x, y,1);
    }
  }
  for(inc=0; inc<4; inc++) {
    XCopyPlane(display, path[2], background, context, 
	      0, 0, 32, 32, lrpoints[inc][0]*32, lrpoints[inc][1]*32,1);
  }
  for(inc=0; inc<2; inc++) {
    XCopyPlane(display, path[3], background, context, 
	      0, 0, 32, 32, llpoints[inc][0]*32, llpoints[inc][1]*32,1);
  }
  for(inc=0; inc<2; inc++) {
    XCopyPlane(display, path[4], background, context, 
	      0, 0, 32, 32, urpoints[inc][0]*32, urpoints[inc][1]*32,1);
  }
  for(inc=0; inc<4; inc++) {
    XCopyPlane(display, path[5], background, context, 
	      0, 0, 32, 32, ulpoints[inc][0]*32, ulpoints[inc][1]*32,1);
  }

  /* River Loop */
  for(inc=1; inc<r0points[0]; inc+=2) {
    num = randNum(2);
    XCopyPlane(display, mask[num], background, maskGC, 
	      0, 0, 32, 32, r0points[inc]*32, r0points[inc+1]*32,1);
    XCopyPlane(display, river[num], background, drawGC, 
	      0, 0, 32, 32, r0points[inc]*32, r0points[inc+1]*32,1);
  }
  for(inc=1; inc<r2points[0]; inc+=2) {
    num = randNum(2);
    XCopyPlane(display, mask[num+2], background, maskGC, 
	      0, 0, 32, 32, r2points[inc]*32, r2points[inc+1]*32,1);
    XCopyPlane(display, river[num+2], background, drawGC, 
	      0, 0, 32, 32, r2points[inc]*32, r2points[inc+1]*32,1);
  }
  for(inc=1; inc<r4points[0]; inc+=2) {
    num = randNum(2);
    XCopyPlane(display, mask[num+4], background, maskGC, 
	      0, 0, 32, 32, r4points[inc]*32, r4points[inc+1]*32,1);
    XCopyPlane(display, river[num+4], background, drawGC, 
	      0, 0, 32, 32, r4points[inc]*32, r4points[inc+1]*32,1);
  }
  for(inc=1; inc<r6points[0]; inc+=2) {
    num = randNum(2);
    XCopyPlane(display, mask[num+6], background, maskGC, 
	      0, 0, 32, 32, r6points[inc]*32, r6points[inc+1]*32,1);
    XCopyPlane(display, river[num+6], background, drawGC, 
	      0, 0, 32, 32, r6points[inc]*32, r6points[inc+1]*32,1);
  }
  for(inc=1; inc<r8points[0]; inc+=2) {

    XCopyPlane(display, mask[8], background, maskGC, 
	      0, 0, 32, 32, r8points[inc]*32, r8points[inc+1]*32,1);
    XCopyPlane(display, river[8], background, drawGC, 
	      0, 0, 32, 32, r8points[inc]*32, r8points[inc+1]*32,1);
  }
  for(inc=1; inc<r9points[0]; inc+=2) {

    XCopyPlane(display, mask[9], background, maskGC, 
	      0, 0, 32, 32, r9points[inc]*32, r9points[inc+1]*32,1);
    XCopyPlane(display, river[9], background, drawGC, 
	      0, 0, 32, 32, r9points[inc]*32, r9points[inc+1]*32,1);
  }
  for(inc=1; inc<r10points[0]; inc+=2) {
    num = randNum(2);
    XCopyPlane(display, mask[10*num+10], background, maskGC, 
	      0, 0, 32, 32, r10points[inc]*32, r10points[inc+1]*32,1);
    XCopyPlane(display, river[10*num+10], background, drawGC, 
	      0, 0, 32, 32, r10points[inc]*32, r10points[inc+1]*32,1);
  }
  for(inc=1; inc<r11points[0]; inc+=2) {

    XCopyPlane(display, mask[11], background, maskGC, 
	      0, 0, 32, 32, r11points[inc]*32, r11points[inc+1]*32,1);
    XCopyPlane(display, river[11], background, drawGC, 
	      0, 0, 32, 32, r11points[inc]*32, r11points[inc+1]*32,1);
  }
  for(inc=1; inc<r12points[0]; inc+=2) {

    XCopyPlane(display, mask[12], background, maskGC, 
	      0, 0, 32, 32, r12points[inc]*32, r12points[inc+1]*32,1);
    XCopyPlane(display, river[12], background, drawGC, 
	      0, 0, 32, 32, r12points[inc]*32, r12points[inc+1]*32,1);
  }
  for(inc=1; inc<r13points[0]; inc+=2) {

    XCopyPlane(display, mask[13], background, maskGC, 
	      0, 0, 32, 32, r13points[inc]*32, r13points[inc+1]*32,1);
    XCopyPlane(display, river[13], background, drawGC, 
	      0, 0, 32, 32, r13points[inc]*32, r13points[inc+1]*32,1);
  }
  for(inc=1; inc<r14points[0]; inc+=2) {

    XCopyPlane(display, mask[14], background, maskGC, 
	      0, 0, 32, 32, r14points[inc]*32, r14points[inc+1]*32,1);
    XCopyPlane(display, river[14], background, drawGC, 
	      0, 0, 32, 32, r14points[inc]*32, r14points[inc+1]*32,1);
  }
  for(inc=1; inc<r15points[0]; inc+=2) {

    XCopyPlane(display, mask[15], background, maskGC, 
	      0, 0, 32, 32, r15points[inc]*32, r15points[inc+1]*32,1);
    XCopyPlane(display, river[15], background, drawGC, 
	      0, 0, 32, 32, r15points[inc]*32, r15points[inc+1]*32,1);
  }

  for(inc=1; inc<r16points[0]; inc+=2) {
    num = randNum(2);
    XCopyPlane(display, river[16+num], background, context, 
	      0, 0, 32, 32, r16points[inc]*32, r16points[inc+1]*32,1);
  }
  for(x=17*32; x<=25*32; x+=32) {
    for(y=5*32; y<=6*32; y+=32) {
      num = randNum(2);
      XCopyPlane(display, river[17+num], background, context, 
		0, 0, 32, 32, x, y,1);
    }
  }
}
