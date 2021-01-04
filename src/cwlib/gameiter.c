/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/gameiter.c
 * Implementation of routines to store game state
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

#include "util.h"
#include "parse.h"
#include "gameiter.h"

/***********************************************************************
 * This suite of functions implements abstractions of manipulation of
 * the current state of runners on bases, including responsibility and
 * other data tracked on them.
 ***********************************************************************/

int
cw_gamestate_base_occupied(CWGameState *state, int base)
{
  return strcmp(state->runners[base].runner, "");
}

/*
 * This places a runner on a base, setting the responsibility for the
 * runner to the current pitcher and catcher.
 */
static void
cw_gamestate_place_runner(CWGameState *state, int base, char *runner)
{
  strncpy(state->runners[base].runner, runner, 49);
  strncpy(state->runners[base].pitcher,
	  state->fielders[1][1-state->batting_team], 49);
  strncpy(state->runners[base].catcher,
	  state->fielders[2][1-state->batting_team], 49);
  state->runners[base].is_auto = 1;
}

/*
 * This places the batter(-runner) on base 0 at the start of a batter event,
 * setting the responsibility correctly given the event type.
 */
static void
cw_gamestate_place_batter(CWGameState *state, char *batter, int event_type)
{
  strncpy(state->runners[0].runner, batter, 49);
  if ((event_type == CW_EVENT_WALK ||
       event_type == CW_EVENT_INTENTIONALWALK) &&
      state->walk_pitcher) {
    strcpy(state->runners[0].pitcher, state->walk_pitcher);
  }
  else {
    strncpy(state->runners[0].pitcher,
	    state->fielders[1][1-state->batting_team], 49);
  }
  strncpy(state->runners[0].catcher,
	  state->fielders[2][1-state->batting_team], 49);
  state->runners[0].src_event = state->event_count;
  state->runners[0].is_auto = 0;
}

/* 
 * This replaces an existing runner, without changing responsibility;
 * used for pinch-runners or courtesy runners.
 */
static void
cw_gamestate_replace_runner(CWGameState *state, int base, char *runner)
{
  strncpy(state->runners[base].runner, runner, 49);
}

static void
cw_gamestate_move_runner(CWGameState *state, int src, int dest)
{
  strcpy(state->runners[dest].runner, state->runners[src].runner);
  strcpy(state->runners[dest].pitcher, state->runners[src].pitcher);
  strcpy(state->runners[dest].catcher, state->runners[src].catcher);
  state->runners[dest].src_event = state->runners[src].src_event;
  state->runners[dest].is_auto = state->runners[src].is_auto;
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
cw_gamestate_reassign_responsibility(CWGameState *state, int base)
{
  int b;

  for (b = base - 1; b > 0; b--) {
    if (cw_gamestate_base_occupied(state, b)) {
      cw_gamestate_reassign_responsibility(state, b);
      strcpy(state->runners[b].pitcher, state->runners[base].pitcher);
      strcpy(state->runners[b].catcher, state->runners[base].catcher);
      state->runners[b].is_auto = state->runners[base].is_auto;
      return;
    }
  }
  strcpy(state->runners[0].pitcher, state->runners[base].pitcher);
  strcpy(state->runners[0].catcher, state->runners[base].catcher);
  state->runners[0].is_auto = state->runners[base].is_auto;
}

static void
cw_gamestate_clear_runner(CWGameState *state, int base)
{
  strcpy(state->runners[base].runner, "");
  strcpy(state->runners[base].pitcher, "");
  strcpy(state->runners[base].catcher, "");
  state->runners[base].src_event = 0;
  state->runners[base].is_auto = 0;
}

static void
cw_gamestate_copy_runners(CWGameState *dest, CWGameState *src)
{
  int i;

  for (i = 0; i <= 3; i++) {
    strcpy(dest->runners[i].runner, src->runners[i].runner);
    strcpy(dest->runners[i].pitcher, src->runners[i].pitcher);
    strcpy(dest->runners[i].catcher, src->runners[i].catcher);
    dest->runners[i].src_event = src->runners[i].src_event;
    dest->runners[i].is_auto = src->runners[i].is_auto;
  }
}

/***********************************************************************/

void 
cw_gamestate_initialize(CWGameState *state)
{
  int i, t;

  state->event_count = 0;
  state->inning = 1;
  state->batting_team = 0;
  state->outs = 0;
  state->inning_batters = 0;
  state->inning_score = 0;

  state->score[0] = state->score[1] = 0;
  state->hits[0] = state->hits[1] = 0;
  state->errors[0] = state->errors[1] = 0;
  state->times_out[0] = state->times_out[1] = 0;
  state->next_batter[0] = 1;
  state->next_batter[1] = 1;
  state->num_batters[0] = state->num_batters[1] = 0;
  state->dh_slot[0] = state->dh_slot[1] = 0;
  state->num_auto_runners[0] = state->num_auto_runners[1] = 0;
  state->is_leadoff = 1;
  state->is_new_pa = 1;
  state->ph_flag = 0;

  for (i = 0; i <= 3; i++) {
    cw_gamestate_clear_runner(state, i);
  }

  /* Make sure to set all these to null, so reset does not attempt
   * to free() non-malloc()ed memory
   */
  state->removed_for_ph = NULL;
  state->walk_pitcher = NULL;
  state->strikeout_batter = NULL;
  state->strikeout_batter_hand = ' ';
  state->go_ahead_rbi = NULL;

  for (i = 0; i <= 3; i++) {
    state->removed_for_pr[i] = NULL;
  }

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      state->lineups[i][t].player_id = NULL;
      state->lineups[i][t].name = NULL;
      state->fielders[i][t] = NULL;
    }
  }

  state->batter_hand = ' ';
  state->pitcher_hand = ' ';
}

/* Create a copy of orig_state */
CWGameState * 
cw_gamestate_copy(CWGameState *orig_state)
{
  int i, t;
  CWGameState *state = (CWGameState *) malloc(sizeof(CWGameState));

  state->event_count = orig_state->event_count;
  state->inning = orig_state->inning;
  state->batting_team = orig_state->batting_team;
  state->outs = orig_state->outs;
  state->inning_batters = orig_state->inning_batters;
  state->inning_score = orig_state->inning_score;

  for (t = 0; t <= 1; t++) {
    state->score[t] = orig_state->score[t];
    state->hits[t] = orig_state->hits[t];
    state->errors[t] = orig_state->errors[t];
    state->times_out[t] = orig_state->times_out[t];
    state->next_batter[t] = orig_state->next_batter[t];
    state->num_batters[t] = orig_state->num_batters[t];
    state->dh_slot[t] = orig_state->dh_slot[t];
  }

  state->is_leadoff = orig_state->is_leadoff;
  state->is_new_pa = orig_state->is_new_pa;
  state->ph_flag = orig_state->ph_flag;

  cw_gamestate_copy_runners(state, orig_state);

  XCOPY(state->removed_for_ph, orig_state->removed_for_ph);
  XCOPY(state->walk_pitcher, orig_state->walk_pitcher);
  XCOPY(state->strikeout_batter, orig_state->strikeout_batter);
  state->strikeout_batter_hand = orig_state->strikeout_batter_hand;
  XCOPY(state->go_ahead_rbi, orig_state->go_ahead_rbi);

  for (i = 0; i <= 3; i++) {
    XCOPY(state->removed_for_pr[i], orig_state->removed_for_pr[i]);
  }

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      XCOPY(state->lineups[i][t].player_id, 
	    orig_state->lineups[i][t].player_id);
      XCOPY(state->lineups[i][t].name, orig_state->lineups[i][t].name);
      XCOPY(state->fielders[i][t], orig_state->fielders[i][t]);
    }
  }

  state->batter_hand = orig_state->batter_hand;
  state->pitcher_hand = orig_state->pitcher_hand;
  return state;
}

void
cw_gamestate_cleanup(CWGameState *state)
{
  int i, t;

  for (t = 0; t <= 1; t++) {
    for (i = 0; i <= 9; i++) {
      XFREE(state->lineups[i][t].player_id);
      XFREE(state->lineups[i][t].name);
      XFREE(state->fielders[i][t]);
    }
  }

  XFREE(state->removed_for_ph);
  XFREE(state->walk_pitcher);
  XFREE(state->strikeout_batter);
  XFREE(state->go_ahead_rbi);

  for (i = 0; i <= 3; i++) {
    XFREE(state->removed_for_pr[i]);
  }
}

/*
 * Private auxiliary function to check whether go-ahead RBI changes
 * based on the given event.
 */
static void
cw_gamestate_check_go_ahead_rbi(CWGameState *state, char *batter,
				CWEventData *event_data)
{
  int diff = (state->score[state->batting_team] -
	      state->score[1-state->batting_team]);
  int base;

  for (base = 3; base >= 0; base--) {
    if (event_data->advance[base] >= 4) {
      diff++;
      if (diff == 1) {
	/* This was the go-ahead run */
	if (event_data->rbi_flag[base]) {
	  state->go_ahead_rbi = (char *) malloc(strlen(batter)+1);
	  strcpy(state->go_ahead_rbi, batter);
	}
	else if (state->go_ahead_rbi) {
	  free(state->go_ahead_rbi);
	  state->go_ahead_rbi = NULL;
	}
	return;
      }
      else if (diff == 0) {
	/* This was the tying run */
	if (state->go_ahead_rbi) {
	  free(state->go_ahead_rbi);
	  state->go_ahead_rbi = NULL;
	}
      }
    }
  }
}

static void
cw_gamestate_process_advance(CWGameState *state, 
			     char *batter, CWEventData *event_data)
{
  cw_gamestate_place_batter(state, batter, event_data->event_type);
  
  if (event_data->advance[3] >= 4 ||
      cw_event_runner_put_out(event_data, 3)) {
    if (event_data->fc_flag[3] && cw_event_runner_put_out(event_data, 3)) {
      cw_gamestate_reassign_responsibility(state, 3);
    }
    cw_gamestate_clear_runner(state, 3);
  }

  if (event_data->advance[2] == 3) {
    cw_gamestate_move_runner(state, 2, 3);
  }

  if (event_data->advance[2] >= 3 || 
      cw_event_runner_put_out(event_data, 2)) {
    if (event_data->fc_flag[2] && cw_event_runner_put_out(event_data, 2)) {
      cw_gamestate_reassign_responsibility(state, 2);
    }
    cw_gamestate_clear_runner(state, 2);
  }

  if (event_data->advance[1] == 2) {
    cw_gamestate_move_runner(state, 1, 2);
  }
  else if (event_data->advance[1] == 3) {
    cw_gamestate_move_runner(state, 1, 3);
  }
  if (event_data->advance[1] >= 2 || cw_event_runner_put_out(event_data, 1)) {
    if (event_data->fc_flag[1] && cw_event_runner_put_out(event_data, 1)) {
      cw_gamestate_reassign_responsibility(state, 1);
    }
    cw_gamestate_clear_runner(state, 1);
  }

  /* Backwards advances are now supported thanks to Jean Segura.
   * These need to be processed after forward advances, to avoid
   * clobbering runner data.
   */
  if (event_data->advance[3] == 2) {
    cw_gamestate_move_runner(state, 3, 2);
    cw_gamestate_clear_runner(state, 3);
  }
  else if (event_data->advance[3] == 1) {
    cw_gamestate_move_runner(state, 3, 1);
    cw_gamestate_clear_runner(state, 3);
  }
  if (event_data->advance[2] == 1) {
    cw_gamestate_move_runner(state, 2, 1);
    cw_gamestate_clear_runner(state, 2);
  }

  if (event_data->advance[0] >= 1 && event_data->advance[0] <= 3) {
    cw_gamestate_move_runner(state, 0, event_data->advance[0]);
  }
}

void cw_gamestate_update(CWGameState *state, 
			 char *batter, CWEventData *event_data)
{
  int i;

  /* We check the go-ahead RBI change first, before updating the score.
   * It just seems easier that way.
   */
  cw_gamestate_check_go_ahead_rbi(state, batter, event_data);

  state->event_count++;
  state->score[state->batting_team] += cw_event_runs_on_play(event_data);
  state->inning_score += cw_event_runs_on_play(event_data);
  state->hits[state->batting_team] +=
    (event_data->event_type >= CW_EVENT_SINGLE &&
     event_data->event_type <= CW_EVENT_HOMERUN) ? 1 : 0;
  state->errors[1 - state->batting_team] += event_data->num_errors;
  state->times_out[state->batting_team] += cw_event_outs_on_play(event_data);
  state->outs += cw_event_outs_on_play(event_data);

  cw_gamestate_process_advance(state, batter, event_data);

  if (cw_event_is_batter(event_data)) {
    state->num_batters[state->batting_team]++;
    state->next_batter[state->batting_team]++;
    if (state->next_batter[state->batting_team] == 10) {
      state->next_batter[state->batting_team] = 1;
    }
    state->inning_batters++;
    state->ph_flag = 0;
    state->is_leadoff = 0;
    state->is_new_pa = 1;

    XFREE(state->removed_for_ph);
    XFREE(state->walk_pitcher);
    XFREE(state->strikeout_batter);
  }
  else {
    state->is_new_pa = 0;
  }

  for (i = 1; i <= 3; i++) {
    XFREE(state->removed_for_pr[i]);
  }
}

static void
cw_gamestate_substitute(CWGameState *state, 
			char *batter, char *count,
			char *player_id, char *name,
			int team, int slot, int pos)
{
  char *removedPlayer = state->lineups[slot][team].player_id;
  int removedPosition = state->lineups[slot][team].position;

  state->lineups[slot][team].player_id = 
    (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(state->lineups[slot][team].player_id,
	 player_id);

  free(state->lineups[slot][team].name);
  state->lineups[slot][team].name =
    (char *) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(state->lineups[slot][team].name, name);
  
  state->lineups[slot][team].position = pos;
  
  if (strlen(count) == 2 && count[0] != '?' && count[1] != '?') {
    if (pos == 1 && 
	(!strcmp(count, "20") ||
	 !strcmp(count, "21") || count[0] == '3')) {
      state->walk_pitcher =
	(char *) malloc((strlen(state->fielders[1][team]) + 1)
			* sizeof(char));
      strcpy(state->walk_pitcher,
	     state->fielders[1][team]);
    }
    else if (pos == 11 && state->strikeout_batter == NULL &&
	     count[1] == '2') {
      state->strikeout_batter = 
	(char *) malloc((strlen(batter) + 1) * sizeof(char));
      strcpy(state->strikeout_batter, batter);
      state->strikeout_batter_hand = state->batter_hand;
    }
  }

  if (pos <= 9) {
    free(state->fielders[pos][team]);
    state->fielders[pos][team] =
      (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
    strcpy(state->fielders[pos][team],
	   player_id);
    if (pos == 1 && slot > 0 &&
	state->lineups[0][team].player_id != NULL) {
      /* Substituting a pitcher into the batting order, eliminating
       * the DH.  Clear out slot zero.
       */
      free(state->lineups[0][team].player_id);
      state->lineups[0][team].player_id = NULL;
      free(state->lineups[0][team].name);
      state->lineups[0][team].name = NULL;
      state->dh_slot[team] = 0;
    }
  }
  else if (pos == 11) {
    state->removed_for_ph = removedPlayer;
    state->ph_flag = 1;
    state->removed_position = removedPosition;
  }
  else if (pos == 12) {
    if (!strcmp(state->runners[1].runner, removedPlayer)) {
      state->removed_for_pr[1] = removedPlayer;
      cw_gamestate_replace_runner(state, 1, player_id);
    }
    else if (!strcmp(state->runners[2].runner, removedPlayer)) {
      state->removed_for_pr[2] = removedPlayer;
      cw_gamestate_replace_runner(state, 2, player_id);
    }
    else if (!strcmp(state->runners[3].runner, removedPlayer)) {
      state->removed_for_pr[3] = removedPlayer;
      cw_gamestate_replace_runner(state, 3, player_id);
    }
  }

  if (slot > 0 && state->lineups[0][team].player_id != NULL &&
      !strcmp(state->lineups[0][team].player_id, player_id)) {
      /* Substituting a pitcher into the batting order, eliminating
       * the DH.  Clear out slot zero.
       * This circumstance ought to be illegal, but has happened
       * on at least one occasion, on 1976/9/5 when Catfish Hunter
       * came in as a pinch-hitter for a player other than the DH.
       */
    free(state->lineups[0][team].player_id);
    state->lineups[0][team].player_id = NULL;
    free(state->lineups[0][team].name);
    state->lineups[0][team].name = NULL;
    state->dh_slot[team] = 0;
  }
}

static void
cw_gamestate_change_sides(CWGameState *state, CWEvent *event)
{
  int i;

  state->inning = event->inning;
  state->batting_team = event->batting_team;
  state->outs = 0;
  state->is_leadoff = 1;
  state->is_new_pa = 1;
  state->ph_flag = 0;
  state->inning_batters = 0;
  state->inning_score = 0;

  for (i = 0; i <= 3; i++) {
    cw_gamestate_clear_runner(state, i);
  }

  /* Pinch-hitters or -runners for DH automatically become DH,
   * even though no sub record occurs */
  for (i = 0; i <= 1; i++) {
    if (state->dh_slot[i] > 0 &&
	state->lineups[state->dh_slot[i]][i].position > 10) {
      state->lineups[state->dh_slot[i]][i].position = 10;
    }
  }

  /* Clear removed batter, in case inning ends on non-batter event */
  XFREE(state->removed_for_ph);
}

int
cw_gamestate_left_on_base(CWGameState *state, int team)
{
  return (state->num_batters[team] + state->num_auto_runners[team] -
	  state->score[team] - state->times_out[team]);
}

int
cw_gamestate_lineup_slot(CWGameState *state, int team, char *player_id)
{
  int i;

  for (i = 0; i <= 9; i++) {
    if (state->lineups[i][team].player_id &&
	!strcmp(player_id, state->lineups[i][team].player_id)) {
      return i;
    }
  }

  return -1;
}

int
cw_gamestate_player_position(CWGameState *state,
			     int team, char *player_id)
{
  int i;

  for (i = 1; i <= 9; i++) {
    if (state->lineups[i][team].player_id &&
	!strcmp(player_id, state->lineups[i][team].player_id)) {
      if (state->lineups[i][team].position > 10 &&
	  state->dh_slot[team] == i) {
	/* Bit of a special case: bevent considers PH for DH to be
	 * a DH right away, issuing position code 10 instead of 11 */
	return 10;
      }
      else if (state->lineups[i][team].position > 10 &&
	       !state->ph_flag) {
	/* Pinch-hitters and pinch-runners are assigned a position
	 * of 0 ("no position") if they come up again in the same inning */
	return 0;
      }
      else {
	return state->lineups[i][team].position;
      }
    }
  }

  /* Check the pitcher last: this is in those cases where the pitcher
   * comes to bat even though the DH was in effect */
  if (state->lineups[0][team].player_id &&
      !strcmp(player_id, state->lineups[0][team].player_id)) {
    return state->lineups[0][team].position;
  }

  return -1;
}

char *
cw_gamestate_charged_batter(CWGameState *state, 
			    char *batter, CWEventData *event_data)
{
  if (event_data->event_type == CW_EVENT_STRIKEOUT &&
      state->strikeout_batter != NULL) {
    return state->strikeout_batter;
  }
  else {
    return batter;
  }
}

char
cw_gamestate_charged_batter_hand(CWGameState *state, char *batter,
				 CWEventData *event_data,
				 CWRoster *offRoster, CWRoster *defRoster)
{
  char resPitcherHand, resBatterHand;

  if (event_data->event_type == CW_EVENT_STRIKEOUT &&
      state->strikeout_batter != NULL &&
      state->strikeout_batter_hand != ' ') {
    return state->strikeout_batter_hand;
  }
  
  if (state->batter_hand == ' ') {
    resBatterHand = 
      cw_roster_batting_hand(offRoster,
			     cw_gamestate_charged_batter(state, batter,
							 event_data));
  }
  else {
    resBatterHand = state->batter_hand;
  }

  if (resBatterHand == 'B') {
    if (state->pitcher_hand != ' ') {
      resPitcherHand = state->pitcher_hand;
    }
    else {
      resPitcherHand = 
	cw_roster_throwing_hand(defRoster,
				cw_gamestate_charged_pitcher(state,
							     event_data));
    }
    if (resPitcherHand == 'L') {
      return 'R';
    }
    else if (resPitcherHand == 'R') {
      return 'L';
    }
    else {
      /* Needed in case pitcher hand is unknown */
      return '?';
    }
  }
  else {
    return resBatterHand;
  }
}

char *
cw_gamestate_charged_pitcher(CWGameState *state, CWEventData *event_data)
{
  if ((event_data->event_type == CW_EVENT_WALK || 
       event_data->event_type == CW_EVENT_INTENTIONALWALK) &&
      state->walk_pitcher) {
    return state->walk_pitcher;
  }
  else {
    return state->fielders[1][1-state->batting_team];
  }
}

/*
 * The "responsible pitcher" is usually the pitcher responsible
 * at the beginning of the play.  However, on a play like 32(3)/FO.2-H(E2),
 * the runner scoring should be charged to the pitcher who was initially
 * responsible for the runner on third, and so that pitcher is listed
 * as the responsible pitcher so that stats can be calculated directly
 * from the cwevent output without having to reparse the play.
 */
char *
cw_gamestate_responsible_pitcher(CWGameState *state, CWEventData *event_data,
				 int base)
{
  if (!cw_gamestate_base_occupied(state, base)) {
    return "";
  }
  if (base == 3) {
    return state->runners[3].pitcher;
  }
  else if (base == 2) {
    if (cw_event_runner_put_out(event_data, 3) &&
	event_data->fc_flag[3] && event_data->advance[2] >= 4) {
      return state->runners[3].pitcher;
    }
    else {
      return state->runners[2].pitcher;
    }
  }
  else {
    if (cw_event_runner_put_out(event_data, 3) &&
	event_data->fc_flag[3] && event_data->advance[2] >= 4) {
      return state->runners[2].pitcher;
    }
    else if (cw_event_runner_put_out(event_data, 3) &&
	     event_data->fc_flag[3] &&
	     !cw_gamestate_base_occupied(state, 2) &&
	     event_data->advance[1] >= 4) {
      return state->runners[3].pitcher;
    }
    else {
      return state->runners[1].pitcher;
    }
  }
}

/*
 * The "responsible catcher" (for catcher ERA) is computed using the
 * same rules as the "responsible pitcher."  See the above note for
 * cwevent_responsible_pitcher for how this is operationalized in cwevent.
 */
char *
cw_gamestate_responsible_catcher(CWGameState *state, CWEventData *event_data,
				 int base)
{
  if (base == 3) {
    return state->runners[3].catcher;
  }
  else if (base == 2) {
    if (cw_event_runner_put_out(event_data, 3) &&
	event_data->fc_flag[3] && event_data->advance[2] >= 4) {
      return state->runners[3].catcher;
    }
    else {
      return state->runners[2].catcher;
    }
  }
  else {
    if (cw_event_runner_put_out(event_data, 3) &&
	event_data->fc_flag[3] && event_data->advance[2] >= 4) {
      return state->runners[2].catcher;
    }
    else if (cw_event_runner_put_out(event_data, 3) &&
	     !cw_gamestate_base_occupied(state, 2) &&
	     event_data->advance[1] >= 4) {
      return state->runners[3].catcher;
    }
    else {
      return state->runners[1].catcher;
    }
  }
}

/*
 * Private auxiliary function to set up lineups with starters 
 */
static void
cw_gameiter_lineup_setup(CWGameIterator *gameiter)
{
  CWAppearance *starter = gameiter->game->first_starter;

  while (starter != NULL) {
    gameiter->state->lineups[starter->slot][starter->team].player_id =
      (char *) malloc(sizeof(char) * (strlen(starter->player_id) + 1));
    strcpy(gameiter->state->lineups[starter->slot][starter->team].player_id,
	   starter->player_id);

    gameiter->state->lineups[starter->slot][starter->team].name =
      (char *) malloc(sizeof(char) * (strlen(starter->name) + 1));
    strcpy(gameiter->state->lineups[starter->slot][starter->team].name,
	   starter->name);

    gameiter->state->lineups[starter->slot][starter->team].position = 
      starter->pos;

    if (starter->pos <= 9) {
      gameiter->state->fielders[starter->pos][starter->team] =
	(char *) malloc(sizeof(char) * (strlen(starter->player_id) + 1));
      strcpy(gameiter->state->fielders[starter->pos][starter->team],
	     starter->player_id);
    }
    else if (starter->pos == 10) {
      gameiter->state->dh_slot[starter->team] = starter->slot;
    }

    starter = starter->next;
  }
}

void
cw_gameiter_reset(CWGameIterator *gameiter)
{
  char *date = cw_game_info_lookup(gameiter->game, "date");

  gameiter->event = gameiter->game->first_event;

  cw_gamestate_cleanup(gameiter->state);
  cw_gamestate_initialize(gameiter->state);
  sprintf(gameiter->state->date, "%c%c%c%c%c%c%c%c",
	  date[0], date[1], date[2], date[3],
	  date[5], date[6], date[8], date[9]);
  cw_gameiter_lineup_setup(gameiter);

  if (cw_game_info_lookup(gameiter->game, "htbf") &&
      !strcmp(cw_game_info_lookup(gameiter->game, "htbf"), "true")) {
    gameiter->state->batting_team = 1;
  }
  else {
    gameiter->state->batting_team = 0;
  }

  if (gameiter->event) {
    if (strcmp(gameiter->event->event_text, "NP")) {
      gameiter->state->batter_hand = gameiter->event->batter_hand;
      gameiter->state->pitcher_hand = gameiter->event->pitcher_hand;
      gameiter->parse_ok = cw_parse_event(gameiter->event->event_text, 
					  gameiter->event_data);
    }
    else {
      /* There are some very rare instances with an NP as the first play */
      gameiter->parse_ok = 1;
    }
  }
}

CWGameIterator *
cw_gameiter_create(CWGame *game)
{
  CWGameIterator *gameiter = (CWGameIterator *) malloc(sizeof(CWGameIterator));
  gameiter->game = game;

  gameiter->event_data = (CWEventData *) malloc(sizeof(CWEventData));
  gameiter->state = (CWGameState *) malloc(sizeof(CWGameState));

  /* Initialize before reset, since initialization checks for cleanup */
  cw_gamestate_initialize(gameiter->state);
  cw_gameiter_reset(gameiter);

  return gameiter;
}

CWGameIterator *
cw_gameiter_copy(CWGameIterator *orig_gameiter)
{
  CWGameIterator *gameiter = (CWGameIterator *) malloc(sizeof(CWGameIterator));

  gameiter->game = orig_gameiter->game;
  gameiter->event = orig_gameiter->event;

  gameiter->event_data = (CWEventData *) malloc(sizeof(CWEventData));
  cw_event_data_copy(gameiter->event_data, orig_gameiter->event_data);

  gameiter->parse_ok = orig_gameiter->parse_ok;
  gameiter->state = cw_gamestate_copy(orig_gameiter->state);

  return gameiter;
}

void
cw_gameiter_cleanup(CWGameIterator *gameiter)
{
  cw_gamestate_cleanup(gameiter->state);
  XFREE(gameiter->state);
  XFREE(gameiter->event_data);
}

static void
cw_gameiter_process_subs(CWGameIterator *gameiter)
{
  CWAppearance *sub = gameiter->event->first_sub;

  while (sub != NULL) {
    cw_gamestate_substitute(gameiter->state,
			    gameiter->event->batter, gameiter->event->count,
			    sub->player_id, sub->name,
			    sub->team, sub->slot, sub->pos);
    sub = sub->next;
  }
}

static void
cw_gameiter_process_comments(CWGameIterator *gameiter)
{
  char *token;
  CWComment *comment = gameiter->event->first_comment;

  while (comment != NULL) {
    if (strstr(comment->text, "suspended,") == comment->text) {
      token = strtok(comment->text, ",");
      token = strtok(NULL, ",");
      strncpy(gameiter->state->date, token, 8);
    }
    comment = comment->next;
  }
}

void 
cw_gameiter_next(CWGameIterator *gameiter)
{
  if (strcmp(gameiter->event->event_text, "NP")) {
    cw_gamestate_update(gameiter->state, 
			gameiter->event->batter, gameiter->event_data);

  }
  else {
    gameiter->state->batter_hand = gameiter->event->batter_hand;
    gameiter->state->pitcher_hand = gameiter->event->pitcher_hand;
  }

  cw_gameiter_process_comments(gameiter);
  cw_gameiter_process_subs(gameiter);

  /* Now, move on to the next event, and parse it.
   * There are a few entries in the CWEventData that are context-dependent,
   * in the sense that they cannot fully be inferred from the 
   * event text alone.  The remaining code handles those cases.
   */
  gameiter->event = gameiter->event->next;

  if (gameiter->event != NULL &&
      (gameiter->state->inning != gameiter->event->inning || 
       gameiter->state->batting_team != gameiter->event->batting_team)) {
    /* Starting in version 0.5, we change sides whenever the event file
     * tells us the inning or batting team changes.  Prior versions
     * changed sides only when three were out -- basically ignoring
     * the inning and batting team portions of play records.  This was
     * because when Chadwick was first written, there were some Retrosheet
     * event files which had play records with incorrect inning fields,
     * presumably due to manual editing.  (They involved stolen base
     * plays which appear to have been moved from a different inning.)
     * Since these no longer occur in Retrosheet files, we respect
     * what the file tells us.
     *
     * Therefore, this would be a good place to put in some possible
     * error reporting for innings which don't have three outs.
     */
    cw_gamestate_change_sides(gameiter->state, gameiter->event);
  }

  if (gameiter->event && gameiter->event->ladj_slot != 0) {
    gameiter->state->next_batter[gameiter->state->batting_team] = gameiter->event->ladj_slot;
  }
  if (gameiter->event && gameiter->event->auto_base != 0) {
    cw_gamestate_place_runner(gameiter->state,
			      gameiter->event->auto_base,
			      gameiter->event->auto_runner_id);
  }
  if (gameiter->event && strcmp(gameiter->event->event_text, "NP")) {
    int i;
    gameiter->state->batter_hand = gameiter->event->batter_hand;
    gameiter->state->pitcher_hand = gameiter->event->pitcher_hand;
    gameiter->parse_ok = cw_parse_event(gameiter->event->event_text,
					gameiter->event_data);
    for (i = 1; i <= 3; i++) {
      if (gameiter->event_data->advance[i] == 0 &&
	  cw_gamestate_base_occupied(gameiter->state, i) &&
	  !cw_event_runner_put_out(gameiter->event_data, i)) {
	gameiter->event_data->advance[i] = i;
      }
    }

    if (gameiter->event_data->event_type == CW_EVENT_ERROR &&
	gameiter->state->outs == 2 &&
	gameiter->event_data->rbi_flag[3] == 1) {
      /* No RBIs should be awarded, even if not explicitly noted (NR)
       * in event text*/
      gameiter->event_data->rbi_flag[3] = 0;
    }
    else if ((gameiter->event_data->event_type == CW_EVENT_WALK ||
	      gameiter->event_data->event_type == CW_EVENT_INTENTIONALWALK) &&
	     (!cw_gamestate_base_occupied(gameiter->state, 2) ||
	      !cw_gamestate_base_occupied(gameiter->state, 1))) {
      gameiter->event_data->rbi_flag[3] = 0;
    }

    for (i = 0; i <= 3; i++) {
      if (gameiter->event_data->rbi_flag[i] == 2) {
	gameiter->event_data->rbi_flag[i] = 1;
      }
    }

    for (i = 0; i <- 3; i++) {
      /* New convention from 2020: Automatic runners who score
       * are reported as scoring code 7
       */
      if (gameiter->event_data->advance[i] >= 4 &&
	  gameiter->state->runners[i].is_auto) {
	gameiter->event_data->advance[i] = 7;
      }
    }
  }
}

/* Compute the eventual "fate" of the runner on 'base' */
int cw_gameiter_runner_fate(CWGameIterator *orig_gameiter, int base)
{
  CWGameIterator *gameiter;
  if (orig_gameiter->event_data->advance[base] == 0 ||
      orig_gameiter->event_data->advance[base] >= 4) {
    return orig_gameiter->event_data->advance[base];
  }
  
  base = orig_gameiter->event_data->advance[base];
  gameiter = cw_gameiter_copy(orig_gameiter);
  while (gameiter->event != NULL && 
	 gameiter->state->inning == orig_gameiter->state->inning &&
	 gameiter->state->batting_team == orig_gameiter->state->batting_team &&
	 base >= 1 && base <= 3) {
    cw_gameiter_next(gameiter);
    if (gameiter->event && strcmp(gameiter->event->event_text, "NP")) {
      base = gameiter->event_data->advance[base];
    }
  }

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
  return base;
}		

