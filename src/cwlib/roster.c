/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/roster.c
 * Implementation of roster management and manipulation routines
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


void
cw_player_set_first_name(CWPlayer *player, char *name)
{
  char *x = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  if (x == NULL)  return;
  strcpy(x, name);
  free(player->first_name);
  player->first_name = x;    
}

void
cw_player_set_last_name(CWPlayer *player, char *name)
{
  char *x = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  if (x == NULL)  return;
  strcpy(x, name);
  free(player->last_name);
  player->last_name = x;    
}

CWRoster *
cw_roster_create(char *team_id, int year, char *league,
		 char *city, char *nickname)
{
  CWRoster *roster = (CWRoster *) malloc(sizeof(CWRoster));
  roster->team_id = (char *) malloc(sizeof(char) * (strlen(team_id) + 1));
  strcpy(roster->team_id, team_id);

  roster->city = (char *) malloc(sizeof(char) * (strlen(city) + 1));
  strcpy(roster->city, city);

  roster->nickname = (char *) malloc(sizeof(char) * (strlen(nickname) + 1));
  strcpy(roster->nickname, nickname);

  roster->league = (char *) malloc(sizeof(char) * (strlen(league) + 1));
  strcpy(roster->league, league);

  roster->year = year;

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
  free(roster->league);
}

void
cw_roster_set_city(CWRoster *roster, char *city)
{
  char *x = (char *) malloc(sizeof(char) * (strlen(city) + 1));
  if (x == NULL)  return;
  strcpy(x, city);
  free(roster->city);
  roster->city = x;    
}

void
cw_roster_set_nickname(CWRoster *roster, char *nickname)
{
  char *x = (char *) malloc(sizeof(char) * (strlen(nickname) + 1));
  if (x == NULL)  return;
  strcpy(x, nickname);
  free(roster->nickname);
  roster->nickname = x;    
}

void
cw_roster_set_league(CWRoster *roster, char *league)
{
  char *x = (char *) malloc(sizeof(char) * (strlen(league) + 1));
  if (x == NULL)  return;
  strcpy(x, league);
  free(roster->league);
  roster->league = x;    
}


void
cw_roster_player_insert(CWRoster *roster, CWPlayer *player)
{
  if (roster->first_player == NULL) {
    roster->first_player = player;
    roster->last_player = player;
  }
  else {
    CWPlayer *x = roster->first_player;
    while (x != NULL && strcmp(x->player_id, player->player_id) < 0) {
      x = x->next;
    }

    if (x == NULL) {
      player->prev = roster->last_player;
      roster->last_player->next = player;
      roster->last_player = player;
    }
    else if (x->prev == NULL) {
      roster->first_player->prev = player;
      player->next = roster->first_player;
      roster->first_player = player;
    }
    else {
      player->prev = x->prev;
      player->prev->next = player;
      x->prev = player;
      player->next = x;
    }
  }
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

int
cw_roster_read(CWRoster *roster, FILE *file)
{
  char buf[256], *player_id, *last_name, *first_name, *bats, *throws;

  rewind(file);

  while (!feof(file)) {
    strcpy(buf, "");
    if (fgets(buf, 256, file) == NULL) {
      return 0;
    }
    player_id = cw_strtok(buf);
    last_name = cw_strtok(NULL);
    first_name = cw_strtok(NULL);
    bats = cw_strtok(NULL);
    throws = cw_strtok(NULL);

    if (!player_id || !last_name || !first_name || !bats || !throws) {
      continue;
    }

    /* TODO: Some Retrosheet roster files have additional fields
     * at the end.  Preserve these (and write them out in cw_roster_write() ).
     */
    cw_roster_player_append(roster, 
			    cw_player_create(player_id, 
					     last_name, first_name,
					     bats[0], throws[0]));
  }
  return 1;
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

