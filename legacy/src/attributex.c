#include <stdio.h>
#include <X11/Xlib.h>
#include <time.h>
#include "standardx.h"
#include "lib.h"
#include "def.h"
#include "password.h"
#include "xx.h"
#include "attributex.h"
#include "bitmapx.h"
  
time_t evaluate_time_string();

void setup_attr(disp)
disp_fields	disp;
{
  int x = 0,y = 0,counter;
  /* Clear the window */
  XClearWindow (disp->display, disp->player_attributes_win->win);
  /* Draw horizontal line(s) */
  counter = Y_DIVISIONS_ATTR;
  while (--counter)
    {
      
      y += Y_SEP_ATTR;
      XDrawLine (disp->display, disp->player_attributes_win->win,
		 disp->regular_gc,x,y,
                 x + ATTR_WINDOW_WIDTH,y);
    }
  /* Draw vertical line(s) */
  counter = X_DIVISIONS_ATTR;
  y = 0; x = 0;
  while (--counter)
    {
      if (counter == 2) {x+= X_SEP_ATTR; counter--;} /* Skip first line */
      x += X_SEP_ATTR;
      XDrawLine (disp->display, disp->player_attributes_win->win,
		 disp->regular_gc,x,y,
                 x,y + ATTR_WINDOW_HEIGHT);
    }
}

void clear_attribute_fields (disp)
     disp_fields	disp;
{
  int y = 1,counter = 1;
  for (; counter <= Y_DIVISIONS_ATTR; counter++, y += Y_SEP_ATTR)
    XClearArea (disp->display, disp->player_attributes_win->win, 0, y,
		((X_SEP_ATTR * 2) ), Y_SEP_ATTR - 1,FALSE);
}

void draw_attr_fields (disp, file)
     disp_fields disp;
     PersonLink	*file;
{
  float	ratio;
  int x,y,counter;
  static char *stable[27] = {"Name: ", "Password: ", "Login: ", "Host: ", 
	     "Exp Points: ", "Rank: ", "Kills: ",
	     "Losses : ", "Ratio : ", "Games Won: ", "Deity Mode: ",
	     "Character Date: ", "Quit", "Add Character", "Delete Character",
	     "Bitmap File Name", "Mask File Name", "Toggle Bitmaps",
	     "Toggle Grid", "Memory >> Bitmap", "Memory >> Mask",
	     "Memory << Bitmap", "Memory << Mask",
	     "Edit Bitmap", "Edit Mask", "Toggle Genocide"};
  char *un_stable[7];
  int un_stable2[6];

  un_stable[0] = file->person->name;
  un_stable[1] = file->person->password;
  un_stable[2] = file->person->login;
  un_stable[3] = file->person->host;
  un_stable[4] = NULL;
  un_stable[5] = file->person->rank;
  un_stable[6] = file->person->strdate;

  un_stable2[0] = file->person->ExpPts;
  un_stable2[1] = 0;
  un_stable2[2] = file->person->kills;
  un_stable2[3] = file->person->losses;
  un_stable2[4] = 0;
  un_stable2[5] = file->person->games;
  
  x = 0;
  y = BEGIN_Y_ATTR_TEXT;
  
  clear_attribute_fields (disp);
  
  for (counter = 1; counter <= (Y_DIVISIONS_ATTR * 2); counter++)
    {
      if (counter == 9) /* Ratio */
	{
	  if (!file->person->losses) ratio = (float) file->person->kills;
	  else ratio = (float) file->person->kills / (float) file->person->losses;
	  sprintf (disp->line, "%s", stable[counter - 1]);
	  sprintf (disp->line2,"%2.2f",ratio);
	}
      else if (counter == 11) /* Deity Mode */
	{
	  sprintf (disp->line,"%s", stable[counter - 1]);
	  if (file->person->deity) sprintf (disp->line2, "%s", "On");
	  else sprintf (disp->line2, "%s", "Off");
	}
      else if ((counter < 5) || (counter == 6))
	{
	  sprintf (disp->line, "%s", stable[counter - 1]);
	  sprintf (disp->line2, "%s", un_stable[counter - 1]);
	}
      else if (counter == 12)
	{
	  sprintf (disp->line, "%s", stable [counter - 1]);
	  sprintf (disp->line2, "%s", un_stable [counter - 6]);
	}
      else if (counter < 12)
	{
	  sprintf (disp->line, "%s", stable[counter - 1]);
	  sprintf (disp->line2, "%d", un_stable2[counter - 5]);
	}
      
      else if (counter == 15)
	{
	  x = 2 * X_SEP_ATTR + 1;
	  y = BEGIN_Y_ATTR_TEXT;
	  sprintf (disp->line, "%s", stable[counter - 3]);
	  sprintf (disp->line2, "%s", "");
	}
      else if ((counter <= 28) && (counter > 15))
	{
	  sprintf (disp->line, "%s", stable[counter - 3]);
	  sprintf (disp->line2, "%s", "");
	}
      
      if ((counter < 13) || (counter > 14) && (counter <= 28))
	{
	  strcat (disp->line, disp->line2);
	  
	  text_write (disp->player_attributes_win->win,
		      disp->regfont, x , y , 0, 0, disp->line,
		      disp->display, disp->regular_gc);
	}
      y += Y_SEP_ATTR;
    }
}

int attributes_area (x,y,disp)
     int x,y;
     disp_fields	disp;
{
  if (x >= (2 * X_SEP_ATTR)) /* Command section */
    return ((int) (1 + Y_DIVISIONS_ATTR + (y / Y_SEP_ATTR)));
  else return ((int) (1 + (y / Y_SEP_ATTR)));
}

void flash_area (x,y,disp)
     int x,y;
     disp_fields	disp;
{
  int area,x1,y1,height,width,counter,dummy;
  area = attributes_area (x,y,disp);
  
  if (area > Y_DIVISIONS_ATTR) /* Command option */
    {
      x1 = 2 * X_SEP_ATTR + 1;
      y1 = Y_SEP_ATTR * (area - Y_DIVISIONS_ATTR - 1) + 1;
      width = X_SEP_ATTR ; height = Y_SEP_ATTR - 1;
    }
  else {
    x1 = 0;
    y1 = (Y_SEP_ATTR) * (area - 1) + 1;
    width = 2 * X_SEP_ATTR ; height = Y_SEP_ATTR - 1;
  }
  XCopyArea (disp->display, disp->player_attributes_win->win,
	     disp->player_attributes_win->win,
	     disp->inversion_gc, x1,y1,
	     width, height,x1,y1);
}

void handle_attributes_events (x,y,disp)
     int x,y;
     disp_fields	disp;
{
  char input_text[MAXSTRLEN2];
  int area,beenexposed,dummy;
  PersonLink *file, *temp;
  Pixmap	temp_map;
  
  if (!disp->genocide_kluge)
    {
      /* If we are not editing anyone or pressing a button, exit */
      if ((area <= Y_DIVISIONS_ATTR) && (!disp->current_edit_no)) return;
      area = attributes_area(x,y,disp);
      dummy = disp->current_edit_no - 1;
      /* Find the right file */
      file = disp->file;
      while ((file) && (dummy)) {file = file->next; dummy--;}
    }
  else
    area = 17;
  
  switch (area) {
  case 1:  /* Change Name */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New Name : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	strcpy (file->person->name, input_text);
	strcpy (disp->message, "Name Field Changed");
	handle_message (disp, TRUE);
      }
      break;
    }
  case 2: /* Change password */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New Password : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	strcpy (file->person->password, input_text);
	strcpy (disp->message, "Password Field Changed");
	handle_message (disp, TRUE);
      }
      break;
    }
  case 3: /* Change login name */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New Login : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	strcpy (file->person->login, input_text);
	strcpy (disp->message, "Login Field Changed");
	handle_message (disp, TRUE);
	
      }
      break;
    }
  case 4: /* Change host name */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New Host Name : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	strcpy (file->person->host, input_text);
	strcpy (disp->message, "Host Name Field Changed");
	handle_message (disp, TRUE);
	
      }
      break;
    }
  case 5: /* Change # of Exp points name */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New # Of Exp Points : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	file->person->ExpPts = atoi (input_text);
	strcpy (disp->message, "Experience Points Field Changed");
	handle_message (disp, TRUE);
	
      }
      break;
    }
  case 6: /* Change rank name */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New Rank : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	strcpy (file->person->rank, input_text);
	strcpy (disp->message, "Rank Name Field Changed");
	handle_message (disp, TRUE);
	
      }
      break;
    }
  case 7: /* Change # of kills */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New # Of Kills : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	file->person->kills = atoi (input_text);
	strcpy (disp->message, "# Of Kills Field Changed");
	handle_message (disp, TRUE);
	
      }
      break;
    }
  case 8: /* Change # of losses */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New # of Losses : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	file->person->losses = atoi (input_text);
	strcpy (disp->message, "# Of Losses Field Changed");
	handle_message (disp, TRUE);
	
      }
      break;
    }
  case 9: /* Try to change ratio */
    {
      strcpy (disp->message, "Change Kills & Losses To Change Ratio");
      handle_message (disp, TRUE);
      break;
    }
  case 10: /* Change # of games won */
    {
      flash_area (x,y,disp);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New # Of Games Won : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text != '\0') {
	file->person->games = atoi (input_text);
	strcpy (disp->message, "# Of Games Won Field Changed");
	handle_message (disp, TRUE);
      }
      break;
    }
  case 11: /* Change deity mode */
    {
      file->person->deity = !(file->person->deity);
      strcpy (disp->message, "Deity Mode Field Changed");
      handle_message (disp, TRUE);
      break;
    }
    
  case 12: /* Change Character Date */
    {
      flash_area (x,y,disp);
      if (!disp->current_edit_no)
	{
	  strcpy (disp->message, "Edit A Character First");
	  handle_message (disp, TRUE);
	  flash_area (x,y,disp);
	  break;
	}
      *(disp->line) = '\0';
      clear_text_line (disp);
      strcpy (disp->message, "Press <Enter> To Set Date To Now");
      handle_message (disp, TRUE);
      strcpy (input_text,  get_string (disp->win,
				       disp->regfont,
				       TEXT_LINE_X,TEXT_LINE_Y,
				       "New Month : ",
				       MAX_TEXT_LINE_LENGHT,
				       &beenexposed,
				       disp->display,
				       disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*input_text == '\0')
	{
	  strcpy (disp->line , get_current_time_string());
	  strcpy (disp->message, "Date Set To Now");
	  handle_message (disp, TRUE);
	}
      else
	{
	  strncat (disp->line, input_text, 2);
	  disp->line[2] = '/';
	  disp->line[3] = '\0';
	  clear_text_line (disp);
	  strcpy (disp->message, "Enter New Day");
	  handle_message (disp, TRUE);
	  strcpy (input_text,  get_string (disp->win,
					   disp->regfont,
					   TEXT_LINE_X,TEXT_LINE_Y,
					   "New Day : ",
					   MAX_TEXT_LINE_LENGHT,
					   &beenexposed,
					   disp->display,
					   disp->regular_gc));
	  if (beenexposed) redraw_all_windows (disp);
	  strncat (disp->line, input_text,2);
	  disp->line[5] = '/';
	  disp->line[6] = '\0';
	  clear_text_line (disp);
	  strcpy (disp->message, "Enter New Year");
	  handle_message (disp, TRUE);
	  strcpy (input_text,  get_string (disp->win,
					   disp->regfont,
					   TEXT_LINE_X,TEXT_LINE_Y,
					   "New Year : ",
					   MAX_TEXT_LINE_LENGHT,
					   &beenexposed,
					   disp->display,
					   disp->regular_gc));
	  if (beenexposed) redraw_all_windows (disp);
	  strcat (disp->line, input_text);
	  disp->line[8] = '\0';
	  strcat (disp->line, " 00:00.00");
	}
      temp = (PersonLink *) return_user_link (disp, disp->current_edit_no);
      strcpy (temp->person->strdate , disp->line);
      temp->person->date = evaluate_time_string(temp->person->strdate);
      clear_text_line (disp);
      break;
    }
    
  case 15: /* Quit */
    {
      flash_area (x,y,disp);
      disp->slave_x = x; disp->slave_y = y;
      strcpy (disp->message, "Are You Sure You Want To Quit?");
      handle_message (disp, TRUE);
      disp->button_value = QUIT_VALUE;
      return;
    }
  case 16: /* Add a character */
    {
      flash_area (x,y,disp);
      temp = (PersonLink *) malloc (sizeof (PersonLink));
      if (!temp) handle_error (FATAL, "Not enough Memory.");
      else temp->person = (PersonRec *) malloc (sizeof (PersonRec));
      if (!temp->person) handle_error (FATAL, "Not enough Memory.");
      
      *(temp->person->name) = '\0';
      *(temp->person->password) = '\0';
      temp->person->date = time(NULL);
      temp->person->ExpPts = 0;
      *(temp->person->rank) =  '\0';
      temp->person->bitmap_saved = FALSE;
      temp->person->kills = temp->person->losses = temp->person->games = 0;
      temp->person->deity = FALSE;
      *(temp->person->login) = *(temp->person->host) = '\0';
      strcpy (temp->person->strdate , get_current_time_string());
      
      disp->end->next = temp;
      temp->next = NULL;
      disp->end = temp;
      strcpy (disp->message, "New Character Created");
      handle_message (disp, TRUE);
      disp->MaxUsers++;
      flash_area (x,y,disp);
      break;
    }
  case 17: /* Delete a character */
    {
      flash_area (x,y,disp);
      if (disp->genocide_mode)
	{
	  disp->button_value = DELETE_VALUE;
	  disp->attr_win_expose = TRUE;
	  handle_confirm_cancel (disp, TRUE);
	  disp->genocide_kluge = FALSE;
	  return;
	}
      sprintf (disp->line, "Delete %d ?", disp->current_edit_no);
      strcpy (disp->message, disp->line);
      handle_message (disp, TRUE);
      disp->button_value = DELETE_VALUE;
      disp->attr_win_expose = FALSE;
      return;
    }
  case 18: /* Bitmap File Name */
    {
      flash_area (x,y,disp);
      strcpy (disp->line, get_string (disp->win, disp->regfont,
				      TEXT_LINE_X, TEXT_LINE_Y,
				      "New File Name : ", 30, &beenexposed, disp->display,
				      disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*(disp->line) != '\0') {
	strcpy (disp->bm_file, disp->line);
	strcpy (disp->message, "Bitmap File Name Changed");
	handle_message (disp, TRUE);
	redraw_bitmap_win (disp);
      }
      clear_text_line (disp);
      flash_area (x,y,disp);
      break;
    }
  case 19: /* Mask File Name */
    {
      flash_area (x,y,disp);
      strcpy (disp->line, get_string (disp->win, disp->regfont, 
				      TEXT_LINE_X, TEXT_LINE_Y,
				      "New Mask File Name : ", 30, &beenexposed, disp->display,
				      disp->regular_gc));
      if (beenexposed) redraw_all_windows (disp);
      if (*(disp->line) != '\0') {
	strcpy (disp->bm_m_file, disp->line);
	redraw_bitmap_win (disp);
      }
      clear_text_line (disp);
      flash_area (x,y,disp);
      break;
    }
  case 20: /* Show bitmaps */
    {
      flash_area (x,y,disp);
      /* If we are showing the bitmaps for the first time, 
	 clear the background as to not royally irritate the user */
      if (!disp->erase_background)
	{
	  disp->erase_background = TRUE;
	  XSetWindowBackground (disp->display, disp->bitmap_win->win,
				disp->bgcolor);
	}
      if (disp->show_bitmaps) disp->show_bitmaps = FALSE;
      else disp->show_bitmaps = TRUE;
      if (disp->show_bitmaps) strcpy (disp->message, "Bitmaps Shown Now");
      else strcpy (disp->message, "Bitmaps Not Shown Now");
      /* Send an expose event to the bitmap window, so it re-draws instantly */
      XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
      handle_message (disp, TRUE);
      flash_area (x,y,disp);
      break;
    }
  case 21: /* Toggle Grid */
    {
      flash_area (x,y,disp);
      if (disp->grid_on) disp->grid_on = FALSE;
      else disp->grid_on = TRUE;
      if (disp->grid_on) strcpy (disp->message, "Grid On Now");
      else strcpy (disp->message, "Grid Off Now");
      /* Send an expose event to the bitmap window, so it re-draws instantly */
      XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
      handle_message (disp, TRUE);
      flash_area (x,y,disp);
      break;
    }
  case 22: /* Transfer memory to bitmap */
    {
      flash_area (x,y, disp);
      if (transfer_memory_to_pixmaps (disp, TRUE))
	XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
      flash_area (x,y, disp);
      break;
    }
  case 23: /* Transfer memory to mask */
    {
      flash_area (x,y, disp);
      if (transfer_memory_to_pixmaps (disp, FALSE))
	XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
      flash_area (x,y, disp);
      break;
    }
  case 24: /* Transfer bitmap to memory */
    {	
      flash_area (x,y,disp);
      if (!disp->current_edit_no)
	{
	  strcpy (disp->message, "Edit A Person First.");
	  handle_message (disp, TRUE);
	  flash_area (x,y,disp);
	  return;
	}
      else
	{
	  temp = (PersonLink *) 
	    return_user_link (disp, disp->current_edit_no);
	  if (temp->person->bitmap_saved) 
	    {
	      disp->memory_bitmap = XCreatePixmapFromBitmapData
		(disp->display,
                 disp->bitmap_win->win, temp->person->bitmap,
                 WIDTH, HEIGHT, disp->fgcolor, disp->bgcolor,
		 DefaultDepth(disp->display, DefaultScreen(disp->display)));
	      disp->memory_bm = TRUE;
	      strcpy (disp->message, "Bitmap Copied To Memory");
	      handle_message (disp, TRUE);
	    }
	  else
	    {
	      strcpy (disp->message, "This Person Has No Bitmap");
	      handle_message (disp, TRUE);
	      flash_area (x,y,disp);
	      return;
	    }
	  flash_area (x,y,disp);
	  break;
	}
    }
    
  case 25: /* Tranfer Mask to memory */
    {	
      flash_area (x,y,disp);
      if (!disp->current_edit_no)
	{
	  strcpy (disp->message, "Edit A Person First.");
	  handle_message (disp, TRUE);
	  flash_area (x,y,disp);
	  return;
	}
      else
	{
	  temp = (PersonLink *)
	    return_user_link (disp, disp->current_edit_no);
	  if (temp->person->bitmap_saved) 
	    {
	      disp->memory_mask = XCreatePixmapFromBitmapData
		(disp->display,
                 disp->bitmap_win->win, temp->person->mask,
                 WIDTH, HEIGHT, disp->fgcolor, disp->bgcolor,
		 DefaultDepth(disp->display, DefaultScreen(disp->display)));
	      disp->memory_m = TRUE;
	      strcpy (disp->message, "Mask Copied To Memory");
	      handle_message (disp, TRUE);
	    }
	  else
	    {
	      strcpy (disp->message, "This Person Has No Mask");
	      handle_message (disp, TRUE);
	      flash_area (x,y,disp);
	      return;
	    }
	  flash_area (x,y,disp);
	  break;
	}
    }
    
  case 26 : /* Edit the Bitmap using bitmap system call */
    {
      flash_area (x,y,disp);
      if (disp->current_edit_no)
	{
	  temp = (PersonLink *)
	    return_user_link (disp, disp->current_edit_no);
	  if (!temp->person->bitmap_saved)
	    {
	      strcpy (disp->message, "This Person Has No Bitmap");
	      handle_message (disp, TRUE);
	    }
	  else
	    {
	      temp_map = XCreatePixmapFromBitmapData
		(disp->display, disp->bitmap_win->win,
		 temp->person->bitmap,
		 WIDTH2, HEIGHT, disp->fgcolor,
		 disp->bgcolor,
		 DefaultDepth(disp->display, DefaultScreen(disp->display)));
	      if (edit_pixmap (disp, &temp_map))
		{
		  TransferPixmapToData (temp_map,
					temp->person->bitmap, disp);
		  XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
		}
	      else
		{
		  strcpy (disp->message, "Error In Editing Bitmap");
		  handle_message (disp, TRUE);
		}
	    }
	}
      flash_area (x,y, disp);
      break;
    }
    
  case 27 : /* Edit the Bitmap using bitmap system call */
    {
      flash_area (x,y,disp);
      if (disp->current_edit_no)
	{
	  temp = (PersonLink *)
	    return_user_link (disp, disp->current_edit_no);
	  if (!temp->person->bitmap_saved)
	    {
	      strcpy (disp->message, "This Person Has No Mask");
	      handle_message (disp, TRUE);
	    }
	  else
	    {
	      temp_map = XCreatePixmapFromBitmapData
		(disp->display, disp->bitmap_win->win,
		 temp->person->mask,
		 WIDTH2, HEIGHT, disp->fgcolor,
		 disp->bgcolor,
		 DefaultDepth(disp->display, DefaultScreen(disp->display)));
	      if (edit_pixmap (disp, &temp_map))
		{
		  TransferPixmapToData (temp_map,
					temp->person->mask, disp);
		  XClearArea (disp->display, disp->bitmap_win->win,0,0,1,1,True);
		}
	      else
		{
		  strcpy (disp->message, "Error In Editing Mask");
		  handle_message (disp, TRUE);
		}
	    }
	}
      flash_area (x,y, disp);
      break;
    }
    
  case 28 : /* Genocide Mode Toggle */
    {
      flash_area (x,y,disp);
      if (disp->genocide_mode)
	{
	  strcpy (disp->message, "Genocide Mode Disabled");
	  disp->genocide_mode = FALSE;
	}
      else {
	strcpy (disp->message, "Genocide Mode Enabled");
	disp->genocide_mode = TRUE;
      }
      handle_message (disp, TRUE);
      flash_area (x,y,disp);
      break;
    }
    
  default: /* Error */
    {
      strcpy (disp->message, "Improper Field Selection");
      handle_message (disp, TRUE);
      return;
    }
  }
  XClearArea (disp->display, disp->win, 0,0,1,1,True);
  draw_attr_fields (disp, file);
  clear_text_line(disp);
}

void redraw_attr_win (disp)
     disp_fields	disp;
{
  PersonLink *link;
  int counter;
  
  setup_attr(disp);
  if (disp->current_edit_no)
    if (person_is_in_scroll_fields (disp, disp->current_edit_no))
      {
	counter = disp->current_edit_no - 1;
	link = disp->file;
	while (link && counter) {link = link->next; counter--;}
	draw_attr_fields (disp, link);
      }
}

