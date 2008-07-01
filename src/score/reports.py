#
# $Source$
# $Date: 2008-05-04 23:01:22 -0500 (Sun, 04 May 2008) $
# $Revision: 328 $
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
import report
import report.statline
import report.team
import report.register

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


def process_game(game, reports):
    gameiter = cw.GameIterator(game)
    for report in reports:  report.OnBeginGame(game, gameiter)

    while gameiter.event is not None:
        if gameiter.event.event_text != "NP":
            for report in reports:  report.OnEvent(game, gameiter)

        if gameiter.event.first_sub is not None:
            for report in reports:  report.OnSubstitution(game, gameiter)

        gameiter.NextEvent()

    for report in reports:  report.OnEndGame(game, gameiter)
 
def process_file(book, acclist, f=lambda x: True, monitor=None):
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
            process_game(game, acclist)
            if monitor is not None:
                if not monitor.Update(round(float(i)/float(numGames)*100)):
                    return Fanlse
    return True

def print_report(book, report):
    process_file(book, [ report ])
    print str(report)

class Leaderboard:
    def __init__(self, category, header, limit=3, sorter=None, fmt="%5d"):
        self.category = category
        self.header = header
        self.limit = limit
        if sorter is None:
            self.sorter = lambda x,y: cmp(getattr(y, self.category),
                                          getattr(x, self.category))
        else:
            self.sorter = sorter
        self.fmt = fmt

    def format(self, report):
        stats = report.stats.values()
        stats.sort(self.sorter)
        if getattr(stats[0], self.category) >= getattr(stats[-1], self.category):
            stats = filter(lambda x: getattr(x, self.category) >=
                           getattr(stats[self.limit-1], self.category),
                           stats)
        else:
            stats = filter(lambda x: getattr(x, self.category) <=
                           getattr(stats[self.limit-1], self.category),
                           stats)

        s = "%s\n\n" % self.header
        for (i, stat) in enumerate(stats):
            s += "%-30s " % (stat.player.GetSortName() + " (" +
                             stat.team.GetID() + ")")
            s += (self.fmt + "\n") % getattr(stat, self.category)

        return s
    
def print_leaders(report, category, header, limit=3, sorter=None, fmt="%5d"):
    board = Leaderboard(category, header, limit, sorter, fmt)
    print board.format(report)


def standings_hack(rep):
    return
    rep.stats["MCK"]["g"] += 1
    rep.stats["MCK"]["l"] += 1
    rep.stats["MCK"]["hl"] += 1
    rep.stats["MCK"]["ol"] += 1

    rep.stats["COR"]["g"] += 1
    rep.stats["COR"]["w"] += 1
    rep.stats["COR"]["ow"] += 1
    rep.stats["COR"]["rw"] += 1
    


if __name__ == "__main__":
    import sys
    import scorebook
    import dw

    book = dw.Reader(sys.argv[1])

    print "CONTINENTAL BASEBALL LEAGUE OFFICIAL STATISTICS"
    print "COORDINATOR OF STATISTICAL SERVICES, THEODORE L. TUROCY, COLLEGE STATION TX -- (979) 997-0666 -- cblstatistics@gmail.com"
    print

    print "STANDING OF CLUBS THROUGH GAMES OF %s" % sys.argv[2]

    standings = report.team.Standings(book)
    process_file(book, [standings])
    standings_hack(standings)
    print str(standings)


    print "TEAM BATTING"
    print_report(book, report.team.TeamBattingTotals(book))

    print "TEAM PITCHING"
    print_report(book, report.team.TeamPitchingTotals(book))

    print "TEAM FIELDING"
    print_report(book, report.team.TeamFieldingTotals(book))

    batting = report.register.Batting(book)
    process_file(book, [ batting ])

    print "LEADING BATTERS (MINIMUM 3.1 PLATE APPEARANCES PER TEAM GAME PLAYED)"
    subrep = batting.filter(lambda x: x.pa>=3.1*standings.stats[x.team.GetID()].g)
    subrep.sorter = lambda x,y: cmp(y.avg, x.avg)
    subrep.limit = 10
    print str(subrep)

    print_leaders(batting, "hr", "HOME RUNS")
    print_leaders(batting, "bi", "RUNS BATTED IN")

    print_leaders(subrep, "slg", "SLUGGING PERCENTAGE",
                  fmt = "%.3f")
    print_leaders(subrep, "obp", "ON-BASE PERCENTAGE",
                  fmt = "%.3f")

    print_leaders(batting, "r", "RUNS")
    print_leaders(batting, "h", "HITS")
    print_leaders(batting, "h2b", "DOUBLES")
    print_leaders(batting, "h3b", "TRIPLES")
    print_leaders(batting, "tb", "TOTAL BASES")
    print_leaders(batting, "bb", "BASES ON BALLS")
    print_leaders(batting, "sb", "STOLEN BASES")
    
    print "ALL BATTERS, ALPHABETICALLY"
    print str(batting)

    #for team in book.Teams():
    #    subrep = rep.filter(lambda x: x.team is team)
    #    print str(subrep)

    pitching = report.register.Pitching(book)
    process_file(book, [ pitching ])

    print "LEADING PITCHERS (MINIMUM 1 INNING PITCHED PER TEAM GAME PLAYED)"

    subrep = pitching.filter(lambda x: x.outs>=3*standings.stats[x.team.GetID()].g)
    subrep.sorter = lambda x,y: cmp(x.era, y.era)
    subrep.limit = 10
    print str(subrep)

    print_leaders(pitching, "w", "WINS")
    print_leaders(pitching, "sv", "SAVES")
    print_leaders(pitching, "so", "STRIKEOUTS")

    print_leaders(subrep, "so9", "STRIKEOUTS PER 9 INNINGS",
                  sorter = lambda x,y: cmp(getattr(y, "so9"),
                                           getattr(x, "so9")),
                  fmt = "%5.2f")
    
    print_leaders(subrep, "br9", "FEWEST RUNNERS PER 9 INNINGS",
                  sorter = lambda x,y: cmp(getattr(x, "br9"),
                                           getattr(y, "br9")),
                  fmt = "%5.2f")

    print_leaders(subrep, "bb9", "FEWEST WALKS PER 9 INNINGS",
                  sorter = lambda x,y: cmp(getattr(x, "bb9"),
                                           getattr(y, "bb9")),
                  fmt = "%5.2f")

    print_leaders(subrep, "h9", "FEWEST HITS PER 9 INNINGS",
                  sorter = lambda x,y: cmp(getattr(x, "h9"),
                                           getattr(y, "h9")),
                  fmt = "%5.2f")


    print_leaders(pitching, "g", "GAMES PITCHED")
    

    print "ALL PITCHERS, ALPHABETICALLY"
    print str(pitching)
    

    print "INDIVIDUAL FIELDING BY POSITION"
    print
    print "KEY TO EXPANDED FIELDING STATISTICS:"
    print "BIP: BATTED BALLS IN PLAY"
    print "BF:  BALLS FIELDED AND TURNED INTO OUTS"
    print "BF/9:  NUMBER OF BALLS TURNED INTO OUTS PER NINE INNINGS AT THE POSITION"
    
    for pos in xrange(1, 10):
        fielding = report.register.Fielding(book, pos)
        process_file(book, [ fielding ])
        print str(fielding)


