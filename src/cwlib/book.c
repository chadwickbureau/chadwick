/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * Implementation of scorebook (i.e. collections of games) module.
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

#include "file.h"
#include "game.h"
#include "book.h"

CWScorebook *
cw_scorebook_create(void)
{
  CWScorebook *scorebook = (CWScorebook *) malloc(sizeof(CWScorebook));
  scorebook->first_game = scorebook->last_game = NULL;
  return scorebook;
}

void
cw_scorebook_cleanup(CWScorebook *scorebook)
{
  CWGame *game = scorebook->first_game;
  
  while (game != NULL) {
    CWGame *next_game = game->next;
    cw_game_cleanup(game);
    free(game);
    game = next_game;
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
cw_scorebook_read(CWScorebook *scorebook, char *path)
{
  int game_count = 0;
  FILE *file = fopen(path, "r");

  if (file != NULL) {
    cw_file_find_first_game(file);
    while (!feof(file)) {
      if (!cw_scorebook_append_game(scorebook, cw_game_read(file))) {
	break;
      }
      else {
	game_count++;
      }
    }
    fclose(file);
    return game_count;
  }
  else {
    return -1;
  }
}

void
cw_scorebook_write(CWScorebook *scorebook, FILE *file)
{
  CWGame *game = scorebook->first_game;

  while (game != NULL) {
    cw_game_write(game, file);
    game = game->next;
  }
}
