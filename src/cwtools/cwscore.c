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

char *cwscore_get_info(char *prompt, char *data)
{
  printf("%s\n", prompt);
  cwscore_get_line(data);
  return data;
}


CWGame *cwscore_create_game(char *game_id, char *visitors, char *home)
{
  char buffer[256];
  CWGame *game = cw_game_create(game_id);
  cw_game_set_version(game, "1");
  
  cw_game_info_append(game, "inputprogvers",
		      "Chadwick version " VERSION);
  cw_game_info_append(game, "visteam", visitors);
  cw_game_info_append(game, "hometeam", home);

  cw_game_info_append(game, "site", 
		      cwscore_get_info("Site: (CCCNN)", buffer));
  cw_game_info_append(game, "date", 
		      cwscore_get_info("Date: (YYYY/MM/DD)", buffer));
  cw_game_info_append(game, "number",
		      cwscore_get_info("Game number:", buffer));
  cw_game_info_append(game, "starttime",
		      cwscore_get_info("Start time: (0:00PM if unknown)", buffer));
  cw_game_info_append(game, "daynight",
		      cwscore_get_info("Day/night: (day night unknown)", buffer));
  cw_game_info_append(game, "usedh",
		      cwscore_get_info("Use DH? (false true)", buffer));
  cw_game_info_append(game, "umphome",
		      cwscore_get_info("Home umpire:", buffer));
  cw_game_info_append(game, "ump1b",
		      cwscore_get_info("1b umpire:", buffer));
  cw_game_info_append(game, "ump2b",
		      cwscore_get_info("2b umpire:", buffer));
  cw_game_info_append(game, "ump3b",
		      cwscore_get_info("3b umpire:", buffer));
  cw_game_info_append(game, "scorer",
		      cwscore_get_info("Scorer:", buffer));
  cw_game_info_append(game, "translator",
		      cwscore_get_info("Translator:", buffer));
  cw_game_info_append(game, "inputter",
		      cwscore_get_info("Inputter:", buffer));
  cw_game_info_append(game, "inputtime", "");
  cw_game_info_append(game, "howscored", 
		      cwscore_get_info("How scored: (park radio tv unknown)", buffer));
  cw_game_info_append(game, "pitches", "none");
  cw_game_info_append(game, "temp", 
		      cwscore_get_info("Temperature: (0 if unknown)", buffer));
  cw_game_info_append(game, "winddir",
		      cwscore_get_info("Wind direction: (fromlf fromcf fromrf ltor rtol tolf tocf torf unknown)", buffer));
  cw_game_info_append(game, "windspeed",
		      cwscore_get_info("Wind speed: (-1 if unknown)", buffer));
  cw_game_info_append(game, "fieldcond",
		      cwscore_get_info("Field condition: (dry wet soaked unknown)", buffer));
  cw_game_info_append(game, "precip",
		      cwscore_get_info("Precipitation: (none drizzle rain showers snow unknown)", buffer));
  cw_game_info_append(game, "sky",
		      cwscore_get_info("Sky: (sunny cloudy overcast night dome unknown)", buffer));
  cw_game_info_append(game, "timeofgame",
		      cwscore_get_info("Time of game: (in minutes, 0 if unknown)", buffer));
  cw_game_info_append(game, "attendance",
		      cwscore_get_info("Attendance: (0 if unknown)", buffer));

  return game;
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

void cwscore_switchboard(void)
{
  CWScorebook *scorebook = NULL;
  char buffer[256];

  while (1) {
    printf("Command: (NEW, LOAD, SAVE, LIST, ENTER, QUIT)\n");
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
    else if (!strcmp(buffer, "ENTER")) {
      if (scorebook == NULL) {
	printf("Error: No scorebook opened!\n");
      }
      else {
	cwscore_enter_game(scorebook);
      }
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

 



