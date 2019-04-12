/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwtools/cwdaily.c
 * Chadwick player game-by-game generator
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
int fields[154] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1
};

int max_field = 153;

char program_name[20] = "cwdaily";

int print_header = 0;


/* Auxiliary function: negative numbers in the boxscore structure
 * correspond to nulls, which should be rendered as blanks in output.
 */
int cwdaily_print_integer_or_null(char *buffer, int value)
{
  if (value >= 0) {
    return sprintf(buffer, "%d", value);
  }
  else {
    return sprintf(buffer, "%s", "");
  }
}

/*
 * typedef to declare the pointer-to-function type
 */
typedef int (*field_func)(char *, CWGameIterator *, CWBoxscore *,
			  int, int, int,
			  CWBoxPlayer *, CWRoster *, CWRoster *);

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
             CWBoxscore *box, \
             int team, int slot, int seq, CWBoxPlayer *player, \
             CWRoster *visitors, CWRoster *home)

/* Field 0 */
DECLARE_FIELDFUNC(cwdaily_game_id)
{
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-12s",
		 gameiter->game->game_id);
}

/* Field 1 */
DECLARE_FIELDFUNC(cwdaily_date)
{
  char *date = cw_game_info_lookup(gameiter->game, "date");
  return sprintf(buffer, (ascii) ? "\"%c%c%c%c%c%c%c%c\"" : "%c%c%c%c%c%c%c%c",
		 date[0], date[1], date[2], date[3],
		 date[5], date[6], date[8], date[9]);
}

/* Field 2 */
DECLARE_FIELDFUNC(cwdaily_number)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "%d" : "%5d",
		 (tmp = cw_game_info_lookup(gameiter->game, "number")) ?
		 cw_atoi(tmp) : 0);
}

/* Field 3 */
DECLARE_FIELDFUNC(cwdaily_app_date)
{
  /* The appearance date differs from the game date in the case of
   * games which are suspended and then resumed.
   */
  return sprintf(buffer, "\"%s\"", player->date);
}  

DECLARE_FIELDFUNC(cwdaily_team_id)
{
  char *tmp;
  if (team == 0) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   (tmp = cw_game_info_lookup(gameiter->game, "visteam")) ?
		   tmp : "");
  }
  else {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   (tmp = cw_game_info_lookup(gameiter->game, "hometeam")) ?
		   tmp : "");
  }
}

DECLARE_FIELDFUNC(cwdaily_player_id)
{
  return sprintf(buffer, "\"%s\"", player->player_id);
}

DECLARE_FIELDFUNC(cwdaily_player_slot)
{
  return sprintf(buffer, "%d", (slot < 10) ? slot : 0);
}

DECLARE_FIELDFUNC(cwdaily_player_seq)
{
  return sprintf(buffer, "%d", seq);
}

DECLARE_FIELDFUNC(cwdaily_home_fl)
{
  return sprintf(buffer, "%d", team==1);
}

DECLARE_FIELDFUNC(cwdaily_opponent_id)
{
  char *tmp;
  if (team == 1) {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   (tmp = cw_game_info_lookup(gameiter->game, "visteam")) ?
		   tmp : "");
  }
  else {
    return sprintf(buffer, (ascii) ? "\"%s\"" : "%-3s",
		   (tmp = cw_game_info_lookup(gameiter->game, "hometeam")) ?
		   tmp : "");
  }
}

DECLARE_FIELDFUNC(cwdaily_site)
{
  char *tmp;
  return sprintf(buffer, (ascii) ? "\"%s\"" : "%-5s",
		 (tmp = cw_game_info_lookup(gameiter->game, "site")) ?
		 tmp : "");
}


#define DECLARE_BATTING_CATEGORY(funcname, cat) \
DECLARE_FIELDFUNC(funcname) \
{ \
  return cwdaily_print_integer_or_null(buffer, player->batting->cat); \
}

DECLARE_BATTING_CATEGORY(cwdaily_B_G, g)
DECLARE_BATTING_CATEGORY(cwdaily_B_PA, pa)
DECLARE_BATTING_CATEGORY(cwdaily_B_AB, ab)
DECLARE_BATTING_CATEGORY(cwdaily_B_R, r)
DECLARE_BATTING_CATEGORY(cwdaily_B_H, h)

DECLARE_FIELDFUNC(cwdaily_B_TB)
{
  return cwdaily_print_integer_or_null(buffer,
				       player->batting->h +
				       player->batting->b2 +
				       2*player->batting->b3 +
				       3*player->batting->hr);
}
	       

DECLARE_BATTING_CATEGORY(cwdaily_B_2B, b2)
DECLARE_BATTING_CATEGORY(cwdaily_B_3B, b3)
DECLARE_BATTING_CATEGORY(cwdaily_B_HR, hr)
DECLARE_BATTING_CATEGORY(cwdaily_B_HR4, hrslam)
DECLARE_BATTING_CATEGORY(cwdaily_B_RBI, bi)
DECLARE_BATTING_CATEGORY(cwdaily_B_GW, gw)
DECLARE_BATTING_CATEGORY(cwdaily_B_BB, bb)
DECLARE_BATTING_CATEGORY(cwdaily_B_IBB, ibb)
DECLARE_BATTING_CATEGORY(cwdaily_B_SO, so)
DECLARE_BATTING_CATEGORY(cwdaily_B_GDP, gdp)
DECLARE_BATTING_CATEGORY(cwdaily_B_HP, hp)
DECLARE_BATTING_CATEGORY(cwdaily_B_SH, sh)
DECLARE_BATTING_CATEGORY(cwdaily_B_SF, sf)
DECLARE_BATTING_CATEGORY(cwdaily_B_SB, sb)
DECLARE_BATTING_CATEGORY(cwdaily_B_CS, cs)
DECLARE_BATTING_CATEGORY(cwdaily_B_XI, xi)

DECLARE_FIELDFUNC(cwdaily_B_G_DH)
{
  int i;
  
  for (i = 0; i < player->num_positions; i++) {
    if (player->positions[i] == 10) {
      return sprintf(buffer, "1");
    }
  }
  return sprintf(buffer, "0");
}

DECLARE_FIELDFUNC(cwdaily_B_G_PH)
{
  if (player->ph_inn > 0) {
    return sprintf(buffer, "1");
  }
  return sprintf(buffer, "0");
}

DECLARE_FIELDFUNC(cwdaily_B_G_PR)
{
  if (player->pr_inn > 0) {
    return sprintf(buffer, "1");
  }
  return sprintf(buffer, "0");
}

DECLARE_FIELDFUNC(cwdaily_P_G)
{
  return sprintf(buffer, "%d",
		 (player->fielding[1] != NULL) ? 1 : 0);
}

#define DECLARE_PITCHING_CATEGORY(funcname, cat) \
DECLARE_FIELDFUNC(funcname) \
{ \
  int stat = 0; \
  CWBoxPitcher *pitcher = box->pitchers[team]; \
  if (player->fielding[1] != NULL) { \
    while (pitcher != NULL) { \
      if (!strcmp(pitcher->player_id, player->player_id)) { \
	if (pitcher->pitching->cat < 0) { \
	  stat = -1; \
	  break; \
	} \
	else { \
	  stat += pitcher->pitching->cat; \
	} \
      } \
      pitcher = pitcher->prev; \
    } \
  } \
  return cwdaily_print_integer_or_null(buffer, stat); \
}

DECLARE_PITCHING_CATEGORY(cwdaily_P_GS, gs)
DECLARE_PITCHING_CATEGORY(cwdaily_P_CG, cg)
DECLARE_PITCHING_CATEGORY(cwdaily_P_SHO, sho)
DECLARE_PITCHING_CATEGORY(cwdaily_P_GF, gf)
DECLARE_PITCHING_CATEGORY(cwdaily_P_W, w)
DECLARE_PITCHING_CATEGORY(cwdaily_P_L, l)
DECLARE_PITCHING_CATEGORY(cwdaily_P_SV, sv)
DECLARE_PITCHING_CATEGORY(cwdaily_P_OUT, outs)
DECLARE_PITCHING_CATEGORY(cwdaily_P_TBF, bf)
DECLARE_PITCHING_CATEGORY(cwdaily_P_AB, ab)
DECLARE_PITCHING_CATEGORY(cwdaily_P_R, r)
DECLARE_PITCHING_CATEGORY(cwdaily_P_ER, er)
DECLARE_PITCHING_CATEGORY(cwdaily_P_H, h)

DECLARE_FIELDFUNC(cwdaily_P_TB)
{ 
  int stat = 0;
  CWBoxPitcher *pitcher = box->pitchers[team]; 
  if (player->fielding[1] != NULL) { 
    while (pitcher != NULL) {
      if (!strcmp(pitcher->player_id, player->player_id)) {
	if ((pitcher->pitching->h < 0) ||
	    (pitcher->pitching->b2 < 0) ||
	    (pitcher->pitching->b3 < 0) ||
	    (pitcher->pitching->hr < 0)) { 
	  stat = -1; 
	  break; 
	} 
	else { 
	  stat += pitcher->pitching->h + pitcher->pitching->b2 +
    	          2*pitcher->pitching->b3 + 3*pitcher->pitching->hr;; 
	} 
      } 
      pitcher = pitcher->prev; 
    } 
  } 
  return cwdaily_print_integer_or_null(buffer, stat); 
}


DECLARE_PITCHING_CATEGORY(cwdaily_P_2B, b2)
DECLARE_PITCHING_CATEGORY(cwdaily_P_3B, b3)
DECLARE_PITCHING_CATEGORY(cwdaily_P_HR, hr)
DECLARE_PITCHING_CATEGORY(cwdaily_P_HR4, hrslam)
DECLARE_PITCHING_CATEGORY(cwdaily_P_BB, bb)
DECLARE_PITCHING_CATEGORY(cwdaily_P_IBB, ibb)
DECLARE_PITCHING_CATEGORY(cwdaily_P_SO, so)
DECLARE_PITCHING_CATEGORY(cwdaily_P_GDP, gdp)
DECLARE_PITCHING_CATEGORY(cwdaily_P_HP, hb)
DECLARE_PITCHING_CATEGORY(cwdaily_P_SH, sh)
DECLARE_PITCHING_CATEGORY(cwdaily_P_SF, sf)
DECLARE_PITCHING_CATEGORY(cwdaily_P_XI, xi)
DECLARE_PITCHING_CATEGORY(cwdaily_P_WP, wp)
DECLARE_PITCHING_CATEGORY(cwdaily_P_BK, bk)
DECLARE_PITCHING_CATEGORY(cwdaily_P_IR, inr)
DECLARE_PITCHING_CATEGORY(cwdaily_P_IRS, inrs)
DECLARE_PITCHING_CATEGORY(cwdaily_P_GO, gb)
DECLARE_PITCHING_CATEGORY(cwdaily_P_AO, fb)

DECLARE_FIELDFUNC(cwdaily_P_PITCH)
{
  int stat = 0;
  char *pitches = cw_game_info_lookup(gameiter->game, "pitches");

  if (pitches && !strcmp(pitches, "pitches")) {
    CWBoxPitcher *pitcher = box->pitchers[team];
    if (player->fielding[1] != NULL) {
      while (pitcher != NULL) {
	if (!strcmp(pitcher->player_id, player->player_id)) {
	  if (pitcher->pitching->pitches < 0) {
	    stat = -1;
	    break;
	  }
	  else {
	    stat += pitcher->pitching->pitches;
	  }
	}
	pitcher = pitcher->prev;
      }
    }
  }
  else {
    stat = -1;
  }
  return cwdaily_print_integer_or_null(buffer, stat);
}

DECLARE_FIELDFUNC(cwdaily_P_STRIKE)
{
  int stat = 0;
  char *pitches = cw_game_info_lookup(gameiter->game, "pitches");
  
  if (pitches && !strcmp(pitches, "pitches")) {
    CWBoxPitcher *pitcher = box->pitchers[team];
    if (player->fielding[1] != NULL) {
      while (pitcher != NULL) {
	if (!strcmp(pitcher->player_id, player->player_id)) {
	  if (pitcher->pitching->strikes < 0) {
	    stat = -1;
	    break;
	  }
	  else {
	    stat += pitcher->pitching->strikes;
	  }
	}
	pitcher = pitcher->prev;
      }
    }
  }
  else {
    stat = -1;
  }
  return cwdaily_print_integer_or_null(buffer, stat);
}


#define DECLARE_FIELDING_CATEGORY(funcname, pos, cat) \
DECLARE_FIELDFUNC(funcname) \
{ \
  if (player->fielding[pos] != NULL) { \
    return cwdaily_print_integer_or_null(buffer, player->fielding[pos]->cat); \
  } \
  else { \
    return sprintf(buffer, "0"); \
  } \
}

#define DECLARE_FIELDING_STARTER(funcname, pos) \
DECLARE_FIELDFUNC(funcname) \
{ \
  return sprintf(buffer, "%d", (player->start_position==pos) ? 1 : 0); \
}

#define DECLARE_FIELDING_TC(funcname, pos) \
DECLARE_FIELDFUNC(funcname) \
{ \
  if (player->fielding[pos] != NULL) { \
    if ((player->fielding[pos]->po < 0) || \
	(player->fielding[pos]->a < 0) || \
	(player->fielding[pos]->e < 0)) { \
      return cwdaily_print_integer_or_null(buffer, -1); \
    } \
    else { \
      return cwdaily_print_integer_or_null(buffer, \
					   player->fielding[pos]->po + \
					   player->fielding[pos]->a + \
					   player->fielding[pos]->e); \
    } \
  } \
  else { \
    return sprintf(buffer, "0"); \
  } \
}

DECLARE_FIELDING_CATEGORY(cwdaily_F_P_OUT, 1, outs)
DECLARE_FIELDING_TC(cwdaily_F_P_TC, 1)
DECLARE_FIELDING_CATEGORY(cwdaily_F_P_PO, 1, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_P_A, 1, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_P_E, 1, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_P_DP, 1, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_P_TP, 1, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_C_G, 2, g)
DECLARE_FIELDING_STARTER(cwdaily_F_C_GS, 2)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_OUT, 2, outs)
DECLARE_FIELDING_TC(cwdaily_F_C_TC, 2)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_PO, 2, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_A, 2, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_E, 2, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_DP, 2, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_TP, 2, tp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_PB, 2, pb)
DECLARE_FIELDING_CATEGORY(cwdaily_F_C_XI, 2, xi)

DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_G, 3, g)
DECLARE_FIELDING_STARTER(cwdaily_F_1B_GS, 3)
DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_OUT, 3, outs)
DECLARE_FIELDING_TC(cwdaily_F_1B_TC, 3)
DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_PO, 3, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_A, 3, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_E, 3, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_DP, 3, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_1B_TP, 3, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_G, 4, g)
DECLARE_FIELDING_STARTER(cwdaily_F_2B_GS, 4)
DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_OUT, 4, outs)
DECLARE_FIELDING_TC(cwdaily_F_2B_TC, 4)
DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_PO, 4, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_A, 4, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_E, 4, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_DP, 4, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_2B_TP, 4, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_G, 5, g)
DECLARE_FIELDING_STARTER(cwdaily_F_3B_GS, 5)
DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_OUT, 5, outs)
DECLARE_FIELDING_TC(cwdaily_F_3B_TC, 5)
DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_PO, 5, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_A, 5, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_E, 5, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_DP, 5, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_3B_TP, 5, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_G, 6, g)
DECLARE_FIELDING_STARTER(cwdaily_F_SS_GS, 6)
DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_OUT, 6, outs)
DECLARE_FIELDING_TC(cwdaily_F_SS_TC, 6)
DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_PO, 6, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_A, 6, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_E, 6, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_DP, 6, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_SS_TP, 6, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_G, 7, g)
DECLARE_FIELDING_STARTER(cwdaily_F_LF_GS, 7)
DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_OUT, 7, outs)
DECLARE_FIELDING_TC(cwdaily_F_LF_TC, 7)
DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_PO, 7, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_A, 7, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_E, 7, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_DP, 7, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_LF_TP, 7, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_G, 8, g)
DECLARE_FIELDING_STARTER(cwdaily_F_CF_GS, 8)
DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_OUT, 8, outs)
DECLARE_FIELDING_TC(cwdaily_F_CF_TC, 8)
DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_PO, 8, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_A, 8, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_E, 8, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_DP, 8, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_CF_TP, 8, tp)

DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_G, 9, g)
DECLARE_FIELDING_STARTER(cwdaily_F_RF_GS, 9)
DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_OUT, 9, outs)
DECLARE_FIELDING_TC(cwdaily_F_RF_TC, 9)
DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_PO, 9, po)
DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_A, 9, a)
DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_E, 9, e)
DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_DP, 9, dp)
DECLARE_FIELDING_CATEGORY(cwdaily_F_RF_TP, 9, tp)

static field_struct field_data[] = {
  /*  0 */ { cwdaily_game_id, "GAME_ID", "game id" },
  /*  1 */ { cwdaily_date, "GAME_DT", "date" },
  /*  2 */ { cwdaily_number, "GAME_CT", "game number (0 = no double header)" },
  /*  3 */ { cwdaily_app_date, "APPEAR_DT", "apperance date" },
  { cwdaily_team_id, "TEAM_ID", "team id" },
  { cwdaily_player_id, "PLAYER_ID", "player id" },
  { cwdaily_player_slot, "SLOT_CT", "player slot in batting order" },
  { cwdaily_player_seq, "SEQ_CT", "sequence in batting order slot" },
  { cwdaily_home_fl, "HOME_FL", "home flag" },
  { cwdaily_opponent_id, "OPPONENT_ID", "opponent id" },
  { cwdaily_site, "PARK_ID", "park id" },
  { cwdaily_B_G, "B_G", "B_G:   games played" },
  { cwdaily_B_PA, "B_PA", "B_PA:  plate appearances" },
  { cwdaily_B_AB, "B_AB", "B_AB:  at bats" },
  { cwdaily_B_R, "B_R", "B_R:   runs" },
  { cwdaily_B_H, "B_H", "B_H:   hits" },
  { cwdaily_B_TB, "B_TB", "B_TB:  total bases" },
  { cwdaily_B_2B, "B_2B", "B_2B:  doubles" },
  { cwdaily_B_3B, "B_3B", "B_3B:  triples" },
  { cwdaily_B_HR, "B_HR", "B_HR:  home runs" },
  { cwdaily_B_HR4, "B_HR4", "B_HR4: grand slams" },
  { cwdaily_B_RBI, "B_RBI", "B_RBI: runs batted in" },
  { cwdaily_B_GW, "B_GW", "B_GW:  game winning RBI" },
  { cwdaily_B_BB, "B_BB", "B_BB:  walks" },
  { cwdaily_B_IBB, "B_IBB", "B_IBB: intentional walks" },
  { cwdaily_B_SO, "B_SO", "B_SO:  strikeouts" },
  { cwdaily_B_GDP, "B_GDP", "B_GDP: grounded into DP" },
  { cwdaily_B_HP, "B_HP", "B_HP:  hit by pitch" },
  { cwdaily_B_SH, "B_SH", "B_SH:  sacrifice hits" },
  { cwdaily_B_SF, "B_SF", "B_SF:  sacrifice flies" },
  { cwdaily_B_SB, "B_SB", "B_SB:  stolen bases" },
  { cwdaily_B_CS, "B_CS", "B_CS:  caught stealing" },
  { cwdaily_B_XI, "B_XI", "B_XI:  reached on interference" },
  { cwdaily_B_G_DH, "B_G_DH", "B_G_DH: games as DH" },
  { cwdaily_B_G_PH, "B_G_PH", "B_G_PH: games as PH" },
  { cwdaily_B_G_PR, "B_G_PR", "B_G_PR: games as PR" },
  { cwdaily_P_G, "P_G", "P_G:   games pitched" },
  { cwdaily_P_GS, "P_GS", "P_GS:  games started" },
  { cwdaily_P_CG, "P_CG", "P_CG:  complete games" },
  { cwdaily_P_SHO, "P_SHO", "P_SHO: shutouts" },
  { cwdaily_P_GF, "P_GF", "P_GF:  games finished" },
  { cwdaily_P_W, "P_W", "P_W:  wins" },
  { cwdaily_P_L, "P_L", "P_L:  losses" },
  { cwdaily_P_SV, "P_SV", "P_SV:  saves" },
  { cwdaily_P_OUT, "P_OUT", "P_OUT: outs recorded (innings pitched times 3)" },
  { cwdaily_P_TBF, "P_TBF", "P_TBF: batters faced" },
  { cwdaily_P_AB, "P_AB", "P_AB:  at bats" },
  { cwdaily_P_R, "P_R", "P_R:   runs allowed" },
  { cwdaily_P_ER, "P_ER", "P_ER:  earned runs allowed" },
  { cwdaily_P_H, "P_H", "P_H:   hits allowed" },
  { cwdaily_P_TB, "P_TB", "P_TB:  total bases allowed" },
  { cwdaily_P_2B, "P_2B", "P_2B:  doubles allowed" },
  { cwdaily_P_3B, "P_3B", "P_3B:  triples allowed" },
  { cwdaily_P_HR, "P_HR", "P_HR:  home runs allowed" },
  { cwdaily_P_HR4, "P_HR4", "P_HR4:  grand slams allowed" },
  { cwdaily_P_BB, "P_BB", "P_BB:  walks allowed" },
  { cwdaily_P_IBB, "P_IBB", "P_IBB: intentional walks allowed" },
  { cwdaily_P_SO, "P_SO", "P_SO:  strikeouts" },
  { cwdaily_P_GDP, "P_GDP", "P_GDP: grounded into double play" },
  { cwdaily_P_HP, "P_HP", "P_HP:  hit batsmen" },
  { cwdaily_P_SH, "P_SH", "P_SH:  sacrifice hits against" },
  { cwdaily_P_SF, "P_SF", "P_SF:  sacrifice flies against" },
  { cwdaily_P_XI, "P_XI", "P_XI:  reached on interference" },
  { cwdaily_P_WP, "P_WP", "P_WP:  wild pitches" },
  { cwdaily_P_BK, "P_BK", "P_BK:  balks" },
  { cwdaily_P_IR, "P_IR", "P_IR:  inherited runners" },
  { cwdaily_P_IRS, "P_IRS", "P_IRS: inherited runners scored" },
  { cwdaily_P_GO, "P_GO", "P_GO:  ground outs" },
  { cwdaily_P_AO, "P_AO", "P_AO:  air outs" },
  { cwdaily_P_PITCH, "P_PITCH", "P_PITCH:  pitches" },
  { cwdaily_P_STRIKE, "P_STRIKE", "P_STRIKE: strikes" },
  { cwdaily_P_G, "F_P_G", "F_P_G:    games at P" },
  { cwdaily_P_GS, "F_P_GS", "F_P_GS:   games started at P" },
  { cwdaily_F_P_OUT, "F_P_OUT", "F_P_OUT:  outs recorded at P (innings fielded times 3)" },
  { cwdaily_F_P_TC, "F_P_TC", "F_P_TC:   total chances at P" },
  { cwdaily_F_P_PO, "F_P_PO", "F_P_PO:   putouts at P" },
  { cwdaily_F_P_A, "F_P_A", "F_P_A:    assists at P" },
  { cwdaily_F_P_E, "F_P_E", "F_P_E:    errors at P" },
  { cwdaily_F_P_DP, "F_P_DP", "F_P_DP:   double plays at P" },
  { cwdaily_F_P_TP, "F_P_TP", "F_P_TP:   triple plays at P" },
  { cwdaily_F_C_G, "F_C_G", "F_C_G:    games at C" },
  { cwdaily_F_C_GS, "F_C_GS", "F_C_GS:   games started at C" },
  { cwdaily_F_C_OUT, "F_C_OUT", "F_C_OUT:  outs recorded at C (innings fielded times 3)" },
  { cwdaily_F_C_TC, "F_C_TC", "F_C_TC:   total chances at C" },
  { cwdaily_F_C_PO, "F_C_PO", "F_C_PO:   putouts at C" },
  { cwdaily_F_C_A, "F_C_A", "F_C_A:    assists at C" },
  { cwdaily_F_C_E, "F_C_E", "F_C_E:    errors at C" },
  { cwdaily_F_C_DP, "F_C_DP", "F_C_DP:   double plays at C" },
  { cwdaily_F_C_TP, "F_C_TP", "F_C_TP:   triple plays at C" },
  { cwdaily_F_C_PB, "F_C_PB", "F_C_PB:   passed balls at C" },
  { cwdaily_F_C_XI, "F_C_XI", "F_C_IX:   catcher's interference at C" },
  { cwdaily_F_1B_G, "F_1B_G", "F_1B_G:   games at 1B" },
  { cwdaily_F_1B_GS, "F_1B_GS", "F_1B_GS:  games started at 1B" },
  { cwdaily_F_1B_OUT, "F_1B_OUT", "F_1B_OUT: outs recorded at 1B (innings fielded times 3)" },
  { cwdaily_F_1B_TC, "F_1B_TC", "F_1B_TC:  total chances at 1B" },
  { cwdaily_F_1B_PO, "F_1B_PO", "F_1B_PO:  putouts at 1B" },
  { cwdaily_F_1B_A, "F_1B_A", "F_1B_A:   assists at 1B" },
  { cwdaily_F_1B_E, "F_1B_E", "F_1B_E:   errors at 1B" },
  { cwdaily_F_1B_DP, "F_1B_DP", "F_1B_DP:  double plays at 1B" },
  { cwdaily_F_1B_TP, "F_1B_TP", "F_1B_TP:  triple plays at 1B" },
  { cwdaily_F_2B_G, "F_2B_G", "F_2B_G:   games at 2B" },
  { cwdaily_F_2B_GS, "F_2B_GS", "F_2B_GS:  games started at 2B" },
  { cwdaily_F_2B_OUT, "F_2B_OUT", "F_2B_OUT: outs recorded at 2B (innings fielded times 3)" },
  { cwdaily_F_2B_TC, "F_2B_TC", "F_2B_TC:  total chances at 2B" },
  { cwdaily_F_2B_PO, "F_2B_PO", "F_2B_PO:  putouts at 2B" },
  { cwdaily_F_2B_A, "F_2B_A", "F_2B_A:   assists at 2B" },
  { cwdaily_F_2B_E, "F_2B_E", "F_2B_E:   errors at 2B" },
  { cwdaily_F_2B_DP, "F_2B_DP", "F_2B_DP:  double plays at 2B" },
  { cwdaily_F_2B_TP, "F_2B_TP", "F_2B_TP:  triple plays at 2B" },
  { cwdaily_F_3B_G, "F_3B_G", "F_3B_G:   games at 3B" },
  { cwdaily_F_3B_GS, "F_3B_GS", "F_3B_GS:  games started at 3B" },
  { cwdaily_F_3B_OUT, "F_3B_OUT", "F_3B_OUT: outs recorded at 3B (innings fielded times 3)" },
  { cwdaily_F_3B_TC, "F_3B_TC", "F_3B_TC:  total chances at 3B" },
  { cwdaily_F_3B_PO, "F_3B_PO", "F_3B_PO:  putouts at 3B" },
  { cwdaily_F_3B_A, "F_3B_A", "F_3B_A:   assists at 3B" },
  { cwdaily_F_3B_E, "F_3B_E", "F_3B_E:   errors at 3B" },
  { cwdaily_F_3B_DP, "F_3B_DP", "F_3B_DP:  double plays at 3B" },
  { cwdaily_F_3B_TP, "F_3B_TP", "F_3B_TP:  triple plays at 3B" },
  { cwdaily_F_SS_G, "F_SS_G", "F_SS_G:    games at SS" },
  { cwdaily_F_SS_GS, "F_SS_GS", "F_SS_GS:  games started at SS" },
  { cwdaily_F_SS_OUT, "F_SS_OUT", "F_SS_OUT: outs recorded at SS (innings fielded times 3)" },
  { cwdaily_F_SS_TC, "F_SS_TC", "F_SS_TC:  total chances at SS" },
  { cwdaily_F_SS_PO, "F_SS_PO", "F_SS_PO:  putouts at SS" },
  { cwdaily_F_SS_A, "F_SS_A", "F_SS_A:   assists at SS" },
  { cwdaily_F_SS_E, "F_SS_E", "F_SS_E:   errors at SS" },
  { cwdaily_F_SS_DP, "F_SS_DP", "F_SS_DP:  double plays at SS" },
  { cwdaily_F_SS_TP, "F_SS_TP", "F_SS_TP:  triple plays at SS" },
  { cwdaily_F_LF_G, "F_LF_G", "F_LF_G:   games at LF" },
  { cwdaily_F_LF_GS, "F_LF_GS", "F_LF_GS:  games started at LF" },
  { cwdaily_F_LF_OUT, "F_LF_OUT", "F_LF_OUT: outs recorded at LF (innings fielded times 3)" },
  { cwdaily_F_LF_TC, "F_LF_TC", "F_LF_TC:  total chances at LF" },
  { cwdaily_F_LF_PO, "F_LF_PO", "F_LF_PO:  putouts at LF" },
  { cwdaily_F_LF_A, "F_LF_A", "F_LF_A:   assists at LF" },
  { cwdaily_F_LF_E, "F_LF_E", "F_LF_E:   errors at LF" },
  { cwdaily_F_LF_DP, "F_LF_DP", "F_LF_DP:  double plays at LF" },
  { cwdaily_F_LF_TP, "F_LF_TP", "F_LF_TP:  triple plays at LF" },
  { cwdaily_F_CF_G, "F_CF_G", "F_CF_G:   games at CF" },
  { cwdaily_F_CF_GS, "F_CF_GS", "F_CF_GS:  games started at CF" },
  { cwdaily_F_CF_OUT, "F_CF_OUT", "F_CF_OUT: outs recorded at CF (innings fielded times 3)" },
  { cwdaily_F_CF_TC, "F_CF_TC", "F_CF_TC:  total chances at CF" },
  { cwdaily_F_CF_PO, "F_CF_PO", "F_CF_PO:  putouts at CF" },
  { cwdaily_F_CF_A, "F_CF_A", "F_CF_A:   assists at CF" },
  { cwdaily_F_CF_E, "F_CF_E", "F_CF_E:   errors at CF" },
  { cwdaily_F_CF_DP, "F_CF_DP", "F_CF_DP:  double plays at CF" },
  { cwdaily_F_CF_TP, "F_CF_TP", "F_CF_TP:  triple plays at CF" },
  { cwdaily_F_RF_G, "F_RF_G", "F_RF_G:   games at RF" },
  { cwdaily_F_RF_GS, "F_RF_GS", "F_RF_GS:  games started at RF" },
  { cwdaily_F_RF_OUT, "F_RF_OUT", "F_RF_OUT: outs recorded at RF (innings fielded times 3)" },
  { cwdaily_F_RF_TC, "F_RF_TC", "F_RF_TC:  total chances at RF" },
  { cwdaily_F_RF_PO, "F_RF_PO", "F_RF_PO:  putouts at RF" },
  { cwdaily_F_RF_A, "F_RF_A", "F_RF_A:   assists at RF" },
  { cwdaily_F_RF_E, "F_RF_E", "F_RF_E:   errors at RF" },
  { cwdaily_F_RF_DP, "F_RF_DP", "F_RF_DP:  double plays at RF" },
  { cwdaily_F_RF_TP, "F_RF_TP", "F_RF_TP:  triple plays at RF" }
};


void cwdaily_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char *buf;
  char output_line[4096];
  int i, j, t, seq, comma = 0;
  CWGameIterator *gameiter = cw_gameiter_create(game);
  CWBoxscore *box = cw_box_create(game);
  CWBoxPlayer *player;
  
  while (gameiter->event != NULL) {
    cw_gameiter_next(gameiter);
  }

  for (t = 0; t <= 1; t++) {
    for (j = 1; j <= 10; j++) {
      /* We list non-batting pitchers last, but they are coded as slot 0 */
      player = cw_box_get_starter(box, t, j % 10);
      seq = 1;
      while (player != NULL) {
	strcpy(output_line, "");
	buf = output_line;
	comma = 0;
	for (i = 0; i <= max_field; i++) {
	  if (fields[i]) {
	    if (ascii && comma) {
	      *(buf++) = ',';
	    }
	    else {
	      comma = 1;
	    }
	    buf += (*field_data[i].f)(buf, gameiter, box,
				      t, j, seq, player,
				      visitors, home);
	  }
	}
	printf("%s\n", output_line);
	player = player->next;
	seq++;
      }
    }
  }

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwdaily_process_game;

void cwdaily_print_help(void)
{
  fprintf(stderr, "\n\ncwdaily generates files suitable for use by dBase or Lotus-like programs\n");
  fprintf(stderr, "Each record describes one game.\n");
  fprintf(stderr, "Usage: cwdaily [options] eventfile...\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "  -h        print this help\n");
  fprintf(stderr, "  -i id     only process game given by id\n");
  fprintf(stderr, "  -y year   Year to process (for teamyyyy and aaayyyy.ros).\n");
  fprintf(stderr, "  -s start  Earliest date to process (mmdd).\n");
  fprintf(stderr, "  -e end    Last date to process (mmdd).\n");
  fprintf(stderr, "  -a        generate Ascii-delimited format files (default)\n");
  fprintf(stderr, "  -ft       generate Fortran format files\n");
  fprintf(stderr, "  -f flist  give list of fields to output\n");
  fprintf(stderr, "              Default is 0-83\n");
  fprintf(stderr, "  -x flist  give list of extended fields to output\n");
  fprintf(stderr, "              Default is none\n");
  fprintf(stderr, "  -d        print list of field numbers and descriptions\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");
  fprintf(stderr, "  -n        print field names in first row of output\n\n");

  exit(0);
}

void (*cwtools_print_help)(void) = cwdaily_print_help;

void
cwdaily_print_field_list(void)
{
  int i;

  fprintf(stderr, "\nThese are the available fields and the numbers to use with the -f option\n");
  fprintf(stderr, "to name them.  All are included by default.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "number  field\n");
  fprintf(stderr, "------  -----\n");
  for (i = 0; i <= max_field; i++) {
    fprintf(stderr, "%-3d     %s\n", i, field_data[i].description);
  }
  fprintf(stderr, "\n");

  exit(0);
}

void (*cwtools_print_field_list)(void) = cwdaily_print_field_list;

void
cwdaily_print_welcome_message(char *argv0)
{
  fprintf(stderr, 
	  "\nChadwick player game-by-game generator, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "Copyright (c) 2002-2019\nDr T L Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)\n");
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwdaily_print_welcome_message;

void
cwdaily_initialize(void)
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

  printf("%s", output_line);
  printf("\n");
}

void (*cwtools_initialize)(void) = cwdaily_initialize;

void
cwdaily_cleanup(void)
{
}

void (*cwtools_cleanup)(void) = cwdaily_cleanup;

extern char year[5];
extern char first_date[5];
extern char last_date[5];
extern char game_id[20];
extern int ascii;
extern int quiet;

extern void
cwtools_parse_field_list(char *text, int max_field, int *fields);

int
cwdaily_parse_command_line(int argc, char *argv[])
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

int (*cwtools_parse_command_line)(int, char *argv[]) = cwdaily_parse_command_line;
