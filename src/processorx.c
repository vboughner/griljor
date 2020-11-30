#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include "lib.h"
#include "def.h"
#include "password.h"
#include "standardx.h"
#include "processorx.h"
#include "xx.h"
  
  void draw_processor_speed_1 (disp)
disp_fields	disp;
{
  static double phi;
  double y,angle;
  int x;
  
  XClearArea (disp->display, disp->win, TOP_X_PROCESSOR, TOP_Y_PROCESSOR,
	      LENGTH_PROCESSOR, 2 * AMPLITUDE_PROCESSOR, FALSE); 
  
  if (phi > ((double) (2 * PI))) phi -= (2 * PI) * ( (int) (phi / (((double) 2)
								   * PI)));
  x = TOP_X_PROCESSOR;
  angle = phi;
  phi += ANGLE_INCREMENT;
  while (x < (TOP_X_PROCESSOR + LENGTH_PROCESSOR))
    {
      y =  TOP_Y_PROCESSOR + AMPLITUDE_PROCESSOR + ( (int) (AMPLITUDE_PROCESSOR
							    * cos (angle)));
      x++;
      XDrawPoint (disp->display, disp->win, disp->regular_gc, x, (int) y);
      angle += ANGLE_INCREMENT;
    }
  
}

void draw_processor_speed_2 (disp)
     disp_fields	disp;
{
  static int x;
  static int dx;
  if (!disp->processor_on) return;
  if (!dx) dx = 1;
  if (!x) x = TOP_X_PROCESSOR + 1;
  
  XDrawLine (disp->display, disp->win, disp->regular_erase_gc,x,
	     TOP_Y_PROCESSOR,x, TOP_Y_PROCESSOR + 2 * AMPLITUDE_PROCESSOR - 1);
  
  if ((x <= TOP_X_PROCESSOR) ||
      (x >= (TOP_X_PROCESSOR + LENGTH_PROCESSOR - 1))) dx = -dx;
  x += dx;
  
  XDrawLine (disp->display, disp->win, disp->regular_gc,x,
	     TOP_Y_PROCESSOR,x, TOP_Y_PROCESSOR + 2 * AMPLITUDE_PROCESSOR - 1);
}

void draw_processor_box (disp)
     disp_fields	disp;
{
  draw_erase_box (TOP_X_PROCESSOR - 1,TOP_Y_PROCESSOR - 1,LENGTH_PROCESSOR + 1,
		  AMPLITUDE_PROCESSOR * 2 + 1, FALSE, disp);
}

void draw_button (disp)
     disp_fields disp;
{
  XDrawArc (disp->display, disp->win, disp->regular_gc, 
	    CENTER_BUTTON_X - RADIUS_BUTTON,
	    CENTER_BUTTON_Y - RADIUS_BUTTON,
	    RADIUS_BUTTON * 2, RADIUS_BUTTON * 2, 
	    0, 360 * 64);
}

void display_top_line (disp)
     disp_fields	disp;
{
  XClearArea (disp->display, disp->win, 2,2, BITM_WINDOW_WIDTH, 22, False);
  /* Draw The Title of Window */
  XSetFont (disp->display, disp->regular_gc, disp->regfont->fid);
  XDrawString (disp->display, disp->win, disp->regular_gc,
	       5,18, "Griljor Character Editor",
	       strlen ("Griljor Character Editor"));
  if (disp->processor_on)
    {
      XDrawString (disp->display, disp->win, disp->regular_gc,
		   230,18, "Processor Status On",
		   strlen ("Processor Status On"));
      draw_processor_box (disp);
    }
  else
    XDrawString (disp->display, disp->win, disp->regular_gc,
		 230,18, "Processor Status Off",
		 strlen ("Processor Status Off"));
  draw_button (disp);
  if (disp->processor_on)
    XFillArc (disp->display, disp->win, disp->regular_gc, 
	      CENTER_BUTTON_X - RADIUS_BUTTON + 4,
	      CENTER_BUTTON_Y - RADIUS_BUTTON + 4,
	      RADIUS_BUTTON * 2 - 8, RADIUS_BUTTON * 2 - 8, 
	      0, 360 * 64);
}

void handle_button_events (disp)
     disp_fields disp;
{
  int dummy;
  disp->processor_on = !disp->processor_on;
  display_top_line (disp);
  if (disp->processor_on)
    XFillArc (disp->display, disp->win, disp->regular_gc, 
	      CENTER_BUTTON_X - RADIUS_BUTTON + 4,
	      CENTER_BUTTON_Y - RADIUS_BUTTON + 4,
	      RADIUS_BUTTON * 2 - 8, RADIUS_BUTTON * 2 - 8, 
	      0, 360 * 64);
  else
    XFillArc (disp->display, disp->win, disp->regular_erase_gc, 
	      CENTER_BUTTON_X - RADIUS_BUTTON + 4,
	      CENTER_BUTTON_Y - RADIUS_BUTTON + 4,
	      RADIUS_BUTTON * 2 - 8, RADIUS_BUTTON * 2 - 8, 
	      0, 360 * 64);
}

boolean in_button_area (x,y)
     int x,y;
{
  double dist,value1, value2;
  int dist_int;
  
  value1 = x - (double) CENTER_BUTTON_X;
  value2 = y - (double) CENTER_BUTTON_Y;
  value1 = pow (value1, 2.0);
  value2 = pow (value2, 2.0);
  dist = sqrt (value1 + value2);
  dist_int = (int) dist;
  if (dist_int <= RADIUS_BUTTON) return TRUE; else return FALSE;
}

/* This piece of code has been copied directly from XSWARM, a really
   neat little program that puts bees and a wasp on your screen, and
   was written by Jeff Butterworth on 7/11/90.  He in turn used 
   psychoII to do stuff. */

/* nap
 **
 ** put the process to sleep for a while
 */

int catchalarm() { }    /* SIGALRM (null) signal handler */
     
     void nap(sec,usec)
     long sec, usec;
{
  extern int setitimer();
  extern int sigpause();
  struct itimerval wait;
  static int sigset = FALSE;
  
  /* If usec is larger than 1 second: */
  while(usec >= 1e6) { sec++;  usec -= 1e6; }
  
  if(!sigset) {
    signal(SIGALRM,catchalarm);
    sigset = TRUE;
  }
  
  wait.it_value.tv_sec = sec;
  wait.it_value.tv_usec = usec;
  wait.it_interval.tv_sec = 0;
  wait.it_interval.tv_usec = 0;
  
  if((setitimer(ITIMER_REAL,&wait,NULL)) == -1) {
    handle_error(FATAL, "bad setitimer()");
  }
  sigpause(0);
}
