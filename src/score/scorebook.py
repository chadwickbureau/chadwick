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

import random            # used for random player ID generation
import libchadwick as cw
import statscan          # for report updating

class Team:
    def __init__(self, teamID, year, league, city, nickname):
        self.teamID = teamID
        self.year = year
        self.league = league
        self.city = city
        self.nickname = nickname
        self.players = { }

    def GetID(self):                   return self.teamID

    def GetCity(self):                 return self.city
    def SetCity(self, city):           self.city = city

    def GetNickname(self):             return self.nickname
    def SetNickname(self, nickname):   self.nickname = nickname

    def GetName(self):                 return self.city + " " + self.nickname

    def GetLeague(self):               return self.league
    def SetLeague(self, league):       self.league = league

    def GetYear(self):                 return self.year
    def SetYear(self, year):           self.year = year

    def NumPlayers(self):              return len(self.players)
    def Players(self, sorted=True):
        if not sorted:
            return self.players.itervalues()
        else:
            players = [ x for x in self.players.values() ]
            players.sort(lambda x, y: cmp(x.GetSortName(), y.GetSortName()))
            return players
                         
    
    def AddPlayer(self, player):
        self.players[player.GetID()] = player

    def GetPlayer(self, playerID):
        try:
            return self.players[playerID]
        except KeyError:
            return None


class Player:
    def __init__(self, playerID, nameLast, nameFirst, bats, throws):
        self.playerID = playerID
        self.nameLast = nameLast
        self.nameFirst = nameFirst
        self.bats = bats
        self.throws = throws

    def GetID(self):               return self.playerID

    def GetFirstName(self):        return self.nameFirst
    def SetFirstName(self, name):  self.nameFirst = name

    def GetLastName(self):         return self.nameLast
    def SetLastName(self, name):   self.nameLast = name

    def GetName(self):             return self.nameFirst + " " + self.nameLast
    def GetSortName(self):         return self.nameLast + ", " + self.nameFirst

    def GetBats(self):             return self.bats
    def SetBats(self, side):       self.bats = side

    def GetThrows(self):           return self.throws
    def SetThrows(self, side):     self.throws = side
    

class Scorebook:
    def __init__(self, year=2007):
        self.books = { }
        self.modified = False
        self.year = year
        self.teams = { }
        self.games = { }
        self.players = { }
        self.filename = "untitled.chw"
        self.reports = [ ]
        
    def GetFilename(self):   return self.filename
    def IsModified(self):   return self.modified
    def SetModified(self, value):  self.modified = value
    def GetYear(self):    return self.year


    def NumTeams(self):           return len(self.teams)
    def Teams(self):              return self.teams.itervalues()
    def GetTeam(self, teamID):    return self.teams[teamID]

    def SetTeam(self, teamID, city, nickname, leagueID):
        if teamID in self.teams:
            t = self.teams[teamID]
            t.SetCity(city)
            t.SetNickname(nickname)
            t.SetLeague(leagueID)
        else:
            self.teams[teamID] = Team(teamID, self.year,
                                      leagueID, city, nickname)
        self.modified = True

    def NumPlayers(self):  return len(self.players)
    def Players(self):     return self.players.itervalues()
    def GetPlayer(self, playerID):   return self.players[playerID]

    def SetPlayer(self, playerID, firstName, lastName, bats, throws):
        if playerID in self.players:
            p = self.players[playerID]
            p.SetFirstName(firstName)
            p.SetLastName(lastName)
            p.SetBats(bats)
            p.SetThrows(throws)
        else:
            self.players[playerID] = Player(playerID, lastName, firstName,
                                            bats, throws)
        self.modified = True

    def SetPlayerTeam(self, playerID, teamID):
        self.teams[teamID].AddPlayer(self.players[playerID])

    def UniquePlayerID(self):
        i = random.randrange(1, 10000)
        while "%04d" % i in self.players: i = random.randrange(1, 10000)
        return "%04d" % i

    def NumGames(self):          return len(self.games)
    def Games(self):              return self.games.itervalues()

    def GetGame(self, gameID):
        try:
            return self.games[gameID]
        except KeyError:
            return None


    def SetGame(self, game):
        if game.GetGameID() not in self.games:
            self.games[game.GetGameID()] = game
        statscan.ProcessGame(game, self.reports)
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
            self.modified = True
        
    def AddReport(self, report):
        #self.reports.append(report)
        pass

    def GetReport(self, label):
        report = filter(lambda x: x.GetName() == label, self.reports)
        if len(report) > 0:
            return report[0]
        else:
            return None

    def RemoveReport(self, label):
        self.reports = filter(lambda x: x.GetName() != label, self.reports)
