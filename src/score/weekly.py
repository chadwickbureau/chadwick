

if __name__ == "__main__":
    import sys
    import scorebook
    import dw
    import reports
    import report, report.register
    
    book = dw.Reader(sys.argv[1])

    print "CONTINENTAL BASEBALL LEAGUE OFFICIAL STATISTICS"
    print "COORDINATOR OF STATISTICAL SERVICES, THEODORE L. TUROCY, COLLEGE STATION TX -- (979) 997-0666 -- cblstatistics@gmail.com"
    print

    batting = report.register.Batting(book)
    reports.process_file(book, [batting],
                        f = lambda x: x.GetDate()>="2008/08/11")

    subrep = batting.filter(lambda x: x.pa>=10)
    subrep.sorter = lambda x,y: cmp(y.avg, x.avg)
    print str(subrep)


    pitching = report.register.Pitching(book)
    reports.process_file(book, [pitching],
                        f = lambda x: x.GetDate()>="2008/08/11")
    
    subrep = pitching.filter(lambda x: x.outs>=9)
    subrep.sorter = lambda x,y: cmp(x.era, y.era)
    print str(subrep)
    
