/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of game manipulation routines
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
#include <ctype.h>

#include "game.h"
#include "file.h"

CWGame *cw_game_create(char *game_id)
{
  CWGame *game = (CWGame *) malloc(sizeof(CWGame));

  game->game_id = (char *) malloc(sizeof(char) * (strlen(game_id) + 1));
  strcpy(game->game_id, game_id);
  game->version = NULL;
  game->first_info = NULL;
  game->last_info = NULL;
  game->first_starter = NULL;
  game->last_starter = NULL;
  game->first_event = NULL;
  game->last_event = NULL;
  game->first_data = NULL;
  game->last_data = NULL;
  game->prev = NULL;
  game->next = NULL;

  return game;
}

/*
 * Private auxiliary function to cleanup memory from tag list 
 */
static void cw_game_cleanup_tags(CWGame *game)
{
  CWInfo *info = game->first_info;
  while (info != NULL) {
    CWInfo *next_info = info->next;
    free(info->label);
    free(info->data);
    free(info);
    info = next_info;
  }

  game->first_info = NULL;
  game->last_info = NULL;
}

/*
 * Private auxiliary function to cleanup memory from starters list
 */
static void cw_game_cleanup_starters(CWGame *game)
{
  CWAppearance *starter = game->first_starter;

  while (starter != NULL) {
    CWAppearance *next_starter = starter->next;
    free(starter->player_id);
    free(starter->name);
    free(starter);
    starter = next_starter;
  }

  game->first_starter = NULL;
  game->last_starter = NULL;
}

/*
 * Private auxiliary function to cleanup memory from events list
 * 'event' is the event at which to start
 */
static void cw_game_cleanup_events(CWGame *game, CWEvent *event)
{
  if (event->prev != NULL) {
    event->prev->next = NULL;
  }
  game->last_event = event->prev;

  while (event != NULL) {
    CWEvent *next_event = event->next;
    CWAppearance *sub = event->first_sub;
    free(event->batter);
    free(event->count);
    free(event->pitches);
    free(event->event_text);
    while (sub != NULL) {
      CWAppearance *next_sub = sub->next;
      free(sub->player_id);
      free(sub->name);
      free(sub);
      sub = next_sub;
    }
    free(event);
    event = next_event;
  }
}

/*
 * Private auxiliary function to cleanup memory from data list
 */
static void cw_game_cleanup_data(CWGame *game)
{
  CWData *data = game->first_data;
  while (data != NULL) {
    int i;
    CWData *next_data = data->next;

    for (i = 0; i < data->num_data; free(data->data[i++]));
    free(data->data);
    data = next_data;
  }

  game->first_data = NULL;
  game->last_data = NULL;
}

void cw_game_cleanup(CWGame *game)
{
  cw_game_cleanup_tags(game);
  cw_game_cleanup_starters(game);
  cw_game_cleanup_events(game, game->first_event);
  game->first_event = NULL;
  cw_game_cleanup_data(game);
  
  free(game->version);
  game->version = NULL;
  free(game->game_id);
  game->game_id = NULL;
}

void cw_game_set_version(CWGame *game, char *version)
{
  game->version = (char *) malloc(sizeof(char) * (strlen(version) + 1));
  strcpy(game->version, version);
}


void cw_game_info_append(CWGame *game, char *label, char *data)
{
  CWInfo *info = (CWInfo *) malloc(sizeof(CWInfo));
  info->label = (char *) malloc(sizeof(char) * (strlen(label) + 1));
  strcpy(info->label, label);
  info->data = (char *) malloc(sizeof(char) * (strlen(data) + 1));
  strcpy(info->data, data);
  info->prev = game->last_info;
  info->next = NULL;

  if (game->first_info == NULL) {
    game->first_info = info;
  }
  else {
    game->last_info->next = info;
  }
  game->last_info = info;
}

/*
 * Returns a pointer to a string containing the data associated with
 * 'label', or NULL if not found.
 * The caller should copy the data for manipulation elsewhere.
 * The list of tags is searched from the end, as the convention in
 * the DiamondWare library seems to be that, in the case of duplicated
 * tags (which happens often with edit dates), the *last* one is the one
 * which is reported.
 */
char *cw_game_info_lookup(CWGame *game, char *label)
{
  CWInfo *info;
  for (info = game->last_info; info; info = info->prev) {
    if (!strcmp(info->label, label)) {
      return info->data;
    }
  }

  return NULL;
}

void cw_game_starter_append(CWGame *game, char *player_id, char *name,
			    int team, int slot, int pos)
{
  CWAppearance *starter = (CWAppearance *) malloc(sizeof(CWAppearance));
  starter->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(starter->player_id, player_id);
  starter->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(starter->name, name);
  starter->team = team;
  starter->slot = slot;
  starter->pos = pos;
  starter->prev = game->last_starter;
  starter->next = NULL;

  if (game->first_starter == NULL) {
    game->first_starter = starter;
  }
  else {
    game->last_starter->next = starter;
  }
  game->last_starter = starter;
}

CWAppearance *
cw_game_starter_find(CWGame *game, int team, int slot)
{
  CWAppearance *starter = game->first_starter;
  while (starter != NULL) {
    if (starter->team == team && starter->slot == slot) {
      return starter;
    }
    starter = starter->next;
  }

  return NULL;
}

CWAppearance *
cw_game_starter_find_by_position(CWGame *game, int team, int pos)
{
  CWAppearance *starter = game->first_starter;
  while (starter != NULL) {
    if (starter->team == team && starter->pos == pos) {
      return starter;
    }
    starter = starter->next;
  }

  return NULL;
}

void cw_game_event_append(CWGame *game, int inning, int half_inning,
			  char *batter, char *count, char *pitches,
			  char *event_text)
{
  CWEvent *event = (CWEvent *) malloc(sizeof(CWEvent));
  event->inning = inning;
  event->half_inning = half_inning;
  event->batter = (char *) malloc(sizeof(char) * (strlen(batter) + 1));
  strcpy(event->batter, batter);
  event->count = (char *) malloc(sizeof(char) * (strlen(count) + 1));
  strcpy(event->count, count);
  event->pitches = (char *) malloc(sizeof(char) * (strlen(pitches) + 1));
  strcpy(event->pitches, pitches);
  event->event_text = (char *) malloc(sizeof(char) * (strlen(event_text) + 1));
  strcpy(event->event_text, event_text);
  event->batter_hand = ' ';
  event->pitcher_hand = ' ';
  event->prev = game->last_event;
  event->next = NULL;
  event->first_sub = NULL;
  event->last_sub = NULL;

  if (game->first_event == NULL) {
    game->first_event = event;
  }
  else {
    game->last_event->next = event;
  }
  game->last_event = event;
}

void cw_game_truncate(CWGame *game, CWEvent *event)
{
  cw_game_cleanup_events(game, event);
}

void cw_game_substitute_append(CWGame *game, char *player_id, char *name,
			       int team, int slot, int pos)
{
  CWAppearance *sub = (CWAppearance *) malloc(sizeof(CWAppearance));
  sub->player_id = (char *) malloc(sizeof(char) * (strlen(player_id) + 1));
  strcpy(sub->player_id, player_id);
  sub->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(sub->name, name);
  sub->team = team;
  sub->slot = slot;
  sub->pos = pos;
  sub->prev = game->last_event->last_sub;
  sub->next = NULL;
  if (game->last_event->last_sub) {
    game->last_event->last_sub->next = sub;
  }
  else {
    game->last_event->first_sub = sub;
  }
  game->last_event->last_sub = sub;
}

void cw_game_data_append(CWGame *game, int num_data, char **data)
{
  int i;
  CWData *d = (CWData *) malloc(sizeof(CWData));

  d->num_data = num_data;
  d->data = (char **) malloc(sizeof(char *) * num_data);
  
  for (i = 1; i < num_data; i++) {
    d->data[i-1] = (char *) malloc(sizeof(char) * (strlen(data[i]) + 1));
    strcpy(d->data[i-1], data[i]);
  }

  if (game->first_data) {
    game->last_data->next = d;
  }
  else {
    game->last_data = d;
  }
  d->prev = game->last_data;
  game->last_data = d;
}

CWGame *
cw_game_read(FILE *file)
{
  char buf[256];
  char **tokens;
  int numTokens, i;
  fpos_t filepos;
  char batHand = ' ', batHandBatter[256];
  CWGame *game;

  tokens = (char **) malloc(sizeof(char *) * CW_MAX_TOKENS);
  for (i = 0; i < CW_MAX_TOKENS; i++) {
    tokens[i] = (char *) malloc(sizeof(char) * CW_MAX_TOKEN_LENGTH);
  }

  fgets(buf, 256, file);
  numTokens = cw_file_tokenize_line(buf, tokens);
  if (!strcmp(tokens[0], "id")) {
    game = cw_game_create(tokens[1]);
  }
  else {
    for (i = 0; i < CW_MAX_TOKENS; i++) {
      free(tokens[i]);
    }
    free(tokens);
    return NULL;
  }

  while (!feof(file)) {
    fgetpos(file, &filepos);
    fgets(buf, 256, file);
    numTokens = cw_file_tokenize_line(buf, tokens);
    if (!strcmp(tokens[0], "id")) {
      fsetpos(file, &filepos);
      break;
    }
    else if (!strcmp(tokens[0], "version")) {
      cw_game_set_version(game, tokens[1]);
    }
    else if (!strcmp(tokens[0], "info")) {
      cw_game_info_append(game, tokens[1], tokens[2]);
    }
    else if (!strcmp(tokens[0], "start")) {
      cw_game_starter_append(game, tokens[1], tokens[2],
			     atoi(tokens[3]), atoi(tokens[4]), 
			     atoi(tokens[5]));
    } 
    else if (!strcmp(tokens[0], "play")) {
      cw_game_event_append(game, atoi(tokens[1]), atoi(tokens[2]),
			   tokens[3], tokens[4], tokens[5], tokens[6]);
      if (batHand != ' ' && !strcmp(batHandBatter, tokens[3])) {
	game->last_event->batter_hand = batHand;
      }
      else {
	/* Once batter changes, clear this out */
	batHand = ' ';
	strcpy(batHandBatter, "");
      }
    }
    else if (!strcmp(tokens[0], "sub")) {
      cw_game_substitute_append(game, tokens[1], tokens[2],
				atoi(tokens[3]), atoi(tokens[4]), 
				atoi(tokens[5]));
    }
    else if (!strcmp(tokens[0], "data")) {
      cw_game_data_append(game, numTokens, tokens);
    }
    else if (!strcmp(tokens[0], "badj")) {
      strncpy(batHandBatter, tokens[1], 255);
      batHand = tokens[2][0];
    }
  }

  for (i = 0; i < CW_MAX_TOKENS; i++) {
    free(tokens[i]);
  }
  free(tokens);

  return game;
}

static void
cw_game_write_header(CWGame *game, FILE *file)
{
  CWInfo *info = game->first_info;

  fprintf(file, "id,%s\n", game->game_id);
  fprintf(file, "version,%s\n", game->version);

  while (info != NULL) {
    /*
     * Use explicit quotes around the data if either a comma appears
     * in the data, or to be output-compatible with existing tools 
     */
    if (strstr(info->data, ",") ||
	!strcmp(info->label, "inputprogvers") ||
	!strcmp(info->label, "umphome") ||
	!strcmp(info->label, "ump1b") ||
	!strcmp(info->label, "ump2b") ||
	!strcmp(info->label, "ump3b") ||
	!strcmp(info->label, "scorer") ||
	!strcmp(info->label, "translator") ||
	!strcmp(info->label, "inputter")) {
      fprintf(file, "info,%s,\"%s\"\n", info->label, info->data);
    }
    else {
      fprintf(file, "info,%s,%s\n", info->label, info->data);
    }
    info = info->next;
  }
}

static void
cw_game_write_starters(CWGame *game, FILE *file)
{
  CWAppearance *starter = game->first_starter;

  while (starter != NULL) {
    fprintf(file, "start,%s,\"%s\",%d,%d,%d\n",
	    starter->player_id, starter->name,
	    starter->team, starter->slot, starter->pos);
    starter = starter->next;
  }
}

static void
cw_game_write_events(CWGame *game, FILE *file)
{
  CWEvent *event = game->first_event;

  while (event != NULL) {
    if (event->batter_hand != ' ') {
      fprintf(file, "badj,%s,%c\n", event->batter, event->batter_hand);
    }
	      
    fprintf(file, "play,%d,%d,%s,%s,%s,%s\n",
	    event->inning, event->half_inning,
	    event->batter, event->count, event->pitches,
	    event->event_text);
    if (event->first_sub != NULL) {
      CWAppearance *sub = event->first_sub;
      while (sub != NULL) {
	fprintf(file, "sub,%s,\"%s\",%d,%d,%d\n",
		sub->player_id, sub->name, 
		sub->team, sub->slot, sub->pos);
	sub = sub->next;
      }
    }
    event = event->next;
  }
}

static void
cw_game_write_data(CWGame *game, FILE *file)
{
  CWData *data = game->first_data;
  
  while (data != NULL) {
    int i;

    fprintf(file, "data");
    for (i = 0; i < data->num_data; i++) {
      fprintf(file, ",%s", data->data[i]);
    }
    fprintf(file, "\n");
    data = data->next;
  }
}

void
cw_game_write(CWGame *game, FILE *file)
{
  cw_game_write_header(game, file);
  cw_game_write_starters(game, file);
  cw_game_write_events(game, file);
  cw_game_write_data(game, file);
}






