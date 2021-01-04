/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2021, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/cwevent.c
 * Chadwick expanded event descriptor program
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
#include <ctype.h>

#include "cwlib/chadwick.h"

/*************************************************************************
 * Global variables for command-line options
 *************************************************************************/

extern int ascii;

/* Fields to display (-f) */
int fields[97] = {
  1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0
};

int max_field = 96;

/* Extended fields to display (-x) */
int ext_fields[63] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int max_ext_field = 62;

char program_name[20] = "cwevent";

int print_header = 0;


/*************************************************************************
 * Utility functions (in some cases, candidates for refactor to cwlib)
 *************************************************************************/

/* Compute the number of runs scored after this play in half inning */
int cwevent_future_runs(CWGameIterator *orig_gameiter)
{
  int runs = 0;
  CWGameIterator *gameiter = cw_gameiter_copy(orig_gameiter);
  cw_gameiter_next(gameiter);
  while (gameiter->event != NULL &&
	 gameiter->state->inning == orig_gameiter->state->inning &&
	 gameiter->state->batting_team == orig_gameiter->state->batting_team) {
    if (strcmp(gameiter->event->event_text, "NP")) {
      runs += cw_event_runs_on_play(gameiter->event_data);
    }
    cw_gameiter_next(gameiter);
  }
  
  cw_gameiter_cleanup(gameiter);
  free(gameiter);
  return runs;
}


/*************************************************************************
 * Functions to output fields
 *************************************************************************/

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *,
			  CWRoster *, CWRoster *);


/*
 * convenient structure to hold all information relating to a field
 * together in one place
 */
typedef struct field_struct {
  field_func f;
  char *header, *description;
} field_struct;


/*
 * preprocessor directive for conveniently declaring function signature
 */

#define DECLARE_FIELDFUNC(funcname) \
int funcname(char *buffer, CWGameIterator *gameiter, \
	     CWRoster *visitors, CWRoster *home)

/* Field 0 */
DECLARE_FIELDFUNC(cwevent_game_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-12s",
		 gameiter->game->game_id);
} 

/* Field 1 */
DECLARE_FIELDFUNC(cwevent_visiting_team)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		 cw_game_info_lookup(gameiter->game, "visteam"));
}

/* Field 2 */
DECLARE_FIELDFUNC(cwevent_inning)
{
  return sprintf(buffer, (ascii) ? "%d" : "%4d", gameiter->event->inning);
}

/* Field 3 */
DECLARE_FIELDFUNC(cwevent_batting_team)
{
  return sprintf(buffer, "%d", gameiter->event->batting_team);
}

/* Field 4 */
DECLARE_FIELDFUNC(cwevent_outs)
{
  return sprintf(buffer, "%d", gameiter->state->outs); 
}

/* Field 5 */
DECLARE_FIELDFUNC(cwevent_balls)
{
  if (strlen(gameiter->event->count) >= 2 && 
      gameiter->event->count[0] != '?' &&
      gameiter->event->count[1] != '?') {
    return sprintf(buffer, "%c", gameiter->event->count[0]);
  }
  else {
    return sprintf(buffer, "0");
  }
}

/* Field 6 */
DECLARE_FIELDFUNC(cwevent_strikes)
{
  if (strlen(gameiter->event->count) >= 2 &&
      gameiter->event->count[0] != '?' &&
      gameiter->event->count[1] != '?') {
    return sprintf(buffer, "%c", gameiter->event->count[1]);
  }
  else {
    return sprintf(buffer, "0");
  }
}

/* Field 7 */
DECLARE_FIELDFUNC(cwevent_pitches)
{  
  /* This bit of code wipes out leading whitespace, which bevent
   * does not include in its output */
  char *foo = gameiter->event->pitches;
  while (foo != '\0' && isspace(*foo)) {
    foo++;
  }
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-20s", foo);
}

/* Field 8 */
DECLARE_FIELDFUNC(cwevent_visitor_score)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->score[0]);
}

/* Field 9 */
DECLARE_FIELDFUNC(cwevent_home_score)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->state->score[1]);
}

/* Field 10 */
DECLARE_FIELDFUNC(cwevent_batter)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 gameiter->event->batter);
}

/* Field 11 */
DECLARE_FIELDFUNC(cwevent_batter_hand)
{
  char batterHand, pitcherHand;
  if (gameiter->event->batter_hand == ' ') {
    batterHand = cw_roster_batting_hand((gameiter->event->batting_team == 0) ? 
					visitors : home,
					gameiter->event->batter);
  }
  else {
    batterHand = gameiter->event->batter_hand;
  }

  if (batterHand == 'B') {
    if (gameiter->state->pitcher_hand != ' ') {
      pitcherHand = gameiter->state->pitcher_hand;
    }
    else{
      pitcherHand =
	cw_roster_throwing_hand((gameiter->event->batting_team == 0) ?
				home : visitors,
				gameiter->state->fielders[1][1-gameiter->state->batting_team]);
    }
    if (pitcherHand == 'L') {
      batterHand = 'R';
    }
    else if (pitcherHand == 'R') {
      batterHand = 'L';
    }
    else {
      batterHand = '?';
    }
  }

  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", batterHand);
}

/* Field 12 */
DECLARE_FIELDFUNC(cwevent_res_batter)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_charged_batter(gameiter->state,
					     gameiter->event->batter,
					     gameiter->event_data));
}

/* Field 13 */
DECLARE_FIELDFUNC(cwevent_res_batter_hand)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 cw_gamestate_charged_batter_hand(gameiter->state,
						  gameiter->event->batter,
						  gameiter->event_data,
						  (gameiter->event->batting_team == 0) ? visitors : home,
						  (gameiter->event->batting_team == 0) ? home : visitors));
}

/* Field 14 */
DECLARE_FIELDFUNC(cwevent_pitcher)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
	  gameiter->state->fielders[1][1-gameiter->state->batting_team]);
}

/* Field 15 */
DECLARE_FIELDFUNC(cwevent_pitcher_hand)
{
  char pitcherHand;

  if (gameiter->event->pitcher_hand == ' ') {
    pitcherHand = 
      cw_roster_throwing_hand((gameiter->event->batting_team == 0) ?
			      home : visitors,
			      gameiter->state->fielders[1][1-gameiter->state->batting_team]);
  }
  else {
    pitcherHand = gameiter->event->pitcher_hand;
  }

  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", pitcherHand);
}

/* Field 16 */
DECLARE_FIELDFUNC(cwevent_res_pitcher)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_charged_pitcher(gameiter->state,
					      gameiter->event_data));
}

/* Field 17 */
DECLARE_FIELDFUNC(cwevent_res_pitcher_hand)
{
  char resPitcherHand;

  if (gameiter->event->pitcher_hand == ' ') {
    resPitcherHand = 
      cw_roster_throwing_hand((gameiter->event->batting_team == 0) ?
			      home : visitors,
			      cw_gamestate_charged_pitcher(gameiter->state,
							   gameiter->event_data));
  }
  else {
    resPitcherHand = gameiter->event->pitcher_hand;
  }

  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", resPitcherHand);
}

/* Field 18 */
DECLARE_FIELDFUNC(cwevent_catcher)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[2][1-gameiter->state->batting_team]);
}

/* Field 19 */
DECLARE_FIELDFUNC(cwevent_first_baseman)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[3][1-gameiter->state->batting_team]);
}

/* Field 20 */
DECLARE_FIELDFUNC(cwevent_second_baseman)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[4][1-gameiter->state->batting_team]);
}

/* Field 21 */
DECLARE_FIELDFUNC(cwevent_third_baseman)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[5][1-gameiter->state->batting_team]);
}

/* Field 22 */
DECLARE_FIELDFUNC(cwevent_shortstop)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[6][1-gameiter->state->batting_team]);
}

/* Field 23 */
DECLARE_FIELDFUNC(cwevent_left_fielder)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[7][1-gameiter->state->batting_team]);
}

/* Field 24 */
DECLARE_FIELDFUNC(cwevent_center_fielder)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->fielders[8][1-gameiter->state->batting_team]);
}

/* Field 25 */
DECLARE_FIELDFUNC(cwevent_right_fielder)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
	  gameiter->state->fielders[9][1-gameiter->state->batting_team]);
}

/* Field 26 */
DECLARE_FIELDFUNC(cwevent_runner_first)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->runners[1].runner);
}

/* Field 27 */
DECLARE_FIELDFUNC(cwevent_runner_second)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->runners[2].runner);
}

/* Field 28 */
DECLARE_FIELDFUNC(cwevent_runner_third)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->state->runners[3].runner);
}

/* Field 29 */
DECLARE_FIELDFUNC(cwevent_event_text)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-20s",
		 gameiter->event->event_text);
}

/* Field 30 */
DECLARE_FIELDFUNC(cwevent_leadoff_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->state->is_leadoff) ? 'T' : 'F');
}

/* Field 31 */
DECLARE_FIELDFUNC(cwevent_ph_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->state->ph_flag) ? 'T' : 'F');
}

/* Field 32 */
DECLARE_FIELDFUNC(cwevent_defensive_position)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 cw_gamestate_player_position(gameiter->state,
					      gameiter->state->batting_team,
					      gameiter->event->batter));
}

/* Field 33 */
DECLARE_FIELDFUNC(cwevent_lineup_position)
{
  /* 20090812: used to kludge to handle the case where the pitcher entered
   * the lineup after the DH goes away.  Removing that, as I believe this
   * case should be viewed as an error in the Retrosheet files.  Will
   * discuss with DWS. Removing the kludge will cause a diff to show up
   * against DiamondWare output (and in regression testing).
   */

  return sprintf(buffer, "%d",
		 cw_gamestate_lineup_slot(gameiter->state,
					  gameiter->state->batting_team,
					  gameiter->event->batter));
}

/* Field 34 */
DECLARE_FIELDFUNC(cwevent_event_type)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->event_data->event_type);
}

/* Field 35 */
DECLARE_FIELDFUNC(cwevent_batter_event_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 cw_event_is_batter(gameiter->event_data) ? 'T' : 'F');
}

/* Field 36 */
DECLARE_FIELDFUNC(cwevent_ab_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 cw_event_is_official_ab(gameiter->event_data) ? 'T' : 'F');
}

/* Field 37 */
DECLARE_FIELDFUNC(cwevent_hit_value)
{
  return sprintf(buffer, "%d", 
		 (gameiter->event_data->event_type >= CW_EVENT_SINGLE &&
		  gameiter->event_data->event_type <= CW_EVENT_HOMERUN) ?
		 gameiter->event_data->event_type - CW_EVENT_SINGLE + 1 : 0);
}

/* Field 38 */
DECLARE_FIELDFUNC(cwevent_sh_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->sh_flag ? 'T' : 'F');
}

/* Field 39 */
DECLARE_FIELDFUNC(cwevent_sf_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->sf_flag ? 'T' : 'F');
}

/* Field 40 */
DECLARE_FIELDFUNC(cwevent_outs_on_play)
{
  return sprintf(buffer, "%d", cw_event_outs_on_play(gameiter->event_data));
}

/* Field 41 */
DECLARE_FIELDFUNC(cwevent_dp_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->dp_flag ? 'T' : 'F');
}

/* Field 42 */
DECLARE_FIELDFUNC(cwevent_tp_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->tp_flag ? 'T' : 'F');
}

/* Field 43 */
DECLARE_FIELDFUNC(cwevent_rbi_on_play)
{
  return sprintf(buffer, "%d", cw_event_rbi_on_play(gameiter->event_data));
}

/* Field 44 */
DECLARE_FIELDFUNC(cwevent_wp_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->wp_flag ? 'T' : 'F');
}

/* Field 45 */
DECLARE_FIELDFUNC(cwevent_pb_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->pb_flag ? 'T' : 'F');
}

/* Field 46 */
DECLARE_FIELDFUNC(cwevent_fielded_by)
{
  return sprintf(buffer, "%d", gameiter->event_data->fielded_by);
}

/* Field 47 */
DECLARE_FIELDFUNC(cwevent_batted_ball_type)
{
  if (gameiter->event_data->batted_ball_type != ' ') {
    return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		   gameiter->event_data->batted_ball_type);
  }
  else {
    return sprintf(buffer, (ascii) ? "\"\"" : " ");
  }
}

/* Field 48 */
DECLARE_FIELDFUNC(cwevent_bunt_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->bunt_flag ? 'T' : 'F');
}

/* Field 49 */
DECLARE_FIELDFUNC(cwevent_foul_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->foul_flag ? 'T' : 'F');
}

/* Field 50 */
DECLARE_FIELDFUNC(cwevent_hit_location)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%4s",
		 gameiter->event_data->hit_location);
}

/* Field 51 */
DECLARE_FIELDFUNC(cwevent_num_errors)
{
  return sprintf(buffer, "%d", gameiter->event_data->num_errors);
}

/* Field 52 */
DECLARE_FIELDFUNC(cwevent_error1_player)
{
  return sprintf(buffer, "%d", gameiter->event_data->errors[0]);
}

/* Field 53 */
DECLARE_FIELDFUNC(cwevent_error1_type)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "   %c",
		 gameiter->event_data->error_types[0]);
}

/* Field 54 */
DECLARE_FIELDFUNC(cwevent_error2_player)
{
  return sprintf(buffer, "%d", gameiter->event_data->errors[1]);
}

/* Field 55 */
DECLARE_FIELDFUNC(cwevent_error2_type)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "   %c",
		 gameiter->event_data->error_types[1]);
}

/* Field 56 */
DECLARE_FIELDFUNC(cwevent_error3_player)
{
  return sprintf(buffer, "%d", gameiter->event_data->errors[2]);
}

/* Field 57 */
DECLARE_FIELDFUNC(cwevent_error3_type)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "   %c",
		 gameiter->event_data->error_types[2]);
}

/* Field 58 */
DECLARE_FIELDFUNC(cwevent_batter_advance)
{
  return sprintf(buffer, "%d", gameiter->event_data->advance[0]);
}

/* Field 59 */
DECLARE_FIELDFUNC(cwevent_runner1_advance)
{
  return sprintf(buffer, "%d", gameiter->event_data->advance[1]);
}

/* Field 60 */
DECLARE_FIELDFUNC(cwevent_runner2_advance)
{
  return sprintf(buffer, "%d", gameiter->event_data->advance[2]);
}

/* Field 61 */
DECLARE_FIELDFUNC(cwevent_runner3_advance)
{
  return sprintf(buffer, "%d", gameiter->event_data->advance[3]);
}

/* Field 62 */
DECLARE_FIELDFUNC(cwevent_play_on_batter)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%12s",
		 gameiter->event_data->play[0]);
}

/* Field 63 */
DECLARE_FIELDFUNC(cwevent_play_on_runner1)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%12s",
		 gameiter->event_data->play[1]);
}

/* Field 64 */
DECLARE_FIELDFUNC(cwevent_play_on_runner2)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%12s",
		 gameiter->event_data->play[2]);
}

/* Field 65 */
DECLARE_FIELDFUNC(cwevent_play_on_runner3)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%12s",
		 gameiter->event_data->play[3]);
}

/* Field 66 */
DECLARE_FIELDFUNC(cwevent_sb2_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->sb_flag[1] ? 'T' : 'F');
}

/* Field 67 */
DECLARE_FIELDFUNC(cwevent_sb3_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->sb_flag[2] ? 'T' : 'F');
}

/* Field 68 */
DECLARE_FIELDFUNC(cwevent_sbh_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->sb_flag[3] ? 'T' : 'F');
}

/* Field 69 */
DECLARE_FIELDFUNC(cwevent_cs2_flag)
{ 
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->cs_flag[1] ? 'T' : 'F');
}

/* Field 70 */
DECLARE_FIELDFUNC(cwevent_cs3_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->cs_flag[2] ? 'T' : 'F');
}

/* Field 71 */
DECLARE_FIELDFUNC(cwevent_csh_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->cs_flag[3] ? 'T' : 'F');
}

/* Field 72 */
DECLARE_FIELDFUNC(cwevent_po1_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->po_flag[1] ? 'T' : 'F');
}

/* Field 73 */
DECLARE_FIELDFUNC(cwevent_po2_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->po_flag[2] ? 'T' : 'F');
}

/* Field 74 */
DECLARE_FIELDFUNC(cwevent_po3_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 gameiter->event_data->po_flag[3] ? 'T' : 'F');
}

/* Field 75 */
DECLARE_FIELDFUNC(cwevent_responsible_pitcher1)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_responsible_pitcher(gameiter->state, 
						  gameiter->event_data, 1));
}

/* Field 76 */
DECLARE_FIELDFUNC(cwevent_responsible_pitcher2)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_responsible_pitcher(gameiter->state,
						  gameiter->event_data, 2));
}

/* Field 77 */
DECLARE_FIELDFUNC(cwevent_responsible_pitcher3)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_responsible_pitcher(gameiter->state,
						  gameiter->event_data, 3));
}

/* Field 78 */
DECLARE_FIELDFUNC(cwevent_new_game_flag)
{
  CWEvent *event = gameiter->event->prev;
  while (event && !strcmp(event->event_text, "NP")) {
    event = event->prev;
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (event == NULL) ? 'T' : 'F');
}

/* Field 79 */
DECLARE_FIELDFUNC(cwevent_end_game_flag)
{
  CWEvent *event = gameiter->event->next;
  while (event && !strcmp(event->event_text, "NP")) {
    event = event->next;
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (event == NULL) ? 'T' : 'F');
}

/* Field 80 */
DECLARE_FIELDFUNC(cwevent_pr1_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "       %c", 
		 (gameiter->state->removed_for_pr[1]) ? 'T' : 'F');
}

/* Field 81 */
DECLARE_FIELDFUNC(cwevent_pr2_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "       %c",
		 (gameiter->state->removed_for_pr[2]) ? 'T' : 'F');
}

/* Field 82 */
DECLARE_FIELDFUNC(cwevent_pr3_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "       %c",
		 (gameiter->state->removed_for_pr[3]) ? 'T' : 'F');
}

/* Field 83 */
DECLARE_FIELDFUNC(cwevent_removed_runner1)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 ((gameiter->state->removed_for_pr[1]) ?
		  gameiter->state->removed_for_pr[1] : ""));
}

/* Field 84 */
DECLARE_FIELDFUNC(cwevent_removed_runner2)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 ((gameiter->state->removed_for_pr[2]) ?
		  gameiter->state->removed_for_pr[2] : ""));
}

/* Field 85 */
DECLARE_FIELDFUNC(cwevent_removed_runner3)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 ((gameiter->state->removed_for_pr[3]) ?
		  gameiter->state->removed_for_pr[3] : ""));
}

/* Field 86 */
DECLARE_FIELDFUNC(cwevent_removed_batter)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 ((gameiter->state->removed_for_ph) ? 
		  gameiter->state->removed_for_ph : ""));
}

/* Field 87 */
DECLARE_FIELDFUNC(cwevent_removed_batter_position)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 ((gameiter->state->removed_for_ph) ? 
		  gameiter->state->removed_position : 0));
}

/* Field 88 */
DECLARE_FIELDFUNC(cwevent_putout1)
{
  return sprintf(buffer, "%d", gameiter->event_data->putouts[0]);
}

/* Field 89 */
DECLARE_FIELDFUNC(cwevent_putout2)
{
  return sprintf(buffer, "%d", gameiter->event_data->putouts[1]);
}

/* Field 90 */
DECLARE_FIELDFUNC(cwevent_putout3)
{
  return sprintf(buffer, "%d", gameiter->event_data->putouts[2]);
}

/* Field 91 */
DECLARE_FIELDFUNC(cwevent_assist1)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[0]);
}

/* Field 92 */
DECLARE_FIELDFUNC(cwevent_assist2)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[1]);
}

/* Field 93 */
DECLARE_FIELDFUNC(cwevent_assist3)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[2]);
}

/* Field 94 */
DECLARE_FIELDFUNC(cwevent_assist4)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[3]);
}

/* Field 95 */
DECLARE_FIELDFUNC(cwevent_assist5)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[4]);
}

/* Field 96 */
DECLARE_FIELDFUNC(cwevent_event_number)
{
  return sprintf(buffer, (ascii) ? "%d" : "%3d",		 
		 gameiter->state->event_count + 1);
}

static field_struct field_data[] = {
  /*  0 */ { cwevent_game_id, "GAME_ID", "game id*" },
  /*  1 */ { cwevent_visiting_team, "AWAY_TEAM_ID", "visiting team*" },
  /*  2 */ { cwevent_inning, "INN_CT", "inning*" },
  /*  3 */ { cwevent_batting_team, "BAT_HOME_ID", "batting_team*" },
  /*  4 */ { cwevent_outs, "OUTS_CT", "outs*" },
  /*  5 */ { cwevent_balls, "BALLS_CT", "balls*" },
  /*  6 */ { cwevent_strikes, "STRIKES_CT", "strikes*" },
  /*  7 */ { cwevent_pitches, "PITCH_SEQ_TX", "pitch sequence" },
  /*  8 */ { cwevent_visitor_score, "AWAY_SCORE_CT", "vis score*" },
  /*  9 */ { cwevent_home_score, "HOME_SCORE_CT", "home score*" },
  /* 10 */ { cwevent_batter, "BAT_ID", "batter" },
  /* 11 */ { cwevent_batter_hand, "BAT_HAND_CD", "batter hand" },
  /* 12 */ { cwevent_res_batter, "RESP_BAT_ID", "res batter*" },
  /* 13 */ { cwevent_res_batter_hand, "RESP_BAT_HAND_CD", "res batter hand*" },
  /* 14 */ { cwevent_pitcher, "PIT_ID", "pitcher" },
  /* 15 */ { cwevent_pitcher_hand, "PIT_HAND_CD", "pitcher hand" },
  /* 16 */ { cwevent_res_pitcher, "RESP_PIT_ID", "res pitcher*" },
  /* 17 */ { cwevent_res_pitcher_hand, "RESP_PIT_HAND_CD", "res pitcher hand*" },
  /* 18 */ { cwevent_catcher, "POS2_FLD_ID", "catcher" },
  /* 19 */ { cwevent_first_baseman, "POS3_FLD_ID", "first base" },
  /* 20 */ { cwevent_second_baseman, "POS4_FLD_ID", "second base" },
  /* 21 */ { cwevent_third_baseman, "POS5_FLD_ID", "third base" },
  /* 22 */ { cwevent_shortstop, "POS6_FLD_ID", "shortstop" },
  /* 23 */ { cwevent_left_fielder, "POS7_FLD_ID", "left field" },
  /* 24 */ { cwevent_center_fielder, "POS8_FLD_ID", "center field" },
  /* 25 */ { cwevent_right_fielder, "POS9_FLD_ID", "right field" },
  /* 26 */ { cwevent_runner_first, "BASE1_RUN_ID", "first runner*" },
  /* 27 */ { cwevent_runner_second, "BASE2_RUN_ID", "second runner*" },
  /* 28 */ { cwevent_runner_third, "BASE3_RUN_ID", "third runner*" },
  /* 29 */ { cwevent_event_text, "EVENT_TX", "event text*" },
  /* 30 */ { cwevent_leadoff_flag, "LEADOFF_FL", "leadoff flag*" },
  /* 31 */ { cwevent_ph_flag, "PH_FL", "pinchhit flag*" },
  /* 32 */ { cwevent_defensive_position, "BAT_FLD_CD", "defensive position*" },
  /* 33 */ { cwevent_lineup_position, "BAT_LINEUP_ID", "lineup position*" },
  /* 34 */ { cwevent_event_type, "EVENT_CD", "event type*" },
  /* 35 */ { cwevent_batter_event_flag, "BAT_EVENT_FL", "batter event flag*" },
  /* 36 */ { cwevent_ab_flag, "AB_FL", "ab flag*" },
  /* 37 */ { cwevent_hit_value, "H_CD", "hit value*" },
  /* 38 */ { cwevent_sh_flag, "SH_FL", "SH flag*" },
  /* 39 */ { cwevent_sf_flag, "SF_FL", "SF flag*" },
  /* 40 */ { cwevent_outs_on_play, "EVENT_OUTS_CT", "outs on play*" },
  /* 41 */ { cwevent_dp_flag, "DP_FL", "double play flag" },
  /* 42 */ { cwevent_tp_flag, "TP_FL", "triple play flag" },
  /* 43 */ { cwevent_rbi_on_play, "RBI_CT", "RBI on play*" },
  /* 44 */ { cwevent_wp_flag, "WP_FL", "wild pitch flag*" },
  /* 45 */ { cwevent_pb_flag, "PB_FL", "passed ball flag*" },
  /* 46 */ { cwevent_fielded_by, "FLD_CD", "fielded by" },
  /* 47 */ { cwevent_batted_ball_type, "BATTEDBALL_CD", "batted ball type" },
  /* 48 */ { cwevent_bunt_flag, "BUNT_FL", "bunt flag" },
  /* 49 */ { cwevent_foul_flag, "FOUL_FL", "foul flag" },
  /* 50 */ { cwevent_hit_location, "BATTEDBALL_LOC_TX", "hit location" },
  /* 51 */ { cwevent_num_errors, "ERR_CT", "num errors*" },
  /* 52 */ { cwevent_error1_player, "ERR1_FLD_CD", "1st error player" },
  /* 53 */ { cwevent_error1_type, "ERR1_CD", "1st error type" },
  /* 54 */ { cwevent_error2_player, "ERR2_FLD_CD", "2nd error player" },
  /* 55 */ { cwevent_error2_type, "ERR2_CD", "2nd error type" },
  /* 56 */ { cwevent_error3_player, "ERR3_FLD_CD", "3rd error player" },
  /* 57 */ { cwevent_error3_type, "ERR3_CD", "3rd error type" },
  /* 58 */ { cwevent_batter_advance, "BAT_DEST_ID", 
	     "batter dest* (5 if scores and unearned, 6 if team unearned)" },
  /* 59 */ { cwevent_runner1_advance, "RUN1_DEST_ID",
	     "runner on 1st dest* (5 if scores and unearned, 6 if team unearned)" },
  /* 60 */ { cwevent_runner2_advance, "RUN2_DEST_ID",
	     "runner on 2nd dest* (5 if scores and unearned, 6 if team unearned)" },
  /* 61 */ { cwevent_runner3_advance, "RUN3_DEST_ID",
	     "runner on 3rd dest* (5 if scores and unearned, 6 if team unearned)" },
  /* 62 */ { cwevent_play_on_batter, "BAT_PLAY_TX", "play on batter" },
  /* 63 */ { cwevent_play_on_runner1, "RUN1_PLAY_TX", 
	     "play on runner on first" },
  /* 64 */ { cwevent_play_on_runner2, "RUN2_PLAY_TX", 
	     "play on runner on second" },
  /* 65 */ { cwevent_play_on_runner3, "RUN3_PLAY_TX", 
	     "play on runner on third" },
  /* 66 */ { cwevent_sb2_flag, "RUN1_SB_FL", "SB for runner on 1st flag" },
  /* 67 */ { cwevent_sb3_flag, "RUN2_SB_FL", "SB for runner on 2nd flag" },
  /* 68 */ { cwevent_sbh_flag, "RUN3_SB_FL", "SB for runner on 3rd flag" },
  /* 69 */ { cwevent_cs2_flag, "RUN1_CS_FL", "CS for runner on 1st flag" },
  /* 70 */ { cwevent_cs3_flag, "RUN2_CS_FL", "CS for runner on 2nd flag" },
  /* 71 */ { cwevent_csh_flag, "RUN3_CS_FL", "CS for runner on 3rd flag" },
  /* 72 */ { cwevent_po1_flag, "RUN1_PK_FL", "PO for runner on 1st flag" },
  /* 73 */ { cwevent_po2_flag, "RUN2_PK_FL", "PO for runner on 2nd flag" },
  /* 74 */ { cwevent_po3_flag, "RUN3_PK_FL", "PO for runner on 3rd flag" },
  /* 75 */ { cwevent_responsible_pitcher1, "RUN1_RESP_PIT_ID", 
	     "Responsible pitcher for runner on 1st" },
  /* 76 */ { cwevent_responsible_pitcher2, "RUN2_RESP_PIT_ID", 
	     "Responsible pitcher for runner on 2nd" },
  /* 77 */ { cwevent_responsible_pitcher3, "RUN3_RESP_PIT_ID", 
	     "Responsible pitcher for runner on 3rd" },
  /* 78 */ { cwevent_new_game_flag, "GAME_NEW_FL", "New Game Flag" },
  /* 79 */ { cwevent_end_game_flag, "GAME_END_FL", "End Game Flag" },
  /* 80 */ { cwevent_pr1_flag, "PR_RUN1_FL", "Pinch-runner on 1st" },
  /* 81 */ { cwevent_pr2_flag, "PR_RUN2_FL", "Pinch-runner on 2nd" },
  /* 82 */ { cwevent_pr3_flag, "PR_RUN3_FL", "Pinch-runner on 3rd" },
  /* 83 */ { cwevent_removed_runner1, "REMOVED_FOR_PR_RUN1_ID",
	     "Runner removed for pinch-runner on 1st" },
  /* 84 */ { cwevent_removed_runner2, "REMOVED_FOR_PR_RUN2_ID",
	     "Runner removed for pinch-runner on 2nd" },
  /* 85 */ { cwevent_removed_runner3, "REMOVED_FOR_PR_RUN3_ID",
	     "Runner removed for pinch-runner on 3rd" },
  /* 86 */ { cwevent_removed_batter, "REMOVED_FOR_PH_BAT_ID",
	     "Batter removed for pinch-hitter " },
  /* 87 */ { cwevent_removed_batter_position, "REMOVED_FOR_PH_BAT_FLD_CD",
	     "Position of batter removed for pinch-hitter" },
  /* 88 */ { cwevent_putout1, "PO1_FLD_CD",
	     "Fielder with First Putout (0 if none)" },
  /* 89 */ { cwevent_putout2, "PO2_FLD_CD",
	     "Fielder with Second Putout (0 if none)" },
  /* 90 */ { cwevent_putout3, "PO3_FLD_CD",
	     "Fielder with Third Putout (0 if none)" },
  /* 91 */ { cwevent_assist1, "ASS1_FLD_CD",
	     "Fielder with First Assist (0 if none)" },
  /* 92 */ { cwevent_assist2, "ASS2_FLD_CD",
	     "Fielder with Second Assist (0 if none)" },
  /* 93 */ { cwevent_assist3, "ASS3_FLD_CD",
	     "Fielder with Third Assist (0 if none)" },
  /* 94 */ { cwevent_assist4, "ASS4_FLD_CD",
	     "Fielder with Fourth Assist (0 if none)" },
  /* 95 */ { cwevent_assist5, "ASS5_FLD_CD",
	     "Fielder with Fifth Assist (0 if none)" },
  /* 96 */ { cwevent_event_number, "EVENT_ID", "event num" }
};

/*************************************************************************
 * Implementation of "extended" fields
 *************************************************************************/

/* Extended Field 0 */
DECLARE_FIELDFUNC(cwevent_home_team_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		 cw_game_info_lookup(gameiter->game, "hometeam"));
}

/* Extended Field 1 */
DECLARE_FIELDFUNC(cwevent_batting_team_id)
{
  if (gameiter->state->batting_team == 0) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   cw_game_info_lookup(gameiter->game, "visteam"));
  }
  else {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   cw_game_info_lookup(gameiter->game, "hometeam"));
  }
}

/* Extended Field 2 */
DECLARE_FIELDFUNC(cwevent_fielding_team_id)
{
  if (gameiter->state->batting_team == 1) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   cw_game_info_lookup(gameiter->game, "visteam"));
  }
  else {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   cw_game_info_lookup(gameiter->game, "hometeam"));
  }
}

/* Extended Field 3 */
DECLARE_FIELDFUNC(cwevent_half_inning)
{
  if (cw_game_info_lookup(gameiter->game, "htbf") &&
      !strcmp(cw_game_info_lookup(gameiter->game, "htbf"), "true")) {
    return sprintf(buffer, "%d", 1-gameiter->state->batting_team);
  }
  else {
    return sprintf(buffer, "%d", gameiter->state->batting_team);
  }
}

/* Extended Field 4 */
DECLARE_FIELDFUNC(cwevent_start_half_inning)
{
  CWEvent *event = gameiter->event->prev;

  while (event) {
    if (event->inning != gameiter->event->inning ||
	event->batting_team != gameiter->event->batting_team) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
    }
    else if (strcmp(event->event_text, "NP")) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
    }
    else {
      event = event->prev;
    }
  }

  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
}

/* Extended Field 5 */
DECLARE_FIELDFUNC(cwevent_end_half_inning)
{
  CWEvent *event = gameiter->event->next;

  while (event) {
    if (event->inning != gameiter->event->inning ||
	event->batting_team != gameiter->event->batting_team) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
    }
    else if (strcmp(event->event_text, "NP")) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
    }
    else {
      event = event->next;
    }
  }

  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
}

/* Extended Field 6 */
DECLARE_FIELDFUNC(cwevent_offense_score)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 gameiter->state->score[gameiter->state->batting_team]);
}

/* Extended Field 7 */
DECLARE_FIELDFUNC(cwevent_defense_score)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 gameiter->state->score[1-gameiter->state->batting_team]);
}

/* Extended Field 8 */
DECLARE_FIELDFUNC(cwevent_offense_score_inning)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 gameiter->state->inning_score);
}

/* Extended Field 9 */
DECLARE_FIELDFUNC(cwevent_offense_batters_game)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%3d", 
		 gameiter->state->num_batters[gameiter->state->batting_team]);
}

/* Extended Field 10 */
DECLARE_FIELDFUNC(cwevent_offense_batters_inning)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 gameiter->state->inning_batters);
}

/* Extended Field 11 */
DECLARE_FIELDFUNC(cwevent_start_pa_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 
		 ((gameiter->state->is_new_pa) ? 'T' : 'F'));
}

/* Extended Field 12 */
DECLARE_FIELDFUNC(cwevent_truncated_pa_flag)
{
  CWGameIterator *gi;

  if (cw_event_is_batter(gameiter->event_data)) {
    return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
  }

  gi = cw_gameiter_copy(gameiter);
  cw_gameiter_next(gi);
  while (gi->event != NULL &&
	 gi->state->inning == gameiter->state->inning &&
	 gi->state->batting_team == gameiter->state->batting_team) {
    if (strcmp(gi->event->event_text, "NP")) {
      if (cw_event_is_batter(gi->event_data)) {
	cw_gameiter_cleanup(gi);
	free(gi);
	return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
      }
    }
    cw_gameiter_next(gi);
  }

  cw_gameiter_cleanup(gi);
  free(gi);
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
}

/* Extended Field 13 */
DECLARE_FIELDFUNC(cwevent_base_state_start)
{
  return sprintf(buffer, "%d",
		 ((cw_gamestate_base_occupied(gameiter->state, 3) ? 4 : 0) +
		  (cw_gamestate_base_occupied(gameiter->state, 2) ? 2 : 0) +
		  (cw_gamestate_base_occupied(gameiter->state, 1) ? 1 : 0)));
}

/* Extended Field 14 */
DECLARE_FIELDFUNC(cwevent_base_state_end)
{
  int r3 = 0, r2 = 0, r1 = 0;
  int base;

  for (base = 0; base <= 3; base++) {
    if (gameiter->event_data->advance[base] == 3)  r3 = 1;
    if (gameiter->event_data->advance[base] == 2)  r2 = 1;
    if (gameiter->event_data->advance[base] == 1)  r1 = 1;
  }

  return sprintf(buffer, "%d", 4*r3 + 2*r2 + r1);
}

/* Extended Field 15 */
DECLARE_FIELDFUNC(cwevent_batter_is_starter)
{
  CWAppearance *app;

  for (app = gameiter->game->first_starter; app != NULL; app = app->next) {
    if (!strcmp(app->player_id, gameiter->event->batter)) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
    }
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
}

/* Extended Field 16 */
DECLARE_FIELDFUNC(cwevent_res_batter_is_starter)
{
  CWAppearance *app;

  for (app = gameiter->game->first_starter; app != NULL; app = app->next) {
    if (!strcmp(app->player_id, 
		cw_gamestate_charged_batter(gameiter->state,
					    gameiter->event->batter,
					    gameiter->event_data))) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
    }
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
}

/* For on deck and in the hold batters, the lineup slot of the current batter
 * is used. In the case of batting out-of-order, this lists the would-be
 * proper batters assuming the current batter's place at bat is not challenged.
 */

/* Extended Field 17 */
DECLARE_FIELDFUNC(cwevent_batter_on_deck)
{
  int lineup_slot = cw_gamestate_lineup_slot(gameiter->state,
					     gameiter->state->batting_team,
					     gameiter->event->batter);
  /* remember that lineups are 1-based, not 0-based */
  int next_batter = lineup_slot % 9 + 1;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 gameiter->state->lineups[next_batter][gameiter->state->batting_team].player_id);
}

/* Dickson's encyclopedia is somewhat on the fence as to whether 
 * "in the hold" is more proper than "in the hole."
 */
/* Extended Field 18 */
DECLARE_FIELDFUNC(cwevent_batter_in_the_hold)
{
  int lineup_slot = cw_gamestate_lineup_slot(gameiter->state,
					     gameiter->state->batting_team,
					     gameiter->event->batter);
  /* remember that lineups are 1-based, not 0-based */
  int next_batter = (lineup_slot + 1) % 9 + 1;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 gameiter->state->lineups[next_batter][gameiter->state->batting_team].player_id);
}

/* Extended Field 19 */
DECLARE_FIELDFUNC(cwevent_pitcher_is_starter)
{
  CWAppearance *app = 
    cw_game_starter_find_by_position(gameiter->game,
				     1-gameiter->state->batting_team,
				     1);

  if (app &&
      !strcmp(app->player_id,
	      gameiter->state->fielders[1][1-gameiter->state->batting_team])) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
}

/* Extended Field 20 */
DECLARE_FIELDFUNC(cwevent_res_pitcher_is_starter)
{
  CWAppearance *app = 
    cw_game_starter_find_by_position(gameiter->game,
				     1-gameiter->state->batting_team,
				     1);

  if (app &&
      !strcmp(app->player_id,
	      cw_gamestate_charged_pitcher(gameiter->state,
					   gameiter->event_data))) {
      return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'T');
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", 'F');
}

/* Extended Field 21 */
DECLARE_FIELDFUNC(cwevent_runner1_defensive_position)
{
  if (!cw_gamestate_base_occupied(gameiter->state, 1)) {
    return sprintf(buffer, "0");
  }
  else {
    return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		   cw_gamestate_player_position(gameiter->state,
						gameiter->state->batting_team,
						gameiter->state->runners[1].runner));
  }
}

/* Extended Field 22 */
DECLARE_FIELDFUNC(cwevent_runner1_lineup_position)
{
  if (!cw_gamestate_base_occupied(gameiter->state, 1)) {
    return sprintf(buffer, "0");
  }

  return sprintf(buffer, "%d",
		 cw_gamestate_lineup_slot(gameiter->state,
					  gameiter->state->batting_team,
					  gameiter->state->runners[1].runner));
}

/* Extended Field 23 */
DECLARE_FIELDFUNC(cwevent_runner1_src_event)
{
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 gameiter->state->runners[1].src_event);
}

/* Extended Field 24 */
DECLARE_FIELDFUNC(cwevent_runner2_defensive_position)
{
  if (!cw_gamestate_base_occupied(gameiter->state, 2)) {
    return sprintf(buffer, "0");
  }
  else {
    return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		   cw_gamestate_player_position(gameiter->state,
						gameiter->state->batting_team,
						gameiter->state->runners[2].runner));
  }
}

/* Extended Field 25 */
DECLARE_FIELDFUNC(cwevent_runner2_lineup_position)
{
  if (!cw_gamestate_base_occupied(gameiter->state, 2)) {
    return sprintf(buffer, "0");
  }

  return sprintf(buffer, "%d",
		 cw_gamestate_lineup_slot(gameiter->state,
					  gameiter->state->batting_team,
					  gameiter->state->runners[2].runner));
}

/* Extended Field 26 */
DECLARE_FIELDFUNC(cwevent_runner2_src_event)
{
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 gameiter->state->runners[2].src_event);
}

/* Extended Field 27 */
DECLARE_FIELDFUNC(cwevent_runner3_defensive_position)
{
  if (!cw_gamestate_base_occupied(gameiter->state, 3)) {
    return sprintf(buffer, "0");
  }
  else {
    return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		   cw_gamestate_player_position(gameiter->state,
						gameiter->state->batting_team,
						gameiter->state->runners[3].runner));
  }
}

/* Extended Field 28 */
DECLARE_FIELDFUNC(cwevent_runner3_lineup_position)
{
  if (!cw_gamestate_base_occupied(gameiter->state, 3)) {
    return sprintf(buffer, "0");
  }

  return sprintf(buffer, "%d",
		 cw_gamestate_lineup_slot(gameiter->state,
					  gameiter->state->batting_team,
					  gameiter->state->runners[3].runner));
}

/* Extended Field 29 */
DECLARE_FIELDFUNC(cwevent_runner3_src_event)
{
  return sprintf(buffer, (ascii) ? "%d" : "%3d",
		 gameiter->state->runners[3].src_event);
}


/* Extended Field 30 */
DECLARE_FIELDFUNC(cwevent_responsible_catcher1)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_responsible_catcher(gameiter->state, 
						  gameiter->event_data, 1));
}

/* Extended Field 31 */
DECLARE_FIELDFUNC(cwevent_responsible_catcher2)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_responsible_catcher(gameiter->state,
						  gameiter->event_data, 2));
}

/* Extended Field 32 */
DECLARE_FIELDFUNC(cwevent_responsible_catcher3)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gamestate_responsible_catcher(gameiter->state,
						  gameiter->event_data, 3));
}

/* Extended Field 33 */
DECLARE_FIELDFUNC(cwevent_pitches_balls)
{
  int balls = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'B' || *pitch == 'H' || *pitch == 'I' || *pitch == 'P' || 
	*pitch == 'V') {
      balls++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", balls);
}

/* Extended Field 34 */
DECLARE_FIELDFUNC(cwevent_pitches_balls_called)
{
  int balls = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'B') {
      balls++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", balls);
}

/* Extended Field 35 */
DECLARE_FIELDFUNC(cwevent_pitches_balls_intentional)
{
  int balls = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'I') {
      balls++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", balls);
}

/* Extended Field 36 */
DECLARE_FIELDFUNC(cwevent_pitches_balls_pitchout)
{
  int balls = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'P') {
      balls++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", balls);
}

/* Extended Field 37 */
DECLARE_FIELDFUNC(cwevent_pitches_balls_hit_batter)
{
  int balls = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'H') {
      balls++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", balls);
}

/* Extended Field 38 */
DECLARE_FIELDFUNC(cwevent_pitches_balls_other)
{
  int balls = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'V') {
      balls++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", balls);
}

/* Extended Field 39 */
DECLARE_FIELDFUNC(cwevent_pitches_strikes)
{
  int strikes = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'C' || *pitch == 'F' || *pitch == 'K' || *pitch == 'L' ||
	*pitch == 'M' || *pitch == 'O' || *pitch == 'Q' || *pitch == 'R' ||
	*pitch == 'S' || *pitch == 'T' || *pitch == 'X' || *pitch == 'Y') {
      strikes++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", strikes);
}

/* Extended Field 40 */
DECLARE_FIELDFUNC(cwevent_pitches_strikes_called)
{
  int strikes = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'C') {
      strikes++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", strikes);
}

/* Extended Field 41 */
DECLARE_FIELDFUNC(cwevent_pitches_strikes_swinging)
{
  int strikes = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'S' || *pitch == 'M' || *pitch == 'Q') {
      strikes++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", strikes);
}

/* Extended Field 42 */
DECLARE_FIELDFUNC(cwevent_pitches_strikes_foul)
{
  int strikes = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'F' || *pitch == 'L' || *pitch == 'O' || 
	*pitch == 'T' || *pitch == 'R') {
      strikes++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", strikes);
}

/* Extended Field 43 */
DECLARE_FIELDFUNC(cwevent_pitches_strikes_inplay)
{
  int strikes = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'X' || *pitch == 'Y') {
      strikes++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", strikes);
}

/* Extended Field 44 */
DECLARE_FIELDFUNC(cwevent_pitches_strikes_other)
{
  int strikes = 0;
  char *pitch = gameiter->event->pitches;

  while (*pitch != '\0') {
    if (*pitch == 'K') {
      strikes++;
    }
    pitch++;
  }

  return sprintf(buffer, (ascii) ? "%d" : "%02d", strikes);
}

/* Extended Field 45 */
DECLARE_FIELDFUNC(cwevent_runs_on_play)
{
  return sprintf(buffer, "%d", cw_event_runs_on_play(gameiter->event_data));
}

/* Extended Field 46 */
DECLARE_FIELDFUNC(cwevent_fielded_by_id)
{
  if (gameiter->event_data->fielded_by == 0) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", "");
  }
  else {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		   gameiter->state->fielders[gameiter->event_data->fielded_by][1-gameiter->state->batting_team]);
  }
}

/* Extended Field 47 */
DECLARE_FIELDFUNC(cwevent_force_second_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->event_data->fc_flag[1] &&
		  (gameiter->event_data->gdp_flag || 
		   gameiter->event_data->force_flag)) ? 'T' : 'F');
}

/* Extended Field 48 */
DECLARE_FIELDFUNC(cwevent_force_third_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->event_data->fc_flag[2] &&
		  (gameiter->event_data->gdp_flag || 
		   gameiter->event_data->force_flag)) ? 'T' : 'F');
}

/* Extended Field 49 */
DECLARE_FIELDFUNC(cwevent_force_home_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->event_data->fc_flag[3] &&
		  (gameiter->event_data->gdp_flag || 
		   gameiter->event_data->force_flag)) ? 'T' : 'F');
}

/* Extended Field 50 */
DECLARE_FIELDFUNC(cwevent_safe_on_error_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->event_data->event_type == CW_EVENT_ERROR ||
		  (gameiter->event_data->event_type == CW_EVENT_GENERICOUT &&
		   gameiter->event_data->muff_flag[0])) ? 'T' : 'F');
}

/* Extended Field 51 */
DECLARE_FIELDFUNC(cwevent_batter_fate)
{
  return sprintf(buffer, "%d", cw_gameiter_runner_fate(gameiter, 0));
}

/* Extended Field 52 */
DECLARE_FIELDFUNC(cwevent_runner1_fate)
{
  return sprintf(buffer, "%d", cw_gameiter_runner_fate(gameiter, 1));
}

/* Extended Field 53 */
DECLARE_FIELDFUNC(cwevent_runner2_fate)
{
  return sprintf(buffer, "%d", cw_gameiter_runner_fate(gameiter, 2));
}

/* Extended Field 54 */
DECLARE_FIELDFUNC(cwevent_runner3_fate)
{
  return sprintf(buffer, "%d", cw_gameiter_runner_fate(gameiter, 3));
}

/* Extended Field 55 */
DECLARE_FIELDFUNC(cwevent_inning_future_runs)
{
  return sprintf(buffer, (ascii) ? "%d" : "%02d",
		 cwevent_future_runs(gameiter));
}

/* Extended Field 56 */
DECLARE_FIELDFUNC(cwevent_assist6)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[5]);
}

/* Extended Field 57 */
DECLARE_FIELDFUNC(cwevent_assist7)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[6]);
}

/* Extended Field 58 */
DECLARE_FIELDFUNC(cwevent_assist8)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[7]);
}

/* Extended Field 59 */
DECLARE_FIELDFUNC(cwevent_assist9)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[8]);
}

/* Extended Field 60 */
DECLARE_FIELDFUNC(cwevent_assist10)
{
  return sprintf(buffer, "%d", gameiter->event_data->assists[9]);
}

/* Extended Field 61 */
DECLARE_FIELDFUNC(cwevent_unknown_out_flag)
{
  return sprintf(buffer, "%c",
		 (!strcmp(gameiter->event_data->play[0], "99") ||
		  !strcmp(gameiter->event_data->play[1], "99") ||
		  !strcmp(gameiter->event_data->play[2], "99") ||
		  !strcmp(gameiter->event_data->play[3], "99")) ? 'T' : 'F');
}

/* Extended Field 62 */
DECLARE_FIELDFUNC(cwevent_uncertain_play_flag)
{
  return sprintf(buffer, "%c",
		 (gameiter->event->event_text[strlen(gameiter->event->event_text)-1] == '#') ? 'T' : 'F');
}

static field_struct ext_field_data[] = {
  /*  0 */ { cwevent_home_team_id, "HOME_TEAM_ID", "home team id" },
  /*  1 */ { cwevent_batting_team_id, "BAT_TEAM_ID", "batting team id" },
  /*  2 */ { cwevent_fielding_team_id, "FLD_TEAM_ID", "fielding team id" },    
  /*  3 */ { cwevent_half_inning, "BAT_LAST_ID", 
	     "half inning (differs from batting team if home team bats first" },
  /*  4 */ { cwevent_start_half_inning, "INN_NEW_FL", 
	     "start of half inning flag" },
  /*  5 */ { cwevent_end_half_inning, "INN_END_FL", 
	     "end of half inning flag" },
  /*  6 */ { cwevent_offense_score, "START_BAT_SCORE_CT", 
	     "score for team on offense" },
  /*  7 */ { cwevent_defense_score, "START_FLD_SCORE_CT", 
	     "score for team on defense" },
  /*  8 */ { cwevent_offense_score_inning, "INN_RUNS_CT", 
	     "runs scored in this half inning" },
  /*  9 */ { cwevent_offense_batters_game, "GAME_PA_CT", 
	     "number of plate appearances in game for team on offense" },
  /* 10 */ { cwevent_offense_batters_inning, "INN_PA_CT", 
	     "number of plate appearances in inning for team on offense" },
  /* 11 */ { cwevent_start_pa_flag, "PA_NEW_FL", 
	     "start of plate appearance flag" },
  /* 12 */ { cwevent_truncated_pa_flag, "PA_TRUNC_FL", 
	     "truncated plate appearance flag" },
  /* 13 */ { cwevent_base_state_start, "START_BASES_CD", 
	     "base state at start of play" },
  /* 14 */ { cwevent_base_state_end, "END_BASES_CD", 
	     "base state at end of play" },
  /* 15 */ { cwevent_batter_is_starter, "BAT_START_FL",
	     "batter is starter flag" },
  /* 16 */ { cwevent_res_batter_is_starter, "RESP_BAT_START_FL",
	     "result batter is starter flag" }, 
  /* 17 */ { cwevent_batter_on_deck, "BAT_ON_DECK_ID",
	     "ID of batter on deck" },
  /* 18 */ { cwevent_batter_in_the_hold, "BAT_IN_HOLD_ID",
	     "ID of batter in the hold" },
  /* 19 */ { cwevent_pitcher_is_starter, "PIT_START_FL",
	     "pitcher is starter flag" },
  /* 20 */ { cwevent_res_pitcher_is_starter, "RESP_PIT_START_FL",
	     "result pitcher is starter flag" },
  /* 21 */ { cwevent_runner1_defensive_position, "RUN1_FLD_CD", 
	     "defensive position of runner on first" },
  /* 22 */ { cwevent_runner1_lineup_position, "RUN1_LINEUP_CD", 
	     "lineup position of runner on first" },
  /* 23 */ { cwevent_runner1_src_event, "RUN1_ORIGIN_EVENT_ID",
	     "event number on which runner on first reached base" },
  /* 24 */ { cwevent_runner2_defensive_position, "RUN2_FLD_CD", 
	     "defensive position of runner on second" },
  /* 25 */ { cwevent_runner2_lineup_position, "RUN2_LINEUP_CD", 
	     "lineup position of runner on second" },
  /* 26 */ { cwevent_runner2_src_event, "RUN2_ORIGIN_EVENT_ID",
	     "event number on which runner on second reached base" },
  /* 27 */ { cwevent_runner3_defensive_position, "RUN3_FLD_CD", 
	     "defensive position of runner on third" },
  /* 28 */ { cwevent_runner3_lineup_position, "RUN3_LINEUP_CD", 
	     "lineup position of runner on third" },
  /* 29 */ { cwevent_runner3_src_event, "RUN3_ORIGIN_EVENT_ID",
	     "event number on which runner on third reached base" },
  /* 30 */ { cwevent_responsible_catcher1, "RUN1_RESP_CAT_ID", 
	     "Responsible catcher for runner on 1st" },
  /* 31 */ { cwevent_responsible_catcher2, "RUN2_RESP_CAT_ID", 
	     "Responsible catcher for runner on 2nd" },
  /* 32 */ { cwevent_responsible_catcher3, "RUN3_RESP_CAT_ID", 
	     "Responsible catcher for runner on 3rd" },
  /* 33 */ { cwevent_pitches_balls, "PA_BALL_CT", 
	     "number of balls in plate appearance" },
  /* 34 */ { cwevent_pitches_balls_called, "PA_CALLED_BALL_CT", 
	     "number of called balls in plate appearance" },
  /* 35 */ { cwevent_pitches_balls_intentional, "PA_INTENT_BALL_CT", 
	     "number of intentional balls in plate appearance" },
  /* 36 */ { cwevent_pitches_balls_pitchout, "PA_PITCHOUT_BALL_CT", 
	     "number of pitchouts in plate appearance" },
  /* 37 */ { cwevent_pitches_balls_hit_batter, "PA_HITBATTER_BALL_CT", 
	     "number of pitches hitting batter in plate appearance" },
  /* 38 */ { cwevent_pitches_balls_other, "PA_OTHER_BALL_CT", 
	     "number of other balls in plate appearance" },
  /* 39 */ { cwevent_pitches_strikes, "PA_STRIKE_CT", 
	     "number of strikes in plate appearance" },
  /* 40 */ { cwevent_pitches_strikes_called, "PA_CALLED_STRIKE_CT", 
	     "number of called strikes in plate appearance" },
  /* 41 */ { cwevent_pitches_strikes_swinging, "PA_SWINGMISS_STRIKE_CT", 
	     "number of swinging strikes in plate appearance" },
  /* 42 */ { cwevent_pitches_strikes_foul, "PA_FOUL_STRIKE_CT", 
	     "number of foul balls in plate appearance" },
  /* 43 */ { cwevent_pitches_strikes_inplay, "PA_INPLAY_STRIKE_CT", 
	     "number of balls in play in plate appearance" },
  /* 44 */ { cwevent_pitches_strikes_other, "PA_OTHER_STRIKE_CT", 
	     "number of other strikes in plate appearance" },
  /* 45 */ { cwevent_runs_on_play, "EVENT_RUNS_CT", "number of runs on play" },
  /* 46 */ { cwevent_fielded_by_id, "FLD_ID", 
	     "id of player fielding batted ball" },
  /* 47 */ { cwevent_force_second_flag, "BASE2_FORCE_FL", 
	     "force play at second flag" },
  /* 48 */ { cwevent_force_third_flag, "BASE3_FORCE_FL", 
	     "force play at third flag" },
  /* 49 */ { cwevent_force_home_flag, "BASE4_FORCE_FL", 
	     "force play at home flag" },
  /* 50 */ { cwevent_safe_on_error_flag, "BAT_SAFE_ERR_FL", 
	     "batter safe on error flag" },
  /* 51 */ { cwevent_batter_fate, "BAT_FATE_ID", 
	     "fate of batter (base ultimately advanced to)" },
  /* 52 */ { cwevent_runner1_fate, "RUN1_FATE_ID", 
	     "fate of runner on first" },
  /* 53 */ { cwevent_runner2_fate, "RUN2_FATE_ID", 
	     "fate of runner on second" },
  /* 54 */ { cwevent_runner3_fate, "RUN3_FATE_ID", 
	     "fate of runner on third" },
  /* 55 */ { cwevent_inning_future_runs, "FATE_RUNS_CT", 
	     "runs scored in half inning after this event" },
  /* 56 */ { cwevent_assist6, "ASS6_FLD_CD", "fielder with sixth assist" },
  /* 57 */ { cwevent_assist7, "ASS7_FLD_CD", "fielder with seventh assist" },
  /* 58 */ { cwevent_assist8, "ASS8_FLD_CD", "fielder with eighth assist" },
  /* 59 */ { cwevent_assist9, "ASS9_FLD_CD", "fielder with ninth assist" },
  /* 60 */ { cwevent_assist10, "ASS10_FLD_CD", "fielder with tenth assist" },
  /* 61 */ { cwevent_unknown_out_flag, "UNKNOWN_OUT_EXC_FL",
             "unknown fielding credit flag" },
  /* 62 */ { cwevent_uncertain_play_flag, "UNCERTAIN_PLAY_EXC_FL",
             "uncertain play flag" }
};

void
cwevent_process_game(CWGame *game, CWRoster *visitors, CWRoster *home) 
{
  char *buf;
  char output_line[4096];
  int i, comma;
  CWGameIterator *gameiter = cw_gameiter_create(game);

  while (gameiter->event != NULL) {
    CWEvent *event = gameiter->event;

    if (!strcmp(event->event_text, "NP")) {
      cw_gameiter_next(gameiter);
      continue;
    }

    comma = 0;
    strcpy(output_line, "");
    buf = output_line;
    for (i = 0; i <= max_field; i++) {
      if (fields[i]) {
	if (ascii && comma) {
	  *(buf++) = ',';
	}
	else {
	  comma = 1;
	}
	buf += (*field_data[i].f)(buf, gameiter, visitors, home);
      }
    }

    for (i = 0; i <= max_ext_field; i++) {
      if (ext_fields[i]) {
	if (ascii && comma) {
	  *(buf++) = ',';
	}
	else {
	  comma = 1;
	}
	buf += (*ext_field_data[i].f)(buf, gameiter, visitors, home);
      }
    };

    printf("%s", output_line);
    printf("\n");

    cw_gameiter_next(gameiter);
  }

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwevent_process_game;

void
cwevent_print_help(void)
{
  fprintf(stderr, "\n\ncwevent generates files suitable for use by dBase or Lotus-like programs\n");
  fprintf(stderr, "Each record describes one event.\n");
  fprintf(stderr, "Usage: cwevent [options] eventfile...\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "  -h        print this help\n");
  fprintf(stderr, "  -i id     only process game given by id\n");
  fprintf(stderr, "  -y year   Year to process (for teamyyyy and aaayyyy.ros).\n");
  fprintf(stderr, "  -s start  Earliest date to process (mmdd).\n");
  fprintf(stderr, "  -e end    Last date to process (mmdd).\n");
  fprintf(stderr, "  -a        generate Ascii-delimited format files (default)\n");
  fprintf(stderr, "  -ft       generate Fortran format files\n");
  fprintf(stderr, "  -f flist  give list of fields to output\n");
  fprintf(stderr, "              Default is 0-6,8-9,12-13,16-17,26-40,43-45,51,58-61\n");
  fprintf(stderr, "  -x flist  give list of extended fields to output\n");
  fprintf(stderr, "              Default is none\n");
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");
  fprintf(stderr, "  -n        print field names in first row of output\n\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwevent_print_help;

void
cwevent_print_field_list(void)
{
  int i;

  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  The default fields are marked with an asterisk (*).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  for (i = 0; i <= max_field; i++) {
    fprintf(stderr, "%-2d      %s\n", i, field_data[i].description);
  }
  fprintf(stderr, "\n");

  fprintf(stderr, "These additional fields are available in this version of cwevent.\n");
  fprintf(stderr, "These are specified using the -x option, and appear in the output\n");
  fprintf(stderr, "after all fields specified with -f. By default, none of these\n");
  fprintf(stderr, "fields are output.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  for (i = 0; i <= max_ext_field; i++) {
    fprintf(stderr, "%-2d      %s\n", i, ext_field_data[i].description);
  }

  
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwevent_print_field_list;

void
cwevent_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick expanded event descriptor, version " VERSION); 
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "Copyright (c) 2002-2021\nDr T L Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)\n");
  fprintf(stderr, "This is free software, " 
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwevent_print_welcome_message;

void
cwevent_initialize(void)
{
  int i, comma = 0;
  char output_line[4096];
  char *buf;

  if (!ascii || !print_header) {
    return;
  }

  strcpy(output_line, "");
  buf = output_line;

  for (i = 0; i <= max_field; i++) {
    if (fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += sprintf(buf, "\"%s\"", field_data[i].header);
    }
  }

  for (i = 0; i <= max_ext_field; i++) {
    if (ext_fields[i]) {
      if (ascii && comma) {
	*(buf++) = ',';
      }
      else {
	comma = 1;
      }
      buf += sprintf(buf, "\"%s\"", ext_field_data[i].header);
    }
  }

  printf("%s", output_line);
  printf("\n");
}

void (*cwtools_initialize)(void) = cwevent_initialize;

void
cwevent_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwevent_cleanup;

extern char year[5];
extern char first_date[5];
extern char last_date[5];
extern char game_id[20];
extern int ascii;
extern int quiet;

extern void
cwtools_parse_field_list(char *text, int max_field, int *fields);

int
cwevent_parse_command_line(int argc, char *argv[])
{
  int i;
  strcpy(year, "");

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {
      ascii = 1;
    }
    else if (!strcmp(argv[i], "-d")) {
      (*cwtools_print_welcome_message)(argv[0]);
      (*cwtools_print_field_list)();
    }
    else if (!strcmp(argv[i], "-e")) {
      if (++i < argc) {
	strncpy(last_date, argv[i], 4);
      }
    }
    else if (!strcmp(argv[i], "-h")) {
      (*cwtools_print_welcome_message)(argv[0]);
      (*cwtools_print_help)();
    }
    else if (!strcmp(argv[i], "-q")) {
      quiet = 1;
    }
    else if (!strcmp(argv[i], "-i")) {
      if (++i < argc) {
	strncpy(game_id, argv[i], 19);
      }
    }
    else if (!strcmp(argv[i], "-f")) {
      if (++i < argc) {
	cwtools_parse_field_list(argv[i], max_field, fields);
      }
    }
    else if (!strcmp(argv[i], "-n")) {
      print_header = 1;
    }
    else if (!strcmp(argv[i], "-ft")) {
      ascii = 0;
    }
    else if (!strcmp(argv[i], "-s")) {
      if (++i < argc) {
	strncpy(first_date, argv[i], 4);
      }
    }
    else if (!strcmp(argv[i], "-x")) {
      if (++i < argc) {
	cwtools_parse_field_list(argv[i], max_ext_field, ext_fields);
      }
    }
    else if (!strcmp(argv[i], "-y")) {
      if (++i < argc) {
	strncpy(year, argv[i], 5);
      }
    }
    else if (argv[i][0] == '-') {
      fprintf(stderr, "*** Invalid option '%s'.\n", argv[i]);
      exit(1);
    }
    else {
      break;
    }
  }

  return i;
}

int (*cwtools_parse_command_line)(int, char *argv[]) = cwevent_parse_command_line;
