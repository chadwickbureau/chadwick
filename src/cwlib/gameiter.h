/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Declaration of structures and routines to store game state
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

#ifndef CW_STATE_H
#define CW_STATE_H

#include "game.h"

typedef struct cw_game_iter_struct {
  CWGame *game;
  CWEvent *event;
  CWParsedEvent *event_data;
  int event_count, inning, half_inning, outs, visscore, homescore;
  int num_batters[2], dh_slot[2];
  int is_leadoff, ph_flag;
  struct {
    char *player_id, *name;
    int position;
  } lineups[10][2];
  char *fielders[10][2];
  char runners[4][50], pitchers[4][50];
  char *removed_for_ph, *removed_for_pr[4];
  char *walk_pitcher;      /* For application of rule 10.18(h)(1) */
  char *strikeout_batter;  /* For application of rule 10.17(b) */
  int removed_position;
} CWGameIterator;

/*
 * Creates a new CWGameIterator, allocating memory and initializing
 * data fields.  Iterator is set to first event of the game
 * (see cw_gameiter_reset).
 */
CWGameIterator *cw_gameiter_create(CWGame *game);

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
 * Returns the position in the batting order currently occupied by
 * the player with ID 'player_id'.  Returns -1 if player is not found
 * in the lineup; 0 if the player is a non-batting pitcher.
 */
int cw_gameiter_lineup_slot(CWGameIterator *gameiter,
			    int team, char *player_id);


/*
 * Returns the position which 'player_id' currently plays on defense.
 * For this function, 10 = DH, 11 = PH, 12 = PR.
 */
int cw_gameiter_player_position(CWGameIterator *gameiter,
				int team, char *player_id);


/*
 * The batter who is charged with the outcome of the event
 * (almost always the actual batter, except as indicated in rule 10.17(b)
 */
char *cw_gameiter_charged_batter(CWGameIterator *gameiter);

/*
 * The pitcher who is charged with the outcome of the event
 * (almost always the actual pitcher, except as indicated in rule 10.18(h)
 */
char *cw_gameiter_charged_pitcher(CWGameIterator *gameiter);

#endif   /* CW_STATE_H */



