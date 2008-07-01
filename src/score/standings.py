import sys
import scorebook
import dw
import reports
import report, report.team

book = dw.Reader("/users/arbiter/Documents/CBL2008/2008CBL.chw")

standings = report.team.Standings(book)
reports.process_file(book, [standings])
print standings.html("Tuesday, July 1", firsthalf="COR")



