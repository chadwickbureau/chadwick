/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/cwboxxml.c
 * Chadwick boxscore generator program, XML output
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
#include <string.h>

#include "cwlib/chadwick.h"

/*
 * Output the linescore data
 */
void
cwbox_xml_linescore(CWBoxscore *boxscore)
{
  int i;

  printf("  <linescore "
	 "away_runs=\"%d\" away_hits=\"%d\" away_errors=\"%d\" "
	 "home_runs=\"%d\" home_hits=\"%d\" home_errors=\"%d\">\n",
	 boxscore->score[0], boxscore->hits[0], boxscore->errors[0],
	 boxscore->score[1], boxscore->hits[1], boxscore->errors[1]);

  for (i = 1; i < 50; i++) {
    if (boxscore->linescore[i][0] < 0 &&
	boxscore->linescore[i][1] < 0) {
      break;
    }

    if (boxscore->linescore[i][1] >= 0) {
      printf("    <inning_line_score away=\"%d\" home=\"%d\" inning=\"%d\"/>\n",
	     boxscore->linescore[i][0], boxscore->linescore[i][1], i);
    }
    else {
      printf("    <inning_line_score away=\"%d\" home=\"x\" inning=\"%d\"/>\n",
	     boxscore->linescore[i][0],  i);
    }
  }
  printf("  </linescore>\n");
}

/*
 * Output one player's boxscore entry
 */
void
cwbox_xml_player(CWGame *game,
		 CWBoxPlayer *player, int slot, int seq, CWRoster *roster)
{
  int pos;

  CWPlayer *bio = cw_roster_player_find(roster, player->player_id);
  printf("    <player id=\"%s\" lname=\"%s\" fname=\"%s\" "
	 "slot=\"%d\" seq=\"%d\" ", 
	 player->player_id, 
	 (bio) ? bio->last_name : "", (bio) ? bio->first_name: "", slot, seq);
  
  printf("pos=\"");
  if (player->ph_inn > 0 && player->positions[0] != 11) {
    printf("h");
  }
  else if (player->pr_inn > 0 && player->positions[0] != 12) {
    printf("r");
  }
  for (pos = 0; pos < player->num_positions; pos++) {
    if (player->positions[pos] == 10) {
      printf("d");
    }
    else if (player->positions[pos] == 11) {
      printf("h");
    }
    else if (player->positions[pos] == 12) {
      printf("r");
    }
    else {
      printf("%d", player->positions[pos]);
    }
  }
  printf("\" ");

  if (player->ph_inn > 0) {
    printf("ph_inning=\"%d\" ", player->ph_inn);
  }
  else if (player->pr_inn > 0) {
    printf("pr_inning=\"%d\" ", player->pr_inn);
  }

  printf(">\n");

  if (slot > 0) {
    printf("      <batting ab=\"%d\" r=\"%d\" h=\"%d\" d=\"%d\" "
	   "t=\"%d\" hr=\"%d\" bi=\"%d\" bi2out=\"%d\" ",
	   player->batting->ab, player->batting->r,
	   player->batting->h, player->batting->b2,
	   player->batting->b3, player->batting->hr, 
	   player->batting->bi, player->batting->bi2out);
    printf("bb=\"%d\" ibb=\"%d\" so=\"%d\" gdp=\"%d\" "
	   "hp=\"%d\" sh=\"%d\" sf=\"%d\" ",
	   player->batting->bb, player->batting->ibb,
	   player->batting->so, player->batting->gdp,
	   player->batting->hp, player->batting->sh, player->batting->sf);
    printf("sb=\"%d\" cs=\"%d\" ",
	   player->batting->sb, player->batting->cs);
    if (cw_game_info_lookup(game, "gwrbi") &&
	!strcmp(player->player_id, cw_game_info_lookup(game, "gwrbi"))) {
      printf("gwrbi=\"1\" ");
    }
    printf("/>\n");
  }


  for (pos = 1; pos < 10; pos++) {
    if (player->fielding[pos] == NULL) {
      continue;
    }

    printf("      <fielding pos=\"%d\" ", pos);
    printf("outs=\"%d\" po=\"%d\" a=\"%d\" e=\"%d\" dp=\"%d\" tp=\"%d\" ",
	   player->fielding[pos]->outs, player->fielding[pos]->po,
	   player->fielding[pos]->a, player->fielding[pos]->e,
	   player->fielding[pos]->dp, player->fielding[pos]->tp);
    if (player->positions[pos] == 2) {
      printf("pb=\"%d\" ", player->fielding[pos]->pb);
    }
    printf("bip=\"%d\" bf=\"%d\" ",
	   player->fielding[pos]->bip, player->fielding[pos]->bf);
    printf("/>\n");
  }
  printf("    </player>\n");
}


/*
 * Output the boxscore entries for players on team 't'
 */
void
cwbox_xml_batting(CWGame *game, CWBoxscore *boxscore, int t, CWRoster *roster)
{
  int slot;

  printf("  <players team=\"%s\" lob=\"%d\" dp=\"%d\" tp=\"%d\" risp_ab=\"%d\" risp_h=\"%d\">\n", 
	 roster->team_id, boxscore->lob[t], boxscore->dp[t], boxscore->tp[t],
	 boxscore->risp_ab[t], boxscore->risp_h[t]);

  for (slot = 0; slot <= 9; slot++) {
    CWBoxPlayer *player = cw_box_get_starter(boxscore, t, slot);
    int seq = 1;
    
    while (player != NULL) {
      cwbox_xml_player(game, player, slot, seq++, roster);
      player = player->next;
    }
  }

  printf("  </players>\n");
}

/*
 * Output the pitching lines for team 't'
 */
void
cwbox_xml_pitching(CWGame *game, CWBoxscore *boxscore, int t, CWRoster *roster)
{
  CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(boxscore, t);
  int seq = 1;

  printf("  <pitching team=\"%s\">\n", roster->team_id);

  while (pitcher != NULL) {
    CWPlayer *bio = cw_roster_player_find(roster, pitcher->player_id);
    printf("    <pitcher id=\"%s\" lname=\"%s\" fname=\"%s\" ",
	   pitcher->player_id, (bio) ? bio->last_name : "", 
	   (bio) ? bio->first_name : "");
    /* FIXME: A pitcher gets a shutout if he records all outs for a team,
     * even if not the starting pitcher! */
    printf("gs=\"%d\" cg=\"%d\" sho=\"%d\" gf=\"%d\" ",
	   (pitcher->prev == NULL) ? 1 : 0,
	   (pitcher->prev == NULL && 
	    pitcher->next == NULL) ? 1 : 0,
	   (pitcher->prev == NULL && pitcher->next == NULL &&
	    pitcher->pitching->r == 0) ? 1 : 0,
	   (pitcher->prev != NULL && 
	    pitcher->next == NULL) ? 1 : 0);
    printf("outs=\"%d\" ab=\"%d\" bf=\"%d\" h=\"%d\" r=\"%d\" "
	   "er=\"%d\" hr=\"%d\" ",
	   pitcher->pitching->outs, pitcher->pitching->ab,
	   pitcher->pitching->bf,
	   pitcher->pitching->h, pitcher->pitching->r,
	   pitcher->pitching->er, pitcher->pitching->hr);
    printf("bb=\"%d\" ibb=\"%d\" so=\"%d\" wp=\"%d\" "
	   "bk=\"%d\" hb=\"%d\" ",
	   pitcher->pitching->bb, pitcher->pitching->ibb,
	   pitcher->pitching->so, pitcher->pitching->wp,
	   pitcher->pitching->bk, pitcher->pitching->hb);
    printf("gb=\"%d\" fb=\"%d\" ",
	   pitcher->pitching->gb, pitcher->pitching->fb);
    
    if (pitcher->pitching->xbinn > 0) {
      printf("xb=\"%d\" xbinn=\"%d\" ",
	     pitcher->pitching->xb, pitcher->pitching->xbinn);
    }

    if (cw_game_info_lookup(game, "pitches") &&
      !strcmp(cw_game_info_lookup(game, "pitches"), "pitches")) {
      printf("pitch=\"%d\" strike=\"%d\" ",
	     pitcher->pitching->pitches, pitcher->pitching->strikes);
    }

    if (cw_game_info_lookup(game, "wp") &&
	!strcmp(pitcher->player_id, cw_game_info_lookup(game, "wp"))) {
      printf("dec=\"W\" ");
    }
    else if (cw_game_info_lookup(game, "lp") &&
	     !strcmp(pitcher->player_id, cw_game_info_lookup(game, "lp"))) {
      printf("dec=\"L\" ");
    }
    else if (cw_game_info_lookup(game, "save") &&
	     !strcmp(pitcher->player_id, cw_game_info_lookup(game, "save"))) {
      printf("dec=\"S\" ");
    }


    printf("/>\n");
    seq++;
    pitcher = pitcher->next;
  }
  

  printf("  </pitching>\n");
}

/*
 * Generic output for batting event entries
 */
void
cwbox_xml_batting_events(CWBoxEvent *list, char *mainlabel, char *itemlabel)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <%s>\n", mainlabel);
  while (event != NULL) {
    printf("    <%s batter=\"%s\" pitcher=\"%s\" "
	   "inning=\"%d\" half=\"%d\"/>\n", 
	   itemlabel, event->players[0], event->players[1], 
	   event->inning, event->half_inning);
    event = event->next;
  }
  printf("  </%s>\n", mainlabel);
}

/*
 * Specialized output for home run events
 */
void
cwbox_xml_homeruns(CWBoxEvent *list)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <homeruns>\n");
  while (event != NULL) {
    printf("    <homerun batter=\"%s\" pitcher=\"%s\" "
	   "inning=\"%d\" half=\"%d\" "
	   "runners=\"%d\" outs=\"%d\" location=\"%s\"/>\n", 
	   event->players[0], event->players[1], 
	   event->inning, event->half_inning,
	   event->runners, event->outs, event->location);
    event = event->next;
  }
  printf("  </homeruns>\n");
}

/*
 * Generic output for stolen base events
 */
void
cwbox_xml_steal_events(CWBoxEvent *list, char *mainlabel, char *itemlabel)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <%s>\n", mainlabel);
  while (event != NULL) {
    printf("    <%s runner=\"%s\" pitcher=\"%s\" catcher=\"%s\" "
	   "inning=\"%d\" half=\"%d\" base=\"%d\" pickoff=\"%d\"/>\n", 
	   itemlabel, event->players[0], event->players[1], 
	   (event->players[2]) ? event->players[2] : "", 
	   event->inning, event->half_inning,
	   (event->runners >= 0) ? (event->runners + 1) : -1, 
	   event->pickoff);
    event = event->next;
  }
  printf("  </%s>\n", mainlabel);
}

/*
 * Output pickoff events
 */
void
cwbox_xml_pickoff_events(CWBoxEvent *list)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <pickoffs>\n");
  while (event != NULL) {
    printf("    <pickoff runner=\"%s\" fielder=\"%s\" "
	   "inning=\"%d\" half=\"%d\" base=\"%d\" pickoff=\"%d\"/>\n", 
	   event->players[0], event->players[1], 
	   event->inning, event->half_inning, event->runners,
	   event->pickoff);
    event = event->next;
  }
  printf("  </pickoffs>\n");
}

/*
 * Output wild pitch events
 */
void
cwbox_xml_wildpitch_events(CWBoxEvent *list)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <wildpitches>\n");
  while (event != NULL) {
    printf("    <wildpitch pitcher=\"%s\" catcher=\"%s\" "
	   "inning=\"%d\" half=\"%d\"/>\n",
	   event->players[0], event->players[1], 
	   event->inning, event->half_inning);
    event = event->next;
  }
  printf("  </wildpitches>\n");
}

/*
 * Output passed ball events
 */
void
cwbox_xml_passedball_events(CWBoxEvent *list)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <passedballs>\n");
  while (event != NULL) {
    printf("    <passedball pitcher=\"%s\" catcher=\"%s\" "
	   "inning=\"%d\" half=\"%d\"/>\n",
	   event->players[0], event->players[1], 
	   event->inning, event->half_inning);
    event = event->next;
  }
  printf("  </passedballs>\n");
}

/*
 * Output double plays and triple plays
 */
void
cwbox_xml_multiplay_events(CWBoxEvent *list, char *mainlabel, char *itemlabel)
{
  CWBoxEvent *event = list;
  if (event == NULL) {
    return;
  }
  
  printf("  <%s>\n", mainlabel);
  while (event != NULL) {
    int i; 

    printf("    <%s inning=\"%d\" half=\"%d\" ", itemlabel, 
	   event->inning, event->half_inning);
    for (i = 0; event->players[i] != NULL; i++) {
      printf("player%d=\"%s\" ", i+1, event->players[i]);
    }
    printf("/>\n");
    event = event->next;
  }
  printf("  </%s>\n", mainlabel);
}

/*
 * Outputs the boxscore in XML format
 */
void 
cwbox_print_xml(CWGame *game, CWBoxscore *boxscore, 
		CWRoster *visitors, CWRoster *home)
{
  printf("<boxscore game_id=\"%s\" date=\"%s\" site=\"%s\" "
	 "visitor=\"%s\" visitor_city=\"%s\" visitor_name=\"%s\" "
	 "home=\"%s\" home_city=\"%s\" home_name=\"%s\" ",
	 game->game_id, 
	 cw_game_info_lookup(game, "date"), cw_game_info_lookup(game, "site"),
	 visitors->team_id, visitors->city, visitors->nickname,
	 home->team_id, home->city, home->nickname);

  printf("start_time=\"%s\" day_night=\"%s\" "
	 "temperature=\"%s\" wind_direction=\"%s\" wind_speed=\"%s\" "
	 "field_condition=\"%s\" precip=\"%s\" sky=\"%s\" "
	 "time_of_game=\"%s\" attendance=\"%s\" ",
	 cw_game_info_lookup(game, "starttime"),
	 cw_game_info_lookup(game, "daynight"),
	 cw_game_info_lookup(game, "temp"),
	 cw_game_info_lookup(game, "winddir"),
	 cw_game_info_lookup(game, "windspeed"),
	 cw_game_info_lookup(game, "fieldcond"),
	 cw_game_info_lookup(game, "precip"),
	 cw_game_info_lookup(game, "sky"),
	 cw_game_info_lookup(game, "timeofgame"),
	 cw_game_info_lookup(game, "attendance"));

  if (cw_game_info_lookup(game, "umphome")) {
    printf("umpire_hp=\"%s\" ", cw_game_info_lookup(game, "umphome"));
  }
  if (cw_game_info_lookup(game, "ump1b")) {
    printf("umpire_1b=\"%s\" ", cw_game_info_lookup(game, "ump1b"));
  }
  if (cw_game_info_lookup(game, "ump2b")) {
    printf("umpire_2b=\"%s\" ", cw_game_info_lookup(game, "ump2b"));
  }
  if (cw_game_info_lookup(game, "ump3b")) {
    printf("umpire_3b=\"%s\" ", cw_game_info_lookup(game, "ump3b"));
  }
  if (cw_game_info_lookup(game, "umplf")) {
    printf("umpire_lf=\"%s\" ", cw_game_info_lookup(game, "umplf"));
  }
  if (cw_game_info_lookup(game, "umprf")) {
    printf("umpire_rf=\"%s\" ", cw_game_info_lookup(game, "umprf"));
  }

  if (boxscore->outs_at_end != 3) {
    printf("walk_off=\"%d\" outs_at_end=\"%d\" ",
	   boxscore->walk_off, boxscore->outs_at_end);
  }

  if (cw_game_info_lookup(game, "htbf") &&
      !strcmp(cw_game_info_lookup(game, "htbf"), "true")) {
    printf("htbf=\"1\" ");
  }

  printf(">\n");

  cwbox_xml_linescore(boxscore);

  cwbox_xml_batting(game, boxscore, 0, visitors);
  cwbox_xml_batting(game, boxscore, 1, home);
  cwbox_xml_pitching(game, boxscore, 0, visitors);
  cwbox_xml_pitching(game, boxscore, 1, home);

  cwbox_xml_batting_events(boxscore->b2_list, "doubles", "double");
  cwbox_xml_batting_events(boxscore->b3_list, "triples", "triple");
  cwbox_xml_homeruns(boxscore->hr_list);
  cwbox_xml_batting_events(boxscore->ibb_list, 
			   "intentionalwalks", "intentionalwalk");
  cwbox_xml_batting_events(boxscore->hp_list, "hitbypitches", "hitbypitch");
  cwbox_xml_batting_events(boxscore->sh_list, "sacbunts", "sacbunt");
  cwbox_xml_batting_events(boxscore->sf_list, "sacflies", "sacfly");

  cwbox_xml_steal_events(boxscore->sb_list, "stolenbases", "stolenbase");
  cwbox_xml_steal_events(boxscore->cs_list, "caughtstealings", 
			 "caughtstealing");
  cwbox_xml_pickoff_events(boxscore->po_list);

  cwbox_xml_wildpitch_events(boxscore->wp_list);
  cwbox_xml_passedball_events(boxscore->pb_list);
  cwbox_xml_multiplay_events(boxscore->dp_list, "doubleplays", "doubleplay");
  cwbox_xml_multiplay_events(boxscore->tp_list, "tripleplays", "tripleplay");
    
  printf("</boxscore>\n\n");
}


