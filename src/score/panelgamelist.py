#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for listing/filtering list of games in a scorebook
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
from dialogboxview import BoxscoreViewDialog
from gameeditor import GameEditor

class GameListTable(wxPyGridTableBase):
    def __init__(self, parent):
        wxPyGridTableBase.__init__(self)
        self.parent = parent
        self.filter = lambda x: True
        self.games= [ ]
        self.attr = [ wxGridCellAttr() for i in range(7) ]

        for x in self.attr:
            x.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))
            x.SetTextColour(wxBLACK)
            x.SetBackgroundColour(wxColour(242, 242, 242))
        
        self.attr[0].SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.attr[1].SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.attr[2].SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.attr[3].SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.attr[4].SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER)
        self.attr[5].SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER)
        self.attr[6].SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER)

    def SetScorebook(self, book):
        self.book = book
        self.games = [ g for g in self.book.IterateGames(self.filter) ]

    def SetFilter(self, f):
        self.filter = f
        self.games = [ g for g in self.book.IterateGames(self.filter) ]

    def GetAttr(self, row, col, kind):
        if col not in [2, 3]:
            self.attr[col].IncRef()
            return self.attr[col]
        else:
            attr = self.attr[col].Clone()
            try:
                game = self.games[row]
            except:
                attr.IncRef()
                return attr
            
            score = game.GetScore()
            if (score[0] > score[1] and col == 2) or \
               (score[1] > score[0] and col == 3):
                attr.SetTextColour(wxNamedColour("blue"))
            attr.IncRef()
            return attr

    def GetNumberRows(self):
        if hasattr(self, "book"):
            return self.book.NumGames(self.filter)
        else:
            return 0

    def GetNumberCols(self):
        return 7

    def IsEmptyCell(self, row, col):
        return False

    def GetValue(self, row, col):
        if not hasattr(self, "book"):  return ""

        game = self.games[row]
        if col == 0:
            return game.GetDate()
        elif col == 1:
            return { 0: "", 1: "1", 2: "2" }[game.GetNumber()]
        elif col == 2:
            return ("%s %d" %
                    (self.book.GetTeam(game.GetTeam(0)).GetName(),
                     game.GetScore()[0]))
        elif col == 3:
            return ("%s %d" %
                   (self.book.GetTeam(game.GetTeam(1)).GetName(),
                     game.GetScore()[1]))
        elif col == 4:
            wp = game.GetWinningPitcher()
            if wp != "":
                return self.book.GetPlayer(wp).GetSortName()
            else:
                return ""
        elif col == 5:
            lp = game.GetLosingPitcher()
            if lp != "":
                return self.book.GetPlayer(lp).GetSortName()
            else:
                return ""
        elif col == 6:
            save = game.GetSavePitcher()
            if save != "":
                return self.book.GetPlayer(save).GetSortName()
            else:
                return ""
            

    def SetValue(self, row, col, value):
        pass

    def GetColLabelValue(self, col):
        return [ "Date", "#", "Visitors", "Home", "Winner", "Loser", "Save" ][col]

    def AppendRows(self, howMany):
        msg = wxGridTableMessage(self,
                                 wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
                                 howMany)
        self.parent.ProcessTableMessage(msg)

    def DeleteRows(self, where, howMany):
        msg = wxGridTableMessage(self,
                                 wxGRIDTABLE_NOTIFY_ROWS_DELETED,
                                 where, howMany)
        self.parent.ProcessTableMessage(msg)
                                 

class GameListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.table = GameListTable(self)
        self.filter = lambda x: True
        self.SetTable(self.table, True)
        self.EnableEditing(false)
        self.SetSelectionMode(wxGrid.wxGridSelectRows)

        self.SetRowLabelSize(1)
        self.SetLabelFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))
        self.SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.SetDefaultCellBackgroundColour(wxColour(242, 242, 242))
        self.DisableDragRowSize()
        
        self.SetColSize(0, 100)
        self.SetColSize(1, 25)
        self.SetColSize(2, 175)
        self.SetColSize(3, 175)
        self.SetColSize(4, 150)
        self.SetColSize(5, 150)
        self.SetColSize(6, 150)

        EVT_GRID_CELL_LEFT_DCLICK(self, self.OnLeftDoubleClick)

    def OnUpdate(self, book, f):
        """
        Update grid to list all games in scorebook 'book' matching
        filter function 'f'
        """
        self.table.SetScorebook(book)
        self.table.SetFilter(f)
        self.book = book
        self.filter = f
        if self.GetNumberRows() > book.NumGames(self.filter):
            self.DeleteRows(0, self.GetNumberRows() - book.NumGames(self.filter))
        elif self.GetNumberRows() < book.NumGames(self.filter):
            self.AppendRows(book.NumGames(self.filter) - self.GetNumberRows())

        if self.GetNumberRows() > 0:
            self.AutoSizeRow(0)
            self.SetDefaultRowSize(self.GetRowSize(0))
        self.AdjustScrollbars()

    def OnLeftDoubleClick(self, event):
        for (i,game) in enumerate(self.book.IterateGames(self.filter)):
            if i == event.GetRow():
                teams = [ self.book.GetTeam(t) for t in game.GetTeams() ]
                doc = GameEditor(game, teams[0], teams[1])
                dialog = BoxscoreViewDialog(self, doc)
                dialog.ShowModal()
                return

    def GetFilter(self):  return self.filter

CW_MENU_GAME_NEW = 2000

class GameListPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        filterSizer = wxBoxSizer(wxHORIZONTAL)
        filterSizer.Add(FormattedStaticText(self, "Show games involving"),
                        0, wxALL | wxALIGN_CENTER, 5)
        self.teamList = wxChoice(self, -1, wxDefaultPosition, wxSize(250, -1))
        self.teamList.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.teamList.Clear()
        self.teamList.Append("all teams")
        self.teamList.SetSelection(0)
        filterSizer.Add(self.teamList, 0, wxALL | wxALIGN_CENTER, 5)

        newGameButton = wxButton(self, CW_MENU_GAME_NEW, "Enter new game")
        newGameButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        self.gameList = GameListGrid(self)

        sizer = wxBoxSizer(wxVERTICAL)

        toolSizer = wxBoxSizer(wxHORIZONTAL)
        toolSizer.Add(filterSizer, 0, wxALL | wxALIGN_CENTER, 5)
        toolSizer.Add(wxSize(30, 1))
        toolSizer.Add(newGameButton, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(toolSizer, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(self.gameList, 1, wxEXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        EVT_CHOICE(self, self.teamList.GetId(), self.OnTeamFilter)

    def OnUpdate(self, book):
        self.book = book
        teamChoice = self.teamList.GetStringSelection()
        self.teamList.Clear()
        self.teamList.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.teamList.Append("all teams")
        for team in book.IterateTeams():
            self.teamList.Append(team.GetName())
        self.teamList.SetStringSelection(teamChoice)
        
        self.gameList.OnUpdate(book, self.gameList.GetFilter())

    def OnTeamFilter(self, event):
        if event.GetSelection() == 0:
            self.gameList.OnUpdate(self.book, lambda x: True)
        else:
            team = [ x for x in self.book.IterateTeams() ][event.GetSelection() - 1]
            self.gameList.OnUpdate(self.book,
                                   lambda x: team.team_id in x.GetTeams())
