#
# $Source$
# $Date: 2007-05-28 23:18:12 -0500 (Mon, 28 May 2007) $
# $Revision: 280 $
#
# DESCRIPTION:
# Dialog for editing team roster
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

import wx, wx.grid


def SideToString(side):
    """
    Convert the internal codes for batting/throwing side to words.
    """
    try:
        return { "R":   "Right",
                 "L":   "Left",
                 "B":   "Both",
                 "S":   "Both",
                 "?":   "Unknown" }[side]
    except KeyError:
        return "Unknown"

def StringToSide(side):
    """
    Convert words to internal codes for batting/throwing side.
    """
    try:
        return { "Right":   "R",
                 "Left":    "L",
                 "Both":    "B",
                 "Unknown": "?" }[side]
    except KeyError:
        return "?"

class PlayerListCtrl(wx.grid.Grid):
    def __init__(self, parent, book, team):
        wx.grid.Grid.__init__(self, parent, size=(500, 400))
        self.book = book
        self.team = team

        self.sideEditor = wx.grid.GridCellChoiceEditor([ "Right", "Left",
                                                         "Both", "Unknown" ])

        self.CreateGrid(self.team.NumPlayers(), 5)
        self.EnableEditing(True)
        self.SetRowLabelSize(20)

        for (col, label) in enumerate([ "Player ID", "First", "Last",
                                        "B", "T" ]):
            self.SetColLabelValue(col, label)

        for (col, width) in enumerate([ 75, 100, 100, 100, 100 ]):
            self.SetColSize(col, width)

        plist = [ x for x in self.team.Players() ]
        plist.sort(lambda x, y: cmp(x.GetSortName(), y.GetSortName()))

        for (row, player) in enumerate(plist):
            # Existing player IDs are read-only for now
            self.SetReadOnly(row, 0, True)
            for (col, value) in enumerate([ player.GetID(),
                                            player.GetFirstName(),
                                            player.GetLastName(),
                                            SideToString(player.GetBats()),
                                            SideToString(player.GetThrows()) ]):
                self.SetCellValue(row, col, value)

            for col in [ 3, 4 ]:
                self.SetCellEditor(row, col, self.sideEditor)

        if self.team.NumPlayers() == 0:
            self.AddBlankRow()
            
        self.UpdateFeedback()
        
        self.Bind(wx.EVT_KEY_DOWN, self.OnGridWindowKey, self.GetGridWindow())
        self.Bind(wx.grid.EVT_GRID_CELL_CHANGE, self.OnGridCellChange)

        
    def OnGridWindowKey(self, event):
        """
        This preprocesses some keystrokes targeted for the grid window,
        to produce custom behaviors (e.g., creating a new blank row when
        pressing the down arrow on the last row.)
        """
        if event.GetKeyCode() == wx.WXK_DOWN and \
           self.GetGridCursorRow() == self.GetNumberRows() - 1 and \
           not self.IsRowBlank(self.GetGridCursorRow()):
            self.AddBlankRow()
            self.UpdateFeedback()
        elif event.GetKeyCode() == wx.WXK_UP and \
             self.GetGridCursorRow() > 0 and \
             self.GetGridCursorRow() == self.GetNumberRows() - 1 and \
             self.IsRowBlank(self.GetGridCursorRow()):
            self.DeleteRows(self.GetGridCursorRow());
            self.SetGridCursor(self.GetNumberRows() - 1,
                               self.GetGridCursorCol())
            self.UpdateFeedback()
        elif self.GetGridCursorCol() in [ 3, 4 ]:
            # Keyboard shortcuts for setting batting/throwing side
            # Note that we want to eat these events, so as to
            # short-circuit the normal processing -- the whole point
            # is to avoid bringing up the choice editor when possible.
            if chr(event.GetKeyCode()) in [ "r", "R" ]:
                self.SetCellValue(self.GetGridCursorRow(),
                                  self.GetGridCursorCol(), "Right")
                return
            elif chr(event.GetKeyCode()) in [ "l", "L" ]:
                self.SetCellValue(self.GetGridCursorRow(),
                                  self.GetGridCursorCol(), "Left")
                return
            elif chr(event.GetKeyCode()) in [ "b", "B", "s", "S" ]:
                self.SetCellValue(self.GetGridCursorRow(),
                                  self.GetGridCursorCol(), "Both")
                return
            elif chr(event.GetKeyCode()) in [ "u", "U", "?" ]:
                self.SetCellValue(self.GetGridCursorRow(),
                                  self.GetGridCursorCol(), "Unknown")
                return
            
        event.Skip()

    def OnGridCellChange(self, event):
        """
        Called when the contents of a cell are changed.
        Currently: Update the visual feedback
        """
        self.UpdateFeedback()

    def UpdateFeedback(self):
        """
        Implements rich feedback for validity checking.
        Currently: All player IDs must be unique and nonempty.
        """
        for row in xrange(self.GetNumberRows()):
            if self.GetCellValue(row, 0).strip() == "":
                self.SetCellBackgroundColour(row, 0,
                                             wx.NamedColour("pink"))
            else:
                for row2 in xrange(self.GetNumberRows()):
                    if row != row2 and \
                       self.GetCellValue(row, 0) == self.GetCellValue(row2, 0):
                        self.SetCellBackgroundColour(row, 0,
                                                     wx.NamedColour("pink"))
                        break
                else:
                    self.SetCellBackgroundColour(row, 0, wx.WHITE)

    def IsRowBlank(self, row):
        """
        Determines if an entire row is blank (i.e., contains nothing
        but whitespace.
        """
        for col in xrange(self.GetNumberCols()):
            if self.GetCellValue(row, col).strip() != "":
                return False
        return True


    def AddBlankRow(self):
        """
        Adds a blank row at the end of the grid.
        Makes sure all editors/attributes are set correctly.
        """
        row = self.GetNumberRows()
        self.AppendRows()
        teamPlayers = [ str(self.GetCellValue(row, 0))
                        for row in xrange(self.GetNumberRows()) ]
        playerID = self.book.UniquePlayerID()
        while playerID in teamPlayers:
            playerID = self.book.UniquePlayerID()

        self.SetCellValue(row, 0, playerID)
        self.SetReadOnly(row, 0, True)
        for col in [ 3, 4 ]:
            self.SetCellValue(row, col, "Unknown")
            self.SetCellEditor(row, col, self.sideEditor)

        

        

class RosterDialog(wx.Dialog):
    def __init__(self, parent, book, teamID):
        wx.Dialog.__init__(self, parent)
        self.book = book
        self.team = self.book.GetTeam(teamID)

        self.playerList = PlayerListCtrl(self, book, self.team)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.playerList, 1, wx.EXPAND, 0)

        buttonSizer = wx.StdDialogButtonSizer()
        okButton = wx.Button(self, wx.ID_OK, "OK")
        buttonSizer.Add(okButton)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"))
        buttonSizer.Realize()

        sizer.Add(buttonSizer, 0, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)

        self.Bind(wx.EVT_BUTTON, self.OnOK, okButton)

    def OnOK(self, event):
        for row in xrange(self.playerList.GetNumberRows()):
            self.book.SetPlayer(playerID=str(self.playerList.GetCellValue(row, 0)),
                                firstName=str(self.playerList.GetCellValue(row, 1)),
                                lastName=str(self.playerList.GetCellValue(row, 2)),
                                bats=StringToSide(str(self.playerList.GetCellValue(row, 3))),
                                throws=StringToSide(str(self.playerList.GetCellValue(row, 4))))
            self.book.SetPlayerTeam(playerID=str(self.playerList.GetCellValue(row, 0)),
                                    teamID=self.team.GetID())
            
        event.Skip()

        
