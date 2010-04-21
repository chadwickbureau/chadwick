/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2013, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/python/chadwick/state.i
 * SWIG interface wrapper for Chadwick Python library - game state
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
//          Wrapping and extending CWGameIterator as GameIterator
//==========================================================================

%rename(GameIterator) CWGameIterator;
#if SWIG_VERSION < 0x020000
%extend CWGameIterator {
  CWGameIterator(CWGame *game)  
#else
%extend cw_gameiter_struct {
  cw_gameiter_struct(CWGame *game)
#endif
{ return cw_gameiter_create(game); }

#if SWIG_VERSION < 0x020000
  ~CWGameIterator()
#else
  ~cw_gameiter_struct()
#endif
{ cw_gameiter_cleanup(self);  free(self); }

  void next(void) { cw_gameiter_next(self); }
  void ToEnd(void)  
    { cw_gameiter_reset(self); 
      while (self->event != NULL)  cw_gameiter_next(self); } 
  void Reset(void)  { cw_gameiter_reset(self); }
%pythoncode %{
  @property
  def at_end(self):   return self.event is None
%}

  int _get_inning(void) {
    if (self->state->outs == 3) return self->state->inning + self->state->batting_team;
    else return self->state->inning;
  }
%pythoncode %{
  def _set_inning(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["inning"] = _get_inning
  __swig_setmethods__["inning"] = _set_inning
  inning = _swig_property(_get_inning, _set_inning)
%}

  int _get_half_inning(void) {
    if (self->state->outs == 3) return (self->state->batting_team + 1) % 2;
    else return self->state->batting_team;
  }
%pythoncode %{
  def _set_half_inning(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["half_inning"] = _get_half_inning
  __swig_setmethods__["half_inning"] = _set_half_inning
  half_inning = _swig_property(_get_half_inning, _set_half_inning)
%}

  int _get_outs(void) {
    return (self->state->outs < 3) ? self->state->outs : 0;
  }
%pythoncode %{
  def _set_outs(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["outs"] = _get_outs
  __swig_setmethods__["outs"] = _set_outs
  outs = _swig_property(_get_outs, _set_outs)
%}

%pythoncode %{
  def _get_is_leadoff(self):
    return self.game.first_event is None or self.state.outs == 3
  def _set_is_leadoff(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["is_leadoff"] = _get_is_leadoff
  __swig_setmethods__["is_leadoff"] = _set_is_leadoff
  is_leadoff = _swig_property(_get_is_leadoff, _set_is_leadoff)
%}

%pythoncode %{
  def _get_is_game_over(self):
     event = self.game.last_event
     if event is None:  return False
     if (self.inning >= 9 and self.half_inning == 1 and
         self.totals[1].score > self.totals[0].score):
        return True
     if (self.inning >= 10 and self.half_inning == 0 and
         event.inning < self.inning and
         self.totals[0].score > self.totals[1].score):
        return True
     return False
  def _set_is_game_over(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["is_game_over"] = _get_is_game_over
  __swig_setmethods__["is_game_over"] = _set_is_game_over
  is_game_over = _swig_property(_get_is_game_over, _set_is_game_over)
%}   

%pythoncode %{
  def _get_scheduled_batter(self):
     return self.get_lineup_player(self.half_inning,
				   self._get_scheduled_slot(self.half_inning))
  def _set_scheduled_batter(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["scheduled_batter"] = _get_scheduled_batter
  __swig_setmethods__["scheduled_batter"] = _set_scheduled_batter
  scheduled_batter = _swig_property(_get_scheduled_batter, _set_scheduled_batter)
%}

  char *_get_runner(int base)  { return self->state->runners[base]; }
%pythoncode %{  
  def _get_runners(self):
    class Runners(object):
      def __init__(self, state):  self.state = state
      def __getitem__(self, base):
        if base < 0 or base > 3:  raise IndexError
        if self.state.is_leadoff:  return None
        runner = self.state._get_runner(base)
        if runner == "":  return None
        return runner
    return Runners(self)
  def _set_runners(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["runners"] = _get_runners
  __swig_setmethods__["runners"] = _set_runners
  runners = _swig_property(_get_runners, _set_runners)
%}

  char *get_name(char *player_id)  {
    int team, slot;
    for (team = 0; team <= 1; team++) {
      for (slot = 0; slot <= 9; slot++) {
        if (self->state->lineups[slot][team].player_id &&
            !strcmp(self->state->lineups[slot][team].player_id, player_id)) {
          return self->state->lineups[slot][team].name;
        }
      }
    }
    return NULL;
  }

  char *get_lineup_player(int team, int slot)  
  { return self->state->lineups[slot][team].player_id; }
  int get_lineup_position(int team, int slot)
  { return self->state->lineups[slot][team].position; }

  char *GetFielder(int team, int pos)  { return self->state->fielders[pos][team]; }
  int GetPosition(int team, char *playerID)
    { return cw_gamestate_player_position(self->state, team, playerID); }
  int GetSlot(int team, char *playerID)
    { return cw_gamestate_lineup_slot(self->state, team, playerID); }

  char *GetRespPitcher(int base)       { return self->state->pitchers[base]; }

  int _get_score(int team)          { return self->state->score[team]; }
  int _get_pa(int team)             { return self->state->num_batters[team]; }
  int _get_hits(int team)           { return self->state->hits[team]; }
  int _get_errors(int team)         { return self->state->errors[team]; }
  int _get_lob(int team)            { return cw_gamestate_left_on_base(self->state, team); }
  int _get_scheduled_slot(int team) { return self->state->next_batter[team]; }
%pythoncode %{
  @property
  def totals(self):
    class TeamProxy(object):
      def __init__(self, state, team):
        self.state = state
        self.team = team
      @property
      def score(self):   return self.state._get_score(self.team)
      @property
      def r(self):       return self.state._get_score(self.team)
      @property
      def h(self):       return self.state._get_hits(self.team)
      @property
      def e(self):       return self.state._get_errors(self.team)
      @property
      def lob(self):     return self.state._get_lob(self.team)
      @property
      def pa(self):      return self.state._get_pa(self.team)
    class TotalsProxy(object):
      def __init__(self, state):   self.state = state
      def __getitem__(self, team):
        if team not in [0,1]: raise IndexError
        return TeamProxy(self.state, team)
    return TotalsProxy(self)
%}

  CWEvent *GetEvent(void) const  { return self->event; }
  CWEventData *GetEventData(void) const  { return self->event_data; }  
};

//==========================================================================
//           Wrapping and extending CWEventData as EventData
//==========================================================================

%rename(EventData) CWEventData;
#if SWIG_VERSION < 0x020000
%extend CWEventData {
#else
%extend cw_parsed_event_struct {
#endif
  int IsOfficialAB(void)  { return cw_event_is_official_ab(self); }
  int IsBatterEvent(void) { return cw_event_is_batter(self); }
  int GetRBI(void)    { return cw_event_rbi_on_play(self); }

  int GetOuts(void) { return cw_event_outs_on_play(self); }
  int GetAdvance(int base) const  { return self->advance[base]; }
  int GetSBFlag(int base) const   { return self->sb_flag[base]; }
  int GetCSFlag(int base) const   { return self->cs_flag[base]; }
  int GetPOFlag(int base) const   { return self->po_flag[base]; }
  int GetPutouts(int pos) const {
    return (((self->putouts[0] == pos) ? 1 : 0) +
	    ((self->putouts[1] == pos) ? 1 : 0) +
	    ((self->putouts[2] == pos) ? 1 : 0));
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
