/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2013, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/python/chadwick/roster.i
 * SWIG interface wrapper for Chadwick Python library - league and roster
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

//==========================================================================
//                 Wrapping and extending CWLeague as League
//==========================================================================

%rename(League) CWLeague;
#if SWIG_VERSION < 0x020000
%extend CWLeague {
  CWLeague(void)
#else
%extend cw_league_struct {
  cw_league_struct(void)
#endif
{ return cw_league_create(); }

#if SWIG_VERSION < 0x020000
  ~CWLeague()
#else
  ~cw_league_struct()
#endif
{ cw_league_cleanup(self);  free(self); }  

  void AppendRoster(CWRoster *roster)
     { cw_league_roster_append(self, roster); }
  CWRoster *FindRoster(char *teamID)
     { return cw_league_roster_find(self, teamID); }

%pythoncode %{
  @property
  def teams(self): 
    x = self.first_roster
    while x is not None:  
      yield x
      x = x.next
%}

  void Write(FILE *file)   { cw_league_write(self, file); }
};

%pythoncode %{
def create_league():
  return cw_league_create()

def read_league(f):
  "Read a league from file object 'f'."
  self = cw_league_create()
  cw_league_read(self, f)
  return self
%}

//==========================================================================
//                 Wrapping and extending CWRoster as Roster
//==========================================================================

%rename(Roster) CWRoster;
#if SWIG_VERSION < 0x020000
%extend CWRoster {
  CWRoster(char *team_id, int year, char *league,
           char *city, char *nickname)
#else
%extend cw_roster_struct {
  cw_roster_struct(char *team_id, int year, char *league,
                   char *city, char *nickname)
#endif
{ return cw_roster_create(team_id, year, league, city, nickname); }

%pythoncode %{
  def __str__(self):
    return "<Team [%s] %s %s>" % (self.key_team, self.city, self.name_nick)

  def _get_key_team(self):  return self.team_id
  def _set_key_team(self, v):  self.team_id = v
  __swig_getmethods__["key_team"] = _get_key_team
  __swig_setmethods__["key_team"] = _set_key_team
  key_team = _swig_property(_get_key_team, _set_key_team)
%}

%pythoncode %{
  def _get_name_nick(self):  return self.nickname
  def _set_name_nick(self, v):  self.nickname = v
  __swig_getmethods__["name_nick"] = _get_name_nick
  __swig_setmethods__["name_nick"] = _set_name_nick
  name_nick = _swig_property(_get_name_nick, _set_name_nick)
%}

%pythoncode %{
  def _get_name_full(self):  return self.city + " " + self.name_nick
  def _set_name_full(self, v):  raise AttributeError("can't set attribute")
  __swig_getmethods__["name_full"] = _get_name_full
  __swig_setmethods__["name_full"] = _set_name_full
  name_full = _swig_property(_get_name_full, _set_name_full)
%}

%pythoncode %{
  def InsertPlayer(self, player):
     cw_roster_player_insert(self, player)
     player.thisown = 0

  def AppendPlayer(self, player):
     cw_roster_player_append(self, player)
     player.thisown = 0
%}

%pythoncode %{
  @property
  def players(self): 
    class TeamPlayers(object):
      def __init__(self, team):  self.team = team
      def __iter__(self):
        x = self.team.first_player
        while x is not None:  
          yield x
          x = x.next
      def __getitem__(self, key_player):
        for player in self:
          if player.player_id == key_player:
            return player
        raise KeyError("No player with ID '%s' on team" % key_player)
    return TeamPlayers(self)
%}

  void read(FILE *file)   { cw_roster_read(self, file); }
  void write(FILE *file)  { cw_roster_write(self, file); }

%pythoncode %{
  def Import(self, roster):
    for player in roster.players:
        self.InsertPlayer(Player(player.key_player,
				 player.name_last, player.name_first,
                                 player.bats, player.throws))
%}
};

//==========================================================================
//                Wrapping and extending CWPlayer as Player
//==========================================================================

%rename(Player) CWPlayer;
#if SWIG_VERSION < 0x020000
%extend CWPlayer {
  CWPlayer(char *player_id, char *last_name, char *first_name,
           char bats, char throws)
#else
%extend cw_player_struct {
  cw_player_struct(char *player_id, char *last_name, char *first_name,
                   char bats, char throws)
#endif
{ return cw_player_create(player_id, last_name, first_name,
                          bats, throws); }

%pythoncode %{
  def __str__(self):
    return "<Player [%s] %s %s>" % (self.key_player,
	                            self.name_first, self.name_last)

  def _get_key_player(self):  return self.player_id
  def _set_key_player(self, v):  self.player_id = v
  __swig_getmethods__["key_player"] = _get_key_player
  __swig_setmethods__["key_player"] = _set_key_player
  key_player = _swig_property(_get_key_player, _set_key_player)

  def _get_name_first(self):  return self.first_name
  def _set_name_first(self, v):  self.first_name = v
  __swig_getmethods__["name_first"] = _get_name_first
  __swig_setmethods__["name_first"] = _set_name_first
  name_first = _swig_property(_get_name_first, _set_name_first)

  def _get_name_last(self):  return self.last_name
  def _set_name_last(self, v):  self.last_name = v
  __swig_getmethods__["name_last"] = _get_name_last
  __swig_setmethods__["name_last"] = _set_name_last
  name_last = _swig_property(_get_name_last, _set_name_last)

  def _get_name_full(self):  return self.first_name + " " + self.last_name
  def _set_name_full(self, v):  raise AttributeError("can't set attribute")
  __swig_getmethods__["name_full"] = _get_name_full
  __swig_setmethods__["name_full"] = _set_name_full
  name_full = _swig_property(_get_name_full, _set_name_full)

  def _get_name_sort(self):  return self.last_name + ", " + self.first_name
  def _set_name_sort(self, v):  raise AttributeError("can't set attribute")
  __swig_getmethods__["name_sort"] = _get_name_sort
  __swig_setmethods__["name_sort"] = _set_name_sort
  name_sort = _swig_property(_get_name_sort, _set_name_sort)
%}
};
