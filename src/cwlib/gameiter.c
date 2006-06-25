/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of routines to store game state
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

#include "parse.h"
#include "gameiter.h"

/*
 * Private auxiliary function to set up lineups with starters 
 */
static void
cw_gameiter_lineup_setup(CWGameIterator *gameiter)
{
  CWAppearance *starter = gameiter->game->first_starter;

  while (starter != NULL) {
    gameiter->lineups[starter->slot][starter->team].player_id =
      (char *) malloc(sizeof(char) * (strlen(starter->player_id) + 1));
    strcpy(gameiter->lineups[starter->slot][starter->team].player_id,
	   starter->player_id);

    gameiter->lineups[starter->slot][starter->team].name =
      (char *) malloc(sizeof(char) * (strlen(starter->name) + 1));
    strcpy(gameiter->lineups[starter->slot][starter->team].name,
	   starter->name);

    gameiter->lineups[starter->slot][starter->team].position = 
      starter->pos;

    if (starter->pos <= 9) {
      gameiter->fielders[starter->pos][starter->team] =
	(char *) malloc(sizeof(char) * (strlen(starter->player_id) + 1));
      strcpy(gameiter->fielders[starter->pos][starter->team],
	     starter->player_id);
    }
    else if (starter->pos == 10) {
      gameiter->dh_slot[starter->team] = starter->slot;
    }

    starter = starter->next;
  }
}

/*
 * Private auxiliary function to cleanup memory allocated in lineups
 */
static void
cw_gameiter_lineup_cleanup(CWGameIterator *gameiter)
{
  int i, t;

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      if (gameiter->lineups[i][t].player_id != NULL) {
	free(gameiter->lineups[i][t].player_id);
	gameiter->lineups[i][t].player_id = NULL;
      }
      if (gameiter->lineups[i][t].name != NULL) {
	free(gameiter->lineups[i][t].name);
	gameiter->lineups[i][t].name = NULL;
      }
      if (gameiter->fielders[i][t] != NULL) {
	free(gameiter->fielders[i][t]);
	gameiter->fielders[i][t] = NULL;
      }
    }
  }
}

/*
 * Private auxiliary function to cleanup memory allocated in remembering
 * removed players (PH, PR, mid-count substitutions)
 */
static void
cw_gameiter_removeds_cleanup(CWGameIterator *gameiter)
{
  int i;

  if (gameiter->removed_for_ph != NULL) {
    free(gameiter->removed_for_ph);
    gameiter->removed_for_ph = NULL;
  }
  
  if (gameiter->walk_pitcher != NULL) {
    free(gameiter->walk_pitcher);
    gameiter->walk_pitcher = NULL;
  }

  if (gameiter->strikeout_batter != NULL) {
    free(gameiter->strikeout_batter);
    gameiter->strikeout_batter = NULL;
  }

  for (i = 0; i <= 3; i++) {
    if (gameiter->removed_for_pr[i] != NULL) {
      free(gameiter->removed_for_pr[i]);
      gameiter->removed_for_pr[i] = NULL;
    }
  }
}

void
cw_gameiter_reset(CWGameIterator *gameiter)
{
  int i;
  gameiter->event = gameiter->game->first_event;

  gameiter->event_count = 0;
  gameiter->inning = 1;
  gameiter->half_inning = 0;
  gameiter->outs = 0;
  gameiter->score[0] = gameiter->score[1] = 0;
  gameiter->hits[0] = gameiter->hits[1] = 0;
  gameiter->errors[0] = gameiter->errors[1] = 0;
  gameiter->times_out[0] = gameiter->times_out[1] = 0;
  gameiter->num_batters[0] = gameiter->num_batters[1] = 0;
  gameiter->dh_slot[0] = gameiter->dh_slot[1] = 0;
  gameiter->is_leadoff = 1;
  gameiter->ph_flag = 0;
  for (i = 0; i <= 3; i++) {
    strcpy(gameiter->runners[i], "");
    strcpy(gameiter->pitchers[i], "");
  }

  cw_gameiter_removeds_cleanup(gameiter);
  cw_gameiter_lineup_cleanup(gameiter);
  cw_gameiter_lineup_setup(gameiter);

  if (gameiter->event && strcmp(gameiter->event->event_text, "NP")) {
    cw_parse_event(gameiter->event->event_text, gameiter->event_data);
  }
}

CWGameIterator *
cw_gameiter_create(CWGame *game)
{
  int i, t;
  CWGameIterator *gameiter = (CWGameIterator *) malloc(sizeof(CWGameIterator));
  gameiter->game = game;

  /* Make sure to set all these to null, so reset does not attempt
   * to free() non-malloc()ed memory
   */
  gameiter->removed_for_ph = NULL;
  gameiter->walk_pitcher = NULL;
  gameiter->strikeout_batter = NULL;

  for (i = 0; i <= 3; i++) {
    gameiter->removed_for_pr[i] = NULL;
  }

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      gameiter->lineups[i][t].player_id = NULL;
      gameiter->lineups[i][t].name = NULL;
      gameiter->fielders[i][t] = NULL;
    }
  }

  gameiter->event_data = (CWParsedEvent *) malloc(sizeof(CWParsedEvent));

  cw_gameiter_reset(gameiter);
  return gameiter;
}

void
cw_gameiter_cleanup(CWGameIterator *gameiter)
{
  cw_gameiter_removeds_cleanup(gameiter);
  cw_gameiter_lineup_cleanup(gameiter);
  free(gameiter->event_data);
  gameiter->event_data = NULL;
}

static void
cw_gameiter_change_sides(CWGameIterator *gameiter)
{
  int i;

  /* Ideally, would copy these from the event; however, there are
   * some Retrosheet files where the event inning is wrong */
  gameiter->inning += gameiter->half_inning;
  gameiter->half_inning = (gameiter->half_inning + 1) % 2;
  gameiter->outs = 0;
  gameiter->is_leadoff = 1;
  gameiter->ph_flag = 0;

  for (i = 0; i <= 3; i++) {
    strcpy(gameiter->runners[i], "");
    strcpy(gameiter->pitchers[i], "");
  }

  /* Pinch-hitters or -runners for DH automatically become DH,
   * even though no sub record occurs */
  for (i = 0; i <= 1; i++) {
    if (gameiter->dh_slot[i] > 0 &&
	gameiter->lineups[gameiter->dh_slot[i]][i].position > 10) {
      gameiter->lineups[gameiter->dh_slot[i]][i].position = 10;
    }
  }
}

static void
cw_gameiter_process_subs(CWGameIterator *gameiter, CWEvent *event)
{
  CWAppearance *sub = event->first_sub;

  while (sub != NULL) {
    char *removedPlayer = gameiter->lineups[sub->slot][sub->team].player_id;
    int removedPosition = gameiter->lineups[sub->slot][sub->team].position;

    gameiter->lineups[sub->slot][sub->team].player_id = 
      (char *) malloc(sizeof(char) * (strlen(sub->player_id) + 1));
    strcpy(gameiter->lineups[sub->slot][sub->team].player_id,
	   sub->player_id);

    free(gameiter->lineups[sub->slot][sub->team].name);
    gameiter->lineups[sub->slot][sub->team].name =
      (char *) malloc(sizeof(char) * (strlen(sub->name) + 1));
    strcpy(gameiter->lineups[sub->slot][sub->team].name, sub->name);

    gameiter->lineups[sub->slot][sub->team].position = sub->pos;
      
    if (strlen(event->count) == 2 &&
	event->count[0] != '?' && event->count[1] != '?' &&
	!strcmp(event->event_text, "NP")) {
      if (sub->pos == 1 && 
	  (!strcmp(event->count, "20") ||
	   !strcmp(event->count, "21") || event->count[0] == '3')) {
	gameiter->walk_pitcher =
	  (char *) malloc((strlen(gameiter->fielders[1][sub->team]) + 1)
			  * sizeof(char));
	strcpy(gameiter->walk_pitcher,
	       gameiter->fielders[1][sub->team]);
      }
      else if (sub->pos == 11 && gameiter->strikeout_batter == NULL &&
	       event->count[1] == '2') {
	gameiter->strikeout_batter = 
	  (char *) malloc((strlen(event->batter) + 1) * sizeof(char));
	strcpy(gameiter->strikeout_batter, event->batter);
      }
    }

    if (sub->pos <= 9) {
      free(gameiter->fielders[sub->pos][sub->team]);
      gameiter->fielders[sub->pos][sub->team] =
	(char *) malloc(sizeof(char) * (strlen(sub->player_id) + 1));
      strcpy(gameiter->fielders[sub->pos][sub->team],
	     sub->player_id);
      if (sub->pos == 1 && sub->slot > 0 &&
	  gameiter->lineups[0][sub->team].player_id != NULL) {
	/* Substituting a pitcher into the batting order, eliminating
	 * the DH.  Clear out slot zero.
	 */
	free(gameiter->lineups[0][sub->team].player_id);
	gameiter->lineups[0][sub->team].player_id = NULL;
	free(gameiter->lineups[0][sub->team].name);
	gameiter->lineups[0][sub->team].name = NULL;
      }
    }
    else if (sub->pos == 11) {
      gameiter->removed_for_ph = removedPlayer;
      gameiter->ph_flag = 1;
      gameiter->removed_position = removedPosition;
    }
    else if (sub->pos == 12) {
      if (!strcmp(gameiter->runners[1], removedPlayer)) {
	gameiter->removed_for_pr[1] = removedPlayer;
	strncpy(gameiter->runners[1], sub->player_id, 49);
      }
      else if (!strcmp(gameiter->runners[2], removedPlayer)) {
	gameiter->removed_for_pr[2] = removedPlayer;
	strncpy(gameiter->runners[2], sub->player_id, 49);
      }
      else if (!strcmp(gameiter->runners[3], removedPlayer)) {
	gameiter->removed_for_pr[3] = removedPlayer;
	strncpy(gameiter->runners[3], sub->player_id, 49);
      }
    }

    sub = sub->next;
  }
}

/*
 * The only tricky part of advancement is correctly implementing pitcher
 * responsibility on force outs and fielder's choices.  See rule
 * 10.18(g) and the notes and examples following.  Basically,
 * what one has to do is, if a runner belonging to pitcher X is
 * out on a fielder's choice, "push" the responsibilities for all
 * runners back one runner.
 */
static void
cw_gameiter_push_pitchers(CWGameIterator *gameiter, int base)
{
  int b;

  for (b = base - 1; b > 0; b--) {
    if (strcmp(gameiter->runners[b], "")) {
      cw_gameiter_push_pitchers(gameiter, b);
      strcpy(gameiter->pitchers[b], gameiter->pitchers[base]);
      return;
    }
  }
  strcpy(gameiter->pitchers[0], gameiter->pitchers[base]);
}

static void
cw_gameiter_process_advance(CWGameIterator *gameiter)
{
  if ((gameiter->event_data->event_type == CW_EVENT_WALK ||
       gameiter->event_data->event_type == CW_EVENT_INTENTIONALWALK) &&
      gameiter->walk_pitcher) {
    strcpy(gameiter->pitchers[0], gameiter->walk_pitcher);
  }
  else {
    strncpy(gameiter->pitchers[0],
	    gameiter->fielders[1][1-gameiter->half_inning], 49);
  }

  if (gameiter->event_data->advance[3] >= 4 ||
      cw_event_runner_put_out(gameiter->event_data, 3)) {
    if (gameiter->event_data->fc_flag[3] &&
	cw_event_runner_put_out(gameiter->event_data, 3)) {
      cw_gameiter_push_pitchers(gameiter, 3);
    }
    strcpy(gameiter->runners[3], "");
    strcpy(gameiter->pitchers[3], "");
  }

  if (gameiter->event_data->advance[2] == 3) {
    strcpy(gameiter->runners[3], gameiter->runners[2]);
    strcpy(gameiter->pitchers[3], gameiter->pitchers[2]);
  }
  if (gameiter->event_data->advance[2] >= 3 ||
      cw_event_runner_put_out(gameiter->event_data, 2)) {
    if (gameiter->event_data->fc_flag[2] &&
	cw_event_runner_put_out(gameiter->event_data, 2)) {
      cw_gameiter_push_pitchers(gameiter, 2);
    }
    strcpy(gameiter->runners[2], "");
    strcpy(gameiter->pitchers[2], "");
  }
    
  if (gameiter->event_data->advance[1] == 2) {
    strcpy(gameiter->runners[2], gameiter->runners[1]);
    strcpy(gameiter->pitchers[2], gameiter->pitchers[1]);
  }
  else if (gameiter->event_data->advance[1] == 3) {
    strcpy(gameiter->runners[3], gameiter->runners[1]);
    strcpy(gameiter->pitchers[3], gameiter->pitchers[1]);
  }
  if (gameiter->event_data->advance[1] >= 2 || 
      cw_event_runner_put_out(gameiter->event_data, 1)) {
    if (gameiter->event_data->fc_flag[1] &&
	cw_event_runner_put_out(gameiter->event_data, 1)) {
      strcpy(gameiter->pitchers[0], gameiter->pitchers[1]);
    }
    strcpy(gameiter->runners[1], "");
    strcpy(gameiter->pitchers[1], "");
  }

  if (gameiter->event_data->advance[0] >= 1 &&
      gameiter->event_data->advance[0] <= 3) {
    strncpy(gameiter->runners[gameiter->event_data->advance[0]], 
	    gameiter->event->batter, 49);
    strcpy(gameiter->pitchers[gameiter->event_data->advance[0]], 
	   gameiter->pitchers[0]);
  }
}

void 
cw_gameiter_next(CWGameIterator *gameiter)
{
  CWEvent *event = gameiter->event;

  if (strcmp(event->event_text, "NP")) {
    int i;
    gameiter->event_count++;

    gameiter->score[gameiter->half_inning] += 
      cw_event_runs_on_play(gameiter->event_data);
    gameiter->hits[gameiter->half_inning] +=
      (gameiter->event_data->event_type >= CW_EVENT_SINGLE &&
       gameiter->event_data->event_type <= CW_EVENT_HOMERUN) ? 1 : 0;
    gameiter->errors[1 - gameiter->half_inning] += 
      gameiter->event_data->num_errors;
    gameiter->times_out[gameiter->half_inning] += 
      cw_event_outs_on_play(gameiter->event_data);
    gameiter->outs += cw_event_outs_on_play(gameiter->event_data);

    cw_gameiter_process_advance(gameiter);

    if (gameiter->removed_for_ph) {
      free(gameiter->removed_for_ph);
      gameiter->removed_for_ph = NULL;
    }

    if (cw_event_is_batter(gameiter->event_data)) {
      gameiter->num_batters[gameiter->half_inning]++;
      gameiter->ph_flag = 0;
      gameiter->is_leadoff = 0;

      if (gameiter->walk_pitcher) {
	free(gameiter->walk_pitcher);
	gameiter->walk_pitcher = NULL;
      }

      if (gameiter->strikeout_batter) {
	free(gameiter->strikeout_batter);
	gameiter->strikeout_batter = NULL;
      }
    }

    for (i = 1; i <= 3; i++) {
      if (gameiter->removed_for_pr[i]) {
	free(gameiter->removed_for_pr[i]);
	gameiter->removed_for_pr[i] = NULL;
      }
    }

    if (gameiter->outs >= 3 && gameiter->event->next != NULL) {
      /* Suppress changing sides if game is over */
      cw_gameiter_change_sides(gameiter);
    }
  }

  cw_gameiter_process_subs(gameiter, event);
  gameiter->event = gameiter->event->next;
  if (gameiter->event && strcmp(gameiter->event->event_text, "NP")) {
    int i;
    cw_parse_event(gameiter->event->event_text, gameiter->event_data);
    for (i = 1; i <= 3; i++) {
      if (gameiter->event_data->advance[i] == 0 &&
	  strcmp(gameiter->runners[i], "") &&
	  !cw_event_runner_put_out(gameiter->event_data, i)) {
	gameiter->event_data->advance[i] = i;
      }
    }

    if (gameiter->event_data->event_type == CW_EVENT_ERROR &&
	gameiter->outs == 2 &&
	gameiter->event_data->rbi_flag[3] == 1) {
      /* No RBIs should be awarded, even if not explicitly noted (NR)
       * in event text*/
      gameiter->event_data->rbi_flag[3] = 0;
    }

    for (i = 0; i <= 3; i++) {
      if (gameiter->event_data->rbi_flag[i] == 2) {
	gameiter->event_data->rbi_flag[i] = 1;
      }
    }

  }
}

int
cw_gameiter_lineup_slot(CWGameIterator *gameiter, int team, char *player_id)
{
  int i;

  for (i = 0; i <= 9; i++) {
    if (gameiter->lineups[i][team].player_id &&
	!strcmp(player_id, gameiter->lineups[i][team].player_id)) {
      return i;
    }
  }

  return -1;
}

int
cw_gameiter_player_position(CWGameIterator *gameiter, 
			    int team, char *player_id)
{
  int i;

  for (i = 1; i <= 9; i++) {
    if (gameiter->lineups[i][team].player_id &&
	!strcmp(player_id, gameiter->lineups[i][team].player_id)) {
      if (gameiter->lineups[i][team].position > 10 &&
	  gameiter->dh_slot[team] == i) {
	/* Bit of a special case: bevent considers PH for DH to be
	 * a DH right away, issuing position code 10 instead of 11 */
	return 10;
      }
      else {
	return gameiter->lineups[i][team].position;
      }
    }
  }

  /* Check the pitcher last: this is in those cases where the pitcher
   * comes to bat even though the DH was in effect */
  if (gameiter->lineups[0][team].player_id &&
      !strcmp(player_id, gameiter->lineups[0][team].player_id)) {
    return gameiter->lineups[0][team].position;
  }

  return -1;
}

char *
cw_gameiter_charged_batter(CWGameIterator *gameiter)
{
  if (gameiter->event_data->event_type == CW_EVENT_STRIKEOUT &&
      gameiter->strikeout_batter != NULL) {
    return gameiter->strikeout_batter;
  }
  else {
    return gameiter->event->batter;
  }
}

char *
cw_gameiter_charged_pitcher(CWGameIterator *gameiter)
{
  if ((gameiter->event_data->event_type == CW_EVENT_WALK || 
       gameiter->event_data->event_type == CW_EVENT_INTENTIONALWALK) &&
      gameiter->walk_pitcher != NULL) {
    return gameiter->walk_pitcher;
  }
  else {
    return gameiter->fielders[1][1-gameiter->half_inning];
  }
}

int
cw_gameiter_left_on_base(CWGameIterator *gameiter, int team)
{
  return (gameiter->num_batters[team] - gameiter->score[team] -
	  gameiter->times_out[team]);
}
