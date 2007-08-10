/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick gamelog program
 * 
 * This file is part of Chadwick, tools for baseball play-by-play and stats
 * Copyright (C) 2006-2007, 
 *    Ted Turocy (drarbiter@gmail.com)
 *    Sean Forman (sean-forman@baseball-reference.com)
 *    XML Team Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */ 

/*
 * This variant on cwgame generates fields that match the Retrosheet
 * gamelog specification.  Any fields that cannot be generated automatically
 * (e.g., team game number, suspension and forfeit information) are
 * left blank.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cwlib/chadwick.h"

/*************************************************************************
 * Global variables for command-line options
 *************************************************************************/

extern int ascii;

/* Fields to display (-f) */
/* Note that our field numbers start at zero, while the Retrosheet gamelog
 * spec starts at field 1. */
int fields[161] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1
};

int max_field = 161;

char program_name[20] = "cwlog";


/*************************************************************************
 * Utility routines to compute day of week
 *************************************************************************/

/*
 * This function converts month, date, and year to the day of the week,
 * returning an integer between 0 and 6, inclusive, with 0 indicating Sunday.
 * This function should work for any year from 1901 to 2099.
*/
static int
get_day_of_week(int month, int date, int year) 
{
  /*
   * The base year is 1900.  Since 1 January 1900 was a Monday, we get the
   * following keys for the months.
   */
  static int month_keys[12] = { 1, 4, 4, 0, 2, 5, 0, 3, 6, 1, 4, 6 };

  int day;

  day = (year - 1900) + (year - 1900) / 4 + month_keys[month - 1] + date - 1;
  /* The above counts the leap day even if it occurs later in the year */
  if ((year > 1900) && (year % 4 == 0) && (month < 2)) {
    day--;
  }
  day %= 7;
  return day;
}

/*************************************************************************
 * Utility routines to convert strings into numeric codes
 *************************************************************************/

typedef struct cwlog_lookup_struct {
  int code;
  char *text;
} CWLookup;

static int
cwlog_lookup(char *text, CWLookup *table)
{
  int i;
  
  if (text == NULL) {
    return 0;
  }

  for (i = 0; table[i].code != -1; i++) {
    if (!strcmp(table[i].text, text)) {
      return table[i].code;
    }
  }

  return 0;
}

/* Auxiliary function: find a player's bio entry and print his
 * first and last names to the buffer
 */
int cwlog_find_player_name(char *buffer, char *player_id,
		     CWRoster *visitors, CWRoster *home)
{
  CWPlayer *bio = cw_roster_player_find(visitors, player_id);
  if (!bio) {
    bio = cw_roster_player_find(home, player_id);
  }
  if (bio) {
    return sprintf(buffer, "\"%s %s\"", bio->first_name, bio->last_name);
  }
  else {
    return sprintf(buffer, "");
  }
}


/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *, CWBoxscore *, 
			  CWRoster *, CWRoster *);

/*
 * preprocessor directive for conveniently declaring function signature
 */

#define DECLARE_FIELDFUNC(funcname) \
int funcname(char *buffer, CWGameIterator *gameiter, CWBoxscore *box, 	\
             CWRoster *visitors, CWRoster *home)

/* Field 0 */
DECLARE_FIELDFUNC(cwlog_date)
{
  char *date = cw_game_info_lookup(gameiter->game, "date");
  if (date) {
    return sprintf(buffer, "%c%c%c%c%c%c%c%c",
		   date[0], date[1], date[2], date[3],
		   date[5], date[6], date[8], date[9]);
  }
  else {
    return sprintf(buffer, "");
  }
}

/* Field 1 */
DECLARE_FIELDFUNC(cwlog_number)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%5d",
		 (tmp = cw_game_info_lookup(gameiter->game, "number")) ?
		 atoi(tmp) : 0);
}

/* Field 2 */
DECLARE_FIELDFUNC(cwlog_day_of_week)
{
  static char *day_names[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };

  int month, day, year;
  char *date = cw_game_info_lookup(gameiter->game, "date");

  if (date == NULL) {
    strcpy(buffer, "");
    return 0;
  }
  sscanf(date, "%d/%d/%d", &year, &month, &day);
  if (year > 0 && year <= 99) {
    /* assume that two-digit years are in the 20th century */
    year += 1900;
  }

  return sprintf(buffer, (ascii) ? "\"%s\"" : "%s",
		 day_names[get_day_of_week(month, day, year)]);
}

/* Field 3 */
DECLARE_FIELDFUNC(cwlog_visitors)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		 (tmp = cw_game_info_lookup(gameiter->game, "visteam")) ?
		 tmp : "");
}

/* Field 4 */
DECLARE_FIELDFUNC(cwlog_visitors_league)
{
  return sprintf(buffer, "\"%s\"", visitors->league);
}

/* Field 5 */
DECLARE_FIELDFUNC(cwlog_visitors_game)
{
  return sprintf(buffer, "");
}

/* Field 6 */
DECLARE_FIELDFUNC(cwlog_home)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s", 
		 (tmp = cw_game_info_lookup(gameiter->game, "hometeam")) ?
		 tmp : "");
}

/* Field 7 */
DECLARE_FIELDFUNC(cwlog_home_league)
{
  return sprintf(buffer, "\"%s\"", home->league);
}

/* Field 8 */
DECLARE_FIELDFUNC(cwlog_home_game)
{
  return sprintf(buffer, "");
}

/* Field 9 */
DECLARE_FIELDFUNC(cwlog_visitors_score)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->score[0]);
}

/* Field 10 */
DECLARE_FIELDFUNC(cwlog_home_score)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->score[1]);
}

/* Field 11 */
DECLARE_FIELDFUNC(cwlog_length_outs)
{
  int t, outs = 0;

  for (t = 0; t <= 1; t++) {
    CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, t);

    while (pitcher != NULL) {
      outs += pitcher->pitching->outs;
      pitcher = pitcher->next;
    }
  }
  
  return sprintf(buffer, "%d", outs);
}

/* Field 12 */
DECLARE_FIELDFUNC(cwlog_day_night)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 
		 (tmp = cw_game_info_lookup(gameiter->game, "daynight")) ?
		 ((!strcmp(tmp, "night")) ? 'N' : 'D') : 'D');
}

/* Field 13 */
DECLARE_FIELDFUNC(cwlog_completion_info)
{
  return sprintf(buffer, "");
}

/* Field 14 */
DECLARE_FIELDFUNC(cwlog_forfeit_info)
{
  return sprintf(buffer, "");
}

/* Field 15 */
DECLARE_FIELDFUNC(cwlog_protest_info)
{
  return sprintf(buffer, "");
}

/* Field 16 */
DECLARE_FIELDFUNC(cwlog_site)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-5s",
		 (tmp = cw_game_info_lookup(gameiter->game, "site")) ?
		 tmp : "");
}

/* Field 17 */
DECLARE_FIELDFUNC(cwlog_attendance)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%5d", 
		 (tmp = cw_game_info_lookup(gameiter->game, "attendance")) ? 
		 atoi(tmp) : 0);
}

/* Field 18 */
DECLARE_FIELDFUNC(cwlog_time_of_game)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 (tmp = cw_game_info_lookup(gameiter->game, "timeofgame")) ? 
		 atoi(tmp) : 0);
}

/* Field 19 */
DECLARE_FIELDFUNC(cwlog_visitors_line)
{
  int i;
  char *c = buffer;
  
  for (i = 1; i < 50; i++) {
    if (box->linescore[i][0] < 0 &&
	box->linescore[i][1] < 0) {
      break;
    }

    if (box->linescore[i][0] >= 10) {
      c += sprintf(c, "(%d)", box->linescore[i][0]);
    }
    else if (box->linescore[i][0] >= 0) {
      c += sprintf(c, "%d", box->linescore[i][0]);
    }
    else {
      c += sprintf(c, "x");
    }
  }

  return c - buffer;
}

/* Field 20 */
DECLARE_FIELDFUNC(cwlog_home_line)
{
  int i;
  char *c = buffer;
  
  for (i = 1; i < 50; i++) {
    if (box->linescore[i][0] < 0 &&
	box->linescore[i][1] < 0) {
      break;
    }

    if (box->linescore[i][1] >= 10) {
      c += sprintf(c, "(%d)", box->linescore[i][1]);
    }
    else if (box->linescore[i][1] >= 0) {
      c += sprintf(c, "%d", box->linescore[i][1]);
    }
    else {
      c += sprintf(c, "x");
    }
  }

  return c - buffer;
}

/* Field 21 */
DECLARE_FIELDFUNC(cwlog_visitors_ab)
{
  int slot, ab = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      ab += player->batting->ab;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", ab);
}

/* Field 22 */
DECLARE_FIELDFUNC(cwlog_visitors_hits)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->hits[0]);
}

/* Field 23 */
DECLARE_FIELDFUNC(cwlog_visitors_2b)
{
  int slot, b2 = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      b2 += player->batting->b2;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", b2);
}

/* Field 24 */
DECLARE_FIELDFUNC(cwlog_visitors_3b)
{
  int slot, b3 = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      b3 += player->batting->b3;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", b3);
}

/* Field 25 */
DECLARE_FIELDFUNC(cwlog_visitors_hr)
{
  int slot, hr = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      hr += player->batting->hr;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", hr);
}

/* Field 26 */
DECLARE_FIELDFUNC(cwlog_visitors_bi)
{
  int slot, bi = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      bi += player->batting->bi;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", bi);
}

/* Field 27 */
DECLARE_FIELDFUNC(cwlog_visitors_sh)
{
  int slot, sh = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      sh += player->batting->sh;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", sh);
}

/* Field 28 */
DECLARE_FIELDFUNC(cwlog_visitors_sf)
{
  int slot, sf = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      sf += player->batting->sf;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", sf);
}

/* Field 29 */
DECLARE_FIELDFUNC(cwlog_visitors_hp)
{
  int slot, hp = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      hp += player->batting->hp;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", hp);
}

/* Field 30 */
DECLARE_FIELDFUNC(cwlog_visitors_bb)
{
  int slot, bb = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      bb += player->batting->bb;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", bb);
}

/* Field 31 */
DECLARE_FIELDFUNC(cwlog_visitors_ibb)
{
  int slot, ibb = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      ibb += player->batting->ibb;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", ibb);
}

/* Field 32 */
DECLARE_FIELDFUNC(cwlog_visitors_so)
{
  int slot, so = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      so += player->batting->so;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", so);
}

/* Field 33 */
DECLARE_FIELDFUNC(cwlog_visitors_sb)
{
  int slot, sb = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      sb += player->batting->sb;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", sb);
}

/* Field 34 */
DECLARE_FIELDFUNC(cwlog_visitors_cs)
{
  int slot, cs = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      cs += player->batting->cs;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", cs);
}

/* Field 35 */
DECLARE_FIELDFUNC(cwlog_visitors_gdp)
{
  int slot, gdp = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      gdp += player->batting->gdp;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", gdp);
}

/* Field 36 */
DECLARE_FIELDFUNC(cwlog_visitors_xi)
{
  int slot, xi = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      xi += player->batting->xi;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", xi);
}

/* Field 37 */
DECLARE_FIELDFUNC(cwlog_visitors_lob)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 cw_gamestate_left_on_base(gameiter->state, 0));
}

/* Field 38 */
DECLARE_FIELDFUNC(cwlog_visitors_pitchers)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 0);
  int i = 0;

  for (; pitcher != NULL; pitcher = pitcher->next, i++);
  return sprintf(buffer, "%d", i);
}

/* Field 39 */
DECLARE_FIELDFUNC(cwlog_visitors_er)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 0);
  int er = 0;

  while (pitcher != NULL) {
    er += pitcher->pitching->er;
    pitcher = pitcher->next;
  }
  
  return sprintf(buffer, "%d", er);
}

/* Field 40 */
DECLARE_FIELDFUNC(cwlog_visitors_ter)
{
  return sprintf(buffer, "%d", box->er[0]);
}

/* Field 41 */
DECLARE_FIELDFUNC(cwlog_visitors_wp)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 0);
  int wp = 0;

  while (pitcher != NULL) {
    wp += pitcher->pitching->wp;
    pitcher = pitcher->next;
  }
  
  return sprintf(buffer, "%d", wp);
}

/* Field 42 */
DECLARE_FIELDFUNC(cwlog_visitors_bk)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 0);
  int bk = 0;

  while (pitcher != NULL) {
    bk += pitcher->pitching->bk;
    pitcher = pitcher->next;
  }
  
  return sprintf(buffer, "%d", bk);
}

/* Field 43 */
DECLARE_FIELDFUNC(cwlog_visitors_po)
{
  int slot, pos, po = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  po += player->fielding[pos]->po;
	}
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", po);
}

/* Field 44 */
DECLARE_FIELDFUNC(cwlog_visitors_a)
{
  int slot, pos, a = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  a += player->fielding[pos]->a;
	}
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", a);
}

/* Field 45 */
DECLARE_FIELDFUNC(cwlog_visitors_e)
{
  int slot, pos, e = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  e += player->fielding[pos]->e;
	}
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", e);
}

/* Field 46 */
DECLARE_FIELDFUNC(cwlog_visitors_pb)
{
  int slot, pb = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 0, slot);
    
    while (player != NULL) {
      if (player->fielding[2] != NULL) {
	pb += player->fielding[2]->pb;
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", pb);
}

/* Field 47 */
DECLARE_FIELDFUNC(cwlog_visitors_dp)
{
  return sprintf(buffer, "%d", box->dp[0]);
}

/* Field 48 */
DECLARE_FIELDFUNC(cwlog_visitors_tp)
{
  return sprintf(buffer, "%d", box->tp[0]);
}

/* Field 49 */
DECLARE_FIELDFUNC(cwlog_home_ab)
{
  int slot, ab = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      ab += player->batting->ab;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", ab);
}

/* Field 50 */
DECLARE_FIELDFUNC(cwlog_home_hits)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->hits[1]);
}

/* Field 51 */
DECLARE_FIELDFUNC(cwlog_home_2b)
{
  int slot, b2 = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      b2 += player->batting->b2;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", b2);
}

/* Field 52 */
DECLARE_FIELDFUNC(cwlog_home_3b)
{
  int slot, b3 = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      b3 += player->batting->b3;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", b3);
}

/* Field 53 */
DECLARE_FIELDFUNC(cwlog_home_hr)
{
  int slot, hr = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      hr += player->batting->hr;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", hr);
}

/* Field 54 */
DECLARE_FIELDFUNC(cwlog_home_bi)
{
  int slot, bi = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      bi += player->batting->bi;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", bi);
}

/* Field 55 */
DECLARE_FIELDFUNC(cwlog_home_sh)
{
  int slot, sh = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      sh += player->batting->sh;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", sh);
}

/* Field 56 */
DECLARE_FIELDFUNC(cwlog_home_sf)
{
  int slot, sf = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      sf += player->batting->sf;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", sf);
}

/* Field 57 */
DECLARE_FIELDFUNC(cwlog_home_hp)
{
  int slot, hp = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      hp += player->batting->hp;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", hp);
}

/* Field 58 */
DECLARE_FIELDFUNC(cwlog_home_bb)
{
  int slot, bb = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      bb += player->batting->bb;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", bb);
}

/* Field 59 */
DECLARE_FIELDFUNC(cwlog_home_ibb)
{
  int slot, ibb = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      ibb += player->batting->ibb;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", ibb);
}

/* Field 60 */
DECLARE_FIELDFUNC(cwlog_home_so)
{
  int slot, so = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      so += player->batting->so;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", so);
}

/* Field 61 */
DECLARE_FIELDFUNC(cwlog_home_sb)
{
  int slot, sb = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      sb += player->batting->sb;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", sb);
}

/* Field 62 */
DECLARE_FIELDFUNC(cwlog_home_cs)
{
  int slot, cs = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      cs += player->batting->cs;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", cs);
}

/* Field 63 */
DECLARE_FIELDFUNC(cwlog_home_gdp)
{
  int slot, gdp = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      gdp += player->batting->gdp;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", gdp);
}

/* Field 64 */
DECLARE_FIELDFUNC(cwlog_home_xi)
{
  int slot, xi = 0;

  for (slot = 1; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      xi += player->batting->xi;
      player = player->next;
    }
  }
  
  return sprintf(buffer, "%d", xi);
}

/* Field 65 */
DECLARE_FIELDFUNC(cwlog_home_lob)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 cw_gamestate_left_on_base(gameiter->state, 1));
}

/* Field 66 */
DECLARE_FIELDFUNC(cwlog_home_pitchers)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 1);
  int i = 0;

  for (; pitcher != NULL; pitcher = pitcher->next, i++);
  return sprintf(buffer, "%d", i);
}

/* Field 67 */
DECLARE_FIELDFUNC(cwlog_home_er)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 1);
  int er = 0;

  while (pitcher != NULL) {
    er += pitcher->pitching->er;
    pitcher = pitcher->next;
  }
  
  return sprintf(buffer, "%d", er);
}

/* Field 68 */
DECLARE_FIELDFUNC(cwlog_home_ter)
{
  return sprintf(buffer, "%d", box->er[1]);
}

/* Field 69 */
DECLARE_FIELDFUNC(cwlog_home_wp)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 1);
  int wp = 0;

  while (pitcher != NULL) {
    wp += pitcher->pitching->wp;
    pitcher = pitcher->next;
  }
  
  return sprintf(buffer, "%d", wp);
}

/* Field 70 */
DECLARE_FIELDFUNC(cwlog_home_bk)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 1);
  int bk = 0;

  while (pitcher != NULL) {
    bk += pitcher->pitching->bk;
    pitcher = pitcher->next;
  }
  
  return sprintf(buffer, "%d", bk);
}

/* Field 71 */
DECLARE_FIELDFUNC(cwlog_home_po)
{
  int slot, pos, po = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  po += player->fielding[pos]->po;
	}
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", po);
}

/* Field 72 */
DECLARE_FIELDFUNC(cwlog_home_a)
{
  int slot, pos, a = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  a += player->fielding[pos]->a;
	}
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", a);
}

/* Field 73 */
DECLARE_FIELDFUNC(cwlog_home_e)
{
  int slot, pos, e = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  e += player->fielding[pos]->e;
	}
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", e);
}

/* Field 74 */
DECLARE_FIELDFUNC(cwlog_home_pb)
{
  int slot, pb = 0;

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(box, 1, slot);
    
    while (player != NULL) {
      if (player->fielding[2] != NULL) {
	pb += player->fielding[2]->pb;
      }
      player = player->next;
    }
  }

  return sprintf(buffer, "%d", pb);
}

/* Field 75 */
DECLARE_FIELDFUNC(cwlog_home_dp)
{
  return sprintf(buffer, "%d", box->dp[1]);
}

/* Field 76 */
DECLARE_FIELDFUNC(cwlog_home_tp)
{
  return sprintf(buffer, "%d", box->tp[1]);
}

/* Field 77 */
DECLARE_FIELDFUNC(cwlog_umpire_home_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "umphome")) ?
		 tmp : "");
}

/* Field 78 */
DECLARE_FIELDFUNC(cwlog_umpire_home_name)
{
  return sprintf(buffer, "");
}

/* Field 79 */
DECLARE_FIELDFUNC(cwlog_umpire_1b_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "ump1b")) ? 
		 tmp : "");
}

/* Field 80 */
DECLARE_FIELDFUNC(cwlog_umpire_1b_name)
{
  return sprintf(buffer, "");
}

/* Field 81 */
DECLARE_FIELDFUNC(cwlog_umpire_2b_id)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "ump2b");
  if (tmp && strcmp(tmp, "") && strcmp(tmp, "(none)")) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s", tmp);
  }
  else {
    return sprintf(buffer, "");
  }
}

/* Field 82 */
DECLARE_FIELDFUNC(cwlog_umpire_2b_name)
{
  return sprintf(buffer, "");
}

/* Field 83 */
DECLARE_FIELDFUNC(cwlog_umpire_3b_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "ump3b")) ? 
		 tmp : "");
}

/* Field 84 */
DECLARE_FIELDFUNC(cwlog_umpire_3b_name)
{
  return sprintf(buffer, "");
}


/* Field 85 */
DECLARE_FIELDFUNC(cwlog_umpire_lf_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "umplf")) ?
		 tmp : "");
}

/* Field 86 */
DECLARE_FIELDFUNC(cwlog_umpire_lf_name)
{
  return sprintf(buffer, "");
}


/* Field 87 */
DECLARE_FIELDFUNC(cwlog_umpire_rf_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "umprf")) ?
		 tmp : "");
}

/* Field 88 */
DECLARE_FIELDFUNC(cwlog_umpire_rf_name)
{
  return sprintf(buffer, "");
}

/* Field 89 */
DECLARE_FIELDFUNC(cwlog_visitors_manager_id)
{
  return sprintf(buffer, "");
}

/* Field 90 */
DECLARE_FIELDFUNC(cwlog_visitors_manager_name)
{
  return sprintf(buffer, "");
}

/* Field 91 */
DECLARE_FIELDFUNC(cwlog_home_manager_id)
{
  return sprintf(buffer, "");
}

/* Field 92 */
DECLARE_FIELDFUNC(cwlog_home_manager_name)
{
  return sprintf(buffer, "");
}

/* Field 93 */
DECLARE_FIELDFUNC(cwlog_winning_pitcher_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 (tmp = cw_game_info_lookup(gameiter->game, "wp")) ? 
		 tmp : "");
}

/* Field 94 */
DECLARE_FIELDFUNC(cwlog_winning_pitcher_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "wp");
  if (tmp && strcmp(tmp, "")) {
    return cwlog_find_player_name(buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 95 */
DECLARE_FIELDFUNC(cwlog_losing_pitcher_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (tmp = cw_game_info_lookup(gameiter->game, "lp")) ? 
		 tmp : "");
}

/* Field 96 */
DECLARE_FIELDFUNC(cwlog_losing_pitcher_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "lp");
  if (tmp && strcmp(tmp, "")) {
    return cwlog_find_player_name(buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 97 */
DECLARE_FIELDFUNC(cwlog_save_pitcher_id)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (tmp = cw_game_info_lookup(gameiter->game, "save")) ?
		 tmp : "");
}

/* Field 98 */
DECLARE_FIELDFUNC(cwlog_save_pitcher_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "save");
  if (tmp && strcmp(tmp, "")) {
    return cwlog_find_player_name(buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 99 */
DECLARE_FIELDFUNC(cwlog_gwrbi_id)
{
  /*
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (tmp = cw_game_info_lookup(gameiter->game, "gwrbi")) ? 
		 tmp : "");
  */
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (gameiter->state->go_ahead_rbi) ? gameiter->state->go_ahead_rbi : "");
}

/* Field 100 */
DECLARE_FIELDFUNC(cwlog_gwrbi_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "gwrbi");
  if (tmp && strcmp(tmp, "")) {
    return cwlog_find_player_name(buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 101 */
DECLARE_FIELDFUNC(cwlog_visitors_pitcher_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_game_starter_find_by_position(gameiter->game,
						  0, 1)->player_id);
}

/* Field 102 */
DECLARE_FIELDFUNC(cwlog_visitors_pitcher_name)
{
  return cwlog_find_player_name(buffer,
				cw_game_starter_find_by_position(gameiter->game,
								 0, 1)->player_id,
				visitors, home);
}

/* Field 103 */
DECLARE_FIELDFUNC(cwlog_home_pitcher_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_game_starter_find_by_position(gameiter->game,
						  1, 1)->player_id);
}

/* Field 104 */
DECLARE_FIELDFUNC(cwlog_home_pitcher_name)
{
  return cwlog_find_player_name(buffer,
				cw_game_starter_find_by_position(gameiter->game,
								 1, 1)->player_id,
				visitors, home);
}

/* Field 159 */
DECLARE_FIELDFUNC(cwlog_additional_info)
{
  return sprintf(buffer, "");
}

/* Field 160 */
DECLARE_FIELDFUNC(cwlog_acquisition_info)
{
  return sprintf(buffer, "");
}


/* Fields for starting lineups */
int
cwlog_starting_player(char *buffer, CWGame *game, int team, int slot)
{
  CWAppearance *starter = cw_game_starter_find(game, team, slot);
  if (starter) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", starter->player_id); 
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

int
cwlog_starting_position(char *buffer, CWGame *game, int team, int slot)
{
  CWAppearance *starter = cw_game_starter_find(game, team, slot);
  if (starter) {
    return sprintf(buffer, "%d", starter->pos);
  }
  else {
    return sprintf(buffer, "0");
  }
  return 0;
}

static field_func function_ptrs[] = {
  cwlog_date,                    /* 0 */
  cwlog_number,                  /* 1 */
  cwlog_day_of_week,             /* 2 */
  cwlog_visitors,                /* 3 */
  cwlog_visitors_league,         /* 4 */
  cwlog_visitors_game,           /* 5 */
  cwlog_home,                    /* 6 */
  cwlog_home_league,             /* 7 */
  cwlog_home_game,               /* 8 */
  cwlog_visitors_score,          /* 9 */
  cwlog_home_score,              /* 10 */
  cwlog_length_outs,             /* 11 */
  cwlog_day_night,               /* 12 */
  cwlog_completion_info,         /* 13 */
  cwlog_forfeit_info,            /* 14 */
  cwlog_protest_info,            /* 15 */
  cwlog_site,                    /* 16 */
  cwlog_attendance,              /* 17 */
  cwlog_time_of_game,            /* 18 */
  cwlog_visitors_line,           /* 19 */
  cwlog_home_line,               /* 20 */
  cwlog_visitors_ab,             /* 21 */
  cwlog_visitors_hits,           /* 22 */
  cwlog_visitors_2b,             /* 23 */
  cwlog_visitors_3b,             /* 24 */
  cwlog_visitors_hr,             /* 25 */
  cwlog_visitors_bi,             /* 26 */
  cwlog_visitors_sh,             /* 27 */
  cwlog_visitors_sf,             /* 28 */
  cwlog_visitors_hp,             /* 29 */
  cwlog_visitors_bb,             /* 30 */
  cwlog_visitors_ibb,            /* 31 */
  cwlog_visitors_so,             /* 32 */
  cwlog_visitors_sb,             /* 33 */
  cwlog_visitors_cs,             /* 34 */
  cwlog_visitors_gdp,            /* 35 */
  cwlog_visitors_xi,             /* 36 */
  cwlog_visitors_lob,            /* 37 */
  cwlog_visitors_pitchers,       /* 38 */
  cwlog_visitors_er,             /* 39 */
  cwlog_visitors_ter,            /* 40 */
  cwlog_visitors_wp,             /* 41 */
  cwlog_visitors_bk,             /* 42 */
  cwlog_visitors_po,             /* 43 */
  cwlog_visitors_a,              /* 44 */
  cwlog_visitors_e,              /* 45 */
  cwlog_visitors_pb,             /* 46 */
  cwlog_visitors_dp,             /* 47 */
  cwlog_visitors_tp,             /* 48 */
  cwlog_home_ab,                 /* 49 */
  cwlog_home_hits,               /* 50 */
  cwlog_home_2b,                 /* 51 */
  cwlog_home_3b,                 /* 52 */
  cwlog_home_hr,                 /* 53 */
  cwlog_home_bi,                 /* 54 */
  cwlog_home_sh,                 /* 55 */
  cwlog_home_sf,                 /* 56 */
  cwlog_home_hp,                 /* 57 */
  cwlog_home_bb,                 /* 58 */
  cwlog_home_ibb,                /* 59 */
  cwlog_home_so,                 /* 60 */
  cwlog_home_sb,                 /* 61 */
  cwlog_home_cs,                 /* 62 */
  cwlog_home_gdp,                /* 63 */
  cwlog_home_xi,                 /* 64 */
  cwlog_home_lob,                /* 65 */
  cwlog_home_pitchers,           /* 66 */
  cwlog_home_er,                 /* 67 */
  cwlog_home_ter,                /* 68 */
  cwlog_home_wp,                 /* 69 */
  cwlog_home_bk,                 /* 70 */
  cwlog_home_po,                 /* 71 */
  cwlog_home_a,                  /* 72 */
  cwlog_home_e,                  /* 73 */
  cwlog_home_pb,                 /* 74 */
  cwlog_home_dp,                 /* 75 */
  cwlog_home_tp,                 /* 76 */
  cwlog_umpire_home_id,          /* 77 */
  cwlog_umpire_home_name,        /* 78 */
  cwlog_umpire_1b_id,            /* 79 */
  cwlog_umpire_1b_name,          /* 80 */
  cwlog_umpire_2b_id,            /* 81 */
  cwlog_umpire_2b_name,          /* 82 */
  cwlog_umpire_3b_id,            /* 83 */
  cwlog_umpire_3b_name,          /* 84 */
  cwlog_umpire_lf_id,            /* 85 */
  cwlog_umpire_lf_name,          /* 86 */
  cwlog_umpire_rf_id,            /* 87 */
  cwlog_umpire_rf_name,          /* 88 */
  cwlog_visitors_manager_id,     /* 89 */
  cwlog_visitors_manager_name,   /* 90 */
  cwlog_home_manager_id,         /* 91 */
  cwlog_home_manager_name,       /* 92 */
  cwlog_winning_pitcher_id,      /* 93 */
  cwlog_winning_pitcher_name,    /* 94 */
  cwlog_losing_pitcher_id,       /* 95 */
  cwlog_losing_pitcher_name,     /* 96 */
  cwlog_save_pitcher_id,         /* 97 */
  cwlog_save_pitcher_name,       /* 98 */
  cwlog_gwrbi_id,                /* 99 */
  cwlog_gwrbi_name,              /* 100 */
  cwlog_visitors_pitcher_id,     /* 101 */
  cwlog_visitors_pitcher_name,   /* 102 */
  cwlog_home_pitcher_id,         /* 103 */
  cwlog_home_pitcher_name        /* 104 */
};

void cwlog_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char *buf;
  char output_line[1024];
  int i, j, t, comma = 0;
  CWGameIterator *gameiter = cw_gameiter_create(game);
  CWBoxscore *box = cw_box_create(game);

  while (gameiter->event != NULL) {
    cw_gameiter_next(gameiter);
  }

  strcpy(output_line, "");
  buf = output_line;
  for (i = 0; i < 105; i++) {
    if (fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += (*function_ptrs[i])(buf, gameiter, box, visitors, home);
    }
  }

  for (t = 0; t <= 1; t++) {
    for (i = 1; i <= 9; i++) {
      for (j = 0; j <= 2; j++) {
	if (fields[105 + t*18 + 2*(i-1) + j]) {
	  if (ascii && comma) {
	    *(buf++) = ',';
	  }
	  else {
	    comma = 1;
	  }

	  if (j == 0) {
	    buf += cwlog_starting_player(buf, game, t, i);
	  }
	  else if (j == 1) {
	    CWAppearance *starter = cw_game_starter_find(game, t, i);
	    buf += cwlog_find_player_name(buf, starter->player_id,
					  visitors, home);
	  }
	  else {
	    buf += cwlog_starting_position(buf, game, t, i);
	  }
	}
      }
    }
  }

  if (fields[159]) {
    if (ascii && comma) {
      *(buf++) = ',';
    }
    else {
      comma = 1;
    }
    buf += cwlog_additional_info(buf, gameiter, box, visitors, home);
  }
  if (fields[160]) {
    if (ascii && comma) {
      *(buf++) = ',';
    }
    else {
      comma = 1;
    }
    buf += cwlog_acquisition_info(buf, gameiter, box, visitors, home);
  }

  printf(output_line);
  printf("\n");

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwlog_process_game;

void cwlog_print_help(void)
{
  fprintf(stderr, "\n\ncwlog generates files suitable for use by dBase or Lotus-like programs\n");
  fprintf(stderr, "The format matches the Retrosheet gamelogs, where possible.\n");
  fprintf(stderr, "Each record describes one game.\n");
  fprintf(stderr, "Usage: cwlog [options] eventfile...\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "  -h        print this help\n");
  fprintf(stderr, "  -i id     only process game given by id\n");
  fprintf(stderr, "  -y year   Year to process (for teamyyyy and aaayyyy.ros).\n");
  fprintf(stderr, "  -s start  Earliest date to process (mmdd).\n");
  fprintf(stderr, "  -e end    Last date to process (mmdd).\n");
  fprintf(stderr, "  -a        generate Ascii-delimited format files (default)\n");
  fprintf(stderr, "  -ft       generate Fortran format files\n");
  fprintf(stderr, "  -f flist  give list of fields to output\n");
  fprintf(stderr, "              Default is 0-160\n");
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwlog_print_help;

void
cwlog_print_field_list(void)
{
  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  All are included by default.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  fprintf(stderr, "0       game id\n");
  fprintf(stderr, "1       date\n");
  fprintf(stderr, "2       game number (0 = no double header)\n");
  fprintf(stderr, "3       day of week\n");
  fprintf(stderr, "4       start time\n");
  fprintf(stderr, "5       DH used flag\n");
  fprintf(stderr, "6       day/night flag\n");
  fprintf(stderr, "7       visiting team\n");
  fprintf(stderr, "8       home team\n");
  fprintf(stderr, "9       game site\n");
  fprintf(stderr, "10      vis. starting pitcher\n");
  fprintf(stderr, "11      home starting pitcher\n");
  fprintf(stderr, "12      home plate umpire\n");
  fprintf(stderr, "13      first base umpire\n");
  fprintf(stderr, "14      second base umpire\n");
  fprintf(stderr, "15      third base umpire\n");
  fprintf(stderr, "16      left field umpire\n");
  fprintf(stderr, "17      right field umpire\n");
  fprintf(stderr, "18      attendance\n");
  fprintf(stderr, "19      PS scorer\n");
  fprintf(stderr, "20      translator\n");
  fprintf(stderr, "21      inputter\n");
  fprintf(stderr, "22      input time\n");
  fprintf(stderr, "23      edit time\n");
  fprintf(stderr, "24      how scored\n");
  fprintf(stderr, "25      pitches entered?\n");
  fprintf(stderr, "26      temperature\n");
  fprintf(stderr, "27      wind direction\n");
  fprintf(stderr, "28      wind speed\n");
  fprintf(stderr, "29      field condition\n");
  fprintf(stderr, "30      precipitation\n");
  fprintf(stderr, "31      sky\n");
  fprintf(stderr, "32      time of game\n");
  fprintf(stderr, "33      number of innings\n");
  fprintf(stderr, "34      visitor final score\n");
  fprintf(stderr, "35      home final score\n");
  fprintf(stderr, "36      visitor hits\n");
  fprintf(stderr, "37      home hits\n");
  fprintf(stderr, "38      visitor errors\n");
  fprintf(stderr, "39      home errors\n");
  fprintf(stderr, "40      visitor left on base\n");
  fprintf(stderr, "41      home left on base\n");
  fprintf(stderr, "42      winning pitcher\n");
  fprintf(stderr, "43      losing pitcher\n");
  fprintf(stderr, "44      save for\n");
  fprintf(stderr, "45      GW RBI\n");
  fprintf(stderr, "46      visitor batter 1\n");
  fprintf(stderr, "47      visitor position 1\n");
  fprintf(stderr, "48      visitor batter 2\n");
  fprintf(stderr, "49      visitor position 2\n");
  fprintf(stderr, "50      visitor batter 3\n");
  fprintf(stderr, "51      visitor position 3\n");
  fprintf(stderr, "52      visitor batter 4\n");
  fprintf(stderr, "53      visitor position 4\n");
  fprintf(stderr, "54      visitor batter 5\n");
  fprintf(stderr, "55      visitor position 5\n");
  fprintf(stderr, "56      visitor batter 6\n");
  fprintf(stderr, "57      visitor position 6\n");
  fprintf(stderr, "58      visitor batter 7\n");
  fprintf(stderr, "59      visitor position 7\n");
  fprintf(stderr, "60      visitor batter 8\n");
  fprintf(stderr, "61      visitor position 8\n");
  fprintf(stderr, "62      visitor batter 9\n");
  fprintf(stderr, "63      visitor position 9\n");
  fprintf(stderr, "64      home batter 1\n");
  fprintf(stderr, "65      home position 1\n");
  fprintf(stderr, "66      home batter 2\n");
  fprintf(stderr, "67      home position 2\n");
  fprintf(stderr, "68      home batter 3\n");
  fprintf(stderr, "69      home position 3\n");
  fprintf(stderr, "70      home batter 4\n");
  fprintf(stderr, "71      home position 4\n");
  fprintf(stderr, "72      home batter 5\n");
  fprintf(stderr, "73      home position 5\n");
  fprintf(stderr, "74      home batter 6\n");
  fprintf(stderr, "75      home position 6\n");
  fprintf(stderr, "76      home batter 7\n");
  fprintf(stderr, "77      home position 7\n");
  fprintf(stderr, "78      home batter 8\n");
  fprintf(stderr, "79      home position 8\n");
  fprintf(stderr, "80      home batter 9\n");
  fprintf(stderr, "81      home position 9\n");
  fprintf(stderr, "82      visiting finisher (NULL if complete game)\n");
  fprintf(stderr, "83      home finisher (NULL if complete game)\n");

  exit(0);
}

void (*cwtools_print_field_list)(void) = cwlog_print_field_list;

void
cwlog_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick expanded game descriptor, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwlog_print_welcome_message;

void
cwlog_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwlog_initialize;

void
cwlog_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwlog_cleanup;

extern int cwtools_default_parse_command_line(int, char *argv[]);
int (*cwtools_parse_command_line)(int, char *argv[]) = cwtools_default_parse_command_line;
