#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for viewing and editing players
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
import string

from wxutils import FormattedStaticText

class NameValidator(wxPyValidator):
    """
    This validates a text control as a valid name.  We permit
    letters, whitespace, and apostrophes.
    """
    def __init__(self, pyVar=None):
        wxPyValidator.__init__(self)
        EVT_CHAR(self, self.OnChar)

    def TransferToWindow(self):     return True
    def TransferFromWindow(self):   return True 
    def Clone(self):                return NameValidator()

    def Validate(self, win):
        textCtrl = self.GetWindow()
        val = str(textCtrl.GetValue())
        
        for x in val:
            if x not in string.letters and x not in [ " ", "'" ]:
                return false

        return true

    def OnChar(self, event):
        key = event.KeyCode()

        if key < wx.WXK_SPACE or key == wx.WXK_DELETE or key > 255:
            event.Skip()
            return

        if chr(key) in string.letters or chr(key) in [ " ", "'" ]:
            event.Skip()
            return

        if not wx.Validator_IsSilent():
            wxBell()

        # Returning without calling even.Skip eats the event before it
        # gets to the text control
        return


class NewPlayerDialog(wxDialog):
    def __init__(self, parent, book):
        wxDialog.__init__(self, parent, -1, "Create new player")
        self.book = book

        sizer = wxFlexGridSizer(6)

        sizer.Add(FormattedStaticText(self, "First name"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.firstName = wxTextCtrl(self, -1, "",
                                    wxDefaultPosition, wxSize(150, -1))
        self.firstName.SetValidator(NameValidator())
        sizer.Add(self.firstName, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Last name"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.lastName = wxTextCtrl(self, -1, "",
                                   wxDefaultPosition, wxSize(150, -1))
        self.lastName.SetValidator(NameValidator())
        sizer.Add(self.lastName, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Bats"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.bats = wxChoice(self, -1, wxDefaultPosition, wxSize(150, -1),
                             [ "Unknown", "Right", "Left", "Both" ])
        self.bats.SetSelection(1)
        sizer.Add(self.bats, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Throws"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.throws = wxChoice(self, -1, wxDefaultPosition, wxSize(150, -1),
                               [ "Unknown", "Right", "Left" ])
        self.throws.SetSelection(1)
        sizer.Add(self.throws, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team"),
                  0, wxALL | wxALIGN_CENTER, 5)
        teamList = [ team.city + " " + team.nickname
                     for team in book.IterateTeams() ]
        self.team = wxChoice(self, -1, wxDefaultPosition, wxSize(150, -1),
                             teamList)
        self.team.SetSelection(0)
        sizer.Add(self.team, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Player ID"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.playerID = wxTextCtrl(self, -1, "", wxDefaultPosition,
                                   wxSize(150, -1))
        sizer.Add(self.playerID, 0, wxALL | wxALIGN_CENTER, 5)

        generateButton = wxButton(self, -1, "Generate unique ID")
        generateButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(wxID_OK).Enable(false)

        topSizer = wxBoxSizer(wxVERTICAL)

        topSizer.Add(sizer, 0, wxALL, 5)
        topSizer.Add(generateButton, 0, wxALL | wxALIGN_CENTER, 5)
        topSizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        EVT_TEXT(self, self.playerID.GetId(), self.OnIDChange)
        EVT_BUTTON(self, generateButton.GetId(), self.OnGenerateID)

    def OnGenerateID(self, event):
        self.playerID.SetValue(self.book.UniquePlayerID(self.GetFirstName(),
                                                        self.GetLastName()))
        self.FindWindowById(wxID_OK).Enable(true)

    def OnIDChange(self, event):
        playerID = self.GetPlayerID()
        if (playerID == "" or
            playerID in [ p.player_id for p in self.book.IteratePlayers() ]):
            self.FindWindowById(wxID_OK).Enable(false)
        else:
            self.FindWindowById(wxID_OK).Enable(true)
        
    def GetPlayerID(self):   return str(self.playerID.GetValue()).strip()
    def GetFirstName(self):  return str(self.firstName.GetValue()).strip()
    def GetLastName(self):   return str(self.lastName.GetValue()).strip()
    def GetBats(self):
        return [ "?", "R", "L", "B" ][self.bats.GetSelection()]
    def GetThrows(self):
        return [ "?", "R", "L" ][self.throws.GetSelection()]
    def GetTeam(self):
        return [ t for t in self.book.IterateTeams() ][self.team.GetSelection()].team_id
    

class PlayerListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 5)
        self.SetColLabelValue(0, "Player ID")
        self.SetColLabelValue(1, "First")
        self.SetColLabelValue(2, "Last")
        self.SetColLabelValue(3, "Bats")
        self.SetColLabelValue(4, "Throws")
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.EnableEditing(false)

    def OnUpdate(self, book):
        if self.GetNumberRows() > book.NumPlayers():
            self.DeleteRows(0, self.GetNumberRows() - book.NumPlayers())
        elif self.GetNumberRows() < book.NumPlayers():
            self.InsertRows(0, book.NumPlayers() - self.GetNumberRows())

        for (i,player) in enumerate(book.IteratePlayers()):
            self.SetCellValue(i, 0, player.player_id)
            self.SetCellValue(i, 1, player.first_name)
            self.SetCellValue(i, 2, player.last_name)
            self.SetCellValue(i, 3, player.bats)
            self.SetCellValue(i, 4, player.throws)
            
        self.AutoSizeRows()
        self.AutoSizeColumns()
        self.AdjustScrollbars()

class PlayerListPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        newPlayerButton = wxButton(self, -1, "Create new player")
        newPlayerButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.playerList = PlayerListGrid(self)

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(newPlayerButton, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(self.playerList, 1, wxEXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        EVT_BUTTON(self, newPlayerButton.GetId(), self.OnNewPlayer)

    def OnNewPlayer(self, event):
        dialog = NewPlayerDialog(self, self.book)

        if dialog.ShowModal() == wxID_OK:
            self.book.AddPlayer(dialog.GetPlayerID(),
                                dialog.GetFirstName(),
                                dialog.GetLastName(),
                                dialog.GetBats(),
                                dialog.GetThrows(),
                                dialog.GetTeam())
            self.GetGrandParent().OnUpdate()

    def OnUpdate(self, book):
        self.book = book
        self.playerList.OnUpdate(book)
