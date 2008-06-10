#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A panel showing a narrative of the events in a game
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

import wx, wx.html
import libchadwick as cw

def GetInningLabel(inning, halfInning):
    if halfInning == 0:
        x = "Top of the "
    else:
        x = "Bottom of the "

    if inning % 10 == 1 and inning != 11:
        x += "%dst" % inning
    elif inning % 10 == 2 and inning != 12:
        x += "%dnd" % inning
    elif inning % 10 == 3 and inning != 13:
        x += "%drd" % inning
    else:
        x += "%dth" % inning
    return x


class NarrativePanel(wx.html.HtmlWindow):
    def __init__(self, parent, doc):
        wx.html.HtmlWindow.__init__(self, parent)
        self.doc = doc

    def PrintHeading(self):
        text = '<table width="600" cellspacing="0" cellpadding="0">'
        text += '<tr><td align="center">%s at %s</td></tr>' % \
                (self.doc.GetRoster(0).GetName(),
                 self.doc.GetRoster(1).GetName())
        text += '<tr><td align="center">Game of %s' % \
                self.doc.GetGame().GetDate()
        number = self.doc.GetGame().GetNumber()
        if number == 1:
            text += " (first game)"
        elif number == 2:
            text += " (second game)"

        text += "</tr></table>\n"
        return text

    def PrintStarters(self):
        starters = [ ]
        text = '<table width="600" cellspacing="0" cellpadding="0">'
        for t in [0, 1]:
            starters.append([ self.doc.GetGame().GetStarter(t, slot)
                              for slot in xrange(1, 10) ])

        text += '<tr><td colspan="6" align="center"><b>Starting lineups</b></td></tr>'
        text += '<tr>'
        for t in [ 0, 1 ]:
            text += '<td colspan="3" align="center"><b>%s</b></td>' % \
                    self.doc.GetRoster(t).GetCity()
        text += '</tr>'

        positions = [ "", "p", "c", "1b", "2b", "3b", "ss",
                      "lf", "cf", "rf", "dh" ]
        for slot in range(9):
            text += '<tr>'
            for t in [ 0, 1 ]:
                text += '<td width="5%%" align="center">%d</td>' % (slot+1)
                text += '<td width="35%%">%s</td>' % self.doc.GetRoster(t).GetPlayer(starters[t][slot].player_id).GetName()
                text += '<td width="10%%" align="center">%s</td>' % positions[starters[t][slot].pos]
            text += '</tr>'

        pitchers = [ self.doc.GetGame().GetStarter(t, 0) for t in [0, 1] ]

        if pitchers[0] != None or pitchers[1] != None:
            text += '<tr>'
            for t in [ 0, 1 ]:
                text += '<td width="5%%" align="center">P</td>'
                if pitchers[0] != None:
                    text += '<td>%s</td><td></td>' % self.doc.GetRoster(t).GetPlayer(pitchers[t].player_id).GetName()
                else:
                    text += '<td></td><td></td>'
            text += '</tr>'
                
        text += "</table>\n"
        return text

    def PrintPlays(self):
        gameiter = cw.GameIterator(self.doc.GetGame())
        text = '<table width="600" cellspacing="0" cellpadding="0">'

        lastHalf = 1
        
        if gameiter.game.first_comment != None:
            com = gameiter.game.first_comment
            
            while com != None:
                text += "Comment: %s\n" % com.text
                com = com.next

        while gameiter.event != None:
            team = gameiter.GetHalfInning()
            
            if team != lastHalf:
                text += '<tr><td colspan="5">&nbsp;</td></tr>'
                text += "<tr>"
                text += '<td colspan="5" bgcolor="#DDDDDD"><b>%s - %s</b></td>' % \
                        (self.doc.GetRoster(gameiter.GetHalfInning()).GetCity(),
                         GetInningLabel(gameiter.GetInning(),
                                        gameiter.GetHalfInning()))
                text += "</tr>"
                lastHalf = team
            
            if gameiter.event.event_text == "NP":
                sub = gameiter.event.first_sub
                
                while sub != None:
                    subname = self.doc.GetRoster(sub.GetTeam()).GetPlayer(sub.GetPlayerID()).GetName()
                    replaces = self.doc.GetRoster(sub.GetTeam()).GetPlayer(gameiter.GetPlayer(sub.GetTeam(), sub.GetSlot())).GetName()
                    if sub.GetTeam() == team:
                        if sub.GetPosition() == 11:
                            x = '<tr><td colspan="5"><i><b>%s batting for %s</b></i></td></tr>' % (subname, replaces)
                        else:
                            x = '<tr><td colspan="5"><i><b>%s running for %s</b></i></td></tr>' % (subname, replaces)
                    else:
                        x = ('<tr><td colspan="5"><i><b>%s replaces %s (%s)</b></i></td></tr>' %
                             (subname, replaces,
                              [ "", "pitching", "catching",
                                "first base", "second base", "third base",
                                "shortstop", "left field",
                                "center field", "right field",
                                "designated hitter" ][sub.GetPosition()]))
                    text += x + "\n"
                    sub = sub.next
            else:                
                batterId = gameiter.GetPlayer(team,
                                              gameiter.NumBatters(team) % 9 + 1)
                ros = self.doc.GetRoster(team)
                batter = ros.GetPlayer(batterId)
                
                x = "<tr><td>%s</td> " % batter.GetName()
                x += "<td>%d-%d</td> " % (gameiter.GetTeamScore(0),
                                          gameiter.GetTeamScore(1))
                x += "<td>"
                for base in [3, 2, 1]:
                    x += { True: "x", False: "-" }[gameiter.GetRunner(base) != ""]
                
                x += "%d</td> " % gameiter.GetOuts()
                x += "<td>" + gameiter.event.event_text + "</td></tr>"
                text += x

            if gameiter.event.first_comment != None:
                com = gameiter.event.first_comment

                while com != None:
                    text += '<tr><td colspan="5"><i>%s</i></td></tr>' % com.text
                    com = com.next
                    
            gameiter.NextEvent()

        text += '</table>'
        return text

    def OnUpdate(self):   self.Rebuild()

    def Rebuild(self):
        x = '<html><head></head><body>'
        x += self.PrintHeading() + '&nbsp;' + self.PrintStarters() + '&nbsp;' + self.PrintPlays()
        x += '</body></html>'
        self.SetPage(x)
        
