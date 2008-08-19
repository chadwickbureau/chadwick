import sys
import scorebook
import dw
import reports
import report, report.team
import time

def html(book, overall, firsthalf=None):
    lastDate = max([ x.GetDate() for x in book.Games() ])
    d = time.strftime("%A, %B %d", time.strptime(lastDate, "%Y/%m/%d"))

    teams = overall.stats.values()
    teams.sort(lambda x, y: cmp(y.pct if y.pct is not None else 0.5,
                                x.pct if x.pct is not None else 0.5))

    s = '<table width=75% class=table-nolines cellborder=0>'
    s += '<tr><td colspan=9 nowrap valign=top style=background-color:#cccccc;border-bottom:1px solid #666666><b>Standing of clubs through games of %s</b></td></tr>' % d
        
    s += '<tr><th></th><th>W - L</th><th>PCT</th><th>GB</th><th>Home</th><th>Away</th><th>1-run</th><th>Extra</th></tr>'
    for (i, stat) in enumerate(teams):
        s += '<tr>'
        GB2 = (teams[0].w-stat.w-teams[0].l+stat.l)
        if GB2 == 0:
            GB = ''
        else:
            GB = '%4.1f' % (GB2*0.5)

        s += ("<td><b>%s</b></td> <td align=center>%3d-%3d</td> <td align=center>%s</td><td align=center>%s</td><td align=center>%2d-%2d</td><td align=center>%2d-%2d</td><td align=center>%2d-%2d</td><td align=center>%2d-%2d</td>\n" %
              (("x-"+stat.full_name) if stat.id==firsthalf else stat.full_name,
               stat.w, stat.l,
               ("%5.3f" % stat.pct).replace("0.", " .") if stat.pct is not None else "   - ",
               GB,
               stat.hw, stat.hl,
               stat.rw, stat.rl,
               stat.ow, stat.ol,
               stat.xw, stat.xl))
        s += '</tr>'

    if firsthalf is not None:
        s += '<tr><td colspan=9 nowrap valign=top style=background-color:#cccccc;border-bottom:1px solid #666666><b>x</b>: First-half champion.  First-half champion plus the two other teams with the best records qualify for playoffs.</td></tr>'

    s += '</table>'
    return s
    

book = dw.Reader("/users/arbiter/Documents/CBL2008/2008CBL.chw")

#secondHalf = report.team.Standings(book)
#reports.process_file(book, [secondHalf],
#                     f=lambda x: x.GetDate()>="2008/07/01")

overall = report.team.Standings(book)
reports.process_file(book, [overall])
reports.standings_hack(overall)

print html(book, overall, firsthalf="COR")



