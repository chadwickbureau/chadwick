/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick statistics generation program
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

int fields[1] = { 0 };  

int max_field = 0;

char program_name[] = "cwstats";

typedef struct cw_stats_player_struct {
  char *player_id;
  char *team;
  CWBoxBatting *batting;
  struct cw_stats_player_struct *prev, *next;
} CWStatsPlayer;

CWStatsPlayer *first_player = NULL, *last_player = NULL;

CWStatsPlayer *
cwstats_copy_player(CWBoxPlayer *player)
{
  CWStatsPlayer *copy = (CWStatsPlayer *) malloc(sizeof(CWStatsPlayer));
  copy->player_id = (char *) malloc(sizeof(char) * 
				    (strlen(player->player_id) + 1));
  strcpy(copy->player_id, player->player_id);
  copy->batting = cw_boxscore_batting_create();
  copy->batting->ab = player->batting->ab;
  copy->batting->r = player->batting->r;
  copy->batting->h = player->batting->h;
  copy->batting->bi = player->batting->bi;
  copy->batting->bb = player->batting->bb;
  copy->batting->so = player->batting->so;
  copy->prev = copy->next = NULL;
  return copy;
}

void
cwstats_add_batting_stats(CWBoxPlayer *player)
{
  CWStatsPlayer *iter = first_player;

  while (iter != NULL) {
    if (!strcmp(iter->player_id, player->player_id)) {
      /* aggregate statistics */
      cw_boxscore_batting_add(iter->batting, player->batting);
      return;
    }

    iter = iter->next;
  }

  /* If we get here, player not found; add him */
  if (last_player != NULL) {
    last_player->next = cwstats_copy_player(player);
    last_player->next->prev = last_player;
    last_player = last_player->next;
  }
  else {
    last_player = cwstats_copy_player(player);
    first_player = last_player;
  }
}

void
cwstats_print_batting_stats(void)
{
  CWStatsPlayer *player = first_player;

  printf("ID        AB   R   H 2B 3B HR  BI  BB IW  SO DP HP SH SF\n");

  while (player != NULL) {
    printf("%8s %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d %2d %2d %2d %2d\n",
	   player->player_id,
	   player->batting->ab, player->batting->r, player->batting->h,
	   player->batting->b2, player->batting->b3, player->batting->hr,
	   player->batting->bi,
	   player->batting->bb, player->batting->ibb, player->batting->so,
	   player->batting->gdp, player->batting->hp,
	   player->batting->sh, player->batting->sf);
    
    player = player->next;
  }
}

void
cwstats_process_game(CWGame *game, CWRoster *visitors, CWRoster *home) 
{
  int i, t;
  CWBoxscore *boxscore = cw_boxscore_create(game);

  for (t = 0; t <= 1; t++) {
    for (i = 1; i <= 9; i++) {
      CWBoxPlayer *player = boxscore->slots[i][t];

      while (player != NULL) {
	cwstats_add_batting_stats(player);
	/* we iterate _backwards_, since slots actually contains the
	 * tail of the list of players */
	player = player->prev;
      }
    }
  }

  cw_boxscore_cleanup(boxscore);
  free(boxscore);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwstats_process_game;

void
cwstats_print_help(void)
{
  exit(0);
}

void (*cwtools_print_help)(void) = cwstats_print_help;

void
cwstats_print_field_list(void)
{
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwstats_print_field_list;

void
cwstats_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick statistics generator, version " VERSION); 
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, " 
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwstats_print_welcome_message;

void
cwstats_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwstats_initialize;

void
cwstats_cleanup(void)
{
  cwstats_print_batting_stats();
}

void (*cwtools_cleanup)(void) = cwstats_cleanup;


