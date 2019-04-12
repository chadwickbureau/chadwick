/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/cwboxsml.c
 * Chadwick boxscore generator program, SportsML output
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
#include <time.h>

#include "cwlib/chadwick.h"
#include "xmlwrite.h"

/*
 * Creates a <player-metadata> element as a child of 'parent'
 *
 * TODO: Update position-event if the vocabulary changes, and/or to
 *       allow for multiple positions.
 *       Add lineup slot and sequence entries if added to standard
 */
static void
cwbox_player_metadata(XMLNode *parent, 
		      CWBoxPlayer *player, int slot, int seq, CWRoster *roster)
{
  XMLNode *node = NULL, *nameNode = NULL;
  CWPlayer *bio;

  node = xml_node_open(parent, "player-metadata");

  if (player->positions[0] == 10) {
    xml_node_attribute(node, "position-event", "dh");
  }
  else if (player->positions[0] == 11 || player->ph_inn > 0) {
    xml_node_attribute(node, "position-event", "ph");
  }
  else if (player->positions[0] == 12 || player->pr_inn > 0) {
    xml_node_attribute(node, "position-event", "pr");
  }
  else {
    xml_node_attribute_int(node, "position-event", player->positions[0]);
  }

  xml_node_attribute(node, "player-key", player->player_id);
  xml_node_attribute(node, "status", 
	     ((seq == 1) ? "starter" : "bench"));
  xml_node_attribute_int(node, "lineup-slot", slot);
  xml_node_attribute_int(node, "lineup-slot-sequence", seq);

  if (roster != NULL)  {
    bio = cw_roster_player_find(roster, player->player_id);

    if (bio != NULL) {
      nameNode = xml_node_open(node, "name");
      xml_node_attribute(nameNode, "first", bio->first_name);
      xml_node_attribute(nameNode, "last", bio->last_name);
    }
  }
}

/*
 * Creates a <stats-baseball-offensive> element for the player
 */
static void
cwbox_player_stats_offensive(XMLNode *parent, CWBoxPlayer *player)
{
  XMLNode *node = NULL;
  
  node = xml_node_open(parent, "stats-baseball-offensive");
  
  xml_node_attribute_posint(node, "plate-appearances", player->batting->pa);
  xml_node_attribute_posint(node, "at-bats", player->batting->ab);
  xml_node_attribute_posint(node, "runs-scored", player->batting->r);
  xml_node_attribute_posint(node, "hits", player->batting->h);
  xml_node_attribute_posint(node, "total-bases",
			    player->batting->h + player->batting->b2 +
			    2*player->batting->b3 + 3*player->batting->hr);
  xml_node_attribute_posint(node, "hits-extra-base",
			    player->batting->b2 + player->batting->b3 +
			    player->batting->hr);
  xml_node_attribute_posint(node, "singles",
			    player->batting->h - player->batting->b2 -
			    player->batting->b3 - player->batting->hr);
  xml_node_attribute_posint(node, "doubles", player->batting->b2);
  xml_node_attribute_posint(node, "triples", player->batting->b3);
  xml_node_attribute_posint(node, "home-runs", player->batting->hr);
  xml_node_attribute_posint(node, "grand-slams", player->batting->hrslam);
  xml_node_attribute_posint(node, "rbi", player->batting->bi);
  xml_node_attribute_posint(node, "bases-on-balls", player->batting->bb);
  xml_node_attribute_posint(node, "bases-on-balls-intentional", 
			    player->batting->ibb);
  xml_node_attribute_posint(node, "strikeouts", player->batting->so);
  xml_node_attribute_posint(node, "grounded-into-double-play",
			    player->batting->gdp);
  xml_node_attribute_posint(node, "hit-by-pitch", player->batting->hp);
  xml_node_attribute_posint(node, "sac-bunts", player->batting->sh);
  xml_node_attribute_posint(node, "sac-flies", player->batting->sf);
  xml_node_attribute_posint(node, "stolen-bases", player->batting->sb);
  xml_node_attribute_posint(node, "stolen-bases-caught", player->batting->cs);
  xml_node_attribute_posint(node, "reached-base-defensive-interference",
			    player->batting->xi);
  xml_node_attribute_posint(node, "left-in-scoring-position",
			    player->batting->lisp);
  xml_node_attribute_posint(node, "moved-up", player->batting->movedup);
}

/*
 * Creates a <stats-baseball-defensive> element for the player
 */
static void
cwbox_player_stats_defensive(XMLNode *parent, CWBoxPlayer *player)
{
  XMLNode *node = NULL;
  int pos, outs = 0, po = 0, a = 0, e = 0, dp = 0, tp = 0, pb = 0, xi = 0;

  node = xml_node_open(parent, "stats-baseball-defensive");

  for (pos = 1; pos < 10; pos++) {
    if (player->fielding[pos] == NULL) {
      continue;
    }

    outs += player->fielding[pos]->outs;
    po += player->fielding[pos]->po;
    a += player->fielding[pos]->a;
    e += player->fielding[pos]->e;
    dp += player->fielding[pos]->dp;
    tp += player->fielding[pos]->tp;

    if (pos == 2) {
      pb += player->fielding[pos]->pb;
      xi += player->fielding[pos]->xi;
    }
  }
 
  if (outs > 0) {
    xml_node_attribute_fmt(node, "innings-played", "%d.%d",
			   outs / 3, outs % 3);
  }
  xml_node_attribute_posint(node, "putouts", po);
  xml_node_attribute_posint(node, "assists", a);
  xml_node_attribute_posint(node, "errors", e);
  xml_node_attribute_posint(node, "double-plays", dp);
  xml_node_attribute_posint(node, "triple-plays", tp);
  xml_node_attribute_posint(node, "errors-passed-ball", pb);
  xml_node_attribute_posint(node, "errors-catchers-interference", xi);
}

/*
 * Creates a <stats-baseball-pitching> element for the player
 */
static void
cwbox_player_stats_pitching(XMLNode *parent, 
			    CWGame *game, CWBoxPitcher *pitcher)
{
  XMLNode *node = NULL;
  
  node = xml_node_open(parent, "stats-baseball-pitching");
  if (pitcher->pitching->outs % 3 == 0) {
    xml_node_attribute_int(node, "innings-pitched", pitcher->pitching->outs / 3);
  }
  else {
    xml_node_attribute_fmt(node, "innings-pitched", "%d.%d",
			   pitcher->pitching->outs / 3,
			   pitcher->pitching->outs % 3);

  }
  xml_node_attribute_posint(node, "batters-at-bats-against",
			    pitcher->pitching->ab);
  xml_node_attribute_posint(node, "batters-total-against",
			    pitcher->pitching->bf);
  xml_node_attribute_posint(node, "hits", pitcher->pitching->h);
  xml_node_attribute_posint(node, "runs-allowed", pitcher->pitching->r);
  xml_node_attribute_posint(node, "earned-runs", pitcher->pitching->er);
  xml_node_attribute_posint(node, "unearned-runs",
			    pitcher->pitching->r - pitcher->pitching->er);
  xml_node_attribute_posint(node, "home-runs-allowed", pitcher->pitching->hr);
  xml_node_attribute_posint(node, "singles-allowed",
			    pitcher->pitching->h - pitcher->pitching->b2 -
			    pitcher->pitching->b3 - pitcher->pitching->hr);
  xml_node_attribute_posint(node, "doubles-allowed", pitcher->pitching->b2);
  xml_node_attribute_posint(node, "triples-allowed", pitcher->pitching->b3);
  xml_node_attribute_posint(node, "sacrifice-bunts-allowed", 
			    pitcher->pitching->sh);
  xml_node_attribute_posint(node, "sacrifice-flies-allowed",
			    pitcher->pitching->sf);
  xml_node_attribute_posint(node, "bases-on-balls", pitcher->pitching->bb);
  xml_node_attribute_posint(node, "bases-on-balls-intentional",
			    pitcher->pitching->ibb);
  xml_node_attribute_posint(node, "strikeouts", pitcher->pitching->so);
  xml_node_attribute_posint(node, "errors-hit-with-pitch",
			    pitcher->pitching->hb);
  xml_node_attribute_posint(node, "errors-wild-pitch", pitcher->pitching->wp);
  xml_node_attribute_posint(node, "balks", pitcher->pitching->bk);
  xml_node_attribute_posint(node, "pick-offs", pitcher->pitching->pk);
  xml_node_attribute_posint(node, "inherited-runners-total",
			    pitcher->pitching->inr);
  xml_node_attribute_posint(node, "inherited-runners-scored",
			    pitcher->pitching->inrs);
  

  /* FIXME: A pitcher gets a shutout if he records all outs for a team,
   * even if not the starting pitcher! */
  xml_node_attribute_int(node, "shutouts",
			 ((pitcher->prev == NULL && pitcher->next == NULL &&
			   pitcher->pitching->r == 0) ? 1 : 0));
  xml_node_attribute_int(node, "games-complete",
			 (pitcher->prev == NULL && 
			  pitcher->next == NULL) ? 1 : 0);
  xml_node_attribute_int(node, "games-finished",
			 (pitcher->prev != NULL && 
			  pitcher->next == NULL) ? 1 : 0);

  if (cw_game_info_lookup(game, "pitches") &&
      !strcmp(cw_game_info_lookup(game, "pitches"), "pitches")) {
    xml_node_attribute_int(node, "number-of-pitches",
			   pitcher->pitching->pitches);
    xml_node_attribute_int(node, "number-of-strikes",
			   pitcher->pitching->strikes);
  }

  if (cw_game_info_lookup(game, "wp") &&
      !strcmp(pitcher->player_id, cw_game_info_lookup(game, "wp"))) {
    xml_node_attribute(node, "event-credit", "win");
  }
  else if (cw_game_info_lookup(game, "lp") &&
	   !strcmp(pitcher->player_id, cw_game_info_lookup(game, "lp"))) {
    xml_node_attribute(node, "event-credit", "loss");
  }
  else if (cw_game_info_lookup(game, "save") &&
	   !strcmp(pitcher->player_id, cw_game_info_lookup(game, "save"))) {
    xml_node_attribute(node, "event-credit", "save");
    xml_node_attribute(node, "save-credit", "save");
  }
}

/*
 * Creates a <player> element as a child of 'parent'
 */
static void
cwbox_player(XMLNode *parent, CWGame *game,
	     CWBoxPlayer *player, CWBoxPitcher *pitching,
	     int slot, int seq, CWRoster *roster)
{
  XMLNode *node = NULL, *statsNode = NULL, *statsBaseballNode = NULL;

  node = xml_node_open(parent, "player");
  xml_node_attribute_fmt(node, "id", "p.%s", player->player_id);

  cwbox_player_metadata(node, player, slot, seq, roster);

  statsNode = xml_node_open(node, "player-stats");
  statsBaseballNode = xml_node_open(statsNode, "player-stats-baseball");

  cwbox_player_stats_offensive(statsBaseballNode, player);
  cwbox_player_stats_defensive(statsBaseballNode, player);
  if (pitching) {
    cwbox_player_stats_pitching(statsBaseballNode, game, pitching);
  }
}

/*
 * TODO: CWBoxscore should probably maintain team stats records,
 *       which would make it feasible to re-use the player stat
 *       generation function.
 */
static void
cwbox_team_stats_baseball(XMLNode *parent,
			  CWGame *game, CWBoxscore *boxscore, int t)
{
  XMLNode *statsNode = NULL, *node = NULL;
  int slot, pos;
  CWBoxPlayer *player;
  CWBoxPitcher *pitcher;

  int pa = 0, ab = 0, r = 0, h = 0, b2 = 0, b3 = 0, hr = 0, hrslam = 0, bi = 0;
  int bb = 0, ibb = 0, so = 0, gdp = 0, hp = 0, sh = 0, sf = 0;
  int sb = 0, cs = 0, xi = 0, lisp = 0, movedup = 0;

  int po = 0, a = 0, e = 0, pb = 0, dxi = 0;

  int outs = 0, ha = 0, ra = 0, er = 0, hra = 0, bba = 0, ibba = 0, soa = 0;
  int sha = 0, sfa = 0, hb = 0, wp = 0, bk = 0, pk = 0, inr = 0, inrs = 0;
  int pitches = 0, strikes = 0;

  for (slot = 1; slot <= 9; slot++) {
    player = cw_box_get_starter(boxscore, t, slot);
    
    while (player != NULL) {
      pa += player->batting->pa;
      ab += player->batting->ab;
      r += player->batting->r;
      h += player->batting->h;
      b2 += player->batting->b2;
      b3 += player->batting->b3;
      hr += player->batting->hr;
      hrslam += player->batting->hrslam;
      bi += player->batting->bi;
      bb += player->batting->bb;
      ibb += player->batting->ibb;
      so += player->batting->so;
      gdp += player->batting->gdp;
      hp += player->batting->hp;
      sh += player->batting->sh;
      sf += player->batting->sf;
      sb += player->batting->sb;
      xi += player->batting->xi;
      cs += player->batting->cs;
      lisp += player->batting->lisp;
      movedup += player->batting->movedup;

      for (pos = 1; pos <= 9; pos++) {
	if (player->fielding[pos] != NULL) {
	  po += player->fielding[pos]->po;
	  a += player->fielding[pos]->a;
	  e += player->fielding[pos]->e;
	}
      }
      if (player->fielding[2] != NULL) {
	pb += player->fielding[2]->pb;
	dxi += player->fielding[2]->xi;
      }

      player = player->next;
    }
  }

  statsNode = xml_node_open(parent, "team-stats-baseball");

  node = xml_node_open(statsNode, "stats-baseball-offensive");
  xml_node_attribute_posint(node, "plate-appearances", pa);
  xml_node_attribute_posint(node, "at-bats", ab);
  xml_node_attribute_posint(node, "runs-scored", r);
  xml_node_attribute_posint(node, "hits", h);
  xml_node_attribute_posint(node, "total-bases", h+b2+2*b3+3*hr);
  xml_node_attribute_posint(node, "hits-extra-base", b2+b3+hr);
  xml_node_attribute_posint(node, "singles", h-b2-b3-hr);
  xml_node_attribute_posint(node, "doubles", b2);
  xml_node_attribute_posint(node, "triples", b3);
  xml_node_attribute_posint(node, "home-runs", hr);
  xml_node_attribute_posint(node, "grand-slams", hrslam);
  xml_node_attribute_posint(node, "rbi", bi);
  xml_node_attribute_posint(node, "bases-on-balls", bb);
  xml_node_attribute_posint(node, "bases-on-balls-intentional", ibb);
  xml_node_attribute_posint(node, "strikeouts", so);
  xml_node_attribute_posint(node, "grounded-into-double-play", gdp);
  xml_node_attribute_posint(node, "hit-by-pitch", hp);
  xml_node_attribute_posint(node, "sac-bunts", sh);
  xml_node_attribute_posint(node, "sac-flies", sf);
  xml_node_attribute_posint(node, "stolen-bases", sb);
  xml_node_attribute_posint(node, "stolen-bases-caught", cs);
  xml_node_attribute_posint(node, "reached-base-defensive-interference", xi);
  xml_node_attribute_posint(node, "left-on-base", boxscore->lob[t]);
  xml_node_attribute_posint(node, "left-in-scoring-position", lisp);
  xml_node_attribute_posint(node, "moved-up", movedup);

  node = xml_node_open(statsNode, "stats-baseball-defensive");
  xml_node_attribute_posint(node, "putouts", po);
  xml_node_attribute_posint(node, "assists", a);
  xml_node_attribute_posint(node, "errors", e);
  xml_node_attribute_posint(node, "double-plays", boxscore->dp[t]);
  xml_node_attribute_posint(node, "triple-plays", boxscore->tp[t]);
  xml_node_attribute_posint(node, "errors-passed-ball", pb);
  xml_node_attribute_posint(node, "errors-defensive-interference", dxi);

  pitcher = cw_box_get_starting_pitcher(boxscore, t);
  while (pitcher != NULL) {
    outs += pitcher->pitching->outs;
    ha += pitcher->pitching->h;
    ra += pitcher->pitching->r;
    er += pitcher->pitching->er;
    hra += pitcher->pitching->hr;
    sha += pitcher->pitching->sh;
    sfa += pitcher->pitching->sf;
    bba += pitcher->pitching->bb;
    ibba += pitcher->pitching->ibb;
    soa += pitcher->pitching->so;
    hb += pitcher->pitching->hb;
    wp += pitcher->pitching->wp;
    bk += pitcher->pitching->bk;
    pk += pitcher->pitching->pk;
    inr += pitcher->pitching->inr;
    inrs += pitcher->pitching->inrs;
    pitches += pitcher->pitching->pitches;
    strikes += pitcher->pitching->strikes;
    
    pitcher = pitcher->next;
  }

  node = xml_node_open(statsNode, "stats-baseball-pitching");
  if (outs % 3 == 0) {
    xml_node_attribute_int(node, "innings-pitched", outs / 3);
  }
  else {
    xml_node_attribute_fmt(node, "innings-pitched", "%d.%d",
			   outs / 3, outs % 3);
  }
  xml_node_attribute_posint(node, "runs-allowed", ra);
  xml_node_attribute_posint(node, "earned-runs", er);
  xml_node_attribute_posint(node, "hits", ha);
  xml_node_attribute_posint(node, "home-runs-allowed", hra);
  xml_node_attribute_posint(node, "bases-on-balls", bba);
  xml_node_attribute_posint(node, "bases-on-balls-intentional", ibba);
  xml_node_attribute_posint(node, "strikeouts", soa);
  xml_node_attribute_posint(node, "sacrifice-bunts-allowed", sha);
  xml_node_attribute_posint(node, "sacrifice-flies-allowed", sfa);
  xml_node_attribute_posint(node, "errors-hit-with-pitch", hb);
  xml_node_attribute_posint(node, "errors-wild-pitch", wp);
  xml_node_attribute_posint(node, "balks", bk);
  xml_node_attribute_posint(node, "pick-offs", pk);
  xml_node_attribute_posint(node, "inherited-runners-total", inr);
  xml_node_attribute_posint(node, "inherited-runners-scored", inrs);
  if (cw_game_info_lookup(game, "pitches") &&
      !strcmp(cw_game_info_lookup(game, "pitches"), "pitches")) {
    xml_node_attribute_int(node, "number-of-pitches", pitches);
    xml_node_attribute_int(node, "number-of-strikes", strikes);
  }

  if (cw_box_get_starting_pitcher(boxscore, t)->next == NULL) {
    xml_node_attribute_int(node, "games-complete", 1);
    xml_node_attribute_int(node, "games-finished", 0);
  }
  else {
    xml_node_attribute_int(node, "games-complete", 0);
    xml_node_attribute_int(node, "games-finished", 1);
  }
  xml_node_attribute_int(node, "shutouts", ((ra == 0) ? 1 : 0));
}


/*
 * Create a <team-stats> node as a child of 'parent'
 */
static void
cwbox_team_stats(XMLNode *parent, CWGame *game,
		 CWBoxscore *boxscore, int t)
{
  int i;
  XMLNode *node = NULL, *subnode = NULL;

  node = xml_node_open(parent, "team-stats");
  xml_node_attribute_int(node, "score", boxscore->score[t]);

  if (boxscore->score[t] > boxscore->score[1-t]) {
    xml_node_attribute(node, "event-outcome", "win");
  }
  else if (boxscore->score[t] < boxscore->score[1-t]) {
    xml_node_attribute(node, "event-outcome", "loss");
  }
  else {
    xml_node_attribute(node, "event-outcome", "tie");
  }

  for (i = 1; i < 50; i++) {
    if (boxscore->linescore[i][0] < 0 &&
	boxscore->linescore[i][1] < 0) {
      break;
    }

    subnode = xml_node_open(node, "sub-score");
    xml_node_attribute_int(subnode, "period-value", i);
    if (boxscore->linescore[i][t] >= 0) {
      xml_node_attribute_int(subnode, "score", boxscore->linescore[i][t]);
    }
  }

  cwbox_team_stats_baseball(node, game, boxscore, t);
}

/*
 * Crates a <team-metadata> node as a child of 'parent'.
 */
static void
cwbox_team_metadata(XMLNode *parent, int t, CWRoster *roster)
{
  XMLNode *node = NULL, *nameNode = NULL;

  node = xml_node_open(parent, "team-metadata");
  xml_node_attribute(node, "alignment", 
	     ((t == 0) ? "away" : "home"));
  if (roster != NULL) {
    xml_node_attribute(node, "team-key", roster->team_id);
    
    nameNode = xml_node_open(node, "name");
    xml_node_attribute(nameNode, "first", roster->city);
    xml_node_attribute(nameNode, "last", roster->nickname);
  }
} 

/*
 * Creates a <team> node as a child of parent, with data for team 't'
 * from the game, using the boxscore and roster
 */
static void
cwbox_team(XMLNode *parent,
	   CWGame *game, CWBoxscore *boxscore,
	   int t, CWRoster *roster)
{
  int slot;
  XMLNode *node = NULL;

  node = xml_node_open(parent, "team");
  cwbox_team_metadata(node, t, roster);
  cwbox_team_stats(node, game, boxscore, t);

  for (slot = 1; slot <= 10; slot++) {
    /* This loop puts the zero slot for non-batting pitchers last */
    CWBoxPlayer *player = cw_box_get_starter(boxscore, t, slot % 10);
    int seq = 1;

    while (player != NULL) {
      cwbox_player(node, game, player, 
		   cw_box_find_pitcher(boxscore, player->player_id),
		   slot % 10, seq++, roster);
      player = player->next;
    }
  }
}

/*
 * Add the official at (Retrosheet) umpire position 'infoLabel',
 * using metadata position label 'metaLabel'.  If umpire position
 * is empty, do nothing.
 */
static void
cwbox_official(XMLNode *parent, CWGame *game, 
	       char *infoLabel, char *metaLabel)
{
  XMLNode *umpNode = NULL, *umpDataNode = NULL;
  char *umpID= cw_game_info_lookup(game, infoLabel);

  if (umpID) {
    umpNode = xml_node_open(parent, "official");
    umpDataNode = xml_node_open(umpNode, "official-metadata");
    xml_node_attribute(umpDataNode, "position", metaLabel);
    xml_node_attribute_fmt(umpDataNode, "official-key", "p.%s", umpID);
  }
}

/*
 * Add <officials> as a child of parent
 */
static void
cwbox_officials(XMLNode *parent, CWGame *game)
{
  XMLNode *node = NULL;

  node = xml_node_open(parent, "officials");

  cwbox_official(node, game, "umphome", "Home Plate Umpire");
  cwbox_official(node, game, "ump1b", "First Base Umpire");
  cwbox_official(node, game, "ump2b", "Second Base Umpire");
  cwbox_official(node, game, "ump3b", "Third Base Umpire");
  cwbox_official(node, game, "umplf", "Left Field Umpire");
  cwbox_official(node, game, "umprf", "Right Field Umpire");
}

/* 
 * NOTE: This is not yet part of the formal SportsML standard.
 * For now, these imitate MLBAM codes, except using SportsML-style
 * dashes in between words, instead of MLBAM's underscores.
 *
 * There are a few places where assumptions about categorization are
 * made; these are noted in comments.
 */
static char *
cwbox_sml_get_play_type(CWGameIterator *gameiter)
{
  switch (gameiter->event_data->event_type) {
  case CW_EVENT_GENERICOUT:
    if (gameiter->event_data->sh_flag) {
      if (gameiter->event_data->dp_flag) {
	return "sac-bunt-double-play";
      }
      else {
	return "sac-bunt";
      }
    }
    else if (gameiter->event_data->sf_flag) {
      if (gameiter->event_data->dp_flag) {
	return "sac-fly-double-play";
      }
      else {
	return "sac-fly";
      }
    }
    else if (gameiter->event_data->gdp_flag) {
      return "grounded-into-double-play";
    }
    else if (gameiter->event_data->dp_flag) {
      return "double-play";
    }
    else if (gameiter->event_data->tp_flag) {
      return "triple-play";
    }
    else if (gameiter->event_data->fc_flag[1] ||
	     gameiter->event_data->fc_flag[2] ||
	     gameiter->event_data->fc_flag[3]) {
      return "force-out";
    }
    else {
      return "field-out";
    }
  case CW_EVENT_STRIKEOUT:
    if (gameiter->event_data->dp_flag) {
      return "strikeout-double-play";
    }
    else if (gameiter->event_data->tp_flag) {
      return "strikeout-triple-play";
    }
    else {
      return "strikeout";
    }
  case CW_EVENT_STOLENBASE:
    /* MLBAM does not have any code for double-steal.  For now, emit
     * the lead base stolen for the event type.
     */
    if (gameiter->event_data->sb_flag[3]) {
      return "stolen-base-home";
    }
    else if (gameiter->event_data->sb_flag[2]) {
      return "stolen-base-3b";
    }
    else {
      return "stolen-base-2b";
    }
  case CW_EVENT_INDIFFERENCE:
    return "defensive-indiff";
  case CW_EVENT_CAUGHTSTEALING:
    if (gameiter->event_data->dp_flag) {
      return "cs-double-play";
    }
    else if (gameiter->event_data->cs_flag[1]) {
      if (gameiter->event_data->po_flag[1]) {
	return "pickoff-caught-stealing-2b";
      }
      else {
	return "caught-stealing-2b";
      }
    }
    else if (gameiter->event_data->cs_flag[2]) {
      if (gameiter->event_data->po_flag[2]) {
	return "pickoff-caught-stealing-3b";
      }
      else {
	return "caught-stealing-3b";
      }
    }
    else {
      if (gameiter->event_data->po_flag[3]) {
	return "pickoff-caught-stealing-home";
      }
      else {
	return "caught-stealing-home";
      }
    }
    /* Note: CW_EVENT_PICKOFFERROR no longer used by Retrosheet */
  case CW_EVENT_PICKOFF:
    if (gameiter->event_data->po_flag[1]) {
      if (gameiter->event_data->cs_flag[1]) {
	return "pickoff-caught-stealing-2b";
      }
      else {
	return "pickoff-1b";
      }
    }
    else if (gameiter->event_data->po_flag[2]) {
      if (gameiter->event_data->cs_flag[2]) {
	return "pickoff-caught-stealing-3b";
      }
      else {
	return "pickoff-2b";
      }
    }
    else {
      if (gameiter->event_data->cs_flag[3]) {
	return "pickoff-caught-stealing-home";
      }
      else {
	return "pickoff-3b";
      }
    }
  case CW_EVENT_WILDPITCH:
    return "wild-pitch";
  case CW_EVENT_PASSEDBALL:
    return "passed-ball";
  case CW_EVENT_BALK:
    return "balk";
  case CW_EVENT_OTHERADVANCE:
    return "other-advance";
  case CW_EVENT_FOULERROR:
    /* This does not appear in the MLBAM codes, but does appear in
     * DiamondWare.  It seems this should be its own event type.
     */
    return "foul-error";
  case CW_EVENT_WALK:
    return "walk";
  case CW_EVENT_INTENTIONALWALK:
    return "intent-walk";
  case CW_EVENT_HITBYPITCH:
    return "hit-by-pitch";
  case CW_EVENT_INTERFERENCE:
    /* Retrosheet uses the C notation for all interferences on which
     * the batter is awarded first base.  Most are catcher's interference
     * C/E2, but some may be on other fielders.
     *
     * Note the inconsistency in MLBAM's naming scheme here.
     */
    if (gameiter->event_data->errors[0] == 2) {
      return "catcher-interf";
    }
    else {
      return "fielder-interference";
    }
  case CW_EVENT_ERROR:
    return "field-error";
  case CW_EVENT_FIELDERSCHOICE:
    if (cw_event_outs_on_play(gameiter->event_data) > 0) {
      return "fielders-choice-out";
    }
    else {
      return "fielders-choice";
    }
  case CW_EVENT_SINGLE:
    return "single";
  case CW_EVENT_DOUBLE:
    return "double";
  case CW_EVENT_TRIPLE:
    return "triple";
  case CW_EVENT_HOMERUN:
    return "home-run";
  default:
    return "unknown-play";
  }
}

/*
 * Generate hit-type entries for the current event
 */
static char *
cwbox_sml_get_hit_type(CWGameIterator *gameiter)
{
  switch (gameiter->event_data->batted_ball_type) {
  case 'F':
    return "fly-ball";
  case 'G':
    return (gameiter->event_data->bunt_flag) ? "bunt-grounder" : "ground-ball";
  case 'L':
    return (gameiter->event_data->bunt_flag) ? "bunt-line-drive" : "line-drive";
  case 'P':
    return (gameiter->event_data->bunt_flag) ? "bunt-popup" : "popup"; 
  default:
    return "";
  }
}

char *
cwbox_print_sml_pitch(XMLNode *action, XMLNode *node, char *pitch)
{
  char *c = pitch + 1;
  XMLNode *contact = NULL;

  if (*c == '{') {
    /* This is an extended pitch datum */

    c++;

    /* First entry: pitch type, one character */
    switch (*c) {
    case 'F':
      xml_node_attribute(node, "pitch-type", "fastball");
      c++;
      break;
    case 'N':
      xml_node_attribute(node, "pitch-type", "sinker");
      c++;
      break;
    case 'C':
      xml_node_attribute(node, "pitch-type", "curve");
      c++;
      break;
    case 'R':
      xml_node_attribute(node, "pitch-type", "splitter");
      c++;
      break;
    case 'S':
      xml_node_attribute(node, "pitch-type", "slider");
      c++;
      break;
    case 'K':
      xml_node_attribute(node, "pitch-type", "knuckleball");
      c++;
      break;
    case 'H':
      xml_node_attribute(node, "pitch-type", "changeup");
      c++;
      break;
    case 'U':
      xml_node_attribute(node, "pitch-type", "unknown");
      c++;
      break;
    case 'T':
      xml_node_attribute(node, "pitch-type", "cutter");
      c++;
      break;
    case '|':
      break;
    default:
      c++;
      break;
    }
    
    /* At this point, 'c' should be pointing at a '|' */
    c++;

    /* Next, pitch velocity */
    if (*c != '|') {
      char buffer[256];
      int i = 0;
      while (*c != '|') {
	buffer[i++] = *c;
	c++;
      }
      buffer[i] = '\0';
      
      xml_node_attribute(node, "pitch-velocity", buffer);
    }

    /* At this point, 'c' should be pointing at a '|' */
    c++;

    /* Next, pitch coordinate X */
    if (*c != '|') {
      char buffer[256];
      int i = 0;
      while (*c != '|') {
	buffer[i++] = *c;
	c++;
      }
      buffer[i] = '\0';
      
      xml_node_attribute(node, "pitch-coordinate-x", buffer);
    }

    /* At this point, 'c' should be pointing at a '|' */
    c++;

    /* Next, pitch coordinate Y */
    if (*c != '|' && *c != '}') {
      char buffer[256];
      int i = 0;
      while (*c != '|' && *c != '}') {
	buffer[i++] = *c;
	c++;
      }
      buffer[i] = '\0';
      
      xml_node_attribute(node, "pitch-coordinate-y", buffer);
    }
    
    /* At this point, 'c' should be pointing at a '|' or a '}' */

    if (*c == '}') {
      return c;
    }

    /* Start action-baseball-contact, which is a child of action-baseball-pitch  */
    contact = xml_node_open(node, "action-baseball-contact");

    /* Hit coordinates X and Y follow */
    c++;

    /* Next, hit coordinate X */
    if (*c != '|') {
      char buffer[256];
      int i = 0;
      while (*c != '|') {
	buffer[i++] = *c;
	c++;
      }
      buffer[i] = '\0';
      
      xml_node_attribute(contact, "hit-location-x", buffer);
    }

    /* At this point, 'c' should be pointing at a '}' */
    c++;

    /* Next, hit coordinate Y */
    if (*c != '}') {
      char buffer[256];
      int i = 0;
      while (*c != '}') {
	buffer[i++] = *c;
	c++;
      }
      buffer[i] = '\0';
      
      xml_node_attribute(contact, "hit-location-y", buffer);
    }
    return c;
  }
  else {
    return pitch;
  }
}

static void
cwbox_action_baseball_play(XMLNode *parent, CWGameIterator *gameiter,
			   CWRoster *visitors, CWRoster *home,
			   int seq, int new_pa)
{
  XMLNode *node = NULL;
  char batHand, *battedBallType;

  if (cw_event_runs_on_play(gameiter->event_data) > 0) {
    node = xml_node_open(parent, "action-baseball-score");
  }
  else {
    node = xml_node_open(parent, "action-baseball-play");
  }

  xml_node_attribute_int(node, "sequence-number", seq);
  xml_node_attribute_int(node, "inning-value", gameiter->state->inning);
  if (cw_game_info_lookup(gameiter->game, "htbf") &&
      !strcmp(cw_game_info_lookup(gameiter->game, "htbf"), "true")) {
    xml_node_attribute(node, "inning-half",
		       ((gameiter->state->batting_team == 0) ? "bottom" : "top"));
  }
  else {
    xml_node_attribute(node, "inning-half",
		       ((gameiter->state->batting_team == 0) ? "top" : "bottom"));
  }
  xml_node_attribute_int(node, "outs", gameiter->state->outs);

  if (gameiter->event_data->advance[0] >= 1 &&
      gameiter->event_data->advance[0] <= 3) {
    xml_node_attribute_int(node, "batter-advance",
			   gameiter->event_data->advance[0]);
  }
  else if (gameiter->event_data->advance[0] >= 4) {
    xml_node_attribute(node, "batter-advance", "home");
  }
  else if (strcmp(gameiter->event_data->play[0], "") &&
	   !strstr(gameiter->event_data->play[0], "E")) {
    xml_node_attribute(node, "batter-advance", "out");
  }

  if (strcmp(gameiter->state->runners[1], "")) {
    xml_node_attribute_fmt(node, "runner-on-first-idref", "p.%s",
			   gameiter->state->runners[1]);
    if (gameiter->event_data->advance[1] >= 1 &&
	gameiter->event_data->advance[1] <= 3) {
      xml_node_attribute_int(node, "runner-on-first-advance",
			     gameiter->event_data->advance[1]);
    }
    else if (gameiter->event_data->advance[1] >= 4) {
      xml_node_attribute(node, "runner-on-first-advance", "home");
    }
    else if (strcmp(gameiter->event_data->play[1], "") &&
	     !strstr(gameiter->event_data->play[1], "E")) {
      xml_node_attribute(node, "runner-on-first-advance", "out");
    }
  }
  if (strcmp(gameiter->state->runners[2], "")) {
    xml_node_attribute_fmt(node, "runner-on-second-idref", "p.%s",
			   gameiter->state->runners[2]);
    if (gameiter->event_data->advance[2] >= 1 &&
	gameiter->event_data->advance[2] <= 3) {
      xml_node_attribute_int(node, "runner-on-second-advance",
			     gameiter->event_data->advance[2]);
    }
    else if (gameiter->event_data->advance[2] >= 4) {
      xml_node_attribute(node, "runner-on-second-advance", "home");
    }
    else if (strcmp(gameiter->event_data->play[2], "") &&
	     !strstr(gameiter->event_data->play[2], "E")) {
      xml_node_attribute(node, "runner-on-second-advance", "out");
    }
  }
  if (strcmp(gameiter->state->runners[3], "")) {
    xml_node_attribute_fmt(node, "runner-on-third-idref", "p.%s",
			   gameiter->state->runners[3]);
    if (gameiter->event_data->advance[3] >= 1 &&
	gameiter->event_data->advance[3] <= 3) {
      xml_node_attribute_int(node, "runner-on-third-advance",
			     gameiter->event_data->advance[3]);
    }
    else if (gameiter->event_data->advance[3] >= 4) {
      xml_node_attribute(node, "runner-on-third-advance", "home");
    }
    else if (strcmp(gameiter->event_data->play[3], "") &&
	     !strstr(gameiter->event_data->play[3], "E")) {
      xml_node_attribute(node, "runner-on-third-advance", "out");
    }
  }

  /* ASSUMPTION: batter-idref and pitcher-idref refer to the
   * charged batter and pitcher, which might not be the ones who
   * took the final action!
   */
  xml_node_attribute_fmt(node, "pitcher-idref", "p.%s",
			 cw_gamestate_charged_pitcher(gameiter->state,
						      gameiter->event_data));
  xml_node_attribute_fmt(node, "batter-idref", "p.%s",
			 cw_gamestate_charged_batter(gameiter->state,
						     gameiter->event->batter,
						     gameiter->event_data));
  
  batHand = cw_gamestate_charged_batter_hand(gameiter->state,
					     gameiter->event->batter,
					     gameiter->event_data,
					     (gameiter->state->batting_team == 0) ? visitors : home,
					     (gameiter->state->batting_team == 0) ? home : visitors);
  /* Note that batHand might be '?', unknown */
  if (batHand == 'R') {
    xml_node_attribute(node, "batter-side", "right");
  }
  else if (batHand == 'L') {
    xml_node_attribute(node, "batter-side", "left");
  }

  xml_node_attribute(node, "play-type", 
	     cwbox_sml_get_play_type(gameiter));
  
  battedBallType = cwbox_sml_get_hit_type(gameiter);
  if (strcmp(battedBallType, "")) {
    xml_node_attribute(node, "hit-type", battedBallType);
  }
  
  xml_node_attribute(node, "play-scorekeepers-notation",
	     gameiter->event->event_text);
  xml_node_attribute_int(node, "outs-recorded",
		cw_event_outs_on_play(gameiter->event_data));

  if (cw_event_runs_on_play(gameiter->event_data) > 0) {
    xml_node_attribute_int(node, "rbi",
			   cw_event_rbi_on_play(gameiter->event_data));
    xml_node_attribute_int(node, "runs-scored",
			   cw_event_runs_on_play(gameiter->event_data));

    /* FIXME: Is earned-runs-scored team or individually earned? */
    xml_node_attribute_int(node, "earned-runs-scored",
			   ((gameiter->event_data->advance[0] == 4) ? 1 : 0) +
			   ((gameiter->event_data->advance[1] == 4) ? 1 : 0) +
			   ((gameiter->event_data->advance[2] == 4) ? 1 : 0) +
			   ((gameiter->event_data->advance[3] == 4) ? 1 : 0));

    xml_node_attribute_int(node, "score-team",
			   gameiter->state->score[gameiter->state->batting_team] +
			   cw_event_runs_on_play(gameiter->event_data));
    xml_node_attribute_int(node, "score-team-opposing",
			   gameiter->state->score[1-gameiter->state->batting_team]);
  }

  /* Pitches here */
  /* Earlier versions checked for whether the "info,pitches" field
   * read "pitches."  However, some files aren't marked as having pitches,
   * but do have pitches for some plate appearances; and, some are marked
   * as having pitches, but clearly only have incomplete pitch data.
   * So, we ignore the info,pitches field, and simply report what the
   * event file has.
   */
  {
    XMLNode *pitch;
    char *pitches = gameiter->event->pitches;
    char *c;

    if (!new_pa) {
      /* Skip pitches in previous event */
      pitches += strlen(gameiter->event->prev->pitches);
    }

    for (c = pitches; *c != '\0'; c++) {
      if (*c == '.' || *c == '>' ||
	  *c == '1' || *c == '2' || *c == '3' || *c == '+') {
	continue;
      }

      pitch = xml_node_open(node, "action-baseball-pitch");

      switch (*c) {
      case 'B':
	xml_node_attribute(pitch, "umpire-call", "ball");
	xml_node_attribute(pitch, "ball-type", "called");
	break;
      case 'C':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "called");
	break;
      case 'F':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "foul");
	break;
      case 'H':
	xml_node_attribute(pitch, "umpire-call", "ball");
	xml_node_attribute(pitch, "ball-type", "hit-by-pitch");
	break;
      case 'I':
	xml_node_attribute(pitch, "umpire-call", "ball");
	xml_node_attribute(pitch, "ball-type", "intentional");
	break;
      case 'K':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "unknown");
	break;
      case 'L':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "foul-bunt");
	break;
      case 'M':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "missed-bunt");
	break;
      case 'N':
	xml_node_attribute(pitch, "umpire-call", "no-pitch");
	break;
      case 'O':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "foul-tip-on-bunt");
	break;
      case 'P':
	xml_node_attribute(pitch, "umpire-call", "ball");
	xml_node_attribute(pitch, "ball-type", "pitchout");
	break;
      case 'Q':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "swinging-on-pitchout");
	break;
      case 'R':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "foul-on-pitchout");
	break;
      case 'S':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "swinging");
	break;
      case 'T':
	xml_node_attribute(pitch, "umpire-call", "strike");
	xml_node_attribute(pitch, "strike-type", "foul-tip");
	break;
      case 'U':
	break;
      case 'V':
	xml_node_attribute(pitch, "umpire-call", "ball");
	xml_node_attribute(pitch, "ball-type", "automatic");
	break;
      case 'X':
	xml_node_attribute(pitch, "umpire-call", "in-play");
	break;
      case 'Y':
	xml_node_attribute(pitch, "umpire-call", "in-play");
	break;
      case '*':
	c++;
	if (*c == 'B') {
	  xml_node_attribute(pitch, "umpire-call", "ball");
	  xml_node_attribute(pitch, "ball-type", "blocked");
	}
	else if (*c == 'S') {
	  xml_node_attribute(pitch, "umpire-call", "strike");
	  xml_node_attribute(pitch, "strike-type", "swinging-blocked");
	}
	break;
      default:
	break;
      }

      c = cwbox_print_sml_pitch(node, pitch, c);
    }
  }
}

/*
 * Semantic note on substitutions: Retrosheet files do not contain
 * an additional substitution for a player who PH or PRs for a DH
 * to indicate that they automatically become the DH.
 */
static void
cwbox_action_baseball_substitution(XMLNode *parent,
				   CWGameIterator *gameiter,
				   CWAppearance *sub,
				   CWRoster *visitors, CWRoster *home,
				   int seq)
{
  XMLNode *node = NULL;

  node = xml_node_open(parent, "action-baseball-substitution");

  xml_node_attribute_int(node, "sequence-number", seq);
  xml_node_attribute_int(node, "inning-value", gameiter->state->inning);
  if (cw_game_info_lookup(gameiter->game, "htbf") &&
      !strcmp(cw_game_info_lookup(gameiter->game, "htbf"), "true")) {
    xml_node_attribute(node, "inning-half",
		       ((gameiter->state->batting_team == 0) ? "bottom" : "top"));
  }
  else {
    xml_node_attribute(node, "inning-half",
		       ((gameiter->state->batting_team == 0) ? "top" : "bottom"));
  }
  xml_node_attribute_int(node, "outs", gameiter->state->outs);
  xml_node_attribute(node, "person-type", "player");

  xml_node_attribute_fmt(node, "person-original-idref", "p.%s",
			 gameiter->state->lineups[sub->slot][sub->team].player_id);

  switch (gameiter->state->lineups[sub->slot][sub->team].position) {
  case 10:
    xml_node_attribute(node, "person-original-position", "dh");
    break;
  case 11:
    xml_node_attribute(node, "person-original-position", "ph");
    break;
  case 12:
    xml_node_attribute(node, "person-original-position", "pr");
    break;
  default:
    xml_node_attribute_int(node, "person-original-position",
			   gameiter->state->lineups[sub->slot][sub->team].position);
    break;
  }

  if (sub->slot > 0 && 
      gameiter->state->lineups[0][sub->team].player_id != NULL &&
      !strcmp(gameiter->state->lineups[0][sub->team].player_id, sub->player_id)) {
    /* This is a case of the pitcher assuming a lineup slot */
    xml_node_attribute(node, "person-original-lineup-slot", "0");
  }
  else {
    xml_node_attribute_int(node, "person-original-lineup-slot", sub->slot);
  }
  xml_node_attribute_fmt(node, "person-replacing-idref", "p.%s", 
			 sub->player_id);
  
  if (gameiter->state->lineups[sub->slot][sub->team].position == 10 &&
      gameiter->state->batting_team == sub->team) {
    /* Convention: when pinch-hitting or pinch-running for the DH,
       report new position as DH
    */
    xml_node_attribute(node, "person-replacing-position", "dh");
  }
  else {
    switch (sub->pos) {
    case 10:
      xml_node_attribute(node, "person-replacing-position", "dh");
      break;
    case 11:
      xml_node_attribute(node, "person-replacing-position", "ph");
      break;
    case 12:
      xml_node_attribute(node, "person-replacing-position", "pr");
      break;
    default:
      xml_node_attribute_int(node, "person-replacing-position", sub->pos);
      break;
    }
  }

  xml_node_attribute_int(node, "person-replacing-lineup-slot", sub->slot);
}

/*
 * Creates <event-actions>, with all PBP actions, as child of parent.
 */
static void
cwbox_actions(XMLNode *parent, CWGame *game, CWBoxscore *boxscore,
	      CWRoster *visitors, CWRoster *home)
{
  XMLNode *actionNode = NULL, *node = NULL;
  CWGameIterator *gameiter = cw_gameiter_create(game);
  int seq = 1;
  int new_pa = 1;   /* Nonzero if current event starts a new PA */

  actionNode = xml_node_open(parent, "event-actions");
  node = xml_node_open(actionNode, "event-actions-baseball");
  
  while (gameiter->event != NULL) {
    CWAppearance *sub = gameiter->event->first_sub;

    if (!gameiter->parse_ok) {
      fprintf(stderr, "Parse error in game %s at event %d:\n",
	      game->game_id, gameiter->state->event_count+1);
      fprintf(stderr, "Invalid play string \"%s\" (%s batting)\n",
	      gameiter->event->event_text, gameiter->event->batter);
      exit(1);
    }

    if (strcmp(gameiter->event->event_text, "NP")) {
      cwbox_action_baseball_play(node, 
				 gameiter, visitors, home, seq++, new_pa);

      if (cw_event_is_batter(gameiter->event_data) ||
	  gameiter->state->outs + cw_event_outs_on_play(gameiter->event_data) >= 3) {
	new_pa = 1;
      }
      else {
	new_pa = 0;
      }
    }

    while (sub != NULL) {
      cwbox_action_baseball_substitution(node,
					 gameiter, sub, visitors, home, seq++);
      sub = sub->next;
    }
    

    cw_gameiter_next(gameiter);
  }
}

/*
 * Add a <site> element as child of 'parent'
 */
static void
cwbox_site(XMLNode *parent, CWGame *game)
{
  XMLNode *siteNode = NULL, *metadataNode = NULL, *statsNode = NULL;

  siteNode = xml_node_open(parent, "site");
  metadataNode = xml_node_open(siteNode, "site-metadata");
  if (cw_game_info_lookup(game, "site")) {
    /* Retrosheet convention is that the 'site' entry is a code */
    xml_node_attribute(metadataNode, "site-key",
		       cw_game_info_lookup(game, "site"));
  }
  if (cw_game_info_lookup(game, "site-name")) {
    /* Extension: use info,site-name to embed the site's name in files */
    XMLNode *nameNode = xml_node_open(metadataNode, "name");
    xml_node_attribute(nameNode, "full",
		       cw_game_info_lookup(game, "site-name"));
  }
  xml_node_open(metadataNode, "home-location");

  statsNode = xml_node_open(siteNode, "site-stats");
  if (cw_game_info_lookup(game, "attendance")) {
    xml_node_attribute(statsNode, "attendance", 
		       cw_game_info_lookup(game, "attendance"));
  }
}

/*
 * Add a <event-metadata> element as child of 'parent'
 */
static void
cwbox_event_metadata(XMLNode *parent, CWGame *game)
{
  XMLNode *node = NULL;
  char season[5];
  int timeofgame;

  strncpy(season, cw_game_info_lookup(game, "date"), 4); 
  season[4] = '\0';

  node = xml_node_open(parent, "event-metadata");
  xml_node_attribute(node, "date-coverage-type", "event");

  xml_node_attribute_fmt(node, "event-key", "l.mlb.com-%s-e.%s", 
			 season, game->game_id);

  /* date-coverage-value is in fact same as event-key */
  xml_node_attribute_fmt(node, "date-coverage-value", "l.mlb.com-%s-e.%s", 
			 season, game->game_id);

  xml_node_attribute_fmt(node, "start-date-time",
			 "%c%c%c%c%c%c%c%cT000000-0000",
			 cw_game_info_lookup(game, "date")[0],
			 cw_game_info_lookup(game, "date")[1],
			 cw_game_info_lookup(game, "date")[2],
			 cw_game_info_lookup(game, "date")[3],
			 cw_game_info_lookup(game, "date")[5],
			 cw_game_info_lookup(game, "date")[6],
			 cw_game_info_lookup(game, "date")[8],
			 cw_game_info_lookup(game, "date")[9]);

  xml_node_attribute(node, "event-status", "post-event");
  
  if (cw_game_info_lookup(game, "timeofgame") &&
      sscanf(cw_game_info_lookup(game, "timeofgame"), "%d", &timeofgame) &&
      timeofgame > 0) {
    xml_node_attribute_fmt(node, "duration", "%d:%02d",
			   timeofgame / 60, timeofgame % 60);
  }

  xml_node_attribute_int(node, "game-of-day",
			 (cw_atoi(cw_game_info_lookup(game, "number")) == 0) ? 1 :
			  cw_atoi(cw_game_info_lookup(game, "number")));

  if (cw_game_info_lookup(game, "htbf") &&
      !strcmp(cw_game_info_lookup(game, "htbf"), "true")) {
    xml_node_attribute(node, "site-alignment", "away");
  }


  /* For the moment, <event-metadata-baseball> is empty, so we just
   * create it here rather than using a separate function.
   */
  xml_node_open(node, "event-metadata-baseball");

  cwbox_site(node, game);
}


/*
 * Add a <sports-event> element as child of 'parent'
 */
static void
cwbox_sports_event(XMLNode *parent, CWGame *game, CWBoxscore *boxscore,
		   CWRoster *visitors, CWRoster *home)
{
  XMLNode *node = NULL;

  node = xml_node_open(parent, "sports-event");

  cwbox_event_metadata(node, game);

  cwbox_team(node, game, boxscore, 0, visitors);
  cwbox_team(node, game, boxscore, 1, home);

  cwbox_officials(node, game);
  cwbox_actions(node, game, boxscore, visitors, home);
}

/*
 * Add a <sports-title> element as child of 'parent'
 */
static void
cwbox_sports_title(XMLNode *parent, CWGame *game,
		   CWRoster *visitors, CWRoster *home)
{
  char season[5], buffer[256];
  XMLNode *node = xml_node_open(parent, "sports-title");

  strncpy(season, cw_game_info_lookup(game, "date"), 4); 
  season[4] = '\0';

  sprintf(buffer, "%s %s at %s %s, %c%c/%c%c/%s",
	  visitors->city, visitors->nickname, home->city, home->nickname,
	  cw_game_info_lookup(game, "date")[5],
	  cw_game_info_lookup(game, "date")[6],
	  cw_game_info_lookup(game, "date")[8],
	  cw_game_info_lookup(game, "date")[9], season);

  xml_node_cdata(node, buffer);
}

/*
 * Add <sports-content-codes> and children as child of 'parent'.
 *
 * TODO: More/all of these should be configurable in the data structure,
 *       like with "publisher".
 */
static void
cwbox_sports_content_codes(XMLNode *parent, CWGame *game,
			   CWRoster *visitors, CWRoster *home)
{
  XMLNode *codes = NULL, *node = NULL;
  char season[5];

  strncpy(season, cw_game_info_lookup(game, "date"), 4); 
  season[4] = '\0';

  codes = xml_node_open(parent, "sports-content-codes");

  node = xml_node_open(codes, "sports-content-code");
  if (cw_game_info_lookup(game, "publisher")) {
    xml_node_attribute(node, "code-name",
		       cw_game_info_lookup(game, "publisher"));
    xml_node_attribute(node, "code-key", 
		       cw_game_info_lookup(game, "publisher-key"));
    xml_node_attribute(node, "code-type", "publisher");
  }
  else {
    xml_node_attribute(node, "code-name", "Retrosheet");
    xml_node_attribute(node, "code-key", "retrosheet.org");
    xml_node_attribute(node, "code-type", "publisher");
  }

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-name", "XML Team Solutions, Inc.");
  xml_node_attribute(node, "code-key", "xmlteam.com");
  xml_node_attribute(node, "code-type", "distributor");

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "sport");
  xml_node_attribute(node, "code-key", "15007000");
  xml_node_attribute(node, "code-name", "Baseball");

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "league");
  if (cw_game_info_lookup(game, "league")) {
    xml_node_attribute(node, "code-name", 
		       cw_game_info_lookup(game, "league"));
  }
  else {
    xml_node_attribute(node, "code-name", "Major League Baseball");
  }

  if (cw_game_info_lookup(game, "league-key")) {
    xml_node_attribute(node, "code-key", 
		       cw_game_info_lookup(game, "league-key"));
  }
  else {
    xml_node_attribute(node, "code-key", "l.mlb.com");
  }

  if (cw_game_info_lookup(game, "level")) {
    node = xml_node_open(codes, "sports-content-code");
    xml_node_attribute(node, "code-type", "level");
    xml_node_attribute(node, "code-key", cw_game_info_lookup(game, "level"));
  }

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "season-type");
  if (cw_game_info_lookup(game, "season-type")) {
    xml_node_attribute(node, "code-key",
		       cw_game_info_lookup(game, "season-type"));
  }
  else {
    xml_node_attribute(node, "code-key", "regular");
  }

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "season");
  if (cw_game_info_lookup(game, "season")) {
    xml_node_attribute(node, "code-key", cw_game_info_lookup(game, "season"));
  }
  else {
    char season[5];
    strncpy(season, cw_game_info_lookup(game, "date"), 4); 
    season[4] = '\0';
    xml_node_attribute(node, "code-key", season);
  }

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "priority");
  xml_node_attribute(node, "code-key", "normal");

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "team");
  xml_node_attribute(node, "code-key", visitors->team_id);
  xml_node_attribute_fmt(node, "code-name", "%s %s",
			 visitors->city, visitors->nickname);

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "team");
  xml_node_attribute(node, "code-key", home->team_id);
  xml_node_attribute_fmt(node, "code-name", "%s %s",
			 home->city, home->nickname);

  node = xml_node_open(codes, "sports-content-code");
  xml_node_attribute(node, "code-type", "action-listing");
  xml_node_attribute(node, "code-key", "complete");
}

/*
 * Add a <sports-metadata> element as child of 'parent'
 */
static void
cwbox_sports_metadata(XMLNode *parent, CWGame *game,
		      CWRoster *visitors, CWRoster *home)
{
  time_t t;
  struct tm *tt;
  char season[5];
  char buffer1[256], buffer2[256];
  XMLNode *node = xml_node_open(parent, "sports-metadata");

  xml_node_attribute(node, "language", "en-US");

  time(&t);
  tt = localtime(&t);
  strftime(buffer1, 256, "%Y%m%dT%H%M%S", tt);
#if defined(HAVE_STRUCT_TM_TM_GMTOFF)
  sprintf(buffer2, "%s%+03ld%02ld", 
	 buffer1, tt->tm_gmtoff / 3600, tt->tm_gmtoff % 3600);
#elif defined(HAVE_UNDERSCORE_TIMEZONE)
  sprintf(buffer2, "%s%+03ld%02ld", 
	  buffer1, _timezone / 3600, _timezone % 3600);
#else
#error "Don't know how to find current time zone"
#endif  /* HAVE_STRUCT_TM_TM_GMTOFF, HAVE_UNDERSCORE_TIMEZONE */
  xml_node_attribute(node, "date-time", buffer2);

  sprintf(buffer1, "%s.%s.box", "Retrosheet", game->game_id);
  xml_node_attribute(node, "doc-id", buffer1);

  strncpy(season, cw_game_info_lookup(game, "date"), 4); 
  season[4] = '\0';
  sprintf(buffer1, "l.mlb.com-%s-e.%s-event-stats", 
	  season, game->game_id);
  xml_node_attribute(node, "revision-id", buffer1);

  xml_node_attribute(node, "fixture-key", "event-stats");
  xml_node_attribute(node, "document-class", "event-summary");
  xml_node_attribute(node, "fixture-name", "Box Score");

  cwbox_sports_title(node, game, visitors, home);
  cwbox_sports_content_codes(node, game, visitors, home);
} 

/*
 * Outputs the boxscore in SportsML format
 */
void
cwbox_print_sportsml(XMLDoc *doc,
		     CWGame *game, CWBoxscore *boxscore,
		     CWRoster *visitors, CWRoster *home)
{
  XMLNode *node;

  if (!doc) {
    doc = xml_document_create(stdout, "sports-content");
    node = doc->root;
  }
  else {
    node = xml_node_open(doc->root, "sports-content");
  }
  xml_node_attribute(node, "xmlns:xts", "http://www.xmlteam.com");
  xml_node_attribute(node, "xts:systemid", "MLB_Boxscore_XML");

  cwbox_sports_metadata(node, game, visitors, home);
  cwbox_sports_event(node, game, boxscore, visitors, home);

  if (node == doc->root) {
    xml_document_cleanup(doc);
  }
}
