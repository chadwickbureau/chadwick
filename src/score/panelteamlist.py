#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for displaying and editing teams
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

from libchadwick import *

from wxutils import FormattedStaticText

class NewTeamDialog(wxDialog):
    def __init__(self, parent, book):
        wxDialog.__init__(self, parent, -1, "Create new team")
        self.book = book

        sizer = wxFlexGridSizer(4)

        sizer.Add(FormattedStaticText(self, "City"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.city = wxTextCtrl(self, -1, "",
                               wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.city, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Nickname"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.nickname = wxTextCtrl(self, -1, "",
                                   wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.nickname, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team ID"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.teamID = wxTextCtrl(self, -1, "",
                                 wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.teamID, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "League"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.league = wxTextCtrl(self, -1, "",
                                 wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.league, 0, wxALL | wxALIGN_CENTER, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(wxID_OK).Enable(false)

        topSizer = wxBoxSizer(wxVERTICAL)

        topSizer.Add(sizer, 0, wxALL, 5)
        topSizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        EVT_TEXT(self, self.teamID.GetId(), self.OnTeamIDChange)

    def OnTeamIDChange(self, event):
        if str(self.teamID.GetValue()) == "":
            self.FindWindowById(wxID_OK).Enable(false)
        elif (str(self.teamID.GetValue()) in
              [ x.team_id for x in self.book.IterateTeams() ]):
            self.FindWindowById(wxID_OK).Enable(false)
        else:
            self.FindWindowById(wxID_OK).Enable(true)
              
    def GetCity(self):       return str(self.city.GetValue())
    def GetNickname(self):   return str(self.nickname.GetValue())
    def GetTeamID(self):     return str(self.teamID.GetValue())
    def GetLeague(self):     return str(self.league.GetValue())


class TeamListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 4)
        self.SetColLabelValue(0, "Team ID")
        self.SetColLabelValue(1, "City")
        self.SetColLabelValue(2, "Nickname")
        self.SetColLabelValue(3, "League ID")
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.EnableEditing(false)

    def OnUpdate(self, book):
        if self.GetNumberRows() > book.NumTeams():
            self.DeleteRows(self.GetNumberRows() - book.NumTeams())
        elif self.GetNumberRows() < book.NumTeams():
            self.InsertRows(0, book.NumTeams() - self.GetNumberRows())

        for (i,team) in enumerate(book.IterateTeams()):
            self.SetCellValue(i, 0, team.team_id)
            self.SetCellAlignment(i, 0, wxALIGN_CENTER, wxALIGN_CENTER)
            self.SetCellValue(i, 1, team.city)
            self.SetCellValue(i, 2, team.nickname)
            self.SetCellValue(i, 3, team.league)
            self.SetCellAlignment(i, 3, wxALIGN_CENTER, wxALIGN_CENTER)
            
        self.AutoSizeRows()
        self.AutoSizeColumns()
        self.AdjustScrollbars()

class TeamListPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        newTeamButton = wxButton(self, -1, "Add new team")
        newTeamButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.teamList = TeamListGrid(self)

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(newTeamButton, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(self.teamList, 1, wxEXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        EVT_BUTTON(self, newTeamButton.GetId(), self.OnNewTeam)

    def OnUpdate(self, book):
        self.book = book
        self.teamList.OnUpdate(book)

    def OnNewTeam(self, event):
        dialog = NewTeamDialog(self, self.book)

        if dialog.ShowModal() == wxID_OK:
            self.book.AddTeam(dialog.GetTeamID(),
                              dialog.GetCity(), dialog.GetNickname(),
                              dialog.GetLeague())
            self.OnUpdate(self.book)
