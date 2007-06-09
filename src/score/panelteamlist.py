#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for displaying and editing teams
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

from wxutils import FormattedStaticText

class EditTeamDialog(wx.Dialog):
    def __init__(self, parent, book, team):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Edit team")

        self.book = book
        self.team = team
        
        self.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        sizer = wx.FlexGridSizer(2)

        sizer.Add(FormattedStaticText(self, "City"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.city = wx.TextCtrl(self, wx.ID_ANY,
                                team.GetCity(), size=(150, -1))
        sizer.Add(self.city, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Nickname"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.nickname = wx.TextCtrl(self, wx.ID_ANY, team.GetNickname(),
                                    size=(150, -1))
        sizer.Add(self.nickname, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team ID"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.teamID = wx.TextCtrl(self, wx.ID_ANY, team.GetID(), size=(150,-1))
        self.teamID.Enable(False)
        sizer.Add(self.teamID, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "League"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.league = wx.TextCtrl(self, wx.ID_ANY, team.GetLeague(),
                                  size = (150, -1))
        sizer.Add(self.league, 0, wx.ALL | wx.ALIGN_CENTER, 5)


        playerBox = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_STATIC,
                                                   "Players on team"),
                                      wx.HORIZONTAL)
        self.players = wx.ListView(self, wx.ID_ANY, size=(250, 150))
        self.players.InsertColumn(0, "Name (ID)", width=230)
        self.playerIDs = [ x.GetID() for x in team.Players() ]
        for (pl,player) in enumerate(team.Players()):
            self.players.InsertStringItem(pl,
                                          "%s (%s)" % (player.GetSortName(),
                                                       player.GetID()))
            self.players.SetItemTextColour(pl, wx.BLUE)
            
        playerBox.Add(self.players, 0, wx.ALL, 5)
        
        allPlayerBox = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_STATIC,
                                                      "All players"),
                                         wx.HORIZONTAL)
        self.allPlayers = wx.ListView(self, wx.ID_ANY, size=(250, 150),
                                      style=wx.LC_REPORT)
        self.allPlayers.InsertColumn(0, "Name (ID)", width=230)
        self.allPlayerIDs = [ x.GetID() for x in book.Players() ]
        for (pl,player) in enumerate(book.Players()):
            self.allPlayers.InsertStringItem(pl,
                                             "%s (%s)" % (player.GetSortName(),
                                                          player.GetID()))
            if team.FindPlayer(player.GetID()) != None:
                self.allPlayers.SetItemTextColour(pl, wx.BLUE)
            else:
                self.allPlayers.SetItemTextColour(pl, wx.BLACK)

        wx.EVT_LIST_ITEM_ACTIVATED(self, self.allPlayers.GetId(),
                                   self.OnPlayerActivated)
            
        allPlayerBox.Add(self.allPlayers, 0, wx.ALL, 5)


        playerSizer = wx.BoxSizer(wx.HORIZONTAL)
        playerSizer.Add(playerBox, 0, wx.ALL, 5)
        playerSizer.Add(allPlayerBox, 0, wx.ALL, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)

        topSizer = wx.BoxSizer(wx.VERTICAL)

        topSizer.Add(sizer, 0, wx.ALL, 5)
        topSizer.Add(playerSizer, 0, wx.ALL, 5)
        topSizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

    def GetCity(self):       return str(self.city.GetValue())
    def GetNickname(self):   return str(self.nickname.GetValue())
    def GetLeague(self):     return str(self.league.GetValue())

    def OnPlayerActivated(self, event):
        playerID = self.allPlayerIDs[event.GetIndex()]
        if self.team.FindPlayer(playerID) == None:
            player = self.book.GetPlayer(playerID)
            entry = "%s (%s)" % (player.GetSortName(), player.GetID())
            self.playerIDs.append(playerID)
            self.playerIDs.sort(lambda x, y:
                                cmp(self.book.GetPlayer(x).GetSortName().upper(),
                                    self.book.GetPlayer(y).GetSortName().upper()))
            self.players.InsertStringItem(self.playerIDs.index(playerID), entry)
            self.players.SetItemTextColour(self.playerIDs.index(playerID),
                                           wx.Colour(0, 192, 0))
            self.players.EnsureVisible(self.playerIDs.index(playerID))
            self.allPlayers.SetItemTextColour(event.GetIndex(), wx.BLUE)

    def UpdateTeam(self):
        for p in self.playerIDs:
            if self.team.FindPlayer(p) == None:
                player = self.book.GetPlayer(p)
                self.book.AddToTeam(p,
                                    player.GetFirstName(), player.GetLastName(),
                                    player.GetBats(), player.GetThrows(),
                                    self.team.GetID())

class TeamListCtrl(wx.grid.Grid):
    def __init__(self, parent, book):
        wx.grid.Grid.__init__(self, parent, size=(500, 400))
        self.book = book

        self.CreateGrid(max(book.NumTeams(), 1), 4)
        self.EnableEditing(True)
        self.SetRowLabelSize(0)
        
        for (col, label) in enumerate([ "Team ID", "City", "Nickname",
                                        "League" ]):
            self.SetColLabelValue(col, label)

        for (col, width) in enumerate([ 100, 150, 150, 100 ]):
            self.SetColSize(col, width)

        for (row, team) in enumerate(book.Teams()):
            # Existing team IDs are read-only for now
            self.SetReadOnly(row, 0, True)
            for (col, method) in enumerate([ team.GetID, team.GetCity,
                                             team.GetNickname,
                                             team.GetLeague ]):
                self.SetCellValue(row, col, method())

        self.UpdateFeedback()
        
        self.Bind(wx.EVT_KEY_DOWN, self.OnGridWindowKey, self.GetGridWindow())
        self.Bind(wx.grid.EVT_GRID_SELECT_CELL, self.OnSelectCell)
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
            self.AppendRows()
            self.UpdateFeedback()
        elif event.GetKeyCode() == wx.WXK_UP and \
             self.GetGridCursorRow() > 0 and \
             self.GetGridCursorRow() == self.GetNumberRows() - 1 and \
             self.IsRowBlank(self.GetGridCursorRow()):
            self.DeleteRows(self.GetGridCursorRow());
            self.SetGridCursor(self.GetNumberRows() - 1,
                               self.GetGridCursorCol())
            self.UpdateFeedback()
            
        event.Skip()

    def OnSelectCell(self, event):
        if event.GetRow() < self.GetNumberRows() - 1 and \
           self.IsRowBlank(self.GetNumberRows() - 1):
            self.DeleteRows(self.GetNumberRows() - 1)
            self.UpdateFeedback()

        event.Skip()

    def OnGridCellChange(self, event):
        """
        Called when the contents of a cell are changed.
        Currently, just need to update the feedback.
        """
        self.UpdateFeedback()
        
    def UpdateFeedback(self):
        """
        Implements rich feedback for validity checking.
        Currently: all team IDs must be unique and nonempty.
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
    

class TeamListDialog(wx.Dialog):
    def __init__(self, parent, book):
        wx.Dialog.__init__(self, parent)
        self.book = book

        self.teamList = TeamListCtrl(self, book)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.teamList, 1, wx.EXPAND, 0)

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
        for row in xrange(self.book.NumTeams()):
            self.book.ModifyTeam(teamID=str(self.teamList.GetCellValue(row, 0)),
                                 city=str(self.teamList.GetCellValue(row, 1)),
                                 nickname=str(self.teamList.GetCellValue(row, 2)),
                                 leagueID=str(self.teamList.GetCellValue(row, 3)))

        for row in xrange(self.book.NumTeams(), self.teamList.GetNumberRows()):
            self.book.AddTeam(teamID=str(self.teamList.GetCellValue(row, 0)),
                              city=str(self.teamList.GetCellValue(row, 1)),
                              nickname=str(self.teamList.GetCellValue(row, 2)),
                              leagueID=str(self.teamList.GetCellValue(row, 3)))
            
        event.Skip()
        

