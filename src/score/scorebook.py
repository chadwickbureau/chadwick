#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Class representing a Chadwick "scorebook"
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

import os                # for low-level manipulation of tempfiles
import zipfile
import tempfile

def TempFile():
    """
    Wrapper around tempfile.mkstemp(): closes the original file and
    returns the file name.
    """
    f = tempfile.mkstemp()
    name = f[1]
    os.close(f[0])
    return name    

class ChadwickScorebook:
    def __init__(self, year=2005):
        self.books = { }
        self.modified = False
        self.year = year
        self.league = CWLeague()
        self.games = [ ]
        self.players = { }
        self.filename = "untitled.chw"
        
    def GetFilename(self):   return self.filename

    def Read(self, filename):
        zf = zipfile.ZipFile(filename, "r")
        self.year = int(self.FindLeagueEntry(zf)[4:])
        self.books = { }

        self.ReadLeague(zf)
        self.BuildIndices()
        self.modified = False
        self.filename = filename

    def FindLeagueEntry(self, zf):
        """
        Find the teamfile entry ('TEAMyyyy') in the
        zipfile 'zf'.  Robust to the use of capital
        or lowercase letters in the zipfile.
        """
        for entry in zf.namelist():
            if entry.upper()[:4] == "TEAM":
                return entry
        return ""

    def FindEntry(self, zf, fnlist):
        """
        Find a file matching an entry in the list 'fnlist' in the
        zipfile 'zf', being robust to case.  Returns
        None if no matching name is found: this may happen
        in Retrosheet zipfiles, where the event files for
        one league only are present.
        """
        for entry in zf.namelist():
            if entry.upper() in fnlist:
                return entry
        return None
            
    def ReadLeague(self, zf):
        name = TempFile()
        
        f = file(name, "w")
        f.write(zf.read(self.FindLeagueEntry(zf)))
        f.close()
        
        f = file(name, "r")
        self.league = CWLeague()
        self.league.Read(f)

        f.close()
        os.remove(name)

        for team in self.IterateTeams():
            self.ReadTeam(zf, team)

    def ReadTeam(self, zf, team):
        fn = self.FindEntry(zf,
                            [ team.team_id + str(self.year) + ".ROS" ])
        if fn != None:
            name = TempFile()
            f = file(name, "w")
            f.write(zf.read(fn))
            f.close()
            
            f = file(name, "r")
            team.Read(f)
            f.close()

            os.remove(name)
            
        fn = self.FindEntry(zf,
                            [ str(self.year) + team.team_id + ".EV" + team.league,
                              str(self.year % 100) + team.team_id + ".EV" + team.league ])
        if fn != None:
            name = TempFile()
            f = file(name, "w")
            f.write(zf.read(fn))
            f.close()
            
            f = file(name, "r")
            self.books[team.team_id] = CWScorebook()
            self.books[team.team_id].Read(f)
            f.close()
            
            os.remove(name)
        else:
            # Just create an empty scorebook if event file is
            # not present.
            self.books[team.team_id] = CWScorebook()

    def Write(self, filename):
        zf = zipfile.ZipFile(filename, "w", zipfile.ZIP_DEFLATED)
        name = TempFile()
        f = file(name, "w")
        self.league.Write(f)
        f.close()
        
        f = file(name, "r")
        zf.writestr("TEAM%d" % self.year, f.read())
        f.close()
        os.remove(name)

        for team in self.IterateTeams():
            self.WriteTeam(zf, team)
        
        zf.close()
        self.filename = filename
        self.modified = False

    def WriteTeam(self, zf, team):
        fn = team.team_id + str(self.year) + ".ROS"
        name = TempFile()
        f = file(name, "w")
        team.Write(f)
        f.close()
        
        f = file(name, "r")
        zf.writestr(fn, f.read())
        f.close()

        fn = str(self.year) + team.team_id + ".EV" + team.league
        f = file(name, "w")
        self.books[team.team_id].Write(f)
        f.close()
        
        f = file(name, "r")
        zf.writestr(fn, f.read())
        f.close()
        
    def NumTeams(self):
        i = 0
        x = self.league.first_roster
        while x != None:
            i += 1
            x = x.next
        return i

    def IterateTeams(self):
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

        self.players = { }
        for team in self.IterateTeams():
            x = team.first_player
            while x != None:
                self.players[x.player_id] = x
                x = x.next

    def NumGames(self, crit=lambda x: True):
        return len(filter(crit, self.games))

    def IterateGames(self, crit=lambda x: True):
        for g in self.games:
            if crit(g):  yield g

    def NumPlayers(self):  return len(self.players)

    def IteratePlayers(self):
        keys = self.players.keys()
        keys.sort()
        for p in keys: yield self.players[p]

    def GetPlayer(self, playerID):
        return self.players[playerID]

    def UniquePlayerID(self, first, last):
        playerID = (last.replace(" ", "").replace("'", ""))[:4].lower()
        while len(playerID) < 4: playerID += "-"
        playerID += first[0].lower()
        i = 1
        while "%s%03d" % (playerID,i) in self.players.keys(): i += 1
        return "%s%03d" % (playerID,i)

    def AddPlayer(self, playerID, firstName, lastName, bats, throws, team):
        p = cw_player_create(playerID, lastName, firstName, bats, throws)

        roster = self.league.first_roster
        while roster.team_id != team:  roster = roster.next

        roster.InsertPlayer(p)
        self.players[playerID] = p
        self.modified = True
        
    def GetTeam(self, teamId):
        return self.league.FindRoster(teamId)

    def GetYear(self):    return self.year

    def AddTeam(self, teamID, city, nickname, leagueID):
        t = CWRoster(teamID, self.year, leagueID, city, nickname)
        
        # We try to keep teams in playerID order.
        # This really ought to be part of the underlying C library
        if self.league.first_roster == None:
            self.league.first_roster = t
            self.league.last_roster = t
        else:
            x = self.league.first_roster
            while x != None and x.team_id < t.team_id:
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
        self.modified = True

    def AddGame(self, game):
        hometeam = game.GetTeam(1)
        self.books[hometeam].InsertGame(game)
        self.games.append(game)
        self.games.sort(lambda x, y: cmp(x.GetDate(), y.GetDate()))
        self.modified = True

    def IsModified(self):   return self.modified
