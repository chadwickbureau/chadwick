/*
 * This file is part of Chadwick
 * Copyright (c) 2002-2014, Dr T L Turocy (ted.turocy@gmail.com)
 *                          Chadwick Baseball Bureau (http://www.chadwick-bureau.com)
 *
 * FILE: src/python/chadwick/boxscore.i
 * SWIG interface wrapper for Chadwick Python library - boxscore
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
//              Wrapping and extending CWBoxscore as Boxscore
//==========================================================================

#if SWIG_VERSION < 0x020000
%extend CWBoxPlayer {
#else
%extend cw_box_player_struct {
#endif
  CWBoxFielding *_get_fielding_at_pos(int pos) { return self->fielding[pos]; }
  int _get_position(int seq)  { return self->positions[seq]; }
};


%pythoncode %{
class BoxPlayer(object):
  def __init__(self, box, box_player):
    self._box = box
    self._box_player = box_player

  @property
  def player_id(self):    return self._box_player.player_id
  @property
  def key_player(self):   return self._box_player.player_id
  @property
  def name_full(self):    return self._box_player.name

  @property
  def pos(self):
    lookup = [ "", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf",
               "dh", "ph", "pr" ]
    return "-".join([ lookup[self._box_player._get_position(i)]
                      for i in xrange(self._box_player.num_positions) ])
  @property
  def B_G_DH(self):  return "dh" in self.pos
  @property
  def B_G_PH(self):  return "ph" in self.pos
  @property
  def B_G_PR(self):  return "pr" in self.pos

  @property
  def order_slot(self):
    for t in [ 0, 1 ]:
      for slot in range(1, 10) + [ 0 ]:
        p = cw_box_get_starter(self._box, t, slot) 
        while p is not None:
          if p.player_id == self._box_player.player_id:
            return slot
          p = p.next
    return None

  @property 
  def order_seq(self):  
    for t in [ 0, 1 ]:
      for slot in range(1, 10) + [ 0 ]:
        p = cw_box_get_starter(self._box, t, slot) 
        seq = 1
        while p is not None:
          if p.player_id == self._box_player.player_id:
            return seq
          p = p.next
          seq += 1
    return None

  def _batting_stat(self, attr):
    return getattr(self._box_player.batting, attr)
  def _fielding_stat(self, pos, attr):
    fielding = self._box_player._get_fielding_at_pos(pos)    
    if fielding is not None:  return getattr(fielding, attr)
    return 0
  def _pitching_stat(self, attr):
    if self.F_P_G == 0:  
        # This takes care of the case of a player re-entry: a player
        # might pitch, be removed, and then re-appear later as a 
        # courtesy runner (or illegal substitution).  We only report
        # pitching stats against the boxscore entry in which he pitched
        return 0
    value = 0
    for t in [ 0, 1 ]:
      for pitcher in self._box.teams[t]._pitching_records:
        if pitcher.player_id == self.key_player:
          value += getattr(pitcher.pitching, attr)
    return value
  @property
  def B_G(self):    return self._batting_stat("g")
  @property
  def B_PA(self):   return self._batting_stat("pa")
  @property
  def B_AB(self):   return self._batting_stat("ab")
  @property
  def B_R(self):    return self._batting_stat("r")
  @property
  def B_H(self):    return self._batting_stat("h")
  @property
  def B_TB(self):   return self.B_H+self.B_2B+2*self.B_3B+3*self.B_HR
  @property
  def B_2B(self):   return self._batting_stat("b2")
  @property
  def B_3B(self):   return self._batting_stat("b3")
  @property
  def B_HR(self):   return self._batting_stat("hr")
  @property
  def B_HR4(self):  return self._batting_stat("hrslam")
  @property
  def B_RBI(self):  return self._batting_stat("bi")
  @property
  def B_BB(self):   return self._batting_stat("bb")
  @property
  def B_IBB(self):  return self._batting_stat("ibb")
  @property
  def B_SO(self):   return self._batting_stat("so")
  @property
  def B_GDP(self):  return self._batting_stat("gdp")
  @property
  def B_HP(self):   return self._batting_stat("hp")
  @property
  def B_SH(self):   return self._batting_stat("sh")
  @property
  def B_SF(self):   return self._batting_stat("sf")
  @property
  def B_SB(self):   return self._batting_stat("sb")
  @property
  def B_CS(self):   return self._batting_stat("cs")
  @property
  def B_XI(self):   return self._batting_stat("xi")
  @property
  def B_PITCH(self):   
    if self._box.game.pitches == "pitches":
      return self._batting_stat("pitches")
    else:
      return None
  @property
  def B_BALL(self):
    if self.B_PITCH is not None:
      return self.B_PITCH - self.B_STRIKE
    else:
      return None
  @property
  def B_STRIKE(self):   
    if self._box.game.pitches == "pitches":
      return self._batting_stat("strikes")
    else:
      return None
  
  @property
  def F_P_G(self):    return self._fielding_stat(1, "g")
  @property
  def F_P_OUT(self):  return self._fielding_stat(1, "outs")
  @property
  def F_P_BIP(self):  return self._fielding_stat(1, "bip")
  @property
  def F_P_BF(self):   return self._fielding_stat(1, "bf")
  @property
  def F_P_TC(self):   return self.F_P_PO+self.F_P_A+self.F_P_E
  @property
  def F_P_PO(self):   return self._fielding_stat(1, "po")
  @property
  def F_P_A(self):    return self._fielding_stat(1, "a")
  @property
  def F_P_E(self):    return self._fielding_stat(1, "e")
  @property
  def F_P_DP(self):   return self._fielding_stat(1, "dp")
  @property
  def F_P_TP(self):   return self._fielding_stat(1, "tp")

  @property
  def F_C_G(self):    return self._fielding_stat(2, "g")
  @property
  def F_C_OUT(self):  return self._fielding_stat(2, "outs")
  @property
  def F_C_BIP(self):  return self._fielding_stat(2, "bip")
  @property
  def F_C_BF(self):   return self._fielding_stat(2, "bf")
  @property
  def F_C_TC(self):   return self.F_C_PO+self.F_C_A+self.F_C_E
  @property
  def F_C_PO(self):   return self._fielding_stat(2, "po")
  @property
  def F_C_A(self):    return self._fielding_stat(2, "a")
  @property
  def F_C_E(self):    return self._fielding_stat(2, "e")
  @property
  def F_C_DP(self):   return self._fielding_stat(2, "dp")
  @property
  def F_C_TP(self):   return self._fielding_stat(2, "tp")
  @property
  def F_C_PB(self):   return self._fielding_stat(2, "pb")
  @property
  def F_C_XI(self):   return self._fielding_stat(2, "xi")

  @property
  def F_1B_G(self):    return self._fielding_stat(3, "g")
  @property
  def F_1B_OUT(self):  return self._fielding_stat(3, "outs")
  @property
  def F_1B_BIP(self):  return self._fielding_stat(3, "bip")
  @property
  def F_1B_BF(self):   return self._fielding_stat(3, "bf")
  @property
  def F_1B_TC(self):   return self.F_1B_PO+self.F_1B_A+self.F_1B_E
  @property
  def F_1B_PO(self):   return self._fielding_stat(3, "po")
  @property
  def F_1B_A(self):    return self._fielding_stat(3, "a")
  @property
  def F_1B_E(self):    return self._fielding_stat(3, "e")
  @property
  def F_1B_DP(self):   return self._fielding_stat(3, "dp")
  @property
  def F_1B_TP(self):   return self._fielding_stat(3, "tp")

  @property
  def F_2B_G(self):    return self._fielding_stat(4, "g")
  @property
  def F_2B_OUT(self):  return self._fielding_stat(4, "outs")
  @property
  def F_2B_BIP(self):  return self._fielding_stat(4, "bip")
  @property
  def F_2B_BF(self):   return self._fielding_stat(4, "bf")
  @property
  def F_2B_TC(self):   return self.F_2B_PO+self.F_2B_A+self.F_2B_E
  @property
  def F_2B_PO(self):   return self._fielding_stat(4, "po")
  @property
  def F_2B_A(self):    return self._fielding_stat(4, "a")
  @property
  def F_2B_E(self):    return self._fielding_stat(4, "e")
  @property
  def F_2B_DP(self):   return self._fielding_stat(4, "dp")
  @property
  def F_2B_TP(self):   return self._fielding_stat(4, "tp")

  @property
  def F_3B_G(self):    return self._fielding_stat(5, "g")
  @property
  def F_3B_OUT(self):  return self._fielding_stat(5, "outs")
  @property
  def F_3B_BIP(self):  return self._fielding_stat(5, "bip")
  @property
  def F_3B_BF(self):   return self._fielding_stat(5, "bf")
  @property
  def F_3B_TC(self):   return self.F_3B_PO+self.F_3B_A+self.F_3B_E
  @property
  def F_3B_PO(self):   return self._fielding_stat(5, "po")
  @property
  def F_3B_A(self):    return self._fielding_stat(5, "a")
  @property
  def F_3B_E(self):    return self._fielding_stat(5, "e")
  @property
  def F_3B_DP(self):   return self._fielding_stat(5, "dp")
  @property
  def F_3B_TP(self):   return self._fielding_stat(5, "tp")

  @property
  def F_SS_G(self):    return self._fielding_stat(6, "g")
  @property
  def F_SS_OUT(self):  return self._fielding_stat(6, "outs")
  @property
  def F_SS_BIP(self):  return self._fielding_stat(6, "bip")
  @property
  def F_SS_BF(self):   return self._fielding_stat(6, "bf")
  @property
  def F_SS_TC(self):   return self.F_SS_PO+self.F_SS_A+self.F_SS_E
  @property
  def F_SS_PO(self):   return self._fielding_stat(6, "po")
  @property
  def F_SS_A(self):    return self._fielding_stat(6, "a")
  @property
  def F_SS_E(self):    return self._fielding_stat(6, "e")
  @property
  def F_SS_DP(self):   return self._fielding_stat(6, "dp")
  @property
  def F_SS_TP(self):   return self._fielding_stat(6, "tp")

  @property
  def F_LF_G(self):    return self._fielding_stat(7, "g")
  @property
  def F_LF_OUT(self):  return self._fielding_stat(7, "outs")
  @property
  def F_LF_BIP(self):  return self._fielding_stat(7, "bip")
  @property
  def F_LF_BF(self):   return self._fielding_stat(7, "bf")
  @property
  def F_LF_TC(self):   return self.F_LF_PO+self.F_LF_A+self.F_LF_E
  @property
  def F_LF_PO(self):   return self._fielding_stat(7, "po")
  @property
  def F_LF_A(self):    return self._fielding_stat(7, "a")
  @property
  def F_LF_E(self):    return self._fielding_stat(7, "e")
  @property
  def F_LF_DP(self):   return self._fielding_stat(7, "dp")
  @property
  def F_LF_TP(self):   return self._fielding_stat(7, "tp")

  @property
  def F_CF_G(self):    return self._fielding_stat(8, "g")
  @property
  def F_CF_OUT(self):  return self._fielding_stat(8, "outs")
  @property
  def F_CF_BIP(self):  return self._fielding_stat(8, "bip")
  @property
  def F_CF_BF(self):   return self._fielding_stat(8, "bf")
  @property
  def F_CF_TC(self):   return self.F_CF_PO+self.F_CF_A+self.F_CF_E
  @property
  def F_CF_PO(self):   return self._fielding_stat(8, "po")
  @property
  def F_CF_A(self):    return self._fielding_stat(8, "a")
  @property
  def F_CF_E(self):    return self._fielding_stat(8, "e")
  @property
  def F_CF_DP(self):   return self._fielding_stat(8, "dp")
  @property
  def F_CF_TP(self):   return self._fielding_stat(8, "tp")

  @property
  def F_RF_G(self):    return self._fielding_stat(9, "g")
  @property
  def F_RF_OUT(self):  return self._fielding_stat(9, "outs")
  @property
  def F_RF_BIP(self):  return self._fielding_stat(9, "bip")
  @property
  def F_RF_BF(self):   return self._fielding_stat(9, "bf")
  @property
  def F_RF_TC(self):   return self.F_RF_PO+self.F_RF_A+self.F_RF_E
  @property
  def F_RF_PO(self):   return self._fielding_stat(9, "po")
  @property
  def F_RF_A(self):    return self._fielding_stat(9, "a")
  @property
  def F_RF_E(self):    return self._fielding_stat(9, "e")
  @property
  def F_RF_DP(self):   return self._fielding_stat(9, "dp")
  @property
  def F_RF_TP(self):   return self._fielding_stat(9, "tp")

  @property
  def F_OF_G(self):    return min(1, self.F_LF_G+self.F_CF_G+self.F_RF_G)
  @property
  def F_OF_OUT(self):  return self.F_LF_OUT+self.F_CF_OUT+self.F_RF_OUT
  @property
  def F_OF_BIP(self):  return self.F_LF_BIP+self.F_CF_BIP+self.F_RF_BIP
  @property
  def F_OF_BF(self):   return self.F_LF_BF+self.F_CF_BF+self.F_RF_BF
  @property
  def F_OF_TC(self):   return self.F_LF_TC+self.F_CF_TC+self.F_RF_TC
  @property
  def F_OF_PO(self):   return self.F_LF_PO+self.F_CF_PO+self.F_RF_PO
  @property
  def F_OF_A(self):    return self.F_LF_A+self.F_CF_A+self.F_RF_A
  @property
  def F_OF_E(self):    return self.F_LF_E+self.F_CF_E+self.F_RF_E
  @property
  def F_OF_DP(self):   return self.F_LF_DP+self.F_CF_DP+self.F_RF_DP
  @property
  def F_OF_TP(self):   return self.F_LF_TP+self.F_CF_TP+self.F_RF_TP

  @property
  def F_G(self):       return min(1, sum([ getattr(self, "F_%s_G" % pos)
                                         for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                      "LF", "CF", "RF" ]]))
  @property
  def F_OUT(self):     return sum([ getattr(self, "F_%s_OUT" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_BIP(self):     return sum([ getattr(self, "F_%s_BIP" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_BF(self):      return sum([ getattr(self, "F_%s_BF" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_TC(self):      return sum([ getattr(self, "F_%s_TC" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_PO(self):      return sum([ getattr(self, "F_%s_PO" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_A(self):       return sum([ getattr(self, "F_%s_A" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_E(self):       return sum([ getattr(self, "F_%s_E" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_DP(self):      return sum([ getattr(self, "F_%s_DP" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_TP(self):      return sum([ getattr(self, "F_%s_TP" % pos)
                                    for pos in [ "P", "C", "1B", "2B", "3B", "SS",
                                                 "LF", "CF", "RF" ]])
  @property
  def F_PB(self):      return self.F_C_PB
  @property
  def F_XI(self):      return self.F_C_XI

  @property
  def P_G(self):    
    # Multiple pitching stints in a game still count only as one game pitched
    return min(self._pitching_stat("g"), 1)
  @property
  def P_GS(self):   return self._pitching_stat("gs")
  @property
  def P_CG(self):   return self._pitching_stat("cg")
  @property
  def P_SHO(self):  return self._pitching_stat("sho")
  @property
  def P_GF(self):   return self._pitching_stat("gf")
  @property
  def P_W(self):    return self._pitching_stat("w")
  @property
  def P_L(self):    return self._pitching_stat("l")
  @property
  def P_SV(self):   return self._pitching_stat("sv")
  @property
  def P_HLD(self):  return None
  @property
  def P_OUT(self):  return self._pitching_stat("outs")
  @property
  def P_TBF(self):  return self._pitching_stat("bf")
  @property
  def P_AB(self):   return self._pitching_stat("ab")
  @property
  def P_R(self):    return self._pitching_stat("r")
  @property
  def P_ER(self):   return self._pitching_stat("er")
  @property
  def P_H(self):    return self._pitching_stat("h")
  @property
  def P_TB(self):   return self.P_H+self.P_2B+2*self.P_3B+3*self.P_HR
  @property
  def P_2B(self):   return self._pitching_stat("b2")
  @property
  def P_3B(self):   return self._pitching_stat("b3")
  @property
  def P_HR(self):   return self._pitching_stat("hr")
  @property
  def P_HR4(self):  return None
  @property
  def P_BB(self):   return self._pitching_stat("bb")
  @property
  def P_IBB(self):  return self._pitching_stat("ibb")
  @property
  def P_SO(self):   return self._pitching_stat("so")
  @property
  def P_GDP(self):  return self._pitching_stat("gdp")
  @property
  def P_HP(self):   return self._pitching_stat("hb")
  @property
  def P_SH(self):   return self._pitching_stat("sh")
  @property
  def P_SF(self):   return self._pitching_stat("sf")
  @property
  def P_WP(self):   return self._pitching_stat("wp")
  @property
  def P_BK(self):   return self._pitching_stat("bk")
  @property
  def P_XI(self):   return None
  @property
  def P_GO(self):   return self._pitching_stat("gb")
  @property
  def P_AO(self):   return self._pitching_stat("fb")
  @property
  def P_IR(self):   return self._pitching_stat("inr")
  @property
  def P_IRS(self):  return self._pitching_stat("inrs")
  @property
  def P_PITCH(self):   
    if self._box.game.pitches == "pitches":
      return self._pitching_stat("pitches")
    else:
      return None
  @property
  def P_BALL(self):
    if self.P_PITCH is not None:
      return self.P_PITCH - self.P_STRIKE
    else:
      return None
  @property
  def P_STRIKE(self):   
    if self._box.game.pitches == "pitches":
      return self._pitching_stat("strikes")
    else:
      return None
%}

%pythoncode %{
class CWBoxscoreTeam(object):
  def __init__(self, box, t):
    self.box = box
    self.t = t

  @property
  def players(self):
    for slot in range(1, 10) + [ 0 ]:
      p = cw_box_get_starter(self.box, self.t, slot) 
      while p is not None:
        yield BoxPlayer(self.box, p)
        p = p.next

  @property
  def pitchers(self):
    for player in self.players:
      if player.P_G == 1:  yield player

  @property
  def _pitching_records(self):
    pitcher = cw_box_get_starting_pitcher(self.box, self.t)
    while pitcher is not None:
      yield pitcher
      pitcher = pitcher.next


  def __getattr__(self, attr):
    if attr[:2] in [ "B_", "P_", "F_" ]:
      try:
        return sum([ getattr(x, attr) for x in self.players ])
      except TypeError:
        return None
    raise AttributeError(attr)

  @property
  def B_LOB(self):  return self.box._get_lob(self.t)
  @property
  def P_GP(self):   return len(list(self.pitchers))
  @property
  def P_ER(self):   return self.box._get_er(self.t)
  @property
  def F_DP(self):   return self.box._get_dp(self.t)
  @property
  def F_TP(self):   return self.box._get_tp(self.t)


%}

%rename(Boxscore) CWBoxscore;
#if SWIG_VERSION < 0x020000
%extend CWBoxscore {
  CWBoxscore(CWGame *game)   
#else
%extend cw_boxscore_struct {
  cw_boxscore_struct(CWGame *game)
#endif
{ return cw_box_create(game); }

#if SWIG_VERSION < 0x020000
  ~CWBoxscore()
#else
  ~cw_boxscore_struct()
#endif
{ cw_box_cleanup(self);  free(self); }

  int _get_lob(int t)  { return self->lob[t]; }
  int _get_er(int t)  { return self->er[t]; }
  int _get_dp(int t)  { return self->dp[t]; }
  int _get_tp(int t)  { return self->tp[t]; }

%pythoncode %{
  def players(self, team):
    for slot in range(1, 10) + [ 0 ]:
      p = cw_box_get_starter(self, team, slot) 
      while p is not None:
        yield BoxPlayer(self, p)
        p = p.next

  def pitchers(self, team):
    for player in self.players(team):
      if player.P_G == 1:  yield player


  @property
  def away(self):     return CWBoxscoreTeam(self.box, 0)
  @property
  def home(self):     return CWBoxscoreTeam(self.box, 1)
  @property
  def teams(self):
    class BoxscoreTeams(object):
      def __init__(self, box):    self.box = box
      def __getitem__(self, t):   return CWBoxscoreTeam(self.box, t)
      def __iter__(self):
         yield CWBoxscoreTeam(self.box, 0)
         yield CWBoxscoreTeam(self.box, 1)
    return BoxscoreTeams(self)
%}

};

