import report.team



if __name__ == "__main__":
    import sys
    import scorebook
    import dw
    import reports

    if len(sys.argv) >= 2:
        filename = sys.argv[1]
    else:
        filename = "/users/arbiter/Documents/CBL2008/2008CBL.chw"

    book = dw.Reader(filename)

    teams = [ x for x in book.Teams() ]

    for team in teams:
        for opp in [ x for x in teams if x != team ]:
            standings = report.team.Standings(book)

            reports.process_file(book, [ standings ],
                                 f=lambda y: team.GetID() in y.GetTeams() and \
                                             opp.GetID() in y.GetTeams())
            
            print str(standings)
