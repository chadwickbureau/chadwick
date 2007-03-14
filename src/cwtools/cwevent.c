/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick expanded event descriptor program
 * 
 * This file is part of Chadwick, tools for baseball play-by-play and stats
 * Copyright (C) 2002-2007, Ted Turocy (drarbiter@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

char program_name[20] = "cwevent";

/*************************************************************************
 * Functions to output fields
 *************************************************************************/

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *,
			  CWRoster *, CWRoster *);

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
DECLARE_FIELDFUNC(cwevent_half_inning)
{
  return sprintf(buffer, "%d", gameiter->event->half_inning);
}

/* Field 4 */
DECLARE_FIELDFUNC(cwevent_outs)
{
  return sprintf(buffer, "%d", gameiter->outs); 
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
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->score[0]);
}

/* Field 9 */
DECLARE_FIELDFUNC(cwevent_home_score)
{ 
  return sprintf(buffer, (ascii) ? "%d" : "%2d", gameiter->score[1]);
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
    batterHand = cw_roster_batting_hand((gameiter->event->half_inning == 0) ? 
					visitors : home,
					gameiter->event->batter);
  }
  else {
    batterHand = gameiter->event->batter_hand;
  }

  if (batterHand == 'B') {
    pitcherHand =
      cw_roster_throwing_hand((gameiter->event->half_inning == 0) ?
			      home : visitors,
			      gameiter->fielders[1][1-gameiter->half_inning]);

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
		 cw_gameiter_charged_batter(gameiter));
}

/* Field 13 */
DECLARE_FIELDFUNC(cwevent_res_batter_hand)
{
  char resPitcherHand, resBatterHand;

  if (gameiter->event->batter_hand == ' ') {
    resBatterHand = 
      cw_roster_batting_hand((gameiter->event->half_inning == 0) ?
			     visitors : home,
			     cw_gameiter_charged_batter(gameiter));
  }
  else {
    resBatterHand = gameiter->event->batter_hand;
  }

  if (resBatterHand == 'B') {
    resPitcherHand = 
      cw_roster_throwing_hand((gameiter->event->half_inning == 0) ?
			      home : visitors,
			      cw_gameiter_charged_pitcher(gameiter));
    if (resPitcherHand == 'L') {
      resBatterHand = 'R';
    }
    else if (resPitcherHand == 'R') {
      resBatterHand = 'L';
    }
    else {
      /* Needed in case pitcher hand is unknown */
      resBatterHand = '?';
    }
  }
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c", resBatterHand);
}

/* Field 14 */
DECLARE_FIELDFUNC(cwevent_pitcher)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
	  gameiter->fielders[1][1-gameiter->half_inning]);
}

/* Field 15 */
DECLARE_FIELDFUNC(cwevent_pitcher_hand)
{
  char pitcherHand;

  if (gameiter->event->pitcher_hand == ' ') {
    pitcherHand = 
      cw_roster_throwing_hand((gameiter->event->half_inning == 0) ?
			      home : visitors,
			      gameiter->fielders[1][1-gameiter->half_inning]);
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
		 cw_gameiter_charged_pitcher(gameiter));
}

/* Field 17 */
DECLARE_FIELDFUNC(cwevent_res_pitcher_hand)
{
  char resPitcherHand;

  if (gameiter->event->pitcher_hand == ' ') {
    resPitcherHand = 
      cw_roster_throwing_hand((gameiter->event->half_inning == 0) ?
			      home : visitors,
			      cw_gameiter_charged_pitcher(gameiter));
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
		 gameiter->fielders[2][1-gameiter->half_inning]);
}

/* Field 19 */
DECLARE_FIELDFUNC(cwevent_first_baseman)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->fielders[3][1-gameiter->half_inning]);
}

/* Field 20 */
DECLARE_FIELDFUNC(cwevent_second_baseman)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->fielders[4][1-gameiter->half_inning]);
}

/* Field 21 */
DECLARE_FIELDFUNC(cwevent_third_baseman)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->fielders[5][1-gameiter->half_inning]);
}

/* Field 22 */
DECLARE_FIELDFUNC(cwevent_shortstop)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->fielders[6][1-gameiter->half_inning]);
}

/* Field 23 */
DECLARE_FIELDFUNC(cwevent_left_fielder)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->fielders[7][1-gameiter->half_inning]);
}

/* Field 24 */
DECLARE_FIELDFUNC(cwevent_center_fielder)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->fielders[8][1-gameiter->half_inning]);
}

/* Field 25 */
DECLARE_FIELDFUNC(cwevent_right_fielder)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
	  gameiter->fielders[9][1-gameiter->half_inning]);
}

/* Field 26 */
DECLARE_FIELDFUNC(cwevent_runner_first)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->runners[1]);
}

/* Field 27 */
DECLARE_FIELDFUNC(cwevent_runner_second)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->runners[2]);
}

/* Field 28 */
DECLARE_FIELDFUNC(cwevent_runner_third)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 gameiter->runners[3]);
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
		 (gameiter->is_leadoff) ? 'T' : 'F');
}

/* Field 31 */
DECLARE_FIELDFUNC(cwevent_ph_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "%c",
		 (gameiter->ph_flag) ? 'T' : 'F');
}

/* Field 32 */
DECLARE_FIELDFUNC(cwevent_defensive_position)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 cw_gameiter_player_position(gameiter,
					     gameiter->half_inning,
					     gameiter->event->batter));
}

/* Field 33 */
DECLARE_FIELDFUNC(cwevent_lineup_position)
{
  /* A bit of a kludge to handle case where pitcher enters the lineup
   * after DH goes away */
  int lineupSlot = cw_gameiter_lineup_slot(gameiter,
					   gameiter->half_inning,
					   gameiter->event->batter);
  if (lineupSlot > 0) {
    return sprintf(buffer, "%d", lineupSlot);
  }
  else {
    return sprintf(buffer, "%d",
	    (gameiter->num_batters[gameiter->half_inning]) % 9 + 1);
  }
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
		 cw_gameiter_responsible_pitcher(gameiter, 1));
}

/* Field 76 */
DECLARE_FIELDFUNC(cwevent_responsible_pitcher2)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gameiter_responsible_pitcher(gameiter, 2));
}

/* Field 77 */
DECLARE_FIELDFUNC(cwevent_responsible_pitcher3)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 cw_gameiter_responsible_pitcher(gameiter, 3));
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
		 (gameiter->removed_for_pr[1]) ? 'T' : 'F');
}

/* Field 81 */
DECLARE_FIELDFUNC(cwevent_pr2_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "       %c",
		 (gameiter->removed_for_pr[2]) ? 'T' : 'F');
}

/* Field 82 */
DECLARE_FIELDFUNC(cwevent_pr3_flag)
{
  return sprintf(buffer, (ascii) ? "\"%c\"" : "       %c",
		 (gameiter->removed_for_pr[3]) ? 'T' : 'F');
}

/* Field 83 */
DECLARE_FIELDFUNC(cwevent_removed_runner1)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 ((gameiter->removed_for_pr[1]) ?
		  gameiter->removed_for_pr[1] : ""));
}

/* Field 84 */
DECLARE_FIELDFUNC(cwevent_removed_runner2)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 ((gameiter->removed_for_pr[2]) ?
		  gameiter->removed_for_pr[2] : ""));
}

/* Field 85 */
DECLARE_FIELDFUNC(cwevent_removed_runner3)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s", 
		 ((gameiter->removed_for_pr[3]) ?
		  gameiter->removed_for_pr[3] : ""));
}

/* Field 86 */
DECLARE_FIELDFUNC(cwevent_removed_batter)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-8s",
		 ((gameiter->removed_for_ph) ? 
		  gameiter->removed_for_ph : ""));
}

/* Field 87 */
DECLARE_FIELDFUNC(cwevent_removed_batter_position)
{
  return sprintf(buffer, (ascii) ? "%d" : "%2d", 
		 ((gameiter->removed_for_ph) ? 
		  gameiter->removed_position : 0));
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
		 gameiter->event_count + 1);
}

static field_func function_ptrs[] = {
  cwevent_game_id,                /* 0 */
  cwevent_visiting_team,          /* 1 */
  cwevent_inning,                 /* 2 */
  cwevent_half_inning,            /* 3 */
  cwevent_outs,                   /* 4 */
  cwevent_balls,                  /* 5 */
  cwevent_strikes,                /* 6 */
  cwevent_pitches,                /* 7 */
  cwevent_visitor_score,          /* 8 */ 
  cwevent_home_score,             /* 9 */
  cwevent_batter,                 /* 10 */
  cwevent_batter_hand,            /* 11 */
  cwevent_res_batter,             /* 12 */
  cwevent_res_batter_hand,        /* 13 */
  cwevent_pitcher,                /* 14 */
  cwevent_pitcher_hand,           /* 15 */
  cwevent_res_pitcher,            /* 16 */
  cwevent_res_pitcher_hand,       /* 17 */
  cwevent_catcher,                /* 18 */
  cwevent_first_baseman,          /* 19 */
  cwevent_second_baseman,         /* 20 */
  cwevent_third_baseman,          /* 21 */
  cwevent_shortstop,              /* 22 */
  cwevent_left_fielder,           /* 23 */
  cwevent_center_fielder,         /* 24 */
  cwevent_right_fielder,          /* 25 */
  cwevent_runner_first,           /* 26 */
  cwevent_runner_second,          /* 27 */
  cwevent_runner_third,           /* 28 */
  cwevent_event_text,             /* 29 */
  cwevent_leadoff_flag,           /* 30 */
  cwevent_ph_flag,                /* 31 */
  cwevent_defensive_position,     /* 32 */
  cwevent_lineup_position,        /* 33 */
  cwevent_event_type,             /* 34 */
  cwevent_batter_event_flag,      /* 35 */
  cwevent_ab_flag,                /* 36 */
  cwevent_hit_value,              /* 37 */
  cwevent_sh_flag,                /* 38 */
  cwevent_sf_flag,                /* 39 */
  cwevent_outs_on_play,           /* 40 */
  cwevent_dp_flag,                /* 41 */
  cwevent_tp_flag,                /* 42 */
  cwevent_rbi_on_play,            /* 43 */
  cwevent_wp_flag,                /* 44 */
  cwevent_pb_flag,                /* 45 */
  cwevent_fielded_by,             /* 46 */
  cwevent_batted_ball_type,       /* 47 */
  cwevent_bunt_flag,              /* 48 */
  cwevent_foul_flag,              /* 49 */
  cwevent_hit_location,           /* 50 */
  cwevent_num_errors,             /* 51 */
  cwevent_error1_player,          /* 52 */
  cwevent_error1_type,            /* 53 */
  cwevent_error2_player,          /* 54 */
  cwevent_error2_type,            /* 55 */
  cwevent_error3_player,          /* 56 */
  cwevent_error3_type,            /* 57 */
  cwevent_batter_advance,         /* 58 */
  cwevent_runner1_advance,        /* 59 */
  cwevent_runner2_advance,        /* 60 */
  cwevent_runner3_advance,        /* 61 */
  cwevent_play_on_batter,         /* 62 */
  cwevent_play_on_runner1,        /* 63 */
  cwevent_play_on_runner2,        /* 64 */
  cwevent_play_on_runner3,        /* 65 */
  cwevent_sb2_flag,               /* 66 */
  cwevent_sb3_flag,               /* 67 */
  cwevent_sbh_flag,               /* 68 */
  cwevent_cs2_flag,               /* 69 */
  cwevent_cs3_flag,               /* 70 */
  cwevent_csh_flag,               /* 71 */
  cwevent_po1_flag,               /* 72 */
  cwevent_po2_flag,               /* 73 */
  cwevent_po3_flag,               /* 74 */
  cwevent_responsible_pitcher1,   /* 75 */
  cwevent_responsible_pitcher2,   /* 76 */
  cwevent_responsible_pitcher3,   /* 77 */
  cwevent_new_game_flag,          /* 78 */
  cwevent_end_game_flag,          /* 79 */
  cwevent_pr1_flag,               /* 80 */
  cwevent_pr2_flag,               /* 81 */
  cwevent_pr3_flag,               /* 82 */
  cwevent_removed_runner1,        /* 83 */
  cwevent_removed_runner2,        /* 84 */
  cwevent_removed_runner3,        /* 85 */
  cwevent_removed_batter,         /* 86 */
  cwevent_removed_batter_position, /* 87 */
  cwevent_putout1,                /* 88 */
  cwevent_putout2,                /* 89 */
  cwevent_putout3,                /* 90 */
  cwevent_assist1,                /* 91 */
  cwevent_assist2,                /* 92 */
  cwevent_assist3,                /* 93 */
  cwevent_assist4,                /* 94 */
  cwevent_assist5,                /* 95 */
  cwevent_event_number            /* 96 */
};

void
cwevent_process_game(CWGame *game, CWRoster *visitors, CWRoster *home) 
{
  char *buf;
  char output_line[1024];
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
    for (i = 0; i < 97; i++) {
      if (fields[i]) {
	if (ascii && comma) {
	  *(buf++) = ',';
	}
	else {
	  comma = 1;
	}
	buf += (*function_ptrs[i])(buf, gameiter, visitors, home);
      }
    }
    printf(output_line);
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
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwevent_print_help;

void
cwevent_print_field_list(void)
{
  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  The default fields are marked with an asterisk (*).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  fprintf(stderr, "0       game id*\n");
  fprintf(stderr, "1       visiting team*\n");
  fprintf(stderr, "2       inning*\n");
  fprintf(stderr, "3       batting team*\n");
  fprintf(stderr, "4       outs*\n");
  fprintf(stderr, "5       balls*\n");
  fprintf(stderr, "6       strikes*\n");
  fprintf(stderr, "7       pitch sequence\n");
  fprintf(stderr, "8       vis score*\n");
  fprintf(stderr, "9       home score*\n");
  fprintf(stderr, "10      batter\n");
  fprintf(stderr, "11      batter hand\n");
  fprintf(stderr, "12      res batter*\n");
  fprintf(stderr, "13      res batter hand*\n");
  fprintf(stderr, "14      pitcher\n");
  fprintf(stderr, "15      pitcher hand\n");
  fprintf(stderr, "16      res pitcher*\n");
  fprintf(stderr, "17      res pitcher hand*\n");
  fprintf(stderr, "18      catcher\n");
  fprintf(stderr, "19      first base\n");
  fprintf(stderr, "20      second base\n");
  fprintf(stderr, "21      third base\n");
  fprintf(stderr, "22      shortstop\n");
  fprintf(stderr, "23      left field\n");
  fprintf(stderr, "24      center field\n");
  fprintf(stderr, "25      right field\n");
  fprintf(stderr, "26      first runner*\n");
  fprintf(stderr, "27      second runner*\n");
  fprintf(stderr, "28      third runner*\n");
  fprintf(stderr, "29      event text*\n");
  fprintf(stderr, "30      leadoff flag*\n");
  fprintf(stderr, "31      pinchhit flag*\n");
  fprintf(stderr, "32      defensive position*\n");
  fprintf(stderr, "33      lineup position*\n");
  fprintf(stderr, "34      event type*\n");
  fprintf(stderr, "35      batter event flag*\n");
  fprintf(stderr, "36      ab flag*\n");
  fprintf(stderr, "37      hit value*\n");
  fprintf(stderr, "38      SH flag*\n");
  fprintf(stderr, "39      SF flag*\n");
  fprintf(stderr, "40      outs on play*\n");
  fprintf(stderr, "41      double play flag\n");
  fprintf(stderr, "42      triple play flag\n");
  fprintf(stderr, "43      RBI on play*\n");
  fprintf(stderr, "44      wild pitch flag*\n");
  fprintf(stderr, "45      passed ball flag*\n");
  fprintf(stderr, "46      fielded by\n");
  fprintf(stderr, "47      batted ball type\n");
  fprintf(stderr, "48      bunt flag\n");
  fprintf(stderr, "49      foul flag\n");
  fprintf(stderr, "50      hit location\n");
  fprintf(stderr, "51      num errors*\n");
  fprintf(stderr, "52      1st error player\n");
  fprintf(stderr, "53      1st error type\n");
  fprintf(stderr, "54      2nd error player\n");
  fprintf(stderr, "55      2nd error type\n");
  fprintf(stderr, "56      3rd error player\n");
  fprintf(stderr, "57      3rd error type\n");
  fprintf(stderr, "58      batter dest* (5 if scores and unearned, 6 if team unearned)\n");
  fprintf(stderr, "59      runner on 1st dest* (5 if scores and unearned, 6 if team unearned)\n");
  fprintf(stderr, "60      runner on 2nd dest* (5 if scores and unearned, 6 if team unearned)\n");
  fprintf(stderr, "61      runner on 3rd dest* (5 if scores and unearned, 6 if team unearned)\n");
  fprintf(stderr, "62      play on batter\n");
  fprintf(stderr, "63      play on runner on 1st\n");
  fprintf(stderr, "64      play on runner on 2nd\n");
  fprintf(stderr, "65      play on runner on 3rd\n");
  fprintf(stderr, "66      SB for runner on 1st flag\n");
  fprintf(stderr, "67      SB for runner on 2nd flag\n");
  fprintf(stderr, "68      SB for runner on 3rd flag\n");
  fprintf(stderr, "69      CS for runner on 1st flag\n");
  fprintf(stderr, "70      CS for runner on 2nd flag\n");
  fprintf(stderr, "71      CS for runner on 3rd flag\n");
  fprintf(stderr, "72      PO for runner on 1st flag\n");
  fprintf(stderr, "73      PO for runner on 2nd flag\n");
  fprintf(stderr, "74      PO for runner on 3rd flag\n");
  fprintf(stderr, "75      Responsible pitcher for runner on 1st\n");
  fprintf(stderr, "76      Responsible pitcher for runner on 2nd\n");
  fprintf(stderr, "77      Responsible pitcher for runner on 3rd\n");
  fprintf(stderr, "78      New Game Flag\n");
  fprintf(stderr, "79      End Game Flag\n");
  fprintf(stderr, "80      Pinch-runner on 1st\n");
  fprintf(stderr, "81      Pinch-runner on 2nd\n");
  fprintf(stderr, "82      Pinch-runner on 3rd\n");
  fprintf(stderr, "83      Runner removed for pinch-runner on 1st\n");
  fprintf(stderr, "84      Runner removed for pinch-runner on 2nd\n");
  fprintf(stderr, "85      Runner removed for pinch-runner on 3rd\n");
  fprintf(stderr, "86      Batter removed for pinch-hitter\n");
  fprintf(stderr, "87      Position of batter removed for pinch-hitter\n");
  fprintf(stderr, "88      Fielder with First Putout (0 if none)\n");
  fprintf(stderr, "89      Fielder with Second Putout (0 if none)\n");
  fprintf(stderr, "90      Fielder with Third Putout (0 if none)\n");
  fprintf(stderr, "91      Fielder with First Assist (0 if none)\n");
  fprintf(stderr, "92      Fielder with Second Assist (0 if none)\n");
  fprintf(stderr, "93      Fielder with Third Assist (0 if none)\n");
  fprintf(stderr, "94      Fielder with Fourth Assist (0 if none)\n");
  fprintf(stderr, "95      Fielder with Fifth Assist (0 if none)\n");
  fprintf(stderr, "96      event num\n");
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwevent_print_field_list;

void
cwevent_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick expanded event descriptor, version " VERSION); 
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "This is free software, " 
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwevent_print_welcome_message;

void
cwevent_initialize(void)
{
}

void (*cwtools_initialize)(void) = cwevent_initialize;

void
cwevent_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwevent_cleanup;
