#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Engine for scanning scorebooks to accumulate statistics
# 
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2005-2007, Ted Turocy (drarbiter@gmail.com)
#
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 2 of the License, or (at 
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
# for more details.
#
# You should have received a copy of the GNU General Public License along 
# with this program; if not, write to the Free Software Foundation, Inc., 
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
# 

import libchadwick as cw

class BattingStatline:
    def __init__(self):
        self.stats = { "games": [ ],
                       "ab":0, "r":0, "h":0,
                       "2b":0, "3b":0, "hr":0, "bi":0,
                       "bb":0, "ibb":0, "so":0,
                       "gdp":0, "hp":0, "sh":0, "sf":0,
                       "sb":0, "cs":0, "lob":0 }

    def ProcessBatting(self, eventData):
        if eventData.IsOfficialAB(): self.stats["ab"] += 1

        if eventData.event_type == cw.EVENT_SINGLE:
            self.stats["h"] += 1
        elif eventData.event_type == cw.EVENT_DOUBLE:
            self.stats["h"] += 1
            self.stats["2b"] += 1
        elif eventData.event_type == cw.EVENT_TRIPLE:
            self.stats["h"] += 1
            self.stats["3b"] += 1
        elif eventData.event_type == cw.EVENT_HOMERUN:
            self.stats["h"] += 1
            self.stats["hr"] += 1
        elif eventData.event_type == cw.EVENT_WALK:
            self.stats["bb"] += 1
        elif eventData.event_type == cw.EVENT_INTENTIONALWALK:
            self.stats["bb"] += 1
            self.stats["ibb"] += 1
        elif eventData.event_type == cw.EVENT_STRIKEOUT:
            self.stats["so"] += 1
        elif eventData.event_type == cw.EVENT_HITBYPITCH:
            self.stats["hp"] += 1

        self.stats["bi"] += eventData.GetRBI()

        if eventData.sh_flag > 0:
            self.stats["sh"] += 1
        if eventData.sf_flag > 0:
            self.stats["sf"] += 1
        if eventData.gdp_flag > 0:
            self.stats["gdp"] += 1

        if eventData.GetAdvance(0) >= 4: self.stats["r"] += 1

    def ProcessRunning(self, eventData, base):
        destBase = eventData.GetAdvance(base)
        if destBase >= 4:  self.stats["r"] += 1
        if eventData.GetSBFlag(base) > 0: self.stats["sb"] += 1
        if eventData.GetCSFlag(base) > 0: self.stats["cs"] += 1

    def __add__(self, x):
        y = BattingStatline()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

    def __repr__(self):  return repr(self.stats)

class PitchingStatline:
    def __init__(self):
        self.stats = { "games": [ ],
                       "gs":0, "cg":0, "sho":0, "gf":0,
                       "w":0, "l":0, "sv":0,
                       "outs":0, "bf":0, "r":0, "er":0, "tur":0,
                       "h":0, "2b":0, "3b":0, "hr":0,
                       "bb":0, "ibb":0, "so":0,
                       "wp":0, "bk":0, "hb":0 }

    def ProcessBatting(self, eventData):
        self.stats["outs"] += eventData.GetOuts()

        if eventData.IsBatterEvent():
            self.stats["bf"] += 1

        if eventData.event_type == cw.EVENT_SINGLE:
            self.stats["h"] += 1
        elif eventData.event_type == cw.EVENT_DOUBLE:
            self.stats["h"] += 1
            self.stats["2b"] += 1
        elif eventData.event_type == cw.EVENT_TRIPLE:
            self.stats["h"] += 1
            self.stats["3b"] += 1
        elif eventData.event_type == cw.EVENT_HOMERUN:
            self.stats["h"] += 1
            self.stats["hr"] += 1
        elif eventData.event_type == cw.EVENT_WALK:
            self.stats["bb"] += 1
        elif eventData.event_type == cw.EVENT_INTENTIONALWALK:
            self.stats["bb"] += 1
            self.stats["ibb"] += 1
        elif eventData.event_type == cw.EVENT_STRIKEOUT:
            self.stats["so"] += 1
        elif eventData.event_type == cw.EVENT_HITBYPITCH:
            self.stats["hb"] += 1
        elif eventData.event_type == cw.EVENT_BALK:
            self.stats["bk"] += 1

        if eventData.wp_flag > 0:
            self.stats["wp"] += 1

        destBase = eventData.GetAdvance(0)
        if destBase < 4: return

        if destBase == 4:
            self.stats["r"] += 1
            self.stats["er"] += 1
        elif destBase == 5:
            self.stats["r"] += 1
        elif destBase == 6:
            self.stats["r"] += 1
            self.stats["er"] += 1
            self.stats["tur"] += 1

    def ProcessRunning(self, eventData, base):
        destBase = eventData.GetAdvance(base)
        if destBase < 4: return

        if destBase == 4:
            self.stats["r"] += 1
            self.stats["er"] += 1
        elif destBase == 5:
            self.stats["r"] += 1
        elif destBase == 6:
            self.stats["r"] += 1
            self.stats["er"] += 1
            self.stats["tur"] += 1
        
    def __add__(self, x):
        y = PitchingStatline()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value


class FieldingStatline:
    def __init__(self):
        self.stats = { "games": [ ],
                       "gs":0, "outs":0, "bip":0, "bf":0,
                       "po":0, "a":0, "e":0, "dp":0, "tp":0 }

    def ProcessFielding(self, eventData, pos):
        self.stats["outs"] += eventData.GetOuts()
        po = eventData.GetPutouts(pos)
        self.stats["po"] += po
        a = eventData.GetAssists(pos)
        self.stats["a"] += a
        self.stats["e"] += eventData.GetErrors(pos)

        if eventData.dp_flag and po + a > 0:
            self.stats["dp"] += 1
        elif eventData.tp_flag and po + a > 0:
            self.stats["tp"] += 1

        if eventData.fielded_by == pos and eventData.GetOuts() > 0:
            self.stats["bf"] += 1
        if (eventData.fielded_by > 0 or
            eventData.event_type in [cw.EVENT_SINGLE,
                                     cw.EVENT_DOUBLE,
                                     cw.EVENT_TRIPLE]):
            self.stats["bip"] += 1
        
    def __add__(self, x):
        y = FieldingStatline()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

class TeamFieldingStatline:
    def __init__(self):
        self.stats = { "games": [ ],
                       "gs":0, "outs":0, "bip":0, "bf":0,
                       "po":0, "a":0, "e":0, "dp":0, "tp":0 }

    def ProcessFielding(self, eventData):
        self.stats["po"] += eventData.GetOuts()
        self.stats["a"] += eventData.num_assists
        self.stats["e"] += eventData.num_errors

        if eventData.dp_flag:    self.stats["dp"] += 1
        elif eventData.tp_flag:  self.stats["tp"] += 1

        if eventData.fielded_by > 0 and eventData.GetOuts() > 0:
            self.stats["bf"] += 1
        if (eventData.fielded_by > 0 or
            eventData.event_type in [cw.EVENT_SINGLE,
                                     cw.EVENT_DOUBLE,
                                     cw.EVENT_TRIPLE]):
            self.stats["bip"] += 1

    def __add__(self, x):
        y = TeamFieldingStatline()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

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


class BattingRegister:
    def __init__(self, book):
        self.stats = { }
        self.book = book

    def GetName(self):  return "batting-register"

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)
                if player.player_id not in self.stats:
                    self.stats[player.player_id] = BattingStatline()
            
                if game.GetGameID() not in self.stats[player.player_id]["games"]:
                    self.stats[player.player_id]["games"].append(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.player_id not in self.stats:
                self.stats[rec.player_id] = BattingStatline()

            if game.GetGameID() not in self.stats[rec.player_id]["games"]:
                self.stats[rec.player_id]["games"].append(game.GetGameID())

            rec = rec.next


    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()
        
        batterId = gameiter.GetPlayer(team,
                                      gameiter.NumBatters(team) % 9 + 1)
        batter = self.stats[batterId]
        batter.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "":  continue
            runner = self.stats[gameiter.GetRunner(base)]
            runner.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        pass

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if i % 20 == 0:
                s += "\nPlayer                 AVG   SLG   OBP   G  AB   R   H 2B 3B HR RBI  BB IW  SO DP HP SH SF SB CS\n"
            

            player = self.book.GetPlayer(key)
            s += ("%-20s %s %s %s %3d %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d %2d %2d %2d %2d %2d %2d\n" %
                (player.GetSortName(),
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

    def __repr__(self):  return repr(self.stats)

    def derepr(self, text):
        x = eval(text)
        self.stats = { }
        for key in x:
            self.stats[key] = BattingStatline()
            self.stats[key].stats = x[key]

class BattingDailies:
    def __init__(self, book, playerID):
        self.stats = { }
        self.book = book
        self.playerID = playerID

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)
                if player.player_id == self.playerID:
                    self.stats[game] = BattingStatline()
                    return

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if (rec.player_id == self.playerID and
                game not in self.stats.keys()):
                self.stats[game] = BattingStatline()
                return

            rec = rec.next


    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()
        
        batterId = gameiter.GetPlayer(team,
                                      gameiter.NumBatters(team) % 9 + 1)
        if batterId == self.playerID:
            batter = self.stats[game]
            batter.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == self.playerID:
                runner = self.stats[game]
                runner.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        pass

    def __str__(self):
        keys = self.stats.keys()
        keys.sort(lambda x, y: cmp(x.GetDate() + str(x.GetNumber()),
                                   y.GetDate() + str(y.GetNumber())))

        s = ("Game-by-game batting for %s\n\n" %
             self.book.GetPlayer(self.playerID).GetName())
        accum = BattingStatline()
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            if i % 20 == 0:
                s += "\nGame             AVG   SLG   OBP AB  R  H 2B 3B HR BI BB IW SO DP HP SH SF SB CS\n"
            

            accum += stat
            
            s += ("%-10s %-3s %s %s %s %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n" %
                (key.GetDate(),
                 { 1: "(1)", 2: "(2)", 0: "" }[key.GetNumber()],
                 FormatAverage(accum["h"], accum["ab"]),
                 FormatAverage(accum["h"] + accum["2b"] +
                               2*accum["3b"] + 3*accum["hr"],
                               accum["ab"]),
                 FormatAverage(accum["h"] + accum["bb"] + accum["hp"],
                               accum["ab"] + accum["bb"] + accum["hp"] + accum["sf"]),
                 stat["ab"], stat["r"], stat["h"],
                 stat["2b"], stat["3b"], stat["hr"],
                 stat["bi"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["gdp"], stat["hp"],
                 stat["sh"], stat["sf"],
                 stat["sb"], stat["cs"]))

        return s

class TeamBattingRegister:
    def __init__(self, book, team):
        self.book = book
        self.team = team
        self.stats = { }

    def OnBeginGame(self, game, gameiter):
        if not self.team in game.GetTeams():  return
        
        t = game.GetTeams().index(self.team)
        for slot in range(9):
            player = game.GetStarter(t, slot+1)
            if player.player_id not in self.stats:
                self.stats[player.player_id] = BattingStatline()
                
            if game.GetGameID() not in self.stats[player.player_id]["games"]:
                self.stats[player.player_id]["games"].append(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        if not self.team in game.GetTeams():  return

        t = game.GetTeams().index(self.team)

        rec = gameiter.event.first_sub
        while rec != None:
            if rec.team == t:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = BattingStatline()

                if game.GetGameID() not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.GetGameID())

            rec = rec.next


    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()
        
        if game.GetTeams()[team] != self.team:  return

        batterId = gameiter.GetPlayer(team,
                                      gameiter.NumBatters(team) % 9 + 1)
        batter = self.stats[batterId]
        batter.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "": continue
            runner = self.stats[gameiter.GetRunner(base)]
            runner.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        pass

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = "\n%-20s   AVG   SLG   OBP   G  AB   R   H 2B 3B HR RBI  BB IW  SO DP HP SH SF SB CS\n" % self.book.GetTeam(self.team).GetCity()
        for (i,key) in enumerate(keys):
            stat = self.stats[key]

            player = self.book.GetPlayer(key)
            
            s += ("%-20s %s %s %s %3d %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d %2d %2d %2d %2d %2d %2d\n" %
                (player.GetSortName(),
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

        stat = reduce(lambda x,y: x+y, [self.stats[key] for key in self.stats])

        s += ("%-20s %s %s %s    %4d    %4d   %3d   %4d    %3d     %2d    %2d    %2d\n" %
              ("Totals",
               FormatAverage(stat["h"], stat["ab"]),
               FormatAverage(stat["h"] + stat["2b"] +
                             2*stat["3b"] + 3*stat["hr"],
                             stat["ab"]),
               FormatAverage(stat["h"] + stat["bb"] + stat["hp"],
                             stat["ab"] + stat["bb"] + stat["hp"] + stat["sf"]),
               stat["ab"], stat["h"], stat["3b"],
               stat["bi"], stat["ibb"], 
               stat["gdp"], stat["sh"], stat["sb"]))

        s += ("%-20s %s %s %s%4d    %4d    %3d   %3d     %3d   %4d   %3d   %3d   %3d\n" %
              ("", "     ", "     ", "     ",
               len(stat["games"]),
               stat["r"], stat["2b"], stat["hr"], stat["bb"], stat["so"],
               stat["hp"], stat["sf"], stat["cs"]))

        return s

class TeamBattingTotals:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        for team in book.Teams():
            self.stats[team.GetID()] = BattingStatline()

    def OnBeginGame(self, game, gameiter):
        for team in game.GetTeams():
            self.stats[team]["games"].append(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()

        batter = self.stats[game.GetTeams()[team]]
        batter.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "":  continue
            batter.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        for t in [0,1]:
            self.stats[game.GetTeams()[t]]["lob"] += gameiter.GetTeamLOB(t)

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            team = self.book.GetTeam(key)
            
            if i % 20 == 0:
                s += "\nClub            AVG   SLG   OBP   G   AB   R    H  2B 3B  HR RBI  BB IW   SO  DP HP SH SF  SB CS  LOB\n"

            
            s += ("%-13s %s %s %s %3d %4d %3d %4d %3d %2d %3d %3d %3d %2d %4d %3d %2d %2d %2d %3d %2d %4d\n" %
                (team.GetCity(),
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

        stat = reduce(lambda x,y: x+y, [self.stats[key] for key in self.stats])
        s += ("%-13s %s %s %s %3d %4d %3d %4d %3d %2d %3d %3d %3d %2d %4d %3d %2d %2d %2d %3d %2d %4d\n" %
              ("Totals",
               FormatAverage(stat["h"], stat["ab"]),
               FormatAverage(stat["h"] + stat["2b"] +
                             2*stat["3b"] + 3*stat["hr"],
                             stat["ab"]),
               FormatAverage(stat["h"] + stat["bb"] + stat["hp"],
                             stat["ab"] + stat["bb"] + stat["hp"] + stat["sf"]),
               len(stat["games"])/2,
               stat["ab"], stat["r"], stat["h"],
               stat["2b"], stat["3b"], stat["hr"],
               stat["bi"],
               stat["bb"], stat["ibb"], stat["so"],
               stat["gdp"], stat["hp"],
               stat["sh"], stat["sf"],
               stat["sb"], stat["cs"], stat["lob"]))

        return s



class PitchingRegister:
    def __init__(self, book):
        self.stats = { }
        self.book = book

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)

                if player.pos == 1:
                    if player.player_id not in self.stats:
                        self.stats[player.player_id] = PitchingStatline()
                    
                    if game.GetGameID() not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.GetGameID())
                    self.stats[player.player_id]["gs"] += 1

            if game.GetStarter(t, 0) != None:
                player = game.GetStarter(t, 0)
                
                if player.player_id not in self.stats:
                    self.stats[player.player_id] = PitchingStatline()
                    if game.GetGameID() not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.GetGameID())
                    self.stats[player.player_id]["gs"] += 1
                

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.pos == 1:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = PitchingStatline()
                if game.GetGameID() not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.GetGameID())

            rec = rec.next

    def OnEvent(self, game, gameiter):
        team = gameiter.GetHalfInning()

        pitcherId = gameiter.GetFielder(1-team, 1)
        pitcher = self.stats[pitcherId]

        eventData = gameiter.GetEventData()
        pitcher.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "": continue
            
            resppitcher = self.stats[gameiter.GetRespPitcher(base)]
            resppitcher.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        if game.GetWinningPitcher() != "":
            self.stats[game.GetWinningPitcher()]["w"] += 1
        if game.GetLosingPitcher() != "":
            self.stats[game.GetLosingPitcher()]["l"] += 1
        if game.GetSavePitcher() != "":
            self.stats[game.GetSavePitcher()]["sv"] += 1

        for t in [0, 1]:
            startP = game.GetStarterAtPos(t, 1).player_id
            endP = gameiter.GetFielder(t, 1)
            if startP == endP:
                # TODO: It's possible but rare to start and end game but
                # not pitch a complete game!
                self.stats[startP]["cg"] += 1
                if gameiter.GetTeamScore(1-t) == 0:
                    self.stats[startP]["sho"] += 1
            else:
                self.stats[endP]["gf"] += 1

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            player = self.book.GetPlayer(key)
            
            if stat["outs"] == 0:
                era = "-----"
            else:
                era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)
            if i % 20 == 0:
                s += "\nPlayer                 ERA  G GS CG SH GF  W- L SV   BF    IP   R  ER   H HR  BB IW  SO BK WP HB\n"
            s += ("%-20s %s %2d %2d %2d %2d %2d %2d-%2d %2d %4d %3d.%1d %3d %3d %3d %2d %3d %2d %3d %2d %2d %2d\n" %
                (player.GetSortName(), era,
                 len(stat["games"]),
                 stat["gs"], stat["cg"], stat["sho"],
                 stat["gf"],
                 stat["w"], stat["l"], stat["sv"],
                 stat["bf"], stat["outs"] / 3, stat["outs"] % 3,
                 stat["r"], stat["er"],
                 stat["h"], stat["hr"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["bk"], stat["wp"], stat["hb"]))

        return s

class TeamPitchingRegister:
    def __init__(self, book, team):
        self.book = book
        self.team = team
        self.stats = { }

    def OnBeginGame(self, game, gameiter):
        if not self.team in game.GetTeams():  return
        
        t = game.GetTeams().index(self.team)

        for slot in range(9):
            player = game.GetStarter(t, slot+1)

            if player.pos == 1:
                if player.player_id not in self.stats:
                    self.stats[player.player_id] = PitchingStatline()
                    
                if game.GetGameID() not in self.stats[player.player_id]["games"]:
                    self.stats[player.player_id]["games"].append(game.GetGameID())
                self.stats[player.player_id]["gs"] += 1

        if game.GetStarter(t, 0) != None:
            player = game.GetStarter(t, 0)
                
            if player.player_id not in self.stats:
                self.stats[player.player_id] = PitchingStatline()
            if game.GetGameID() not in self.stats[player.player_id]["games"]:
                self.stats[player.player_id]["games"].append(game.GetGameID())
            self.stats[player.player_id]["gs"] += 1
                

    def OnSubstitution(self, game, gameiter):
        if not self.team in game.GetTeams():  return
        
        t = game.GetTeams().index(self.team)

        rec = gameiter.event.first_sub
        while rec != None:
            if rec.team == t and rec.pos == 1:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = PitchingStatline()
                if game.GetGameID() not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.GetGameID())

            rec = rec.next

    def OnEvent(self, game, gameiter):
        team = gameiter.GetHalfInning()
        if game.GetTeams()[1-team] != self.team:  return

        pitcherId = gameiter.GetFielder(1-team, 1)
        pitcher = self.stats[pitcherId]

        eventData = gameiter.GetEventData()
        pitcher.ProcessBatting(eventData)
        
        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "": continue            

            resppitcher = self.stats[gameiter.GetRespPitcher(base)]
            resppitcher.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        if not self.team in game.GetTeams():  return

        if game.GetWinningPitcher() != "" and game.GetWinningPitcher() in self.stats.keys():
            self.stats[game.GetWinningPitcher()]["w"] += 1
        if game.GetLosingPitcher() != "" and game.GetLosingPitcher() in self.stats.keys():
            self.stats[game.GetLosingPitcher()]["l"] += 1
        if game.GetSavePitcher() != "" and game.GetSavePitcher() in self.stats.keys():
            self.stats[game.GetSavePitcher()]["sv"] += 1

        t = game.GetTeams().index(self.team)
        startP = game.GetStarterAtPos(t, 1).player_id
        endP = gameiter.GetFielder(t, 1)
        if startP == endP:
            # TODO: It's possible but rare to start and end game but
            # not pitch a complete game!
            self.stats[startP]["cg"] += 1
            if gameiter.GetTeamScore(1-t) == 0:
                self.stats[startP]["sho"] += 1
        else:
            self.stats[endP]["gf"] += 1

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = "\n%-20s   ERA  G GS CG SH GF  W- L SV   BF    IP   R  ER   H HR  BB IW  SO BK WP HB\n" % self.book.GetTeam(self.team).GetCity()
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            player = self.book.GetPlayer(key)
            
            if stat["outs"] == 0:
                era = "-----"
            else:
                era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)
            s += ("%-20s %s %2d %2d %2d %2d %2d %2d-%2d %2d %4d %3d.%1d %3d %3d %3d %2d %3d %2d %3d %2d %2d %2d\n" %
                (player.GetSortName(), era,
                 len(stat["games"]),
                 stat["gs"], stat["cg"], stat["sho"],
                 stat["gf"],
                 stat["w"], stat["l"], stat["sv"],
                 stat["bf"], stat["outs"] / 3, stat["outs"] % 3,
                 stat["r"], stat["er"],
                 stat["h"], stat["hr"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["bk"], stat["wp"], stat["hb"]))

        stat = reduce(lambda x,y: x+y, [self.stats[key] for key in self.stats])

        if stat["outs"] == 0:
            era = "-----"
        else:
            era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)
        s += ("%-20s %s %2d %2d %2d %2d %2d %2d-%2d %2d %4d %3d.%1d %3d %3d %3d %2d %3d %2d %3d %2d %2d %2d\n" %
              ("Totals", era,
               len(stat["games"]),
               stat["gs"], stat["cg"], stat["sho"],
               stat["gf"],
               stat["w"], stat["l"], stat["sv"],
               stat["bf"], stat["outs"] / 3, stat["outs"] % 3,
               stat["r"], stat["er"],
               stat["h"], stat["hr"],
               stat["bb"], stat["ibb"], stat["so"],
               stat["bk"], stat["wp"], stat["hb"]))

        return s



class TeamPitchingTotals:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        for team in book.Teams():
            self.stats[team.GetID()] = PitchingStatline()

    def OnBeginGame(self, game, gameiter):
        for team in game.GetTeams():
            self.stats[team]["games"].append(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()

        pitcher = self.stats[game.GetTeams()[1-team]]

        pitcher.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "": continue
            pitcher.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        teams = game.GetTeams()
        if gameiter.GetTeamScore(0) > gameiter.GetTeamScore(1):
            self.stats[teams[0]]["w"] += 1
            self.stats[teams[1]]["l"] += 1
            if game.GetSavePitcher() != "":
                self.stats[teams[0]]["sv"] += 1
        elif gameiter.GetTeamScore(0) < gameiter.GetTeamScore(1):
            self.stats[teams[1]]["w"] += 1
            self.stats[teams[0]]["l"] += 1
            if game.GetSavePitcher() != "":
                self.stats[teams[1]]["sv"] += 1

        for t in [0, 1]:
            startP = game.GetStarterAtPos(t, 1).player_id
            endP = gameiter.GetFielder(t, 1)
            if startP == endP:
                # TODO: It's possible but rare to start and end game but
                # not pitch a complete game!
                self.stats[teams[t]]["cg"] += 1
            if gameiter.GetTeamScore(1-t) == 0:
                self.stats[teams[t]]["sho"] += 1

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        league = PitchingStatline()
        
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            team = self.book.GetTeam(key)
            
            if stat["outs"] == 0:
                era = "-----"
            else:
                era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)
            if i % 20 == 0:
                s += "\nClub            ERA   G CG SH   W-  L SV     IP   R  ER    H  HR  BB IW   SO BK WP HB\n"
            s += ("%-13s %s %3d %2d %2d %3d-%3d %2d %4d.%1d %3d %3d %4d %3d %3d %2d %4d %2d %2d %2d\n" %
                (team.GetCity(), era,
                 len(stat["games"]),
                 stat["cg"], stat["sho"],
                 stat["w"], stat["l"], stat["sv"],
                 stat["outs"] / 3, stat["outs"] % 3,
                 stat["r"], stat["er"]-stat["tur"],
                 stat["h"], stat["hr"],
                 stat["bb"], stat["ibb"], stat["so"],
                 stat["bk"], stat["wp"], stat["hb"]))

        stat = reduce(lambda x,y: x+y, [self.stats[key] for key in self.stats])

        if stat["outs"] == 0:
            era = "-----"
        else:
            era = "%5.2f" % (float(stat["er"]) / float(stat["outs"]) * 27)

        s += ("%-13s %s %3d %2d %2d %3d-%3d %2d %4d.%1d %3d %3d %4d %3d %3d %2d %4d %2d %2d %2d\n" %
              ("Totals", era,
               len(stat["games"])/2,
               stat["cg"], stat["sho"],
               stat["w"], stat["l"], stat["sv"],
               stat["outs"] / 3, stat["outs"] % 3,
               stat["r"], stat["er"]-stat["tur"],
               stat["h"], stat["hr"],
               stat["bb"], stat["ibb"], stat["so"],
               stat["bk"], stat["wp"], stat["hb"]))

        return s
        


class FieldingRegister:
    def __init__(self, book, pos):
        self.stats = { }
        self.book = book
        self.pos = pos

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)

                if player.pos == self.pos:
                    if player.player_id not in self.stats:
                        self.stats[player.player_id] = FieldingStatline()
                    
                    if game.GetGameID() not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.GetGameID())
                    self.stats[player.player_id]["gs"] += 1

            if self.pos == 1 and game.GetStarter(t, 0) != None:
                player = game.GetStarter(t, 0)
                
                if player.player_id not in self.stats:
                    self.stats[player.player_id] = FieldingStatline()
                    if game.GetGameID() not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.GetGameID())
                    self.stats[player.player_id]["gs"] += 1

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.pos == self.pos:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = FieldingStatline()
                if game.GetGameID() not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.GetGameID())

            rec = rec.next


    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()

        fielderId = gameiter.GetFielder(1-team, self.pos)
        fielder = self.stats[fielderId]

        fielder.ProcessFielding(eventData, self.pos)

    def OnEndGame(self, game, gameiter):
        pass

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
            player = self.book.GetPlayer(key)
            
            if i % 20 == 0:
                s += "\n%-20s   PCT   G  GS    INN   PO   A  E  DP TP  BIP  BF\n" % posStr
            
            s += ("%-20s %s %3d %3d %4d.%1d %4d %3d %2d %3d %2d %4d %3d\n" %
                (player.GetSortName(),
                 FormatAverage(stat["po"] + stat["a"],
                               stat["po"] + stat["a"] + stat["e"]),
                 len(stat["games"]), stat["gs"],
                 stat["outs"] / 3, stat["outs"] % 3,
                 stat["po"], stat["a"], stat["e"],
                 stat["dp"], stat["tp"],
                 stat["bip"], stat["bf"]))

        return s

class TeamFieldingTotals:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        for team in book.Teams():
            self.stats[team.GetID()] = TeamFieldingStatline()

    def OnBeginGame(self, game, gameiter):
        for team in game.GetTeams():
            self.stats[team]["games"].append(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()
        fielder = self.stats[game.GetTeams()[1-team]]
        fielder.ProcessFielding(eventData)

    def OnEndGame(self, game, gameiter):
        pass

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            team = self.book.GetTeam(key)
            
            if i % 20 == 0:
                s += "\nClub            PCT   G   PO    A   E  DP TP  BIP   BF\n"
            
            s += ("%-13s %s %3d %4d %4d %3d %3d %2d %4d %4d\n" %
                (team.GetCity(),
                 FormatAverage(stat["po"] + stat["a"],
                               stat["po"] + stat["a"] + stat["e"]),
                 len(stat["games"]),
                 stat["po"], stat["a"], stat["e"],
                 stat["dp"], stat["tp"],
                 stat["bip"], stat["bf"]))


        stat = reduce(lambda x,y: x+y, [self.stats[key] for key in self.stats])
        s += ("%-13s %s %3d %4d %4d %3d %3d %2d %4d %4d\n" %
              ("Totals",
               FormatAverage(stat["po"] + stat["a"],
                             stat["po"] + stat["a"] + stat["e"]),
               len(stat["games"])/2,
               stat["po"], stat["a"], stat["e"],
               stat["dp"], stat["tp"],
               stat["bip"], stat["bf"]))

        return s


class TeamGameLog:
    def __init__(self, cwf):
        self.cwf = cwf
        self.stats = { }
        for team in cwf.Teams():
            self.stats[team.GetID()] = [ ]

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass
    def OnEvent(self, game, gameiter):  pass

    def OnEndGame(self, game, gameiter):
        ids = game.GetTeams()
        scores = [ gameiter.GetTeamScore(t) for t in [0,1] ]

        for t in [0,1]:
            self.stats[ids[t]].append({ "date": game.GetDate(),
                                        "number": game.GetNumber(),
                                        "teams": ids,
                                        "scores": scores,
                                        "innings": game.GetInnings(),
                                        "wp": game.GetWinningPitcher(),
                                        "lp": game.GetLosingPitcher(),
                                        "save": game.GetSavePitcher() })
   
    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""

        for team in keys:
            self.stats[team].sort(lambda x, y: cmp(x["date"] + str(x["number"]),
                                                   y["date"] + str(y["number"])))
            roster = self.cwf.GetTeam(team)

            s += "\nGame log for %s\n" % roster.GetName()

            wins = 0
            losses = 0
            
            for (i,game) in enumerate(self.stats[team]):
                if i % 20 == 0:
                    s += "\nDate       #  Opponent Result         Record  Win              Loss             Save\n"
                
                if game["wp"] != "":
                    wp = self.cwf.GetPlayer(game["wp"])
                    wpname = wp.GetFirstName()[:1] + ". " + wp.GetLastName()
                else:
                    wpname = ""
                    
                if game["lp"] != "":
                    lp = self.cwf.GetPlayer(game["lp"])
                    lpname = lp.GetFirstName()[:1] + ". " + lp.GetLastName()
                else:
                    lpname = ""
                    
                if game["save"] != "":
                    save = self.cwf.GetPlayer(game["save"])
                    savename = save.GetFirstName()[:1] + ". " + save.GetLastName()
                else:
                    savename = ""

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
                          (game["date"], [ " ", "1", "2" ][game["number"]],
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
                          (game["date"], [ " ", "1", "2" ][game["number"]],
                           game["teams"][0], dec,
                           game["scores"][1], game["scores"][0], inningStr,
                           wins, losses, wpname, lpname, savename))
                    
            s += "\n"

        return s

class TeamRecordTotals:
    def __init__(self, cwf):
        self.stats = { }
        for team in cwf.Teams():
            self.stats[team.GetID()] = self.NewTeamStats(team)

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass
    def OnEvent(self, game, gameiter):  pass

    def OnEndGame(self, game, gameiter):
        ids = game.GetTeams()
        scores = [ gameiter.GetTeamScore(t) for t in [0,1] ]

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

            if gameiter.state.inning >= 10:
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
        return { "id": team.GetID(),
                 "city": team.GetCity(), "nickname": team.GetNickname(),
                 "g":0, "w":0, "l":0, "t":0,
                 "hw":0, "hl":0,  # home record
                 "rw":0, "rl":0,  # road record
                 "ow":0, "ol":0,  # one-run games record
                 "xw":0, "xl":0}  # extra-inning games record

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = "\nClub                             G   W-  L    PCT  HOME  AWAY  1RUN  XINN\n";
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            s += ("%-30s %3d %3d-%3d  %s %2d-%2d %2d-%2d %2d-%2d %2d-%2d\n" %
                (stat["city"] + " " + stat["nickname"],
                 stat["g"], stat["w"], stat["l"],
                 FormatAverage(stat["w"], stat["w"]+stat["l"]),
                 stat["hw"], stat["hl"],
                 stat["rw"], stat["rl"],
                 stat["ow"], stat["ol"],
                 stat["xw"], stat["xl"]))

        return s

class GrandSlamLog:
    """
    Compiles a list of all grand slams hit.
    """
    def __init__(self, cwf):
        self.cwf = cwf
        self.stats = [ ]

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass

    def OnEvent(self, game, gameiter):
        if (gameiter.GetEventData().event_type == cw.EVENT_HOMERUN and
            gameiter.GetRunner(1) != "" and
            gameiter.GetRunner(2) != "" and
            gameiter.GetRunner(3) != ""):
            inning = gameiter.GetInning()
            halfInning = gameiter.GetHalfInning()
            if halfInning == 0:
                team = game.GetTeams()[0]
                opp = game.GetTeams()[1]
                site = opp
            else:
                team = game.GetTeams()[1]
                opp = game.GetTeams()[0]
                site = team
                
            self.stats.append({ "date": game.GetDate(),
                                "number": game.GetNumber(),
                                
                                "inning": inning,
                                "halfInning": halfInning,
                                "batter": gameiter.GetBatter(),
                                "pitcher": gameiter.GetFielder(1-halfInning, 1),
                                "team": team, "opp": opp, "site": site })
        
    def OnEndGame(self, game, gameiter):  pass
   
    def __str__(self):
        self.stats.sort(lambda x, y: cmp(x["date"] + str(x["number"]),
                                         y["date"] + str(y["number"])))

        s = "\nDate       # Site Batter               Pitcher           Inning\n"
        for rec in self.stats:
            if rec["number"] == "0":  rec["number"] = " "
            batter = self.cwf.GetPlayer(rec["batter"])
            pitcher = self.cwf.GetPlayer(rec["pitcher"])

            s += ("%s %s %s  %-20s %-20s %d\n" % 
                  (rec["date"], rec["number"], rec["site"],
                   batter.GetFirstName()[0] + ". " + batter.GetLastName() + " (" + rec["team"] + ")",
                   pitcher.GetFirstName()[0] + ". " + pitcher.GetLastName() + " (" + rec["opp"] + ")",
                   rec["inning"]))
        return s

class BigGameLog:
    """
    This is a log report for 'big games' by players.  It provides
    the generic stuff needed to generate such a report.  Derived
    reports should provide three functions: the OnEvent() function;
    a GetHeader() function, which returns the text string placed over
    the count in the text output; and a GetThreshold() function to
    indicate the statistical count needed to appear in the report.
    """
    def __init__(self, book):
        self.events = [ ]
        self.book = book

    # Provide us in derived class!
    def GetTitle(self):   return ""
    def GetHeader(self):  return ""
    def GetThreshold(self):  return 2
    def IsOffense(self):  return True
    def OnEvent(self, game, gameiter):  pass
    
    def OnBeginGame(self, game, gameiter): self.counts = [ { }, { } ]
    def OnSubstitution(self, game, gameiter): pass

    def OnEndGame(self, game, gameiter):
        for (t,team) in enumerate(self.counts):
            for key in team:
                if team[key] >= self.GetThreshold():
                    if self.IsOffense():
                        self.events.append({ "game": game,
                                             "batter": key,
                                             "team": game.GetTeam(t),
                                             "opp": game.GetTeam(1-t),
                                             "site": game.GetTeam(1),
                                             "count": team[key] })
                    else:
                        self.events.append({ "game": game,
                                             "batter": key,
                                             "team": game.GetTeam(1-t),
                                             "opp": game.GetTeam(t),
                                             "site": game.GetTeam(1),
                                             "count": team[key] })


    def __str__(self):
        s = "\n%s\n" % self.GetTitle()
        s += ("\nPlayer                         Date           Team Opp  Site  %2s\n" %
             self.GetHeader())

        self.events.sort(lambda x, y:
                         cmp(x["game"].GetDate()+x["site"]+str(x["game"].GetNumber()),
                             y["game"].GetDate()+y["site"]+str(y["game"].GetNumber())))
        
        for rec in self.events:
            player = self.book.GetPlayer(rec["batter"])
            s += ("%-30s %10s %s %s  %s  %s   %2d\n" %
                  (player.GetSortName(),
                   rec["game"].GetDate(),
                   [ "   ", "(1)", "(2)" ][rec["game"].GetNumber()],
                   rec["team"], rec["opp"], rec["site"],
                   rec["count"]))

        return s


class MultiHRLog(BigGameLog):
    def __init__(self, book):  BigGameLog.__init__(self, book)

    def GetTitle(self):
        return ("Players with at least %d home runs in a game" %
                self.GetThreshold())
    def GetHeader(self):  return "HR"
    def GetThreshold(self):  return 2
    def IsOffense(self):  return True
    
    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.event.batting_team

        if eventData.event_type == cw.EVENT_HOMERUN:
            batter = gameiter.event.batter
            if gameiter.event.batter in self.counts[team]:
                self.counts[team][batter] += 1
            else:
                self.counts[team][batter] = 1

class MultiHitLog(BigGameLog):
    def __init__(self, book):  BigGameLog.__init__(self, book)

    def GetTitle(self):
        return ("Players with at least %d hits in a game" %
                self.GetThreshold())
    def GetHeader(self):  return "H"
    def GetThreshold(self):  return 4
    def IsOffense(self):  return True
    
    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.event.batting_team

        if eventData.event_type in [ cw.EVENT_SINGLE,
                                     cw.EVENT_DOUBLE,
                                     cw.EVENT_TRIPLE,
                                     cw.EVENT_HOMERUN ]:
            batter = gameiter.event.batter
            if gameiter.event.batter in self.counts[team]:
                self.counts[team][batter] += 1
            else:
                self.counts[team][batter] = 1

class MultiStrikeoutLog(BigGameLog):
    def __init__(self, book):  BigGameLog.__init__(self, book)

    def GetTitle(self):
        return ("Pitchers with at least %d strikeouts in a game" %
                self.GetThreshold())
    def GetHeader(self):  return "SO"
    def GetThreshold(self):  return 10
    def IsOffense(self):  return False
    
    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.event.batting_team

        if eventData.event_type == cw.EVENT_STRIKEOUT:
            pitcher = gameiter.GetFielder(1-team, 1)
            if pitcher in self.counts[team]:
                self.counts[team][pitcher] += 1
            else:
                self.counts[team][pitcher] = 1


def ProcessGame(game, acclist):
    gameiter = cw.GameIterator(game)
    map(lambda x: x.OnBeginGame(game, gameiter), acclist)

    while gameiter.event != None:
        if gameiter.event.event_text != "NP":
            map(lambda x: x.OnEvent(game, gameiter), acclist)

        if gameiter.event.first_sub != None:
            map(lambda x: x.OnSubstitution(game, gameiter), acclist)

        gameiter.NextEvent()

    map(lambda x: x.OnEndGame(game, gameiter), acclist)

def ProcessFile(book, acclist, f=lambda x: True, monitor=None):
    """
    Process the games in scorebook 'book' through the list of
    accumulators in 'acclist'.  Instrumented so that if
    'monitor' is None, progress indications (via calls to
    monitor.Update) are given -- thus the wxWidgets wxProgressDialog
    automatically works for this parameter.
    """
    numGames = book.NumGames()
    for (i,game) in enumerate(book.Games()):
        if f(game):
            ProcessGame(game, acclist)
            if monitor != None:
                if not monitor.Update(round(float(i)/float(numGames)*100)):
                    return False
    return True

if __name__ == "__main__":
    import sys
    import scorebook

    fn = sys.argv[1]

    book = scorebook.Scorebook()
    book.Read(fn)

    #x = [ PitchingRegister(book) ]
    #for team in book.Teams():
    #    x.append(TeamPitchingRegister(book, team.GetID()))
    #x = [ TeamRecordTotals(book),
    #      TeamBattingTotals(book),
    #      TeamPitchingTotals(book),
    #      TeamFieldingTotals(book) ]
    #x = [ MultiHRLog(book), MultiHitLog(book), MultiStrikeoutLog(book) ]
    #x = [ BattingDailies(book, "bondb001") ]
    x = [ BattingRegister(book) ]
    ProcessFile(book, x)

    for acc in x:
        print repr(acc)
        print "\n"
        print len(repr(acc))
