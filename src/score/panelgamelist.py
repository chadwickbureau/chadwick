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

from dialogboxview import BoxscoreViewDialog
from gameeditor import GameEditor

class GameListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 4)
        self.SetColLabelValue(0, "Game ID")
        self.SetColLabelValue(1, "Date")
        self.SetColLabelValue(2, "Visitors")
        self.SetColLabelValue(3, "Home")
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.EnableEditing(false)

        EVT_GRID_CELL_LEFT_DCLICK(self, self.OnLeftDoubleClick)

    def OnUpdate(self, book):
        self.book = book
        if self.GetNumberRows() > book.NumGames():
            self.DeleteRows(self.GetNumberRows() - book.NumGames())
        elif self.GetNumberRows() < book.NumGames():
            self.InsertRows(0, book.NumGames() - self.GetNumberRows())

        for (i,game) in enumerate(book.IterateGames()):
            self.SetCellValue(i, 0, game.game_id)
            self.SetCellValue(i, 1, cw_game_info_lookup(game, "date"))
            self.SetCellValue(i, 2, cw_game_info_lookup(game, "visteam"))
            self.SetCellValue(i, 3, cw_game_info_lookup(game, "hometeam"))

        self.AutoSizeRows()
        self.AutoSizeColumns()
        self.AdjustScrollbars()

    def OnLeftDoubleClick(self, event):
        for game in self.book.IterateGames():
            if game.game_id == str(self.GetCellValue(event.GetRow(), 0)):
                vis = str(self.GetCellValue(event.GetRow(), 2))
                home = str(self.GetCellValue(event.GetRow(), 3))
                doc = GameEditor(game,
                                 self.book.GetTeam(vis),
                                 self.book.GetTeam(home))
                dialog = BoxscoreViewDialog(self, doc)
                dialog.ShowModal()
                return

CW_MENU_GAME_NEW = 2000

class GameListPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        newGameButton = wxButton(self, CW_MENU_GAME_NEW, "Enter new game")
        newGameButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.gameList = GameListGrid(self)

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(newGameButton, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(self.gameList, 1, wxEXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

    def OnUpdate(self, book):
        self.gameList.OnUpdate(book)
