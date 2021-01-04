/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/cwtools.c
 * Functions common to Chadwick command-line tools
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
#include <ctype.h>   /* for isdigit() */
#if HAVE_DIR_H
#include <dos.h>
#include <dir.h>
#elif defined(MSDOS)
#include <dir.h>
#endif /* HAVE_DIR_H/MSDOS */

#include "cwlib/chadwick.h"

/*************************************************************************
 * Global variables provided by individual programs
 *************************************************************************/

/* The maximum field number; field numbers start at zero */
extern unsigned int max_field;

/* An array of size max_field + 1, to hold field flags */
extern int fields[];

/* The name of the program */
extern char program_name[20];

/* Hook to function to print program identifying message */
extern void (*cwtools_print_welcome_message)(char *);

/* Hook to function to display field listing */
extern void (*cwtools_print_field_list)(void);

/* Hook to function to print help message */
extern void (*cwtools_print_help)(void);

/* Hook to function to parse command line */
extern int (*cwtools_parse_command_line)(int, char *argv[]);

/* Hook to function to process game */
extern void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *);

/* Hook to function to initialize data */
extern void (*cwtools_initialize)(void);

/* Hook to function to clean up data */
extern void (*cwtools_cleanup)(void);

/* Year for TEAMyyyy access */
char year[5] = "";
char first_date[5] = "0101";
char last_date[5] = "1231";
char game_id[20] = "";

int ascii = 1;

/* If 'quiet', programs should write no status messages to stderr */
int quiet = 0;

void
cwtools_read_rosters(CWLeague *league)
{
  char filename[256];
  FILE *teamfile;
  CWRoster *roster;

  sprintf(filename, "TEAM%s", year);

  teamfile = fopen(filename, "r");

  if (teamfile == NULL) {
    /* Also try lowercase version */
    sprintf(filename, "team%s", year);

    teamfile = fopen(filename, "r");
    
    if (teamfile == NULL) {
      fprintf(stderr, "Can't find teamfile (%s)\n", filename);
      exit(1);
    }
  }

  cw_league_read(league, teamfile);
  fclose(teamfile);

  for (roster = league->first_roster; roster; roster = roster->next) {
    FILE *file;

    sprintf(filename, "%s%s.ROS", roster->team_id, year);
    file = fopen(filename, "r");

    if (file == NULL) {
      /* bevent silently ignores missing roster files and generates
       * question marks for bats/throws for unknown players */
      continue;
    }

    cw_roster_read(roster, file);
    fclose(file);
  }
}

int
cwtools_game_in_range(CWGame *game, char *first_date, char *last_date)
{
  int month, day, year;
  char date_string[5];
  sscanf(cw_game_info_lookup(game, "date"), "%d/%d/%d", &year, &month, &day);
  sprintf(date_string, "%02d%02d", month, day);
  return (strcmp(date_string, first_date) >= 0 &&
	  strcmp(date_string, last_date) <= 0);
}

int
cwtools_select_game(CWGame *game)
{
  return ((!strcmp(game_id, "") ||
	   !strcmp(game_id, game->game_id)) &&
	  cwtools_game_in_range(game, first_date, last_date));
}

void
cwtools_iterate_games(CWScorebook *scorebook, CWLeague *league)
{
  CWScorebookIterator *iterator = cw_scorebook_iterate(scorebook,
						       cwtools_select_game);
  CWGame *game;

  while ((game = cw_scorebook_iterator_next(iterator)) != NULL) {
    (*cwtools_process_game)(game,
			    cw_league_roster_find(league,
						  cw_game_info_lookup(game,
								      "visteam")),
			    cw_league_roster_find(league,
						  cw_game_info_lookup(game,
								      "hometeam")));
  }
}

void
cwtools_process_scorebook(CWLeague *league, char *filename)
{
  CWScorebook *scorebook = cw_scorebook_create();
  FILE *file = fopen(filename, "r");

  if (!quiet) {
    fprintf(stderr, "[Processing file %s.]\n", filename);
  }

  if (!file || cw_scorebook_read(scorebook, file) < 0) {
    fprintf(stderr, "Warning: could not open file '%s'\n", filename);
  }
  else {
    cwtools_iterate_games(scorebook, league);
  }
  cw_scorebook_cleanup(scorebook);
  free(scorebook);
}

#if HAVE_DIR_H
void
cwtools_process_filespec(CWLeague *league, char *filespec)
{
  int handle;
  struct _finddata_t state;
  if ((handle = _findfirst(filespec, &state)) > 0) {
    do {
      cwtools_process_scorebook(league, state.name);
    } while (!_findnext(handle, &state));
  }
}
#elif defined(MSDOS)
void 
cwtools_process_filespec(CWLeague *league, char *filespec)
{
  struct ffblk state;
  int done = findfirst(filespec, &state, 0);

  while (!done) {
    cwtools_process_scorebook(league, state.ff_name);
    done = findnext(&state);
  }
}
#else  /* not HAVE_DIR_H/MSDOS */
void
cwtools_process_filespec(CWLeague *league, char *filespec)
{
  cwtools_process_scorebook(league, filespec);
}
#endif  /* HAVE_DIR_H/MSDOS */

void
cwtools_parse_field_list(char *text, int maxfield, int *field)
{
  unsigned int i = 0, j, firstNum, secondNum, err = 0;

  for (j = 0; j <= maxfield; field[j++] = 0);

  while (i < strlen(text)) {
    if (!isdigit(text[i])) {
      break;
    }

    firstNum = text[i++] - '0';
    while (isdigit(text[i])) {
      firstNum = firstNum * 10 + text[i++] - '0';
    }

    if (firstNum > maxfield) {
      break;
    }

    if (text[i] == '-') {
      i++;
      if (!isdigit(text[i])) {
	break;
      }

      secondNum = text[i++] - '0';
      while (isdigit(text[i])) {
	secondNum = secondNum * 10 + text[i++] - '0';
      }

      if (secondNum > maxfield || secondNum < firstNum) {
	err = 1;
	break;
      }
      
      for (j = firstNum; j <= secondNum; field[j++] = 1);
    }
    else {
      field[firstNum] = 1;
    }

    if (text[i] == ',') {
      i++;
    }
    /* Anything else is an error; will be caught at beginning of
     * next iteration of loop */
  }

  if (i < strlen(text) || err) {
    fprintf(stderr,
	    "\n*** Invalid field spec.  A field spec is a list of fields\n");
    fprintf(stderr,
	    "and ranges, separated by commas.  No spaces are allowed.\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s -f 0-4,7,12,20-31\n", program_name);
    fprintf(stderr,
	    "The spec is invalid if any value is larger than the max\n");
    fprintf(stderr, "field number, %d.\n", maxfield);
    exit(1);
  }
}

int
cwtools_default_parse_command_line(int argc, char *argv[])
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
    else if (!strcmp(argv[i], "-ft")) {
      ascii = 0;
    }
    else if (!strcmp(argv[i], "-s")) {
      if (++i < argc) {
	strncpy(first_date, argv[i], 4);
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

int main(int argc, char *argv[])
{
  int i;
  CWLeague *league = cw_league_create();

  i = cwtools_parse_command_line(argc, argv);
  if (!quiet) {
    (*cwtools_print_welcome_message)(argv[0]);
  }
  cwtools_read_rosters(league);
  cwtools_initialize();
  for (; i < argc; i++) {
    cwtools_process_filespec(league, argv[i]);
  }
  cwtools_cleanup();
  cw_league_cleanup(league);
  free(league);

  return 0;
}
