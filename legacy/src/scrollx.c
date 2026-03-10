#include <stdio.h>
#include <stdlib.h>
#include "standardx.h"
#include "lib.h"
#include "def.h"
#include "password.h"
#include "scrollx.h" /* This one has to be last */
  
  int display_users (disp, first_number, return_last_number)
disp_fields	disp;
int		first_number;
boolean		return_last_number;
{
  int counter,temp,y;
  PersonLink *file;
  
  file = disp->file;
  
  XClearArea (disp->display, disp->win, SCROLL_X, SCROLL_Y - 20,
	      SCROLL_WIDTH - BITM_WINDOW_BORDER, SCROLL_HEIGHT + 2, False);
  y = SCROLL_Y;
  counter = 1;
  while ((counter < first_number) && (file)) {file = file->next; counter++;}
  if (!file) {
    if (return_last_number) return counter; else return 0;
  }
  for (temp = counter; (file && ((temp + Y_DIVISIONS) > counter)); counter++,
       file = file->next, y += SEP_Y)
    draw_scroll_line (disp, file, counter,y);
  
  if (return_last_number)
    {
      while (file) 
	{
	  counter++;
	  file = file->next; 
	} 
      return (--counter);
    }
  else return;
}

boolean scroll_down_one (disp, curr_top_num)
     disp_fields	disp;
     int		curr_top_num;
{
  PersonLink	*person;
  int		counter = 1;
  
  /* Forget it if there is no one to edit */
  if (!curr_top_num) return FALSE;
  /* Also forget it if we want to scroll down when we are looking at #1 */
  if (curr_top_num == 1) return FALSE;
  person = disp->file;
  
  /* Find the one right below the current one */
  while ((person) && (counter < curr_top_num - 1)) {counter++; person = person->next;}
  /* If it doesn't exist we can't scroll */
  if (!person) return FALSE;
  
  /* Scroll the area down one level */
  XCopyArea (disp->display, disp->win, disp->win, disp->regular_gc,
	     SCROLL_X, SCROLL_Y - SEP_Y , SCROLL_WIDTH -
	     BITM_WINDOW_BORDER, SCROLL_HEIGHT - SEP_Y + 1, SCROLL_X, SCROLL_Y);
  /* Blank the top line */
  XClearArea (disp->display, disp->win, SCROLL_X, SCROLL_Y - SEP_Y - 1,
	      SCROLL_WIDTH - BITM_WINDOW_BORDER, SEP_Y + 1, FALSE);
  
  /* Print out the top line */
  draw_scroll_line (disp, person, counter, SCROLL_Y);
  
  return TRUE;
}

boolean scroll_up_one (disp, curr_top_num)
     disp_fields	disp;
     int		curr_top_num;
{
  PersonLink	*person;
  int		counter = 1,end_num;
  boolean		bottom_exists;
  
  /* Forget it if there is no one to edit */
  if (!curr_top_num) return FALSE;
  
  person = disp->file;
  /* Skip to the next person */
  while ((person) && (counter < (curr_top_num + 1)))
    {counter++; person = person->next;}
  
  if (!person) return FALSE;
  
  end_num = counter;
  while ((person) && (end_num < (curr_top_num + Y_DIVISIONS)))
    {end_num++; person = person->next;}
  if (person) bottom_exists = TRUE; else bottom_exists= FALSE;
  
  /* Scroll the area up one */
  XCopyArea (disp->display, disp->win, disp->win, disp->regular_gc,
	     SCROLL_X, SCROLL_Y , SCROLL_WIDTH -
	     BITM_WINDOW_BORDER,SCROLL_HEIGHT - SEP_Y + 1, SCROLL_X , SCROLL_Y - SEP_Y);
  
  /* Blank the bottom line */
  XClearArea (disp->display, disp->win, SCROLL_X, 
	      SCROLL_Y + ((Y_DIVISIONS - 2) * SEP_Y) + FONT_ADJUST,
	      SCROLL_WIDTH - WINDOW_BORDER, SEP_Y + 10, FALSE);
  
  /* If there is another line below the scroll area that needs to be copied
     up */
  
  if (bottom_exists) draw_scroll_line (disp, person, end_num,(SCROLL_Y +
							      ((Y_DIVISIONS - 1) * SEP_Y)));
  return TRUE;
}

draw_scroll_seg (disp, x, y, text, textlen)
     disp_fields	disp;
     int		x,y,textlen;
     char		*text;
{
  XSetFont (disp->display, disp->regular_gc, disp->tinyfont->fid);
  XDrawString (disp->display, disp->win, disp->regular_gc,
	       x, y, text, textlen);
}

draw_scroll_line (disp, char_record,counter,y)
     disp_fields	disp;
     PersonLink	*char_record;
     int		counter,y;
     
{
  sprintf (disp->line,"%d", counter);
  /* Number */
  draw_scroll_seg (disp, X_NUMBER, y, disp->line,
		   strlen (disp->line));
  /* Name */
  strncpy (disp->line, char_record->person->name, NAME_MAX_SIZE);
  disp->line[NAME_MAX_SIZE] = '\0';
  disp->line[strlen (char_record->person->name)] = '\0';
  draw_scroll_seg (disp,X_NAME, y, disp->line,
		   strlen (disp->line));
  
  if (!disp->genocide_mode)
    sprintf (disp->line,"%d", char_record->person->ExpPts);
  else 
    {  
      strcpy (disp->line2, char_record->person->strdate);
      strncpy (disp->line, disp->line2, 8);
      disp->line[8] = '\0';
    }
  
  /* Exp pts */
  strncpy (disp->line2, disp->line, EXP_PT_MAX_SIZE);
  disp->line2[EXP_PT_MAX_SIZE] = '\0';
  disp->line2[strlen (disp->line)] = '\0';
  draw_scroll_seg (disp,X_EXP_PTS, y, disp->line2,
		   strlen (disp->line2));
  /* Login */
  strncpy (disp->line, char_record->person->login, LOGIN_NAME_MAX_SIZE);
  disp->line[LOGIN_NAME_MAX_SIZE] = '\0';
  disp->line[strlen (char_record->person->login)] = '\0';
  draw_scroll_seg (disp,X_LOGIN, y, disp->line, 
		   strlen (disp->line));
  /* Host */
  strncpy (disp->line, char_record->person->host, HOST_MAX_SIZE);
  disp->line[HOST_MAX_SIZE] = '\0';
  disp->line[strlen (char_record->person->host)] = '\0';
  draw_scroll_seg (disp,X_HOST, y, disp->line,
		   strlen (disp->line));
  /* Deity */
  if (char_record->person->deity)
    {
      XDrawLine (disp->display, disp->win, disp->regular_gc,
		 X_DEITY, y - DEITY_SCALER, X_DEITY, y + DEITY_HEIGHT - DEITY_SCALER);
      
      XDrawLine (disp->display, disp->win, disp->regular_gc,
		 X_DEITY - SIZE_DEITY, y + Y_DEITY_SEP - DEITY_SCALER,
		 X_DEITY + SIZE_DEITY, y + Y_DEITY_SEP - DEITY_SCALER);
    }
  /* Bitmap */
  if (char_record->person->bitmap_saved)
    XCopyArea (disp->display, disp->pict, disp->win, disp->regular_gc,
	       0,0,16,16, X_BITMAP , y - 11);
}

boolean cursor_is_in_scroll_field (x, y)
{
  if ((x > (SLID_WINDOW_X + SLID_WINDOW_WIDTH))
      && (x < BITM_WINDOW_WIDTH)
      && (y > (SCROLL_Y - SEP_Y))
      && (y < (SCROLL_Y - SEP_Y + SLID_WINDOW_HEIGHT)))
    return TRUE; else return FALSE;
}

boolean person_is_in_scroll_fields (disp,person_no)
     disp_fields	disp;
     int person_no;
{
  if ((person_no <= disp->MaxUsers) && (person_no >= disp->user) &&
      (person_no <= (disp->user + Y_DIVISIONS - 1))) return TRUE;
}

int scroll_area (disp, x, y)
     disp_fields disp;
     int x,y;
{
  int area = 0;
  y -= FONT_ADJUST;
  while (y >= SLID_WINDOW_Y)
    {
      area++;
      y -= SEP_Y;
    } 
  return area;
}

void invert_flash_flag (disp)
     disp_fields disp;
{
  if (disp->scroll_flash) 
    disp->scroll_flash = FALSE; else disp->scroll_flash = TRUE;
}

void flash_scroll_area (disp,area)
     disp_fields disp;
     int area;
{
  int x,y,width,height;
  
  x = SLID_WINDOW_X + SLID_WINDOW_WIDTH;
  y = SLID_WINDOW_Y + (area - 1) * SEP_Y + FONT_ADJUST;
  width = SCROLL_WIDTH + 2;
  height = SEP_Y;
  
  XCopyArea (disp->display, disp->win,disp->win,
	     disp->inversion_gc, x,y + 2,width,height - 2,x,y + 2);
  
  invert_flash_flag (disp);
}

void handle_scroll_events (disp, x, y)
     disp_fields	disp;
     int x,y;
{
  int area,person_no;
  PersonLink	*file;
  
  area = scroll_area (disp, x, y);
  if ((!disp->current_edit_no) && ((disp->user + area - 1) > disp->MaxUsers)) return;
  if (disp->current_edit_no)
    {
      /* Special cases */
      /* if it is still on the screen, don't inverse it */
      if (!disp->genocide_mode)
	if (disp->current_edit_no == (disp->user + area - 1)) return;
      /* If we are below the maximum number of users, don't inverse */
      if ((disp->user + area - 1) > disp->MaxUsers) return;
      /* If we are moving to a new area, unflash the old one */
      if ((!disp->genocide_mode) && (disp->current_edit_no !=
				     (disp->user + area - 1)))
	if ((!disp->high_light_off_screen)  && (disp->current_edit_no))
	  flash_scroll_area (disp, disp->current_edit_no - disp->user + 1);
      /* If we were once off the screen, we are not anymore ! */
      if (disp->high_light_off_screen) 
	disp->high_light_off_screen = FALSE;
      /* If We are in genocide mode, Kill! */
      if (disp->genocide_mode)
	{
	  disp->current_edit_no = disp->user + area - 1;
	  disp->genocide_kluge = TRUE;
	  handle_attributes_events (0,0,disp);
	}
    }
  /* Flash the new area */
  disp->current_edit_no = person_no = disp->user + area - 1;
  /* Send an expose event to the bitmap window */
  XClearArea (disp->display, disp->bitmap_win->win, 0,0,1,1,True);
  file = disp->file;
  flash_scroll_area (disp,area);
  while ((person_no > 1) && (file)) {file = file->next; person_no--;}
  if (!file) {disp->current_edit_no = 0; return;}
  draw_attr_fields (disp, file);
  disp->current_edit = file;
}

PersonLink *return_user_link (disp, number)
     disp_fields	disp;
     int number;
{
  PersonLink *return_value;
  return_value = disp->file;
  while (number > 1)
    { 
      number--;
      if (return_value->next == NULL) return return_value;
      return_value = return_value->next;
    }
  return return_value;
}
