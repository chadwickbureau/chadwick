/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2012, T. L. Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *			    
 * FILE: src/cwlib/lint.c
 * Implement sanity checking for game data.
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

#include "chadwick.h"

/*
 * Check for errors in game starter records
 */
static int
cw_game_lint_starters(CWGame *game)
{
  int ok = 1;
  CWAppearance *app;

  for (app = game->first_starter; app; app = app->next) {
    if (app->slot < 0 || app->slot > 9) {
      fprintf(stderr, 
	      "ERROR: In %s, invalid slot %d for player '%s'.\n",
	      game->game_id, app->slot, app->player_id);
      ok = 0;
    }
    if (app->team < 0 || app->team > 1) {
      fprintf(stderr,
	      "ERROR: In %s, invalid team %d for player '%s'.\n",
	      game->game_id, app->team, app->player_id);
      ok = 0;
    }
    if (app->pos < 1 || app->pos > 10) {
      fprintf(stderr,
	      "ERROR: In %s, invalid position %d for player '%s'.\n",
	      game->game_id, app->pos, app->player_id);
      ok = 0;
    }
  }
  return ok;
}

/*
 * Check for errors in current game event.
 */
static int
cw_game_lint_state(CWGameIterator *gameiter)
{
  int ok = 1;
  int src, dest;

  if (!gameiter->parse_ok) {
    fprintf(stderr, "Parse error in game %s at event %d:\n",
	    gameiter->game->game_id, gameiter->state->event_count+1);
    fprintf(stderr, "Invalid play string \"%s\" (%s batting)\n",
	    gameiter->event->event_text, gameiter->event->batter);
    ok = 0;
  }

  if (gameiter->event_data->dp_flag &&
      cw_event_outs_on_play(gameiter->event_data) < 2) {
    fprintf(stderr, "Play-by-play error in game %s at event %d:\n",
	    gameiter->game->game_id, gameiter->state->event_count+1);
    fprintf(stderr, "Fewer than two outs on play marked DP (event \"%s\", %s batting)\n",
	    gameiter->event->event_text, gameiter->event->batter);
    ok = 0;
  }

  if (gameiter->event_data->tp_flag &&
      cw_event_outs_on_play(gameiter->event_data) < 3) {
    fprintf(stderr, "Play-by-play error in game %s at event %d:\n",
	    gameiter->game->game_id, gameiter->state->event_count+1);
    fprintf(stderr, "Fewer than three outs on play marked TP (event \"%s\", %s batting)\n",
	    gameiter->event->event_text, gameiter->event->batter);
    ok = 0;
  }

  for (src = 1; src <= 3; src++) {
    if (!strcmp(gameiter->state->runners[src], "") &&
	(gameiter->event_data->advance[src] != 0 ||
	 cw_event_runner_put_out(gameiter->event_data, src))) {
      fprintf(stderr, "Play-by-play error in game %s at event %d:\n",
	      gameiter->game->game_id, gameiter->state->event_count+1);
      fprintf(stderr, "Advancement from empty base %d (event \"%s\", %s batting)\n",
	      src,
	      gameiter->event->event_text, gameiter->event->batter);
      ok = 0;
    }
  }

  for (dest = 1; dest <= 3; dest++) {
    if (!strcmp(gameiter->state->runners[dest], "")) {
      continue;
    }

    for (src = 0; src < dest; src++) {
      int srcAdv = gameiter->event_data->advance[src];
      int destAdv = gameiter->event_data->advance[dest];

      if (srcAdv >= destAdv && 
	  !cw_event_runner_put_out(gameiter->event_data, dest) &&
	  destAdv < 4 &&
	  gameiter->state->outs + cw_event_outs_on_play(gameiter->event_data) < 3) {
	fprintf(stderr, "Play-by-play error in game %s at event %d:\n",
		gameiter->game->game_id, gameiter->state->event_count+1);
	fprintf(stderr, "Runner on %d overtaken by runner on %d (event \"%s\", %s batting)\n",
		dest, src,
		gameiter->event->event_text, gameiter->event->batter);
	ok = 0;
      }
    }
  }
  return ok;
}


/*
 * Examine game for internal consistency.
 */
int
cw_game_lint(CWGame *game)
{
  int ok = 1;
  CWGameIterator *gameiter = cw_gameiter_create(game);

  ok = cw_game_lint_starters(game);
  
  while (ok && gameiter->event != NULL) {
    if (strcmp(gameiter->event->event_text, "NP")) {
      ok &= cw_game_lint_state(gameiter);
    }
    cw_gameiter_next(gameiter);
  }

  free(gameiter);
  return ok;
}
