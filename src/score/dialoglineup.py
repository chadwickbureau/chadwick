#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A dialog box for setting and editing a lineup
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
from libchadwick import *

def iterate_roster(roster):
    x = roster.first_player
    while x != None:
        yield x
        x = x.next

class LineupDialog(wxDialog):
    def __init__(self, parent, title):
        wxDialog.__init__(self, parent, -1, title)

        sizer = wxBoxSizer(wxVERTICAL)

        gridSizer = wxFlexGridSizer(10)

        self.players = [ wxChoice(self, -1, wxDefaultPosition,
                                  wxSize(300, -1)) for i in range(10) ]
        self.positions = [ wxChoice(self, -1,
                                    wxDefaultPosition, wxSize(50, -1)) 
                                    for i in range(10) ]
        

        for i in range(10):
            if i < 9:
                gridSizer.Add(wxStaticText(self, wxID_STATIC, "%d" % (i+1)),
                              0, wxALL | wxALIGN_CENTER, 5)
            else:
                self.pitcherText = wxStaticText(self, wxID_STATIC, "P")
                gridSizer.Add(self.pitcherText,
                              0, wxALL | wxALIGN_CENTER, 5)
                
            self.players[i].SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
            gridSizer.Add(self.players[i], 0, wxALL | wxALIGN_CENTER, 5)
            EVT_CHOICE(self, self.players[i].GetId(), self.OnSetEntry)
            self.positions[i].SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
            gridSizer.Add(self.positions[i], 0, wxALL | wxALIGN_CENTER, 5)
            EVT_CHOICE(self, self.positions[i].GetId(), self.OnSetEntry)

        self.pitcherText.Show(false)
        self.players[-1].Show(false)
        self.positions[-1].Show(false)

        sizer.Add(gridSizer, 0, wxALL, 0)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(wxID_OK).Enable(false)
        sizer.Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5)
        
        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)

        EVT_BUTTON(self, wxID_OK, self.OnOK)

    def OnOK(self, event):
        # Do some validation
        #for row in range(0, 9):
        #    if (self.lineup.GetCellValue(row, 0) == "" or
        #        self.lineup.GetCellValue(row, 1) == ""):
        #        # Eats the event
        #        return

        event.Skip()

    def OnSetEntry(self, event):
        self.CheckValidity()

    def CheckValidity(self):
        """
        Check the status of the lineup after an entry (either a player or his
        position) is set.  Activate the OK button iff the lineup is valid.
        """
        self.pitcherText.Show(self.HasDH())
        self.players[-1].Show(self.HasDH())

        lineupOK = true

        for slot in range(9):
            if (self.players[slot].GetSelection() == 0 or
                self.positions[slot].GetSelection() == 0):
                lineupOK = false
                break

            if self.HasDH() and self.positions[slot] == 1:
                lineupOK = false
                break

        if self.HasDH():
            numSlots = 10
            if self.players[-1].GetSelection() == 0:
                lineupOK = false
        else:
            numSlots = 9
            
        for slot in range(numSlots):
            for slot2 in range(slot+1, numSlots):
                if (self.players[slot].GetSelection() == 
                    self.players[slot2].GetSelection() or
                    self.positions[slot].GetSelection() ==
                    self.positions[slot2].GetSelection()):
                    lineupOK = false
        
        self.FindWindowById(wxID_OK).Enable(lineupOK)

    def LoadRoster(self, roster, team, useDH):
        self.roster = roster
        if useDH:
            posList = [ "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf", "dh" ]
        else:
            posList = [ "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf" ]

        fgColors = [ wxRED, wxBLUE ]

        for ctrl in self.players:
            ctrl.Clear()
            ctrl.Append("")
            for player in iterate_roster(self.roster):
                ctrl.Append(player.first_name + " " + player.last_name)
            ctrl.SetForegroundColour(fgColors[team])
            ctrl.SetSelection(0)

        for ctrl in self.positions:
            ctrl.Clear()
            ctrl.Append("-")
            for pos in posList:  ctrl.Append(pos)
            ctrl.SetForegroundColour(fgColors[team])
            ctrl.SetSelection(0)

    def LoadLineup(self, doc, team):
        gameiter = doc.GetState()
        self.origPlayers = []
        self.origPositions = []

        pitcher = gameiter.GetPlayer(team, 0)
        if pitcher != None:
            self.pitcherText.Show(true)
            self.players[-1].Show(true)

            for player in iterate_roster(self.roster):
                if player.player_id == pitcher:
                    self.players[-1].SetStringSelection(player.first_name + " " + player.last_name)

        for slot in range(9):
            playerId = gameiter.GetPlayer(team, slot+1)
            for player in iterate_roster(self.roster):
                if player.player_id == playerId:
                    self.players[slot].SetStringSelection(player.first_name + " " + player.last_name)
                    self.origPlayers.append(self.players[slot].GetSelection())
            if cw_gameiter_player_position(gameiter, team, playerId) <= 10:
                self.positions[slot].SetSelection(cw_gameiter_player_position(gameiter, team, playerId))
            self.origPositions.append(cw_gameiter_player_position(gameiter, team, playerId))
            
        if pitcher != None:
            self.origPlayers.append(self.players[-1].GetSelection())

    def GetPlayerInSlot(self, slot):
        return [x for x in iterate_roster(self.roster)][self.players[slot-1].GetSelection()-1]

    def SetPlayerInSlot(self, slot, name, pos):
        if slot > 0:
            self.players[slot-1].SetStringSelection(name)
            self.positions[slot-1].SetSelection(pos)
        else:
            self.players[-1].SetStringSelection(name)
        self.CheckValidity()

    def GetPositionInSlot(self, slot):
        return self.positions[slot-1].GetSelection()

    def HasDH(self):
        for slot in range(9):
            if self.positions[slot].GetSelection() == 10:
                return true
        return false

    def WriteChanges(self, doc, team):
        for slot in range(9):
            if ((self.players[slot].GetSelection() !=
                 self.origPlayers[slot]) or
                (self.positions[slot].GetSelection() !=
                 self.origPositions[slot])):
                player = self.GetPlayerInSlot(slot+1)
                doc.AddSubstitute(player, team, slot+1,
                                  self.positions[slot].GetSelection())

        if self.HasDH():
            if self.players[-1].GetSelection() != self.origPlayers[-1]:
                player = self.GetPlayerInSlot(10)
                doc.AddSubstitute(player, team, 0, 1)

class PinchDialog(wxDialog):
    def __init__(self, parent, title):
        wxDialog.__init__(self, parent, -1, title)

        sizer = wxBoxSizer(wxVERTICAL)

        self.player = wxChoice(self, -1, wxDefaultPosition,
                               wxSize(300, -1))
        self.player.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        sizer.Add(self.player, 0, wxALL | wxALIGN_CENTER, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(wxID_OK).Enable(false)
        sizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)
        
        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)
        
        EVT_CHOICE(self, self.player.GetId(), self.OnSetPlayer)

    def OnSetPlayer(self, event):
        if self.player.GetSelection() >= 0:
            self.FindWindowById(wxID_OK).Enable(true)
            
    def LoadRoster(self, roster, team):
        self.roster = roster

        self.player.Clear()
        for player in iterate_roster(self.roster):
            self.player.Append(player.first_name + " " + player.last_name)

        if team == 0:
            self.player.SetForegroundColour(wxRED)
        else:
            self.player.SetForegroundColour(wxBLUE)
        
    def WriteChanges(self, doc, oldPlayer, team, pos):
        player = self.GetPlayer()
        gameiter = doc.gameiter
        slot = cw_gameiter_lineup_slot(gameiter, team, oldPlayer)
        doc.AddSubstitute(player, team, slot, pos)

    def GetPlayer(self):
        return [x for x in iterate_roster(self.roster)][self.player.GetSelection()]

