/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwlib/box.h
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

#ifndef CW_BOX_H
#define CW_BOX_H

#include "game.h"

typedef struct cw_box_batting_struct {
  int g, pa, ab, r, h, b2, b3, hr, hrslam, bi, bi2out, gw, bb, ibb, so, gdp, hp, sh, sf, sb, cs, xi;
  int lisp, movedup, pitches, strikes;
} CWBoxBatting;

typedef struct cw_box_fielding_struct {
  int g, outs, bip, bf, po, a, e, dp, tp, pb, xi;
} CWBoxFielding;

/*
 * The main data structure for a player's entry in the boxscore.
 */
typedef struct cw_box_player_struct {
  char *player_id, *name;
  /* The "appearance date" - taking into account resumed games */
  char date[9];  
  CWBoxBatting *batting;
  int ph_inn, pr_inn, num_positions;
  /* The position the player was listed at in the starting lineup;
   * -1 if not a starter
   */
  int start_position;
  /* The list of positions the player played. */
  int positions[40];
  /* Fielding statistics per position (uses usual position numbering) */
  CWBoxFielding *fielding[10];
  struct cw_box_player_struct *prev, *next;
} CWBoxPlayer;

typedef struct cw_box_pitching_struct {
  int g, gs, cg, sho, gf, outs, ab, r, er, h, b2, b3, hr, hrslam, bb, ibb, so, bf, bk, wp, hb;
  int gdp, sh, sf, xi, pk;
  int w, l, sv;

  /* inherited runners/inherited runners scored
   * inherited runner is counted as scoring even if pitcher was relieved
   * before the runner scored
   */
  int inr, inrs;

  /* xb is 'extra batters' -- batters faced in an inning without getting
   * an out; xbinn is inning in which this occurred */
  int xb, xbinn;
  /* gb and fb are ground-ball outs and fly-ball outs */
  int gb, fb;
  int pitches, strikes;
} CWBoxPitching;

/*
 * The main data structure for a pitcher's entry in the boxscore.
 */
typedef struct cw_box_pitcher_struct {
  char *player_id, *name;
  CWBoxPitching *pitching;
  struct cw_box_pitcher_struct *prev, *next;
} CWBoxPitcher;

/*
 * Data structure for storing information about notable events
 * (extra-base hits, wild pitches, etc.)
 * The 'players' entry may contain a variable number of entries,
 * depending on the type of event this is reporting.
 */
typedef struct cw_box_event_struct {
  char *players[20];
  int inning, half_inning, runners, pickoff, outs, mark;
  char location[10];
  struct cw_box_event_struct *prev, *next;
} CWBoxEvent;

/*
 * The boxscore.
 * - 'slots' refers to batting order slots.  The pointer points to the
 *   last (i.e., current) player in the slot; traverse the linked list
 *   backwards to find earlier players.  Slot 0 is reserved for pitchers
 *   who do not appear in the batting order due to the use of the DH rule.
 * - 'pitchers' refers to the list of pitchers; again, the pointer refers
 *   to the last (i.e., current) pitcher; traverse the linked list backwards
 *   to find earlier pitchers.
 */
typedef struct cw_boxscore_struct {
  CWBoxPlayer *slots[10][2];
  CWBoxPitcher *pitchers[2];
  int linescore[50][2], score[2], hits[2], errors[2], dp[2], tp[2], lob[2], er[2], risp_ab[2], risp_h[2];
  /* This is a bit of a hack... really, should be able to track a full
     vector of situationals? */
  /* outs_at_end is how many outs there were when the game ended.
   * If not 3, it could be because the game was called in the middle of
   * an inning, or because of a 'walk off' event.  walk_off is 1 if
   * the latter occurs, and 0 otherwise. */
  int outs_at_end, walk_off;
  CWBoxEvent *b2_list, *b3_list, *hr_list, *sb_list, *cs_list, *po_list;
  CWBoxEvent *sh_list, *sf_list, *hp_list, *ibb_list;
  CWBoxEvent *wp_list, *bk_list, *err_list, *pb_list;
  CWBoxEvent *dp_list, *tp_list;
} CWBoxscore;

/*
 * Create a boxscore from the game 'game'
 */
CWBoxscore *cw_box_create(CWGame *game);

/*
 * Cleans up internal memory allocation associated with 'boxscore'.
 * Caller is responsible for free()ing the boxscore itself
 */
void cw_box_cleanup(CWBoxscore *boxscore);

/*
 * Find the starter for 'team' in batting order position 'slot'.
 */
CWBoxPlayer *cw_box_get_starter(CWBoxscore *boxscore, int team, int slot);

/*
 * Find the starting pitcher for 'team'.
 */
CWBoxPitcher *cw_box_get_starting_pitcher(CWBoxscore *boxscore, int team);

/*
 * Find the player entry for player with ID player_id
 */
CWBoxPlayer *cw_box_find_player(CWBoxscore *boxscore, char *player_id);

/*
 * Find the pitching entry for player with ID player_id
 */
CWBoxPitcher *cw_box_find_pitcher(CWBoxscore *boxscore, char *player_id);

#endif  /* CW_BOX_H */


