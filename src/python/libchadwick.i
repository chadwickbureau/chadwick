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

int IsValid(char *play)
{
  CWParsedEvent data;
  return cw_parse_event(play, &data); 
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

%extend CWLeague {
  CWLeague(void)        { return cw_league_create(); }
  ~CWLeague()           { cw_league_cleanup(self);  free(self); }  

  void AppendRoster(CWRoster *roster)
     { cw_league_roster_append(self, roster); }
  CWRoster *FindRoster(char *teamID)
     { return cw_league_roster_find(self, teamID); }

  void Read(FILE *file)    { cw_league_read(self, file); }
  void Write(FILE *file)   { cw_league_write(self, file); }
};

%extend CWScorebook {
  CWScorebook(void)     { return cw_scorebook_create(); }
  ~CWScorebook()        { cw_scorebook_cleanup(self);  free(self); }

  int AppendGame(CWGame *game) { return cw_scorebook_append_game(self, game); }
  int InsertGame(CWGame *game) { return cw_scorebook_insert_game(self, game); }
  int Read(FILE *file)         { return cw_scorebook_read(self, file); }
  void Write(FILE *file)       { cw_scorebook_write(self, file); }
};

%extend CWRoster {
  CWRoster(char *team_id, int year, char league,
           char *city, char *nickname)
    { return cw_roster_create(team_id, year, league, city, nickname); }

  void InsertPlayer(CWPlayer *player)
    { cw_roster_player_insert(self, player); }
  void AppendPlayer(CWPlayer *player)  
    { cw_roster_player_append(self, player); }
  CWPlayer *FindPlayer(char *id)    
    { return cw_roster_player_find(self, id); }
  int NumPlayers(void)    { return cw_roster_player_count(self); }

  void Read(FILE *file)   { cw_roster_read(self, file); }
  void Write(FILE *file)  { cw_roster_write(self, file); }
};

%extend CWGame {
  char *GetGameID(void) { return self->game_id; }
  char *GetDate(void)   { return cw_game_info_lookup(self, "date"); }
  int GetNumber(void)   { return (int) (cw_game_info_lookup(self, "number")[0] - '0'); }
  char *GetTeam(int t) 
    { if (t == 0)    return cw_game_info_lookup(self, "visteam");
      else           return cw_game_info_lookup(self, "hometeam");
    }

%pythoncode %{
  def GetTeams(self):
    return [ self.GetTeam(t) for t in [0,1] ]

  def GetScore(self):
    gameiter = CWGameIterator(self)
    gameiter.ToEnd()
    return [ gameiter.GetTeamScore(t) for t in [0,1] ]
%}

  int GetInnings(void)  
    { if (self->last_event != NULL)  return self->last_event->inning;
      else return 0;
    }

  char *GetInfo(char *label) { return cw_game_info_lookup(self, label); }

  CWAppearance *GetStarter(int team, int slot)
     { return cw_game_starter_find(self, team, slot); }
  CWAppearance *GetStarterAtPos(int team, int pos)
     { return cw_game_starter_find_by_position(self, team, pos); }

  char *GetWinningPitcher(void)   { return cw_game_info_lookup(self, "wp"); }
  char *GetLosingPitcher(void)    { return cw_game_info_lookup(self, "lp"); }
  char *GetSavePitcher(void)      { return cw_game_info_lookup(self, "save"); }
};

%extend CWGameIterator {
  CWGameIterator(CWGame *game)  { return cw_gameiter_create(game); }
  ~CWGameIterator()             { cw_gameiter_cleanup(self);  free(self); }

  void NextEvent(void) { cw_gameiter_next(self); }
  void ToEnd(void)  
    { cw_gameiter_reset(self); while (self->event != NULL)  cw_gameiter_next(self); } 

  int GetInning(void) {
    if (self->outs == 3) return self->inning + self->half_inning;
    else return self->inning;
  }

  int GetHalfInning(void) {
    if (self->outs == 3) return (self->half_inning + 1) % 2;
    else return self->half_inning;
  }

  char *GetBatter(void) {
    int halfInning = self->half_inning;
    if (self->outs == 3) halfInning = (halfInning + 1) % 2;
    return self->lineups[self->num_batters[halfInning] % 9 + 1][halfInning].player_id;
  }

  char *GetPlayer(int team, int slot)  { return self->lineups[slot][team].player_id; }
  char *GetFielder(int team, int pos)  { return self->fielders[pos][team]; }
  char *GetNextBatter(int team)
    { return self->lineups[self->num_batters[team] % 9 + 1][team].player_id; }

  char *GetRunner(int base)            { return self->runners[base]; }
  char *GetRespPitcher(int base)       { return self->pitchers[base]; }

  int NumBatters(int team)             { return self->num_batters[team]; }
  int GetTeamScore(int team)           { return self->score[team]; }
  int GetTeamHits(int team)            { return self->hits[team]; }
  int GetTeamErrors(int team)          { return self->errors[team]; }
};

int cw_gameiter_get_advancement(CWGameIterator *iterator, int base);
int cw_gameiter_get_sb_flag(CWGameIterator *iterator, int base);
int cw_gameiter_get_cs_flag(CWGameIterator *iterator, int base);
int cw_gameiter_get_putouts(CWGameIterator *iterator, int pos);
int cw_gameiter_get_assists(CWGameIterator *iterator, int pos);
int cw_gameiter_get_fielder_errors(CWGameIterator *iterator, int pos);
void cw_game_set_er(CWGame *game, char *pitcher, int er);
int IsValid(char *play);
