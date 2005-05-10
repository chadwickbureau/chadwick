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

from wxPython.wx import *
from wxPython.grid import *
from wxPython.calendar import *
from libchadwick import *

def iterate_teams(league):
    x = league.first_roster
    while x != None:
        yield x
        x = x.next
    raise StopIteration

class NewGameDialog(wxDialog):
    def __init__(self, parent, f):
        wxDialog.__init__(self, parent, -1, "Select teams")

        sizer = wxBoxSizer(wxVERTICAL)

        gridSizer = wxFlexGridSizer(2)
        gridSizer.AddGrowableCol(1)

        self.teams = [ wxChoice(self, -1,
                                wxDefaultPosition, wxDefaultSize,
                                [ ] )
                       for t in [0, 1] ]
        for ctrl in self.teams:
            ctrl.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
            
        # Control to select visiting team
        gridSizer.Add(wxStaticText(self, wxID_STATIC, "Visiting team"),
                      0, wxALL | wxALIGN_CENTER, 5)
        gridSizer.Add(self.teams[0], 0, wxALL | wxEXPAND, 5)

        # Control to select home team
        gridSizer.Add(wxStaticText(self, wxID_STATIC, "Home team"),
                      0, wxALL | wxALIGN_CENTER, 5)
        gridSizer.Add(self.teams[1], 0, wxALL | wxEXPAND, 5)
        sizer.Add(gridSizer, 0, wxALL | wxEXPAND, 5)

        box = wxStaticBox(self, wxID_STATIC, "Game date")
        boxSizer = wxStaticBoxSizer(box, wxVERTICAL)

        now = wxDateTime.Now()
        now.SetYear(f.GetYear())

        self.gameDate = wxCalendarCtrl(self, -1, now,
                                       wxDefaultPosition,
                                       wxSize(300, -1))
        boxSizer.Add(self.gameDate, 1, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(boxSizer, 1, wxALL | wxEXPAND | wxALIGN_CENTER, 5)
        
        self.gameNumber = wxRadioBox(self, -1, "Game number",
                                     wxDefaultPosition, wxDefaultSize,
                                     [ "Only game", "First game", "Second game" ])
        sizer.Add(self.gameNumber, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        #self.pitches = wxRadioBox(self, -1, "Enter pitches?",
        #                          wxDefaultPosition, wxDefaultSize,
        #                          [ "No pitches", "Count only", "All pitches" ])
        #self.pitches.SetSelection(0)
        #sizer.Add(self.pitches, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)
        
        self.UpdateTeamLists(f)
                                     
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                        0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)
        
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

        self.teams[0].SetForegroundColour(wxRED)
        self.teams[1].SetForegroundColour(wxBLUE)

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
    
