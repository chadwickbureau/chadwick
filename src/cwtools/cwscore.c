/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Chadwick program for creating and editing scorebooks and games
 * 
 * This file is part of Chadwick, tools for baseball play-by-play and stats
 * Copyright (C) 2002, Ted Turocy (turocy@econ.tamu.edu)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "cwlib/chadwick.h"
 
char *cwscore_position_from_number(int position)
{
  static char positions[13][3] = {
    "", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf",
    "dh", "ph", "pr"
  };

  return positions[position];
}

/*
 * cwscore_get_line: Get a line of text
 */
void cwscore_get_line(char *buffer)
{
  fgets(buffer, 256, stdin);
  /* this gets rid of the newline character */
  buffer[strlen(buffer) - 1] = '\0';
}
 
/*
 * cwscore_get_team: Get a valid team ID from the user, return in team_id
 */
void cwscore_get_team(char *prompt, char *team_id)
{
  do {
    printf("%s\n", prompt);
    cwscore_get_line(team_id);  
    if (strlen(team_id) != 7) {
      printf("Error: team ID length is 7\n");
    }
    else {
      return;
    }
  } while (1);
}
 
CWRoster *cwscore_read_roster(char *team_id)
{
  CWRoster *roster;
  char year[5], city[4], filename[20];
  FILE *file;

  strncpy(city, team_id, 3);
  city[3] = '\0';
  strncpy(year, team_id + 3, 4);
  year[4] = '\0';
  strcpy(filename, team_id);
  strcat(filename, ".ROS");

  if ((file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Can't open roster file '%s'\n", filename);
    exit(1);
  }

  roster = cw_roster_create(city, atoi(year), '?', "", "");
  cw_roster_read(roster, file);

  return roster;
}

CWGame *cwscore_create_game(char *game_id, char *visitors, char *home)
{
  char buffer[256];
  time_t curtime;
  struct tm *loctime;
  CWGame *game = cw_game_create(game_id);
  cw_game_set_version(game, "1");
  
  cw_game_info_set(game, "inputprogvers", "Chadwick version " VERSION);
  cw_game_info_set(game, "visteam", visitors);
  cw_game_info_set(game, "hometeam", home);
  cw_game_info_set(game, "site", "");
  cw_game_info_set(game, "date", "");
  cw_game_info_set(game, "number", "0");
  cw_game_info_set(game, "starttime", "0:00PM");
  cw_game_info_set(game, "daynight", "unknown");
  cw_game_info_set(game, "usedh", "false");
  cw_game_info_set(game, "umphome", "");
  cw_game_info_set(game, "ump1b", "");
  cw_game_info_set(game, "ump2b", "");
  cw_game_info_set(game, "ump3b", "");
  cw_game_info_set(game, "scorer", "");
  cw_game_info_set(game, "translator", "");
  cw_game_info_set(game, "inputter", "");
  curtime = time(NULL);
  loctime = localtime(&curtime);
  strftime(buffer, 256, "%Y/%m/%d %I:%M%p", loctime);
  cw_game_info_set(game, "inputtime", buffer);
  cw_game_info_set(game, "howscored", "unknown");
  cw_game_info_set(game, "pitches", "none");
  cw_game_info_set(game, "temp", "0");
  cw_game_info_set(game, "winddir", "unknown");
  cw_game_info_set(game, "windspeed", "-1");
  cw_game_info_set(game, "fieldcond", "unknown");
  cw_game_info_set(game, "precip", "unknown");
  cw_game_info_set(game, "sky", "unknown");
  cw_game_info_set(game, "timeofgame", "0");
  cw_game_info_set(game, "attendance", "0");

  return game;
}

int cwscore_get_info_count(CWGame *game)
{
  int count = 0;
  CWInfo *info = game->first_info;

  while (info != NULL) {
    count++;
    info = info->next;
  }

  return count;
}

CWInfo *cwscore_get_info_number(CWGame *game, int index)
{
  int i = 0;
  CWInfo *info = game->first_info;

  while (info != NULL) {
    if (i == index) {
      return info;
    }
    else {
      info = info->next;
      i++;
    }
  }

  return NULL;
}

void cwscore_print_info(CWGame *game)
{
  int num_info = cwscore_get_info_count(game);
  int i, offset = num_info / 2 + num_info % 2;
  
  for (i = 0; i < offset; i++) {
    CWInfo *info = cwscore_get_info_number(game, i);
    printf("%-13s: %-23s ", info->label, info->data);
    info = cwscore_get_info_number(game, i + offset);
    if (info != NULL) {
      printf("%-13s: %-23s", info->label, info->data);
    }
    printf("\n");
  }
}

void
cwscore_edit_info(CWGame *game)
{
  char buffer1[256], buffer2[256];

  while (1) {
    cwscore_print_info(game);
    printf("Enter field to modify (end to finish)\n");
    cwscore_get_line(buffer1);

    if (!strcmp(buffer1, "end")) {
      break;
    }

    if (!strcmp(buffer1, "site")) {
      printf("Enter site: (format: CCCNN)\n");
    }
    else if (!strcmp(buffer1, "date")) {
      printf("Enter date: (format: YYYY/MM/DD)\n");
    }
    else if (!strcmp(buffer1, "number")) {
      printf("Enter game number:\n");
    }
    else if (!strcmp(buffer1, "starttime")) {
      printf("Enter start time: (0:00PM if unknown)\n");
    }
    else if (!strcmp(buffer1, "daynight")) {
      printf("Enter day/night: (day night unknown)\n");
    }
    else if (!strcmp(buffer1, "usedh")) {
      printf("Enter usedh: (false true)\n");
    }
    else if (!strcmp(buffer1, "umphome")) {
      printf("Enter home plate umpire:\n");
    }
    else if (!strcmp(buffer1, "ump1b")) {
      printf("Enter first base umpire:\n");
    }
    else if (!strcmp(buffer1, "ump2b")) {
      printf("Enter second base umpire:\n");
    }
    else if (!strcmp(buffer1, "ump3b")) {
      printf("Enter third base umpire:\n");
    }
    else if (!strcmp(buffer1, "scorer")) {
      printf("Enter scorer:\n");
    }
    else if (!strcmp(buffer1, "translator")) {
      printf("Enter translator:\n");
    }
    else if (!strcmp(buffer1, "inputter")) {
      printf("Enter inputter:\n");
    }
    else if (!strcmp(buffer1, "howscored")) {
      printf("Enter how scored: (park radio tv unknown)\n");
    }
    else if (!strcmp(buffer1, "temp")) {
      printf("Enter temperature: (0 if unknown)\n");
    }
    else if (!strcmp(buffer1, "winddir")) {
      printf("Enter wind direction: (fromlf fromcf fromrf ltor rtol tolf tocf torf unknown)\n");
    }
    else if (!strcmp(buffer1, "windspeed")) {
      printf("Enter wind speed: (-1 if unknown)\n");
    }
    else if (!strcmp(buffer1, "fieldcond")) {
      printf("Enter field condition: (dry wet soaked unknown)\n");
    }
    else if (!strcmp(buffer1, "precip")) {
      printf("Enter precipitation: (none drizzle rain showers snow unknown)\n");
    }
    else if (!strcmp(buffer1, "sky")) {
      printf("Enter sky: (sunny cloudy overcase night dome unknown)\n");
    }
    else if (!strcmp(buffer1, "timeofgame")) {
      printf("Enter time of game: (0 if unknown)\n");
    }
    else if (!strcmp(buffer1, "attendance")) {
      printf("Enter attendance: (0 if unknown)\n");
    }
    else {
      printf("Unknown field '%s'\n", buffer1);
      continue;
    }

    cwscore_get_line(buffer2);
    cw_game_info_set(game, buffer1, buffer2); 
  }
}

CWPlayer *cwscore_get_player_number(CWRoster *roster, int index)
{
  CWPlayer *player = roster->first_player;
  int i = 0;
  
  while (player != NULL) {
    if (i == index) {
      return player;
    }
    else {
      player = player->next;
      i++;
    }
  }

  return NULL;
}

void cwscore_print_lineup(CWRoster *roster, 
			  char lineup[10][9], int positions[10])
{
  int num_players = cw_roster_player_count(roster);
  int i, offset = num_players / 2 + num_players % 2;

  for (i = 0; i < offset; i++) {
    char name[256];
    CWPlayer *player = cwscore_get_player_number(roster, i);
    sprintf(name, "%2d %s, %s", i, player->last_name, player->first_name);
    printf("%-25s", name);

    player = cwscore_get_player_number(roster, i + offset);
    if (player != NULL) {
      sprintf(name, "%2d %s, %s", i + offset,
	      player->last_name, player->first_name);
      printf("%-25s", name);
    }

    if (i <= 9 && strcmp(lineup[i], "")) {
      player = cw_roster_player_find(roster, lineup[i]);
      sprintf(name, "%d: %s, %s", i, player->last_name, player->first_name);
      printf("%-25s %-2s", name, 
	     cwscore_position_from_number(positions[i]));
    }
    printf("\n");
  }
}

void cwscore_get_lineup(CWGame *game, CWRoster *roster, int team)
{
  int slot, positions[10];
  char lineup[10][9];
  CWPlayer *player;

  for (slot = 0; slot <= 9; slot++) {
    positions[slot] = 0;
    strcpy(lineup[slot], "");
  }

  while (1) {
    char buffer[256], name[256];
    int slot, position, index;

    printf("Enter lineup for %d %s\n", roster->year, roster->team_id);
    cwscore_print_lineup(roster, lineup, positions);

    printf("Enter: name <space> slot <space> position; END to end\n", slot);
    cwscore_get_line(buffer);
    if (!strcmp(buffer, "END")) {
      break;
    }
    if (isdigit(buffer[0])) {
      if (sscanf(buffer, "%d %d %d", &index, &slot, &position) != 3) {
	continue;
      }

      if (slot < 1 || slot > 9) {
	continue;
      }

      if (position < 1 || position > 9) {
	continue;
      }
      
      if (index >= 0 && index < cw_roster_player_count(roster)) {
	player = cwscore_get_player_number(roster, index);
	strcpy(lineup[slot], player->player_id);
	positions[slot] = position;
      }
      else {
	printf("Invalid player number '%d'\n", index);
      } 
    }
    else {
      if (sscanf(buffer, "%s %d %d", name, &slot, &position) != 3) {
	continue;
      }

      if (slot < 1 || slot > 9) {
	continue;
      }

      if (position < 1 || position > 9) {
	continue;
      }

      for (player = roster->first_player; player; player = player->next) {
	if (!strcmp(player->last_name, name)) {
	  strcpy(lineup[slot], player->player_id);
	  positions[slot] = position;
	  break;
	}
      }
      
      if (player == NULL) {
	printf("Couldn't figure out who '%s' is\n", name);
	slot--;
      }
    }
  }

  for (slot = 1; slot <= 9; slot++) {
    CWPlayer *player = cw_roster_player_find(roster, lineup[slot]);
    char name[256];
    sprintf(name, "%s %s", player->first_name, player->last_name); 
    cw_game_starter_append(game, player->player_id, name, 
			   team, slot, positions[slot]);
  }
}

void cwscore_uppercase(char *buffer)
{
  int i;

  for (i = 0; i < strlen(buffer); i++) {
    buffer[i] = toupper(buffer[i]);
  }
}

int cwscore_gameover(CWGameIterator *gameiter)
{
  if (gameiter->inning >= 9 &&
      gameiter->half_inning == 1 &&
      gameiter->score[1] > gameiter->score[0]) {
    return 1;
  }

  if (gameiter->outs == 3 &&
      gameiter->inning == 9 &&
      gameiter->half_inning == 0 &&
      gameiter->score[1] > gameiter->score[0]) {
    return 1;
  }

  if (gameiter->inning >= 9 &&
      gameiter->outs == 3 &&
      gameiter->half_inning == 1 &&
      gameiter->score[0] > gameiter->score[1]) {
    return 1;
  }

  return 0;
}

void cwscore_get_substitute(CWGame *game, CWRoster *roster,
			    int inning, int half_inning, int team)
{
  char buffer[256], name[256];
  int position, slot;
  CWPlayer *player;

  while (1)  {
    printf("Enter substitute (name <space> position <space> slot)\n");
    cwscore_get_line(buffer);
    sscanf(buffer, "%s %d %d", name, &position, &slot);
    for (player = roster->first_player; player; player = player->next) {
      if (!strcmp(player->last_name, name)) {
	sprintf(name, "%s %s", player->first_name, player->last_name);
	cw_game_substitute_append(game, player->player_id, name,
				  team, slot, position);
	return;
      }
    }
    
    if (player == NULL) {
      printf("Couldn't figure out who '%s' is\n", name);
    }
  }
}

void cwscore_get_pinch_hitter(CWGameIterator *gameiter, CWRoster *roster,
			      int inning, int half_inning, int team)
{
  int slot, index, positions[10];
  char lineup[10][9];
  char buffer[256];

  cw_gameiter_reset(gameiter);

  while (gameiter->event != NULL) {
    cw_gameiter_next(gameiter);
  }

  for (slot = 1; slot <= 9; slot++) {
    strcpy(lineup[slot], gameiter->lineups[slot][team].player_id);
    positions[slot] = gameiter->lineups[slot][team].position;
  }

  cwscore_print_lineup(roster, lineup, positions);

  slot = (gameiter->num_batters[team] % 9) + 1;

  printf("Enter pinch-hitter:\n");
  cwscore_get_line(buffer);
  
  if (isdigit(buffer[0])) {
    sscanf(buffer, "%d", &index);
      
    if (index >= 0 && index < cw_roster_player_count(roster)) {
      CWPlayer *player = cwscore_get_player_number(roster, index);
      sprintf(buffer, "%s %s", player->first_name, player->last_name);
      cw_game_substitute_append(gameiter->game, player->player_id, buffer,
				team, slot, 11);
    }
    else {
      printf("Invalid player number '%d'\n", index);
    } 
  }
  else {
    CWPlayer *player;
    for (player = roster->first_player; player; player = player->next) {
      if (!strcmp(player->last_name, buffer)) {
	sprintf(buffer, "%s %s", player->first_name, player->last_name);
	cw_game_substitute_append(gameiter->game, player->player_id,
				  buffer, team, slot, 11);
	return;
      }
    }
      
    printf("Couldn't figure out who '%s' is\n", buffer);
  }
}

void cwscore_undo(CWGame *game)
{
  if (game->first_event == NULL) {
    return;
  }
  
  cw_game_truncate(game, game->last_event);
}

void cwscore_enter_comment(CWGame *game)
{
  char buffer[256];
  printf("Enter one-line comment:\n");
  cwscore_get_line(buffer);
  cw_game_comment_append(game, buffer);
}

void cwscore_display_lineups(CWGameIterator *gameiter,
			     CWRoster *visitors, CWRoster *home)
{
  int i;
  static char positions[13][3] = {
    "", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf",
    "dh", "ph", "pr"
  };

  for (i = 1; i <= 9; i++) {
    printf("%-20s %-2s   %-20s %-2s\n",
	   gameiter->lineups[i][0].name,
	   positions[gameiter->lineups[i][0].position],
	   gameiter->lineups[i][1].name,
	   positions[gameiter->lineups[i][1].position]);
  }
}

int cwscore_display_state(CWGameIterator *gameiter,
			  CWRoster *visitors, CWRoster *home,
			  int *inning, int *half_inning, char **batter)
{
  int outs, i;
  char runners[4][20];

  cw_gameiter_reset(gameiter);

  while (gameiter->event != NULL) {
    cw_gameiter_next(gameiter);
  }

  if (cwscore_gameover(gameiter)) {
    return 0;
  }

  if (gameiter->outs == 3) {
    *inning = gameiter->inning + gameiter->half_inning;
    *half_inning = (gameiter->half_inning + 1) % 2;
    outs = 0;
    for (i = 1; i <= 3; i++) {
      strcpy(runners[i], "");
    }
  }
  else {
    *inning = gameiter->inning;
    *half_inning = gameiter->half_inning;
    outs = gameiter->outs;
    for (i = 1; i <= 3; i++) {
      strcpy(runners[i], gameiter->runners[i]);
    }
  }

  cwscore_display_lineups(gameiter, visitors, home);

  *batter = gameiter->lineups[gameiter->num_batters[*half_inning] % 9 + 1][*half_inning].player_id;

  printf("B: %8s  1: %8s  2: %8s  3: %8s  PA: %2d I: %2d  O: %d  S: %2d-%2d\n", 
	 *batter, runners[1], runners[2], runners[3],
	 gameiter->num_batters[*half_inning] + 1,
	 *inning, outs, gameiter->score[0], gameiter->score[1]);
  return 1;
}

void cwscore_enter_events(CWGame *game, CWRoster *visitors, CWRoster *home)
{
  char buffer[256];
  CWGameIterator *gameiter = cw_gameiter_create(game);

  while (1) {
    int inning, half_inning;
    char *batter;

    if (!cwscore_display_state(gameiter, visitors, home, 
			       &inning, &half_inning, &batter)) {
      /* game is over */
      return;
    }
    cwscore_get_line(buffer);
    cwscore_uppercase(buffer);
    if (!strcmp(buffer, "OS")) {
      cw_game_event_append(game, inning, half_inning,
			   batter, "??", "", "NP");
      cwscore_get_substitute(game, (half_inning == 0) ? visitors : home,
			     inning, half_inning, half_inning);
    }
    else if (!strcmp(buffer, "PH")) {
      cwscore_get_pinch_hitter(gameiter, (half_inning == 0) ? visitors : home,
			       inning, half_inning, half_inning);
    }
    else if (!strcmp(buffer, "DS")) {
      cw_game_event_append(game, inning, half_inning,
			   batter, "??", "", "NP");
      cwscore_get_substitute(game, (half_inning == 0) ? home : visitors,
			     inning, half_inning, 1 - half_inning);
    }
    else if (!strcmp(buffer, "BACK")) {
      cwscore_undo(game);
    }
    else if (!strcmp(buffer, "COM")) {
      cwscore_enter_comment(game);
    }
    else if (!strcmp(buffer, "END")) {
      break;
    }
    else {
      CWParsedEvent event_data;
      if (!cw_parse_event(buffer, &event_data)) {
	printf("Parse error in event text '%s'\n", buffer);
      }
      else {
	cw_game_event_append(game, inning, half_inning,
			     batter, "??", "", buffer);
      }
    }
  }

  cw_gameiter_cleanup(gameiter);
  free(gameiter);
}

void cwscore_enter_game(CWScorebook *scorebook)
{
  char buffer[20];
  CWRoster *visitor, *home;
  CWGame *game;
  FILE *file;

  cwscore_get_team("Enter visiting team ID: (CCCYYYY)", buffer);
  visitor = cwscore_read_roster(buffer);
  cwscore_get_team("Enter home team ID: (CCCYYYY)", buffer);
  home = cwscore_read_roster(buffer);
  printf("Enter game ID: (CCCYYYYMMDDG)\n");
  cwscore_get_line(buffer);
  game = cwscore_create_game(buffer, visitor->team_id, home->team_id);
  cwscore_edit_info(game);
  cwscore_get_lineup(game, visitor, 0);
  cwscore_get_lineup(game, home, 1);
  cwscore_enter_events(game, visitor, home);

  cw_scorebook_append_game(scorebook, game);

  cw_roster_cleanup(home);
  free(home);
  cw_roster_cleanup(visitor);
  free(visitor);
}

CWScorebook *cwscore_load(void)
{
  FILE *file;
  char buffer[256];
  CWScorebook *scorebook;

  printf("Enter file name to open:\n");
  cwscore_get_line(buffer);
  if ((file = fopen(buffer, "r")) == NULL) {
    printf("Error opening file '%s'\n", buffer);
    return NULL;
  }
  fclose(file);

  scorebook = cw_scorebook_create();
  cw_scorebook_read(scorebook, buffer);
  return scorebook;
}

void cwscore_save(CWScorebook *scorebook)
{
  FILE *file;
  char buffer[256];

  printf("Enter file name to save:\n");
  cwscore_get_line(buffer);
  if ((file = fopen(buffer, "w")) == NULL) {
    printf("Error opening file '%s'\n", buffer);
    return;
  }

  cw_scorebook_write(scorebook, file);
  fclose(file);
}

void cwscore_list(CWScorebook *scorebook)
{
  CWGame *game = scorebook->first_game;
  int count = 0;

  while (game != NULL) {
    printf("Game: %3d  ID: %12s  V: %3s  H: %3s  Site: %5s\n",
	   ++count, game->game_id,
	   cw_game_info_lookup(game, "visteam"),
	   cw_game_info_lookup(game, "hometeam"),
	   cw_game_info_lookup(game, "site"));
    game = game->next;
  }
}

void cwscore_delete(CWScorebook *scorebook)
{
  char buffer[256];

  printf("Enter game ID to delete:\n");
  cwscore_get_line(buffer);

  cw_scorebook_remove_game(scorebook, buffer);
}

void cwscore_help(void)
{
  printf("List of commands:\n");
  printf("NEW      Create a new empty scorebook\n");
  printf("LOAD     Load a scorebook from file\n");
  printf("SAVE     Save a scorebook to file\n");
  printf("LIST     List the games in the current scorebook\n");
  printf("ENTER    Enter a new game\n");
  printf("DELETE   Delete a game from the scorebook\n");
  printf("QUIT     Exit program (does *not* save!)\n");
}


void cwscore_switchboard(void)
{
  CWScorebook *scorebook = NULL;
  char buffer[256];

  while (1) {
    printf("Command: (HELP for list of commands)\n");
    cwscore_get_line(buffer);
    cwscore_uppercase(buffer);

    if (!strcmp(buffer, "NEW")) {
      if (scorebook != NULL) {
	cw_scorebook_cleanup(scorebook);
	free(scorebook);
      }
      scorebook = cw_scorebook_create();
    }
    else if (!strcmp(buffer, "LOAD")) {
      if (scorebook != NULL) {
	cw_scorebook_cleanup(scorebook);
	free(scorebook);
      }
      scorebook = cwscore_load();
    }
    else if (!strcmp(buffer, "SAVE")) {
      if (scorebook == NULL) {
	printf("Error: No scorebook opened!\n");
      }
      else {
	cwscore_save(scorebook);
      }
    }
    else if (!strcmp(buffer, "LIST")) {
      if (scorebook == NULL) {
	printf("Error: No scorebook opened!\n");
      }
      else {
	cwscore_list(scorebook);
      }
    }
    else if (!strcmp(buffer, "DELETE")) {
      if (scorebook == NULL) {
	printf("Error: No scorebook opened!\n");
      }
      else {
	cwscore_delete(scorebook);
      }
    }
    else if (!strcmp(buffer, "ENTER")) {
      if (scorebook == NULL) {
	printf("Error: No scorebook opened!\n");
      }
      else {
	cwscore_enter_game(scorebook);
      }
    }
    else if (!strcmp(buffer, "HELP")) {
      cwscore_help();
    }
    else if (!strcmp(buffer, "QUIT")) {
      return;
    }
  }
}
 
int main(int argc, char *argv[])
{
  fprintf(stderr, "This is CWScore, the Chadwick scorebook program\n");
  fprintf(stderr, "You are using Chadwick version " VERSION "\n\n");

  cwscore_switchboard();

  return 0;
}

 



