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

CWBoxBatting *
cw_boxscore_batting_create(void)
{
  CWBoxBatting *batting = (CWBoxBatting *) malloc(sizeof(CWBoxBatting));
  batting->ab = 0;
  batting->r = 0;
  batting->h = 0;
  batting->b2 = 0;
  batting->b3 = 0;
  batting->hr = 0;
  batting->bi = 0;
  batting->bb = 0;
  batting->ibb = 0;
  batting->so = 0;
  batting->gdp = 0;
  batting->hp = 0;
  batting->sh = 0;
  batting->sf = 0;
  batting->sb = 0;
  batting->cs = 0;
  return batting;
}

void
cw_boxscore_batting_add(CWBoxBatting *dest, CWBoxBatting *src)
{
  dest->ab += src->ab;
  dest->r += src->r;
  dest->h += src->h;
  dest->b2 += src->b2;
  dest->b3 += src->b3;
  dest->hr += src->hr;
  dest->bi += src->bi;
  dest->bb += src->bb;
  dest->ibb += src->ibb;
  dest->so += src->so;
  dest->gdp += src->gdp;
  dest->hp += src->hp;
  dest->sh += src->sh;
  dest->sf += src->sf;
  dest->sb += src->sb;
  dest->cs += src->cs;
}

CWBoxPitching *
cw_boxscore_pitching_create(void)
{
  CWBoxPitching *pitching = (CWBoxPitching *) malloc(sizeof(CWBoxPitching));
  pitching->outs = 0;
  pitching->r = 0;
  pitching->er = 0;
  pitching->h = 0;
  pitching->hr = 0;
  pitching->bb = 0;
  pitching->ibb = 0;
  pitching->so = 0;
  pitching->bf = 0; 
  pitching->wp = 0;
  pitching->bk = 0;
}

void
cw_boxscore_pitching_add(CWBoxPitching *dest, CWBoxPitching *src)
{
  dest->outs += src->outs;
  dest->r += src->r;
  dest->er += src->er;
  dest->h += src->h;
  dest->hr += src->hr;
  dest->bb += src->bb;
  dest->ibb += src->ibb;
  dest->so += src->so;
  dest->bf += src->bf;
  dest->wp += src->wp;
  dest->bk += src->bk;
}

/************************************************************************
 * Private routines for dealing with auxiliary struct init and dealloc
 ************************************************************************/

static CWBoxPlayer *
cw_boxscore_player_create(char *player_id)
{
  CWBoxPlayer *player = (CWBoxPlayer *) malloc(sizeof(CWBoxPlayer));
  player->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(player->player_id, player_id);
  player->batting = cw_boxscore_batting_create();
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

static CWBoxPitcher *
cw_boxscore_pitcher_create(char *player_id)
{
  CWBoxPitcher *pitcher = (CWBoxPitcher *) malloc(sizeof(CWBoxPitcher));
  pitcher->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(pitcher->player_id, player_id);
  pitcher->pitching = cw_boxscore_pitching_create();
  pitcher->prev = NULL;
  pitcher->next = NULL;
  return pitcher;
}

static void
cw_boxscore_pitcher_cleanup(CWBoxPitcher *pitcher)
{
  free(pitcher->pitching);
  free(pitcher->player_id);
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
      if (app->pos == 1) {
	boxscore->pitchers[t] = cw_boxscore_pitcher_create(app->player_id);
      }
    }

    if (!strcmp(cw_game_info_lookup(game, "usedh"), "true")) {
      CWAppearance *app = cw_game_starter_find(game, t, 0);
      boxscore->pitchers[t] = cw_boxscore_pitcher_create(app->player_id);
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
    if (sub->slot > 0 &&
	strcmp(sub->player_id, 
	       boxscore->slots[sub->slot][sub->team]->player_id)) {
      CWBoxPlayer *player = cw_boxscore_player_create(sub->player_id);
      boxscore->slots[sub->slot][sub->team]->next = player;
      player->prev = boxscore->slots[sub->slot][sub->team];
      boxscore->slots[sub->slot][sub->team] = player;
    }
    if (sub->pos == 1) {
      CWBoxPitcher *pitcher = cw_boxscore_pitcher_create(sub->player_id);
      boxscore->pitchers[sub->team]->next = pitcher;
      pitcher->prev = boxscore->pitchers[sub->team];
      boxscore->pitchers[sub->team] = pitcher;
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

CWBoxPitcher *
cw_boxscore_find_pitcher(CWBoxscore *boxscore, char *player_id)
{
  int t;

  for (t = 0; t <= 1; t++) {
    CWBoxPitcher *pitcher = boxscore->pitchers[t];
    while (pitcher != NULL && strcmp(pitcher->player_id, player_id)) {
      pitcher = pitcher->prev;
    }

    if (pitcher != NULL) {
      return pitcher;
    }
  }

  return NULL;
}

static void
cw_boxscore_batter_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  CWParsedEvent *event_data = gameiter->event_data;
  CWBoxPlayer *player;
  CWBoxPitcher *pitcher;

  player = cw_boxscore_find_player(boxscore, gameiter->event->batter);
  pitcher = boxscore->pitchers[1-gameiter->half_inning];

  if (cw_event_is_batter(event_data)) {
    pitcher->pitching->bf++;
  }
  pitcher->pitching->outs += cw_event_outs_on_play(event_data);

  if (cw_event_is_official_ab(event_data)) {
    player->batting->ab++;

    if (event_data->event_type >= EVENT_SINGLE &&
	event_data->event_type <= EVENT_HOMERUN) {
      player->batting->h++;
      pitcher->pitching->h++;
      if (event_data->event_type == EVENT_DOUBLE) {
	player->batting->b2++;
      }
      else if (event_data->event_type == EVENT_TRIPLE) {
	player->batting->b3++;
      }
      else if (event_data->event_type == EVENT_HOMERUN) {
	player->batting->hr++;
	pitcher->pitching->hr++;
      }
    }
    else if (event_data->event_type == EVENT_STRIKEOUT) {
      player->batting->so++;
      pitcher->pitching->so++;
    }
    else if (event_data->gdp_flag) {
      player->batting->gdp++;
    }
  }
  else if (event_data->event_type == EVENT_WALK ||
	   event_data->event_type == EVENT_INTENTIONALWALK) {
    player->batting->bb++;
    pitcher->pitching->bb++;
    if (event_data->event_type == EVENT_INTENTIONALWALK) {
      player->batting->ibb++;
      pitcher->pitching->ibb++;
    }
  }
  else if (event_data->event_type == EVENT_HITBYPITCH) {
    player->batting->hp++;
  }
  else if (event_data->event_type == EVENT_BALK) {
    pitcher->pitching->bk++;
  }
  else if (event_data->wp_flag) {
    pitcher->pitching->wp++;
  }
  else if (event_data->sh_flag) {
    player->batting->sh++;
  }
  else if (event_data->sf_flag) {
    player->batting->sf++;
  }

  if (event_data->advance[0] >= 4) {
    player->batting->r++;
    pitcher->pitching->r++;
    if (event_data->advance[0] != 5) {
      pitcher->pitching->er++;
    }
  }
  player->batting->bi += cw_event_rbi_on_play(event_data);
}

static void
cw_boxscore_runner_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  int base;
  CWBoxPlayer *player;
  CWBoxPitcher *pitcher;

  for (base = 1; base <= 3; base++) {
    if (!strcmp(gameiter->runners[base], "")) {
      continue;
    }

    player = cw_boxscore_find_player(boxscore, gameiter->runners[base]);
    pitcher = cw_boxscore_find_pitcher(boxscore, gameiter->pitchers[base]);

    if (gameiter->event_data->advance[base] >= 4) {
      player->batting->r++;
      pitcher->pitching->r++;
      if (gameiter->event_data->advance[base] != 5) {
	pitcher->pitching->er++;
      }
    }

    if (gameiter->event_data->sb_flag[base]) {
      player->batting->sb++;
    }

    if (gameiter->event_data->cs_flag[base]) {
      player->batting->cs++;
    }
  }
}

static void
cw_boxscore_iterate_game(CWBoxscore *boxscore, CWGame *game)
{
  CWGameIterator *gameiter = cw_gameiter_create(game);

  while (gameiter->event != NULL) {
    if (strcmp(gameiter->event->event_text, "NP")) {
      cw_boxscore_batter_stats(boxscore, gameiter);
      cw_boxscore_runner_stats(boxscore, gameiter);
    }
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
    boxscore->pitchers[t] = NULL;
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
    CWBoxPitcher *pitcher = boxscore->pitchers[t];
    while (pitcher != NULL) {
      CWBoxPitcher *prev_pitcher = pitcher->prev;
      cw_boxscore_pitcher_cleanup(pitcher);
      free(pitcher);
      pitcher = prev_pitcher;
    }

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


