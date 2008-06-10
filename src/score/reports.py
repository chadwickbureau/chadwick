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
    def __init__(self, category, header, limit=3):
        self.category = category
        self.header = header
        self.limit = limit
        self.sorter = lambda x,y: cmp(getattr(y, self.category),
                                      getattr(x, self.category))
        
    def format(self, report):
        stats = report.stats.values()
        stats.sort(self.sorter)
        stats = filter(lambda x: getattr(x, self.category) >=
                                 getattr(stats[self.limit-1], self.category),
                       stats)

        s = "%s\n\n" % self.header
        for (i, stat) in enumerate(stats):
            s += "%-30s %3d\n" % (stat.player.GetSortName() + " (" +
                                  stat.team.GetID() + ")",
                                  getattr(stat, self.category))

        return s
    
def print_leaders(report, category, header, limit=3):
    board = Leaderboard(category, header, limit)
    print board.format(report)

# This is a hack to add statistics for the two missing games
def batting_hack(report):
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Elliot" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.games.add("MCK200805282")
    player.ab += 5+3
    player.h += 2+1
    player.bb += 1
    player.bi += 1+1
    player.sb += 2
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Wilkerson" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.games.add("MCK200805282")
    player.ab += 4+2
    player.r += 1
    player.h += 1+1
    player.bb += 1
    player.bi += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Forsythe" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.games.add("MCK200805282")
    player.ab += 3+4
    player.r += 2
    player.h += 2
    player.sb += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Pagan" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.games.add("MCK200805282")
    player.ab += 3+4
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Filyaw" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.games.add("MCK200805282")
    player.ab += 2+3
    player.r += 1
    player.h += 2
    player.bi += 3
    player.sb += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Wilson" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.ab += 4
    player.h += 2
    player.bi += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Bourgeois" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")
    player.ab += 3
    player.r += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Anderson" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")
    player.ab += 1+4
    player.r += 1
    player.h += 1
    player.bb += 2
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Bird" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")
    player.ab += 2
    player.h += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Revis" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")
    player.r += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Rugowski" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")
    player.ab += 2

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Revis" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.ab += 2+4

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Betourne" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")
    player.games.add("MCK200805282")
    player.ab += 1+2
    player.r += 1
    player.h += 1
    player.bi += 1
    player.bb += 2
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Leger" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Kennedy" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805282")

    # Also 1 G for Foeman
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Milton" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Hastings" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200805281")

# Batting hack for games of June 7 only.
def batting_hack(report):
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Mayer" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 5
    player.h += 1
    player.bi += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Wade" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.h += 2
    player.cs += 1
    player.bb += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Perri" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.h += 0
    player.hp += 1
    player.sb += 1
    player.so += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Toland" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 1
    player.h += 1
    player.so += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Brown" and x.player.GetFirstName() == "Eric" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 1
    player.h += 1
    player.sb += 1
    player.so += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Massie" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 1
    player.h += 2
    player.bi += 1
    player.so += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Czekaj" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 1
    player.h += 1
    player.bi += 1
    player.sb += 1
    
    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Suncar" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 4

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Wells" and
                  x.team.GetID() == "BAY" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.h += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Elliot" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.h += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Wilkerson" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 1
    player.h += 2

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Forsythe" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 1
    player.h += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Pagan" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 4
    player.r += 0
    player.h += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Filyaw" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.r += 0
    player.h += 0
    player.bi += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Wilson" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.r += 0
    player.h += 1
    player.bi += 1
    player.so += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Betourne" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.r += 0
    player.h += 0
    player.bi += 0
    player.so += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Bourgeois" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.r += 0
    player.h += 1
    player.bi += 0
    player.so += 1

    player = [ x for x in report.stats.values()
               if x.player.GetLastName() == "Revis" and
                  x.team.GetID() == "MCK" ][0]
    player.games.add("MCK200806070")
    player.ab += 3
    player.r += 0
    player.h += 0
    player.bi += 0

# Pitching hack for games of June 7 only.
def pitching_hack(rep, book):
    import report.statline
    try:
        player = [ x for x in report.stats.values()
                   if x.player.GetLastName() == "Revere" and
                   x.team.GetID() == "BAY" ][0]
    except:
        player = report.statline.Pitching(book.GetPlayer("2009"),
                                          book.GetTeam("BAY"))
        rep.stats[("2009", "BAY")] = player
    player.games.add("MCK200806070")
    player.w += 1
    player.gs += 1
    player.cg += 1
    player.outs += 27
    player.h += 7
    player.r += 2
    player.er += 2
    player.so += 3
    player.bf += 31
    player.ab += 31

    try:
        player = [ x for x in report.stats.values()
                   if x.player.GetLastName() == "Milton" and
                   x.team.GetID() == "MCK" ][0]
    except:
        player = report.statline.Pitching(book.GetPlayer("2138"),
                                          book.GetTeam("MCK"))
        rep.stats[("2138", "MCK")] = player

    player.games.add("MCK200806070")
    player.gs += 1
    player.l += 1
    player.outs += 20
    player.h += 8
    player.r += 4
    player.er += 4
    player.so += 2
    player.wp += 1
    player.bf += 28
    player.ab += 27
    player.hb += 1

    try:
        player = [ x for x in report.stats.values()
                   if x.player.GetLastName() == "Foeman" and
                   x.team.GetID() == "MCK" ][0]
    except:
        player = report.statline.Pitching(book.GetPlayer("1838"),
                                          book.GetTeam("MCK"))
        rep.stats[("1838", "MCK")] = player
    player.games.add("MCK200806070")
    player.outs += 4
    player.bb += 1
    player.so += 2
    player.bf += 5
    player.ab += 4
    
    try:
        player = [ x for x in report.stats.values()
                   if x.player.GetLastName() == "Brown" and
                   x.team.GetID() == "MCK" ][0]
    except:
        player = report.statline.Pitching(book.GetPlayer("4985"),
                                          book.GetTeam("MCK"))
        rep.stats[("4985", "MCK")] = player
    player.games.add("MCK200806070")
    player.outs += 3
    player.h += 1
    player.bf += 4
    player.ab += 4

def standings_hack(report):
    report.stats["MCK"].g += 2
    report.stats["MCK"].w += 1
    report.stats["MCK"].l += 1
    report.stats["MCK"].ow += 1
    report.stats["MCK"].ol += 1
    report.stats["MCK"].hw += 1
    report.stats["MCK"].hl += 1

    report.stats["COR"].g += 2
    report.stats["COR"].w += 1
    report.stats["COR"].l += 1
    report.stats["COR"].ow += 1
    report.stats["COR"].ol += 1
    report.stats["COR"].rw += 1
    report.stats["COR"].rl += 1

    # for game of 6-7
    report.stats["MCK"].g += 1
    report.stats["MCK"].l += 1
    report.stats["MCK"].hl += 1

    report.stats["BAY"].g += 1
    report.stats["BAY"].w += 1
    report.stats["BAY"].rw += 1
    


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

    #batting_hack(batting)

    print "LEADING BATTERS (MINIMUM 3.1 PLATE APPEARANCES PER TEAM GAME PLAYED)"
    subrep = batting.filter(lambda x: x.pa>=3.1*standings.stats[x.team.GetID()].g)
    subrep.sorter = lambda x,y: cmp(y.avg, x.avg)
    subrep.limit = 10
    print str(subrep)

    print_leaders(batting, "r", "RUNS")
    print_leaders(batting, "h", "HITS")
    print_leaders(batting, "hr", "HOME RUNS")
    print_leaders(batting, "h2b", "DOUBLES")
    print_leaders(batting, "h3b", "TRIPLES")
    print_leaders(batting, "tb", "TOTAL BASES")
    print_leaders(batting, "bb", "BASES ON BALLS")
    print_leaders(batting, "so", "STRIKEOUTS")
    print_leaders(batting, "sb", "STOLEN BASES")
    print_leaders(batting, "cs", "CAUGHT STEALING")
    
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
    print_leaders(pitching, "bb", "BASES ON BALLS")
    print_leaders(pitching, "so", "STRIKEOUTS")
    print_leaders(pitching, "g", "GAMES PITCHED")
    print_leaders(pitching, "hr", "HOME RUNS ALLOWED")
    

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


