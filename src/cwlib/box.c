/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Declaration of boxscore data structures and API
 * 
 * This file is part of Chadwick, a library for baseball play-by-play and stats
 * Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chadwick.h"

/************************************************************************
 * Private routines for dealing with auxiliary struct init and dealloc
 ************************************************************************/

static CWBoxPlayer *
cw_boxscore_player_create(char *player_id)
{
  CWBoxPlayer *player = (CWBoxPlayer *) malloc(sizeof(CWBoxPlayer));
  player->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(player->player_id, player_id);
  player->batting = (CWBoxBatting *) malloc(sizeof(CWBoxBatting));
  player->batting->ab = 0;
  player->batting->r = 0;
  player->batting->h = 0;
  player->batting->bi = 0;
  player->batting->bb = 0;
  player->batting->so = 0;
  player->prev = NULL;
  player->next = NULL;
  return player;
}

static void
cw_boxscore_player_cleanup(CWBoxPlayer *player)
{
  free(player->batting);
  free(player->player_id);
}

/*
 * Initialize slots with starting players
 */
static void
cw_boxscore_enter_starters(CWBoxscore *boxscore, CWGame *game)
{
  int i, t;

  for (t = 0; t <= 1; t++) {
    for (i = 1; i <= 9; i++) {
      CWAppearance *app = cw_game_starter_find(game, t, i);
      boxscore->slots[i][t] = cw_boxscore_player_create(app->player_id);
    }
  }
}

/*
 * Add a substitute into a slot
 */
static void
cw_boxscore_add_substitute(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  CWAppearance *sub = gameiter->event->first_sub;

  while (sub != NULL) {
    if (strcmp(sub->player_id, 
	       boxscore->slots[sub->slot][sub->team]->player_id)) {
      CWBoxPlayer *player = cw_boxscore_player_create(sub->player_id);
      boxscore->slots[sub->slot][sub->team]->next = player;
      player->prev = boxscore->slots[sub->slot][sub->team];
      boxscore->slots[sub->slot][sub->team] = player;
    }
    sub = sub->next;
  }
}

CWBoxPlayer *
cw_boxscore_find_player(CWBoxscore *boxscore, char *player_id)
{
  int i, t;

  for (t = 0; t <= 1; t++) {
    for (i = 1; i <= 9; i++) {
      if (!strcmp(boxscore->slots[i][t]->player_id, player_id)) {
	return boxscore->slots[i][t];
      }
    }
  }

  return NULL;
}

static void
cw_boxscore_batter_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  CWParsedEvent *event_data = gameiter->event_data;
  CWBoxPlayer *player;

  if (!strcmp(gameiter->event->batter, "NP")) {
    return;
  }

  player = cw_boxscore_find_player(boxscore, gameiter->event->batter);

  if (cw_event_is_official_ab(event_data)) {
    player->batting->ab++;

    if (event_data->event_type >= EVENT_SINGLE &&
	event_data->event_type <= EVENT_HOMERUN) {
      player->batting->h++;
    }
    else if (event_data->event_type == EVENT_STRIKEOUT) {
      player->batting->so++;
    }
  }
  else if (event_data->event_type == EVENT_WALK ||
	   event_data->event_type == EVENT_INTENTIONALWALK) {
    player->batting->bb++;
  }

  if (event_data->advance[0] >= 4) {
    player->batting->r++;
  }
  player->batting->bi += cw_event_rbi_on_play(event_data);
}

static void
cw_boxscore_runner_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  int base;
  CWBoxPlayer *player;

  for (base = 1; base <= 3; base++) {
    if (!strcmp(gameiter->runners[base], "")) {
      continue;
    }

    player = cw_boxscore_find_player(boxscore, gameiter->runners[base]);

    if (gameiter->event_data->advance[base] >= 4) {
      player->batting->r++;
    }
  }
}

static void
cw_boxscore_iterate_game(CWBoxscore *boxscore, CWGame *game)
{
  CWGameIterator *gameiter = cw_gameiter_create(game);

  while (gameiter->event != NULL) {
    cw_boxscore_batter_stats(boxscore, gameiter);
    cw_boxscore_runner_stats(boxscore, gameiter);
    cw_boxscore_add_substitute(boxscore, gameiter);
    cw_gameiter_next(gameiter);
  }

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

CWBoxscore *
cw_boxscore_create(CWGame *game)
{
  int i, t;
  CWBoxscore *boxscore = (CWBoxscore *) malloc(sizeof(CWBoxscore));

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      boxscore->slots[i][t] = NULL;
    }
  }

  cw_boxscore_enter_starters(boxscore, game);
  cw_boxscore_iterate_game(boxscore, game);

  return boxscore;
}

void
cw_boxscore_cleanup(CWBoxscore *boxscore)
{
  int i, t;
  
  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      CWBoxPlayer *player = boxscore->slots[i][t];

      while (player != NULL) {
	CWBoxPlayer *prev_player = player->prev;
	cw_boxscore_player_cleanup(player);
	free(player);
	player = prev_player;
      }

      boxscore->slots[i][t] = NULL;
    }
  }
}


