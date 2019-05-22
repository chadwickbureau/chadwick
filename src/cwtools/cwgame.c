/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/cwgame.c
 * Chadwick game descriptor program
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
int fields[84] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1
};

int max_field = 83;

/* Extended fields to display (-x) */
int ext_fields[95] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0
};

int max_ext_field = 94;

char program_name[20] = "cwgame";

int print_header = 0;


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

typedef struct cwgame_lookup_struct {
  int code;
  char *text;
} CWLookup;

static int
cwgame_lookup(char *text, CWLookup *table)
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

/*
 * Derive a player name from an appearance record in a game.
 * Used when roster file is not available.
 */
char *
cwgame_game_find_name(CWGame *game, char *player_id)
{
  CWAppearance *app;
  CWEvent *event;

  for (app = game->first_starter; app != NULL; app = app->next) {
    if (!strcmp(app->player_id, player_id)) {
      return app->name;
    }
  }

  for (event = game->first_event; event != NULL; event = event->next) {
    for (app = event->first_sub; app != NULL; app = app->next) {
      if (!strcmp(app->player_id, player_id)) {
	return app->name;
      }
    }
  }
  return NULL;
}

/* Auxiliary function: find a player's bio entry and print his
 * first and last names to the buffer
 */
int cwgame_find_player_name(CWGame *game, char *buffer, char *player_id,
			    CWRoster *visitors, CWRoster *home)
{
  CWPlayer *bio = NULL;
  if (visitors) {
    bio = cw_roster_player_find(visitors, player_id);
  }    
  if (!bio && home) {
    bio = cw_roster_player_find(home, player_id);
  }
  if (bio) {
    return sprintf(buffer, "\"%s %s\"", bio->first_name, bio->last_name);
  }
  else {
    return sprintf(buffer, "\"%s\"", cwgame_game_find_name(game, player_id));
  }
}

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *, CWBoxscore *,
			  CWRoster *, CWRoster *);

/*
 * convenient structure to hold all information relating to a field
 * together in one place
 */
typedef struct field_struct {
  field_func f;
  char *header, *description;
} field_struct;

/*
 * preprocessor directive for conveniently declaring function signature
 */

#define DECLARE_FIELDFUNC(funcname) \
int funcname(char *buffer, CWGameIterator *gameiter, CWBoxscore *box, \
             CWRoster *visitors, CWRoster *home)

/* Field 0 */
DECLARE_FIELDFUNC(cwgame_game_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-12s",
		 gameiter->game->game_id);
}

/* Field 1 */
DECLARE_FIELDFUNC(cwgame_date)
{
  char *date = cw_game_info_lookup(gameiter->game, "date");
  return sprintf(buffer, (ascii) ? "\"%c%c%c%c%c%c%c%c\"" : "%c%c%c%c%c%c%c%c",
		 date[0], date[1], date[2], date[3],
		 date[5], date[6], date[8], date[9]);
}

/* Field 2 */
DECLARE_FIELDFUNC(cwgame_number)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%5d",
		 (tmp = cw_game_info_lookup(gameiter->game, "number")) ?
		 cw_atoi(tmp) : 0);
}

/* Field 3 */
DECLARE_FIELDFUNC(cwgame_day_of_week)
{
  static char *day_names[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
    "Saturday"
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

/* Field 4 */
DECLARE_FIELDFUNC(cwgame_start_time)
{
  int hour, min;
  char *time = cw_game_info_lookup(gameiter->game, "starttime");

  if (time == NULL) {
    return sprintf(buffer, (ascii) ? "0" : "   0");
  }

  sscanf(time, "%d:%d", &hour, &min);
  return sprintf(buffer, (ascii) ? "%d" : "%4d", hour * 100 + min);
}

/* Field 5 */
DECLARE_FIELDFUNC(cwgame_use_dh)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 
		 (tmp = cw_game_info_lookup(gameiter->game, "usedh")) ? 
		 ((!strcmp(tmp, "true")) ? 'T' : 'F') : 'F');
}

/* Field 6 */
DECLARE_FIELDFUNC(cwgame_day_night)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 
		 (tmp = cw_game_info_lookup(gameiter->game, "daynight")) ?
		 ((!strcmp(tmp, "night")) ? 'N' : 'D') : 'D');
}

/* Field 7 */
DECLARE_FIELDFUNC(cwgame_visitors)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		 (tmp = cw_game_info_lookup(gameiter->game, "visteam")) ?
		 tmp : "");
}

/* Field 8 */
DECLARE_FIELDFUNC(cwgame_home)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s", 
		 (tmp = cw_game_info_lookup(gameiter->game, "hometeam")) ?
		 tmp : "");
}

/* Field 9 */
DECLARE_FIELDFUNC(cwgame_site)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-5s",
		 (tmp = cw_game_info_lookup(gameiter->game, "site")) ?
		 tmp : "");
}

/* Field 10 */
DECLARE_FIELDFUNC(cwgame_visitors_pitcher)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_game_starter_find_by_position(gameiter->game,
						  0, 1)->player_id);
}

/* Field 11 */
DECLARE_FIELDFUNC(cwgame_home_pitcher)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_game_starter_find_by_position(gameiter->game,
						  1, 1)->player_id);
}

/* Field 12 */
DECLARE_FIELDFUNC(cwgame_umpire_home)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "umphome")) ?
		 tmp : "");
}

/* Field 13 */
DECLARE_FIELDFUNC(cwgame_umpire_1b)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "ump1b")) ? 
		 tmp : "");
}

/* Field 14 */
DECLARE_FIELDFUNC(cwgame_umpire_2b)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "ump2b")) ?
		 tmp : "");
}

/* Field 15 */
DECLARE_FIELDFUNC(cwgame_umpire_3b)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "ump3b")) ? 
		 tmp : "");
}

/* Field 16 */
DECLARE_FIELDFUNC(cwgame_umpire_lf)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "umplf")) ?
		 tmp : "");
}

/* Field 17 */
DECLARE_FIELDFUNC(cwgame_umpire_rf)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "umprf")) ?
		 tmp : "");
}

/* Field 18 */
DECLARE_FIELDFUNC(cwgame_attendance)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%5d", 
		 (tmp = cw_game_info_lookup(gameiter->game, "attendance")) ? 
		 cw_atoi(tmp) : 0);
}

/* Field 19 */
DECLARE_FIELDFUNC(cwgame_scorer)
{
  char *tmp;

  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "scorer")) ?
		 tmp : "");
}

/* Field 20 */
DECLARE_FIELDFUNC(cwgame_translator)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "translator")) ?
		 tmp : "");
}

/* Field 21 */
DECLARE_FIELDFUNC(cwgame_inputter)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "inputter")) ?
		 tmp : "");
}

/* Field 22 */
DECLARE_FIELDFUNC(cwgame_inputtime)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "inputtime")) ? 
		 tmp : "");
}

/* Field 23 */
DECLARE_FIELDFUNC(cwgame_edittime)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-30s",
		 (tmp = cw_game_info_lookup(gameiter->game, "edittime")) ? 
		 tmp : "");
}

/* Field 24 */
DECLARE_FIELDFUNC(cwgame_howscored)
{
  static CWLookup table[] = {
    { 0, "unknown" }, { 1, "park" }, { 2, "tv" }, { 3, "radio" },
    { -1, "" }
  };

  return sprintf(buffer, "%d", 
		 cwgame_lookup(cw_game_info_lookup(gameiter->game,
						   "howscored"), table));
}

/* Field 25 */
DECLARE_FIELDFUNC(cwgame_pitches)
{
  static CWLookup table[] = {
    { 0, "unknown" }, { 1, "pitches" }, { 2, "count" }, { 0, "none" },
    { -1, "" }
  };

  return sprintf(buffer, "%d", 
		 cwgame_lookup(cw_game_info_lookup(gameiter->game, "pitches"),
			       table));
}

/* Field 26 */
DECLARE_FIELDFUNC(cwgame_temperature)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 (tmp = cw_game_info_lookup(gameiter->game, "temp")) ? 
		 cw_atoi(tmp) : 0);
}

/* Field 27 */
DECLARE_FIELDFUNC(cwgame_wind_direction)
{
  CWLookup table[] = {
    { 0, "unknown" }, { 1, "tolf" }, { 2, "tocf" }, { 3, "torf" },
    { 4, "ltor" }, { 5, "fromlf" }, { 6, "fromcf" }, { 7, "fromrf" },
    { 8, "rtol" }, { -1, "" }
  };

  return sprintf(buffer, "%d",
		 cwgame_lookup(cw_game_info_lookup(gameiter->game, "winddir"),
			       table));
}

/* Field 28 */
DECLARE_FIELDFUNC(cwgame_wind_speed)
{
  char *tmp;
  return sprintf(buffer, "%d", 
		 (tmp = cw_game_info_lookup(gameiter->game, "windspeed")) ? 
		 cw_atoi(tmp) : 0);
}

/* Field 29 */
DECLARE_FIELDFUNC(cwgame_field_condition)
{
  CWLookup table[] = {
    { 0, "unknown" }, { 1, "soaked" }, { 2, "wet" }, 
    { 3, "damp" }, { 4, "dry" }, { -1, "" }
  };

  return sprintf(buffer, "%d",
		 cwgame_lookup(cw_game_info_lookup(gameiter->game,
						   "fieldcond"), table));
}

/* Field 30 */
DECLARE_FIELDFUNC(cwgame_precipitation)
{
  CWLookup table[] = {
    { 0, "unknown" }, { 1, "none" }, { 2, "drizzle" }, { 3, "showers" },
    { 4, "rain" }, { 5, "snow" }, { -1, "" }
  };

  return sprintf(buffer, "%d",
		 cwgame_lookup(cw_game_info_lookup(gameiter->game,
						   "precip"), table));
}

/* Field 31 */
DECLARE_FIELDFUNC(cwgame_sky)
{
  CWLookup table[] = {
    { 0, "unknown" }, { 1, "sunny" }, { 2, "cloudy" }, { 3, "overcast" },
    { 4, "night" }, { 5, "dome" }, { -1, "" }
  };

  return sprintf(buffer, "%d",
		 cwgame_lookup(cw_game_info_lookup(gameiter->game, "sky"),
			       table));
}

/* Field 32 */
DECLARE_FIELDFUNC(cwgame_time_of_game)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 (tmp = cw_game_info_lookup(gameiter->game, "timeofgame")) ? 
		 cw_atoi(tmp) : 0);
}

/* Field 33 */
DECLARE_FIELDFUNC(cwgame_innings)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->inning);
}

/* Field 34 */
DECLARE_FIELDFUNC(cwgame_visitor_score)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->score[0]);
}

/* Field 35 */
DECLARE_FIELDFUNC(cwgame_home_score)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->score[1]);
}

/* Field 36 */
DECLARE_FIELDFUNC(cwgame_visitor_hits)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->hits[0]);
}

/* Field 37 */
DECLARE_FIELDFUNC(cwgame_home_hits)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->hits[1]);
}

/* Field 38 */
DECLARE_FIELDFUNC(cwgame_visitor_errors)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->errors[0]);
}

/* Field 39 */
DECLARE_FIELDFUNC(cwgame_home_errors)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->errors[1]);
}

/* Field 40 */
DECLARE_FIELDFUNC(cwgame_visitor_lob)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 cw_gamestate_left_on_base(gameiter->state, 0));
}

/* Field 41 */
DECLARE_FIELDFUNC(cwgame_home_lob)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 cw_gamestate_left_on_base(gameiter->state, 1));
}

/* Field 42 */
DECLARE_FIELDFUNC(cwgame_winning_pitcher)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 (tmp = cw_game_info_lookup(gameiter->game, "wp")) ? tmp : "");
}

/* Field 43 */
DECLARE_FIELDFUNC(cwgame_losing_pitcher)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (tmp = cw_game_info_lookup(gameiter->game, "lp")) ? tmp : "");
}

/* Field 44 */
DECLARE_FIELDFUNC(cwgame_save)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (tmp = cw_game_info_lookup(gameiter->game, "save")) ?
		 tmp : "");
}

/* Field 45 */
DECLARE_FIELDFUNC(cwgame_gwrbi)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (tmp = cw_game_info_lookup(gameiter->game, "gwrbi")) ? 
		 tmp : "");
}

int
cwgame_final_pitcher(char *buffer, CWGame *game, CWBoxscore *box, int team)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (box->pitchers[team]->prev != NULL) ?
		 box->pitchers[team]->player_id : "");
}

/* Fields for starting lineups */
int
cwgame_starting_player(char *buffer, CWGame *game, int team, int slot)
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
cwgame_starting_position(char *buffer, CWGame *game, int team, int slot)
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

static field_struct field_data[] = {
  /*  0 */ { cwgame_game_id, "GAME_ID", "game id" },
  /*  1 */ { cwgame_date, "GAME_DT", "date" },
  /*  2 */ { cwgame_number, "GAME_CT", "game number (0 = no double header)" },
  /*  3 */ { cwgame_day_of_week, "GAME_DY", "day of week" },
  /*  4 */ { cwgame_start_time, "START_GAME_TM", "start time" },
  /*  5 */ { cwgame_use_dh, "DH_FL", "DH used flag" },
  /*  6 */ { cwgame_day_night, "DAYNIGHT_PARK_CD", "day/night flag" },
  /*  7 */ { cwgame_visitors, "AWAY_TEAM_ID", "visiting team" },
  /*  8 */ { cwgame_home, "HOME_TEAM_ID", "home team" },
  /*  9 */ { cwgame_site, "PARK_ID", "game site" },
  /* 10 */ { cwgame_visitors_pitcher, "AWAY_START_PIT_ID", 
	     "vis. starting pitcher" },
  /* 11 */ { cwgame_home_pitcher, "HOME_START_PIT_ID", 
	     "home starting pitcher" },
  /* 12 */ { cwgame_umpire_home, "BASE4_UMP_ID", "home plate umpire" },
  /* 13 */ { cwgame_umpire_1b, "BASE1_UMP_ID", "first base umpire" },
  /* 14 */ { cwgame_umpire_2b, "BASE2_UMP_ID", "second base umpire" },
  /* 15 */ { cwgame_umpire_3b, "BASE3_UMP_ID", "third base umpire" },
  /* 16 */ { cwgame_umpire_lf, "LF_UMP_ID", "left field umpire" },
  /* 17 */ { cwgame_umpire_rf, "RF_UMP_ID", "right field umpire" },
  /* 18 */ { cwgame_attendance, "ATTEND_PARK_CT", "attendance" },
  /* 19 */ { cwgame_scorer, "SCORER_RECORD_ID", "PS scorer" },
  /* 20 */ { cwgame_translator, "TRANSLATOR_RECORD_ID", "translator" },
  /* 21 */ { cwgame_inputter, "INPUTTER_RECORD_ID", "inputter" },
  /* 22 */ { cwgame_inputtime, "INPUT_RECORD_TS", "input time" },
  /* 23 */ { cwgame_edittime, "EDIT_RECORD_TS", "edit time" },
  /* 24 */ { cwgame_howscored, "METHOD_RECORD_CD", "how scored" },
  /* 25 */ { cwgame_pitches, "PITCHES_RECORD_CD", "pitches entered?" },
  /* 26 */ { cwgame_temperature, "TEMP_PARK_CT", "temperature" },
  /* 27 */ { cwgame_wind_direction, "WIND_DIRECTION_PARK_CD", 
	     "wind direction" },
  /* 28 */ { cwgame_wind_speed, "WIND_SPEED_PARK_CT", "wind speed" },
  /* 29 */ { cwgame_field_condition, "FIELD_PARK_CD", "field condition" },
  /* 30 */ { cwgame_precipitation, "PRECIP_PARK_CD", "precipitation" },
  /* 31 */ { cwgame_sky, "SKY_PARK_CD", "sky" },
  /* 32 */ { cwgame_time_of_game, "MINUTES_GAME_CT", "time of game" },
  /* 33 */ { cwgame_innings, "INN_CT", "number of innings" },
  /* 34 */ { cwgame_visitor_score, "AWAY_SCORE_CT", "visitor final score" },
  /* 35 */ { cwgame_home_score, "HOME_SCORE_CT", "home final score" },
  /* 36 */ { cwgame_visitor_hits, "AWAY_HITS_CT", "visitor hits" },
  /* 37 */ { cwgame_home_hits, "HOME_HITS_CT", "home hits" },
  /* 38 */ { cwgame_visitor_errors, "AWAY_ERR_CT", "visitor errors" },
  /* 39 */ { cwgame_home_errors, "HOME_ERR_CT", "home errors" },
  /* 40 */ { cwgame_visitor_lob, "AWAY_LOB_CT", "visitor left on base" },
  /* 41 */ { cwgame_home_lob, "HOME_LOB_CT", "home left on base" },
  /* 42 */ { cwgame_winning_pitcher, "WIN_PIT_ID", "winning pitcher" },
  /* 43 */ { cwgame_losing_pitcher, "LOSE_PIT_ID", "losing pitcher" },
  /* 44 */ { cwgame_save, "SAVE_PIT_ID", "save for" },
  /* 45 */ { cwgame_gwrbi, "GWRBI_BAT_ID", "GW RBI" },
  /* 46 */ { NULL, "AWAY_LINEUP1_BAT_ID", "visitor batter 1" },
  /* 47 */ { NULL, "AWAY_LINEUP1_FLD_CD", "visitor position 1" },
  /* 48 */ { NULL, "AWAY_LINEUP2_BAT_ID", "visitor batter 2" },
  /* 49 */ { NULL, "AWAY_LINEUP2_FLD_CD", "visitor position 2" },
  /* 50 */ { NULL, "AWAY_LINEUP3_BAT_ID", "visitor batter 3" },
  /* 51 */ { NULL, "AWAY_LINEUP3_FLD_CD", "visitor position 3" },
  /* 52 */ { NULL, "AWAY_LINEUP4_BAT_ID", "visitor batter 4" },
  /* 53 */ { NULL, "AWAY_LINEUP4_FLD_CD", "visitor position 4" },
  /* 54 */ { NULL, "AWAY_LINEUP5_BAT_ID", "visitor batter 5" },
  /* 55 */ { NULL, "AWAY_LINEUP5_FLD_CD", "visitor position 5" },
  /* 56 */ { NULL, "AWAY_LINEUP6_BAT_ID", "visitor batter 6" },
  /* 57 */ { NULL, "AWAY_LINEUP6_FLD_CD", "visitor position 6" },
  /* 58 */ { NULL, "AWAY_LINEUP7_BAT_ID", "visitor batter 7" },
  /* 59 */ { NULL, "AWAY_LINEUP7_FLD_CD", "visitor position 7" },
  /* 60 */ { NULL, "AWAY_LINEUP8_BAT_ID", "visitor batter 8" },
  /* 61 */ { NULL, "AWAY_LINEUP8_FLD_CD", "visitor position 8" },
  /* 62 */ { NULL, "AWAY_LINEUP9_BAT_ID", "visitor batter 9" },
  /* 63 */ { NULL, "AWAY_LINEUP9_FLD_CD", "visitor position 9" },
  /* 64 */ { NULL, "HOME_LINEUP1_BAT_ID", "home batter 1" },
  /* 65 */ { NULL, "HOME_LINEUP1_FLD_CD", "home position 1" },
  /* 66 */ { NULL, "HOME_LINEUP2_BAT_ID", "home batter 2" },
  /* 67 */ { NULL, "HOME_LINEUP2_FLD_CD", "home position 2" },
  /* 68 */ { NULL, "HOME_LINEUP3_BAT_ID", "home batter 3" },
  /* 69 */ { NULL, "HOME_LINEUP3_FLD_CD", "home position 3" },
  /* 70 */ { NULL, "HOME_LINEUP4_BAT_ID", "home batter 4" },
  /* 71 */ { NULL, "HOME_LINEUP4_FLD_CD", "home position 4" },
  /* 72 */ { NULL, "HOME_LINEUP5_BAT_ID", "home batter 5" },
  /* 73 */ { NULL, "HOME_LINEUP5_FLD_CD", "home position 5" },
  /* 74 */ { NULL, "HOME_LINEUP6_BAT_ID", "home batter 6" },
  /* 75 */ { NULL, "HOME_LINEUP6_FLD_CD", "home position 6" },
  /* 76 */ { NULL, "HOME_LINEUP7_BAT_ID", "home batter 7" },
  /* 77 */ { NULL, "HOME_LINEUP7_FLD_CD", "home position 7" },
  /* 78 */ { NULL, "HOME_LINEUP8_BAT_ID", "home batter 8" },
  /* 79 */ { NULL, "HOME_LINEUP8_FLD_CD", "home position 8" },
  /* 80 */ { NULL, "HOME_LINEUP9_BAT_ID", "home batter 9" },
  /* 81 */ { NULL, "HOME_LINEUP9_FLD_CD", "home position 9" },
  /* 82 */ { NULL, "AWAY_FINISH_PIT_ID", 
	     "visiting finisher (NULL if complete game)" },
  /* 83 */ { NULL, "HOME_FINISH_PIT_ID", 
	     "home finisher (NULL if complete game)" }
};

/*************************************************************************
 * Implementation of "extended" fields
 *************************************************************************/

/* Extended Field 0 */
DECLARE_FIELDFUNC(cwgame_visitors_league)
{
  return sprintf(buffer, "\"%s\"", (visitors) ? visitors->league : "");
}

/* Extended Field 1 */
DECLARE_FIELDFUNC(cwgame_home_league)
{
  return sprintf(buffer, "\"%s\"", (home) ? home->league : "");
}

/* Extended Field 2 */
DECLARE_FIELDFUNC(cwgame_visitors_game)
{
  return sprintf(buffer, "%s", "");
}

/* Extended Field 3 */
DECLARE_FIELDFUNC(cwgame_home_game)
{
  return sprintf(buffer, "%s", "");
}

/* Extended Field 4 */
DECLARE_FIELDFUNC(cwgame_length_outs)
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

/* Extended Field 5 */
DECLARE_FIELDFUNC(cwgame_completion_info)
{
  return sprintf(buffer, "%s", "");
}

/* Extended Field 6 */
DECLARE_FIELDFUNC(cwgame_forfeit_info)
{
  return sprintf(buffer, "%s", "");
}

/* Extended Field 7 */
DECLARE_FIELDFUNC(cwgame_protest_info)
{
  return sprintf(buffer, "%s", "");
}

/* Extended Field 8 */
DECLARE_FIELDFUNC(cwgame_visitors_line)
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

/* Extended Field 9 */
DECLARE_FIELDFUNC(cwgame_home_line)
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
DECLARE_FIELDFUNC(cwgame_visitors_ab)
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

/* Field 23 */
DECLARE_FIELDFUNC(cwgame_visitors_2b)
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
DECLARE_FIELDFUNC(cwgame_visitors_3b)
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
DECLARE_FIELDFUNC(cwgame_visitors_hr)
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
DECLARE_FIELDFUNC(cwgame_visitors_bi)
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
DECLARE_FIELDFUNC(cwgame_visitors_sh)
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
DECLARE_FIELDFUNC(cwgame_visitors_sf)
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
DECLARE_FIELDFUNC(cwgame_visitors_hp)
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
DECLARE_FIELDFUNC(cwgame_visitors_bb)
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
DECLARE_FIELDFUNC(cwgame_visitors_ibb)
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
DECLARE_FIELDFUNC(cwgame_visitors_so)
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
DECLARE_FIELDFUNC(cwgame_visitors_sb)
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
DECLARE_FIELDFUNC(cwgame_visitors_cs)
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
DECLARE_FIELDFUNC(cwgame_visitors_gdp)
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
DECLARE_FIELDFUNC(cwgame_visitors_xi)
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

/* Field 38 */
DECLARE_FIELDFUNC(cwgame_visitors_pitchers)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 0);
  int i = 0;

  for (; pitcher != NULL; pitcher = pitcher->next, i++);
  return sprintf(buffer, "%d", i);
}

/* Field 39 */
DECLARE_FIELDFUNC(cwgame_visitors_er)
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
DECLARE_FIELDFUNC(cwgame_visitors_ter)
{
  return sprintf(buffer, "%d", box->er[0]);
}

/* Field 41 */
DECLARE_FIELDFUNC(cwgame_visitors_wp)
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
DECLARE_FIELDFUNC(cwgame_visitors_bk)
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
DECLARE_FIELDFUNC(cwgame_visitors_po)
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
DECLARE_FIELDFUNC(cwgame_visitors_a)
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

/* Field 46 */
DECLARE_FIELDFUNC(cwgame_visitors_pb)
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
DECLARE_FIELDFUNC(cwgame_visitors_dp)
{
  return sprintf(buffer, "%d", box->dp[0]);
}

/* Field 48 */
DECLARE_FIELDFUNC(cwgame_visitors_tp)
{
  return sprintf(buffer, "%d", box->tp[0]);
}

/* Field 49 */
DECLARE_FIELDFUNC(cwgame_home_ab)
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

/* Field 51 */
DECLARE_FIELDFUNC(cwgame_home_2b)
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
DECLARE_FIELDFUNC(cwgame_home_3b)
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
DECLARE_FIELDFUNC(cwgame_home_hr)
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
DECLARE_FIELDFUNC(cwgame_home_bi)
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
DECLARE_FIELDFUNC(cwgame_home_sh)
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
DECLARE_FIELDFUNC(cwgame_home_sf)
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
DECLARE_FIELDFUNC(cwgame_home_hp)
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
DECLARE_FIELDFUNC(cwgame_home_bb)
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
DECLARE_FIELDFUNC(cwgame_home_ibb)
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
DECLARE_FIELDFUNC(cwgame_home_so)
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
DECLARE_FIELDFUNC(cwgame_home_sb)
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
DECLARE_FIELDFUNC(cwgame_home_cs)
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
DECLARE_FIELDFUNC(cwgame_home_gdp)
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
DECLARE_FIELDFUNC(cwgame_home_xi)
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

/* Field 66 */
DECLARE_FIELDFUNC(cwgame_home_pitchers)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(box, 1);
  int i = 0;

  for (; pitcher != NULL; pitcher = pitcher->next, i++);
  return sprintf(buffer, "%d", i);
}

/* Field 67 */
DECLARE_FIELDFUNC(cwgame_home_er)
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
DECLARE_FIELDFUNC(cwgame_home_ter)
{
  return sprintf(buffer, "%d", box->er[1]);
}

/* Field 69 */
DECLARE_FIELDFUNC(cwgame_home_wp)
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
DECLARE_FIELDFUNC(cwgame_home_bk)
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
DECLARE_FIELDFUNC(cwgame_home_po)
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
DECLARE_FIELDFUNC(cwgame_home_a)
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

/* Field 74 */
DECLARE_FIELDFUNC(cwgame_home_pb)
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
DECLARE_FIELDFUNC(cwgame_home_dp)
{
  return sprintf(buffer, "%d", box->dp[1]);
}

/* Field 76 */
DECLARE_FIELDFUNC(cwgame_home_tp)
{
  return sprintf(buffer, "%d", box->tp[1]);
}

/* Field 78 */
DECLARE_FIELDFUNC(cwgame_umpire_home_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 78 */
DECLARE_FIELDFUNC(cwgame_umpire_1b_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 78 */
DECLARE_FIELDFUNC(cwgame_umpire_2b_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 78 */
DECLARE_FIELDFUNC(cwgame_umpire_3b_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 78 */
DECLARE_FIELDFUNC(cwgame_umpire_lf_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 78 */
DECLARE_FIELDFUNC(cwgame_umpire_rf_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 89 */
DECLARE_FIELDFUNC(cwgame_visitors_manager_id)
{
  return sprintf(buffer, "%s", "");
}

/* Field 90 */
DECLARE_FIELDFUNC(cwgame_visitors_manager_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 91 */
DECLARE_FIELDFUNC(cwgame_home_manager_id)
{
  return sprintf(buffer, "%s", "");
}

/* Field 92 */
DECLARE_FIELDFUNC(cwgame_home_manager_name)
{
  return sprintf(buffer, "%s", "");
}

/* Field 94 */
DECLARE_FIELDFUNC(cwgame_winning_pitcher_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "wp");
  if (tmp && strcmp(tmp, "")) {
    return cwgame_find_player_name(gameiter->game, buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 96 */
DECLARE_FIELDFUNC(cwgame_losing_pitcher_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "lp");
  if (tmp && strcmp(tmp, "")) {
    return cwgame_find_player_name(gameiter->game, buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 98 */
DECLARE_FIELDFUNC(cwgame_save_pitcher_name)
{
  char *tmp = cw_game_info_lookup(gameiter->game, "save");
  if (tmp && strcmp(tmp, "")) {
    return cwgame_find_player_name(gameiter->game, buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

/* Field 99 */
DECLARE_FIELDFUNC(cwgame_goahead_rbi_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 (gameiter->state->go_ahead_rbi) ? gameiter->state->go_ahead_rbi : "");
}

/* Field 100 */
DECLARE_FIELDFUNC(cwgame_goahead_rbi_name)
{
  char *tmp = gameiter->state->go_ahead_rbi;
  if (tmp && strcmp(tmp, "")) {
    return cwgame_find_player_name(gameiter->game, buffer, tmp, visitors, home);
  }
  else {
    return sprintf(buffer, "\"(none)\"");
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter1_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 1);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter2_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 2);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter3_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 3);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter4_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 4);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter5_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 5);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter6_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 6);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter7_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 7);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter8_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 8);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_visitors_batter9_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 0, 9);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter1_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 1);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter2_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 2);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter3_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 3);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter4_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 4);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter5_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 5);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter6_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 6);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter7_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 7);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter8_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 8);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

DECLARE_FIELDFUNC(cwgame_home_batter9_name)
{
  CWAppearance *starter = cw_game_starter_find(gameiter->game, 1, 9);
  if (starter) {
    return cwgame_find_player_name(gameiter->game, buffer, starter->player_id,
				   visitors, home);
  }
  else {
    return sprintf(buffer, "(null)"); 
  }
}

/* Field 159 */
DECLARE_FIELDFUNC(cwgame_additional_info)
{
  if (cw_game_info_lookup(gameiter->game, "htbf") &&
      !strcmp(cw_game_info_lookup(gameiter->game, "htbf"), "true")) {
    return sprintf(buffer, "%s", "HTBF");
  }
  else {
    return sprintf(buffer, "%s", "");
  }
}

/* Field 160 */
DECLARE_FIELDFUNC(cwgame_acquisition_info)
{
  return sprintf(buffer, "%s", "");
}

static field_struct ext_field_data[] = {
  { cwgame_visitors_league, "AWAY_TEAM_LEAGUE_ID", "visiting team league" },
  { cwgame_home_league, "HOME_TEAM_LEAGUE_ID", "home team league" },
  { cwgame_visitors_game, "AWAY_TEAM_GAME_CT", "visiting team game number" },
  { cwgame_home_game, "HOME_TEAM_GAME_CT", "home team game number" },
  { cwgame_length_outs, "OUTS_CT", "length of game in outs" },
  { cwgame_completion_info, "COMPLETION_TX", "information on completion of game" },
  { cwgame_forfeit_info, "FORFEIT_TX", "information on forfeit of game" },
  { cwgame_protest_info, "PROTEST_TX", "information on protest of game" },
  { cwgame_visitors_line, "AWAY_LINE_TX", "visiting team linescore" },
  { cwgame_home_line, "HOME_LINE_TX", "home team linescore" },
  { cwgame_visitors_ab, "AWAY_AB_CT", "visiting team AB" },
  { cwgame_visitors_2b, "AWAY_2B_CT", "visiting team 2B" },
  { cwgame_visitors_3b, "AWAY_3B_CT", "visiting team 3B" },
  { cwgame_visitors_hr, "AWAY_HR_CT", "visiting team HR" },
  { cwgame_visitors_bi, "AWAY_BI_CT", "visiting team RBI" },
  { cwgame_visitors_sh, "AWAY_SH_CT", "visiting team SH" },
  { cwgame_visitors_sf, "AWAY_SF_CT", "visiting team SF" },
  { cwgame_visitors_hp, "AWAY_HP_CT", "visiting team HP" },
  { cwgame_visitors_bb, "AWAY_BB_CT", "visiting team BB" },
  { cwgame_visitors_ibb, "AWAY_IBB_CT", "visiting team IBB" },
  { cwgame_visitors_so, "AWAY_SO_CT", "visiting team SO" },
  { cwgame_visitors_sb, "AWAY_SB_CT", "visiting team SB" },
  { cwgame_visitors_cs, "AWAY_CS_CT", "visiting team CS" },
  { cwgame_visitors_gdp, "AWAY_GDP_CT", "visiting team GDP" },
  { cwgame_visitors_xi, "AWAY_XI_CT", "visiting team reach on interference" },
  { cwgame_visitors_pitchers, "AWAY_PITCHER_CT", "number of pitchers used by visiting team" },
  { cwgame_visitors_er, "AWAY_ER_CT", "visiting team individual ER allowed" },
  { cwgame_visitors_ter, "AWAY_TER_CT", "visiting team team ER allowed" },
  { cwgame_visitors_wp, "AWAY_WP_CT", "visiting team WP" },
  { cwgame_visitors_bk, "AWAY_BK_CT", "visiting team BK" },
  { cwgame_visitors_po, "AWAY_PO_CT", "visiting team PO" },
  { cwgame_visitors_a, "AWAY_A_CT", "visiting team A" },
  { cwgame_visitors_pb, "AWAY_PB_CT", "visiting team PB" },
  { cwgame_visitors_dp, "AWAY_DP_CT", "visiting team DP" },
  { cwgame_visitors_tp, "AWAY_TP_CT", "visiting team TP" },
  { cwgame_home_ab, "HOME_AB_CT", "home team AB" },
  { cwgame_home_2b, "HOME_2B_CT", "home team 2B" },
  { cwgame_home_3b, "HOME_3B_CT", "home team 3B" },
  { cwgame_home_hr, "HOME_HR_CT", "home team HR" },
  { cwgame_home_bi, "HOME_BI_CT", "home team RBI" },
  { cwgame_home_sh, "HOME_SH_CT", "home team SH" },
  { cwgame_home_sf, "HOME_SF_CT", "home team SF" },
  { cwgame_home_hp, "HOME_HP_CT", "home team HP" },
  { cwgame_home_bb, "HOME_BB_CT", "home team BB" },
  { cwgame_home_ibb, "HOME_IBB_CT", "home team IBB" },
  { cwgame_home_so, "HOME_SO_CT", "home team SO" },
  { cwgame_home_sb, "HOME_SB_CT", "home team SB" },
  { cwgame_home_cs, "HOME_CS_CT", "home team CS" },
  { cwgame_home_gdp, "HOME_GDP_CT", "home team GDP" },
  { cwgame_home_xi, "HOME_XI_CT", "home team reach on interference" },
  { cwgame_home_pitchers, "HOME_PITCHER_CT", "number of pitchers used by home team" },
  { cwgame_home_er, "HOME_ER_CT", "home team individual ER allowed" },
  { cwgame_home_ter, "HOME_TER_CT", "home team team ER allowed" },
  { cwgame_home_wp, "HOME_WP_CT", "home team WP" },
  { cwgame_home_bk, "HOME_BK_CT", "home team BK" },
  { cwgame_home_po, "HOME_PO_CT", "home team PO" },
  { cwgame_home_a, "HOME_A_CT", "home team A" },
  { cwgame_home_pb, "HOME_PB_CT", "home team PB" },
  { cwgame_home_dp, "HOME_DP_CT", "home team DP" },
  { cwgame_home_tp, "HOME_TP_CT", "home team TP" },
  { cwgame_umpire_home_name, "UMP_HOME_NAME_TX", "home plate umpire name" },
  { cwgame_umpire_1b_name, "UMP_1B_NAME_TX", "first base umpire name" },
  { cwgame_umpire_2b_name, "UMP_2B_NAME_TX", "second base umpire name" },
  { cwgame_umpire_3b_name, "UMP_3B_NAME_TX", "third base umpire name" },
  { cwgame_umpire_lf_name, "UMP_LF_NAME_TX", "left field umpire name" },
  { cwgame_umpire_rf_name, "UMP_RF_NAME_TX", "right field umpire name" },
  { cwgame_visitors_manager_id, "AWAY_MANAGER_ID", "visitors manager ID" },
  { cwgame_visitors_manager_name, "AWAY_MANAGER_NAME_TX", "visitors manager name" },
  { cwgame_home_manager_id, "HOME_MANAGER_ID", "home manager ID" },
  { cwgame_home_manager_name, "HOME_MANAGER_NAME_TX", "home manager name" },
  { cwgame_winning_pitcher_name, "WIN_PIT_NAME_TX", "winning pitcher name" },
  { cwgame_losing_pitcher_name, "LOSE_PIT_NAME_TX", "losing pitcher name" },
  { cwgame_save_pitcher_name, "SAVE_PIT_NAME_TX", "save pitcher name" },
  { cwgame_goahead_rbi_id, "GOAHEAD_RBI_ID", "batter with goahead RBI ID" },
  { cwgame_goahead_rbi_name, "GOAHEAD_RBI_NAME_TX", "batter with goahead RBI" },
  { cwgame_visitors_batter1_name, "AWAY_LINEUP1_BAT_NAME_TX", "visitor batter 1 name" },
  { cwgame_visitors_batter2_name, "AWAY_LINEUP2_BAT_NAME_TX", "visitor batter 2 name" },
  { cwgame_visitors_batter3_name, "AWAY_LINEUP3_BAT_NAME_TX", "visitor batter 3 name" },
  { cwgame_visitors_batter4_name, "AWAY_LINEUP4_BAT_NAME_TX", "visitor batter 4 name" },
  { cwgame_visitors_batter5_name, "AWAY_LINEUP5_BAT_NAME_TX", "visitor batter 5 name" },
  { cwgame_visitors_batter6_name, "AWAY_LINEUP6_BAT_NAME_TX", "visitor batter 6 name" },
  { cwgame_visitors_batter7_name, "AWAY_LINEUP7_BAT_NAME_TX", "visitor batter 7 name" },
  { cwgame_visitors_batter8_name, "AWAY_LINEUP8_BAT_NAME_TX", "visitor batter 8 name" },
  { cwgame_visitors_batter9_name, "AWAY_LINEUP9_BAT_NAME_TX", "visitor batter 9 name" },
  { cwgame_home_batter1_name, "HOME_LINEUP1_BAT_NAME_TX", "home batter 1 name" },
  { cwgame_home_batter2_name, "HOME_LINEUP2_BAT_NAME_TX", "home batter 2 name" },
  { cwgame_home_batter3_name, "HOME_LINEUP3_BAT_NAME_TX", "home batter 3 name" },
  { cwgame_home_batter4_name, "HOME_LINEUP4_BAT_NAME_TX", "home batter 4 name" },
  { cwgame_home_batter5_name, "HOME_LINEUP5_BAT_NAME_TX", "home batter 5 name" },
  { cwgame_home_batter6_name, "HOME_LINEUP6_BAT_NAME_TX", "home batter 6 name" },
  { cwgame_home_batter7_name, "HOME_LINEUP7_BAT_NAME_TX", "home batter 7 name" },
  { cwgame_home_batter8_name, "HOME_LINEUP8_BAT_NAME_TX", "home batter 8 name" },
  { cwgame_home_batter9_name, "HOME_LINEUP9_BAT_NAME_TX", "home batter 9 name" },
  { cwgame_additional_info, "ADD_INFO_TX", "additional information" },
  { cwgame_acquisition_info, "ACQ_INFO_TX", "acquisition information" }
};

void cwgame_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char *buf;
  char output_line[4096];
  int i, j, t, comma = 0;
  CWGameIterator *gameiter = cw_gameiter_create(game);
  CWBoxscore *box = cw_box_create(game);

  while (gameiter->event != NULL) {
    cw_gameiter_next(gameiter);
  }

  strcpy(output_line, "");
  buf = output_line;
  for (i = 0; i < 46; i++) {
    if (fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += (*field_data[i].f)(buf, gameiter, box, visitors, home);
    }
  }

  for (t = 0; t <= 1; t++) {
    for (i = 1; i <= 9; i++) {
      for (j = 0; j <= 1; j++) {
	if (fields[46 + t*18 + 2*(i-1) + j]) {
	  if (ascii && comma) {
	    *(buf++) = ',';
	  }
	  else {
	    comma = 1;
	  }
	  buf += ((j == 0) ? cwgame_starting_player(buf, game, t, i) :
		  cwgame_starting_position(buf, game, t, i));
	}
      }
    }
  }

  t = 0;
  for (i = 82; i < 84; i++) {
    if (fields[i]) {
      if (ascii && comma) {
        *(buf++) = ',';
      }
      else {
        comma = 1;
      }
      buf += cwgame_final_pitcher(buf, game, box, t);
    }
    t++;
  }

  for (i = 0; i <= max_ext_field; i++) {
    if (ext_fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += (*ext_field_data[i].f)(buf, gameiter, box, visitors, home);
    }
  }
  
  printf("%s", output_line);
  printf("\n");

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwgame_process_game;

void cwgame_print_help(void)
{
  fprintf(stderr, "\n\ncwgame generates files suitable for use by dBase or Lotus-like programs\n");
  fprintf(stderr, "Each record describes one game.\n");
  fprintf(stderr, "Usage: cwgame [options] eventfile...\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "  -h        print this help\n");
  fprintf(stderr, "  -i id     only process game given by id\n");
  fprintf(stderr, "  -y year   Year to process (for teamyyyy and aaayyyy.ros).\n");
  fprintf(stderr, "  -s start  Earliest date to process (mmdd).\n");
  fprintf(stderr, "  -e end    Last date to process (mmdd).\n");
  fprintf(stderr, "  -a        generate Ascii-delimited format files (default)\n");
  fprintf(stderr, "  -ft       generate Fortran format files\n");
  fprintf(stderr, "  -f flist  give list of fields to output\n");
  fprintf(stderr, "              Default is 0-83\n");
  fprintf(stderr, "  -x flist  give list of extended fields to output\n");
  fprintf(stderr, "              Default is none\n");
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");
  fprintf(stderr, "  -n        print field names in first row of output\n\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwgame_print_help;

void
cwgame_print_field_list(void)
{
  int i;

  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  All are included by default.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  for (i = 0; i <= max_field; i++) {
    fprintf(stderr, "%-2d      %s\n", i, field_data[i].description);
  }
  fprintf(stderr, "\n");

  fprintf(stderr, "These additional fields are available in this version of cwgame.\n");
  fprintf(stderr, "These are specified using the -x option, and appear in the output\n");
  fprintf(stderr, "after all fields specified with -f. By default, none of these\n");
  fprintf(stderr, "fields are output.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  for (i = 0; i <= max_ext_field; i++) {
    fprintf(stderr, "%-2d      %s\n", i, ext_field_data[i].description);
  }

  exit(0);
}

void (*cwtools_print_field_list)(void) = cwgame_print_field_list;

void
cwgame_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick expanded game descriptor, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "Copyright (c) 2002-2019\nDr T L Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)\n");
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwgame_print_welcome_message;

void
cwgame_initialize(void)
{
  int i, comma = 0;
  char output_line[4096];
  char *buf;

  if (!ascii || !print_header) {
    return;
  }

  strcpy(output_line, "");
  buf = output_line;

  for (i = 0; i <= max_field; i++) {
    if (fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += sprintf(buf, "\"%s\"", field_data[i].header);
    }
  }

  for (i = 0; i <= max_ext_field; i++) {
    if (ext_fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += sprintf(buf, "\"%s\"", ext_field_data[i].header);
    }
  }

  printf("%s", output_line);
  printf("\n");
}

void (*cwtools_initialize)(void) = cwgame_initialize;

void
cwgame_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwgame_cleanup;

extern char year[5];
extern char first_date[5];
extern char last_date[5];
extern char game_id[20];
extern int ascii;
extern int quiet;

extern void
cwtools_parse_field_list(char *text, int max_field, int *fields);

int
cwgame_parse_command_line(int argc, char *argv[])
{
  int i;
  strcpy(year, "");

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {
      ascii = 1;
    }
    else if (!strcmp(argv[i], "-d")) {
      (*cwtools_print_welcome_message)(argv[0]);
      (*cwtools_print_field_list)();
    }
    else if (!strcmp(argv[i], "-e")) {
      if (++i < argc) {
	strncpy(last_date, argv[i], 4);
      }
    }
    else if (!strcmp(argv[i], "-h")) {
      (*cwtools_print_welcome_message)(argv[0]);
      (*cwtools_print_help)();
    }
    else if (!strcmp(argv[i], "-q")) {
      quiet = 1;
    }
    else if (!strcmp(argv[i], "-i")) {
      if (++i < argc) {
	strncpy(game_id, argv[i], 19);
      }
    }
    else if (!strcmp(argv[i], "-f")) {
      if (++i < argc) {
	cwtools_parse_field_list(argv[i], max_field, fields);
      }
    }
    else if (!strcmp(argv[i], "-n")) {
      print_header = 1;
    }
    else if (!strcmp(argv[i], "-ft")) {
      ascii = 0;
    }
    else if (!strcmp(argv[i], "-s")) {
      if (++i < argc) {
	strncpy(first_date, argv[i], 4);
      }
    }
    else if (!strcmp(argv[i], "-x")) {
      if (++i < argc) {
	cwtools_parse_field_list(argv[i], max_ext_field, ext_fields);
      }
    }
    else if (!strcmp(argv[i], "-y")) {
      if (++i < argc) {
	strncpy(year, argv[i], 5);
      }
    }
    else if (argv[i][0] == '-') {
      fprintf(stderr, "*** Invalid option '%s'.\n", argv[i]);
      exit(1);
    }
    else {
      break;
    }
  }

  return i;
}

int (*cwtools_parse_command_line)(int, char *argv[]) = cwgame_parse_command_line;
