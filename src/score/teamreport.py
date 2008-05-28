

if __name__ == "__main__":
    import sys
    import scorebook
    import dw
    import report.register
    import reports

    book = dw.Reader(sys.argv[1])

    batting = report.register.Batting(book)
    pitching = report.register.Pitching(book)
    
    reports.process_file(book, [batting, pitching])
    
    for team in book.Teams():
        print "CONTINENTAL BASEBALL LEAGUE OFFICIAL STATISTICS"
        print "COORDINATOR OF STATISTICAL SERVICES, THEODORE L. TUROCY, COLLEGE STATION TX -- (979) 997-0666 -- cblstatistics@gmail.com"
        print

        print "%s %s TEAM STATISTICS THROUGH GAMES OF %s" % \
              (team.GetCity().upper(), team.GetNickname().upper(),
               sys.argv[2])

        teambatting = batting.filter(lambda x: x.team is team)
        teambatting.page_break = 100
        print str(teambatting)

        teampitching = pitching.filter(lambda x: x.team is team)
        teampitching.page_break = 100
        print str(teampitching)
        
        
