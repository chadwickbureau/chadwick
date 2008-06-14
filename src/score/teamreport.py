import report
import report.statline

def team_batting_report(rep, sorter=None, limit=None, page_break=None):
    stats = rep.stats.values()
    if sorter is not None:
        stats.sort(sorter)

    s = ""
    for (i, stat) in enumerate(stats):
        if limit is not None and i >= limit:
            break
            
        if page_break is not None and i % page_break == 0:
            s += "\n #  Player                   AVG   SLG   OBP   G  AB   R   H  TB 2B 3B HR RBI  BB IW  SO GDP HP SH SF SB CS  1B 2B SS 3B OF  C\n"
            

        if stat.player.GetBats() == "R":
            bats = " "
        elif stat.player.GetBats() == "L":
            bats = "*"
        elif stat.player.GetBats() == "S" or stat.player.GetBats() == "B":
            bats = "#"
        else:
            bats = " "
                
        s += ("%2s %s%-19s    %s %s %s %3d %3d %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d  %2d %2d %2d %2d %2d %2d  %2d %2d %2d %2d %2d %2d\n" %
              (stat.player.GetUniform() if stat.player.GetUniform() is not None else "",
               bats,
               stat.player.GetSortName(),
               ("%5.3f" % stat.avg).replace("0.", " .")
               if stat.avg is not None else "   - ",
               ("%5.3f" % stat.slg).replace("0.", " .")
               if stat.slg is not None else "   - ",
               ("%5.3f" % stat.obp).replace("0.", " .")
               if stat.obp is not None else "   - ",
               len(stat.games),
               stat.ab, stat.r, stat.h, stat.tb,
               stat.h2b, stat.h3b, stat.hr, stat.bi,
               stat.bb, stat.ibb, stat.so, stat.gdp, stat.hp,
               stat.sh, stat.sf, stat.sb, stat.cs,
               len(stat.gamespos[3]),
               len(stat.gamespos[4]),
               len(stat.gamespos[6]),
               len(stat.gamespos[5]),
               len(stat.gamespos[7])+len(stat.gamespos[8])+len(stat.gamespos[9]),
               len(stat.gamespos[2])))

    stat = sum(stats, report.statline.Batting())
    
    s += ("%2s %s%-19s    %s %s %s %3d %3d %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d  %2d %2d %2d %2d %2d %2d\n" %
              ("", " ", "TOTALS",
               ("%5.3f" % stat.avg).replace("0.", " .")
               if stat.avg is not None else "   - ",
               ("%5.3f" % stat.slg).replace("0.", " .")
               if stat.slg is not None else "   - ",
               ("%5.3f" % stat.obp).replace("0.", " .")
               if stat.obp is not None else "   - ",
               len(stat.games),
               stat.ab, stat.r, stat.h, stat.tb,
               stat.h2b, stat.h3b, stat.hr, stat.bi,
               stat.bb, stat.ibb, stat.so, stat.gdp, stat.hp,
               stat.sh, stat.sf, stat.sb, stat.cs))

    return s

def team_pitching_report(rep, sorter=None, limit=None, page_break=None):
    stats = rep.stats.values()
    if sorter is not None:
        stats.sort(sorter)

    s = ""
    for (i, stat) in enumerate(stats):
        if limit is not None and i >= limit:
            break

        if stat.player.GetThrows() == "R":
            throws = " "
        elif stat.player.GetThrows() == "L":
            throws = "*"
        else:
            throws = " "

        if page_break is not None and i % page_break == 0:
            s += "\n #  Player                  W- L   PCT    ERA  G GS CG SHO GF SV    IP TBF  AB   H   R  ER HR SH SF HB  BB IW  SO WP BK\n"

        s += ("%2s %s%-20s   %2d-%2d %s %s %2d %2d %2d  %2d %2d %2d %3d.%1d %3d %3d %3d %3d %3d %2d %2d %2d %2d %3d %2d %3d %2d %2d\n" % 
                (stat.player.GetUniform() if stat.player.GetUniform() is not None else "",
                 throws, stat.player.GetSortName(),
                 stat.w, stat.l,
                 ("%.3f" % stat.pct).replace("0.", " .")
                 if stat.pct is not None else "   - ",
                 ("%6.2f" % stat.era) if stat.era is not None else "    - ",
                 len(stat.games),
                 stat.gs, stat.cg, stat.sho, stat.gf, stat.sv,
                 stat.outs / 3, stat.outs % 3, stat.bf, stat.ab, stat.h,
                 stat.r, stat.er,
                 stat.hr, stat.sh, stat.sf, stat.hb, stat.bb, stat.ibb,
                 stat.so, stat.wp, stat.bk))

    stat = sum(stats, report.statline.Pitching())
    
    s += ("%2s %s%-20s   %2d-%2d %s %s %2d %2d %2d  %2d %2d %2d %3d.%1d %3d %3d %3d %3d %3d %2d %2d %2d %2d %3d %2d %3d %2d %2d\n" % 
          ("", " ", "TOTALS",
                 stat.w, stat.l,
                 ("%.3f" % stat.pct).replace("0.", " .")
                 if stat.pct is not None else "   - ",
                 ("%6.2f" % stat.era) if stat.era is not None else "    - ",
                 len(stat.games),
                 stat.gs, stat.cg, stat.sho, stat.gf, stat.sv,
                 stat.outs / 3, stat.outs % 3, stat.bf, stat.ab, stat.h,
                 stat.r, stat.er-stat.tur,
                 stat.hr, stat.sh, stat.sf, stat.hb, stat.bb, stat.ibb,
                 stat.so, stat.wp, stat.bk))

    return s

if __name__ == "__main__":
    import sys
    import scorebook
    import dw
    import report.register
    import reports

    book = dw.Reader(sys.argv[1])

    print "CONTINENTAL BASEBALL LEAGUE OFFICIAL STATISTICS"
    print "COORDINATOR OF STATISTICAL SERVICES, THEODORE L. TUROCY, COLLEGE STATION TX -- (979) 997-0666 -- cblstatistics@gmail.com"
    print

    print "STANDING OF CLUBS THROUGH GAMES OF %s" % sys.argv[2]

    standings = report.team.Standings(book)
    reports.process_file(book, [standings])
    print str(standings)


    print "TEAM BATTING"
    reports.print_report(book, report.team.TeamBattingTotals(book))

    print "TEAM PITCHING"
    reports.print_report(book, report.team.TeamPitchingTotals(book))

    print "TEAM FIELDING"
    reports.print_report(book, report.team.TeamFieldingTotals(book))

    print "\f",
    
    batting = report.register.Batting(book)
    pitching = report.register.Pitching(book)
    
    reports.process_file(book, [batting, pitching])

    teams = [ x for x in book.Teams() ]
    teams.sort(lambda x, y: cmp(x.GetCity(), y.GetCity()))
    
    for team in teams:
        print "CONTINENTAL BASEBALL LEAGUE OFFICIAL STATISTICS"
        print "COORDINATOR OF STATISTICAL SERVICES, THEODORE L. TUROCY, COLLEGE STATION TX -- (979) 997-0666 -- cblstatistics@gmail.com"
        print

        print "%s %s TEAM STATISTICS THROUGH GAMES OF %s" % \
              (team.GetCity().upper(), team.GetNickname().upper(),
               sys.argv[2])

        teambatting = batting.filter(lambda x: x.team is team and
                                               len(x.gamesbatting) > 0)
        teambatting.page_break = 100
        print team_batting_report(teambatting,
                                  page_break=100,
                                  sorter=lambda x, y: cmp(y.avg, x.avg))

        teampitching = pitching.filter(lambda x: x.team is team)
        teampitching.page_break = 100
        print team_pitching_report(teampitching,
                                   page_break=100,
                                   sorter=lambda x, y: cmp(x.era, y.era))
        
        print "\f",

    print
