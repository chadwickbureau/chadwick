/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick boxscore generator program
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

char program_name[] = "cwbox";

CWBoxPlayer *cwbox_get_starter(CWBoxscore *boxscore, int team, int slot)
{
  CWBoxPlayer *player = boxscore->slots[slot][team];
  
  if (player == NULL) {
    return NULL;
  }

  while (player->prev != NULL) {
    player = player->prev;
  }

  return player;
}

void
cwbox_print_header(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  printf("Boxscore for %s %s at %s %s\n",
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
  printf("\n");
}

void
cwbox_print_player(CWBoxPlayer *player, CWRoster *roster)
{
  CWPlayer *bio = cw_roster_player_find(roster, player->player_id);
  char name[256];
  sprintf(name, (player->prev == NULL) ? "%s %s" : " %s %s",
	  bio->first_name, bio->last_name);

  printf("%-25s %2d %2d %2d %2d %2d %2d\n",
	 name, player->batting->ab, player->batting->r,
	 player->batting->h, player->batting->bi,
	 player->batting->bb, player->batting->so);
}

void cwbox_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  int i, t;

  CWBoxscore *boxscore = cw_boxscore_create(game);

  cwbox_print_header(game, visitors, home);
  for (t = 0; t <= 1; t++) {
    printf("%-25s ab  r  h bi bb so\n",
	   ((t == 0) ? visitors : home)->nickname);
    for (i = 1; i <= 9; i++) {
      CWBoxPlayer *player = cwbox_get_starter(boxscore, t, i);
      while (player != NULL) {
	cwbox_print_player(player, (t == 0) ? visitors : home);
	player = player->next;
      }
    }

    printf("\n");
  }

  printf("\f");
  cw_boxscore_cleanup(boxscore);
  free(boxscore);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwbox_process_game;

void cwbox_print_help(void)
{
  exit(0);
}

void (*cwtools_print_help)(void) = cwbox_print_help;

void
cwbox_print_field_list(void)
{
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwbox_print_field_list;

void
cwbox_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick boxscore generator, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwbox_print_welcome_message;
