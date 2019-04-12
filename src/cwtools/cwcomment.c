/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwtools/cwcomment.c
 * Chadwick comment extractor program
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
int fields[3] = {
  1, 1, 1,
};

int max_field = 2;

char program_name[20] = "cwcomment";

/*************************************************************************
 * Functions to output fields
 *************************************************************************/

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *, int beginning);

/*
 * preprocessor directive for conveniently declaring function signature
 */

#define DECLARE_FIELDFUNC(funcname) \
  int funcname(char *buffer, CWGameIterator *gameiter, int beginning)

/* Field 0 */
DECLARE_FIELDFUNC(cwcomment_game_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-12s",
		 gameiter->game->game_id);
}

/* Field 1 */
DECLARE_FIELDFUNC(cwcomment_event_number)
{
  if (beginning) {
    return sprintf(buffer, (ascii) ? "%d" : "%3d", 0);
  }
  else {
    return sprintf(buffer, (ascii) ? "%d" : "%3d",
		   (!strcmp(gameiter->event->event_text, "NP")) ?
		   gameiter->state->event_count : gameiter->state->event_count + 1);
  }
}


/* Field 2 */
DECLARE_FIELDFUNC(cwcomment_comment)
{
  int chars = 0;
  CWComment *comment;

  if (ascii) {
    chars += sprintf(buffer, "\"");
    buffer += 1;
  }

  for (comment = 
	 (beginning) ? gameiter->game->first_comment :
	 gameiter->event->first_comment; 
       comment; comment = comment->next) {
    if (comment != gameiter->event->first_comment &&
	comment != gameiter->game->first_comment) {
      chars += sprintf(buffer, " ");
      buffer += 1;
    }

    chars += sprintf(buffer, "%s", comment->text);
    buffer += strlen(comment->text);
  }

  if (ascii) {
    chars += sprintf(buffer, "\"");
    buffer += 1;
  }

  return chars;
}

static field_func function_ptrs[] = {
  cwcomment_game_id,                 /* 0 */
  cwcomment_event_number,            /* 1 */
  cwcomment_comment                  /* 2 */
};

void
cwcomment_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char *buf;
  char output_line[4096];
  int i, comma;
  CWGameIterator *gameiter = cw_gameiter_create(game);

  if (gameiter->game->first_comment != NULL) {
    comma = 0;
    strcpy(output_line, "");
    buf = output_line;
    for (i = 0; i < 3; i++) {
      if (fields[i]) {
	if (ascii && comma) {
	  *(buf++) = ',';
	}
	else {
	  comma = 1;
	}
	buf += (*function_ptrs[i])(buf, gameiter, 1);
      }
    }

    printf("%s", output_line);
    printf("\n");
  }

  while (gameiter->event != NULL) {
    if (gameiter->event->first_comment == NULL) {
      cw_gameiter_next(gameiter);
      continue;
    }

    comma = 0;
    strcpy(output_line, "");
    buf = output_line;
    for (i = 0; i < 3; i++) {
      if (fields[i]) {
	if (ascii && comma) {
	  *(buf++) = ',';
	}
	else {
	  comma = 1;
	}
	buf += (*function_ptrs[i])(buf, gameiter, 0);
      }
    }

    printf("%s", output_line);
    printf("\n");

    cw_gameiter_next(gameiter);
  }
  
  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwcomment_process_game;

void
cwcomment_print_help(void)
{
  fprintf(stderr, "\n\ncwcomment generates files suitable for use by dBase or Lotus-like programs\n");
  fprintf(stderr, "Each record contains one comment from the event file.\n");
  fprintf(stderr, "Usage: cwcomment [options] eventfile...\n");
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
  fprintf(stderr, "              Default is 0-2.\n");
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwcomment_print_help;

void
cwcomment_print_field_list(void)
{
  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  All are included by default.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  fprintf(stderr, "0       game id\n");
  fprintf(stderr, "1       event number\n");
  fprintf(stderr, "2       comment text\n");
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwcomment_print_field_list;

void
cwcomment_print_welcome_message(char *argv0)
{
  fprintf(stderr,
	  "\nChadwick comment extractor, version " VERSION); 
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "Copyright (c) 2002-2019\nDr T L Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)\n");
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwcomment_print_welcome_message;

void
cwcomment_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwcomment_initialize;

void
cwcomment_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwcomment_cleanup;


extern int cwtools_default_parse_command_line(int, char *argv[]);
int (*cwtools_parse_command_line)(int, char *argv[]) = cwtools_default_parse_command_line;
