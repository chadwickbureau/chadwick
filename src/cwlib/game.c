/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/game.c
 * Implementation of game manipulation routines
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

#include "game.h"
#include "file.h"

int cw_data_get_item_int(CWData *data, unsigned int index)
{
  if (index >= data->num_data) {
    return -1;
  }
  return cw_atoi(data->data[index]);
}

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
  game->first_stat = NULL;
  game->last_stat = NULL;
  game->first_line = NULL;
  game->last_line = NULL;
  game->first_evdata = NULL;
  game->last_evdata = NULL;
  game->first_comment = NULL;
  game->last_comment = NULL;
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
 * 'event' is the event at which to start; if a null pointer,
 * remove all events from the game.
 */
static void cw_game_cleanup_events(CWGame *game, CWEvent *event)
{
  if (event->prev != NULL) {
    event->prev->next = NULL;
  }
  else {
    game->first_event = NULL;
  }
  game->last_event = event->prev;

  while (event != NULL) {
    CWEvent *next_event = event->next;
    CWAppearance *sub = event->first_sub;
    CWComment *comment = event->first_comment;
    free(event->batter);
    free(event->count);
    free(event->pitches);
    free(event->event_text);
    if (event->pitcher_hand_id) {
      free(event->pitcher_hand_id);
    }
    if (event->itb_runner_id) {
      free(event->itb_runner_id);
    }
    while (sub != NULL) {
      CWAppearance *next_sub = sub->next;
      free(sub->player_id);
      free(sub->name);
      free(sub);
      sub = next_sub;
    }
    while (comment != NULL) {
      CWComment *next_comment = comment->next;
      free(comment->text);
      free(comment);
      comment = next_comment;
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

/*
 * Private auxiliary function to cleanup memory from data list
 */
static void cw_game_cleanup_stat(CWGame *game)
{
  CWData *data = game->first_stat;
  while (data != NULL) {
    int i;
    CWData *next_data = data->next;

    for (i = 0; i < data->num_data; free(data->data[i++]));
    free(data->data);
    data = next_data;
  }

  game->first_stat = NULL;
  game->last_stat = NULL;
}


/*
 * Private auxiliary function to cleanup memory from evdata list
 */
static void cw_game_cleanup_evdata(CWGame *game)
{
  CWData *data = game->first_evdata;
  while (data != NULL) {
    int i;
    CWData *next_data = data->next;

    for (i = 0; i < data->num_data; free(data->data[i++]));
    free(data->data);
    data = next_data;
  }

  game->first_evdata = NULL;
  game->last_evdata = NULL;
}


/*
 * Private auxiliary function to cleanup memory from data list
 */
static void cw_game_cleanup_line(CWGame *game)
{
  CWData *data = game->first_line;
  while (data != NULL) {
    int i;
    CWData *next_data = data->next;

    for (i = 0; i < data->num_data; free(data->data[i++]));
    free(data->data);
    data = next_data;
  }

  game->first_line = NULL;
  game->last_line = NULL;
}

void cw_game_cleanup(CWGame *game)
{
  cw_game_cleanup_tags(game);
  cw_game_cleanup_starters(game);
  if (game->first_event != NULL) {
    cw_game_cleanup_events(game, game->first_event);
    game->first_event = NULL;
  }
  cw_game_cleanup_data(game);
  cw_game_cleanup_stat(game);
  cw_game_cleanup_line(game);
  cw_game_cleanup_evdata(game);
   
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

void
cw_game_info_set(CWGame *game, char *label, char *data)
{
  CWInfo *info = game->first_info;

  while (info != NULL) {
    if (!strcmp(info->label, label)) {
      free(info->data);
      info->data = (char *) malloc(sizeof(char) * (strlen(data) + 1));
      strcpy(info->data, data);
      return;
    }
    else {
      info = info->next;
    }
  }

  cw_game_info_append(game, label, data);
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

void cw_game_event_append(CWGame *game, int inning, int batting_team,
			  char *batter, char *count, char *pitches,
			  char *event_text)
{
  CWEvent *event = (CWEvent *) malloc(sizeof(CWEvent));
  event->inning = inning;
  event->batting_team = batting_team;
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
  event->pitcher_hand_id = NULL;
  event->ladj_align = 0;
  event->ladj_slot = 0;
  event->itb_base = 0;
  event->itb_runner_id = NULL;
  event->prev = game->last_event;
  event->next = NULL;
  event->first_sub = NULL;
  event->last_sub = NULL;
  event->first_comment = NULL;
  event->last_comment = NULL;

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
  d->next = NULL;
  
  for (i = 0; i < num_data; i++) {
    d->data[i] = (char *) malloc(sizeof(char) * (strlen(data[i]) + 1));
    strcpy(d->data[i], data[i]);
  }

  if (game->first_data) {
    game->last_data->next = d;
  }
  else {
    game->first_data = d;
  }
  d->prev = game->last_data;
  game->last_data = d;
}

void cw_game_data_set_er(CWGame *game, char *playerID, int er)
{
  char *foo[3];
  char buffer[10];
  CWData *data = game->first_data;

  while (data != NULL) {
    if (data->num_data >= 3 &&
	!strcmp(data->data[0], "er") &&
	!strcmp(data->data[1], playerID)) {
      free(data->data[2]);
      data->data[2] = (char *) malloc(10 * sizeof(char));
      sprintf(data->data[2], "%d", er);
      return;
    }
    data = data->next;
  }

  foo[0] = "er";
  foo[1] = playerID;
  foo[2] = buffer;
  sprintf(foo[2], "%d", er);
  cw_game_data_append(game, 3, foo);
}


void cw_game_stat_append(CWGame *game, int num_data, char **data)
{
  int i;
  CWData *d = (CWData *) malloc(sizeof(CWData));

  d->num_data = num_data;
  d->data = (char **) malloc(sizeof(char *) * num_data);
  d->next = NULL;
  
  for (i = 0; i < num_data; i++) {
    d->data[i] = (char *) malloc(sizeof(char) * (strlen(data[i]) + 1));
    strcpy(d->data[i], data[i]);
  }

  if (game->first_stat) {
    game->last_stat->next = d;
  }
  else {
    game->first_stat = d;
  }
  d->prev = game->last_stat;
  game->last_stat = d;
}

void cw_game_evdata_append(CWGame *game, int num_data, char **data)
{
  int i;
  CWData *d = (CWData *) malloc(sizeof(CWData));

  d->num_data = num_data;
  d->data = (char **) malloc(sizeof(char *) * num_data);
  d->next = NULL;
  
  for (i = 0; i < num_data; i++) {
    d->data[i] = (char *) malloc(sizeof(char) * (strlen(data[i]) + 1));
    strcpy(d->data[i], data[i]);
  }

  if (game->first_evdata) {
    game->last_evdata->next = d;
  }
  else {
    game->first_evdata = d;
  }
  d->prev = game->last_evdata;
  game->last_evdata = d;
}

void cw_game_line_append(CWGame *game, int num_data, char **data)
{
  int i;
  CWData *d = (CWData *) malloc(sizeof(CWData));

  d->num_data = num_data;
  d->data = (char **) malloc(sizeof(char *) * num_data);
  d->next = NULL;
  
  for (i = 0; i < num_data; i++) {
    d->data[i] = (char *) malloc(sizeof(char) * (strlen(data[i]) + 1));
    strcpy(d->data[i], data[i]);
  }

  if (game->first_line) {
    game->last_line->next = d;
  }
  else {
    game->first_line = d;
  }
  d->prev = game->last_line;
  game->last_line = d;
}

void cw_game_comment_append(CWGame *game, char *text)
{
  CWComment *comment = (CWComment *) malloc(sizeof(CWComment));
  comment->text = (char *) malloc(sizeof(char) * (strlen(text) + 1));
  strcpy(comment->text, text);
  comment->next = NULL;

  if (game->first_event == NULL) {
    comment->prev = game->last_comment;
    if (game->last_comment) {
      game->last_comment->next = comment;
    }
    else {
      game->first_comment = comment;
    }
    game->last_comment = comment;
  }
  else {
    comment->prev = game->last_event->last_comment;
    if (game->last_event->last_comment) {
      game->last_event->last_comment->next = comment;
    }
    else {
      game->last_event->first_comment = comment;
    }
    game->last_event->last_comment = comment;
  }
}

void 
cw_game_replace_player(CWGame *game, char *key_old, char *key_new) 
{
  CWAppearance *sub;
  CWData *data;
  CWEvent *event;

  for (sub = game->first_starter; sub != NULL; sub = sub->next) {
    if (!strcmp(sub->player_id, key_old)) {
      free(sub->player_id);
      sub->player_id = (char *) malloc(sizeof(char) * (strlen(key_new)+1));
      strcpy(sub->player_id, key_new);
    }
  }

  for (event = game->first_event; event != NULL; event = event->next) {
    if (!strcmp(event->batter, key_old)) {
      free(event->batter);
      event->batter = (char *) malloc(sizeof(char) * (strlen(key_new)+1));
      strcpy(event->batter, key_new);
    }

    for (sub = event->first_sub; sub != NULL; sub = sub->next) {
      if (!strcmp(sub->player_id, key_old)) {
	free(sub->player_id);
	sub->player_id = (char *) malloc(sizeof(char) * (strlen(key_new)+1));
	strcpy(sub->player_id, key_new);
      }
    }
  }

  for (data = game->first_data; data != NULL; data = data->next) {
    if (data->num_data >= 3 && !strcmp(data->data[0], "er") &&
	!strcmp(data->data[1], key_old)) {
      free(data->data[1]);
      data->data[1] = (char *) malloc(sizeof(char) * (strlen(key_new)+1));
      strcpy(data->data[1], key_new);
    }
  }

  if (cw_game_info_lookup(game, "wp") != NULL &&
      !strcmp(cw_game_info_lookup(game, "wp"), key_old)) {
    cw_game_info_set(game, "wp", key_new);
  }
  if (cw_game_info_lookup(game, "lp") != NULL &&
      !strcmp(cw_game_info_lookup(game, "lp"), key_old)) {
    cw_game_info_set(game, "lp", key_new);
  }
  if (cw_game_info_lookup(game, "save") != NULL &&
      !strcmp(cw_game_info_lookup(game, "save"), key_old)) {
    cw_game_info_set(game, "save", key_new);
  }
}

CWGame *
cw_game_read(FILE *file)
{
  char buf[1024], *tok;
  fpos_t filepos;
  char batHand = ' ', batHandBatter[1024], pitHand = ' ', pitHandPitcher[1024];
  char itbRunner[1024];
  int ladjAlign = 0, ladjSlot = 0, itbBase = 0;
  CWGame *game;

  if (fgets(buf, 1024, file) == NULL) {
    return NULL;
  }
  tok = cw_strtok(buf);
  if (tok && !strcmp(tok, "id")) {
    char *game_id = cw_strtok(NULL);
    if (game_id) {
      game = cw_game_create(game_id);
    }
    else {
      return NULL;
    }
  }
  else {
    return NULL;
  }

  while (!feof(file)) {
    fgetpos(file, &filepos);
    if (fgets(buf, 1024, file) == NULL) {
      if (feof(file)) {
	break;
      }
      else {
	if (game) {
	  cw_game_cleanup(game);
	  free(game);
	}
	return NULL;
      }
    }
    if (feof(file)) {
      break;
    }

    tok = cw_strtok(buf);
    if (!tok || !strcmp(tok, "id")) {
      fsetpos(file, &filepos);
      break;
    }
    else if (!strcmp(tok, "version")) {
      char *version;
      version = cw_strtok(NULL);
      if (version) {
	cw_game_set_version(game, version);
      }
    }
    else if (!strcmp(tok, "info")) {
      char *field, *value;
      field = cw_strtok(NULL);
      value = cw_strtok(NULL);
      if (field) {
	cw_game_info_append(game, field, (value) ? value : "");
      }
    }
    else if (!strcmp(tok, "start")) {
      char *player_id, *name, *team, *slot, *pos;
      player_id = cw_strtok(NULL);
      name = cw_strtok(NULL);
      team = cw_strtok(NULL);
      slot = cw_strtok(NULL);
      pos = cw_strtok(NULL);
      if (player_id && name && team && slot && pos) {
	cw_game_starter_append(game, player_id, name,
			       cw_atoi(team), cw_atoi(slot), 
			       cw_atoi(pos));
      }
    } 
    else if (!strcmp(tok, "play")) {
      char *inning, *batting_team, *batter, *count, *pitches, *play;
      inning = cw_strtok(NULL);
      batting_team = cw_strtok(NULL);
      batter = cw_strtok(NULL);
      count = cw_strtok(NULL);
      pitches = cw_strtok(NULL);
      play = cw_strtok(NULL);
      if (inning && batting_team && batter && count && pitches && play) {
	cw_game_event_append(game, cw_atoi(inning), cw_atoi(batting_team),
			     batter, count, pitches, play);
      }
      if (batHand != ' ' && !strcmp(batHandBatter, batter)) {
	game->last_event->batter_hand = batHand;
      }
      else {
	/* Once batter changes, clear this out */
	batHand = ' ';
	strcpy(batHandBatter, "");
      }

      if (pitHand != ' ') {
	game->last_event->pitcher_hand = pitHand;
	game->last_event->pitcher_hand_id = (char *) malloc(strlen(pitHandPitcher)+1);
	strcpy(game->last_event->pitcher_hand_id, pitHandPitcher);
	pitHand = ' ';
	strcpy(pitHandPitcher, "");
      }

      if (ladjSlot != 0) {
	game->last_event->ladj_align = ladjAlign;
	game->last_event->ladj_slot = ladjSlot;
	ladjAlign = 0;
	ladjSlot = 0;
      }

      if (itbBase != 0) {
	game->last_event->itb_base = itbBase;
	game->last_event->itb_runner_id = (char *) malloc(strlen(itbRunner)+1);
	strcpy(game->last_event->itb_runner_id, itbRunner);
	itbBase = 0;
	strcpy(itbRunner, "");
      }
    }
    else if (!strcmp(tok, "sub")) {
      char *player_id, *name, *team, *slot, *pos;
      player_id = cw_strtok(NULL);
      name = cw_strtok(NULL);
      team = cw_strtok(NULL);
      slot = cw_strtok(NULL);
      pos = cw_strtok(NULL);
      if (player_id && name && team && slot && pos) {
	cw_game_substitute_append(game, player_id, name,
				  cw_atoi(team), cw_atoi(slot), 
				  cw_atoi(pos));
      }
    }
    else if (!strcmp(tok, "com")) {
      char *comment;
      comment = cw_strtok(NULL);
      if (comment) {
	cw_game_comment_append(game, comment);
      }
    }
    else if (!strcmp(tok, "data")) {
      char *data[256];
      int i;

      for (i = 0; i < 256; i++) {
	data[i] = cw_strtok(NULL);
	if (!data[i]) {
	  cw_game_data_append(game, i, data);
	  break;
	}
      }
    }
    else if (!strcmp(tok, "stat")) {
      char *data[256];
      int i;

      for (i = 0; i < 256; i++) {
	data[i] = cw_strtok(NULL);
	if (!data[i] || isspace(data[i][0])) {
	  cw_game_stat_append(game, i, data);
	  break;
	}
      }
    }
    else if (!strcmp(tok, "event")) {
      char *data[256];
      int i;

      for (i = 0; i < 256; i++) {
	data[i] = cw_strtok(NULL);
	if (!data[i] || isspace(data[i][0])) {
	  cw_game_evdata_append(game, i, data);
	  break;
	}
      }
    }
    else if (!strcmp(tok, "line")) {
      char *data[256];
      int i;

      for (i = 0; i < 256; i++) {
	data[i] = cw_strtok(NULL);
	if (!data[i] || data[i][0] == '\0') {
	  cw_game_line_append(game, i, data);
	  break;
	}
      }
    }
    else if (!strcmp(tok, "badj")) {
      char *batter, *bats;
      batter = cw_strtok(NULL);
      bats = cw_strtok(NULL);
      if (batter && bats) {
	strncpy(batHandBatter, batter, 255);
	batHand = bats[0];
      }
    }
    else if (!strcmp(tok, "padj")) {
      char *pitcher, *throws;
      pitcher = cw_strtok(NULL);
      throws = cw_strtok(NULL);
      if (pitcher && throws) {
	strncpy(pitHandPitcher, pitcher, 255);
	pitHand = throws[0];
      }
    }
    else if (!strcmp(tok, "ladj")) {
      char *align, *slot;
      align = cw_strtok(NULL);
      slot = cw_strtok(NULL);
      if (align && slot) {
	ladjAlign = cw_atoi(align);
	ladjSlot = cw_atoi(slot);
      }      
    }
    else if (!strcmp(tok, "cw:itb")) {
      /* Chadwick extension: international tiebreaker */
      /* Format: cw:itb,runner-id,base */
      char *runner, *base;
      runner = cw_strtok(NULL);
      base = cw_strtok(NULL);
      if (runner && base) {
	strncpy(itbRunner, runner, 255);
	itbBase = cw_atoi(base);
      }
    }      
  }

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
	!strcmp(info->label, "umplf") ||
	!strcmp(info->label, "umprf") ||
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
cw_game_write_comments(CWGame *game, FILE *file)
{
  CWComment *comment = game->first_comment;

  while (comment != NULL) {
    fprintf(file, "com,\"%s\"\n", comment->text);
    comment = comment->next;
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
    if (event->pitcher_hand != ' ') {
      fprintf(file, "padj,%s,%c\n", event->pitcher_hand_id, event->pitcher_hand);
    }
    if (event->ladj_slot != 0) {
      fprintf(file, "ladj,%d,%d\n", event->ladj_align, event->ladj_slot);
    }
    if (event->itb_base != 0) {
      fprintf(file, "cw:itb,%s,%d\n", event->itb_runner_id, event->itb_base);
    }      
    fprintf(file, "play,%d,%d,%s,%s,%s,%s\n",
	    event->inning, event->batting_team,
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
    if (event->first_comment != NULL) {
      CWComment *comment = event->first_comment;
      while (comment != NULL) {
	fprintf(file, "com,\"%s\"\n", comment->text);
	comment = comment->next;
      }
    }
    event = event->next;
  }
}

static void
cw_game_write_stat(CWGame *game, FILE *file)
{
  CWData *data = game->first_stat;
  
  while (data != NULL) {
    int i;

    fprintf(file, "stat");
    for (i = 0; i < data->num_data; i++) {
      fprintf(file, ",%s", data->data[i]);
    }
    fprintf(file, "\n");
    data = data->next;
  }
}

static void
cw_game_write_line(CWGame *game, FILE *file)
{
  CWData *data = game->first_line;
  
  while (data != NULL) {
    int i;

    fprintf(file, "line");
    for (i = 0; i < data->num_data; i++) {
      fprintf(file, ",%s", data->data[i]);
    }
    fprintf(file, "\n");
    data = data->next;
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
  cw_game_write_comments(game, file);
  cw_game_write_events(game, file);
  cw_game_write_stat(game, file);
  cw_game_write_line(game, file);
  cw_game_write_data(game, file);
}

void 
cw_event_comment_append(CWEvent *event, char *text)
{
  CWComment *comment = (CWComment *) malloc(sizeof(CWComment));
  comment->text = (char *) malloc(sizeof(char) * (strlen(text) + 1));
  strcpy(comment->text, text);
  comment->next = NULL;
  comment->prev = event->last_comment;
  if (event->last_comment) {
    event->last_comment->next = comment;
  }
  else {
    event->first_comment = comment;
  }
  event->last_comment = comment;
}




