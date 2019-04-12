/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *                          Sean Forman, Sports Reference LLC
 *                          XML Team Solutions, Inc.
 *
 * FILE: src/cwtools/cwbox.c
 * Chadwick boxscore generator program
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
#include "xmlwrite.h"

/*************************************************************************
 * Global variables for command-line options
 *************************************************************************/

int max_field = 0;
int fields[] = { 0 };

int use_xml = 0;
int use_sportsml = 0;

char program_name[] = "cwbox";

extern char year[], first_date[], last_date[], game_id[];
extern int ascii;
extern int quiet;

XMLDoc *doc = NULL;

void
cwbox_print_field_list(void)
{
  exit(0);
}

void (*cwtools_print_field_list)(void) = cwbox_print_field_list;

void
cwbox_print_welcome_message(char *argv0)
{
  fprintf(stderr, "\nChadwick boxscore generator, version " VERSION);
  fprintf(stderr, "\n  Type '%s -h' for help.\n", argv0);
  fprintf(stderr, "Copyright (c) 2002-2019\nDr T L Turocy, Chadwick Baseball Bureau (ted.turocy@gmail.com)\n");
  fprintf(stderr, "This is free software, "
	  "subject to the terms of the GNU GPL license.\n\n");
}

void (*cwtools_print_welcome_message)(char *) = cwbox_print_welcome_message;

void cwbox_print_help(void)
{
  fprintf(stderr, "\n\ncwbox generates boxscores from play-by-play files\n");
  fprintf(stderr, "Usage: cwbox [options] eventfile...\n");
  fprintf(stderr, "options:\n");
  fprintf(stderr, "  -h        print this help\n");
  fprintf(stderr, "  -i id     only process game given by id\n");
  fprintf(stderr, "  -y year   Year to process (for teamyyyy and aaayyyy.ros).\n");
  fprintf(stderr, "  -s start  Earliest date to process (mmdd).\n");
  fprintf(stderr, "  -e end    Last date to process (mmdd).\n");
  fprintf(stderr, "  -X        output boxscores as XML.\n");
  fprintf(stderr, "  -S        output boxscores as SportsML.\n");
  fprintf(stderr, "  -q        operate quietly; do not output progress messages\n");
  exit(0);
}

void (*cwtools_print_help)(void) = cwbox_print_help;


int
cwbox_parse_command_line(int argc, char *argv[])
{
  int i;
  strcpy(year, "");

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-a")) {
      ascii = 1;
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
    /* This part is cwbox-specific */
    else if (!strcmp(argv[i], "-X")) {
      use_xml = 1;
    }      
    else if (!strcmp(argv[i], "-S")) {
      use_sportsml = 1;
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

int (*cwtools_parse_command_line)(int, char *argv[]) = cwbox_parse_command_line;

/*
 * Output the header for the boxscore (teams, date)
 */
void
cwbox_print_header(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  int month, day, year;
  sscanf(cw_game_info_lookup(game, "date"), "%d/%d/%d",
	 &year, &month, &day);
  if (!strcmp(cw_game_info_lookup(game, "number"), "0")) {
    printf("     Game of %d/%d/%d -- %s at %s",
	   month, day, year, 
	   (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	   (home) ? home->city : cw_game_info_lookup(game, "hometeam"));
  }
  else {
    printf("     Game of %d/%d/%d, game %s -- %s at %s",
	   month, day, year, cw_game_info_lookup(game, "number"),
	   (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	   (home) ? home->city : cw_game_info_lookup(game, "hometeam"));
  }
  
  if (cw_game_info_lookup(game, "daynight")) {
    if (!strcmp(cw_game_info_lookup(game, "daynight"), "day")) {
      printf(" (D)\n");
    }
    else if (!strcmp(cw_game_info_lookup(game, "daynight"), "night")) {
      printf(" (N)\n");
    }
    else {
      printf("\n");
    }
  }
  else {
    printf("\n");
  }

  printf("\n");
}

char positions[][3] = {
  "", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf",
  "dh", "ph", "pr"
};

/*
 * Output one player's batting line
 */
void
cwbox_print_player(CWBoxPlayer *player, CWRoster *roster)
{
  CWPlayer *bio = NULL;
  char name[256], posstr[256], outstr[256];
  int pos;

  if (roster) {
    bio = cw_roster_player_find(roster, player->player_id);
  }
  if (bio) {
    sprintf(name, "%s %c", bio->last_name, bio->first_name[0]);
  }
  else {
    sprintf(name, "%s", player->name);
  }

  if (player->ph_inn > 0 && player->positions[0] != 11) {
    strcpy(posstr, "ph");
  }
  else if (player->pr_inn > 0 && player->positions[0] != 12) {
    strcpy(posstr, "pr");
  }
  else {
    strcpy(posstr, "");
  }
  for (pos = 0; pos < player->num_positions; pos++) {
    if (strlen(posstr) > 0) {
      strcat(posstr, "-");
    }
    strcat(posstr, positions[player->positions[pos]]);
  }

  if (strlen(posstr) <= 10) {
    if (strlen(posstr) + strlen(name) > 18) {
      strncpy(outstr, name, 18 - strlen(posstr));
      strcpy(outstr + 18 - strlen(posstr), ", ");
    }
    else {
      strcpy(outstr, name);
      strcat(outstr, ", ");
    }
    strcat(outstr, posstr);
  }
  else {
    /* When there are a lot of positions, can't do much sensibly... */
    strcpy(outstr, name);
    strcat(outstr, ", ");
    strcat(outstr, positions[player->positions[0]]);
    strcat(outstr, ",...");
  }

  if (player->batting->bi != -1) {
    printf("%-20s %2d %2d %2d %2d",
	   outstr, player->batting->ab, player->batting->r,
	   player->batting->h, player->batting->bi);
  }
  else {
    printf("%-20s %2d %2d %2d   ",
	   outstr, player->batting->ab, player->batting->r,
	   player->batting->h);
  }
}

/*
 * Output one pitcher's pitching line.
 * The parameter 'note_count' keeps track of how many apparatus notes
 * have been emitted (for pitchers who do not record an out in an inning)
 */
void
cwbox_print_pitcher(CWGame *game,
		    CWBoxPitcher *pitcher, CWRoster *roster,
		    int *note_count)
{
  char *markers[] = { "*", "+", "#", "**", "++", "##" };
  CWPlayer *bio = NULL;
  char name[256];

  if (roster) {
    bio = cw_roster_player_find(roster, pitcher->player_id);
  }
  if (bio) {
    sprintf(name, "%s %c", bio->last_name, bio->first_name[0]);
  }
  else {
    sprintf(name, "%s", pitcher->name);
  }

  if (cw_game_info_lookup(game, "wp") &&
      !strcmp(cw_game_info_lookup(game, "wp"), pitcher->player_id)) {
    strcat(name, " (W)");
  }
  else if (cw_game_info_lookup(game, "lp") &&
	   !strcmp(cw_game_info_lookup(game, "lp"), pitcher->player_id)) {
    strcat(name, " (L)");
  }
  else if (cw_game_info_lookup(game, "save") &&
	   !strcmp(cw_game_info_lookup(game, "save"), pitcher->player_id)) {
    strcat(name, " (S)");
  }

  if (pitcher->pitching->xbinn > 0 && pitcher->pitching->xb > 0) {
    strcat(name, markers[(*note_count)++]);
  }

  printf("%-20s %2d.%1d %2d %2d",
	 name, pitcher->pitching->outs / 3, pitcher->pitching->outs % 3,
	 pitcher->pitching->h, pitcher->pitching->r);

  if (pitcher->pitching->er != -1) {
    printf(" %2d", pitcher->pitching->er);
  }
  else {
    printf("   ");
  }

  if (pitcher->pitching->bb != -1) {
    printf(" %2d", pitcher->pitching->bb);
  }
  else {
    printf("   ");
  }
  
  if (pitcher->pitching->so != -1) {
    printf(" %2d\n", pitcher->pitching->so);
  }
  else {
    printf("   \n");
  }
}

/*
 * Derive a player name from an appearance record in a game.
 * Used when roster file is not available.
 */
char *
cwbox_game_find_name(CWGame *game, char *player_id)
{
  CWAppearance *app;
  CWEvent *event;

  for (app = game->first_starter; app != NULL; app = app->next) {
    if (!strcmp(app->player_id, player_id)) {
      return app->name;
    }
  }

  for (event = game->first_event; event != NULL; event = event->next) {
    for (app = event->first_sub; app != NULL; app = app->next) {
      if (!strcmp(app->player_id, player_id)) {
	return app->name;
      }
    }
  }
  return NULL;
}

/*
 * Output the pitching apparatus (listing pitchers who do not record an
 * out in an inning).
 */
void
cwbox_print_pitcher_apparatus(CWBoxscore *boxscore)
{
  int t, count = 0;
  char *markers[] = { "*", "+", "#", "**", "++", "##" };

  for (t = 0; t <= 1; t++) {
    CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(boxscore, t);
    while (pitcher != NULL) {
      if (pitcher->pitching->xbinn > 0 && pitcher->pitching->xb > 0) {
	printf("  %s Pitched to %d batter%s in %d", 
	       markers[count++],
	       pitcher->pitching->xb,
	       (pitcher->pitching->xb == 1) ? "" : "s",
	       pitcher->pitching->xbinn);
	if (pitcher->pitching->xbinn % 10 == 1 &&
	    pitcher->pitching->xbinn != 11) {
	  printf("st\n");
	}
	else if (pitcher->pitching->xbinn % 10 == 2 &&
		 pitcher->pitching->xbinn != 12) {
	  printf("nd\n");
	}
	else if (pitcher->pitching->xbinn % 10 == 3 &&
		 pitcher->pitching->xbinn != 13) {
	  printf("rd\n");
	}
	else {
	  printf("th\n");
	}
      }
      pitcher = pitcher->next;
    }
  }
}

/*
 * Output the linescore and total runs for both teams
 */
void
cwbox_print_linescore(CWGame *game, CWBoxscore *boxscore,
		      CWRoster *visitors, CWRoster *home)
{
  int i, t;

  for (t = 0; t <= 1; t++) {
    int runs = 0;
    
    if (t == 0)  {
      printf("%-17s", (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"));
    }
    else {
      printf("%-17s", (home) ? home->city : cw_game_info_lookup(game, "hometeam"));
    }
    for (i = 1; i < 50; i++) {
      if (boxscore->linescore[i][0] < 0 &&
	  boxscore->linescore[i][1] < 0) {
	break;
      }

      if (boxscore->linescore[i][t] >= 10) {
	printf("(%d)", boxscore->linescore[i][t]);
	runs += boxscore->linescore[i][t];
      }
      else if (boxscore->linescore[i][t] >= 0) {
	printf("%d", boxscore->linescore[i][t]);
	runs += boxscore->linescore[i][t];
      }
      else {
	printf("x");
      }

      if (i % 3 == 0) {
	printf(" ");
      }
    }

    if ((i - 1) % 3 != 0) {
      printf(" ");
    }

    printf("-- %2d\n", runs);
  }

  if (boxscore->outs_at_end != 3) {
    if (!boxscore->walk_off) {
      printf("  %d out%s when game ended.\n", 
	     boxscore->outs_at_end, (boxscore->outs_at_end == 1) ? "" : "s");
    }
    else {
      printf("  %d out%s when winning run was scored.\n", 
	     boxscore->outs_at_end, (boxscore->outs_at_end == 1) ? "" : "s");
    }
  }
}

/*
 * Output the count of double plays by team
 */
void
cwbox_print_double_play(CWGame *game, CWBoxscore *boxscore, 
			CWRoster *visitors, CWRoster *home)
{
  if (boxscore->dp[0] == 0 && boxscore->dp[1] == 0) {
    return;
  }

  printf("DP -- ");
  if (boxscore->dp[0] > 0 && boxscore->dp[1] == 0) {
    printf("%s %d\n", 
	   (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	   boxscore->dp[0]);
  }
  else if (boxscore->dp[0] == 0 && boxscore->dp[1] > 0) {
    printf("%s %d\n",
	   (home) ? home->city : cw_game_info_lookup(game, "hometeam"),
	   boxscore->dp[1]);
  }
  else {
    printf("%s %d, %s %d\n",
	   (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	   boxscore->dp[0],
	   (home) ? home->city : cw_game_info_lookup(game, "hometeam"),
	   boxscore->dp[1]);
  }
}

/*
 * Output the count of triple plays by team
 */
void
cwbox_print_triple_play(CWGame *game, CWBoxscore *boxscore, 
			CWRoster *visitors, CWRoster *home)
{
  if (boxscore->tp[0] == 0 && boxscore->tp[1] == 0) {
    return;
  }

  printf("TP -- ");
  if (boxscore->tp[0] > 0 && boxscore->tp[1] == 0) {
    printf("%s %d\n", 
	   (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	   boxscore->tp[0]);
  }
  else if (boxscore->tp[0] == 0 && boxscore->tp[1] > 0) {
    printf("%s %d\n",
	   (home) ? home->city : cw_game_info_lookup(game, "hometeam"),
	   boxscore->tp[1]);
  }
  else {
    printf("%s %d, %s %d\n",
	   (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	   boxscore->tp[0],
	   (home) ? home->city : cw_game_info_lookup(game, "hometeam"),
	   boxscore->tp[1]);
  }
}

/*
 * Output the number of runners left on base
 */
void
cwbox_print_lob(CWGame *game, CWBoxscore *boxscore, 
		CWRoster *visitors, CWRoster *home)
{
  if (boxscore->lob[0] == 0 && boxscore->lob[1] == 0) {
    return;
  }

  printf("LOB -- %s %d, %s %d\n",
	 (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	 boxscore->lob[0],
	 (home) ? home->city : cw_game_info_lookup(game, "hometeam"),
	 boxscore->lob[1]);
}


/*
 * Generic output for list of events (2B, 3B, WP, etc.)
 */
void 
cwbox_print_player_apparatus(CWGame *game, CWBoxEvent *list, int index, char *label, 
			     CWRoster *visitors, CWRoster *home) 
{ 
  CWBoxEvent *event = list; 
  int comma = 0; 
  if (list == NULL) { 
    return; 
  } 
  printf("%s -- ", label); 
  while (event != NULL) { 
    CWBoxEvent *search_event = event; 
    CWPlayer *bio = NULL; 
    char *name = NULL;
    int count = 0; 
    if (event->mark > 0) { 
      event = event->next; 
      continue; 
    } 
    while (search_event != NULL) { 
      if (!strcmp(event->players[index], search_event->players[index])) { 
	count++; 
	search_event->mark = 1; 
      } 
      search_event = search_event->next; 
    } 
    if (visitors) {
      bio = cw_roster_player_find(visitors, event->players[index]); 
    }
    if (!bio && home) { 
      bio = cw_roster_player_find(home, event->players[index]); 
    } 
    if (!bio) {
      name = cwbox_game_find_name(game, event->players[index]);
    }      
    if (comma) { 
      printf(", "); 
    } 
    if (count == 1) { 
      if (bio) {
	printf("%s %c", bio->last_name, bio->first_name[0]); 
      }
      else if (name) {
	printf("%s", name);
      }
      else {
	printf("%s", event->players[index]);
      }
    } 
    else { 
      if (bio) {
	printf("%s %c %d", bio->last_name, bio->first_name[0], count); 
      }
      else if (name) {
	printf("%s %d", name, count);
      }
      else {
	printf("%s %d", event->players[index], count);
      }
    } 
    comma = 1; 
  } 
  printf("\n"); 
  event = list; 
  while (event != NULL) { 
    event->mark = 0; 
    event = event->next; 
  } 
}

/*
 * Output HBP apparatus
 */
void 
cwbox_print_hbp_apparatus(CWGame *game, CWBoxEvent *list,  
			  CWRoster *visitors, CWRoster *home) 
{ 
  CWBoxEvent *event = list; 
  int comma = 0; 
  if (list == NULL) { 
    return; 
  } 
  printf("HBP -- ");
  while (event != NULL) { 
    CWBoxEvent *search_event = event; 
    CWPlayer *batter = NULL, *pitcher = NULL; 
    char *batter_name, *pitcher_name;
    int count = 0; 
    if (event->mark > 0) { 
      event = event->next; 
      continue; 
    } 
    while (search_event != NULL) { 
      if (!strcmp(event->players[0], search_event->players[0]) &&
	  !strcmp(event->players[1], search_event->players[1])) { 
	count++; 
	search_event->mark = 1; 
      } 
      search_event = search_event->next; 
    } 
    if (visitors) {
      batter = cw_roster_player_find(visitors, event->players[0]); 
    }
    if (!batter && home) { 
      batter = cw_roster_player_find(home, event->players[0]); 
    } 
    if (!batter) {
      batter_name = cwbox_game_find_name(game, event->players[0]);
    }
    
    if (visitors) {
      pitcher = cw_roster_player_find(visitors, event->players[1]); 
    }
    if (!pitcher && home) { 
      pitcher = cw_roster_player_find(home, event->players[1]); 
    } 
    if (!pitcher) {
      pitcher_name = cwbox_game_find_name(game, event->players[1]);
    }

    if (comma) { 
      printf(", "); 
    } 
    if (count == 1) { 
      if (pitcher) {
	printf("by %s %c ", pitcher->last_name, pitcher->first_name[0]);
      }
      else if (pitcher_name) {
	printf("by %s ", pitcher_name);
      }
      else {
	printf("by %s ", event->players[1]);
      }
      if (batter) {
	printf("(%s %c)", batter->last_name, batter->first_name[0]);
      }
      else if (batter_name) {
	printf("(%s)", batter_name);
      }
      else {
	printf("(%s)", event->players[0]);
      }
    } 
    else { 
      if (pitcher) {
	printf("by %s %c ", pitcher->last_name, pitcher->first_name[0]);
      }
      else if (pitcher_name) {
	printf("by %s ", pitcher_name);
      }
      else {
	printf("by %s ", event->players[1]);
      }
      if (batter) {
	printf("(%s %c)", batter->last_name, batter->first_name[0]);
      }
      else if (batter_name) {
	printf("(%s)", batter_name);
      }
      else {
	printf("(%s)", event->players[0]);
      }
      printf(" %d", count);
    } 
    comma = 1; 
  } 
  printf("\n"); 
  event = list; 
  while (event != NULL) { 
    event->mark = 0; 
    event = event->next; 
  } 
}

/*
 * Output the time of game
 */
void
cwbox_print_timeofgame(CWGame *game)
{
  int timeofgame;
  if (cw_game_info_lookup(game, "timeofgame") &&
      sscanf(cw_game_info_lookup(game, "timeofgame"), "%d", &timeofgame) &&
      timeofgame > 0) {
    printf("T -- %d:%02d\n", timeofgame / 60, timeofgame % 60);
  }
}

/*
 * Output the attendance
 */
void
cwbox_print_attendance(CWGame *game)
{
  printf("A -- %s\n", cw_game_info_lookup(game, "attendance"));
}

/*
 * Output the apparatus (the list of events and other miscellaneous
 * game information).
 */
void
cwbox_print_apparatus(CWGame *game, CWBoxscore *boxscore, 
		      CWRoster *visitors, CWRoster *home)
{
  cwbox_print_player_apparatus(game, boxscore->err_list, 0, "E", visitors, home);
  cwbox_print_double_play(game, boxscore, visitors, home);
  cwbox_print_triple_play(game, boxscore, visitors, home);
  cwbox_print_lob(game, boxscore, visitors, home);
  cwbox_print_player_apparatus(game, boxscore->b2_list, 0, "2B", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->b3_list, 0, "3B", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->hr_list, 0, "HR", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->sb_list, 0, "SB", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->cs_list, 0, "CS", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->sh_list, 0, "SH", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->sf_list, 0, "SF", visitors, home);
  cwbox_print_hbp_apparatus(game, boxscore->hp_list, visitors, home);
  cwbox_print_player_apparatus(game, boxscore->wp_list, 0, "WP", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->bk_list, 0, "Balk", visitors, home);
  cwbox_print_player_apparatus(game, boxscore->pb_list, 1, "PB", visitors, home);
  cwbox_print_timeofgame(game);
  cwbox_print_attendance(game);
}

/*
 * Output the boxscore in plaintext
 */
void
cwbox_print_text(CWGame *game, CWBoxscore *boxscore,
		 CWRoster *visitors, CWRoster *home)
{
  int t, note_count = 0;
  int slots[2] = { 1, 1 };
  CWBoxPlayer *players[2] = { 0, 0 };
  int ab[2] = { 0, 0 }, r[2] = { 0, 0 }, h[2] = { 0, 0 }, bi[2] = { 0, 0 };

  players[0] = cw_box_get_starter(boxscore, 0, 1);
  players[1] = cw_box_get_starter(boxscore, 1, 1);

  cwbox_print_header(game, visitors, home);

  printf("  %-18s AB  R  H RBI    %-18s AB  R  H RBI\n",
	 (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"),
	 (home) ? home->city : cw_game_info_lookup(game, "hometeam"));

  while (slots[0] <= 9 || slots[1] <= 9) {
    for (t = 0; t <= 1; t++) {
      if (slots[t] <= 9) {
	cwbox_print_player(players[t], (t == 0) ? visitors : home);
	ab[t] += players[t]->batting->ab;
	r[t] += players[t]->batting->r;
	h[t] += players[t]->batting->h;
	if (players[t]->batting->bi != -1) {
	  bi[t] += players[t]->batting->bi;
	}
	else {
	  bi[t] = -1;
	}
	players[t] = players[t]->next;
	if (players[t] == NULL) {
	  /* In some National Association games, teams played with 8
	   * players.  This generalization allows for printing
	   * boxscores when some batting slots are empty.
	   */
	  while (slots[t] <= 9 && players[t] == NULL) {
	    slots[t]++;
	    if (slots[t] <= 9) {
	      players[t] = cw_box_get_starter(boxscore, t, slots[t]);
	    }
	  }
	}
      }
      else {
	printf("%-32s", "");
      }

      printf("   ");
    }
    printf("\n");
  }

  printf("%-20s -- -- -- -- %-22s -- -- -- --\n", "", "");

  if (bi[0] == -1 || bi[1] == -1) {
    printf("%-20s %2d %2d %2d    %-22s %2d %2d %2d   \n",
	   "", ab[0], r[0], h[0], "", ab[1], r[1], h[1]);
  }
  else {
    printf("%-20s %2d %2d %2d %2d %-22s %2d %2d %2d %2d\n",
	   "", ab[0], r[0], h[0], bi[0], "", ab[1], r[1], h[1], bi[1]);
  }
  printf("\n");

  cwbox_print_linescore(game, boxscore, visitors, home);

  printf("\n");

  for (t = 0; t <= 1; t++) {
    CWBoxPitcher *pitcher = cw_box_get_starting_pitcher(boxscore, t);
    if (t == 0) {
      printf("  %-18s   IP  H  R ER BB SO\n",
	     (visitors) ? visitors->city : cw_game_info_lookup(game, "visteam"));
    }
    else {
      printf("  %-18s   IP  H  R ER BB SO\n",
	     (home) ? home->city : cw_game_info_lookup(game, "hometeam"));
    }
    while (pitcher != NULL) {
      cwbox_print_pitcher(game, pitcher, (t == 0) ? visitors : home,
			  &note_count);
      pitcher = pitcher->next;
    }
    if (t == 0) {
      printf("\n");
    }
  }
  cwbox_print_pitcher_apparatus(boxscore);
  printf("\n");

  cwbox_print_apparatus(game, boxscore, visitors, home);

  printf("\f");

}

extern void cwbox_print_xml(CWGame *, CWBoxscore *, CWRoster *, CWRoster *);
extern void cwbox_print_sportsml(XMLDoc *doc,
				 CWGame *, CWBoxscore *, CWRoster *, CWRoster *);

void cwbox_process_game(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  CWBoxscore *boxscore = 0;

  if (!cw_game_lint(game)) {
    fprintf(stderr, "WARNING: Sanity check fails for game %s, skipping...\n",
	    game->game_id);
    return;
  }

  boxscore = cw_box_create(game);
 
  if (!visitors) {
    fprintf(stderr, "WARNING: In game %s, could not find roster for visiting team.\n",
	    game->game_id);
    /* cw_box_cleanup(boxscore);
    free(boxscore);
    return;
    */
  }

  if (!home) {
    fprintf(stderr, "WARNING: In game %s, could not find roster for home team.\n",
	    game->game_id);
    /*
    cw_box_cleanup(boxscore);
    free(boxscore);
    return;
    */
  }

  if (use_xml) {
    cwbox_print_xml(game, boxscore, visitors, home);
  }
  else if (use_sportsml) {
    cwbox_print_sportsml(doc, game, boxscore, visitors, home);
  }
  else {
    cwbox_print_text(game, boxscore, visitors, home);
  }

  cw_box_cleanup(boxscore);
  free(boxscore);
}

void (*cwtools_process_game)(CWGame *, CWRoster *, CWRoster *) = cwbox_process_game;

void
cwbox_initialize(void)
{
  if (use_sportsml) {
    doc = xml_document_create(stdout, "sports-content-set");
  }
}

void (*cwtools_initialize)(void) = cwbox_initialize;

void
cwbox_cleanup(void)
{
  if (use_sportsml) {
    xml_document_cleanup(doc);
  }
}

void (*cwtools_cleanup)(void) = cwbox_cleanup;
