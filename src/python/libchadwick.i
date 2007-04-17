/*
 * $Source$
 * $Date$
 * $Revision$
 *
 * DESCRIPTION:
 * SWIG interface wrapper for Chadwick library
 * 
 * This file is part of Chadwick, a library for baseball play-by-play and stats
 * Copyright (C) 2005-2007, Ted Turocy (drarbiter@gmail.com)
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
#include <chadwick.h>

int IsValidPlay(char *play)
{
  CWParsedEvent data;
  return cw_parse_event(play, &data); 
}

%}

%include <chadwick.h>

%include <book.h>
%include <file.h>
%include <game.h>
%include <gameiter.h>
%include <league.h>
%include <parse.h>
%include <roster.h>

//==========================================================================
//                            Renaming constants
//==========================================================================

// Using %rename doesn't work on #defines, so we instead simply define
// the appropriate variables

%constant int EVENT_UNKNOWN         = CW_EVENT_UNKNOWN;
%constant int EVENT_NONE            = CW_EVENT_NONE;
%constant int EVENT_GENERICOUT      = CW_EVENT_GENERICOUT;
%constant int EVENT_STRIKEOUT       = CW_EVENT_STRIKEOUT;
%constant int EVENT_STOLENBASE      = CW_EVENT_STOLENBASE;
%constant int EVENT_INDIFFERENCE    = CW_EVENT_INDIFFERENCE;
%constant int EVENT_CAUGHTSTEALING  = CW_EVENT_CAUGHTSTEALING;
%constant int EVENT_PICKOFFERROR    = CW_EVENT_PICKOFFERROR;
%constant int EVENT_PICKOFF         = CW_EVENT_PICKOFF;
%constant int EVENT_WILDPITCH       = CW_EVENT_WILDPITCH;
%constant int EVENT_PASSEDBALL      = CW_EVENT_PASSEDBALL;
%constant int EVENT_BALK            = CW_EVENT_BALK;
%constant int EVENT_OTHERADVANCE    = CW_EVENT_OTHERADVANCE;
%constant int EVENT_FOULERROR       = CW_EVENT_FOULERROR;
%constant int EVENT_WALK            = CW_EVENT_WALK;
%constant int EVENT_INTENTIONALWALK = CW_EVENT_INTENTIONALWALK;
%constant int EVENT_HITBYPITCH      = CW_EVENT_HITBYPITCH;
%constant int EVENT_INTERFERENCE    = CW_EVENT_INTERFERENCE;
%constant int EVENT_ERROR           = CW_EVENT_ERROR;
%constant int EVENT_FIELDERSCHOICE  = CW_EVENT_FIELDERSCHOICE;
%constant int EVENT_SINGLE          = CW_EVENT_SINGLE;
%constant int EVENT_DOUBLE          = CW_EVENT_DOUBLE;
%constant int EVENT_TRIPLE          = CW_EVENT_TRIPLE;
%constant int EVENT_HOMERUN         = CW_EVENT_HOMERUN;
%constant int EVENT_MISSINGPLAY     = CW_EVENT_MISSINGPLAY;


//==========================================================================
//                 Wrapping and extending CWLeague as League
//==========================================================================

%rename(League) CWLeague;
%extend CWLeague {
  CWLeague(void)        { return cw_league_create(); }
  ~CWLeague()           { cw_league_cleanup(self);  free(self); }  

  void AppendRoster(CWRoster *roster)
     { cw_league_roster_append(self, roster); }
  CWRoster *FindRoster(char *teamID)
     { return cw_league_roster_find(self, teamID); }

%pythoncode %{
  def Teams(self): 
    x = self.first_roster
    while x != None:  yield x; x = x.next
    raise StopIteration
%}

  void Read(FILE *file)    { cw_league_read(self, file); }
  void Write(FILE *file)   { cw_league_write(self, file); }

%pythoncode %{
  def Import(self, league):
    for team in league.Teams():
      if self.FindRoster(team.GetID()) == None:
        self.AppendRoster(team.GetID(), team.GetYear(), team.GetLeague(),
                          team.GetCity(), team.GetNickname())
%}
};

//==========================================================================
//                 Wrapping and extending CWRoster as Roster
//==========================================================================

%rename(Roster) CWRoster;
%extend CWRoster {
  CWRoster(char *team_id, int year, char *league,
           char *city, char *nickname)
    { return cw_roster_create(team_id, year, league, city, nickname); }

  char *GetID(void) const   { return self->team_id; }

  void SetCity(char *name)  { cw_roster_set_city(self, name); }
  char *GetCity(void) const  { return self->city; }

  void SetNickname(char *name)  { cw_roster_set_nickname(self, name); }
  char *GetNickname(void) const  { return self->nickname; }

%pythoncode %{
  def GetName(self):   return self.GetCity() + " " + self.GetNickname()
%}

  void SetLeague(char *name)  { cw_roster_set_league(self, name); }
  char *GetLeague(void) const   { return self->league; }

  void SetYear(int year)   { self->year = year; }
  int GetYear(void) const   { return self->year; }

%pythoncode %{
  def InsertPlayer(self, player):
     cw_roster_player_insert(self, player)
     player.thisown = 0

  def AppendPlayer(self, player):
     cw_roster_player_append(self, player)
     player.thisown = 0
%}

  CWPlayer *FindPlayer(char *id)    
    { return cw_roster_player_find(self, id); }
  int NumPlayers(void)    { return cw_roster_player_count(self); }

%pythoncode %{
  def Players(self): 
    x = self.first_player
    while x != None:  yield x; x = x.next
    raise StopIteration
%}

  void Read(FILE *file)   { cw_roster_read(self, file); }
  void Write(FILE *file)  { cw_roster_write(self, file); }

%pythoncode %{
  def Import(self, roster):
    for player in roster.Players():
        self.InsertPlayer(Player(player.GetID(),
				 player.GetLastName(),
                                 player.GetFirstName(),
                                 player.GetBats(),
                                 player.GetThrows()))
%}
};

//==========================================================================
//                Wrapping and extending CWPlayer as Player
//==========================================================================

%rename(Player) CWPlayer;
%extend CWPlayer {
  CWPlayer(char *player_id, char *last_name, char *first_name,
           char bats, char throws)
    { return cw_player_create(player_id, last_name, first_name,
                              bats, throws); }

  char *GetID(void) const { return self->player_id; }

  void SetFirstName(char *name)  { cw_player_set_first_name(self, name); }
  char *GetFirstName(void) const  { return self->first_name; }

  void SetLastName(char *name)  { cw_player_set_last_name(self, name); }
  char *GetLastName(void) const  { return self->last_name; }

  void SetBats(char bats)    { self->bats = bats; }
  char GetBats(void) const   { return self->bats; }

  void SetThrows(char throws)  { self->throws = throws; }
  char GetThrows(void) const   { return self->throws; }

%pythoncode %{
  def GetName(self):   return self.GetFirstName() + " " + self.GetLastName()
  def GetSortName(self):
    return self.GetLastName() + ", " + self.GetFirstName()

  def GetTeams(self):
    if not hasattr(self, "teams"):  self.teams = [ ]
    return self.teams

  def AddTeam(self, teamID):
    if not hasattr(self, "teams"):  self.teams = [ ]
    self.teams.append(teamID)
    self.teams.sort()
%}
};

//==========================================================================
//            Wrapping and extending CWScorebook as Scorebook
//==========================================================================

%rename(Scorebook) CWScorebook;
%extend CWScorebook {
  CWScorebook(void)     { return cw_scorebook_create(); }
  ~CWScorebook()        { cw_scorebook_cleanup(self);  free(self); }

  int AppendGame(CWGame *game) { return cw_scorebook_append_game(self, game); }
  int InsertGame(CWGame *game) { return cw_scorebook_insert_game(self, game); }
  CWGame *RemoveGame(char *gameID)
    { return cw_scorebook_remove_game(self, gameID); }
  int Read(FILE *file)         { return cw_scorebook_read(self, file); }
  void Write(FILE *file)       { cw_scorebook_write(self, file); }
};

//==========================================================================
//                 Wrapping and extending CWGame as Game
//==========================================================================

%rename(Game) CWGame;
%extend CWGame {
  CWGame(char *gameID)  { return cw_game_create(gameID); }

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
    if not hasattr(self, "scorecache"):
      gameiter = GameIterator(self)
      gameiter.ToEnd()
      self.scorecache = [ gameiter.GetTeamScore(t) for t in [0,1] ]
    return self.scorecache
%}

  int GetInnings(void)  
    { if (self->last_event != NULL)  return self->last_event->inning;
      else return 0;
    }

  void SetVersion(char *version) { cw_game_set_version(self, version); }

  char *GetInfo(char *label) { return cw_game_info_lookup(self, label); }
  void SetInfo(char *label, char *value)
    { cw_game_info_set(self, label, value); }
  void AddInfo(char *label, char *value)
    { cw_game_info_append(self, label, value); }


  void AddStarter(char *player, char *name, int team, int slot, int pos)
    { cw_game_starter_append(self, player, name, team, slot, pos); }

  void AddEvent(int inning, int halfInning, char *batter,
                char *count, char *pitches, char *play)
    { cw_game_event_append(self, inning, halfInning,
                           batter, count, pitches, play); }
  void Truncate(CWEvent *atEvent)  { cw_game_truncate(self, atEvent); }

  void AddSubstitute(char *player, char *name, int team, int slot, int pos)
    { cw_game_substitute_append(self, player, name, team, slot, pos); }

  void AddComment(char *text)  { cw_game_comment_append(self, text); }

  CWAppearance *GetStarter(int team, int slot)
     { return cw_game_starter_find(self, team, slot); }
  CWAppearance *GetStarterAtPos(int team, int pos)
     { return cw_game_starter_find_by_position(self, team, pos); }

  char *GetWinningPitcher(void)   { return cw_game_info_lookup(self, "wp"); }
  char *GetLosingPitcher(void)    { return cw_game_info_lookup(self, "lp"); }
  char *GetSavePitcher(void)      { return cw_game_info_lookup(self, "save"); }

  void SetER(char *pitcher, int er)  {
    char **foo = (char **) malloc(4 * sizeof(char *));
    foo[1] = (char *) malloc(3 * sizeof(char));
    strcpy(foo[1], "er");
    foo[2] = (char *) malloc((strlen(pitcher)+1) * sizeof(char));
    strcpy(foo[2], pitcher);
    foo[3] = (char *) malloc(10 * sizeof(char));
    sprintf(foo[3], "%d", er);
    cw_game_data_append(self, 3, foo);
    free(foo[3]);
    free(foo[2]);
    free(foo[1]);
    free(foo);
  }

};

//==========================================================================
//            Wrapping and extending CWAppearance as Appearance
//==========================================================================

%rename(Appearance) CWAppearance;
%extend CWAppearance {
  char *GetPlayerID(void) const { return self->player_id; }
  char *GetName(void) const  { return self->name; }
  int GetTeam(void) const  { return self->team; }
  int GetSlot(void) const  { return self->slot; }
  int GetPosition(void) const  { return self->pos; }

};

//==========================================================================
//                  Wrapping and extending CWEvent as Event
//==========================================================================

%rename(Event) CWEvent;
%extend CWEvent {
%pythoncode %{
  def Substitutes(self):
    x = self.first_sub
    while x != None:
      yield x
      x = x.next
    raise StopIteration
%}
};

//==========================================================================
//          Wrapping and extending CWGameIterator as GameIterator
//==========================================================================

%rename(GameIterator) CWGameIterator;
%extend CWGameIterator {
  CWGameIterator(CWGame *game)  { return cw_gameiter_create(game); }
  ~CWGameIterator()             { cw_gameiter_cleanup(self);  free(self); }

  void NextEvent(void) { cw_gameiter_next(self); }
  void ToEnd(void)  
    { cw_gameiter_reset(self); 
      while (self->event != NULL)  cw_gameiter_next(self); } 
  void Reset(void)  { cw_gameiter_reset(self); }

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
  int GetPosition(int team, char *playerID)
    { return cw_gameiter_player_position(self, team, playerID); }
  int GetSlot(int team, char *playerID)
    { return cw_gameiter_lineup_slot(self, team, playerID); }
  char *GetNextBatter(int team)
    { return self->lineups[self->num_batters[team] % 9 + 1][team].player_id; }

  char *GetRunner(int base)            { return self->runners[base]; }
  char *GetRespPitcher(int base)       { return self->pitchers[base]; }

  int NumBatters(int team)             { return self->num_batters[team]; }
  int GetTeamScore(int team)           { return self->score[team]; }
  int GetTeamHits(int team)            { return self->hits[team]; }
  int GetTeamErrors(int team)          { return self->errors[team]; }
  int GetTeamLOB(int team)  { return cw_gameiter_left_on_base(self, team); }

  CWEvent *GetEvent(void) const  { return self->event; }
  CWParsedEvent *GetEventData(void) const  { return self->event_data; }  
};

//==========================================================================
//           Wrapping and extending CWParsedEvent as ParsedEvent
//==========================================================================

%rename(ParsedEvent) CWParsedEvent;
%extend CWParsedEvent {
  int IsOfficialAB(void)  { return cw_event_is_official_ab(self); }
  int IsBatterEvent(void) { return cw_event_is_batter(self); }
  int GetRBI(void)    { return cw_event_rbi_on_play(self); }

  int GetOuts(void) { return cw_event_outs_on_play(self); }
  int GetAdvance(int base) const  { return self->advance[base]; }
  int GetSBFlag(int base) const   { return self->sb_flag[base]; }
  int GetCSFlag(int base) const   { return self->cs_flag[base]; }
  int GetPutouts(int pos) const {
    return ((self->putouts[0] == pos) ? 1 : 0 +
	    (self->putouts[1] == pos) ? 1 : 0 +
	    (self->putouts[2] == pos) ? 1 : 0);
  }

  int GetAssists(int pos) const {
    int count = 0, i;
    for (i = 0; i < self->num_assists; i++) {
      count += (self->assists[i] == pos) ? 1 : 0;
    }
    return count;
  }

  int GetErrors(int pos) const {
    int count = 0, i;
    for (i = 0; i < self->num_errors; i++) {
      count += (self->errors[i] == pos) ? 1 : 0;
    }
    return count;
  }
};

int IsValidPlay(char *play);
