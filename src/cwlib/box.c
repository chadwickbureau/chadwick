/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *			    
 * FILE: src/cwlib/box.c
 * Declaration of boxscore data structures and API
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
#include <stdarg.h>
#include <string.h>

#include "chadwick.h"

/*
 * Create an initialize a batting statistic entry
 */
static CWBoxBatting *
cw_box_batting_create(void)
{
  CWBoxBatting *batting = (CWBoxBatting *) malloc(sizeof(CWBoxBatting));
  batting->g = 0;
  batting->pa = 0;
  batting->ab = 0;
  batting->r = 0;
  batting->h = 0;
  batting->b2 = 0;
  batting->b3 = 0;
  batting->hr = 0;
  batting->hrslam = 0;
  batting->bi = 0;
  batting->bi2out = 0;
  /* Choose to default GWRBI field to NULL instead of zero. */
  batting->gw = -1;
  batting->bb = 0;
  batting->ibb = 0;
  batting->so = 0;
  batting->gdp = 0;
  batting->hp = 0;
  batting->sh = 0;
  batting->sf = 0;
  batting->sb = 0;
  batting->cs = 0;
  batting->xi = 0;
  batting->lisp = 0;
  batting->movedup = 0;
  batting->pitches = 0;
  batting->strikes = 0;
  return batting;
}

/*
 * Create an initialize a fielding statistic entry
 */
static CWBoxFielding *
cw_box_fielding_create(void)
{
  CWBoxFielding *fielding = (CWBoxFielding *) malloc(sizeof(CWBoxFielding));
  fielding->g = 0;
  fielding->outs = 0;
  fielding->bip = 0;
  fielding->bf = 0;
  fielding->po = 0;
  fielding->a = 0;
  fielding->e = 0;
  fielding->dp = 0;
  fielding->tp = 0;
  fielding->pb = 0;
  fielding->xi = 0;
  return fielding;
}

/*
 * Create an initialize a pitching statistic entry
 */
static CWBoxPitching *
cw_box_pitching_create(void)
{
  CWBoxPitching *pitching = (CWBoxPitching *) malloc(sizeof(CWBoxPitching));
  pitching->g = 0;
  pitching->gs = 0;
  pitching->cg = 0;
  pitching->sho = 0;
  pitching->gf = 0;
  pitching->outs = 0;
  pitching->ab = 0;
  pitching->r = 0;
  pitching->er = 0;
  pitching->h = 0;
  pitching->b2 = 0;
  pitching->b3 = 0;
  pitching->hr = 0;
  pitching->hrslam = 0;
  pitching->bb = 0;
  pitching->ibb = 0;
  pitching->so = 0;
  pitching->bf = 0; 
  pitching->wp = 0;
  pitching->bk = 0;
  pitching->hb = 0;
  pitching->gdp = 0;
  pitching->sh = 0;
  pitching->sf = 0;
  pitching->xi = 0;
  pitching->pk = 0;
  pitching->w = 0;
  pitching->l = 0;
  pitching->sv = 0;
  pitching->inr = 0;
  pitching->inrs = 0;
  pitching->xb = 0;
  pitching->xbinn = 0;
  pitching->gb = 0;
  pitching->fb = 0;
  pitching->pitches = 0;
  pitching->strikes = 0;
  return pitching;
}

/************************************************************************
 * Private routines for dealing with auxiliary struct init and dealloc
 ************************************************************************/

static CWBoxPlayer *
cw_box_player_create(char *player_id, char *name)
{
  int i;

  CWBoxPlayer *player = (CWBoxPlayer *) malloc(sizeof(CWBoxPlayer));
  player->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(player->player_id, player_id);
  player->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(player->name, name);
  player->batting = cw_box_batting_create();
  player->num_positions = player->ph_inn = player->pr_inn = 0;
  player->start_position = -1;
  for (i = 0; i <= 9; i++) {
    player->fielding[i] = NULL;
  }
  player->prev = NULL;
  player->next = NULL;
  return player;
}

static void
cw_box_player_cleanup(CWBoxPlayer *player)
{
  int i;
  for (i = 0; i <= 9; i++) {
    if (player->fielding[i]) {
      free(player->fielding[i]);
    }
  }
  free(player->batting);
  free(player->name);
  free(player->player_id);
}

static CWBoxPitcher *
cw_box_pitcher_create(char *player_id, char *name)
{
  CWBoxPitcher *pitcher = (CWBoxPitcher *) malloc(sizeof(CWBoxPitcher));
  pitcher->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(pitcher->player_id, player_id);
  pitcher->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(pitcher->name, name);
  pitcher->pitching = cw_box_pitching_create();
  pitcher->prev = NULL;
  pitcher->next = NULL;
  return pitcher;
}

static void
cw_box_pitcher_cleanup(CWBoxPitcher *pitcher)
{
  free(pitcher->pitching);
  free(pitcher->name);
  free(pitcher->player_id);
}

/*
 * Initialize slots with starting players
 */
static void
cw_box_enter_starters(CWBoxscore *boxscore, CWGame *game)
{
  int i, t;
  char *date = cw_game_info_lookup(game, "date");

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      CWAppearance *app = cw_game_starter_find(game, t, i);
      if (!app) continue;

      boxscore->slots[i][t] = cw_box_player_create(app->player_id, app->name);
      sprintf(boxscore->slots[i][t]->date, "%c%c%c%c%c%c%c%c",
	      date[0], date[1], date[2], date[3],
	      date[5], date[6], date[8], date[9]);
      boxscore->slots[i][t]->batting->g = 1;
      boxscore->slots[i][t]->num_positions++;
      boxscore->slots[i][t]->positions[0] = app->pos;
      boxscore->slots[i][t]->start_position = app->pos;
      if (app->pos < 10) {
	boxscore->slots[i][t]->fielding[app->pos] = cw_box_fielding_create();
	/* Under modern rules, players only receive credit for a game
	 *  in the field when they appear there for at least one event.
	 *
	 * boxscore->slots[i][t]->fielding[app->pos]->g = 1;
	 */
      }
      if (app->pos == 1) {
	boxscore->pitchers[t] = cw_box_pitcher_create(app->player_id, app->name);
	boxscore->pitchers[t]->pitching->g = 1;
	boxscore->pitchers[t]->pitching->gs = 1;
      }
    }

    /*
    if (!strcmp(cw_game_info_lookup(game, "usedh"), "true")) {
      CWAppearance *app = cw_game_starter_find(game, t, 0);
      boxscore->pitchers[t] = cw_box_pitcher_create(app->player_id, app->name);
      boxscore->pitchers[t]->pitching->g = 1;
      boxscore->pitchers[t]->pitching->gs = 1;
    }
    */
  }
}

/*
 * Add a substitute into a slot
 */
static void
cw_box_add_substitute(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  CWAppearance *sub = gameiter->event->first_sub;
  CWBoxPitcher *pitcher;

  while (sub != NULL) {
    if (sub->slot < 0 || sub->slot > 9) {
      fprintf(stderr, 
	      "ERROR: In %s, invalid slot %d for player '%s'.\n",
	      gameiter->game->game_id, sub->slot, sub->player_id);
      exit(1);
    }
    if (sub->team < 0 || sub->team > 1) {
      fprintf(stderr,
	      "ERROR: In %s, invalid team %d for player '%s'.\n",
	      gameiter->game->game_id, sub->team, sub->player_id);
      exit(1);
    }
    if (sub->pos < 1 || sub->pos > 12) {
      fprintf(stderr,
	      "ERROR: In %s, invalid position %d for player '%s'.\n",
	      gameiter->game->game_id, sub->pos, sub->player_id);
      exit(1);
    }

    if (boxscore->slots[sub->slot][sub->team] == NULL) {
      /* This should never happen; however, there do exist Retrosheet
       * files with bogus substitution entries, including ones which
       * sub players into the 0 slot even though the DH is not in use.
       * Try to do something reasonable here. 
       */
      CWBoxPlayer *player = cw_box_player_create(sub->player_id, sub->name);
      strncpy(player->date, gameiter->state->date, 8);
      player->date[8] = '\0';
      player->batting->g = 1; 
      boxscore->slots[sub->slot][sub->team] = player;
    }
    else if (sub->slot != 0 && boxscore->slots[0][sub->team] != NULL &&
	     !strcmp(boxscore->slots[0][sub->team]->player_id, 
		     sub->player_id)) {
      /* With the DH in use, a pitcher assumes a field position (and
       * therefore a batting order slot */
      CWBoxPlayer *player = boxscore->slots[0][sub->team];

      /* Remove player from special slot zero */
      if (player->prev) {
	boxscore->slots[0][sub->team] = player->prev;
	player->prev->next = NULL;
	player->prev = NULL;
      }
      else {
	boxscore->slots[0][sub->team] = NULL;
      }

      /* Put player in his new slot */
      boxscore->slots[sub->slot][sub->team]->next = player;
      player->prev = boxscore->slots[sub->slot][sub->team];
      boxscore->slots[sub->slot][sub->team] = player;
    }
	       
    else if (strcmp(sub->player_id, 
		    boxscore->slots[sub->slot][sub->team]->player_id)) {
      CWBoxPlayer *player = cw_box_player_create(sub->player_id, sub->name);
      strncpy(player->date, gameiter->state->date, 8);
      player->date[8] = '\0';
      player->batting->g = 1; 
      boxscore->slots[sub->slot][sub->team]->next = player;
      player->prev = boxscore->slots[sub->slot][sub->team];
      boxscore->slots[sub->slot][sub->team] = player;

      if (sub->pos == 11) {
	player->ph_inn = gameiter->state->inning;
      }
      else if (sub->pos == 12) {
	player->pr_inn = gameiter->state->inning;
      }
    }

    if (sub->pos < 10) {
      CWBoxFielding *fielding = boxscore->slots[sub->slot][sub->team]->fielding[sub->pos];
      if (fielding == NULL) {
	boxscore->slots[sub->slot][sub->team]->fielding[sub->pos] = cw_box_fielding_create();
	/* The mere announcement of a player at a position does not award
	 * him a game played at the position (under modern rules).
	 * Therefore, the game played is set when processing fielding credits
	 * for events.
	 * A future enhancement would allow this to be configurable,
	 * as previously the convention was to award a game played at
	 * a position if the player was listed in the lineup.
	 * (See e.g. Lou Gehrig's appearances at shortstop.)
	 *
	 * boxscore->slots[sub->slot][sub->team]->fielding[sub->pos]->g = 1;
	 */
      }
    }

    boxscore->slots[sub->slot][sub->team]->positions[boxscore->slots[sub->slot][sub->team]->num_positions++] = sub->pos;
    if (sub->pos >= 11 && sub->slot == gameiter->state->dh_slot[sub->team]) {
      /* Entering as a PH or PR for a DH automatically makes the player the DH. */
      boxscore->slots[sub->slot][sub->team]->positions[boxscore->slots[sub->slot][sub->team]->num_positions++] = 10;
    }

    /* Guard against possibility of pitcher being subbed into batting
     * order slot when a team loses the DH -- don't want to create a
     * pitcher record for this! */
    if (sub->pos == 1 &&
	strcmp(sub->player_id, boxscore->pitchers[sub->team]->player_id)) {
      CWBoxPitching *cur_pitcher = boxscore->pitchers[sub->team]->pitching;
      if (gameiter->state->outs == 0 && gameiter->state->inning_batters > 0) {
	cur_pitcher->xb = ((cur_pitcher->bf < gameiter->state->inning_batters) ?
			   cur_pitcher->bf : gameiter->state->inning_batters);
	cur_pitcher->xbinn = gameiter->state->inning;
      }
      else if (cur_pitcher->outs == 0) {
	cur_pitcher->xb = cur_pitcher->bf;
	cur_pitcher->xbinn = gameiter->state->inning;
      }

      pitcher = cw_box_pitcher_create(sub->player_id, sub->name);
      pitcher->pitching->g = 1;
      boxscore->pitchers[sub->team]->next = pitcher;
      pitcher->prev = boxscore->pitchers[sub->team];
      boxscore->pitchers[sub->team] = pitcher;
    }

    if (sub->pos == 1) {
      int base;
      CWBoxPitching *pitcher = boxscore->pitchers[sub->team]->pitching;

      for (base = 1; base <= 3; base++) {
	if (strcmp(gameiter->state->runners[base], "")) {
	  pitcher->inr++;
	  if (cw_gameiter_runner_fate(gameiter, base) >= 4) {
	    pitcher->inrs++;
	  }
	}
      }
    }

    sub = sub->next;
  }
}

/*
 * Find the boxscore entry for player with ID player_id
 */
CWBoxPlayer *
cw_box_find_player(CWBoxscore *boxscore, char *player_id)
{
  int i, t;

  if (player_id == NULL)  {
    return NULL;
  }
  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      CWBoxPlayer *player = boxscore->slots[i][t];
      while (player != NULL) {
	if (!strcmp(player->player_id, player_id)) {
	  return player;
	}
	player = player->prev;
      }
    }
  }

  return NULL;
}

/*
 * Find the boxscore entry for player with ID player_id currently in game.
 * 
 * This differs from cw_box_find_player in that it only searches, and returns,
 * entries from among players currently in the game.  This matters when
 * building the boxscore, in the case of an illegal (or sanctioned but
 * unorthodox) case of re-entry of a player in a different lineup slot.
 */
static CWBoxPlayer *
cw_box_find_current_player(CWBoxscore *boxscore, char *player_id)
{
  int i, t;

  if (player_id == NULL)  {
    return NULL;
  }
  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      CWBoxPlayer *player = boxscore->slots[i][t];
      if (player != NULL && !strcmp(player->player_id, player_id)) {
	return player;
      }
    }
  }

  return NULL;
}


/*
 * Find the pitching entry for player with ID player_id
 */
CWBoxPitcher *
cw_box_find_pitcher(CWBoxscore *boxscore, char *player_id)
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

/*
 * Generic routine to add a new "event" entry to the boxscore
 */
static CWBoxEvent *
cw_box_add_event(CWBoxEvent **list, int inning, int half, int count, ...)
{
  int i = 0;
  va_list arg_list;
  va_start(arg_list, count); 

  if (*list == NULL) {
    *list = (CWBoxEvent *) malloc(sizeof(CWBoxEvent));
    for (i = 0; i < 20; (*list)->players[i++] = NULL);
    i = 0;
    while (count--) {
      (*list)->players[i++] = va_arg(arg_list, char *);
    }
    (*list)->inning = inning;
    (*list)->half_inning = half;
    (*list)->runners = -1;
    (*list)->pickoff = -1;
    (*list)->outs = -1;
    (*list)->mark = 0;
    strcpy((*list)->location, "");
    (*list)->prev = NULL;
    (*list)->next = NULL;
    return *list;
  }
  else {
    CWBoxEvent *event = *list;
    while (event->next != NULL) { 
      event = event->next;
    }

    event->next = (CWBoxEvent *) malloc(sizeof(CWBoxEvent));
    for (i = 0; i < 20; event->next->players[i++] = NULL);
    i = 0;
    while (count--) {
      event->next->players[i++] = va_arg(arg_list, char *);
    }
    event->next->inning = inning;
    event->next->half_inning = half;
    event->next->runners = -1;
    event->next->pickoff = -1;
    event->next->outs = -1;
    event->next->mark = 0;
    strcpy(event->next->location, "");
    event->next->prev = event;
    event->next->next = NULL;
    return event->next;
  }
}

static void
cw_box_cleanup_event_list(CWBoxEvent **list)
{
  CWBoxEvent *event = *list;
  
  while (event != NULL) {
    CWBoxEvent *next_event = event->next;
    free(event);
    event = next_event;
  }
  *list = NULL;
}

/* 
 * Update pitch stats with current event.
 * This is called even for NP events, as pitches may occur prior to a
 * substitution.
 * Only counts pitches following the last period in the pitch string.
 */
static void
cw_box_pitch_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  CWBoxPlayer *player;
  CWBoxPitcher *pitcher;
  char *pitch;

  if (gameiter->event->pitches[0] == '\0') {
    return;
  }
  player = cw_box_find_current_player(boxscore, gameiter->event->batter);
  if (player == NULL) {
    fprintf(stderr, 
	    "ERROR: In %s, no entry for batter at event %d.\n",
	    gameiter->game->game_id, gameiter->state->event_count);
    fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	    gameiter->event->batter, gameiter->event->event_text);
    fprintf(stderr, "      Skipping pitches tabulation for this play.\n");
    return;
  }
  pitcher = boxscore->pitchers[1-gameiter->state->batting_team];
  if (pitcher == NULL) {
    if (gameiter->state->batting_team == 0) {
      fprintf(stderr,
	      "ERROR: In %s, no pitcher in lineup for home team.\n",
	      gameiter->game->game_id);
    }
    else {
      fprintf(stderr,
	      "ERROR: In %s, no pitcher in lineup for visiting team.\n",
	      gameiter->game->game_id);
    }
    return;
  }

  pitch = strrchr(gameiter->event->pitches, '.');
  if (pitch == NULL) {
    pitch = gameiter->event->pitches;
  }
  else {
    /* start from the first pitch after the last period */
    pitch++;
  }
  
  while (*pitch != '\0') {
    if (*pitch == 'B' || *pitch == 'H' || *pitch == 'I' || *pitch == 'P' || 
	*pitch == 'V') {
      player->batting->pitches++;
      pitcher->pitching->pitches++;
    }
    else if (*pitch == 'C' || *pitch == 'F' || *pitch == 'K' || *pitch == 'L' ||
	     *pitch == 'M' || *pitch == 'O' || *pitch == 'Q' || *pitch == 'R' ||
	     *pitch == 'S' || *pitch == 'T' || *pitch == 'X' || *pitch == 'Y') {
      player->batting->pitches++;
      player->batting->strikes++;
      pitcher->pitching->pitches++;
      pitcher->pitching->strikes++;
    }
    pitch++;
  }

}

/*
 * Update batter/pitcher stats with current play
 */
static void
cw_box_batter_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  CWEventData *event_data = gameiter->event_data;
  CWBoxPlayer *player;
  CWBoxPitcher *pitcher, *res_pitcher;

  player = cw_box_find_player(boxscore, 
			      cw_gamestate_charged_batter(gameiter->state,
							  gameiter->event->batter,
							  event_data));
  if (cw_event_is_batter(event_data) && player == NULL) {
    /* If not a batter event, we will be tolerant if the player ID
     * in the batter field is bogus.
     */
    fprintf(stderr, 
	    "ERROR: In %s, no entry for batter '%s' at event %d.\n",
	    gameiter->game->game_id, 
	    cw_gamestate_charged_batter(gameiter->state, 
					gameiter->event->batter,
					gameiter->event_data),
	    gameiter->state->event_count);
    fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	    gameiter->event->batter, gameiter->event->event_text);
    fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
    return;
  }
  
  pitcher = boxscore->pitchers[1-gameiter->state->batting_team];
  if (pitcher == NULL) {
    if (gameiter->state->batting_team == 0) {
      fprintf(stderr,
	      "ERROR: In %s, no pitcher in lineup for home team.\n",
	      gameiter->game->game_id);
    }
    else {
      fprintf(stderr,
	      "ERROR: In %s, no pitcher in lineup for visiting team.\n",
	      gameiter->game->game_id);
    }
    exit(1);
  }

  res_pitcher = pitcher;
  while (res_pitcher && 
	 strcmp(res_pitcher->player_id, 
		cw_gamestate_charged_pitcher(gameiter->state,
					     event_data))) {
    res_pitcher = res_pitcher->prev;
  }
  if (res_pitcher == NULL) {
    fprintf(stderr, 
	    "WARNING: In %s, no entry for charged pitcher '%s' at event %d.\n",
	    gameiter->game->game_id,
	    cw_gamestate_charged_pitcher(gameiter->state, event_data),
	    gameiter->state->event_count);
    fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	    gameiter->event->batter, gameiter->event->event_text);
    fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
    return;
  }

  if (cw_event_is_batter(event_data)) {
    player->batting->pa++;
    res_pitcher->pitching->bf++;
  }
  res_pitcher->pitching->outs += cw_event_outs_on_play(event_data);

  if (cw_event_is_official_ab(event_data)) {
    player->batting->ab++;
    res_pitcher->pitching->ab++;

    if (strcmp(gameiter->state->runners[2], "") ||
	strcmp(gameiter->state->runners[3], "")) {
      boxscore->risp_ab[gameiter->state->batting_team] += 1;
    }

    if (event_data->event_type >= CW_EVENT_SINGLE &&
	event_data->event_type <= CW_EVENT_HOMERUN) {
      player->batting->h++;
      res_pitcher->pitching->h++;
      if (strcmp(gameiter->state->runners[2], "") ||
	  strcmp(gameiter->state->runners[3], "")) {
	boxscore->risp_h[gameiter->state->batting_team]++;
      }

      if (event_data->event_type == CW_EVENT_DOUBLE) {
	cw_box_add_event(&(boxscore->b2_list), 
			 gameiter->state->inning, gameiter->state->batting_team,
			 2, player->player_id, res_pitcher->player_id);
	player->batting->b2++;
	res_pitcher->pitching->b2++;
      }
      else if (event_data->event_type == CW_EVENT_TRIPLE) {
	cw_box_add_event(&(boxscore->b3_list), 
			 gameiter->state->inning, gameiter->state->batting_team,
			 2, player->player_id, res_pitcher->player_id);
	player->batting->b3++;
	res_pitcher->pitching->b3++;
      }
      else if (event_data->event_type == CW_EVENT_HOMERUN) {
	CWBoxEvent *event = 
	  cw_box_add_event(&(boxscore->hr_list), 
			   gameiter->state->inning, gameiter->state->batting_team,
			   2, player->player_id, res_pitcher->player_id);
	event->runners = cw_event_runs_on_play(gameiter->event_data);
	event->outs = gameiter->state->outs;
	strcpy(event->location, gameiter->event_data->hit_location);
	player->batting->hr++;
	res_pitcher->pitching->hr++;
	if (cw_event_rbi_on_play(event_data) == 4) {
	  player->batting->hrslam++;
	  res_pitcher->pitching->hrslam++;
	}
      }
    }
    else if (event_data->event_type == CW_EVENT_STRIKEOUT) {
      player->batting->so++;
      res_pitcher->pitching->so++;
    }
    else if (event_data->gdp_flag) {
      player->batting->gdp++;
      res_pitcher->pitching->gdp++;
    }

  }
  else if (event_data->event_type == CW_EVENT_WALK ||
	   event_data->event_type == CW_EVENT_INTENTIONALWALK) {
    player->batting->bb++;
    res_pitcher->pitching->bb++;
    if (event_data->event_type == CW_EVENT_INTENTIONALWALK) {
      player->batting->ibb++;
      res_pitcher->pitching->ibb++;
      cw_box_add_event(&(boxscore->ibb_list), 
		       gameiter->state->inning, gameiter->state->batting_team,
		       2, player->player_id, res_pitcher->player_id);
    }
  }
  else if (event_data->event_type == CW_EVENT_HITBYPITCH) {
    player->batting->hp++;
    res_pitcher->pitching->hb++;
    cw_box_add_event(&(boxscore->hp_list),
		     gameiter->state->inning, gameiter->state->batting_team,
		     2, player->player_id, res_pitcher->player_id);
  }
  else if (event_data->event_type == CW_EVENT_BALK) {
    res_pitcher->pitching->bk++;
    cw_box_add_event(&(boxscore->bk_list), 
		     gameiter->state->inning, gameiter->state->batting_team,
		     1, res_pitcher->player_id);
  }
  else if (event_data->event_type == CW_EVENT_INTERFERENCE) {
    player->batting->xi++;
    res_pitcher->pitching->xi++;
  }

  if (event_data->event_type == CW_EVENT_GENERICOUT &&
      !event_data->bunt_flag) {
    if (event_data->batted_ball_type == 'G') {
      res_pitcher->pitching->gb++;
    }
    else if (event_data->batted_ball_type == 'F' ||
	     event_data->batted_ball_type == 'P' ||
	     event_data->batted_ball_type == 'L') {
      res_pitcher->pitching->fb++;
    }
  }

  if (event_data->wp_flag) {
    CWBoxPlayer *catcher = 
      cw_box_find_current_player(boxscore, 
				 gameiter->state->fielders[2][1-gameiter->state->batting_team]);
    if (catcher == NULL) {
      fprintf(stderr, 
	      "ERROR: In %s, no entry for fielder at position 2 at event %d.\n",
	      gameiter->game->game_id,
	      gameiter->state->event_count);
      fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	      gameiter->event->batter, gameiter->event->event_text);
      fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
      return;
    }
    cw_box_add_event(&(boxscore->wp_list), 
		     gameiter->state->inning, gameiter->state->batting_team,
		     2, pitcher->player_id, catcher->player_id);
    pitcher->pitching->wp++;
  }

  if (event_data->sh_flag) {
    player->batting->sh++;
    res_pitcher->pitching->sh++;
    cw_box_add_event(&(boxscore->sh_list), 
		     gameiter->state->inning, gameiter->state->batting_team,
		     2, player->player_id, res_pitcher->player_id);
  }
  if (event_data->sf_flag) {
    player->batting->sf++;
    res_pitcher->pitching->sf++;
    cw_box_add_event(&(boxscore->sf_list), 
		     gameiter->state->inning, gameiter->state->batting_team,
		     2, player->player_id, res_pitcher->player_id);
  }

  if (event_data->advance[0] >= 4) {
    player->batting->r++;
    res_pitcher->pitching->r++;
    if (event_data->advance[0] != 5) {
      res_pitcher->pitching->er++;
    }
    if (event_data->advance[0] == 4) {
      boxscore->er[1-gameiter->state->batting_team]++;
    }
  }

  if (cw_event_is_batter(event_data)) {
    player->batting->bi += cw_event_rbi_on_play(event_data);
    if (gameiter->state->outs == 2) {
      player->batting->bi2out += cw_event_rbi_on_play(event_data);
    }
  }

  if (gameiter->state->outs + cw_event_outs_on_play(event_data) == 3) {
    if (strcmp(gameiter->state->runners[3], "") && 
	event_data->advance[3] < 4) {
      player->batting->lisp++;
    }
    if (strcmp(gameiter->state->runners[2], "") && 
	event_data->advance[2] < 4) {
      player->batting->lisp++;
    }
  }
  else if (gameiter->event_data->event_type == CW_EVENT_GENERICOUT) {
    if (strcmp(gameiter->state->runners[1], "") &&
	event_data->advance[1] > 1 && 
	(event_data->advance[1] < 4 || 
	 (event_data->advance[1] >= 4 && event_data->rbi_flag[1] == 0))) {
      player->batting->movedup++;
    }
    if (strcmp(gameiter->state->runners[2], "") &&
	(event_data->advance[2] == 3 ||
	 (event_data->advance[2] >= 4 && event_data->rbi_flag[2] == 0))) {
      player->batting->movedup++;
    }
  }
}

/*
 * Update baserunning stats with current play
 */
static void
cw_box_runner_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  int base;
  CWBoxPlayer *player, *catcher;
  CWBoxPitcher *pitcher;

  for (base = 1; base <= 3; base++) {
    if (!strcmp(gameiter->state->runners[base], "")) {
      continue;
    }

    player = cw_box_find_current_player(boxscore, 
					gameiter->state->runners[base]);
    if (player == NULL) {
      fprintf(stderr, 
	      "ERROR: In %s, no entry for runner '%s' at event %d.\n",
	      gameiter->game->game_id, gameiter->state->runners[base],
	      gameiter->state->event_count);
      fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	      gameiter->event->batter, gameiter->event->event_text);
      fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
      return;
    }

    pitcher = cw_box_find_pitcher(boxscore, 
				  cw_gamestate_responsible_pitcher(gameiter->state,
								   gameiter->event_data,
								   base));
    if (pitcher == NULL) {
      fprintf(stderr, 
	      "ERROR: In %s, no entry for responsible pitcher '%s' for base %d at event %d.\n",
	      gameiter->game->game_id, 
	      gameiter->state->pitchers[base], base,
	      gameiter->state->event_count);
      fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	      gameiter->event->batter, gameiter->event->event_text);
      fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
      return;
    }

    /* Since we only store pointers to the player IDs in the event,
     * we need to point to the player ID in the player's roster entry
     */
    catcher = cw_box_find_current_player(boxscore, 
					 gameiter->state->fielders[2][1-gameiter->state->batting_team]);
    if (catcher == NULL) {
      fprintf(stderr, 
	      "ERROR: In %s, no entry for catcher '%s' for base %d at event %d.\n",
	      gameiter->game->game_id, 
	      gameiter->state->fielders[2][1-gameiter->state->batting_team], 
	      base,
	      gameiter->state->event_count);
      fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	      gameiter->event->batter, gameiter->event->event_text);
      fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
      return;
    }

    if (gameiter->event_data->advance[base] >= 4) {
      player->batting->r++;
      pitcher->pitching->r++;
      if (gameiter->event_data->advance[base] != 5) {
	pitcher->pitching->er++;
      }
      if (gameiter->event_data->advance[base] == 4) {
	boxscore->er[1-gameiter->state->batting_team]++;
      }
    }

    pitcher = cw_box_find_pitcher(boxscore, 
				  cw_gamestate_charged_pitcher(gameiter->state,
							       gameiter->event_data));

    if (gameiter->event_data->sb_flag[base]) {
      CWBoxEvent *event =
	cw_box_add_event(&(boxscore->sb_list),
			 gameiter->state->inning, gameiter->state->batting_team, 3,
			 player->player_id, pitcher->player_id,
			 catcher->player_id);
      event->runners = base;
      player->batting->sb++;
      event->pickoff = (gameiter->event_data->po_flag[base]) ? 1 : 0;
    }

    if (gameiter->event_data->cs_flag[base]) {
      CWBoxEvent *event = 
	cw_box_add_event(&(boxscore->cs_list), 
			 gameiter->state->inning, gameiter->state->batting_team, 3,
			 player->player_id, pitcher->player_id,
			 catcher->player_id);
      event->runners = base;
      player->batting->cs++;
      if (gameiter->event_data->po_flag[base]) {
	event->pickoff = (gameiter->event_data->play[base][0] - '0');
      }
      else {
	event->pickoff = 0;
      }

      if (event->pickoff == 1) {
	pitcher->pitching->pk++;
      }
    }
    else if (gameiter->event_data->po_flag[base]) {
      CWBoxEvent *event;
      if (gameiter->event_data->play[base][0] == '2') {
	event = cw_box_add_event(&(boxscore->po_list), 
				 gameiter->state->inning, gameiter->state->batting_team, 2,
				 player->player_id, catcher->player_id);
      }
      else {
	event = cw_box_add_event(&(boxscore->po_list), 
				 gameiter->state->inning, gameiter->state->batting_team, 2,
				 player->player_id, pitcher->player_id);
      }
      event->pickoff = (gameiter->event_data->play[base][0] - '0');
      if (event->pickoff == 1) {
	pitcher->pitching->pk++;
      }
      event->runners = base;
    }

  }
}

/*
 * Update fielding stats with current play
 */
static void
cw_box_fielder_stats(CWBoxscore *boxscore, CWGameIterator *gameiter)
{
  int pos, i;
  CWBoxPlayer *player = NULL; 
  CWBoxFielding *fielding = NULL;

  for (pos = 1; pos <= 9; pos++) {
    int accepted = 0;
    player = cw_box_find_current_player(boxscore, 
					gameiter->state->fielders[pos][1-gameiter->state->batting_team]);
    if (player != NULL) {
      fielding = player->fielding[pos];
    }
    else {
      fielding = NULL;
    }
    if (fielding == NULL) {
      fprintf(stderr, 
	      "ERROR: In %s, no entry for fielder at position %d at event %d.\n",
	      gameiter->game->game_id, pos,
	      gameiter->state->event_count);
      fprintf(stderr, "      (Batter ID '%s', event text '%s')\n",
	      gameiter->event->batter, gameiter->event->event_text);
      fprintf(stderr, "      Skipping statistics tabulation for this play.\n");
      return;
    }

    /* Fielders are credited with a game played only if they are
     * on the field at a position for at least one event.
     */
    fielding->g = 1;
    fielding->outs += cw_event_outs_on_play(gameiter->event_data);

    if (gameiter->event_data->event_type == CW_EVENT_SINGLE ||
	gameiter->event_data->event_type == CW_EVENT_DOUBLE ||
	gameiter->event_data->event_type == CW_EVENT_TRIPLE ||
	(gameiter->event_data->event_type == CW_EVENT_HOMERUN &&
	 gameiter->event_data->fielded_by > 0) ||
	gameiter->event_data->event_type == CW_EVENT_ERROR ||
	gameiter->event_data->event_type == CW_EVENT_GENERICOUT ||
	gameiter->event_data->event_type == CW_EVENT_FIELDERSCHOICE) {
      fielding->bip++;
    }


    if (cw_event_outs_on_play(gameiter->event_data) > 0 &&
	gameiter->event_data->fielded_by == pos) {
      fielding->bf++;
    }

    if (strcmp(gameiter->event_data->play[0], "99") &&
	strcmp(gameiter->event_data->play[1], "99") &&
	strcmp(gameiter->event_data->play[2], "99") &&
	strcmp(gameiter->event_data->play[3], "99")) {
      /* If there are any unknown fielding credits, do not record
	 putouts or assists for any fielder.  May be overly conservative
	 if fielding credit for one part of a DP is known, but I don't know
	 if there is any instance where that occurs in Retrosheet.
      */
      for (i = 0; i <= 2; i++) {
	if (gameiter->event_data->putouts[i] == pos) {
	  fielding->po++;
	  accepted = 1;
	}
      }

      for (i = 0; i < 10; i++) {
	if (gameiter->event_data->assists[i] == pos) {
	  fielding->a++;
	  accepted = 1;
	}
      }
    }

    for (i = 0; i < 10; i++) {
      if (gameiter->event_data->errors[i] == pos) {
	fielding->e++;
	cw_box_add_event(&(boxscore->err_list), 
			 gameiter->state->inning, gameiter->state->batting_team,
			 1, player->player_id);
      }
    }

    if (accepted && gameiter->event_data->dp_flag) {
      fielding->dp++;
    }
    if (accepted && gameiter->event_data->tp_flag) {
      fielding->tp++;
    }

    if (pos == 2 && gameiter->event_data->pb_flag) {
      CWBoxPitcher *pitcher = 
	cw_box_find_pitcher(boxscore,
			    cw_gamestate_charged_pitcher(gameiter->state,
							 gameiter->event_data));
      fielding->pb++;
      cw_box_add_event(&(boxscore->pb_list), 
		       gameiter->state->inning, gameiter->state->batting_team,
		       2, pitcher->player_id, player->player_id);
    }

    if (pos == 2 && 
	gameiter->event_data->event_type == CW_EVENT_INTERFERENCE &&
	gameiter->event_data->errors[0] == 2) {
      fielding->xi++;
    }
  }  

  if (gameiter->event_data->dp_flag) {
    CWBoxEvent *event = 
      cw_box_add_event(&(boxscore->dp_list), 
		       gameiter->state->inning, gameiter->state->batting_team, 0);
    for (i = 0; i < gameiter->event_data->num_touches; i++) {
      int pos = gameiter->event_data->touches[i];
      CWBoxPlayer *player = 
	cw_box_find_current_player(boxscore, 
				   gameiter->state->fielders[pos][1-gameiter->state->batting_team]); 
      event->players[i] = player->player_id;
    }
  }
  else if (gameiter->event_data->tp_flag) {
    CWBoxEvent *event = 
      cw_box_add_event(&(boxscore->tp_list), 
		       gameiter->state->inning, gameiter->state->batting_team, 0);
    for (i = 0; i < gameiter->event_data->num_touches; i++) {
      int pos = gameiter->event_data->touches[i];
      CWBoxPlayer *player = 
	cw_box_find_current_player(boxscore, 
				   gameiter->state->fielders[pos][1-gameiter->state->batting_team]); 
      event->players[i] = player->player_id;
    }
  }
}

/*
 * Iterate through the game, building the boxscore
 */
static void
cw_box_iterate_game(CWBoxscore *boxscore, CWGame *game)
{
  int t, lead_change = 0;
  CWGameIterator *gameiter = cw_gameiter_create(game);

  while (gameiter->event != NULL) {
    if (boxscore->linescore[gameiter->state->inning][gameiter->state->batting_team] < 0) {
      boxscore->linescore[gameiter->state->inning][gameiter->state->batting_team] = 0;
    }

    cw_box_pitch_stats(boxscore, gameiter);
    if (strcmp(gameiter->event->event_text, "NP")) {
      cw_box_batter_stats(boxscore, gameiter);
      cw_box_runner_stats(boxscore, gameiter);
      cw_box_fielder_stats(boxscore, gameiter);
      if (gameiter->event_data->dp_flag) {
	boxscore->dp[1-gameiter->state->batting_team]++;
      }
      if (gameiter->event_data->tp_flag) {
	boxscore->tp[1-gameiter->state->batting_team]++;
      }
      boxscore->linescore[gameiter->state->inning][gameiter->state->batting_team] += cw_event_runs_on_play(gameiter->event_data);
      if (gameiter->state->score[gameiter->state->batting_team] +
	  cw_event_runs_on_play(gameiter->event_data) > 
	  gameiter->state->score[1-gameiter->state->batting_team] &&
	  gameiter->state->score[gameiter->state->batting_team] -
	  gameiter->state->score[1-gameiter->state->batting_team] <= 0) {
	lead_change = 1;
      }
      else {
	lead_change = 0;
      }
    }
    cw_box_add_substitute(boxscore, gameiter);
    cw_gameiter_next(gameiter);
  }

  boxscore->outs_at_end = gameiter->state->outs;
  boxscore->walk_off = lead_change;

  for (t = 0; t <= 1; t++) {
    boxscore->lob[t] = (gameiter->state->num_batters[t] +
			gameiter->state->num_itb_runners[t] - 
			gameiter->state->times_out[t] - 
			gameiter->state->score[t]);
    boxscore->score[t] = gameiter->state->score[t];
    boxscore->hits[t] = gameiter->state->hits[t];
    boxscore->errors[t] = gameiter->state->errors[t];
  }

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}


/*
 * Process a boxscore event file.  In these files, all the statistical
 * information is stored in extended stat records.
 */
void
cw_box_process_boxscore_file(CWBoxscore *boxscore, CWGame *game)
{
  int i, slot, team, seq, pos;
  CWData *stat;
  CWBoxEvent *event;
  CWBoxPlayer *player;
  CWBoxPitcher *pitcher;
  char *date = cw_game_info_lookup(game, "date");

  /* Assume games ended with the conclusion of an inning... */
  boxscore->outs_at_end = 3;


  for (stat = game->first_stat; stat; stat = stat->next) {
    if (!strcmp(stat->data[0], "bline")) {
      slot = cw_data_get_item_int(stat, 3);
      team = cw_data_get_item_int(stat, 2);

      if (cw_data_get_item_int(stat, 4) == 1) {
	/* Record for starter */
	player = cw_box_get_starter(boxscore, team, slot);
      }
      else {
	player = cw_box_player_create(stat->data[1], "");
	sprintf(player->date, "%c%c%c%c%c%c%c%c",
		date[0], date[1], date[2], date[3],
		date[5], date[6], date[8], date[9]);
	player->batting->g = 1; 
	boxscore->slots[slot][team]->next = player;
	player->prev = boxscore->slots[slot][team];
	boxscore->slots[slot][team] = player;
      }

      player->batting->pa = -1;
      player->batting->ab = cw_data_get_item_int(stat, 5);
      player->batting->r = cw_data_get_item_int(stat, 6);
      boxscore->score[team] += player->batting->r;
      player->batting->h = cw_data_get_item_int(stat, 7);
      boxscore->hits[team] += player->batting->h;
      player->batting->b2 = cw_data_get_item_int(stat, 8);
      for (i = 1; i <= player->batting->b2; i++) {
	cw_box_add_event(&(boxscore->b2_list), -1, -1, 2,
			 player->player_id, "");
      }
      player->batting->b3 = cw_data_get_item_int(stat, 9);
      for (i = 1; i <= player->batting->b3; i++) {
	cw_box_add_event(&(boxscore->b3_list), -1, -1, 2,
			 player->player_id, "");
      }
      player->batting->hr = cw_data_get_item_int(stat, 10);
      for (i = 1; i <= player->batting->hr; i++) {
	cw_box_add_event(&(boxscore->hr_list), -1, -1, 2,
			 player->player_id, "");
      }
      player->batting->hrslam = -1;
      player->batting->bi = cw_data_get_item_int(stat, 11);
      player->batting->bi2out = -1;
      player->batting->sh = cw_data_get_item_int(stat, 12);
      for (i = 1; i <= player->batting->sh; i++) {
	cw_box_add_event(&(boxscore->sh_list), -1, -1, 2,
			 player->player_id, "");
      }
      player->batting->sf = cw_data_get_item_int(stat, 13);
      for (i = 1; i <= player->batting->sf; i++) {
	cw_box_add_event(&(boxscore->sf_list), -1, -1, 2,
			 player->player_id, "");
      }
      player->batting->hp = cw_data_get_item_int(stat, 14);
      player->batting->bb = cw_data_get_item_int(stat, 15);
      player->batting->ibb = cw_data_get_item_int(stat, 16);
      player->batting->so = cw_data_get_item_int(stat, 17);
      player->batting->sb = cw_data_get_item_int(stat, 18);
      for (i = 1; i <= player->batting->sb; i++) {
	event = cw_box_add_event(&(boxscore->sb_list), -1, -1, 2,
				 player->player_id, "", "");
	event->runners = -1;
	event->pickoff = -1;
      }
      player->batting->cs = cw_data_get_item_int(stat, 19);
      for (i = 1; i <= player->batting->cs; i++) {
	event = cw_box_add_event(&(boxscore->cs_list), -1, -1, 2,
				 player->player_id, "", "");
	event->runners = -1;
	event->pickoff = -1;
      }
      player->batting->gdp = cw_data_get_item_int(stat, 20);
      player->batting->xi = cw_data_get_item_int(stat, 21);
      player->batting->lisp = -1;
      player->batting->movedup = -1;
    }
    else if (!strcmp(stat->data[0], "pline")) {
      team = cw_data_get_item_int(stat, 2);
      seq = cw_data_get_item_int(stat, 3);

      if (seq == 1) {
	/* Record for starter */
	pitcher = cw_box_get_starting_pitcher(boxscore, team);
	pitcher->pitching->gs = 1;
      }
      else {
	pitcher = cw_box_pitcher_create(stat->data[1], "");
	boxscore->pitchers[team]->next = pitcher;
	pitcher->prev = boxscore->pitchers[team];
	boxscore->pitchers[team] = pitcher;
      }
      pitcher->pitching->g = 1; 
      pitcher->pitching->outs = cw_data_get_item_int(stat, 4);
      pitcher->pitching->xb = cw_data_get_item_int(stat, 5);
      pitcher->pitching->bf = cw_data_get_item_int(stat, 6);
      pitcher->pitching->h = cw_data_get_item_int(stat, 7);
      pitcher->pitching->b2 = cw_data_get_item_int(stat, 8);
      pitcher->pitching->b3 = cw_data_get_item_int(stat, 9);
      pitcher->pitching->hr = cw_data_get_item_int(stat, 10);
      pitcher->pitching->r = cw_data_get_item_int(stat, 11);
      pitcher->pitching->er = cw_data_get_item_int(stat, 12);
      pitcher->pitching->bb = cw_data_get_item_int(stat, 13);
      pitcher->pitching->ibb = cw_data_get_item_int(stat, 14);
      pitcher->pitching->so = cw_data_get_item_int(stat, 15);
      pitcher->pitching->hb = cw_data_get_item_int(stat, 16);
      pitcher->pitching->wp = cw_data_get_item_int(stat, 17);
      pitcher->pitching->bk = cw_data_get_item_int(stat, 18);
      pitcher->pitching->sh = cw_data_get_item_int(stat, 19);
      pitcher->pitching->sf = cw_data_get_item_int(stat, 20);
      pitcher->pitching->ab = -1;
      pitcher->pitching->gdp = -1;
      pitcher->pitching->xi = -1;
      pitcher->pitching->pk = -1;
      pitcher->pitching->inr = -1;
      pitcher->pitching->inrs = -1;
      pitcher->pitching->gb = -1;
      pitcher->pitching->fb = -1;
    }
    else if (!strcmp(stat->data[0], "dline")) {
      team = cw_data_get_item_int(stat, 2);
      seq = cw_data_get_item_int(stat, 3);
      pos = cw_data_get_item_int(stat, 4);
      player = cw_box_find_player(boxscore, stat->data[1]);
      if (player == NULL) {
	fprintf(stderr,
		"ERROR: In %s, cannot find entry for player '%s' listed in dline.\n",
		game->game_id, stat->data[1]);
	exit(1);
      }
      if (player->num_positions < seq) {
	player->num_positions = seq;
      }
      player->positions[seq-1] = pos;
      if (player->fielding[pos] == NULL) {
	player->fielding[pos] = cw_box_fielding_create();
      }
      player->fielding[pos]->g = 1;
      player->fielding[pos]->outs = cw_data_get_item_int(stat, 5);
      player->fielding[pos]->po = cw_data_get_item_int(stat, 6);
      player->fielding[pos]->a = cw_data_get_item_int(stat, 7);
      player->fielding[pos]->e = cw_data_get_item_int(stat, 8);
      boxscore->errors[team] += player->fielding[pos]->e;
      player->fielding[pos]->dp = cw_data_get_item_int(stat, 9);
      player->fielding[pos]->tp = cw_data_get_item_int(stat, 10);
      player->fielding[pos]->pb = cw_data_get_item_int(stat, 11);
      player->fielding[pos]->bip = - 1;
      player->fielding[pos]->bf = -1;
      player->fielding[pos]->xi = -1;
    }
    else if (!strcmp(stat->data[0], "phline")) {
      team = cw_data_get_item_int(stat, 3);
      player = cw_box_find_player(boxscore, stat->data[1]);
      if (player == NULL) {
	fprintf(stderr,
		"ERROR: In %s, cannot find entry for player '%s' listed in phline.\n",
		game->game_id, stat->data[1]);
	exit(1);
      }
      player->ph_inn = cw_data_get_item_int(stat, 2);
    }
    else if (!strcmp(stat->data[0], "prline")) {
      team = cw_data_get_item_int(stat, 3);
      player = cw_box_find_player(boxscore, stat->data[1]);
      if (player == NULL) {
	fprintf(stderr,
		"ERROR: In %s, cannot find entry for player '%s' listed in prline.\n",
		game->game_id, stat->data[1]);
	exit(1);
      }
      player->pr_inn = cw_data_get_item_int(stat, 2);
    }
    else if (!strcmp(stat->data[0], "tline")) {
      int team = cw_data_get_item_int(stat, 1);
      boxscore->lob[team] = cw_data_get_item_int(stat, 2);
      boxscore->er[team] = cw_data_get_item_int(stat, 3);
      boxscore->dp[team] = cw_data_get_item_int(stat, 4);
      boxscore->tp[team] = cw_data_get_item_int(stat, 5);
    }
  }

  for (stat = game->first_line; stat; stat = stat->next) {
    int team = cw_data_get_item_int(stat, 0);
    for (i = 1; i < stat->num_data; i++) {
      boxscore->linescore[i][team] = cw_data_get_item_int(stat, i);
    }
  }

  for (stat = game->first_evdata; stat; stat = stat->next) {
    if (!strcmp(stat->data[0], "dpline")) {
      event = cw_box_add_event(&(boxscore->dp_list), -1,
			       1-cw_data_get_item_int(stat, 1), 0);
      for (i = 2; i < stat->num_data; i++) {
	event->players[i-2] = stat->data[i];
      }
    }
    else if (!strcmp(stat->data[0], "tpline")) {
      event = cw_box_add_event(&(boxscore->tp_list), -1,
			       1-cw_data_get_item_int(stat, 1), 0);
      for (i = 2; i < stat->num_data; i++) {
	event->players[i-2] = stat->data[i];
      }
    }
  }
}

/*
 * Compile a boxscore for game 'game'.
 */
CWBoxscore *
cw_box_create(CWGame *game)
{
  int i, t;
  CWBoxscore *boxscore = (CWBoxscore *) malloc(sizeof(CWBoxscore));
  CWBoxPitcher *pitcher = NULL;
  CWBoxPlayer *batter = NULL;

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      boxscore->slots[i][t] = NULL;
    }
    boxscore->pitchers[t] = NULL;

    for (i = 0; i < 50; i++) {
      boxscore->linescore[i][t] = -1;
    }

    boxscore->score[t] = 0;
    boxscore->hits[t] = 0;
    boxscore->errors[t] = 0;
    boxscore->dp[t] = 0;
    boxscore->tp[t] = 0;
    boxscore->lob[t] = 0;
    boxscore->er[t] = 0;
    boxscore->risp_ab[t] = 0;
    boxscore->risp_h[t] = 0;
  }

  boxscore->outs_at_end = 0;
  boxscore->walk_off = 0;

  boxscore->b2_list = NULL;
  boxscore->b3_list = NULL;
  boxscore->hr_list = NULL;
  boxscore->sb_list = NULL;
  boxscore->cs_list = NULL;
  boxscore->po_list = NULL;
  boxscore->sh_list = NULL;
  boxscore->sf_list = NULL;
  boxscore->hp_list = NULL;
  boxscore->ibb_list = NULL;
  boxscore->wp_list = NULL;
  boxscore->bk_list = NULL;
  boxscore->pb_list = NULL;
  boxscore->err_list = NULL;
  boxscore->dp_list = NULL;
  boxscore->tp_list = NULL;
  
  cw_box_enter_starters(boxscore, game);
  if (game->first_event != NULL) {
    cw_box_iterate_game(boxscore, game);
  }
  else {
    /* There is no play-by-play; this is a new "boxscore event file" */
    cw_box_process_boxscore_file(boxscore, game);
  }
  
  for (t = 0; t <= 1; t++) { 
    if (boxscore->pitchers[t]->prev == NULL) {
      boxscore->pitchers[t]->pitching->cg = 1;
      if (boxscore->pitchers[t]->pitching->r == 0) {
	boxscore->pitchers[t]->pitching->sho = 1;
      }
    }
    else {
      boxscore->pitchers[t]->pitching->gf = 1;
    }
  }
  if (cw_game_info_lookup(game, "wp") != NULL) {
    pitcher = cw_box_find_pitcher(boxscore, cw_game_info_lookup(game, "wp"));
    if (pitcher != NULL)  pitcher->pitching->w = 1;
  }
  if (cw_game_info_lookup(game, "lp") != NULL) {
    pitcher = cw_box_find_pitcher(boxscore, cw_game_info_lookup(game, "lp"));
    if (pitcher != NULL)  pitcher->pitching->l = 1;
  }
  if (cw_game_info_lookup(game, "save") != NULL) {
    pitcher = cw_box_find_pitcher(boxscore, cw_game_info_lookup(game, "save"));
    if (pitcher != NULL)  pitcher->pitching->sv = 1;
  }
  if (cw_game_info_lookup(game, "gwrbi") != NULL) {
    batter = cw_box_find_player(boxscore, cw_game_info_lookup(game, "gwrbi"));
    if (batter != NULL)  batter->batting->gw = 1;
  }
  return boxscore;
}

/*
 * Memory cleanup of 'boxscore'
 */
void
cw_box_cleanup(CWBoxscore *boxscore)
{
  int i, t;
  
  cw_box_cleanup_event_list(&(boxscore->tp_list));
  cw_box_cleanup_event_list(&(boxscore->dp_list));
  cw_box_cleanup_event_list(&(boxscore->pb_list));
  cw_box_cleanup_event_list(&(boxscore->err_list));
  cw_box_cleanup_event_list(&(boxscore->bk_list));
  cw_box_cleanup_event_list(&(boxscore->wp_list));
  cw_box_cleanup_event_list(&(boxscore->po_list));
  cw_box_cleanup_event_list(&(boxscore->cs_list));
  cw_box_cleanup_event_list(&(boxscore->sb_list));
  cw_box_cleanup_event_list(&(boxscore->ibb_list));
  cw_box_cleanup_event_list(&(boxscore->hp_list));
  cw_box_cleanup_event_list(&(boxscore->sf_list));
  cw_box_cleanup_event_list(&(boxscore->sh_list));
  cw_box_cleanup_event_list(&(boxscore->hr_list));
  cw_box_cleanup_event_list(&(boxscore->b3_list));
  cw_box_cleanup_event_list(&(boxscore->b2_list));
  
  for (t = 0; t <= 1; t++) {
    CWBoxPitcher *pitcher = boxscore->pitchers[t];
    while (pitcher != NULL) {
      CWBoxPitcher *prev_pitcher = pitcher->prev;
      cw_box_pitcher_cleanup(pitcher);
      free(pitcher);
      pitcher = prev_pitcher;
    }

    for (i = 0; i <= 9; i++) {
      CWBoxPlayer *player = boxscore->slots[i][t];

      while (player != NULL) {
	CWBoxPlayer *prev_player = player->prev;
	cw_box_player_cleanup(player);
	free(player);
	player = prev_player;
      }

      boxscore->slots[i][t] = NULL;
    }
  }
}

CWBoxPlayer *cw_box_get_starter(CWBoxscore *boxscore, int team, int slot)
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

CWBoxPitcher *cw_box_get_starting_pitcher(CWBoxscore *boxscore, int team)
{
  CWBoxPitcher *pitcher = boxscore->pitchers[team];

  while (pitcher->prev != NULL) {
    pitcher = pitcher->prev;
  }

  return pitcher;
}


