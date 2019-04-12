/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2019, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/cwlib/book.c
 * Implementation of scorebook (i.e. collections of games) module.
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

#include "file.h"
#include "game.h"
#include "book.h"

CWScorebook *
cw_scorebook_create(void)
{
  CWScorebook *scorebook = (CWScorebook *) malloc(sizeof(CWScorebook));
  scorebook->first_comment = scorebook->last_comment = NULL;
  scorebook->first_game = scorebook->last_game = NULL;
  return scorebook;
}

void
cw_scorebook_cleanup(CWScorebook *scorebook)
{
  CWGame *game = scorebook->first_game;
  CWComment *comment = scorebook->first_comment;
  
  while (game != NULL) {
    CWGame *next_game = game->next;
    cw_game_cleanup(game);
    free(game);
    game = next_game;
  }

  while (comment != NULL) {
    CWComment *next_comment = comment->next;
    free(comment->text);
    free(comment);
    comment = next_comment;
  }
}

int
cw_scorebook_append_game(CWScorebook *scorebook, CWGame *game)
{
  if (game == NULL) {
    return 0;
  }
  game->prev = scorebook->last_game;
  game->next = NULL;

  if (scorebook->first_game == NULL) {
    scorebook->first_game = game;
  }
  else {
    scorebook->last_game->next = game;
  }
  scorebook->last_game = game;
  return 1;
}

int
cw_scorebook_insert_game(CWScorebook *scorebook, CWGame *game)
{
  if (game == NULL) {
    return 0;
  }

  if (scorebook->first_game == NULL) {
    scorebook->first_game = game;
    scorebook->last_game = game;
  }
  else {
    CWGame *g = scorebook->first_game;
    while (g != NULL &&
	   (strcmp(cw_game_info_lookup(g, "date"),
		   cw_game_info_lookup(game, "date")) < 0 ||
	    (strcmp(cw_game_info_lookup(g, "date"),
		    cw_game_info_lookup(game, "date")) == 0 &&
	     strcmp(cw_game_info_lookup(g, "number"),
		    cw_game_info_lookup(game, "number")) < 0))) {
      g = g->next;
    }

    if (g == NULL) {
      game->prev = scorebook->last_game;
      scorebook->last_game->next = game;
      scorebook->last_game = game;
    }
    else if (g->prev == NULL) {
      scorebook->first_game->prev = game;
      game->next = scorebook->first_game;
      scorebook->first_game = game;
    }
    else {
      game->prev = g->prev;
      game->prev->next = game;
      g->prev = game;
      game->next = g;
    }
  }
  return 1;
}

CWGame *
cw_scorebook_remove_game(CWScorebook *scorebook, char *game_id)
{
  CWGame *game = scorebook->first_game;

  while (game != NULL) {
    if (!strcmp(game->game_id, game_id)) {
      if (game->prev != NULL) {
	game->prev->next = game->next;
      }
      if (game->next != NULL) {
	game->next->prev = game->prev;
      }
      if (scorebook->first_game == game) {
	scorebook->first_game = game->next;
      }
      if (scorebook->last_game == game) {
	scorebook->last_game = game->prev;
      }

      game->prev = game->next = NULL;
      return game;
    }
    game = game->next;
  }
  return NULL;
}

static int
cw_scorebook_read_comments(CWScorebook *scorebook, FILE *file)
{
  while (1) {
    char buf[256], *tok, *com;
    if (fgets(buf, 256, file) == NULL) {
      return 0;
    }

    tok = cw_strtok(buf);
    com = cw_strtok(NULL);
      
    if (tok && !strcmp(tok, "com") && com) {
      CWComment *comment = (CWComment *) malloc(sizeof(CWComment));
      comment->text = (char *) malloc(sizeof(char) * (strlen(com) + 1));
      strcpy(comment->text, com);
      comment->prev = scorebook->last_comment;
      comment->next = NULL;
      if (scorebook->first_comment == NULL) {
	scorebook->first_comment = comment;
      }
      else {
	scorebook->last_comment->next = comment;
      }
      scorebook->last_comment = comment;
    }
    else {
      return 1;
    }
  }
}

int
cw_scorebook_read(CWScorebook *scorebook, FILE *file)
{
  int game_count = 0;

  if (file != NULL) {
    if (!cw_scorebook_read_comments(scorebook, file)) {
      return -1;
    }
    cw_file_find_first_game(file);
    while (!feof(file)) {
      if (!cw_scorebook_append_game(scorebook, cw_game_read(file))) {
	break;
      }
      else {
	game_count++;
      }
    }
    return game_count;
  }
  else {
    return -1;
  }
}

static void
cw_scorebook_write_comments(CWScorebook *scorebook, FILE *file)
{
  CWComment *comment = scorebook->first_comment;
  
  while (comment != NULL) {
    fprintf(file, "com,\"%s\"\n", comment->text);
    comment = comment->next;
  }
}

void
cw_scorebook_write(CWScorebook *scorebook, FILE *file)
{
  CWGame *game = scorebook->first_game;

  cw_scorebook_write_comments(scorebook, file);

  while (game != NULL) {
    cw_game_write(game, file);
    game = game->next;
  }
}

CWScorebookIterator *
cw_scorebook_iterate(CWScorebook *scorebook,
		     int (*f)(CWGame *))
{
  CWScorebookIterator *iter =
    (CWScorebookIterator *) malloc(sizeof(CWScorebookIterator));
  iter->current = scorebook->first_game;
  iter->f = f;
  return iter;
}

void
cw_scorebook_iterator_cleanup(CWScorebookIterator *iterator)
{
  /* For now, no actions need to be taken.
   * This is provided now for possible future extensions and
   * internal implementation changes.
   */
}

CWGame *
cw_scorebook_iterator_next(CWScorebookIterator *iterator)
{
  if (!iterator->current) {
    return NULL;
  }

  while (iterator->current) {
    CWGame *game = iterator->current;
    iterator->current = iterator->current->next;
    if (!iterator->f || (*iterator->f)(game)) {
      return game;
    }
  }
  return NULL;
}
