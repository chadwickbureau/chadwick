#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A dialog shown when creating a new game
# 
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2005, Ted Turocy (turocy@econ.tamu.edu)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

import wx, wx.grid, wx.calendar
from libchadwick import *

def iterate_teams(league):
    x = league.first_roster
    while x != None:
        yield x
        x = x.next
    raise StopIteration

class NewGameDialog(wx.Dialog):
    def __init__(self, parent, f):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Select teams")

        sizer = wx.BoxSizer(wx.VERTICAL)

        gridSizer = wx.FlexGridSizer(2)
        gridSizer.AddGrowableCol(1)

        self.teams = [ wx.Choice(self, wx.ID_ANY,
                                 wx.DefaultPosition, wx.DefaultSize,
                                 [ ] )
                       for t in [0, 1] ]
        for ctrl in self.teams:
            ctrl.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
            
        # Control to select visiting team
        gridSizer.Add(wx.StaticText(self, wx.ID_STATIC, "Visiting team"),
                      0, wx.ALL | wx.ALIGN_CENTER, 5)
        gridSizer.Add(self.teams[0], 0, wx.ALL | wx.EXPAND, 5)

        # Control to select home team
        gridSizer.Add(wx.StaticText(self, wx.ID_STATIC, "Home team"),
                      0, wx.ALL | wx.ALIGN_CENTER, 5)
        gridSizer.Add(self.teams[1], 0, wx.ALL | wx.EXPAND, 5)
        sizer.Add(gridSizer, 0, wx.ALL | wx.EXPAND, 5)

        box = wx.StaticBox(self, wx.ID_STATIC, "Game date")
        boxSizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        now = wx.DateTime.Now()
        now.SetYear(f.GetYear())

        self.gameDate = wx.calendar.CalendarCtrl(self, wx.ID_ANY,
                                                 now, size=(300, -1))
        boxSizer.Add(self.gameDate, 1, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(boxSizer, 1, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        
        self.gameNumber = wx.RadioBox(self, -1, "Game number",
                                      wx.DefaultPosition, wx.DefaultSize,
                                      [ "Only game", "First game",
                                        "Second game" ])
        sizer.Add(self.gameNumber, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        #self.pitches = wx.RadioBox(self, -1, "Enter pitches?",
        #                          wx.DefaultPosition, wx.DefaultSize,
        #                          [ "No pitches", "Count only", "All pitches" ])
        #self.pitches.SetSelection(0)
        #sizer.Add(self.pitches, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        
        self.UpdateTeamLists(f)
                                     
        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)
        
        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)

    def UpdateTeamLists(self, f):
        self.teams[0].Clear()
        self.teams[1].Clear()
        self.teamIDs = [ ]

        for t in f.Teams():
            t.year = f.GetYear()
            self.teams[0].Append(t.GetName())
            self.teams[1].Append(t.GetName())
            self.teamIDs.append(t.GetID())

        self.teams[0].SetSelection(0)
        self.teams[1].SetSelection(0)

        self.teams[0].SetForegroundColour(wx.RED)
        self.teams[1].SetForegroundColour(wx.BLUE)

        gamedate = self.gameDate.GetDate()
        gamedate.SetYear(f.GetYear())
        self.gameDate.SetDate(gamedate)

    def GetTeam(self, team):
        return self.teamIDs[self.teams[team].GetSelection()]

    def GetGameDate(self):
        return self.gameDate.GetDate()
        
    def GetGameId(self):
        return "%s%04d%02d%02d%d" % (self.GetTeam(1),
                                     self.gameDate.GetDate().GetYear(),
                                     self.gameDate.GetDate().GetMonth()+1,
                                     self.gameDate.GetDate().GetDay(),
                                     self.GetGameNumber())
        
    def GetGameNumber(self):
        return self.gameNumber.GetSelection()

    def GetPitches(self):
        #return [ "none", "count", "pitches" ][self.pitches.GetSelection()]
        return "none"
    
