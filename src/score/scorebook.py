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
    def __init__(self):
        self.books = { }
        self.modified = False
        self.year = 2005
        self.league = cw_league_create()

    def __del__(self):
        #for book in self.books:
        #    cw_scorebook_cleanup(book)
        #cw_league_cleanup(self.league)
        pass

    def Read(self, filename):
        zf = zipfile.ZipFile(filename, "r")
        self.year = int(self.FindLeagueEntry(zf)[4:])
        self.books = { }

        self.ReadLeague(zf)
        self.modified = False

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
        self.league = cw_league_create()
        cw_league_read(self.league, f)

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
            cw_roster_read(team, f)
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
            self.books[team.team_id] = cw_scorebook_create()
            cw_scorebook_read(self.books[team.team_id], f)
            f.close()
            
            os.remove(name)
        else:
            # Just create an empty scorebook if event file is
            # not present.
            self.books[team.team_id] = cw_scorebook_create()

    def Write(self, filename):
        zf = zipfile.ZipFile(filename, "w", zipfile.ZIP_DEFLATED)
        name = TempFile()
        f = file(name, "w")
        cw_league_write(self.league, f)
        f.close()
        
        f = file(name, "r")
        zf.writestr("TEAM%d" % self.year, f.read())
        f.close()
        os.remove(name)

        for team in self.IterateTeams():
            self.WriteTeam(zf, team)
        
        zf.close()
        self.modified = False

    def WriteTeam(self, zf, team):
        fn = team.team_id + str(self.year) + ".ROS"
        name = TempFile()
        f = file(name, "w")
        cw_roster_write(team, f)
        f.close()
        
        f = file(name, "r")
        zf.writestr(fn, f.read())
        f.close()

        fn = str(self.year) + team.team_id + ".EV" + team.league
        f = file(name, "w")
        cw_scorebook_write(self.books[team.team_id], f)
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
    
    def NumGames(self):
        i = 0
        for x in self.books.keys():
            g = self.books[x].first_game
            while g != None:
                i += 1
                g = g.next
        return i
        
    def IterateGames(self):
        for x in self.books.keys():
            g = self.books[x].first_game
            while g != None:
                yield g
                g = g.next
        raise StopIteration
                
    def GetTeam(self, teamId):
        return cw_league_roster_find(self.league, teamId)

    def GetYear(self):    return self.year

    def AddGame(self, game):
        hometeam = cw_game_info_lookup(game, "hometeam")
        cw_scorebook_append_game(self.books[hometeam], game)
        self.modified = True

    def IsModified(self):   return self.modified
