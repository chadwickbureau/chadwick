/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick play-by-play generator program
 * 
 * This file is part of Chadwick, tools for baseball play-by-play and stats
 * Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
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
int fields[82] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1
};

int max_field = 81;

char program_name[] = "cwpbp";

void
cwpbp_print_header(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  printf("Play-by-play for %s %s at %s %s\n",
	 visitors->city, visitors->nickname,
	 home->city, home->nickname);
  printf("Game played on %s",
	 cw_game_info_lookup(game, "date"));
  switch (atoi(cw_game_info_lookup(game, "number"))) {
  case 1:
    printf(" (first game)\n");
    break;
  case 2:
    printf(" (second game)\n");
    break;
  case 3:
    printf(" (third game)\n");
    break;
  default:
    printf("\n");
    break;
  }
}

void
cwpbp_print_defensive_credit(CWGameIterator *gameiter,
			     CWRoster *defense, int base)
{
  int i;

  printf("(");
  for (i = 0; i < strlen(gameiter->event_data->play[base]); i++) {
    if (i > 0) {
      printf(" to ");
    }
    printf("%s",
	   cw_roster_player_find(defense, gameiter->fielders[gameiter->event_data->play[base][i] - '0'][1-gameiter->half_inning])->last_name);
      }
  if (strlen(gameiter->event_data->play[base]) == 1) {
    printf(" unassisted");
  } 
  printf(")");
}

void
cwpbp_process_generic_out(CWGameIterator *gameiter,
			  CWRoster *visitors, CWRoster *home)
{
  CWRoster *defense = ((gameiter->half_inning == 0) ? home : visitors);
  CWRoster *offense = ((gameiter->half_inning == 0) ? visitors : home);

  if (strcmp(gameiter->event_data->play[0], "")) {
    switch (gameiter->event_data->batted_ball_type) {
    case 'G':
      printf(" grounded out ");
      cwpbp_print_defensive_credit(gameiter, defense, 0);
      break;
    case 'F':
      printf(" flied out to %s",
	     cw_roster_player_find(defense, gameiter->fielders[gameiter->event_data->play[0][0] - '0'][1-gameiter->half_inning])->last_name);
      break;
    case 'L':
      printf(" lined out to %s",
	     cw_roster_player_find(defense, gameiter->fielders[gameiter->event_data->play[0][0] - '0'][1-gameiter->half_inning])->last_name);
      break;
    case 'P':
      printf(" popped out to %s",
	     cw_roster_player_find(defense, gameiter->fielders[gameiter->event_data->play[0][0] - '0'][1-gameiter->half_inning])->last_name);
      break;
    default:
      printf(" made an out ");
      break;
    }
  }
  else if (gameiter->event_data->fc_flag[1]) {
    printf(" forced %s ",
	   cw_roster_player_find(offense, gameiter->runners[1])->last_name);
    cwpbp_print_defensive_credit(gameiter, defense, 1);
  }
  else {
    printf(" was put out");
  }
}

void
cwpbp_print_hit_fielder(CWGameIterator *gameiter, CWRoster *defense)
{
  if (gameiter->event_data->fielded_by > 0) {
    printf(" to %s",
	   cw_roster_player_find(defense, gameiter->fielders[gameiter->event_data->fielded_by][1-gameiter->half_inning])->last_name);
  }
}

void
cwpbp_print_advance(CWGameIterator *gameiter, 
		    CWRoster *visitors, CWRoster *home)
{
  int base, comma = 0, bracket = 0;

  for (base = 3; base >= 1; base--) {
    CWPlayer *player;

    if (!strcmp(gameiter->runners[base], "")) {
      continue;
    }

    if (gameiter->event_data->advance[base] == base) {
      continue;
    }

    if (gameiter->event_data->fc_flag[base]) {
      continue;
    }

    if (!bracket) {
      printf(" [");
      bracket = 1;
    }
      
    if (comma) {
      printf(", ");
    }
    else {
      comma = 1;
    }
    player = cw_roster_player_find((gameiter->half_inning == 0) ? visitors : home, gameiter->runners[base]);
    printf("%s %s ", player->first_name, player->last_name);
    switch (gameiter->event_data->advance[base]) {
    case 0:
      printf("??");
      break;
    case 1:
      printf("to first");
      break;
    case 2:
      printf("to second");
      break;
    case 3:
      printf("to third");
      break;
    case 4:
      printf("scored");
      break;
    case 5:
      printf("scored (unearned)");
      break;
    case 6:
      printf("scored (unearned to team)");
      break;
    default:
      printf("??");
      break;
    }

    if (gameiter->event_data->advance[base] >= 4 &&
	gameiter->event_data->rbi_flag[base] == 0) {
      printf(" (no RBI)");
    }
  }

  if (bracket) {
    printf("]");
  }
}

void
cwpbp_print_text(CWGameIterator *gameiter, CWRoster *visitors, CWRoster *home)
{
  if (strcmp(gameiter->event->event_text, "NP")) {
    if (cw_event_is_batter(gameiter->event_data)) {
      CWPlayer *batter = cw_roster_player_find((gameiter->event->half_inning == 0) ? visitors : home, gameiter->event->batter);

      printf("%s %s", batter->first_name, batter->last_name);
      switch (gameiter->event_data->event_type) {
      case EVENT_SINGLE:
	printf(" singled");
	cwpbp_print_hit_fielder(gameiter,
				(gameiter->half_inning == 0) ? home : visitors);
	break;
      case EVENT_DOUBLE:
	printf(" doubled");
	cwpbp_print_hit_fielder(gameiter,
				(gameiter->half_inning == 0) ? home : visitors);
	break;
      case EVENT_TRIPLE:
	printf(" tripled");
	cwpbp_print_hit_fielder(gameiter,
				(gameiter->half_inning == 0) ? home : visitors);
	break;
      case EVENT_HOMERUN:
	printf(" homered");
	cwpbp_print_hit_fielder(gameiter,
				(gameiter->half_inning == 0) ? home : visitors);
	break;
      case EVENT_WALK:
	printf(" walked");
	break;
      case EVENT_INTENTIONALWALK:
	printf(" was intentionally walked");
	break;
      case EVENT_STRIKEOUT:
	printf(" struck out");
	break;
      case EVENT_HITBYPITCH:
	printf(" was hit by a pitch");
	break;
      case EVENT_GENERICOUT:
	cwpbp_process_generic_out(gameiter, visitors, home);
	break;
      case EVENT_ERROR:
	printf(" reached on an error");
	break;
      case EVENT_INTERFERENCE:
	printf(" was awarded first on interference");
	break;
      case EVENT_FIELDERSCHOICE:
	printf(" reached on a fielder's choice");
	break;
      default:
	printf(" (did something)");
	break;
      }
      cwpbp_print_advance(gameiter, visitors, home);
      printf("; ");
    }
    return;
  }
}

void cwpbp_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  CWGameIterator *gameiter = cw_gameiter_create(game);
  int inning = 0, half_inning = 0;
  cwpbp_print_header(game, visitors, home);

  while (gameiter->event != NULL) {
    if (inning != gameiter->inning || half_inning != gameiter->half_inning) {
      printf("\n\n");
      inning = gameiter->inning;
      half_inning = gameiter->half_inning;
      if (inning % 10 == 1) {
	printf("%s %dST: ", ((half_inning == 0) ? visitors : home)->nickname,
	       inning);
      }
      else if (inning % 10 == 2) {
	printf("%s %dND: ", ((half_inning == 0) ? visitors : home)->nickname,
	       inning);
      }
      else if (inning % 10 == 3) {
	printf("%s %dRD: ", ((half_inning == 0) ? visitors : home)->nickname,
	       inning);
      }
      else {
	printf("%s %dTH: ", ((half_inning == 0) ? visitors : home)->nickname,
	       inning);
      }
    }
    cwpbp_print_text(gameiter, visitors, home);
    cw_gameiter_next(gameiter);
  }

  printf("\n\f");
  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwpbp_process_game;

void cwpbp_print_help(void)
{
  exit(0);
}

void (*cwtools_print_help)(void) = cwpbp_print_help;

void
cwpbp_print_field_list(void)
{
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwpbp_print_field_list;

void
cwpbp_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick play-by-play generator, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwpbp_print_welcome_message;

void
cwpbp_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwpbp_initialize;

void
cwpbp_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwpbp_cleanup;

