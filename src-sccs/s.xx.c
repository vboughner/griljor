h34989
s 00000/00000/00668
d D 1.6 92/08/07 01:04:40 vbo 6 5
c source copied to a separate tree for work on new map and object format
e
s 00034/00012/00634
d D 1.5 91/04/17 16:52:31 labc-3id 5 4
c added -text option
e
s 00003/00003/00643
d D 1.4 91/04/16 15:29:29 labc-3id 4 3
c fixed a couple procedure type warnings
e
s 00000/00001/00646
d D 1.3 91/04/14 23:06:25 labc-3id 3 2
c 
e
s 00001/00000/00646
d D 1.2 91/04/14 19:51:55 labc-3id 2 1
c in outputX11.c
e
s 00646/00000/00000
d D 1.1 91/04/14 19:35:48 labc-3id 1 0
c date and time created 91/04/14 19:35:48 by labc-3id
e
u
U
f e 0
t
T
I 1
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "standardx.h"
#include "lib.h"
#include "def.h"
I 2
D 3
#include "extern.c"
E 3
E 2
#include "password.h"
#include "xx.h" /* This has to be last */

/* This procedure loads the three fonts that this program uses, (big,
   regular, and tiny).  The program exits if they cannot be loaded */
	
  load_all_fonts (disp)
disp_fields	disp;
{
  disp->bigfont = XLoadQueryFont (disp->display, BIGX_FONT_PATH);
  disp->regfont = XLoadQueryFont (disp->display, REGX_FONT_PATH);
  disp->tinyfont = XLoadQueryFont(disp->display, TINYX_FONT_PATH);
  if (disp->bigfont == NULL) handle_error (WARNING, "Bigfont could not be loaded.");
  if (disp->regfont == NULL) handle_error (WARNING, "Regfont could not be loaded.");
  if (disp->tinyfont == NULL) handle_error (WARNING, "Tinyfont could not be loaded.");
  
  if ( (disp->bigfont == NULL) || (disp->regfont == NULL) || 
      (disp->tinyfont == NULL)) handle_error (FATAL, "Could not load font files.");
}

/* This procedure sets up all the x-stuff and also mallocs space for the big
   disp structure */

disp_fields setup_window ()
{
  disp_fields		disp;
  XEvent		report;
  XGCValues		values;
  XSetWindowAttributes	win_attr;
  unsigned long		value_mask;
  Drawable		drawable;
  int			flag,counter;
  inferior		gc;
  char			all_done[5];
  XWMHints		wmhints;

  /* All_done is used so that I won't start drawing until all the windows
     are fully mapped */
  
  for (counter = 0; counter < 5; counter++) all_done[counter] = 0;

  /* Allocate memory for the structure & the subwindows */

  if ((disp = (disp_fields) malloc (sizeof (disp_struct))) == NULL)
    handle_error (FATAL, "Out of memory, go buy some more.");
  
  if ((disp->bitmap_win = (inferior) malloc (sizeof (sub_window))) == NULL)
    handle_error (FATAL, "Out of memory, go buy some more.");
  if ((disp->player_attributes_win = (inferior) malloc (sizeof (sub_window)))
      == NULL) handle_error (FATAL, "Out of memory, go buy some more.");
  if ((disp->menu_win = (inferior) malloc (sizeof (sub_window))) == NULL)
    handle_error (FATAL, "Out of memory, go buy some more.");
  if ((disp->slider_win = (inferior) malloc (sizeof (sub_window))) == NULL)
    handle_error (FATAL, "Out of memory, go buy some more.");
  
  /* Connect to X server */
  
  if (!(disp->display = XOpenDisplay (NULL)))
    handle_error (FATAL, "Could not conect to X server.");
  
  /* The screen which we will be displaying onto */
  
  disp->screen = DefaultScreen (disp->display);
  
  /* set up the foreground & background pixels */
  
  disp->fgcolor = WhitePixel (disp->display, disp->screen);
  disp->bgcolor = BlackPixel (disp->display, disp->screen);
  
  /* Create the window */
  
  disp->win = XCreateSimpleWindow (disp->display, RootWindow (disp->display,
							      disp->screen),
				   WINDOW_X, WINDOW_Y, WINDOW_WIDTH, 
				   WINDOW_HEIGHT, WINDOW_BORDER, 
				   disp->fgcolor, disp->bgcolor);
  
  /* Tell the window manager what kind of events I care about */
  
  XSelectInput (disp->display, disp->win, ExposureMask | KeyPressMask |
		ButtonPressMask | PointerMotionMask);
  
  /* Name my window */
  
  XStoreName (disp->display, disp->win, "Griljor Character Editor");
  
  /* Create the menu_window */
  
  disp->menu_win->win = XCreateSimpleWindow (disp->display, disp->win,
					     MENU_WINDOW_X,MENU_WINDOW_Y,
					     MENU_WINDOW_WIDTH,
					     MENU_WINDOW_HEIGHT,MENU_WINDOW_BORDER,
					     disp->fgcolor,disp->bgcolor);
  
  XSelectInput (disp->display, disp->menu_win->win, ExposureMask | 
		ButtonPressMask | PointerMotionMask | LeaveWindowMask);

  /* The CWOverrideRedirect makes sure that the window manage doesn't
     worry about the pacement of the subwindows */

  value_mask = CWOverrideRedirect;
  win_attr.override_redirect = True;
  XChangeWindowAttributes (disp->display, disp->menu_win->win, value_mask,
			   &win_attr);
  
  /* Create the player_attributes_window */
  
  disp->player_attributes_win->win = XCreateSimpleWindow (disp->display, disp->win,
							  ATTR_WINDOW_X,ATTR_WINDOW_Y,
							  ATTR_WINDOW_WIDTH,
							  ATTR_WINDOW_HEIGHT,ATTR_WINDOW_BORDER,
							  disp->fgcolor,disp->bgcolor);
  
  XSelectInput (disp->display, disp->player_attributes_win->win, ExposureMask | 
		ButtonPressMask | PointerMotionMask);
  
  value_mask = CWOverrideRedirect;
  win_attr.override_redirect = True;
  XChangeWindowAttributes (disp->display, disp->player_attributes_win->win, 
			   value_mask,&win_attr);
  
  /* Create the slider_window */
  
  disp->slider_win->win = XCreateSimpleWindow (disp->display, disp->win,
					       SLID_WINDOW_X,SLID_WINDOW_Y,
					       SLID_WINDOW_WIDTH,
					       SLID_WINDOW_HEIGHT,SLID_WINDOW_BORDER,
					       disp->fgcolor,disp->bgcolor);
  
  XSelectInput (disp->display, disp->slider_win->win, ExposureMask | 
		ButtonPressMask | PointerMotionMask);
  
  value_mask = CWOverrideRedirect;
  win_attr.override_redirect = True;
  XChangeWindowAttributes (disp->display, disp->slider_win->win, 
			   value_mask,&win_attr);
  
  /* Create the bitmap_window */
  disp->bitmap_win->win = XCreateSimpleWindow (disp->display, disp->win,
					       BITM_WINDOW_X,BITM_WINDOW_Y,
					       BITM_WINDOW_WIDTH,
					       BITM_WINDOW_HEIGHT,BITM_WINDOW_BORDER,
					       disp->fgcolor,disp->bgcolor);
  
  XSelectInput (disp->display, disp->bitmap_win->win, ExposureMask | 
		ButtonPressMask | PointerMotionMask);
  
  value_mask = CWOverrideRedirect;
  win_attr.override_redirect = True;
  XChangeWindowAttributes (disp->display, disp->bitmap_win->win, 
			   value_mask,&win_attr);
  
  
  /* Load all the pixmaps */

  load_all_bitmaps (disp);

  /* Set the icon of the window */
  wmhints.icon_pixmap = disp->icon;
  wmhints.flags	= IconPixmapHint;
  XSetWMHints (disp->display, disp->win, &wmhints);
  
  /* Setup the GC's */
  
  load_all_fonts (disp);
  
  /* Regular GC */
  
  values.foreground	= disp->fgcolor;
  values.background	= disp->bgcolor;
  values.function	= GXcopy;
  values.font		= disp->tinyfont->fid;
  
  disp->regular_gc = XCreateGC (disp->display, disp->win,
				(GCForeground | GCBackground | GCFunction |
				 GCFont), 
				&values);
  
  /* Regular Erase GC */
  
  values.foreground	= disp->fgcolor;
  values.background	= disp->bgcolor;
  values.function	= GXcopyInverted;
  values.font		= disp->tinyfont->fid;
  
  disp->regular_erase_gc = XCreateGC (disp->display, disp->win,
				      (GCForeground | GCBackground | GCFunction |
				       GCFont), 
				      &values);
  
  /* Inversion GC */
  
  values.foreground	= disp->fgcolor;
  values.background	= disp->bgcolor;
  values.function	= GXinvert;
  
  disp->inversion_gc	= XCreateGC (disp->display, disp->menu_win->win,
				     (GCForeground | GCBackground | GCFunction),
				     &values);
  
  /* Set the background of the bitmap window */
  
  XSetWindowBackgroundPixmap (disp->display, disp->bitmap_win->win,
			      disp->title);
  /* Map the actual windows onto the screen */
  
  XMapWindow (disp->display, disp->win);
  XMapWindow (disp->display, disp->menu_win->win);
  XMapWindow (disp->display, disp->player_attributes_win->win);
  XMapWindow (disp->display, disp->bitmap_win->win);
  XMapWindow (disp->display, disp->slider_win->win);
  
  /* Wait until an expose event comes our way */
  /* Do this for all of the windows */
  
  /* Main window */
  
  while ((!(all_done[0])) || (!(all_done[1])) || (!(all_done[2])) ||
	 (!(all_done[3])) || (!(all_done[4])))
    {
      XNextEvent (disp->display, &report);
      if (report.type == Expose)
	{
	  drawable = report.xexpose.window;
	  if (drawable == disp->win) all_done[0] = 1; 
	  else if (drawable == disp->menu_win->win) all_done[1] = 1; 
	  else if (drawable == disp->player_attributes_win->win) all_done[2] = 1; 
	  else if (drawable == disp->slider_win->win) all_done[3] = 1; 
	  else if (drawable == disp->bitmap_win->win) all_done[4] = 1; 
	}
    }
  return disp;
}

/* This kind of useless procedure draws or erases a rectangle */

void draw_erase_box (x,y,top_size,side_size,erase_flag,disp)
     int x,y,top_size,side_size;
     boolean erase_flag;
     disp_fields disp;
{
  if (erase_flag)
    XDrawRectangle (disp->display, disp->win, disp->regular_erase_gc,
		    x,y,top_size, side_size);
  else
    XDrawRectangle (disp->display, disp->win, disp->regular_gc,
		    x,y,top_size, side_size);
}

/* This draws all the lines that need to be redraws @ the start &
   after expose events to make the main window look nicer */

void draw_bits (disp)
     disp_fields	disp;
{
  
  XSetLineAttributes (disp->display, disp->regular_gc,
		      WINDOW_BORDER, LineSolid, CapButt, JoinMiter);
  
  XDrawLine (disp->display, disp->win, disp->regular_gc, WINDOW_BORDER - 1,0,
	     WINDOW_BORDER - 1, MENU_WINDOW_Y);
  
  XDrawLine (disp->display, disp->win, disp->regular_gc, BITM_WINDOW_WIDTH +
	     WINDOW_BORDER + 1, 0, BITM_WINDOW_WIDTH + WINDOW_BORDER + 1,
	     MENU_WINDOW_Y);
  
  XDrawLine (disp->display, disp->win, disp->regular_gc, 0,WINDOW_BORDER - 1 ,
	     BITM_WINDOW_WIDTH + WINDOW_BORDER , WINDOW_BORDER - 1);
  
  XDrawLine (disp->display, disp->win, disp->regular_gc, 0, 25,
	     WINDOW_WIDTH - WINDOW_BORDER, 25);
  
  XSetLineAttributes (disp->display, disp->regular_gc,
		      0, LineSolid, CapButt, JoinMiter);
}

/* This is the most important procedure.  It handles all events...
   I first check if there are any events using Xpending, and if there
   are, whatever procedures are called */

void handle_events(disp)
     disp_fields	disp;
{
  XEvent	report;
  int		temp,temp2;
  
  if (XPending (disp->display)) {
    XNextEvent (disp->display, &report);
    switch (report.type) {
    /* Self-explanatory */
    case Expose: {
      
      if (report.xexpose.window == disp->bitmap_win->win)
	redraw_bitmap_win (disp);
      if (report.xexpose.window == disp->player_attributes_win->win)
	{
	  redraw_attr_win(disp);
	  disp->attr_win_expose = TRUE;
	}
      else if (report.xexpose.window == disp->slider_win->win)
	redraw_slid_win(disp);
      else if (report.xexpose.window == disp->menu_win->win)
        redraw_menu_win (disp);
      else if ((report.xexpose.window == disp->win) &&
	       (!report.xexpose.count))
	redraw_main_win (disp);
      break;
    }
    /* This is if the cursor moves.  We are concerned about this event
       in the menu window because if a cursor moves in that window,
       a new area might need to be drawn/redrawn */

    case MotionNotify: {
      
      if (report.xmotion.window == disp->menu_win->win)
	{
	  handle_menu_area_cursor (report.xmotion.x,
				   report.xmotion.y,
				   disp, NULL);
	}
      break;
    }
    /* If we leave the menu, we need to erase whatever highlighted
       button there was */

    case LeaveNotify: {
      
      if (report.xcrossing.window == disp->menu_win->win)
	handle_menu_area_cursor (NULL,NULL,disp,LEAVING);
      break;
    }
    /* For a button-press, the following may happen :
	1) A button press When we are being asked the question Yes/No
	   NOT in the Yes/No box, in which case we must cancel the
	   operation 
	2) A press in the player attributes window, call the procedure
	   that handles that.
	3) A button press, when there is a message at the top of the 
	   screen.  If that is so, erase the message
	4) A button press in the slider window.  Scroll the list of 
	   players up or down.  Check if any high-lighted people have
	   appeared in the scrolling window or have moved off.
	5) A button press in the scroll field.  Call the procedure
	   that handles that.
	6) If the button press is in the main window, and inside the
	   button that turns the cheezy Xload thingy off/on, call the
	   procedure that handles that */

    case ButtonPress: {
      
      if ((disp->button_value != NOTHING) &&
	  (report.xbutton.window != disp->menu_win->win))
	handle_confirm_cancel (disp, FALSE);
      else
	if (disp->message_there) handle_message (disp, FALSE);
      if ((report.xbutton.window == disp->player_attributes_win->win) &&
	  disp->current_edit_no)
	handle_attributes_events (report.xbutton.x,
				  report.xbutton.y,
				  disp);
      
      else if (report.xbutton.window == disp->menu_win->win)
	handle_menu_options (report.xbutton.x,
			     report.xbutton.y,
			     disp);
      else if (report.xbutton.window == disp->slider_win->win)
	{
	  temp = area_slider (disp, report.xbutton.x,
			      report.xbutton.y);
	  flash_slider_area (disp, report.xbutton.x,
			     report.xbutton.y);
	  
	  if ((temp == SCROLL_UP)
	      && (scroll_down_one (disp, disp->user)))
	    disp->user--;
	  else if ((temp == SCROLL_DOWN)
		   && (scroll_up_one (disp, disp->user)))
	    disp->user++;
	  else if (temp == SCROLL_UP_PAGE)
	    for (temp = NUM_SELECTIONS; 
		 (temp) && (scroll_down_one (disp,disp->user));
		 temp--, disp->user--);
	  else if (temp == SCROLL_DOWN_PAGE)
	    for (temp = NUM_SELECTIONS;
		 (temp) && (scroll_up_one (disp,disp->user));
		 temp--, disp->user++);
	  
	  flash_slider_area (disp, report.xbutton.x,
			     report.xbutton.y);
	  
	  /* Since we have scrolled, we could be putting someone off
	     the screen */
	  
	  if (disp->current_edit_no)
	    if ((disp->current_edit_no < disp->user) ||
		(disp->current_edit_no > (disp->user + NUM_SELECTIONS - 1)))
	      disp->high_light_off_screen = TRUE;
	  
	  /* Since we have scrolled, it is possible that the person
	     we are editing and should be high-lighted just came back
	     on the scroll region */
	  if (disp->high_light_off_screen)
	    if ((disp->current_edit_no >= disp->user) &&
		(disp->current_edit_no < (disp->user + NUM_SELECTIONS)))
	      {
		disp->high_light_off_screen = FALSE;
		flash_scroll_area (disp, (disp->current_edit_no - disp->user + 1));
	      }
	}
      else if ((report.xbutton.window == disp->win) &&
	       cursor_is_in_scroll_field (report.xbutton.x,report.xbutton.y))
	{
	  handle_scroll_events (disp, report.xbutton.x,report.xbutton.y);
	}
      if ((report.xbutton.window == disp->win) &&
	  in_button_area (report.xbutton.x,report.xbutton.y))
	handle_button_events (disp);
      break; 
    }
    }
  }
}

/* This starts up everything in X and displays the initial screen */

void init_driver(disp)
     disp_fields disp;
{
  display_top_line (disp);
  setup_menu(disp);
  setup_attr(disp);
  display_menu(disp);
  display_slider (disp);
  draw_bits(disp);
}

/* This starts up stuff not in X */

void notx_driver(disp)
     disp_fields	disp;
{
  strcpy (disp->bm_file, "None");
  strcpy (disp->bm_m_file, "None");
}

/* This procedure clears the text_enter line.  It is usually called after 
   some information has been entered in the text-enter line */

void clear_text_line (disp)
     disp_fields disp;
{
  XClearArea (disp->display, disp->win, ATTR_WINDOW_BORDER, 
	      ATTR_WINDOW_Y , ATTR_WINDOW_WIDTH, MENU_WINDOW_Y,
	      False);
}
I 5



/* this procedure takes all the integer dates that might have been changed
   by the driver and converts them to string dates */

void update_all_string_dates(data)
     PersonLink *data;
{
  char *get_time_string();
  PersonLink *ptr;

  for (ptr=data; ptr; ptr=ptr->next)
    strcpy(ptr->person->strdate, get_time_string(ptr->person->date));
}



E 5
main(argc, argv)
     int argc;
D 5
     char *argv;
E 5
I 5
     char **argv;
E 5
{
D 5
  register short counter;
E 5
I 5
  short counter;
E 5
  int dummy;
  disp_fields	disp;
  
I 5
  if (argc == 2) {
    if (!strcmp(argv[1], "-text")) exit(textual_main(argc, argv));
    else if (!strcmp(argv[1], "-h")) exit(print_editpass_usage(argc, argv));
    else set_password_file(argv[1]);
  }
  else if (argc == 3) {
    if (!strcmp(argv[1], "-text")) exit(textual_main(argc, argv));
    else exit(print_editpass_usage(argc, argv));
  }
  else if (argc > 3) exit(print_editpass_usage(argc, argv));

E 5
  disp = setup_window();

  /* Setup some silly things, many booleans */
D 5

E 5
  disp->scroll_flash = FALSE;
  disp->current_edit = NULL;
  if (!disp->file) disp->current_edit_no = 0; else disp->current_edit_no = 1;
  disp->high_light_off_screen = FALSE;
  disp->message_there = FALSE;
  disp->processor_on = FALSE;
  disp->button_value = NOTHING;
  disp->attr_win_expose = FALSE;
  disp->genocide_mode = FALSE;
  disp->genocide_kluge = FALSE;
  disp->erase_background = FALSE;
  
D 5
  /***** From the non-x editpass *****/
  /* This code has been swiped from regular editpass
     written by Albert C. Baker */
  
  if (argc > 1)
    set_password_file(argv[1]);
  
E 5
  /* load the file without locking it */
  disp->file = Load_File(&(disp->end), FALSE);
  demand (disp->file,"File not found.");
  
D 5
  /* Back to my stuff */
E 5
I 5
  /* convert integer dates to strings */
  update_all_string_dates(disp->file);

E 5
  /* Disp->MaxUsers is how many users there currently are */
D 5
  
E 5
  disp->MaxUsers = display_users(disp,1,TRUE);
  if (disp->MaxUsers) disp->user = 1; else disp->user = 0;

  notx_driver(disp);
  init_driver(disp);

  strcpy (disp->message, "Ready to go!");
  handle_message (disp, TRUE);
  handle_button_events (disp);
  
  counter = 0;
  while (1) {
    handle_events(disp);
    if (counter == PROC_SPEED) {draw_processor_speed_2 (disp); counter = 0;}
    else counter++;
  }
}

/* This procedure, usually called after an expose events, clears the main
   window, draws the stuff at the top of the window, the little bits here
   and there, redraws any messages, redraws the scroll field, and highlights
   a person if there is on in the scroll field */

D 4
void redraw_main_win (disp)
E 4
I 4
redraw_main_win (disp)
E 4
     disp_fields	disp;
{
  XClearWindow (disp->display, disp->win);
  display_top_line(disp);
  draw_bits (disp);
  handle_message (disp,TRUE);
  display_users (disp, disp->user, FALSE);
  if ((disp->current_edit_no) &&
      (!(disp->high_light_off_screen)))
    flash_scroll_area
      (disp, disp->current_edit_no - disp->user + 1);
}

/* Obvious */

redraw_all_windows (disp)
     disp_fields disp;
{
  redraw_attr_win(disp);
  redraw_slid_win(disp);
  redraw_menu_win (disp);
  redraw_main_win(disp);
}

/* This procedure is rather unelegant, but it works ! It had to be written
   this way, or otherwise another event loop would of had to be written.
   it handles answers to yes/no questions, which may be answered any amount
   of time after they are asked.  */

D 4
void handle_confirm_cancel (disp,flag)
E 4
I 4
handle_confirm_cancel (disp,flag)
E 4
     disp_fields disp;
     boolean	flag; /* TRUE for Confirm, FALSE for Cancel */
{
  int counter;
D 4
  PersonLink *temp,*temp2;
E 4
I 4
  PersonLink *temp,*temp2, *return_user_link();
E 4
  switch (disp->button_value) {

  /* Delete a person */

  case DELETE_VALUE : {
    if (flag) {
      /* If there is only one person left to delete, set the end of the file
	 and the beginning to NULL */
      if (disp->MaxUsers == 1) 
	disp->end = disp->file = NULL;
      /* Otherwise .... */
      else
	{
	  temp = return_user_link (disp, disp->MaxUsers - 1);
	  if (disp->current_edit_no == disp->MaxUsers) /* Last user */
	    {
	      temp2 = disp->end;
	      disp->end = temp;
	      disp->end->next = NULL;
	      free (temp2);
	    }
	  else
	    if (disp->current_edit_no != 1) /* Kluge Because old editpass doesn't work*/
	      DeleteUser (disp->file, disp->current_edit_no);
	    else /* Erase the first record */
	      {
		temp = disp->file;
		disp->file = disp->file->next;
		free (temp);
	      }
	  disp->MaxUsers--;
	  strcpy (disp->message, "Character Deleted");
	  handle_message (disp, TRUE);
	  /* Send Expose events to the scroll window & the attributes window */
	  XClearArea (disp->display, disp->win,0,0,1,1,True);
	  XClearArea (disp->display, disp->player_attributes_win->win,0,0,1,1,True);
	  disp->current_edit_no = 0;
	  disp->attr_win_expose = FALSE;
	  disp->button_value = NOTHING;
	}
    }
    /* If we answered 'No' we don't want to delete that user */    
    else 
      {
	strcpy (disp->message, "Cancelled");
	handle_message (disp, TRUE);
	if (!disp->attr_win_expose) flash_area (disp->slave_x, disp->slave_y, disp);
	disp->attr_win_expose = FALSE;
	disp->button_value = NOTHING;
      }
    break;
  }
   /* Quitting, a yes, means quit, a no is not quitting */

  case QUIT_VALUE : {
    if (flag)
      {
	strcpy (disp->message, "Make Changes Permanent?");
	handle_message (disp, TRUE);
	disp->button_value = SAVE_CHANGES;
	break;
      }
    else
      {
	strcpy (disp->message, "Cancelled");
	handle_message (disp, TRUE);
	disp->button_value = NOTHING;
	flash_area (disp->slave_x , disp->slave_y, disp);
	return;
      }
  }

  /* After quitting, we are asked if we want to save changes or not.. 
     A yes means save and a no means don't save the changes */

  case SAVE_CHANGES : {
    XUnmapWindow (disp->display, disp->win);
    if (flag)
      {
	Write_File(disp->file);
	printf ("Changes Saved\n");
      }
    else printf ("Changes Not Saved\n");
    printf ("Bye!\n");
    exit(0);
  }
  }
}
E 1
