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

#ifndef CW_BOX_H
#define CW_BOX_H

#include "game.h"

typedef struct cw_box_batting_struct {
  int ab, r, h, bi, bb, so;
} CWBoxBatting;

typedef struct cw_box_player_struct {
  char *player_id;
  CWBoxBatting *batting;
  struct cw_box_player_struct *prev, *next;
} CWBoxPlayer;

typedef struct cw_box_pitching_struct {
  int outs, r, er, h, hr, bb, so, bf;
} CWBoxPitching;

typedef struct cw_box_pitcher_struct {
  char *player_id;
  CWBoxPitching *pitching;
  struct cw_box_pitcher_struct *prev, *next;
} CWBoxPitcher;

typedef struct cw_boxscore_struct {
  CWBoxPlayer *slots[10][2];
  CWBoxPitcher *pitchers[2];
} CWBoxscore;

/*
 * Create a boxscore from the game 'game'
 */
CWBoxscore *cw_boxscore_create(CWGame *game);

/*
 * Cleans up internal memory allocation associated with 'boxscore'.
 * Caller is responsiblefor free()ing the boxscore itself
 */
void cw_boxscore_cleanup(CWBoxscore *boxscore);

#endif  /* CW_BOX_H */


