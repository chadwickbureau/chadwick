/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * SWIG interface wrapper for Chadwick library
 * 
 * This file is part of Chadwick, a library for baseball play-by-play and stats
 * Copyright (C) 2005, Ted Turocy (turocy@econ.tamu.edu)
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

%module libchadwick

%include file.i

%{
#include <chadwick/chadwick.h>

int cw_gameiter_get_inning(CWGameIterator *iterator)
{
  if (iterator->outs == 3) {
    return iterator->inning + iterator->half_inning;
  }
  else {
    return iterator->inning;
  }
}

int cw_gameiter_get_halfinning(CWGameIterator *iterator)
{
  if (iterator->outs == 3) {
    return (iterator->half_inning + 1) % 2;
  }
  else {
    return iterator->half_inning;
  }
}

char *cw_gameiter_get_batter(CWGameIterator *iterator)
{
  int halfInning = cw_gameiter_get_halfinning(iterator);
  return iterator->lineups[iterator->num_batters[halfInning] % 9 + 1][halfInning].player_id;
}

char *cw_gameiter_get_player(CWGameIterator *iterator, 
                             int team, int slot)
{
  return iterator->lineups[slot][team].player_id;
}

char *cw_gameiter_get_fielder(CWGameIterator *iterator, int team, int pos)
{
  return iterator->fielders[pos][team];
}

int cw_gameiter_num_batters(CWGameIterator *iterator, int team)
{
  return iterator->num_batters[team];
}

int cw_gameiter_get_score(CWGameIterator *iterator, int team)
{
  return iterator->score[team];
}

int cw_gameiter_get_hits(CWGameIterator *iterator, int team)
{
  return iterator->hits[team];
}

int cw_gameiter_get_errors(CWGameIterator *iterator, int team)
{
  return iterator->errors[team];
}

char *cw_gameiter_get_runner(CWGameIterator *iterator, int base)
{
  return iterator->runners[base];
}

char *cw_gameiter_get_resp_pitcher(CWGameIterator *iterator, int base)
{
  return iterator->pitchers[base];
}

int cw_gameiter_get_advancement(CWGameIterator *iterator, int base)
{
  return iterator->event_data->advance[base];
}

int cw_gameiter_get_sb_flag(CWGameIterator *iterator, int base)
{
  return iterator->event_data->sb_flag[base];
}

int cw_gameiter_get_cs_flag(CWGameIterator *iterator, int base)
{
  return iterator->event_data->cs_flag[base];
}

int cw_gameiter_get_putouts(CWGameIterator *iterator, int pos)
{
  return ((iterator->event_data->putouts[0] == pos) ? 1 : 0 +
	  (iterator->event_data->putouts[1] == pos) ? 1 : 0 +
	  (iterator->event_data->putouts[2] == pos) ? 1 : 0);
}

int cw_gameiter_get_assists(CWGameIterator *iterator, int pos)
{
  int count = 0, i;
  for (i = 0; i < iterator->event_data->num_assists; i++) {
    count += (iterator->event_data->assists[i] == pos) ? 1 : 0;
  }
  return count;
}

int cw_gameiter_get_fielder_errors(CWGameIterator *iterator, int pos)
{
  int count = 0, i;
  for (i = 0; i < iterator->event_data->num_errors; i++) {
    count += (iterator->event_data->errors[i] == pos) ? 1 : 0;
  }
  return count;
}

void cw_game_set_er(CWGame *game, char *pitcher, int er)
{
  char **foo = (char **) malloc(4 * sizeof(char *));
  foo[1] = (char *) malloc(3 * sizeof(char));
  strcpy(foo[1], "er");
  foo[2] = (char *) malloc((strlen(pitcher)+1) * sizeof(char));
  strcpy(foo[2], pitcher);
  foo[3] = (char *) malloc(10 * sizeof(char));
  sprintf(foo[3], "%d", er);
  cw_game_data_append(game, 3, foo);
  free(foo[3]);
  free(foo[2]);
  free(foo[1]);
  free(foo);
}

%}

%include <chadwick/chadwick.h>

%include <chadwick/book.h>
%include <chadwick/box.h>
%include <chadwick/file.h>
%include <chadwick/game.h>
%include <chadwick/gameiter.h>
%include <chadwick/league.h>
%include <chadwick/parse.h>
%include <chadwick/roster.h>

int cw_gameiter_get_inning(CWGameIterator *iterator);
int cw_gameiter_get_halfinning(CWGameIterator *iterator);
char *cw_gameiter_get_batter(CWGameIterator *iterator);
char *cw_gameiter_get_player(CWGameIterator *iterator, int team, int slot);
char *cw_gameiter_get_fielder(CWGameIterator *iterator, int team, int pos);
int cw_gameiter_num_batters(CWGameIterator *iterator, int team);
int cw_gameiter_get_score(CWGameIterator *iterator, int team);
int cw_gameiter_get_hits(CWGameIterator *iterator, int team);
int cw_gameiter_get_errors(CWGameIterator *iterator, int team);
char *cw_gameiter_get_runner(CWGameIterator *iterator, int base);
char *cw_gameiter_get_resp_pitcher(CWGameIterator *iterator, int base);
int cw_gameiter_get_advancement(CWGameIterator *iterator, int base);
int cw_gameiter_get_sb_flag(CWGameIterator *iterator, int base);
int cw_gameiter_get_cs_flag(CWGameIterator *iterator, int base);
int cw_gameiter_get_putouts(CWGameIterator *iterator, int pos);
int cw_gameiter_get_assists(CWGameIterator *iterator, int pos);
int cw_gameiter_get_fielder_errors(CWGameIterator *iterator, int pos);
void cw_game_set_er(CWGame *game, char *pitcher, int er);
