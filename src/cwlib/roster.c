/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of roster management and manipulation routines
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "roster.h"
#include "file.h"    

CWPlayer *
cw_player_create(char *player_id,
		 char *last_name, char *first_name,
		 char bats, char throws)
{
  CWPlayer *player = (CWPlayer *) malloc(sizeof(CWPlayer));
  player->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(player->player_id, player_id);

  player->last_name = (char *) malloc(sizeof(char) * (strlen(last_name) + 1));
  strcpy(player->last_name, last_name);

  player->first_name = (char *) malloc(sizeof(char) * (strlen(first_name)+1));
  strcpy(player->first_name, first_name);

  player->bats = bats;
  player->throws = throws;
  player->prev = NULL;
  player->next = NULL;

  return player;
}

void
cw_player_cleanup(CWPlayer *player)
{
  free(player->player_id);
  free(player->last_name);
  free(player->first_name);
}


CWRoster *
cw_roster_create(char *team_id, int year, char league,
		 char *city, char *nickname)
{
  CWRoster *roster = (CWRoster *) malloc(sizeof(CWRoster));
  roster->team_id = (char *) malloc(sizeof(char) * (strlen(team_id) + 1));
  strcpy(roster->team_id, team_id);

  roster->city = (char *) malloc(sizeof(char) * (strlen(city) + 1));
  strcpy(roster->city, city);

  roster->nickname = (char *) malloc(sizeof(char) * (strlen(nickname) + 1));
  strcpy(roster->nickname, nickname);

  roster->year = year;
  roster->league = league;

  roster->first_player = NULL;
  roster->last_player = NULL;
  roster->prev = NULL;
  roster->next = NULL;

  return roster;
}

void 
cw_roster_cleanup(CWRoster *roster)
{
  CWPlayer *player = roster->first_player;

  while (player != NULL) {
    CWPlayer *next_player = player->next;
    cw_player_cleanup(player);
    free(player);
    player = next_player;
  }

  free(roster->team_id);
  free(roster->city);
  free(roster->nickname);
}

void
cw_roster_player_append(CWRoster *roster, CWPlayer *player)
{
  player->prev = roster->last_player;
 
  if (roster->first_player == NULL) {
    roster->first_player = player;
  }
  else {
    roster->last_player->next = player;
  }

  roster->last_player = player;
}

CWPlayer *
cw_roster_player_find(CWRoster *roster, char *player_id)
{
  CWPlayer *player = roster->first_player;

  if (player_id == NULL) {
    return NULL;
  }

  while (player != NULL) {
    if (!strcmp(player->player_id, player_id)) {
      return player;
    }

    player = player->next;
  }

  return NULL;
}

int
cw_roster_player_count(CWRoster *roster)
{
  CWPlayer *player = roster->first_player;
  int count = 0;

  while (player != NULL) {
    count++;
    player = player->next;
  }
  return count;
}

void
cw_roster_read(CWRoster *roster, FILE *file)
{
  char buf[256];
  char **tokens;
  int numTokens, i;

  rewind(file);

  tokens = (char **) malloc(sizeof(char *) * CW_MAX_TOKENS);
  for (i = 0; i < CW_MAX_TOKENS; i++) {
    tokens[i] = (char *) malloc(sizeof(char) * CW_MAX_TOKEN_LENGTH);
  }

  while (!feof(file)) {
    strcpy(buf, "");
    fgets(buf, 256, file);
    if ((numTokens = cw_file_tokenize_line(buf, tokens)) != 5) {
      continue;
    }

    cw_roster_player_append(roster, 
			    cw_player_create(tokens[0], tokens[1], tokens[2],
					     tokens[3][0], tokens[4][0]));
  }

  for (i = 0; i < CW_MAX_TOKENS; i++) {
    free(tokens[i]);
  }
  free(tokens);
}

void
cw_roster_write(CWRoster *roster, FILE *file)
{
  CWPlayer *player = roster->first_player;

  while (player != NULL) {
    fprintf(file, "\"%s\",\"%s\",\"%s\",%c,%c\n",
	    player->player_id, player->last_name, player->first_name,
	    player->bats, player->throws);

    player = player->next;
  }
}

char
cw_roster_batting_hand(CWRoster *roster, char *player_id)
{
  CWPlayer *player;

  if (roster == NULL) {
    return '?';
  }

  for (player = roster->first_player; player; player = player->next) {
    if (!strcmp(player->player_id, player_id)) {
      return player->bats;
    }
  }

  return '?';
}

char
cw_roster_throwing_hand(CWRoster *roster, char *player_id)
{
  CWPlayer *player;

  if (roster == NULL) {
    return '?';
  }

  for (player = roster->first_player; player; player = player->next) {
    if (!strcmp(player->player_id, player_id)) {
      return player->throws;
    }
  }

  return '?';
}

