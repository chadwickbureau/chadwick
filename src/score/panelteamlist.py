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

class AddTeamDialog(wxDialog):
    def __init__(self, parent, book):
        wxDialog.__init__(self, parent, -1, "Add team")
        self.book = book

        self.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))

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
        # A blank team ID is invalid, so flag this as invalid
        self.teamID.SetBackgroundColour(wxNamedColour("pink"))
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
            self.teamID.SetBackgroundColour(wxNamedColour("pink"))
        elif (str(self.teamID.GetValue()) in
              [ x.GetID() for x in self.book.Teams() ]):
            self.FindWindowById(wxID_OK).Enable(false)
            self.teamID.SetBackgroundColour(wxNamedColour("pink"))
        else:
            self.FindWindowById(wxID_OK).Enable(true)
            self.teamID.SetBackgroundColour(wxSystemSettings_GetColour(wxSYS_COLOUR_WINDOW))
              
    def GetCity(self):       return str(self.city.GetValue())
    def GetNickname(self):   return str(self.nickname.GetValue())
    def GetTeamID(self):     return str(self.teamID.GetValue())
    def GetLeague(self):     return str(self.league.GetValue())


class EditTeamDialog(wxDialog):
    def __init__(self, parent, team):
        wxDialog.__init__(self, parent, -1, "Edit team")

        self.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))

        sizer = wxFlexGridSizer(4)

        sizer.Add(FormattedStaticText(self, "City"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.city = wxTextCtrl(self, -1, team.GetCity(),
                               wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.city, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Nickname"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.nickname = wxTextCtrl(self, -1, team.GetNickname(),
                                   wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.nickname, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team ID"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.teamID = wxTextCtrl(self, -1, team.GetID(),
                                 wxDefaultPosition, wxSize(150, -1))
        self.teamID.Enable(False)
        sizer.Add(self.teamID, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "League"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.league = wxTextCtrl(self, -1, team.GetLeague(),
                                 wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.league, 0, wxALL | wxALIGN_CENTER, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)

        topSizer = wxBoxSizer(wxVERTICAL)

        topSizer.Add(sizer, 0, wxALL, 5)
        topSizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

    def GetCity(self):       return str(self.city.GetValue())
    def GetNickname(self):   return str(self.nickname.GetValue())
    def GetLeague(self):     return str(self.league.GetValue())


class TeamListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 4)
        self.SetColLabelValue(0, "City")
        self.SetColLabelValue(1, "Nickname")
        self.SetColLabelValue(2, "Team ID")
        self.SetColLabelValue(3, "League")
        self.SetRowLabelSize(1)
        self.SetLabelFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))
        self.SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.SetDefaultCellBackgroundColour(wxColour(242, 242, 242))
        self.DisableDragRowSize()
        
        self.SetColSize(0, 150)
        self.SetColSize(1, 150)
        self.SetColSize(2, 100)
        self.SetColSize(3, 100)

        self.EnableEditing(false)

        EVT_GRID_CELL_LEFT_DCLICK(self, self.OnLeftDoubleClick)

    def OnUpdate(self, book):
        self.book = book
        
        if self.GetNumberRows() > book.NumTeams():
            self.DeleteRows(0, self.GetNumberRows() - book.NumTeams())
        elif self.GetNumberRows() < book.NumTeams():
            self.InsertRows(0, book.NumTeams() - self.GetNumberRows())

        for (i,team) in enumerate(book.Teams()):
            self.SetCellValue(i, 0, team.GetCity())
            self.SetCellAlignment(i, 0, wxALIGN_LEFT, wxALIGN_CENTER)
            self.SetCellValue(i, 1, team.GetNickname())
            self.SetCellAlignment(i, 1, wxALIGN_LEFT, wxALIGN_CENTER)
            self.SetCellValue(i, 2, team.GetID())
            self.SetCellAlignment(i, 2, wxALIGN_CENTER, wxALIGN_CENTER)
            self.SetCellValue(i, 3, team.league)
            self.SetCellAlignment(i, 3, wxALIGN_CENTER, wxALIGN_CENTER)
            
        if self.GetNumberRows() > 0:
            self.AutoSizeRow(0)
            self.SetDefaultRowSize(self.GetRowSize(0))
        self.AdjustScrollbars()

    def OnLeftDoubleClick(self, event):
        for (i,team) in enumerate(self.book.Teams()):
            if i == event.GetRow():
                dialog = EditTeamDialog(self, team)
                if dialog.ShowModal() == wxID_OK:
                    self.book.ModifyTeam(team.GetID(),
                                         dialog.GetCity(),
                                         dialog.GetNickname(),
                                         dialog.GetLeague())
                    self.GetParent().GetGrandParent().OnUpdate()
                return

class TeamListPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        newTeamButton = wxButton(self, -1, "Add team")
        newTeamButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.teamList = TeamListGrid(self)

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(newTeamButton, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(self.teamList, 1, wxEXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        EVT_BUTTON(self, newTeamButton.GetId(), self.OnAddTeam)

    def OnUpdate(self, book):
        self.book = book
        self.teamList.OnUpdate(book)

    def OnAddTeam(self, event):
        dialog = AddTeamDialog(self, self.book)

        if dialog.ShowModal() == wxID_OK:
            self.book.AddTeam(dialog.GetTeamID(),
                              dialog.GetCity(), dialog.GetNickname(),
                              dialog.GetLeague())
            self.GetGrandParent().OnUpdate()
