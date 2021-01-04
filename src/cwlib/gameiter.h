/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/gameiter.h
 * Declaration of structures and routines to store game state
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

#ifndef CW_GAMEITER_H
#define CW_GAMEITER_H

#include "roster.h"
#include "game.h"

typedef struct cw_game_state {
  char date[9];     /* Updates on game resumption after suspension */
  int event_count, inning, batting_team, outs, inning_batters, inning_score;
  int score[2], hits[2], errors[2], times_out[2];
  int next_batter[2], num_batters[2], dh_slot[2];
  /* Number of automatic runners put on by the tiebreaker */
  int num_auto_runners[2];
  int is_leadoff, is_new_pa, ph_flag;

  struct {
    char runner[50], pitcher[50], catcher[50];
    int src_event, is_auto;
  } runners[4];

  struct {
    char *player_id, *name;
    int position;
  } lineups[10][2];
  char *fielders[10][2];

  char *removed_for_ph, *removed_for_pr[4];
  char *walk_pitcher;      /* For application of rule 10.18(h)(1) */
  char *strikeout_batter;  /* For application of rule 10.17(b) */
  char strikeout_batter_hand;
  int removed_position;
  char *go_ahead_rbi;

  char batter_hand, pitcher_hand;
} CWGameState;

void cw_gamestate_initialize(CWGameState *);
CWGameState *cw_gamestate_copy(CWGameState *);
void cw_gamestate_cleanup(CWGameState *);
void cw_gamestate_update(CWGameState *, char *, CWEventData *);

/*
 * Returns the number of runners left on base by 'team'.
 * Current baserunners are included in this count (to match official
 * definition)
 */
int cw_gamestate_left_on_base(CWGameState *state, int team);

/*
 * Returns the position in the batting order currently occupied by
 * the player with ID 'player_id'.  Returns -1 if player is not found
 * in the lineup; 0 if the player is a non-batting pitcher.
 */
int cw_gamestate_lineup_slot(CWGameState *state,
			     int team, char *player_id);


/*
 * Returns the position which 'player_id' currently plays on defense.
 * For this function, 10 = DH, 11 = PH, 12 = PR.
 */
int cw_gamestate_player_position(CWGameState *state,
				 int team, char *player_id);


/*
 * Returns nonzero if and only if 'base' is currently occupied.
 */
int cw_gamestate_base_occupied(CWGameState *state, int base);

/*
 * The batter who is charged with the outcome of the event
 * (almost always the actual batter, except as indicated in rule 10.17(b)
 */
char *cw_gamestate_charged_batter(CWGameState *state, 
				  char *batter, CWEventData *);

/*
 * The side from which the charged batter was batting.  This may be
 * explicitly set with a 'badj' record; otherwise, use 'offRoster'
 * to look up the batter, and 'defRoster' the pitcher, assuming the
 * opposite-side rule for switch-hitters.
 */
char cw_gamestate_charged_batter_hand(CWGameState *state, char *batter,
				      CWEventData *,
				      CWRoster *offRoster,
				      CWRoster *defRoster);


/*
 * The pitcher who is charged with the outcome of the event
 * (almost always the actual pitcher, except as indicated in rule 10.18(h)
 */
char *cw_gamestate_charged_pitcher(CWGameState *state,
				   CWEventData *event_data);


/*
 * The pitcher who is charged with the scoring of the runner on base 'base'
 */
char *cw_gamestate_responsible_pitcher(CWGameState *state, 
				       CWEventData *event_data,
				       int base);

/*
 * The catcher who is charged with the scoring of the runner on base 'base'.
 * This is unofficial, but follows the same conventions as for those
 * of assigning pitcher responsibility.
 */
char *cw_gamestate_responsible_catcher(CWGameState *state, 
				       CWEventData *event_data,
				       int base);

/*
 * TODO:
 * - Add roster context to iterator (or maybe to the game?)
 */
typedef struct cw_gameiter_struct {
  CWGame *game;
  CWEvent *event;
  CWEventData *event_data;
  int parse_ok;            /* Nonzero if last event did not parse */
  CWGameState *state;
} CWGameIterator;

/*
 * Creates a new CWGameIterator, allocating memory and initializing
 * data fields.  Iterator is set to first event of the game
 * (see cw_gameiter_reset).
 */
CWGameIterator *cw_gameiter_create(CWGame *game);

/*
 * Copies a CWGameIterator, allocating memory and initializing
 * data fields.
 */
CWGameIterator *cw_gameiter_copy(CWGameIterator *gameiter);

/*
 * Cleans up a CWGameIterator, deallocating internal memory.
 * Caller is responsible for free()ing of gameiter itself
 */
void cw_gameiter_cleanup(CWGameIterator *gameiter);


/*
 * Reset iterator to beginning of game
 */
void cw_gameiter_reset(CWGameIterator *gameiter);

/*
 * Move forward one event
 */
void cw_gameiter_next(CWGameIterator *gameiter);

/*
 * Compute the eventual "fate" of the runner on 'base'
 */
int cw_gameiter_runner_fate(CWGameIterator *gameiter, int base);


#endif   /* CW_GAMEITER_H */



