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

import wx
from libchadwick import *

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


class NarrativePanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        self.textCtrl = wx.TextCtrl(self, wx.ID_ANY, "",
                                    style = wx.TE_MULTILINE | wx.TE_READONLY)
        self.textCtrl.SetFont(wx.Font(10, wx.MODERN, wx.NORMAL, wx.NORMAL))

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.textCtrl, 1, wx.EXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()

    def SetDocument(self, doc):
        self.doc = doc
        self.OnUpdate()

    def PrintHeading(self):
        text = ""
        text += "%s at %s\n" % (self.doc.GetRoster(0).GetName(),
                                self.doc.GetRoster(1).GetName())
        text += "Game of %s" % self.doc.GetGame().GetDate()
        number = self.doc.GetGame().GetNumber()
        if number == 1:
            text += " (first game)\n"
        elif number == 2:
            text += " (second game)\n"
        else:
            text += "\n"

        text += "\n"
        return text

    def PrintStarters(self):
        starters = [ ]
        text = ""
        for t in [0, 1]:
            starters.append([ self.doc.GetGame().GetStarter(t, slot)
                              for slot in xrange(1, 10) ])

        text += "Starting lineups:\n\n"
        text += "   %-25s %-25s\n" % (self.doc.GetRoster(0).GetCity(),
                                      self.doc.GetRoster(1).GetCity())
        positions = [ "", "p", "c", "1b", "2b", "3b", "ss",
                      "lf", "cf", "rf", "dh" ]
        for slot in range(9):
            text += ("%d. %-20s %-2s   %-20s %-2s\n" %
                     (slot+1,
                      starters[0][slot].name,
                      positions[starters[0][slot].pos],
                      starters[1][slot].name,
                      positions[starters[1][slot].pos]))

        pitchers = [ self.doc.GetGame().GetStarter(t, 0) for t in [0, 1] ]

        if pitchers[0] != None or pitchers[1] != None:
            text += "P  "
            if pitchers[0] != None:
                text += "%-20s      " % pitchers[0].name
            else:
                text += "%-25s " % ""

            if pitchers[1] != None:
                text += "%-20s\n" % pitchers[1].name
            else:
                text += "\n"
                
        text += "\n"
        return text

    def PrintPlays(self):
        gameiter = CWGameIterator(self.doc.game)
        text = ""

        lastHalf = 1
        
        if gameiter.game.first_comment != None:
            com = gameiter.game.first_comment
            
            while com != None:
                text += "Comment: %s\n" % com.text
                com = com.next

        while gameiter.event != None:
            team = gameiter.GetHalfInning()
            
            if team != lastHalf:
                text += "\n"
                text += GetInningLabel(gameiter.GetInning(),
                                       gameiter.GetHalfInning())
                text += "\n"
                lastHalf = team
            
            if gameiter.event.event_text == "NP":
                sub = gameiter.event.first_sub
                
                while sub != None:
                    if sub.team == team:
                        if sub.pos == 11:
                            x = "Pinch hitter: %s" % sub.name
                        else:
                            x = "Pinch runner: %s" % sub.name
                    else:
                        x = ("Defensive sub: %s, batting %d, playing %s" %
                             (sub.name, sub.slot,
                              [ "", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf", "dh" ][sub.pos]))
                    text += x + "\n"
                    sub = sub.next
            else:                
                batterId = gameiter.GetPlayer(team,
                                              gameiter.NumBatters(team) % 9 + 1)
                ros = self.doc.GetRoster(team)
                batter = ros.FindPlayer(batterId)
                
                x = "%-20s " % batter.GetName()
                x += "%2d-%2d " % (gameiter.GetTeamScore(0),
                                   gameiter.GetTeamScore(1))
                x += "("
                for base in [3, 2, 1]:
                    x += { True: "x", False: "-" }[gameiter.GetRunner(base) != ""]
                
                x += "%d): " % gameiter.outs
                x += gameiter.event.event_text + "\n"
                text += x

            if gameiter.event.first_comment != None:
                com = gameiter.event.first_comment

                while com != None:
                    text += "Comment: %s\n" % com.text
                    com = com.next
                    
            gameiter.NextEvent()

        return text

    def OnUpdate(self):   self.Rebuild()

    def Rebuild(self):
        self.textCtrl.Clear()
        x = self.PrintHeading()
        x += self.PrintStarters()
        x += self.PrintPlays()
        self.textCtrl.AppendText(x)
        
