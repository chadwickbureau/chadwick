#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A dialog box for setting and editing a lineup
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

import dialogroster

# This class implements a wx.Choice in which one can select an item by
# typing the unique first N characters in the item string.
class KeySearchChoice(wx.Choice):
    def __init__(self, parent, windowID=wx.ID_ANY,
                 position=wx.DefaultPosition, size=wx.DefaultSize):
        wx.Choice.__init__(self, parent, windowID, position, size)
        self.keybuffer = ""

        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_CHOICE, self.OnSelection)
        
    def OnChar(self, event):
        if event.GetKeyCode() == wx.WXK_TAB:
            # Need to pass this up to the parent so tab traversal works
            event.Skip()
            return
        elif event.GetKeyCode() in [ wx.WXK_BACK, wx.WXK_DELETE ]:
            self.keybuffer = self.keybuffer[:-1]
        else:
            self.keybuffer = self.keybuffer + chr(event.GetKeyCode())

        for (i, label) in enumerate(self.GetStrings()):
            if label[:len(self.keybuffer)].upper() == self.keybuffer.upper():
                self.SetSelection(i)
                wx.PostEvent(self.GetParent(),
                             wx.CommandEvent(wx.wxEVT_COMMAND_CHOICE_SELECTED,
                                             self.GetId()))
                return

    def OnSelection(self, event):
        self.keybuffer = ""
        self.SetFocus()
        event.Skip()

class PositionChoice(wx.Choice):
    def __init__(self, parent, windowID=wx.ID_ANY,
                 position=wx.DefaultPosition, size=wx.DefaultSize):
        wx.Choice.__init__(self, parent, windowID, position, size)

        posList = [ "-", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf" ]
        for pos in posList:  self.Append(pos)

        self.Bind(wx.EVT_CHAR, self.OnChar)

    def OnChar(self, event):
        if event.GetKeyCode() == wx.WXK_TAB:
            # Need to pass this up to the parent so tab traversal works
            event.Skip()
            return
        elif chr(event.GetKeyCode()) >= '1' and \
           chr(event.GetKeyCode()) <= '9':
            self.SetSelection(event.GetKeyCode() - ord('1') + 1)
            wx.PostEvent(self.GetParent(),
                         wx.CommandEvent(wx.wxEVT_COMMAND_CHOICE_SELECTED,
                                         self.GetId()))
        elif chr(event.GetKeyCode()) in [ 'd', 'D' ] and \
                 "dh" in self.GetStrings():
            self.SetStringSelection("dh")
            wx.PostEvent(self.GetParent(),
                         wx.CommandEvent(wx.wxEVT_COMMAND_CHOICE_SELECTED,
                                         self.GetId()))
            

class LineupDialog(wx.Dialog):
    def __init__(self, parent, title):
        wx.Dialog.__init__(self, parent, title=title)

        sizer = wx.BoxSizer(wx.VERTICAL)

        gridSizer = wx.FlexGridSizer(10)

        self.players = [ KeySearchChoice(self, size=(300, -1))
                         for i in range(10) ]
        self.positions = [ PositionChoice(self, size=(50, -1))
                           for i in range(10) ]
        
        for i in range(0, 9):
            self.positions[i].MoveAfterInTabOrder(self.players[i])
            self.players[i+1].MoveAfterInTabOrder(self.positions[i])

        for i in range(10):
            if i < 9:
                gridSizer.Add(wx.StaticText(self, wx.ID_STATIC, "%d" % (i+1)),
                              0, wx.ALL | wx.ALIGN_CENTER, 5)
            else:
                self.pitcherText = wx.StaticText(self, wx.ID_STATIC, "P")
                gridSizer.Add(self.pitcherText,
                              0, wx.ALL | wx.ALIGN_CENTER, 5)
                
            self.players[i].SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
            gridSizer.Add(self.players[i], 0, wx.ALL | wx.ALIGN_CENTER, 5)
            wx.EVT_CHOICE(self, self.players[i].GetId(), self.OnSetEntry)
            self.positions[i].SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
            gridSizer.Add(self.positions[i], 0, wx.ALL | wx.ALIGN_CENTER, 5)
            wx.EVT_CHOICE(self, self.positions[i].GetId(), self.OnSetEntry)
        sizer.Add(gridSizer, 0, wx.ALL, 0)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        rosterButton = wx.Button(self, label="Roster...")
        self.Bind(wx.EVT_BUTTON, self.OnRoster, rosterButton)
        buttonSizer.Add(rosterButton, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(wx.ID_OK).Enable(False)
        sizer.Add(buttonSizer, 0, wx.ALL | wx.ALIGN_RIGHT, 5)
        
        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)

        self.players[-1].Enable(False)
        self.positions[-1].Show(False)

        self.players[0].SetFocus()
        
        wx.EVT_BUTTON(self, wx.ID_OK, self.OnOK)

    def OnOK(self, event):
        # Do some validation
        #for row in range(0, 9):
        #    if (self.lineup.GetCellValue(row, 0) == "" or
        #        self.lineup.GetCellValue(row, 1) == ""):
        #        # Eats the event
        #        return

        event.Skip()

    def OnRoster(self, event):
        self.plist = [ None ] + self.plist

        dialog = dialogroster.RosterDialog(self, self.book,
                                           self.roster.GetID())

        if dialog.ShowModal() == wx.ID_OK:
            # Preserve existing selections
            lineup = [ self.plist[x.GetSelection()] for x in self.players ]
            pos = [ x.GetSelection() for x in self.positions ]

            self.LoadRoster(self.book, self.roster, self.teamNumber,
                            self.useDH)

            # Get new list of players
            self.plist = [ x for x in self.roster.Players() ]
            self.plist.sort(lambda x, y: cmp(x.GetSortName(), y.GetSortName()))

            for (ctrl, player) in zip(self.players, lineup):
                if player is not None:
                    ctrl.SetSelection(self.plist.index(player) + 1)

            for (ctrl, pos) in zip(self.positions, pos):
                ctrl.SetSelection(pos)
        else:
            # Remove the None entry to restore things the way they were
            self.plist = self.plist[1:]

    def OnSetEntry(self, event):
        self.CheckValidity()

    def CheckValidity(self):
        """
        Check the status of the lineup after an entry (either a player or his
        position) is set.  Activate the OK button iff the lineup is valid.
        """
        self.pitcherText.Enable(self.HasDH())
        self.players[-1].Enable(self.HasDH())
        self.Layout()

        lineupOK = True

        for slot in range(9):
            if (self.players[slot].GetSelection() == 0 or
                self.positions[slot].GetSelection() == 0):
                lineupOK = False
                break

            if self.HasDH() and self.positions[slot] == 1:
                lineupOK = False
                break

        if self.HasDH():
            numSlots = 10
            if self.players[-1].GetSelection() == 0:
                lineupOK = False
        else:
            numSlots = 9
            
        for slot in range(numSlots):
            for slot2 in range(slot+1, numSlots):
                if (self.players[slot].GetSelection() == 
                    self.players[slot2].GetSelection() or
                    self.positions[slot].GetSelection() ==
                    self.positions[slot2].GetSelection()):
                    lineupOK = False
        
        self.FindWindowById(wx.ID_OK).Enable(lineupOK)

    def LoadRoster(self, book, roster, team, useDH):
        self.book = book
        self.roster = roster
        self.teamNumber = team
        self.useDH = useDH

        fgColors = [ wx.RED, wx.BLUE ]

        self.plist = [ x for x in self.roster.Players() ]
        self.plist.sort(lambda x, y: cmp(x.GetSortName(), y.GetSortName()))

        for ctrl in self.players:
            ctrl.Clear()
            ctrl.Append("")
            for player in self.plist:
                ctrl.Append(player.GetSortName())
            ctrl.SetForegroundColour(fgColors[team])
            ctrl.SetSelection(0)

        for ctrl in self.positions:
            if useDH:  ctrl.Append("dh")
            ctrl.SetForegroundColour(fgColors[team])
            ctrl.SetSelection(0)

    def LoadLineup(self, doc, team):
        gameiter = doc.GetState()
        self.origPlayers = []
        self.origPositions = []

        pitcher = gameiter.GetPlayer(team, 0)
        if pitcher != None:
            self.players[-1].Enable(True)

            for player in self.roster.Players():
                if player.player_id == pitcher:
                    self.players[-1].SetStringSelection(player.GetSortName())

        for slot in range(9):
            playerId = gameiter.GetPlayer(team, slot+1)
            for player in self.roster.Players():
                if player.player_id == playerId:
                    self.players[slot].SetStringSelection(player.GetSortName())
                    self.origPlayers.append(self.players[slot].GetSelection())
            if gameiter.GetPosition(team, playerId) <= 10:
                self.positions[slot].SetSelection(gameiter.GetPosition(team, playerId))
            self.origPositions.append(gameiter.GetPosition(team, playerId))
            
        if pitcher != None:
            self.origPlayers.append(self.players[-1].GetSelection())

    def GetPlayerInSlot(self, slot):
        return self.plist[self.players[slot-1].GetSelection()-1]

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
        return 10 in [ x.GetSelection() for x in self.positions ]

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

