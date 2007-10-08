/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick substitution descriptor program
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
int fields[10] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

int max_field = 9;

char program_name[20] = "cwsub";

/*************************************************************************
 * Functions to output fields
 *************************************************************************/

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *, CWAppearance *);

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

static field_func function_ptrs[] = {
  cwsub_game_id,                 /* 0 */
  cwsub_inning,                  /* 1 */
  cwsub_batting_team,            /* 2 */
  cwsub_player,                  /* 3 */
  cwsub_team,                    /* 4 */
  cwsub_slot,                    /* 5 */
  cwsub_position,                /* 6 */
  cwsub_removed_player,          /* 7 */
  cwsub_removed_position,        /* 8 */
  cwsub_event_number             /* 9 */
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
	  buf += (*function_ptrs[i])(buf, gameiter, sub);
	}
      }

      printf(output_line);
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
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwsub_print_help;

void
cwsub_print_field_list(void)
{
  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  All are included by default.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  fprintf(stderr, "0       game id\n");
  fprintf(stderr, "1       inning\n");
  fprintf(stderr, "2       batting team\n");
  fprintf(stderr, "3       substitute\n");
  fprintf(stderr, "4       team\n");
  fprintf(stderr, "5       lineup position\n");
  fprintf(stderr, "6       fielding position\n");
  fprintf(stderr, "7       removed player\n");
  fprintf(stderr, "8       position of removed player\n");
  fprintf(stderr, "9       event number\n");
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwsub_print_field_list;

void
cwsub_print_welcome_message(char *argv0)
{
  fprintf(stderr,
	  "\nChadwick substitute descriptor, version " VERSION); 
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwsub_print_welcome_message;

void
cwsub_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwsub_initialize;

void
cwsub_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwsub_cleanup;


extern int cwtools_default_parse_command_line(int, char *argv[]);
int (*cwtools_parse_command_line)(int, char *argv[]) = cwtools_default_parse_command_line;
