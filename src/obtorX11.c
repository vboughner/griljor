/****************************************************************************
 * The War of Griljor
 *
 * By  Van A. Boughner
 *     Mel Nicholson
 * And Albert C. Baker III
 *
 * Students of the University of California at Berkeley
 * October 1989
 **************************************************************************/

#include "obtor.h"
#include "help.h"

/* set up the windows needed by the object editor */

obtor_window_setup()
{
  XSizeHints size;
  XWMHints hint;
  int count,count2;
  XGCValues values;

  rootwin = RootWindow(display, screen);

  values.graphics_exposures = True;
  XChangeGC(display,mainGC,GCGraphicsExposures,&values);

  GumbyCursor = XCreateFontCursor(display,XC_gumby);
  HandCursor = XCreateFontCursor(display,XC_hand2);
  DragCursor = XCreateFontCursor(display,XC_hand1);
  SpiralCursor = XCreateFontCursor(display,XC_box_spiral);
  ArrowCursor = XCreateFontCursor(display,XC_top_left_arrow);
  UpArrowCursor = XCreateFontCursor(display,XC_sb_up_arrow);
  DownArrowCursor = XCreateFontCursor(display,XC_sb_down_arrow);
  PirateCursor = XCreateFontCursor(display,XC_man);
  WatchCursor = XCreateFontCursor(display,XC_watch);
  load_window_pixmaps();
  
  mainwin = XCreateSimpleWindow(display, rootwin,
				MAIN_WIN_X,MAIN_WIN_Y,
				MAIN_WIN_W,MAIN_WIN_H,
				1,fgcolor,bgcolor);

  hint.flags = IconPixmapHint;
  hint.icon_pixmap = icon_pix;
  XSetWMHints(display,mainwin,&hint);
  size.flags = PPosition | PMinSize | PMaxSize;
  size.x = MAIN_WIN_X;
  size.y = MAIN_WIN_Y;
  size.min_width = MAIN_WIN_W;
  size.max_width = MAIN_WIN_W;
  size.min_height = MAIN_WIN_H;
  size.max_height = MAIN_WIN_H;
  XSetNormalHints(display,mainwin,&size);
  XStoreName(display, mainwin, "Griljor Object Editor");
  XDefineCursor(display,mainwin,WatchCursor);
  XSetWindowBackgroundPixmap(display,mainwin,foo_pix);
  XSelectInput(display, mainwin,KeyPressMask | ButtonPressMask);


  intro1win = XCreateSimpleWindow(display, mainwin,
				  INTRO_X, INTRO_X, 
				  intro1w,intro1h,
				  WINDOW_BORDERS, fgcolor, bgcolor);
  XSetWindowBackgroundPixmap(display,intro1win,intro1_pix);
  XSetWindowBorderPixmap(display,intro1win,border_pix);


  intro2win = XCreateSimpleWindow(display, mainwin,
				  INTRO_X, INTRO_Y, 
				  intro2w,intro2h,
				  WINDOW_BORDERS, fgcolor, bgcolor);
  XSetWindowBackgroundPixmap(display,intro2win,intro2_pix);
  XSetWindowBorderPixmap(display,intro2win,border_pix);

  XMapRaised(display,intro1win);  /* map it now for intro screen */
  XMapRaised(display,intro2win);  /* map it now for intro screen */
  XMapRaised(display,mainwin);  /* map it now for intro screen */
  XFlush(display); /* make sure it gets out */

  trywin = XCreateSimpleWindow(display, rootwin,
			       TRY_WIN_X, TRY_WIN_Y, 
			       TRY_WIN_W, TRY_WIN_H, 
			       WINDOW_BORDERS, fgcolor, bgcolor);
  XSelectInput(display, trywin, ButtonPressMask | ExposureMask );
  XDefineCursor(display,trywin,GumbyCursor);
  XStoreName(display,trywin,"Obtor Try Window");
  init_try_pixmaps();

  obtorwin = XCreateSimpleWindow(display, mainwin,
				 OBTOR_WIN_X, OBTOR_WIN_Y, 
				 OBTOR_WIN_W, OBTOR_WIN_H, 
				 WINDOW_BORDERS, fgcolor, bgcolor);

  XSetWindowBackgroundPixmap(display,obtorwin,backg_pix);
  XSelectInput(display, obtorwin,
	       KeyPressMask | ButtonPressMask | ExposureMask );
  XDefineCursor(display,obtorwin,ArrowCursor);
  
  obtmsgwin = 
    XCreateSimpleWindow(display, mainwin, OBTMSG_WIN_X, OBTMSG_WIN_Y, 
			OBTMSG_WIN_W, OBTMSG_WIN_H, WINDOW_BORDERS,
			fgcolor, bgcolor);
  XDefineCursor(display,obtmsgwin,SpiralCursor);
  XSetWindowBackgroundPixmap(display,obtmsgwin,foo_pix);
  XSelectInput(display, obtmsgwin,
	       KeyPressMask | ButtonPressMask | ExposureMask );
  
  for (count = 0;count < MAX_BUFFERS;count++)
    {
      objwin[count] =
	XCreateSimpleWindow(display,obtorwin,
			    0,0,1,1,WINDOW_BORDERS,fgcolor,bgcolor);
      /* with dummy location */
      XSetWindowBorderPixmap(display,objwin[count],border_pix);

      try_pix[count] =
	XCreatePixmap(display,objwin[count],
		      BITMAP_WIDTH+GRAB_SPACING,
		      BITMAP_HEIGHT+GRAB_SPACING,depth);
      set_window_background(count,-1);
      XSelectInput(display, objwin[count],
		   KeyPressMask | ButtonPressMask | ExposureMask );
      
      infowin[count] =
	XCreateSimpleWindow(display,objwin[count],
			    -WINDOW_BORDERS,-WINDOW_BORDERS,
			    OBTOR_WIN_W,BITMAP_HEIGHT,
			    GRAB_SPACING,fgcolor,bgcolor);
      /* with dummy location */
      XSelectInput(display, infowin[count], ExposureMask );
      XSetWindowBackgroundPixmap(display,infowin[count],info_pix);
      XDefineCursor(display,infowin[count],PirateCursor);

      upwin[count] =
	XCreateSimpleWindow(display,objwin[count],
			    -WINDOW_BORDERS,-WINDOW_BORDERS,
			    BITMAP_WIDTH,BITMAP_HEIGHT,
			    GRAB_SPACING,fgcolor,bgcolor);
      /* with dummy location */
      XSelectInput(display, upwin[count], ButtonPressMask);
      XDefineCursor(display,upwin[count],UpArrowCursor);
      XSetWindowBackgroundPixmap(display,upwin[count],up_pix);

      downwin[count] =
	XCreateSimpleWindow(display,objwin[count],
			    -WINDOW_BORDERS,-WINDOW_BORDERS,
			    BITMAP_WIDTH,BITMAP_HEIGHT,
			    GRAB_SPACING,fgcolor,bgcolor);
      /* with dummy location */
      XSelectInput(display, downwin[count], ButtonPressMask);
      XDefineCursor(display,downwin[count],DownArrowCursor);
      XSetWindowBackgroundPixmap(display,downwin[count],down_pix);
    }
  
  set_up_help();
  
  /* set the global redraw procedure */
  GlobalRedraw = redraw_an_obtor_window;
}

/*****************************************************************/

set_up_help()
{
  
  help_rec = make_help_window(rootwin, HELP_X, HELP_Y,
			      HELP_COLS, HELP_LINES, HELP_FONT);
  XStoreName(display,help_rec->win,"Obtor Help");
  XDefineCursor(display,help_rec->win,SpiralCursor);
  XSelectInput(display, help_rec->win,
	       KeyPressMask | ExposureMask );

  load_help_strings(help_rec, help_data);
}

/*****************************************************************/

load_help_strings(rec, data)
Help *rec;
char *data;
{
  int i;
  char *s,*p;
  extern char *malloc();

  s = malloc(strlen(data)+1);  /* +1 for /0 */
  if (s == NULL)
    return;

  strcpy(s,data);

  p = strtok(s,"\n");

  for (i=0; (i<rec->height && p) ; i++)
    {
      strcpy(rec->text[i], p);
      p = strtok(NULL,"\n");
    }

  free(s);
}

/*****************************************************************/

wait_for_ack()
{
  int what;
  int mx,my,detail;
  char keyp;
  Window where;

  XSetWindowBackgroundPixmap(display,intro2win,intro3_pix);
  XClearWindow(display,intro2win);
  XDefineCursor(display,mainwin,GumbyCursor);
  XBell(display,100);
  XSync(display,True);  /* start waiting for things */

  do
    what = get_input(mainwin, &keyp, &mx, &my, &detail, &where);
  while((what != KEYBOARD)&&(what != MOUSE));

  XDestroyWindow(display,intro1win);
  XDestroyWindow(display,intro2win);
  XFreePixmap(display,intro1_pix);
  XFreePixmap(display,intro2_pix);  
  XFreePixmap(display,intro3_pix);
}

/*****************************************************************/

map_obtor_windows()
{
  /* map the windows */
  XMapRaised(display, obtorwin);
  XMapRaised(display, obtmsgwin);
}

/*****************************************************************/

FindObtorWindow(win)
     Window win;
{
  int place;
  
  for (place = 0;place < MAX_BUFFERS;place++)
    if (objwin[place] == win)
      return(place);
  
  return(-1);
}

/*****************************************************************/
/* change the show numbering mode */

SetShowNumbers(int value)
{
  int place;

  if (value == show_numbers)
    return;

  show_numbers = value;

  for (place = 1;place < MAX_BUFFERS;place++)
    redraw_a_grab_window(place);
    
  return;
}


/*****************************************************************/
/* set up the variables about what is on the screen */

obtor_setup_variables()
{
  int i,j;
  
  for (i = 0;i < WIN_X_SPACE;i++)
    for (j = 0;j < WIN_Y_SPACE;j++)
      window_space[i][j] = -1;

  transmark = -1;
  mass_rec = NULL;
  try_win_up = False;
  show_numbers = False;
}

/*****************************************************************/

ClearXData(group,obj)
{
  item_pixmap[group][obj] = (Pixmap) NULL;
  item_mask[group][obj] = (Pixmap) NULL;
}

/*****************************************************************/

MakeBlankXData(group,size)
{
  int i;
  
  ResizeXData(group,size);
  
  for (i = 0;i < size;i++)
    ClearXData(group,i);
}

/*****************************************************************/

ResizeXData(groupnum,size)
     int groupnum,size;
{
  item_pixmap[groupnum] = (Pixmap *) realloc(item_pixmap[groupnum],
					     (sizeof(Pixmap) * size));
  demand(item_pixmap[groupnum], "no more memory for the pixmap array");
  item_mask[groupnum] = (Pixmap *) realloc(item_mask[groupnum],
					     (sizeof(Pixmap) * size));
  demand(item_pixmap[groupnum], "no more memory for the mask array");
}

/*****************************************************************/

/* map a given object window into position, resizing
   and clearing as necessary */

map_data_window(int which)
{
  clear_window_space(which);
  /* go and figure a nice place to put the window */
  if (!SetNewWindowRoot(which))
    return(FALSE);
  ExpandWindow(which);
  
  XMapWindow(display,objwin[which]);
  return(TRUE);
}


/*****************************************************************/

unmap_data_window(int which)
{
  if (transmark == which)
    transmark = -1;
  clear_window_space(which);
  XUnmapWindow(display,objwin[which]);
}

/*****************************************************************/

/* take the objects in a given info array and place pixmaps into 
   the pixmap array. */

objects_into_pixmaps(group)
     int group;
{
  int i;
  
  for (i=0; i<NUM_OBJ(group); i++) 
    certain_object_pixmap(group,i);
}
/*****************************************************************/

/* redo just one object's pixmap and mask, freeing the old ones if they
   are not NULL */

single_object_pixmap(objdef, pixvar, maskvar)
     ObjInfo *objdef;
     Pixmap  *pixvar, *maskvar;
{
  if (*pixvar) 
    {
      XFreePixmap(display, *pixvar);
      *pixvar = (Pixmap) NULL;
    }
  if (*maskvar) 
    {
      XFreePixmap(display, *maskvar);
      *maskvar = (Pixmap) NULL;
    }
  
  /* load/store the bitmap for this object */
  *pixvar = XCreatePixmapFromBitmapData(display, rootwin,
					objdef->bitmap, BITMAP_WIDTH,
					BITMAP_HEIGHT,
					fgcolor, bgcolor, depth);
  
  /* load/store the mask for this object, if it is used */
  if (objdef->masked) {
    *maskvar = XCreatePixmapFromBitmapData(display, rootwin,
					   objdef->mask, BITMAP_WIDTH,
					   BITMAP_HEIGHT,
					   fgcolor, bgcolor, depth);
  }
}


/*****************************************************************/

/* redo a certain object pixmap given group and number */

certain_object_pixmap(groupnum, objnum)
     int groupnum, objnum;
{
  single_object_pixmap(OBJ_DEF(groupnum)[objnum],
		       &(item_pixmap[groupnum][objnum]),
		       &(item_mask[groupnum][objnum]));
}


/* ============================== I N P U T ============================== */

/* the main editing procedure for the program */

obtor_get_input()
{
  int   i, x, y, detail, quit = FALSE;
  char  c;
  Window happenwin;
  
  while (!quit) 
    {
      i = get_input(obtorwin, &c, &x, &y, &detail, &happenwin);
      
      switch (i) 
	{
	  
	case KEYBOARD:
	  quit = handle_obtor_keypress(c, x, y, happenwin);
	  break;
	case MOUSE:
	  handle_obtor_mouse(x, y, c, detail, happenwin);
	  break;
	default:
	  break;
	}
    }
}
/*****************************************************************/

/* handle key press and return TRUE if we should quit, otherwise return 0 */

handle_obtor_keypress(c, x, y, what_win)
     char c;
     int x, y;
     Window what_win;
{
  int win,dest,num;
  char message[105];
  /* clear message area of previous garbage */

  place_obtor_message("");
  win = FindObtorWindow(what_win);
  
  switch (c)
    {
    case 'Q':
      opt_to_save_in_windows();
      return TRUE;
      break;
    case 'q':
      if (!placed_want_query(obtorwin,0,0,OBTOR_WIN_W,OBTOR_WIN_H,
			     "Do you really want to quit?")) break;
      opt_to_save_in_windows();
      return TRUE;
      break;
    case '!':
      if (!placed_want_query(obtorwin,0,0,OBTOR_WIN_W,OBTOR_WIN_H,
			     "Really quit without saving?")) break;
      return TRUE;
      break;
    case 's':
      if (win >= 0)
	save_win(win);
      else 
	opt_to_save_in_windows();
      break;
    case 'S':
      opt_to_save_in_windows();
      break;
    case 'n':
      if ((win = FindNewWindow()) >= 0)
	obtor_create_new(win);
      break;
    case '#':
      SetShowNumbers(!show_numbers);
      break;
    case 'i':
      handle_obtor_info_request();
      break;
    case 'a':
      handle_mass_info(what_win, x, y);
      break;
    case 'x':
      if ((win = FindObjToEdit(&num)) > -1)
	delete_obj_from_group(win,num);
      break;
    case 'o':
      if ((win = FindObjToEdit(&num)) > -1)
	insert_obj_in_group(make_blank_definition(),win,num);
      break;
    case 'I':
      if (window_info_up)
	HideWindowInfo();
      else
	ShowWindowInfo();
      break;
    case 'b':
      edit_obtor_bitmap(what_win);
      break;
    case 'm':
      edit_obtor_mask(what_win);
      break;
    case 't':
      if ((win = FindObjToEdit(&num)) > -1)
	if (IS_VALID_OBJ(win,num))
	  set_window_background(win,num);
      break;
    case 'T':
      if (win >= 0)
	set_window_background(win,-1);
      break;
    case 'p':
      if (win >= 0)
	{
	  dest = transmark;
	  transmark = win;
	  sprintf(message,"Mark set in window #%d",win);
	  place_obtor_message(message);
	  redraw_info_window(win);
	  if (dest >= 0)
	    redraw_info_window(dest);
	}
      break;
    case 'u':
      toggle_try_window();
      break;
    case '?':
      toggle_help_window(help_rec);
      break;
    case 'h':
      if (win >= NORM_WINDOW)
	splitwindow(win,True);
      break;
    case 'v':
      if (win >= NORM_WINDOW)
	splitwindow(win,False);
      break;
    case 'l':
      if (win >= NORM_WINDOW)
	get_bitmap(win);
      break;
    case 'L':
      if (win >= NORM_WINDOW)
	get_bitmap_directory(win);
      break;
    case 'e':
      if (win >= NORM_WINDOW)
	ExpandWindow(win);
      break;
      /* Note: The result of these numbers is dependent
	 on WIN_X_SPACE and WIN_Y_SPACE to some extent */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (c == '0') /* calculate for '10' instead of '0' */
	dest = 9;
      else
	dest = c - '1';
      
      if (win >= NORM_WINDOW)
	force_move_window(win,dest);
      break;
    case 'k':
      if (win >= NORM_WINDOW)
	unload_window(win);
      break;
    case 'f':	
      load_file();
      break;
    case 'd':	
      win = FindNewWindow();
      if (win >= NORM_WINDOW)
	obtor_load_object_file(DEFAULT_OBJ_PATH,win);
      break;
    case 'w':
      if (win >= 0)
	write_info(win);
      break;
    case '.':
      handle_header_edit_request();
      break;
    default:
      break;
    }
  
  return FALSE;
}

/*****************************************************************/


handle_obtor_mouse(x, y, button, detail, what_win)
     int x, y;
     char button;
     int detail;
     Window what_win;
{
  int win,i,num;
  
  place_obtor_message("");

  if ((win = FindObtorWindow(what_win)) >= 0)
    switch (button)
      {
      case Button1:
	drag_object(win,x,y);
	break;
      case Button3:
	handle_obtor_info_request();
	break;
      case Button2:
	if (transmark >= 0)
	  {
	    num = obtor_obj_in_grab_win(x,y,win);
	    if (IS_VALID_OBJ(win,num))
	      recursive_transfer(win,num,transmark);
	  }
	else
	  {
	    XBell(display,100);
	    place_obtor_message("The destination mark is not set.");
	  }
	break;
      }
  
  for (i = 0; i < MAX_BUFFERS;i++)
    {
      if (upwin[i] == what_win)
	ClickArrowWin(i,(int) button,True);
      if (downwin[i] == what_win)
	ClickArrowWin(i,(int) button,False);
    }
}

/*****************************************************************/

/* get a line of input in obtor message area */

char *get_obtor_input_line(prompt, maxlen)
     char *prompt;
     int maxlen;
{
  int exposed;
  char *foo;

  /* clear the message area */
  XClearWindow(display, obtmsgwin);
  
  foo = get_string(obtmsgwin, regfont, OBTMSG_PAD, OBTMSG_PAD,
		   prompt, maxlen, &exposed);
  place_obtor_message("");
  return(foo);
  
}


/* ask user if he wishes to save the two files */

opt_to_save_in_windows()
{
  int place;
  
  for (place = NORM_WINDOW;place < MAX_BUFFERS;place++)
    if (data_loaded[place])
      query_save_win(place);
}


/* ask the user for yes or no on saving one of the object groups */

query_save_win(num)
     int num;
{
  int letter;
  
  if (!file_changed[num])
    return(True);
  
  letter = placed_letter_query(objwin[num], 0, 0, OBJWIN_W(num), OBJWIN_H(num),
			       "Save these (y/n/c)?","ync");
  
  if (letter == 'y')
    {
      save_win(num);
      return(True);
    }
  
  if (letter == 'n')
    return(True);
  
  return(False);    /* cancel */
}

/*****************************************************************/

save_win(num)
     int num;
{
  char *filename;
  
  if (strlen(objfile_name[num]) == 0)
    {
      filename = get_obtor_input_line("Filename: ", 119);
      if (strlen(filename) == 0)
	{
	  place_obtor_message("No file saved");
	  return(False);
	}
      strcpy(objfile_name[num],filename);
      redraw_info_window(num);
    }
  
  if (!obtor_save_object_file(objfile_name[num],num))
    return(False);

    
  return(True);
}

/* ask the user for yes or no on loading info about a group */

load_file()
{
  char *filename,message[1024];
  int which;
  
  filename = get_obtor_input_line("Filename: ", 119);
  if (strlen(filename)) 
    {
      sprintf(message,"Loading %s...",filename);
      place_obtor_message(message);

      which = FindNewWindow();
      if (which >= 0)
	{
	  if (obtor_load_object_file(filename,which))
	    sprintf(message,"%s loaded.",filename);
	  else
	    sprintf(message,"Error, %s not loaded.",filename);
	  
	  place_obtor_message(message);	
	  return(0);
	}
    }
  place_obtor_message("No File Loaded.");
}

/*****************************************************************/

write_info(num)
     int num;
{
  char *filename,oldname[1024],message[1024];
  
  filename = get_obtor_input_line("Filename: ", 119);
  if (strlen(filename))
    {
      strcpy(oldname,objfile_name[num]);
      strcpy(objfile_name[num],filename);
      sprintf(message,"Saving %s...",filename);
      place_obtor_message(message);
      if (!save_win(num))
	{
	  sprintf(message,"Error, not saved as %s.",filename);
	  place_obtor_message(message);
	  strcpy(objfile_name[num],oldname);
	  return(False);
	}
      sprintf(message,"File saved as %s.",filename);
      redraw_info_window(num);
      place_obtor_message(message);
    }
  else
    place_obtor_message("No File Saved.");
}



/* ======================== O U T P U T =============================== */


/* redraw the indicated window for the obtor */

redraw_an_obtor_window(win)
     Window win;
{
  int winnum;
  
  if ((winnum = FindObtorWindow(win)) >= 0) 
    redraw_a_grab_window(winnum);
  
  for (winnum = 0;winnum < MAX_BUFFERS;winnum++)
    if (infowin[winnum] == win)
      redraw_info_window(winnum);
  
  if (win == obtmsgwin)
    place_obtor_message(NULL);

  if (win == trywin)
    draw_try_window();

  check_help_expose(win);
}

/* place a message in the stat window in the appropriate font.  If a null
   length message is given as the message, then redraw the last message */

place_obtor_message(line)
     char *line;
{
  static char current_msg[120] = "";


  if (line != NULL)
    strcpy(current_msg, line);
    
  
  XClearWindow(display, obtmsgwin);
  text_write(obtmsgwin, regfont, OBTMSG_PAD, OBTMSG_PAD, 0, 0, current_msg);
  XFlush(display);  /* make sure this is shown now! */
}


/* place message about the fact that map was just saved on the screen */

obtor_save_msg()
{
  char msg[80];
  
  strcpy(msg, "Saved.");
  place_obtor_message(msg);
}

/* place message about the fact that map was not saved on the screen */

obtor_nosave_msg()
{
  char msg[80];
  
  strcpy(msg, "Error, file not saved.");
  XBell(display,100);
  place_obtor_message(msg);
}



/*****************************************************************/

FindObjToEdit(num)
     int *num;
{
  int groupnum;
  
  groupnum = FindObtorWindow(last_event.xany.window);
  if (groupnum >= 0)
    *num = 
      obtor_obj_in_grab_win(last_event.xkey.x,last_event.xkey.y,groupnum);
  
  if (*num < 0) groupnum = -1;
  return(groupnum);
}

/*****************************************************************/

