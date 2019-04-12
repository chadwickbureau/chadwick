/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwtools/cwsub.c
 * Chadwick substitution descriptor program
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
int fields[10] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

int max_field = 9;

char program_name[20] = "cwsub";

int print_header = 0;

/*************************************************************************
 * Functions to output fields
 *************************************************************************/

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *, CWAppearance *);

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
int funcname(char *buffer, CWGameIterator *gameiter, CWAppearance *sub)

/* Field 0 */
DECLARE_FIELDFUNC(cwsub_game_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-12s",
		 gameiter->game->game_id);
}

/* Field 1 */
DECLARE_FIELDFUNC(cwsub_inning)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 gameiter->event->inning);
}

/* Field 2 */
DECLARE_FIELDFUNC(cwsub_batting_team)
{
  return sprintf(buffer, "%d", gameiter->event->batting_team);
}

/* Field 3 */
DECLARE_FIELDFUNC(cwsub_player)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", sub->player_id);
}

/* Field 4 */
DECLARE_FIELDFUNC(cwsub_team)
{
  return sprintf(buffer, "%d", sub->team);
}

/* Field 5 */
DECLARE_FIELDFUNC(cwsub_slot)
{
  return sprintf(buffer, "%d", sub->slot);
}

/* Field 6 */
DECLARE_FIELDFUNC(cwsub_position)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", sub->pos);
}

/* Field 7 */
DECLARE_FIELDFUNC(cwsub_removed_player)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->lineups[sub->slot][sub->team].player_id);
}

/* Field 8 */
DECLARE_FIELDFUNC(cwsub_removed_position)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d",
		 gameiter->state->lineups[sub->slot][sub->team].position);
}

/* Field 9 */
DECLARE_FIELDFUNC(cwsub_event_number)
{
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 (!strcmp(gameiter->event->event_text, "NP")) ?
		 gameiter->state->event_count : gameiter->state->event_count + 1);
}

static field_struct field_data[] = {
  { cwsub_game_id, "GAME_ID", "game id" },
  { cwsub_inning, "INN_CT", "inning" },
  { cwsub_batting_team, "BAT_HOME_ID", "batting team" },
  { cwsub_player, "SUB_ID", "substitute" },
  { cwsub_team, "SUB_HOME_ID", "team" },
  { cwsub_slot, "SUB_LINEUP_ID", "lineup position" },
  { cwsub_position, "SUB_FLD_CD", "fielding position" },
  { cwsub_removed_player, "REMOVED_ID", "removed player" },
  { cwsub_removed_position, "REMOVED_FLD_CD", "position of removed player" },
  { cwsub_event_number, "EVENT_ID", "event number" }
};

void
cwsub_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char *buf;
  char output_line[1024];
  int i, comma;
  CWGameIterator *gameiter = cw_gameiter_create(game);

  while (gameiter->event != NULL) {
    CWAppearance *sub = gameiter->event->first_sub;
    while (sub) {
      comma = 0;
      strcpy(output_line, "");
      buf = output_line;
      for (i = 0; i < 10; i++) {
	if (fields[i]) {
	  if (ascii && comma) {
	    *(buf++) = ',';
	  }
	  else {
	    comma = 1;
	  }
	  buf += (*field_data[i].f)(buf, gameiter, sub);
	}
      }

      printf("%s", output_line);
      printf("\n");
      sub = sub->next;
    }

    cw_gameiter_next(gameiter);
  }
  
  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwsub_process_game;

void
cwsub_print_help(void)
{
  fprintf(stderr, "\n\ncwsub generates files suitable for use by dBase or Lotus-like programs\n");
  fprintf(stderr, "Each record describes one substitution.\n");
  fprintf(stderr, "Usage: cwsub [options] eventfile...\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "  -h        print this help\n");
  fprintf(stderr, "  -i id     only process game given by id\n");
  fprintf(stderr, "  -y year   Year to process (for teamyyyy and aaayyyy.ros).\n");
  fprintf(stderr, "  -s start  Earliest date to process (mmdd).\n");
  fprintf(stderr, "  -e end    Last date to process (mmdd).\n");
  fprintf(stderr, "  -a        generate Ascii-delimited format files (default)\n");
  fprintf(stderr, "  -ft       generate Fortran format files\n");
  fprintf(stderr, "  -m        use master player file instead of local roster files\n");
  fprintf(stderr, "  -f flist  give list of fields to output\n");
  fprintf(stderr, "              Default is 0-9.\n");
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");
  fprintf(stderr, "  -n        print field names in first row of output\n\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwsub_print_help;

void
cwsub_print_field_list(void)
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
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwsub_print_field_list;

void
cwsub_print_welcome_message(char *argv0)
{
  fprintf(stderr,
	  "\nChadwick substitute descriptor, version " VERSION); 
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "Copyright (c) 2002-2019\nDr T L Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)\n");
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwsub_print_welcome_message;

void
cwsub_initialize(void)
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

  printf("%s", output_line);
  printf("\n");
}

void (*cwtools_initialize)(void) = cwsub_initialize;

void
cwsub_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwsub_cleanup;


extern char year[5];
extern char first_date[5];
extern char last_date[5];
extern char game_id[20];
extern int ascii;
extern int quiet;

extern void
cwtools_parse_field_list(char *text, int max_field, int *fields);

int
cwsub_parse_command_line(int argc, char *argv[])
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

int (*cwtools_parse_command_line)(int, char *argv[]) = cwsub_parse_command_line;
