#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Engine for scanning scorebooks to accumulate statistics
# 
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2005, Ted Turocy (turocy@econ.tamu.edu)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

from libchadwick import *

def FormatAverage(num, den):
    """
    Return a formatted string representing num/den.
    Does not create a leading zero, and prints dashes for
    divide-by-zero.
    """
    if den == 0:  return "   - "
    avg = float(num) / float(den)
    if avg >= 1.0:
        return "%5.3f" % avg
    else:
        return " .%03d" % round(avg*1000)


class BattingAccumulator:
    def __init__(self):
        self.stats = { }

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = cw_game_starter_find(game, t, slot+1)
                if player.player_id not in self.stats:
                    self.stats[player.player_id] = self.NewBattingStats(player)
            
                if game.game_id not in self.stats[player.player_id]["games"]:
                    self.stats[player.player_id]["games"].append(game.game_id)

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.player_id not in self.stats:
                self.stats[rec.player_id] = self.NewBattingStats(rec)

            if game.game_id not in self.stats[rec.player_id]["games"]:
                self.stats[rec.player_id]["games"].append(gameiter.game.game_id)

            rec = rec.next


    def OnEvent(self, game, gameiter):
        event_data = gameiter.event_data
        team = cw_gameiter_get_halfinning(gameiter)
        
        batterId = cw_gameiter_get_player(gameiter, team,
                                          cw_gameiter_num_batters(gameiter, team) % 9 + 1)
        batter = self.stats[batterId]

        if cw_event_is_official_ab(event_data):
            batter["ab"] += 1

        if event_data.event_type == CW_EVENT_SINGLE:
            batter["h"] += 1
        elif event_data.event_type == CW_EVENT_DOUBLE:
            batter["h"] += 1
            batter["2b"] += 1
        elif event_data.event_type == CW_EVENT_TRIPLE:
            batter["h"] += 1
            batter["3b"] += 1
        elif event_data.event_type == CW_EVENT_HOMERUN:
            batter["h"] += 1
            batter["hr"] += 1
        elif event_data.event_type == CW_EVENT_WALK:
            batter["bb"] += 1
        elif event_data.event_type == CW_EVENT_INTENTIONALWALK:
            batter["bb"] += 1
            batter["ibb"] += 1
        elif event_data.event_type == CW_EVENT_STRIKEOUT:
            batter["so"] += 1
        elif event_data.event_type == CW_EVENT_HITBYPITCH:
            batter["hp"] += 1

        batter["bi"] += cw_event_rbi_on_play(event_data)

        if event_data.sh_flag > 0:
            batter["sh"] += 1
        if event_data.sf_flag > 0:
            batter["sf"] += 1
        if event_data.gdp_flag > 0:
            batter["gdp"] += 1

        for base in [1,2,3]:
            if cw_gameiter_get_runner(gameiter, base) == "":
                continue
            
            runner = self.stats[cw_gameiter_get_runner(gameiter, base)]
            destBase = cw_gameiter_get_advancement(gameiter, base)
            if destBase >= 4:
                runner["r"] += 1
            if cw_gameiter_get_sb_flag(gameiter, base) > 0:
                runner["sb"] += 1
            if cw_gameiter_get_cs_flag(gameiter, base) > 0:
                runner["cs"] += 1

        destBase = cw_gameiter_get_advancement(gameiter, 0)
        if destBase >= 4:
            batter["r"] += 1

    def OnEndGame(self, game, gameiter):
        pass

    def NewBattingStats(self, player):
        """
        Generate a new batting stats entry (dictionary)
        for 'player', which is an appearance record.
        """
        return { "id": player.player_id,
                 "name": player.name,
                 "games": [ ],
                 "ab":0, "r":0, "h":0,
                 "2b":0, "3b":0, "hr":0, "bi":0,
                 "bb":0, "ibb":0, "so":0,
                 "gdp":0, "hp":0, "sh":0, "sf":0,
                 "sb":0, "cs":0 }

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if i % 20 == 0:
                s += "\nPlayer                 AVG   SLG   OBP   G  AB   R   H 2B 3B HR RBI  BB IW  SO DP HP SH SF SB CS\n"
            
            s += ("%-20s %s %s %s %3d %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d %2d %2d %2d %2d %2d %2d\n" %
                (stat["name"],
                 FormatAverage(stat["h"], stat["ab"]),
                 FormatAverage(stat["h"] + stat["2b"] +
                               2*stat["3b"] + 3*stat["hr"],
                               stat["ab"]),
                 FormatAverage(stat["h"] + stat["bb"] + stat["hp"],
                               stat["ab"] + stat["bb"] + stat["hp"] + stat["sf"]),
                 len(stat["games"]),
                 stat["ab"], stat["r"], stat["h"],
                 stat["2b"], stat["3b"], stat["hr"],
                 stat["bi"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["gdp"], stat["hp"],
                 stat["sh"], stat["sf"],
                 stat["sb"], stat["cs"]))

        return s

class TeamBattingAccumulator:
    def __init__(self, cwf):
        self.stats = { }
        for team in cwf.IterateTeams():
            self.stats[team.team_id] = self.NewBattingStats(team)

    def OnBeginGame(self, game, gameiter):
        vis = cw_game_info_lookup(game, "visteam")
        self.stats[vis]["games"].append(game.game_id)

        home = cw_game_info_lookup(game, "hometeam")
        self.stats[home]["games"].append(game.game_id)

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        event_data = gameiter.event_data
        team = cw_gameiter_get_halfinning(gameiter)

        if team == 0:
            batter = self.stats[cw_game_info_lookup(game, "visteam")]
        else:
            batter = self.stats[cw_game_info_lookup(game, "hometeam")]

        if cw_event_is_official_ab(event_data):
            batter["ab"] += 1

        if event_data.event_type == CW_EVENT_SINGLE:
            batter["h"] += 1
        elif event_data.event_type == CW_EVENT_DOUBLE:
            batter["h"] += 1
            batter["2b"] += 1
        elif event_data.event_type == CW_EVENT_TRIPLE:
            batter["h"] += 1
            batter["3b"] += 1
        elif event_data.event_type == CW_EVENT_HOMERUN:
            batter["h"] += 1
            batter["hr"] += 1
        elif event_data.event_type == CW_EVENT_WALK:
            batter["bb"] += 1
        elif event_data.event_type == CW_EVENT_INTENTIONALWALK:
            batter["bb"] += 1
            batter["ibb"] += 1
        elif event_data.event_type == CW_EVENT_STRIKEOUT:
            batter["so"] += 1
        elif event_data.event_type == CW_EVENT_HITBYPITCH:
            batter["hp"] += 1

        batter["bi"] += cw_event_rbi_on_play(event_data)

        if event_data.sh_flag > 0:
            batter["sh"] += 1
        if event_data.sf_flag > 0:
            batter["sf"] += 1
        if event_data.gdp_flag > 0:
            batter["gdp"] += 1

        for base in [1,2,3]:
            if cw_gameiter_get_runner(gameiter, base) == "":
                continue
            
            destBase = cw_gameiter_get_advancement(gameiter, base)
            if destBase >= 4:
                batter["r"] += 1
            if cw_gameiter_get_sb_flag(gameiter, base) > 0:
                batter["sb"] += 1
            if cw_gameiter_get_cs_flag(gameiter, base) > 0:
                batter["cs"] += 1

        destBase = cw_gameiter_get_advancement(gameiter, 0)
        if destBase >= 4:
            batter["r"] += 1

    def OnEndGame(self, game, gameiter):
        self.stats[cw_game_info_lookup(game, "visteam")]["lob"] += cw_gameiter_left_on_base(gameiter, 0)
        self.stats[cw_game_info_lookup(game, "hometeam")]["lob"] += cw_gameiter_left_on_base(gameiter, 1)

    def NewBattingStats(self, team):
        """
        Generate a new batting stats entry (dictionary)
        for 'player', which is an appearance record.
        """
        return { "id": team.team_id,
                 "city": team.city, "nickname": team.nickname,
                 "games": [ ],
                 "ab":0, "r":0, "h":0,
                 "2b":0, "3b":0, "hr":0, "bi":0,
                 "bb":0, "ibb":0, "so":0,
                 "gdp":0, "hp":0, "sh":0, "sf":0,
                 "sb":0, "cs":0,
                 "lob":0 }

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if i % 20 == 0:
                s += "\nClub           AVG   SLG   OBP   G   AB   R    H  2B 3B  HR RBI  BB IW   SO  DP HP SH SF  SB CS  LOB\n"

            
            s += ("%-12s %s %s %s %3d %4d %3d %4d %3d %2d %3d %3d %3d %2d %4d %3d %2d %2d %2d %3d %2d %4d\n" %
                (stat["city"],
                 FormatAverage(stat["h"], stat["ab"]),
                 FormatAverage(stat["h"] + stat["2b"] +
                               2*stat["3b"] + 3*stat["hr"],
                               stat["ab"]),
                 FormatAverage(stat["h"] + stat["bb"] + stat["hp"],
                               stat["ab"] + stat["bb"] + stat["hp"] + stat["sf"]),
                 len(stat["games"]),
                 stat["ab"], stat["r"], stat["h"],
                 stat["2b"], stat["3b"], stat["hr"],
                 stat["bi"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["gdp"], stat["hp"],
                 stat["sh"], stat["sf"],
                 stat["sb"], stat["cs"], stat["lob"]))

        return s



class PitchingAccumulator:
    def __init__(self):
        self.stats = { }

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = cw_game_starter_find(game, t, slot+1)

                if player.pos == 1:
                    if player.player_id not in self.stats:
                        self.stats[player.player_id] = self.NewPitchingStats(player)
                    
                    if game.game_id not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.game_id)
                    self.stats[player.player_id]["gs"] += 1

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.pos == 1:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = self.NewPitchingStats(rec)
                if game.game_id not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.game_id)

            rec = rec.next

    def OnEvent(self, game, gameiter):
        team = cw_gameiter_get_halfinning(gameiter)
        pitcherId = cw_gameiter_get_fielder(gameiter, 1-team, 1)
        pitcher = self.stats[pitcherId]

        event_data = gameiter.event_data
        pitcher["outs"] += cw_event_outs_on_play(event_data)

        if event_data.event_type == CW_EVENT_SINGLE:
            pitcher["h"] += 1
        elif event_data.event_type == CW_EVENT_DOUBLE:
            pitcher["h"] += 1
        elif event_data.event_type == CW_EVENT_TRIPLE:
            pitcher["h"] += 1
        elif event_data.event_type == CW_EVENT_HOMERUN:
            pitcher["h"] += 1
            pitcher["hr"] += 1
        elif event_data.event_type == CW_EVENT_WALK:
            pitcher["bb"] += 1
        elif event_data.event_type == CW_EVENT_INTENTIONALWALK:
            pitcher["bb"] += 1
            pitcher["ibb"] += 1
        elif event_data.event_type == CW_EVENT_STRIKEOUT:
            pitcher["so"] += 1
        elif event_data.event_type == CW_EVENT_HITBYPITCH:
            pitcher["hb"] += 1
        elif event_data.event_type == CW_EVENT_BALK:
            pitcher["bk"] += 1

        if event_data.wp_flag > 0:
            pitcher["wp"] += 1

        for base in [1,2,3]:
            if cw_gameiter_get_runner(gameiter, base) == "":
                continue
            
            resppitcher = self.stats[cw_gameiter_get_resp_pitcher(gameiter, base)]
            destBase = cw_gameiter_get_advancement(gameiter, base)
            if destBase >= 4:
                resppitcher["r"] += 1
                if destBase == 4 or destBase == 6:
                    resppitcher["er"] += 1

        destBase = cw_gameiter_get_advancement(gameiter, 0)
        if destBase >= 4:
            pitcher["r"] += 1
            if destBase == 4 or destBase == 6:
                pitcher["er"] += 1

    def OnEndGame(self, game, gameiter):
        if cw_game_info_lookup(game, "wp") != "":
            self.stats[cw_game_info_lookup(game, "wp")]["w"] += 1
        if cw_game_info_lookup(game, "lp") != "":
            self.stats[cw_game_info_lookup(game, "lp")]["l"] += 1
        if cw_game_info_lookup(game, "save") != "":
            self.stats[cw_game_info_lookup(game, "save")]["sv"] += 1

        for t in [0, 1]:
            startP = cw_game_starter_find_by_position(game, t, 1).player_id
            endP = cw_gameiter_get_fielder(gameiter, t, 1)
            if startP == endP:
                # TODO: It's possible but rare to start and end game but
                # not pitch a complete game!
                self.stats[startP]["cg"] += 1
                if cw_gameiter_get_score(gameiter, 1-t) == 0:
                    self.stats[startP]["sho"] += 1
            else:
                self.stats[endP]["gf"] += 1

    def NewPitchingStats(self, player):
        """
        Generate a new pitching stats entry (dictionary)
        for 'player', which is an appearance record.
        """
        return { "id": player.player_id,
                 "name": player.name,
                 "games": [ ],
                 "gs":0, "cg":0, "sho":0, "gf":0,
                 "w":0, "l":0, "sv":0,
                 "outs":0, "r":0, "er":0, "h":0, "hr":0,
                 "bb":0, "ibb":0, "so":0,
                 "wp":0, "bk":0, "hb":0 }

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if stat["outs"] == 0:
                era = "-----"
            else:
                era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)
            if i % 20 == 0:
                s += "\nPlayer                 ERA  G GS CG SH GF  W- L SV    IP   R  ER   H HR  BB IW  SO BK WP HB\n"
            s += ("%-20s %s %2d %2d %2d %2d %2d %2d-%2d %2d %3d.%1d %3d %3d %3d %2d %3d %2d %3d %2d %2d %2d\n" %
                (stat["name"], era,
                 len(stat["games"]),
                 stat["gs"], stat["cg"], stat["sho"],
                 stat["gf"],
                 stat["w"], stat["l"], stat["sv"],
                 stat["outs"] / 3, stat["outs"] % 3,
                 stat["r"], stat["er"],
                 stat["h"], stat["hr"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["bk"], stat["wp"], stat["hb"]))

        return s
        


class TeamPitchingAccumulator:
    def __init__(self, cwf):
        self.stats = { }
        for team in cwf.IterateTeams():
            self.stats[team.team_id] = self.NewPitchingStats(team)

    def OnBeginGame(self, game, gameiter):
        vis = cw_game_info_lookup(game, "visteam")
        self.stats[vis]["games"].append(game.game_id)

        home = cw_game_info_lookup(game, "hometeam")
        self.stats[home]["games"].append(game.game_id)

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        event_data = gameiter.event_data
        team = cw_gameiter_get_halfinning(gameiter)

        if team == 1:
            pitcher = self.stats[cw_game_info_lookup(game, "visteam")]
        else:
            pitcher = self.stats[cw_game_info_lookup(game, "hometeam")]

        pitcher["outs"] += cw_event_outs_on_play(event_data)

        if event_data.event_type == CW_EVENT_SINGLE:
            pitcher["h"] += 1
        elif event_data.event_type == CW_EVENT_DOUBLE:
            pitcher["h"] += 1
        elif event_data.event_type == CW_EVENT_TRIPLE:
            pitcher["h"] += 1
        elif event_data.event_type == CW_EVENT_HOMERUN:
            pitcher["h"] += 1
            pitcher["hr"] += 1
        elif event_data.event_type == CW_EVENT_WALK:
            pitcher["bb"] += 1
        elif event_data.event_type == CW_EVENT_INTENTIONALWALK:
            pitcher["bb"] += 1
            pitcher["ibb"] += 1
        elif event_data.event_type == CW_EVENT_STRIKEOUT:
            pitcher["so"] += 1
        elif event_data.event_type == CW_EVENT_HITBYPITCH:
            pitcher["hb"] += 1
        elif event_data.event_type == CW_EVENT_BALK:
            pitcher["bk"] += 1

        if event_data.wp_flag > 0:
            pitcher["wp"] += 1

        for base in [1,2,3]:
            if cw_gameiter_get_runner(gameiter, base) == "":
                continue
            
            destBase = cw_gameiter_get_advancement(gameiter, base)
            if destBase >= 4:
                pitcher["r"] += 1
                if destBase == 4:
                    pitcher["er"] += 1

        destBase = cw_gameiter_get_advancement(gameiter, 0)
        if destBase >= 4:
            pitcher["r"] += 1
            if destBase == 4:
                pitcher["er"] += 1

    def OnEndGame(self, game, gameiter):
        teams = [ cw_game_info_lookup(game, "visteam"),
                  cw_game_info_lookup(game, "hometeam") ]
        if cw_gameiter_get_score(gameiter, 0) > cw_gameiter_get_score(gameiter, 1):
            self.stats[teams[0]]["w"] += 1
            self.stats[teams[1]]["l"] += 1
            if cw_game_info_lookup(game, "save") != "":
                self.stats[teams[0]]["sv"] += 1
        elif cw_gameiter_get_score(gameiter, 0) < cw_gameiter_get_score(gameiter, 1):
            self.stats[teams[1]]["w"] += 1
            self.stats[teams[0]]["l"] += 1
            if cw_game_info_lookup(game, "save") != "":
                self.stats[teams[1]]["sv"] += 1

        for t in [0, 1]:
            startP = cw_game_starter_find_by_position(game, t, 1).player_id
            endP = cw_gameiter_get_fielder(gameiter, t, 1)
            if startP == endP:
                # TODO: It's possible but rare to start and end game but
                # not pitch a complete game!
                self.stats[teams[t]]["cg"] += 1
            if cw_gameiter_get_score(gameiter, 1-t) == 0:
                self.stats[teams[t]]["sho"] += 1

    def NewPitchingStats(self, team):
        """
        Generate a new pitching stats entry (dictionary)
        for 'team', which is a roster record
        """
        return { "id": team.team_id,
                 "city": team.city, "nickname": team.nickname,
                 "games": [ ],
                 "gs":0, "cg":0, "sho":0, "gf":0,
                 "w":0, "l":0, "sv":0,
                 "outs":0, "r":0, "er":0, "h":0, "hr":0,
                 "bb":0, "ibb":0, "so":0,
                 "wp":0, "bk":0, "hb":0 }

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if stat["outs"] == 0:
                era = "-----"
            else:
                era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)
            if i % 20 == 0:
                s += "\nClub           ERA   G CG SH   W-  L SV     IP   R  ER    H  HR  BB IW   SO BK WP HB\n"
            s += ("%-12s %s %3d %2d %2d %3d-%3d %2d %4d.%1d %3d %3d %4d %3d %3d %2d %4d %2d %2d %2d\n" %
                (stat["city"], era,
                 len(stat["games"]),
                 stat["cg"], stat["sho"],
                 stat["w"], stat["l"], stat["sv"],
                 stat["outs"] / 3, stat["outs"] % 3,
                 stat["r"], stat["er"],
                 stat["h"], stat["hr"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["bk"], stat["wp"], stat["hb"]))

        return s
        


class FieldingAccumulator:
    def __init__(self, pos):
        self.stats = { }
        self.pos = pos

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = cw_game_starter_find(game, t, slot+1)

                if player.pos == self.pos:
                    if player.player_id not in self.stats:
                        self.stats[player.player_id] = self.NewFieldingStats(player)
                    
                    if game.game_id not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.game_id)
                    self.stats[player.player_id]["gs"] += 1

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.pos == self.pos:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = self.NewFieldingStats(rec)
                if game.game_id not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.game_id)

            rec = rec.next


    def OnEvent(self, game, gameiter):
        event_data = gameiter.event_data
        team = cw_gameiter_get_halfinning(gameiter)

        fielderId = cw_gameiter_get_fielder(gameiter, 1-team, self.pos)
        fielder = self.stats[fielderId]

        event_data = gameiter.event_data
        fielder["outs"] += cw_event_outs_on_play(event_data)
        po = cw_gameiter_get_putouts(gameiter, self.pos)
        fielder["po"] += po
        a = cw_gameiter_get_assists(gameiter, self.pos)
        fielder["a"] += a
        fielder["e"] += cw_gameiter_get_fielder_errors(gameiter, self.pos)

        if event_data.dp_flag and po + a > 0:
            fielder["dp"] += 1
        elif event_data.tp_flag and po + a > 0:
            fielder["tp"] += 1

        if event_data.fielded_by == self.pos and cw_event_outs_on_play(event_data) > 0:
            fielder["bf"] += 1
        if (event_data.fielded_by > 0 or
            event_data.event_type in [CW_EVENT_SINGLE,
                                      CW_EVENT_DOUBLE,
                                      CW_EVENT_TRIPLE]):
            fielder["bip"] += 1
            

    def OnEndGame(self, game, gameiter):
        pass

    def NewFieldingStats(self, player):
        """
        Generate a new fielding stats entry (dictionary)
        for 'player', which is an appearance record.
        """
        return { "id": player.player_id,
                 "name": player.name,
                 "games": [ ],
                 "gs":0, "outs":0, "bip":0, "bf":0,
                 "po":0, "a":0, "e":0, "dp":0, "tp":0 }

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        posStr = [ "Pitcher", "Catcher", "First base",
                   "Second base", "Third base",
                   "Shortstop", "Left field", "Center field",
                   "Right field" ][self.pos - 1]
                   
        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if i % 20 == 0:
                s += "\n%-20s   PCT   G  GS    INN   PO   A  E  DP TP  BIP  BF\n" % posStr
            
            s += ("%-20s %s %3d %3d %4d.%1d %4d %3d %2d %3d %2d %4d %3d\n" %
                (stat["name"],
                 FormatAverage(stat["po"] + stat["a"],
                               stat["po"] + stat["a"] + stat["e"]),
                 len(stat["games"]), stat["gs"],
                 stat["outs"] / 3, stat["outs"] % 3,
                 stat["po"], stat["a"], stat["e"],
                 stat["dp"], stat["tp"],
                 stat["bip"], stat["bf"]))

        return s

class TeamFieldingAccumulator:
    def __init__(self, cwf):
        self.stats = { }
        for team in cwf.IterateTeams():
            self.stats[team.team_id] = self.NewFieldingStats(team)

    def OnBeginGame(self, game, gameiter):
        vis = cw_game_info_lookup(game, "visteam")
        self.stats[vis]["games"].append(game.game_id)

        home = cw_game_info_lookup(game, "hometeam")
        self.stats[home]["games"].append(game.game_id)

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        event_data = gameiter.event_data
        team = cw_gameiter_get_halfinning(gameiter)

        if team == 1:
            fielder = self.stats[cw_game_info_lookup(game, "visteam")]
        else:
            fielder = self.stats[cw_game_info_lookup(game, "hometeam")]

        fielder["po"] += cw_event_outs_on_play(event_data)
        fielder["a"] += event_data.num_assists
        fielder["e"] += event_data.num_errors

        if event_data.dp_flag:    fielder["dp"] += 1
        elif event_data.tp_flag:  fielder["tp"] += 1

        if event_data.fielded_by > 0 and cw_event_outs_on_play(event_data) > 0:
            fielder["bf"] += 1
        if (event_data.fielded_by > 0 or
            event_data.event_type in [CW_EVENT_SINGLE,
                                      CW_EVENT_DOUBLE,
                                      CW_EVENT_TRIPLE]):
            fielder["bip"] += 1
            

    def OnEndGame(self, game, gameiter):
        pass

    def NewFieldingStats(self, team):
        """
        Generate a new fielding stats entry (dictionary)
        for 'team', which is a team roster record
        """
        return { "id": team.team_id,
                 "city": team.city, "nickname": team.nickname,
                 "games": [ ],
                 "bip":0, "bf":0,
                 "po":0, "a":0, "e":0, "dp":0, "tp":0 }

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if i % 20 == 0:
                s += "\nClub           PCT   G   PO    A   E  DP TP  BIP   BF\n"
            
            s += ("%-12s %s %3d %4d %4d %3d %3d %2d %4d %4d\n" %
                (stat["city"],
                 FormatAverage(stat["po"] + stat["a"],
                               stat["po"] + stat["a"] + stat["e"]),
                 len(stat["games"]),
                 stat["po"], stat["a"], stat["e"],
                 stat["dp"], stat["tp"],
                 stat["bip"], stat["bf"]))

        return s


class GameLogAccumulator:
    def __init__(self, cwf):
        self.cwf = cwf
        self.stats = { }
        for team in cwf.IterateTeams():
            self.stats[team.team_id] = [ ]

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass
    def OnEvent(self, game, gameiter):  pass

    def OnEndGame(self, game, gameiter):
        ids = [ cw_game_info_lookup(game, "visteam"),
                cw_game_info_lookup(game, "hometeam") ]
        scores = [ cw_gameiter_get_score(gameiter, t) for t in [0,1] ]

        for t in [0,1]:
            self.stats[ids[t]].append({ "date": cw_game_info_lookup(game, "date"),
                                        "number": cw_game_info_lookup(game, "number"),
                                        "teams": ids,
                                        "scores": scores,
                                        "innings": game.last_event.inning,
                                        "wp": cw_game_info_lookup(game, "wp"),
                                        "lp": cw_game_info_lookup(game, "lp"),
                                        "save": cw_game_info_lookup(game, "save") })
   
    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""

        for team in keys:
            self.stats[team].sort(lambda x, y: cmp(x["date"] + x["number"],
                                                   y["date"] + y["number"]))
            roster = self.cwf.GetTeam(team)

            s += "\nGame log for %s %s\n" % (roster.city, roster.nickname)

            wins = 0
            losses = 0
            
            for (i,game) in enumerate(self.stats[team]):
                if i % 20 == 0:
                    s += "\nDate       #  Opponent Result         Record  Win              Loss             Save\n"
                
                if game["wp"] != "":
                    wp = self.cwf.GetPlayer(game["wp"])
                    wpname = wp.first_name[0] + ". " + wp.last_name
                else:
                    wpname = ""
                    
                if game["lp"] != "":
                    lp = self.cwf.GetPlayer(game["lp"])
                    lpname = lp.first_name[0] + ". " + lp.last_name
                else:
                    lpname = ""
                    
                if game["save"] != "":
                    save = self.cwf.GetPlayer(game["save"])
                    savename = save.first_name[0] + ". " + save.last_name
                else:
                    savename = ""

                if game["number"] == "0":
                    game["number"] = " ";

                if game["innings"] == 9:
                    inningStr = ""
                else:
                    inningStr = "(%d)" % game["innings"]
                    
                if game["teams"][0] == team:
                    # Visitors
                    if game["scores"][0] > game["scores"][1]:
                        dec = "W"
                        wins += 1
                    elif game["scores"][0] < game["scores"][1]:
                        dec = "L"
                        losses += 1
                    else:
                        dec = " "
                
                    s += ("%s %s   at %s  %s %2d-%2d %-4s  %3d-%3d  %-16s %-16s %-16s\n" %
                          (game["date"], game["number"],
                           game["teams"][1], dec, 
                           game["scores"][0], game["scores"][1], inningStr,
                           wins, losses, wpname, lpname, savename))
                else:
                    # Home team
                    if game["scores"][0] > game["scores"][1]:
                        dec = "L"
                        losses += 1
                    elif game["scores"][0] < game["scores"][1]:
                        dec = "W"
                        wins += 1
                    else:
                        dec = " "

                    s += ("%s %s   vs %s  %s %2d-%2d %-4s  %3d-%3d  %-16s %-16s %-16s\n" %
                          (game["date"], game["number"],
                           game["teams"][0], dec,
                           game["scores"][1], game["scores"][0], inningStr,
                           wins, losses, wpname, lpname, savename))
                    
            s += "\n"

        return s

class RecordAccumulator:
    def __init__(self, cwf):
        self.stats = { }
        for team in cwf.IterateTeams():
            self.stats[team.team_id] = self.NewTeamStats(team)

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass
    def OnEvent(self, game, gameiter):  pass

    def OnEndGame(self, game, gameiter):
        ids = [ cw_game_info_lookup(game, "visteam"),
                cw_game_info_lookup(game, "hometeam") ]
        scores = [ cw_gameiter_get_score(gameiter, t) for t in [0,1] ]

        self.stats[ids[0]]["g"] += 1
        self.stats[ids[1]]["g"] += 1

        if scores[0] > scores[1]:
            win = 0
        elif scores[1] > scores[0]:
            win = 1
        else:
            win = -1

        if win >= 0:
            self.stats[ids[win]]["w"] += 1
            self.stats[ids[1-win]]["l"] += 1

            if scores[0]-scores[1] in [-1, 1]:
                self.stats[ids[win]]["ow"] += 1
                self.stats[ids[1-win]]["ol"] += 1

            if gameiter.inning >= 10:
                self.stats[ids[win]]["xw"] += 1
                self.stats[ids[1-win]]["xl"] += 1

        if win == 0:
            self.stats[ids[0]]["rw"] += 1
            self.stats[ids[1]]["hl"] += 1
        elif win == 1:
            self.stats[ids[0]]["rl"] += 1
            self.stats[ids[1]]["hw"] += 1


   
    def NewTeamStats(self, team):
        """
        Generate a new team record stats entry (dictionary)
        for 'team', which is a roster record
        """
        return { "id": team.team_id,
                 "city": team.city, "nickname": team.nickname,
                 "g":0, "w":0, "l":0, "t":0,
                 "hw":0, "hl":0,  # home record
                 "rw":0, "rl":0,  # road record
                 "ow":0, "ol":0,  # one-run games record
                 "xw":0, "xl":0}  # extra-inning games record

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = "\nClub                   G   W-  L    PCT  HOME  AWAY  1RUN  XINN\n";
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            s += ("%-20s %3d %3d-%3d  %s %2d-%2d %2d-%2d %2d-%2d %2d-%2d\n" %
                (stat["city"] + " " + stat["nickname"],
                 stat["g"], stat["w"], stat["l"],
                 FormatAverage(stat["w"], stat["w"]+stat["l"]),
                 stat["hw"], stat["hl"],
                 stat["rw"], stat["rl"],
                 stat["ow"], stat["ol"],
                 stat["xw"], stat["xl"]))

        return s

class GrandSlamAccumulator:
    """
    Compiles a list of all grand slams hit.
    """
    def __init__(self, cwf):
        self.cwf = cwf
        self.stats = [ ]

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass

    def OnEvent(self, game, gameiter):
        if (gameiter.event_data.event_type == CW_EVENT_HOMERUN and
            cw_gameiter_get_runner(gameiter, 1) != "" and
            cw_gameiter_get_runner(gameiter, 2) != "" and
            cw_gameiter_get_runner(gameiter, 3) != ""):
            inning = cw_gameiter_get_inning(gameiter)
            halfInning = cw_gameiter_get_halfinning(gameiter)
            if halfInning == 0:
                team = cw_game_info_lookup(game, "visteam")
                opp = cw_game_info_lookup(game, "hometeam")
                site = opp
            else:
                team = cw_game_info_lookup(game, "hometeam")
                opp = cw_game_info_lookup(game, "visteam")
                site = team
                
            self.stats.append({ "date": cw_game_info_lookup(game, "date"),
                                "number": cw_game_info_lookup(game, "number"),
                                
                                "inning": inning,
                                "halfInning": halfInning,
                                "batter": cw_gameiter_get_batter(gameiter),
                                "pitcher": cw_gameiter_get_fielder(gameiter, 1-halfInning, 1),
                                "team": team, "opp": opp, "site": site })
        
    def OnEndGame(self, game, gameiter):  pass
   
    def __str__(self):
        self.stats.sort(lambda x, y: cmp(x["date"] + x["number"],
                                         y["date"] + y["number"]))

        s = "\nDate       # Site Batter               Pitcher           Inning\n"
        for rec in self.stats:
            if rec["number"] == "0":  rec["number"] = " "
            batter = self.cwf.GetPlayer(rec["batter"])
            pitcher = self.cwf.GetPlayer(rec["pitcher"])

            s += ("%s %s %s  %-20s %-20s %d\n" % 
                  (rec["date"], rec["number"], rec["site"],
                   batter.first_name[0] + ". " + batter.last_name + " (" + rec["team"] + ")",
                   pitcher.first_name[0] + ". " + pitcher.last_name + " (" + rec["opp"] + ")",
                   rec["inning"]))
        return s

def ProcessGame(game, acclist):
    gameiter = cw_gameiter_create(game)
    map(lambda x: x.OnBeginGame(game, gameiter), acclist)

    while gameiter.event != None:
        if gameiter.event.event_text == "NP":
            # Note that there exist some Retrosheet files that have subs
            # that aren't preceded by NP...
            map(lambda x: x.OnSubstitution(game, gameiter), acclist)
            cw_gameiter_next(gameiter)
            continue

        map(lambda x: x.OnEvent(game, gameiter), acclist)
        cw_gameiter_next(gameiter)

    map(lambda x: x.OnEndGame(game, gameiter), acclist)
    cw_gameiter_cleanup(gameiter)

def ProcessFile(book, acclist, monitor=None):
    """
    Process the games in scorebook 'book' through the list of
    accumulators in 'acclist'.  Instrumented so that if
    'monitor' is None, progress indications (via calls to
    monitor.Update) are given -- thus the wxWidgets wxProgressDialog
    automatically works for this parameter.
    """
    numGames = book.NumGames()
    for (i,game) in enumerate(book.IterateGames()):
        ProcessGame(game, acclist)
        if monitor != None:
            if not monitor.Update(round(float(i)/float(numGames)*100)):
                return False
    return True

if __name__ == "__main__":
    import sys
    import scorebook

    fn = sys.argv[1]

    book = scorebook.ChadwickScorebook()
    book.Read(fn)

    x = [ RecordAccumulator(book),
          TeamBattingAccumulator(book),
          TeamPitchingAccumulator(book),
          TeamFieldingAccumulator(book),
          GameLogAccumulator(book),
          BattingAccumulator(), PitchingAccumulator() ]
    for pos in range(9):
        x.append(FieldingAccumulator(pos+1))
    ProcessFile(book, x)

    for acc in x:
        print acc
        print "\n"
