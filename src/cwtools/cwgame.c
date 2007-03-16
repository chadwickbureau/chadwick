/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick game descriptor program
 * 
 * This file is part of Chadwick, tools for baseball play-by-play and stats
 * Copyright (C) 2002-2007, Ted Turocy (drarbiter@gmail.com)
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

char program_name[20] = "cwgame";


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
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *,
			  CWRoster *, CWRoster *);

/*
 * preprocessor directive for conveniently declaring function signature
 */

#define DECLARE_FIELDFUNC(funcname) \
int funcname(char *buffer, CWGameIterator *gameiter, \
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
  return sprintf(buffer, (ascii) ? "\"%c%c%c%c%c%c\"" : "%c%c%c%c%c%c",
		 gameiter->game->game_id[5], gameiter->game->game_id[6],
		 gameiter->game->game_id[7], gameiter->game->game_id[8],
		 gameiter->game->game_id[9], gameiter->game->game_id[10]);
}

/* Field 2 */
DECLARE_FIELDFUNC(cwgame_number)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%5d",
		 (tmp = cw_game_info_lookup(gameiter->game, "number")) ?
		 atoi(tmp) : 0);
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
		 atoi(tmp) : 0);
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
		 atoi(tmp) : 0);
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
		 atoi(tmp) : 0);
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
		 atoi(tmp) : 0);
}

/* Field 33 */
DECLARE_FIELDFUNC(cwgame_innings)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->inning);
}

/* Field 34 */
DECLARE_FIELDFUNC(cwgame_visitor_score)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->score[0]);
}

/* Field 35 */
DECLARE_FIELDFUNC(cwgame_home_score)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->score[1]);
}

/* Field 36 */
DECLARE_FIELDFUNC(cwgame_visitor_hits)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->hits[0]);
}

/* Field 37 */
DECLARE_FIELDFUNC(cwgame_home_hits)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->hits[1]);
}

/* Field 38 */
DECLARE_FIELDFUNC(cwgame_visitor_errors)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->errors[0]);
}

/* Field 39 */
DECLARE_FIELDFUNC(cwgame_home_errors)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->errors[1]);
}

/* Field 40 */
DECLARE_FIELDFUNC(cwgame_visitor_lob)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 cw_gameiter_left_on_base(gameiter, 0));
}

/* Field 41 */
DECLARE_FIELDFUNC(cwgame_home_lob)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 cw_gameiter_left_on_base(gameiter, 1));
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
cwgame_final_pitcher(char *buffer, CWGame *game, int team)
{
  char *last = "";

  CWEvent *event = game->first_event;
  while (event != NULL) {
    if (event->first_sub != NULL) {
      CWAppearance *sub = event->first_sub;
      while (sub != NULL) {
        if (sub->team == team && sub->pos == 1) {
          last = sub->player_id;
        }
        sub = sub->next;
      }
    }
    event = event->next;
  }
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", last); 
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

static field_func function_ptrs[] = {
  cwgame_game_id,                 /* 0 */
  cwgame_date,                    /* 1 */
  cwgame_number,                  /* 2 */
  cwgame_day_of_week,             /* 3 */
  cwgame_start_time,              /* 4 */
  cwgame_use_dh,                  /* 5 */
  cwgame_day_night,               /* 6 */
  cwgame_visitors,                /* 7 */
  cwgame_home,                    /* 8 */
  cwgame_site,                    /* 9 */
  cwgame_visitors_pitcher,        /* 10 */
  cwgame_home_pitcher,            /* 11 */
  cwgame_umpire_home,             /* 12 */
  cwgame_umpire_1b,               /* 13 */
  cwgame_umpire_2b,               /* 14 */
  cwgame_umpire_3b,               /* 15 */
  cwgame_umpire_lf,               /* 16 */
  cwgame_umpire_rf,               /* 17 */
  cwgame_attendance,              /* 18 */
  cwgame_scorer,                  /* 19 */
  cwgame_translator,              /* 20 */
  cwgame_inputter,                /* 21 */
  cwgame_inputtime,               /* 22 */
  cwgame_edittime,                /* 23 */
  cwgame_howscored,               /* 24 */
  cwgame_pitches,                 /* 25 */
  cwgame_temperature,             /* 26 */
  cwgame_wind_direction,          /* 27 */
  cwgame_wind_speed,              /* 28 */
  cwgame_field_condition,         /* 29 */
  cwgame_precipitation,           /* 30 */
  cwgame_sky,                     /* 31 */
  cwgame_time_of_game,            /* 32 */
  cwgame_innings,                 /* 33 */
  cwgame_visitor_score,           /* 34 */
  cwgame_home_score,              /* 35 */
  cwgame_visitor_hits,            /* 36 */
  cwgame_home_hits,               /* 37 */
  cwgame_visitor_errors,          /* 38 */
  cwgame_home_errors,             /* 39 */
  cwgame_visitor_lob,             /* 40 */
  cwgame_home_lob,                /* 41 */
  cwgame_winning_pitcher,         /* 42 */
  cwgame_losing_pitcher,          /* 43 */
  cwgame_save,                    /* 44 */
  cwgame_gwrbi                    /* 45 */
};

void cwgame_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char *buf;
  char output_line[1024];
  int i, j, t, comma = 0;
  CWGameIterator *gameiter = cw_gameiter_create(game);

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
      buf += (*function_ptrs[i])(buf, gameiter, visitors, home);
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
      buf += cwgame_final_pitcher(buf, game, t);
    }
    t++;
  }

  printf(output_line);
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
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwgame_print_help;

void
cwgame_print_field_list(void)
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

void (*cwtools_print_field_list)(void) = cwgame_print_field_list;

void
cwgame_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick expanded game descriptor, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwgame_print_welcome_message;

void
cwgame_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwgame_initialize;

void
cwgame_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwgame_cleanup;

