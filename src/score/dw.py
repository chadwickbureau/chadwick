#
# $Source$
# $Date: 2007-04-17 15:19:25 -0500 (Tue, 17 Apr 2007) $
# $Revision: 265 $
#
# DESCRIPTION:
# Functions to import and export scorebooks to Retrosheet-style zipfiles
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

"""
Importing and exporting scorebooks to Retrosheet zip archive format.
"""

import os, tempfile, zipfile
import libchadwick as cw
import scorebook

############################################################################
# Auxiliary helper functions
############################################################################

def TempFile():
    """
    Wrapper around tempfile.mkstemp(): closes the original file and
    returns the file name.
    """
    f = tempfile.mkstemp()
    name = f[1]
    os.close(f[0])
    return name    


############################################################################
# Retrosheet zip archive importer
############################################################################

def FindLeagueEntry(zf):
    """
    Find the teamfile entry ('TEAMyyyy') in the
    zipfile 'zf'.  Robust to the use of capital
    or lowercase letters in the zipfile.
    """
    for entry in zf.namelist():
        if entry.upper()[:4] == "TEAM":
            return entry
    return ""

def FindEntry(zf, fnlist):
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

def ReadTeam(zf, year, team):
    """
    Read a team's roster and event file from the zip archive 'zf'.
    """
    fn = FindEntry(zf, [ team.GetID() + str(year) + ".ROS" ])
    if fn != None:
        name = TempFile()
        f = file(name, "w")
        f.write(zf.read(fn))
        f.close()

        f = file(name, "r")
        team.Read(f)
        f.close()

        os.remove(name)

    fn = FindEntry(zf,
                   [ str(year) + team.GetID() + ".EV" + team.league,
                     str(year % 100) + team.GetID() + ".EV" + team.league ])
    if fn != None:
        name = TempFile()
        f = file(name, "w")
        f.write(zf.read(fn))
        f.close()

        f = file(name, "r")
        book = cw.Scorebook()
        book.Read(f)
        f.close()

        os.remove(name)
    else:
        # Just create an empty scorebook if event file is
        # not present.
        book = cw.Scorebook()

    return book

def ReadLeague(book, zf):
    """
    Find and read the league file, using the underlying Retrosheet
    engine.  This creates a cw.League object, as well as the rosters
    for individual teams.
    """
    name = TempFile()

    f = file(name, "w")
    f.write(zf.read(FindLeagueEntry(zf)))
    f.close()

    f = file(name, "r")
    book.league = cw.League()
    book.league.Read(f)

    f.close()
    os.remove(name)

    for team in book.Teams():
        book.books[team.GetID()] = ReadTeam(zf, book.year, team)

def Reader(filename):
    """
    Imports a Retrosheet zip archive to a new scorebook
    """
    book = scorebook.Scorebook()
    
    zf = zipfile.ZipFile(filename, "r")
    book.year = int(FindLeagueEntry(zf)[4:])
    book.books = { }

    ReadLeague(book, zf)
    book.BuildIndices()
    book.modified = False
    book.filename = filename

    return book


############################################################################
# Retrosheet zip archive exporter
############################################################################

def WriteTeam(book, zf, team):
    """
    Write a team's roster and home game records to the archive
    """
    fn = team.GetID() + str(book.year) + ".ROS"
    name = TempFile()
    f = file(name, "w")
    team.Write(f)
    f.close()

    f = file(name, "r")
    zf.writestr(fn, f.read())
    f.close()

    fn = str(book.year) + team.GetID() + ".EV" + team.league
    f = file(name, "w")
    book.books[team.GetID()].Write(f)
    f.close()

    f = file(name, "r")
    zf.writestr(fn, f.read())
    f.close()

def Writer(book, filename):
    """
    Write the contents of scorebook 'book' to the file 'filename'.
    TODO: Writer should take 'year' as a parameter, since Retrosheet files
    consist of games from one year only.
    """

    zf = zipfile.ZipFile(filename, "w", zipfile.ZIP_DEFLATED)
    name = TempFile()
    f = file(name, "w")
    book.league.Write(f)
    f.close()

    f = file(name, "r")
    zf.writestr("TEAM%d" % book.year, f.read())
    f.close()
    os.remove(name)

    for team in book.Teams():
        WriteTeam(book, zf, team)

    zf.close()
    book.filename = filename
    book.modified = False


if __name__ == '__main__':
    import sys

    book = Reader(sys.argv[1])
    print "Scorebook is from %d" % (book.year)
    print "There are %d teams" % (len([t for t in book.league.Teams()]))

    Writer(book, sys.argv[2])
