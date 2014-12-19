/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2013, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/python/chadwick/game.i
 * SWIG interface wrapper for Chadwick Python library - games
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

%{
char *_cw_state_get_runner(CWGameState *state, int base)
{ return state->runners[base]; }

int _cw_event_get_SB(CWEventData *event, int base)
{ return event->sb_flag[base]; }
int _cw_event_get_CS(CWEventData *event, int base)
{ return event->cs_flag[base]; }
int _cw_event_get_PO(CWEventData *event, int base)
{ return event->po_flag[base]; }
int _cw_event_runner_dest(CWEventData *event, int base)
{ return event->advance[base]; }
%}
char *_cw_state_get_runner(CWGameState *state, int base);
int _cw_event_get_SB(CWEventData *event, int base);
int _cw_event_get_CS(CWEventData *event, int base);
int _cw_event_get_PO(CWEventData *event, int base);
int _cw_event_runner_dest(CWEventData *event, int base);

%pythoncode %{
class EventContext(object):
  "An event in a game, with the corresponding game state context."
  def __init__(self, gameiter):
    self.gameiter = gameiter


  @property 
  def substitutes(self):   return self.gameiter.event.substitutes
  @property
  def S_INN(self):         return self.gameiter.state.inning
  @property
  def S_HALF(self):        return self.gameiter.state.batting_team
  @property
  def S_OUT(self):         return self.gameiter.state.outs
  @property
  def S_BALL(self):
    ct = self.gameiter.event.count[0]
    if ct == "?":  return None
    return int(ct)
  @property
  def S_STRIKE(self):
    ct = self.gameiter.event.count[1]
    if ct == "?":  return None
    return int(ct)
  @property
  def S_AWAY(self):       return self.gameiter._get_score(0)
  @property
  def S_HOME(self):       return self.gameiter._get_score(1)

  @property
  def scheduled_batter(self):  return self.gameiter.scheduled_batter
 
  @property
  def key_batter(self):    return self.gameiter.event.batter
  @property
  def key_res_batter(self):
    return cw_gamestate_charged_batter(self.gameiter.state,
                                       self.gameiter.event.batter,
                                       self.gameiter.event_data)
  @property
  def badj(self):
    if self.gameiter.event.batter_hand != ' ':
      return self.gameiter.event.batter_hand
    return None
  @property
  def key_pitcher(self):
    return self.gameiter.GetFielder(1-self.gameiter.state.batting_team, 1)
  @property
  def key_res_pitcher(self):
    return cw_gamestate_charged_pitcher(self.gameiter.state,
                                        self.gameiter.event_data)
  @property
  def padj(self):
    if self.gameiter.event.pitcher_hand != ' ':
      return self.gameiter.event.pitcher_hand
    return None

  @property
  def key_runner1(self):    
    return _cw_state_get_runner(self.gameiter.state, 1) or None
  @property
  def key_runner2(self):
    return _cw_state_get_runner(self.gameiter.state, 2) or None
  @property
  def key_runner3(self):    
    return _cw_state_get_runner(self.gameiter.state, 3) or None

  @property
  def batter_field_pos(self):
    return cw_gamestate_player_position(self.gameiter.state,
                                        self.gameiter.state.batting_team,
                                        self.key_batter)
  @property
  def runner1_field_pos(self):
    if self.key_runner1 is None: return None
    return cw_gamestate_player_position(self.gameiter.state,
                                        self.gameiter.state.batting_team,
                                        self.key_runner1)
  @property
  def runner2_field_pos(self):
    if self.key_runner2 is None: return None
    return cw_gamestate_player_position(self.gameiter.state,
                                        self.gameiter.state.batting_team,
                                        self.key_runner2)
  @property
  def runner3_field_pos(self):
    if self.key_runner3 is None: return None
    return cw_gamestate_player_position(self.gameiter.state,
                                        self.gameiter.state.batting_team,
                                        self.key_runner3)
  
    
  @property
  def R_TEXT(self):     return self.gameiter.event.event_text
  @property
  def Q_LEAD(self):     return self.gameiter.state.is_leadoff != 0
  @property
  def Q_PH(self):       return self.gameiter.state.ph_flag != 0
  @property
  def R_TYPE(self):     return self.gameiter.event_data.event_type
  @property
  def batter_dest(self):
    return _cw_event_runner_dest(self.gameiter.event_data, 0)
  @property
  def runner1_dest(self):
    return _cw_event_runner_dest(self.gameiter.event_data, 1)
  @property
  def runner2_dest(self):
    return _cw_event_runner_dest(self.gameiter.event_data, 2)
  @property
  def runner3_dest(self):
    return _cw_event_runner_dest(self.gameiter.event_data, 3)
  

  @property
  def B_PA(self):          
    return cw_event_is_batter(self.gameiter.event_data)
  @property
  def B_AB(self):          
    return cw_event_is_official_ab(self.gameiter.event_data)
  @property
  def B_H(self): 
    if self.gameiter.event_data.event_type in \
       [ EVENT_SINGLE, EVENT_DOUBLE, EVENT_TRIPLE, EVENT_HOMERUN ]:
      return 1
    return 0
  @property
  def B_2B(self):
    return 1 if self.gameiter.event_data.event_type == EVENT_DOUBLE else 0
  @property
  def B_3B(self):
    return 1 if self.gameiter.event_data.event_type == EVENT_TRIPLE else 0
  @property
  def B_HR(self):
    return 1 if self.gameiter.event_data.event_type == EVENT_HOMERUN else 0
  @property
  def B_RBI(self):
    return cw_event_rbi_on_play(self.gameiter.event_data)
  @property
  def B_BB(self):
    if self.gameiter.event_data.event_type in [ EVENT_WALK, EVENT_INTENTIONALWALK ]:
      return 1
    return 0
  @property
  def B_IBB(self):
    return 1 if self.gameiter.event_data.event_type == EVENT_INTENTIONALWALK else 0
  @property
  def B_SO(self):
    return 1 if self.gameiter.event_data.event_type == EVENT_STRIKEOUT else 0
  @property
  def B_GDP(self):   return self.gameiter.event_data.gdp_flag
  @property
  def B_HP(self):
    return 1 if self.gameiter.event_data.event_type == EVENT_HITBYPITCH else 0
  @property
  def B_SH(self):    return self.gameiter.event_data.sh_flag
  @property
  def B_SF(self):    return self.gameiter.event_data.sf_flag
  @property
  def B_SB2(self):   return _cw_event_get_SB(self.gameiter.event_data, 1)
  @property
  def B_SB3(self):   return _cw_event_get_SB(self.gameiter.event_data, 2)
  @property
  def B_SBH(self):   return _cw_event_get_SB(self.gameiter.event_data, 3)
  @property
  def B_CS2(self):   return _cw_event_get_CS(self.gameiter.event_data, 1)
  @property
  def B_CS3(self):   return _cw_event_get_CS(self.gameiter.event_data, 2)
  @property
  def B_CSH(self):   return _cw_event_get_CS(self.gameiter.event_data, 3)
  @property
  def B_PO1(self):   return _cw_event_get_PO(self.gameiter.event_data, 1)
  @property
  def B_PO2(self):   return _cw_event_get_PO(self.gameiter.event_data, 2)
  @property
  def B_PO3(self):   return _cw_event_get_PO(self.gameiter.event_data, 3)
  
  @property
  def P_OUT(self):   return cw_event_outs_on_play(self.gameiter.event_data)
  @property
  def P_WP(self):    return self.gameiter.event_data.wp_flag
  @property
  def F_DP(self):    return self.gameiter.event_data.dp_flag
  @property
  def F_TP(self):    return self.gameiter.event_data.tp_flag
  @property
  def F_PB(self):    return self.gameiter.event_data.pb_flag

  @property
  def R_FIELD(self):   return self.gameiter.event_data.fielded_by
  @property
  def R_TRAJ(self):    return self.gameiter.event_data.batted_ball_type
  @property
  def Q_BUNT(self):    return self.gameiter.event_data.bunt_flag != 0
  @property
  def Q_FOUL(self):    return self.gameiter.event_data.foul_flag != 0
%}

%{
char *_cw_stat_get_field(CWData *data, int index)
{ return data->data[index]; }

void _cw_stat_set_field(CWData *data, int index, char *value)
{
  free(data->data[index]);
  data->data[index] = (char *) malloc(sizeof(char) * (strlen(value)+1));
  strcpy(data->data[index], value);
}
%}
char *_cw_stat_get_field(CWData *data, int index);
void _cw_stat_set_field(CWData *data, int index, char *value);

%pythoncode %{
class BoxscoreBattingLine(object):
  def __init__(self, game, stat):
    self.game = game
    self.stat = stat

  @property
  def key_player(self):  return _cw_stat_get_field(self.stat, 1)
  @property
  def align(self):       return int(_cw_stat_get_field(self.stat, 2))
  @property
  def slot(self):        return int(_cw_stat_get_field(self.stat, 3))
  @property
  def seq(self):         return int(_cw_stat_get_field(self.stat, 4))

  def _get_stat_field(self, index):
    value = _cw_stat_get_field(self.stat, index)
    return int(value) if value != "-1" else None
  def _set_stat_field(self, index, value):
    if value is None: _cw_stat_set_field(self.stat, index, "-1")
    else:             _cw_stat_set_field(self.stat, index, str(value))

  @property
  def B_AB(self):          return self._get_stat_field(5)
  @B_AB.setter
  def B_AB(self, value):   return self._set_stat_field(5, value)

  @property
  def B_R(self):          return self._get_stat_field(6)
  @B_R.setter
  def B_R(self, value):   return self._set_stat_field(6, value)

  @property
  def B_H(self):          return self._get_stat_field(7)
  @B_H.setter
  def B_H(self, value):   return self._set_stat_field(7, value)

  @property
  def B_2B(self):          return self._get_stat_field(8)
  @B_2B.setter
  def B_2B(self, value):   return self._set_stat_field(8, value)

  @property
  def B_3B(self):          return self._get_stat_field(9)
  @B_3B.setter
  def B_3B(self, value):   return self._set_stat_field(9, value)

  @property
  def B_HR(self):          return self._get_stat_field(10)
  @B_HR.setter
  def B_HR(self, value):   return self._set_stat_field(10, value)

  @property
  def B_RBI(self):          return self._get_stat_field(11)
  @B_RBI.setter
  def B_RBI(self, value):   return self._set_stat_field(11, value)

  @property
  def B_SH(self):          return self._get_stat_field(12)
  @B_SH.setter
  def B_SH(self, value):   return self._set_stat_field(12, value)

  @property
  def B_SF(self):          return self._get_stat_field(13)
  @B_SF.setter
  def B_SF(self, value):   return self._set_stat_field(13, value)

  @property
  def B_HP(self):          return self._get_stat_field(14)
  @B_HP.setter
  def B_HP(self, value):   return self._set_stat_field(14, value)

  @property
  def B_BB(self):          return self._get_stat_field(15)
  @B_BB.setter
  def B_BB(self, value):   return self._set_stat_field(15, value)

  @property
  def B_IBB(self):          return self._get_stat_field(16)
  @B_IBB.setter
  def B_IBB(self, value):   return self._set_stat_field(16, value)

  @property
  def B_SO(self):          return self._get_stat_field(17)
  @B_SO.setter
  def B_SO(self, value):   return self._set_stat_field(17, value)

  @property
  def B_SB(self):          return self._get_stat_field(18)
  @B_SB.setter
  def B_SB(self, value):   return self._set_stat_field(18, value)

  @property
  def B_CS(self):          return self._get_stat_field(19)
  @B_CS.setter
  def B_CS(self, value):   return self._set_stat_field(19, value)

  @property
  def B_GDP(self):          return self._get_stat_field(20)
  @B_GDP.setter
  def B_GDP(self, value):   return self._set_stat_field(20, value)

  @property
  def B_XI(self):          return self._get_stat_field(21)
  @B_XI.setter
  def B_XI(self, value):   return self._set_stat_field(21, value)

%}

//==========================================================================
//                 Wrapping and extending CWGame as Game
//==========================================================================

%rename(Game) CWGame;
#if SWIG_VERSION < 0x020000
%extend CWGame {
  CWGame(char *gameID)  
#else
%extend cw_game_struct {
  cw_game_struct(char *gameID)
#endif
{ return cw_game_create(gameID); }

#if SWIG_VERSION < 0x020000
  ~CWGame()             
#else
  ~cw_game_struct()
#endif
{ cw_game_cleanup(self);  free(self); }
  
%pythoncode %{
  def __str__(self):  
    return "<Game [%s], %s at %s>" % (self.key_game, self.away, self.home)

  def __repr__(self):
    with tempfile.TemporaryFile() as f:
      self.write(f)
      f.flush()
      f.seek(0)
      return f.read()
%}

  void write(FILE *f)   { cw_game_write(self, f); }

//-------------------------------------------------------------------------
//                  Identifying properties of a game
//-------------------------------------------------------------------------

  char *_get_version(void) { return self->version; }
  void _set_version(char *version) { cw_game_set_version(self, version); }
%pythoncode %{
  __swig_getmethods__["version"] = _get_version
  __swig_setmethods__["version"] = _set_version
  version = _swig_property(_get_version, _set_version)
%}

%pythoncode %{
  def _get_key_game(self):  return self.game_id
  def _set_key_game(self, k):  self.game_id = k
  __swig_getmethods__["key_game"] = _get_key_game
  __swig_setmethods__["key_game"] = _set_key_game
  key_game = _swig_property(_get_key_game, _set_key_game)

  def _get_date(self):
    val = cw_game_info_lookup(self, "date")
    if val is None:  return None
    return datetime.date(*[ int(x) for x in val.split("/") ])
  def _set_date(self, v):
    cw_game_info_set(self, "date", "%4d/%02d/%02d" % (v.year, v.month, v.day))
  __swig_getmethods__["date"] = _get_date
  __swig_setmethods__["date"] = _set_date
  date = _swig_property(_get_date, _set_date)
%}

  int _get_number(void)  
  { return (int) (cw_game_info_lookup(self, "number")[0] - '0'); }
%pythoncode %{
  def _set_number(self, v): return cw_game_info_set(self, "number", str(v))
  __swig_getmethods__["number"] = _get_number
  __swig_setmethods__["number"] = _set_number
  number = _swig_property(_get_number, _set_number)
%}

  char *_get_away(void)  { return cw_game_info_lookup(self, "visteam"); }
%pythoncode %{
  def _set_away(self, v):  return cw_game_info_set(self, "visteam", v)
  __swig_getmethods__["away"] = _get_away
  __swig_setmethods__["away"] = _set_away
  away = _swig_property(_get_away, _set_away)
%}

  char *_get_home(void)  { return cw_game_info_lookup(self, "hometeam"); }
%pythoncode %{
  def _set_home(self, v):  return cw_game_info_set(self, "hometeam", v)
  __swig_getmethods__["home"] = _get_home
  __swig_setmethods__["home"] = _set_home
  home = _swig_property(_get_home, _set_home)
%}

%pythoncode %{
  @property
  def teams(self):   return [ self.away, self.home ]
%}

  int _get_innings(void)  
  { return (self->last_event != NULL) ? self->last_event->inning : 0; }
%pythoncode %{
  def _set_innings(self, v):  raise AttributeError("can't set attribute")
  __swig_getmethods__["innings"] = _get_innings
  __swig_setmethods__["innings"] = _set_innings
  innings = _swig_property(_get_innings, _set_innings)
%}

//-------------------------------------------------------------------------
//                         General game metadata
//-------------------------------------------------------------------------

%pythoncode %{  
  def _get_metadata(self):
    class Metadata(object):
      def __init__(self, game): self.game = game
      def __getitem__(self, label):
        value = cw_game_info_lookup(self.game, label)
        if value is not None:
          return value
        else:
          return ""
      def __setitem__(self, label, value):
        if value is not None:  value = str(value)
        cw_game_info_set(self.game, label, value)
    return Metadata(self)
  def _set_metadata(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["metadata"] = _get_metadata
  __swig_setmethods__["metadata"] = _set_metadata
  metadata = _swig_property(_get_metadata, _set_metadata)
%}
 

%pythoncode %{
  def _get_phase(self):
    phase = cw_game_info_lookup(self, "cw:phase")
    if phase is None:  return "regular"
    return phase
  def _set_phase(self, v):
    return cw_game_info_set(self, "cw:phase", str(v))
  __swig_getmethods__["phase"] = _get_phase
  __swig_setmethods__["phase"] = _set_phase
  phase = _swig_property(_get_phase, _set_phase)

  def _get_status(self):   
    status = cw_game_info_lookup(self, "cw:status")
    if status is None:  return "F"
    return { "final": "F", "suspended": "S" }[status]
  def _set_status(self, v):   
    if v == "F":
      return cw_game_info_set(self, "cw:status", "final")
    elif v == "S":
      return cw_game_info_set(self, "cw:status", "suspended")
    else:
      raise ValueError("Unknown value '%s' for status" % v)
  __swig_getmethods__["status"] = _get_status
  __swig_setmethods__["status"] = _set_status
  status = _swig_property(_get_status, _set_status)

  def _get_scorer(self):
    v = self.metadata["scorer"]
    if v == "" or v is None:  return None
    return v
  def _set_scorer(self, v):
    if v == "" or v is None:  self.metadata["scorer"] = ""
    else:                     self.metadata["scorer"] = str(v)
  __swig_getmethods__["scorer"] = _get_scorer
  __swig_setmethods__["scorer"] = _set_scorer
  scorer = _swig_property(_get_scorer, _set_scorer)

  def _get_translator(self):
    v = self.metadata["translator"]
    if v == "" or v is None:  return None
    return v
  def _set_translator(self, v):
    if v == "" or v is None:  self.metadata["translator"] = ""
    else:                     self.metadata["translator"] = str(v)
  __swig_getmethods__["translator"] = _get_translator
  __swig_setmethods__["translator"] = _set_translator
  translator = _swig_property(_get_translator, _set_translator)

  def _get_inputter(self):
    v = self.metadata["inputter"]
    if v == "" or v is None:  return None
    return v
  def _set_inputter(self, v):
    if v == "" or v is None:  self.metadata["inputter"] = ""
    else:                     self.metadata["inputter"] = str(v)
  __swig_getmethods__["inputter"] = _get_inputter
  __swig_setmethods__["inputter"] = _set_inputter
  inputter = _swig_property(_get_inputter, _set_inputter)

  def _get_inputtime(self):
    return datetime.datetime.strptime(self.metadata["inputtime"],
	                              "%Y/%m/%d %I:%M%p")
  def _set_inputtime(self, t):
    self.metadata["inputtime"] = t.strftime("%Y/%m/%d %I:%M%p")
  __swig_getmethods__["inputtime"] = _get_inputtime
  __swig_setmethods__["inputtime"] = _set_inputtime
  inputtime = _swig_property(_get_inputtime, _set_inputtime)

  def _get_pitches(self):
    return self.metadata["pitches"]
  def _set_pitches(self, v):
    if v not in [ "none", "count", "pitches" ]:
      raise ValueError("Unknown value '%s' for pitches." % v)
    self.metadata["pitches"] = v
  __swig_getmethods__["pitches"] = _get_pitches
  __swig_setmethods__["pitches"] = _set_pitches
  pitches = _swig_property(_get_pitches, _set_pitches)

  def _get_key_park(self):   return self.metadata["site"]
  def _set_key_park(self, v):  
    self.metadata["site"] = str(v) if v is not None else ""
  __swig_getmethods__["key_park"] = _get_key_park
  __swig_setmethods__["key_park"] = _set_key_park
  key_park = _swig_property(_get_key_park, _set_key_park)

  def _get_is_night(self):
    v = self.metadata["daynight"]
    if v == "day":  return False
    if v == "night":  return True
    return None
  def _set_is_night(self, v):
    if v == True:     self.metadata["daynight"] = "night"
    elif v == False:  self.metadata["daynight"] = "day"
    elif v is None:   self.metadata["daynight"] = "unknown"
    else:  raise ValueError("Unknown value '%s' for is_night." % v)
  __swig_getmethods__["is_night"] = _get_is_night
  __swig_setmethods__["is_night"] = _set_is_night
  is_night = _swig_property(_get_is_night, _set_is_night)
  
  def _get_attendance(self):
    v = self.metadata["attendance"]
    if v == "" or v == "0" or v is None:  return None
    return int(v)
  def _set_attendance(self, v):
    if v is None:
      self.metadata["attendance"] = "0"
      return
    try:
      int(v)
    except ValueError:
      raise ValueError("Invalid value '%s' for attendance." % v)
    self.metadata["attendance"] = str(v)
  __swig_getmethods__["attendance"] = _get_attendance
  __swig_setmethods__["attendance"] = _set_attendance
  attendance = _swig_property(_get_attendance, _set_attendance)

  def _get_start_time(self):
    v = self.metadata["starttime"]
    if v == "0:00" or v is None:  return None
    if "PM" in v:
      offset = 12
    else:
      offset = 0
    v = v.replace("AM", "").replace("PM", "")
    hour, minute = v.split(":")
    if hour == "12":  offset = 0
    return datetime.time(int(hour)+offset, int(minute))
  def _set_start_time(self, v):
    if v is None or v == "" or v == "0:00":
      self.metadata["starttime"] = "0:00"
      return
    if isinstance(v, basestring):
      v = datetime.datetime.strptime(v, "%I:%M%p").time()
    t = v.strftime("%I:%M%p").lstrip("0")
    self.metadata["starttime"] = t
    if self.is_night is None:
      self.is_night = (v >= datetime.time(hour=17, minute=0, second=0))
  __swig_getmethods__["start_time"] = _get_start_time
  __swig_setmethods__["start_time"] = _set_start_time
  start_time = _swig_property(_get_start_time, _set_start_time)

  def _get_duration(self):
    v = self.metadata["timeofgame"]
    if v == "" or v == "0":  return None
    return int(v)
  def _set_duration(self, v):
    if v is None:
      self.metadata["timeofgame"] = "0"
      return
    try:
      int(v)
    except ValueError:
      raise ValueError("Invalid value '%s' for duration." % v)
    self.metadata["timeofgame"] = str(v)
  __swig_getmethods__["duration"] = _get_duration
  __swig_setmethods__["duration"] = _set_duration
  duration = _swig_property(_get_duration, _set_duration)

  def _get_temperature(self):
    if self.metadata["temp"] == "0":  return None
    return int(self.metadata["temp"])
  def _set_temperature(self, v):
    if v is None:
      self.metadata["temp"] = "0"
    else:
      self.metadata["temp"] = str(v)
  __swig_getmethods__["temperature"] = _get_temperature
  __swig_setmethods__["temperature"] = _set_temperature

  def _get_sky(self):
    if self.metadata["sky"] == "unknown": return None
    return self.metadata["sky"]
  def _set_sky(self, v):
    if v is None:  v = "unknown"
    if v not in [ "unknown", "sunny", "cloudy", "overcast",
	          "night", "dome" ]:
      raise ValueError("Unknown value '%s' for sky type." % v)
    self.metadata["sky"] = v
  __swig_getmethods__["sky"] = _get_sky
  __swig_setmethods__["sky"] = _set_sky

  def _get_precipitation(self):
    if self.metadata["precip"] == "unknown": return None
    return self.metadata["precip"]
  def _set_precipitation(self, v):
    if v is None:  v = "unknown"
    if v not in [ "unknown", "none", "drizzle", "showers", "rain", "snow" ]:
      raise ValueError("Unknown value '%s' for precipitation type." % v)
    self.metadata["precip"] = v
  __swig_getmethods__["precipitation"] = _get_precipitation
  __swig_setmethods__["precipitation"] = _set_precipitation

  def _get_wind_speed(self):
    if self.metadata["windspeed"] == "-1":  return None
    return int(self.metadata["windspeed"])
  def _set_wind_speed(self, v):
    if v is None:
      self.metadata["windspeed"] = "-1"
    else:
      self.metadata["windspeed"] = str(v)
  __swig_getmethods__["wind_speed"] = _get_wind_speed
  __swig_setmethods__["wind_speed"] = _set_wind_speed

  def _get_wind_direction(self):
    return self.metadata["winddir"]
  def _set_wind_direction(self, v):
    if v is None:  v = "unknown"
    if v not in [ "unknown", "tolf", "tocf", "torf",
	          "ltor", "fromlf", "fromcf", "fromrf", "rtol",
                  "none", "varies" ]:
      raise ValueError("Unknown value '%s' for wind direction."% v)
    self.metadata["winddir"] = v
  __swig_getmethods__["wind_direction"] = _get_wind_direction
  __swig_setmethods__["wind_direction"] = _set_wind_direction

  def _get_use_dh(self):
    ret = cw_game_info_lookup(self, "usedh")
    if ret is None:  return None
    return ret == "true"
  def _set_use_dh(self, v):
    if v == True:
      cw_game_info_set(self, "usedh", "true")
    elif v == False:
      cw_game_info_set(self, "usedh", "false")
    else:
      raise ValueError("Unknown value '%s' for use_dh." % v)
  __swig_getmethods__["use_dh"] = _get_use_dh
  __swig_setmethods__["use_dh"] = _set_use_dh

  def _get_umpire_hp(self):
    v = self.metadata["umphome"]
    if v == "" or v is None:  return None
    return v
  def _set_umpire_hp(self, v):
    if v == "" or v is None:  self.metadata["umphome"] = ""
    else:                     self.metadata["umphome"] = str(v)
  __swig_getmethods__["umpire_hp"] = _get_umpire_hp
  __swig_setmethods__["umpire_hp"] = _set_umpire_hp

  def _get_umpire_1b(self):
    v = self.metadata["ump1b"]
    if v == "" or v is None:  return None
    return v
  def _set_umpire_1b(self, v):
    if v == "" or v is None:  self.metadata["ump1b"] = ""
    else:                     self.metadata["ump1b"] = str(v)
  __swig_getmethods__["umpire_1b"] = _get_umpire_1b
  __swig_setmethods__["umpire_1b"] = _set_umpire_1b

  def _get_umpire_2b(self):
    v = self.metadata["ump2b"]
    if v == "" or v is None:  return None
    return v
  def _set_umpire_2b(self, v):
    if v == "" or v is None:  self.metadata["ump2b"] = ""
    else:                     self.metadata["ump2b"] = str(v)
  __swig_getmethods__["umpire_2b"] = _get_umpire_2b
  __swig_setmethods__["umpire_2b"] = _set_umpire_2b

  def _get_umpire_3b(self):
    v = self.metadata["ump3b"]
    if v == "" or v is None:  return None
    return v
  def _set_umpire_3b(self, v):
    if v == "" or v is None:  self.metadata["ump3b"] = ""
    else:                     self.metadata["ump3b"] = str(v)
  __swig_getmethods__["umpire_3b"] = _get_umpire_3b
  __swig_setmethods__["umpire_3b"] = _set_umpire_3b

  def _get_umpire_lf(self):
    v = self.metadata["umplf"]
    if v == "" or v is None:  return None
    return v
  def _set_umpire_lf(self, v):
    if v == "" or v is None:  self.metadata["umplf"] = ""
    else:                     self.metadata["umplf"] = str(v)
  __swig_getmethods__["umpire_lf"] = _get_umpire_lf
  __swig_setmethods__["umpire_lf"] = _set_umpire_lf

  def _get_umpire_rf(self):
    v = self.metadata["umprf"]
    if v == "" or v is None:  return None
    return v
  def _set_umpire_rf(self, v):
    if v == "" or v is None:  self.metadata["umprf"] = ""
    else:                     self.metadata["umprf"] = str(v)
  __swig_getmethods__["umpire_rf"] = _get_umpire_rf
  __swig_setmethods__["umpire_rf"] = _set_umpire_rf

  def _get_pitcher_win(self): 
    ret = cw_game_info_lookup(self, "wp")
    return ret if ret != "" else None
  def _set_pitcher_win(self, p):
    cw_game_info_set(self, "wp", p if p is not None else "")
  __swig_getmethods__["pitcher_win"] = _get_pitcher_win
  __swig_setmethods__["pitcher_win"] = _set_pitcher_win
  pitcher_win = _swig_property(_get_pitcher_win, _set_pitcher_win)

  def _get_pitcher_loss(self): 
    ret = cw_game_info_lookup(self, "lp")
    return ret if ret != "" else None
  def _set_pitcher_loss(self, p):
    cw_game_info_set(self, "lp", p if p is not None else "")
  __swig_getmethods__["pitcher_loss"] = _get_pitcher_loss
  __swig_setmethods__["pitcher_loss"] = _set_pitcher_loss
  pitcher_loss = _swig_property(_get_pitcher_loss, _set_pitcher_loss)

  def _get_pitcher_save(self): 
    ret = cw_game_info_lookup(self, "save")
    return ret if ret != "" else None
  def _set_pitcher_save(self, p):
    cw_game_info_set(self, "save", p if p is not None else "")
  __swig_getmethods__["pitcher_save"] = _get_pitcher_save
  __swig_setmethods__["pitcher_save"] = _set_pitcher_save
  pitcher_save = _swig_property(_get_pitcher_save, _set_pitcher_save)
%}


//-------------------------------------------------------------------------
//                 Iteration and computed summary data
//-------------------------------------------------------------------------

%pythoncode %{
  @property
  def events(self):
    gameiter = CWGameIterator(self)
    while not gameiter.at_end:
      yield EventContext(gameiter)
      gameiter.next()
%}

%pythoncode %{
  def _get_state(self):
    if not hasattr(self, "_state"):
      self._state = cw_gameiter_create(self)
      self._state.ToEnd()
    return self._state
  def _set_state(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["state"] = _get_state
  __swig_setmethods__["state"] = _set_state
  state = _swig_property(_get_state, _set_state)
%}	  

%pythoncode %{
  def _get_boxscore(self):
    if not hasattr(self, "_boxscore"):
      self._boxscore = cw_box_create(self)
      self._boxscore.game = self
    return self._boxscore
  def _set_boxscore(self, v): raise AttributeError("can't set attribute")
  __swig_getmethods__["boxscore"] = _get_boxscore
  __swig_setmethods__["boxscore"] = _set_boxscore
  boxscore = _swig_property(_get_boxscore, _set_boxscore)
%}	  


//-------------------------------------------------------------------------
//                         Editing operations
//-------------------------------------------------------------------------

%pythoncode %{
  def set_starter(self, player, name, team, slot, pos):
    cw_game_starter_append(self, player, name, team, slot, pos)
    if pos == 10:  self.use_dh = True
    if self.first_event is None: return
    if hasattr(self, "_state"):  self.state.ToEnd()
    if hasattr(self, "_boxscore"): del self._boxscore
    self._calc_pitcher_er()
  def GetStarter(self, team, slot):
    return cw_game_starter_find(self, team, slot)
  def GetStarterAtPos(self, team, pos):
    return cw_game_starter_find_by_position(self, team, pos)

  def append_event(self, play=None, count=None, pitches=None,
                   batter_hand=None):
    """
    Add a new event to the game, using the game state after the current 
    last event.
    """
    if batter_hand not in [ "R", "r", "L", "l", None ]:
      raise ValueError("Invalid value '%s' for batter hand" % batter_hand)
    cw_game_event_append(self, 
                         self.state.inning, self.state.half_inning,
                         self.state.scheduled_batter, 
                         count if count is not None else "??", 
                         pitches if pitches is not None else "", 
                         play if play is not None else "NP")
    if batter_hand is not None:
      self.last_event.batter_hand = batter_hand.upper()
    self.state.ToEnd()
    if hasattr(self, "_boxscore"): del self._boxscore
    self._calc_pitcher_er()

  def append_substitute(self, player, name, team, slot, pos,
                        count=None, pitches=None):
    cw_game_event_append(self, 
                         self.state.inning, self.state.half_inning,
                         self.state.scheduled_batter, 
                         count if count is not None else "??", 
                         pitches if pitches is not None else "", 
                         "NP")
    cw_game_substitute_append(self, player, name, team, slot, pos)
    self.state.ToEnd()
    if hasattr(self, "_boxscore"):  del self._boxscore
    self._calc_pitcher_er()

  def append_comment(self, text):
    cw_game_comment_append(self, text)

  def delete_last_play(self):
    x = self.last_event
    while x is not None and x.event_text == "NP":  x = x.prev
    if x is not None:
      cw_game_truncate(self, x)
      if hasattr(self, "_state"):  self.state.ToEnd()
      if hasattr(self, "_boxscore"): del self._boxscore
      self._calc_pitcher_er()


  def replace_player(self, key_old, key_new):
    cw_game_replace_player(self, key_old, key_new)

  def get_player_name(self, key):
    app = self.first_starter
    while app is not None:
      if app.player_id == key:  return app.name
      app = app.next
    event = self.first_event
    while event is not None:
      app = event.first_sub
      while app is not None:
        if app.player_id == key:  return app.name
        app = app.next
      event = event.next
    return None
%}

//-------------------------------------------------------------------------
//               Editing operations for boxscore event files
//-------------------------------------------------------------------------

  void _append_bline(char *batter, int align, int slot, int seq)  {
    int i;
    char **data;
    data = (char **) malloc(sizeof(char *) * 22);
    data[0] = (char *) malloc(sizeof(char) * 6);
    strcpy(data[0], "bline");
    data[1] = (char *) malloc(sizeof(char) * (strlen(batter)+1));
    strcpy(data[1], batter);
    for (i = 2; i < 22; i++) {
      data[i] = (char *) malloc(sizeof(char) * 10);
    }
    sprintf(data[2], "%d", align);
    sprintf(data[3], "%d", slot);
    sprintf(data[4], "%d", seq);
    for (i = 5; i < 22; i++) {
      sprintf(data[i], "-1");
    }
    cw_game_stat_append(self, 22, data);
    for (i = 0; i < 22; i++) {
      free(data[i]);
    }
    free(data);
  }

%pythoncode %{
  def add_bline(self, batter, align, slot, seq):
    self._append_bline(batter, align, slot, seq)
    return BoxscoreBattingLine(self, self.last_stat)
%}

%pythoncode %{
  def lint(self):  return True if cw_game_lint(self) else False

  def _calc_pitcher_er(self):
    for t in [0, 1]:
      for pitcher in self.boxscore.players(t):
        if pitcher.P_G == 1:
          cw_game_data_set_er(self, pitcher.player_id, pitcher.P_ER)
%}
};



//==========================================================================
//            Wrapping and extending CWAppearance as Appearance
//==========================================================================

%rename(Appearance) CWAppearance;
#if SWIG_VERSION < 0x020000
%extend CWAppearance {
#else
%extend cw_appearance_struct {
#endif
  int _get_position(void) { return self->pos; }
%pythoncode %{
  def _set_position(self, pos):  self.pos = int(pos)
  __swig_getmethods__["position"] = _get_position
  __swig_setmethods__["position"] = _set_position
  position = _swig_property(_get_position, _set_position)
%}

  char *_get_key_player(void)   { return self->player_id; }
  void _set_key_player(char *id) {
    free(self->player_id);
    self->player_id = (char *) malloc(sizeof(char) * (strlen(id)+1));
    strcpy(self->player_id, id);
  }
%pythoncode %{
  __swig_getmethods__["key_player"] = _get_key_player
  __swig_setmethods__["key_player"] = _set_key_player
  key_player = _swig_property(_get_key_player, _set_key_player)
%}

  char *_get_name(void)   { return self->name; }
  void _set_name(char *name) {
    free(self->name);
    self->name = (char *) malloc(sizeof(char) * (strlen(name)+1));
    strcpy(self->name, name);
  }
%pythoncode %{
  __swig_getmethods__["name"] = _get_name
  __swig_setmethods__["name"] = _set_name
  name = _swig_property(_get_name, _set_name)
%}

  
};

//==========================================================================
//                  Wrapping and extending CWEvent as Event
//==========================================================================

%rename(Event) CWEvent;
#if SWIG_VERSION < 0x020000
%extend CWEvent {
#else
%extend cw_event_struct {
#endif
%pythoncode %{
  @property
  def substitutes(self):
    x = self.first_sub
    while x is not None:
      yield x
      x = x.next

  @property
  def comments(self):
    x = self.first_comment
    while x is not None:
      yield x
      x = x.next

  def _get_description(self):
    for comment in self.comments:
      if comment.text.startswith("@description="):
        return comment.text.split("=", 1)[1]
    return None
  def _set_description(self, desc):
    for comment in self.comments:
      if comment.text.startswith("@description="):
        comment.text = "@description=%s" % str(desc)
        return
    cw_event_comment_append(self, "@description=%s" % str(desc))
  __swig_getmethods__["description"] = _get_description
  __swig_setmethods__["description"] = _set_description
  description = _swig_property(_get_description, _set_description)
%}
};

%pythoncode %{
def create_game(key_game, game_date, game_number, away, home):
    """
    Creates a new Chadwick game object, filling in
    informational fields as appropriate.
    """
    game = cw_game_create(key_game)
    game.version = "2"
    game.metadata["inputprogvers"] = "Chadwick version 0.7.0"
    game.away = away
    game.home = home
    game.date = game_date
    game.number = game_number

    # Fill in dummy values for other info fields
    game.start_time = None
    game.is_night = None
    game.key_park = None
    game.use_dh = False
    game.umpire_hp = None
    game.umpire_1b = None
    game.umpire_2b = None
    game.umpire_3b = None
    game.umpire_lf = None
    game.umpire_rf = None
    game.scorer = None
    game.translator = None
    game.inputter = None
    game.inputtime = datetime.datetime.now()
    game.metadata["howscored"] = "unknown"
    game.pitches = "none"
    game.temperature = None
    game.wind_direction = None
    game.wind_speed = None
    game.metadata["fieldcond"] = "unknown"
    game.precipitation = None
    game.sky = None
    game.duration = None
    game.attendance = None
    game.pitcher_win = None
    game.pitcher_loss = None
    game.pitcher_save = None
    return game

def read_game(f):  
  "Read a game from file object 'f'."
  return cw_game_read(f)

%}
