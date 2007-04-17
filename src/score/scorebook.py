#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Class representing a Chadwick "scorebook"
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
import statscan          # for report updating

class Scorebook:
    def __init__(self, year=2007):
        self.books = { }
        self.modified = False
        self.year = year
        self.league = cw.League()
        self.games = [ ]
        self.playerDict = { }
        self.players = [ ]
        self.filename = "untitled.chw"
        self.reports = [ ]
        
    def GetFilename(self):   return self.filename

    def NumTeams(self):
        i = 0
        x = self.league.first_roster
        while x != None:
            i += 1
            x = x.next
        return i

    def Teams(self):
        x = self.league.first_roster
        while x != None:
            yield x
            x = x.next
        raise StopIteration

    def BuildIndices(self):
        self.games = [ ]
        for x in self.books.keys():
            g = self.books[x].first_game
            while g != None:
                self.games.append(g)
                g = g.next

        self.games.sort(lambda x, y: cmp(x.GetDate(), y.GetDate()))

        self.playerDict = { }
        for team in self.Teams():
            x = team.first_player
            while x != None:
                if x.player_id not in self.playerDict.keys():
                    self.playerDict[x.player_id] = x
                self.playerDict[x.player_id].AddTeam(team.GetID())
                x = x.next
        self.players = self.playerDict.keys()
        self.players.sort(lambda x, y: cmp(self.playerDict[x].GetSortName(),
                                           self.playerDict[y].GetSortName()))

    def NumGames(self, crit=lambda x: True):
        return len(filter(crit, self.games))

    def GetGameNumber(self, i, crit=lambda x: True):
        return filter(crit, self.games)[i]

    def GetGame(self, gameID):
        for g in self.games:
            if g.GetGameID() == gameID:  return g
        return None

    def Games(self, crit=lambda x: True):
        for g in self.games:
            if crit(g):  yield g

    def NumPlayers(self):  return len(self.playerDict)

    def Players(self):
        keys = self.playerDict.keys()
        keys.sort()
        for p in keys: yield self.playerDict[p]

    def GetPlayer(self, playerID):   return self.playerDict[playerID]
    def GetPlayerNumber(self, i):    return self.playerDict[self.players[i]]

    def UniquePlayerID(self, first, last):
        playerID = (last.replace(" ", "").replace("'", ""))[:4].lower()
        while len(playerID) < 4: playerID += "-"
        playerID += first[0].lower()
        i = 1
        while "%s%03d" % (playerID,i) in self.playerDict.keys(): i += 1
        return "%s%03d" % (playerID,i)

    def AddPlayer(self, playerID, firstName, lastName, bats, throws, team):
        p = cw.Player(playerID, lastName, firstName, bats, throws)
        p.AddTeam(team)

        roster = self.league.first_roster
        while roster.GetID() != team:  roster = roster.next

        roster.InsertPlayer(p)
        p.thisown = 0
        self.playerDict[playerID] = p
        self.players.append(playerID)
        self.players.sort(lambda x, y: cmp(self.playerDict[x].GetSortName(),
                                           self.playerDict[y].GetSortName()))
        self.modified = True

    def AddToTeam(self, playerID, firstName, lastName, bats, throws, team):
        p = cw.Player(playerID, lastName, firstName, bats, throws)

        roster = self.league.first_roster
        while roster.GetID() != team:  roster = roster.next

        roster.InsertPlayer(p)
        p.thisown = 0
        self.playerDict[playerID].AddTeam(team)
        self.modified = True

    def ModifyPlayer(self, playerID, firstName, lastName, bats, throws):
        p = self.playerDict[playerID]
        p.SetFirstName(firstName)
        p.SetLastName(lastName)
        p.bats = bats
        p.throws = throws

        for team in self.Teams():
            p = team.FindPlayer(playerID)
            if p != None:
                p.SetFirstName(firstName)
                p.SetLastName(lastName)
                p.bats = bats
                p.throws = throws
                
        self.players.sort(lambda x, y: cmp(self.playerDict[x].GetSortName(),
                                           self.playerDict[y].GetSortName()))
        self.modified = True

    def GetTeam(self, teamId):
        return self.league.FindRoster(teamId)

    def GetYear(self):    return self.year

    def AddTeam(self, teamID, city, nickname, leagueID):
        t = cw.Roster(teamID, self.year, leagueID, city, nickname)
        
        # We try to keep teams in playerID order.
        # This really ought to be part of the underlying C library
        if self.league.first_roster == None:
            self.league.first_roster = t
            self.league.last_roster = t
        else:
            x = self.league.first_roster
            while x != None and x.GetID() < t.GetID():
                x = x.next
            if x == None:
                t.prev = self.league.last_roster
                self.league.last_roster.next = t
                self.league.last_roster = t
            elif x.prev == None:
                self.league.first_roster.prev = t
                t.next = self.league.first_roster
                self.league.first_roster = t
            else:
                t.prev = x.prev
                t.prev.next = t
                x.prev = t
                t.next = x
        self.books[teamID] = cw.Scorebook()
        self.modified = True

    def ModifyTeam(self, teamID, city, nickname, leagueID):
        team = self.league.first_roster
        while team.GetID() != teamID:  team = team.next

        team.SetCity(city)
        team.SetNickname(nickname)
        team.SetLeague(leagueID)
        self.modified = True

    def AddGame(self, game):
        hometeam = game.GetTeam(1)
        self.books[hometeam].InsertGame(game)
        self.games.append(game)
        self.games.sort(lambda x, y: cmp(x.GetDate(), y.GetDate()))
        statscan.ProcessGame(game, self.reports)
        self.modified = True

    def RemoveGame(self, game):
        hometeam = game.GetTeam(1)
        self.books[hometeam].RemoveGame(game.GetGameID())
        self.games.remove(game)
        self.games.sort(lambda x, y: cmp(x.GetDate(), y.GetDate()))
        self.modified = True

    def ImportGames(self, book, games):
        for game in games:
            for t in [0, 1]:
                if self.GetTeam(game.GetTeam(t)) == None:
                    team = book.GetTeam(game.GetTeam(t))
                    self.AddTeam(team.GetID(),
                                 team.GetCity(), team.GetNickname(),
                                 team.GetLeague())
                    self.books[game.GetTeam(t)] = cw.Scorebook()

                self.GetTeam(game.GetTeam(t)).Import(book.GetTeam(game.GetTeam(t)))
                
            book.RemoveGame(game)
            self.books[game.GetTeam(1)].InsertGame(game)
            statscan.ProcessGame(game, self.reports)
            
        if len(games) > 0:
            self.BuildIndices()
            self.modified = True
        
    def IsModified(self):   return self.modified
    def SetModified(self, value):  self.modified = value
        
    def AddReport(self, report):
        self.reports.append(report)

    def GetReport(self, label):
        report = filter(lambda x: x.GetName() == label, self.reports)
        if len(report) > 0:
            return report[0]
        else:
            return None

    def RemoveReport(self, label):
        self.reports = filter(lambda x: x.GetName() != label, self.reports)
