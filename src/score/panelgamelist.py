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

class GameListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 2)
        self.SetColLabelValue(0, "Date")
        self.SetColLabelValue(1, "Result")
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER)
        self.EnableEditing(false)
        self.SetSelectionMode(wxGrid.wxGridSelectRows)

        self.filter = lambda x: True

        EVT_GRID_CELL_LEFT_DCLICK(self, self.OnLeftDoubleClick)

    def OnUpdate(self, book, f):
        """
        Update grid to list all games in scorebook 'book' matching
        filter function 'f'
        """
        self.book = book
        self.filter = f
        if self.GetNumberRows() > book.NumGames(self.filter):
            self.DeleteRows(0, self.GetNumberRows() - book.NumGames(self.filter))
        elif self.GetNumberRows() < book.NumGames(self.filter):
            self.InsertRows(0, book.NumGames(self.filter) - self.GetNumberRows())
        
        for (i,game) in enumerate(book.IterateGames(self.filter)):
            number = game.GetNumber()
            if number == 1:
                self.SetCellValue(i, 0, game.GetDate() + " (G1)")
            elif number == 2:
                self.SetCellValue(i, 0, game.GetDate() + " (G2)")
            else:
                self.SetCellValue(i, 0, game.GetDate())

            teams = [ book.GetTeam(t) for t in game.GetTeams() ]
            score = game.GetScore()
            result = ("%s %s %d at %s %s %d" %
                      (teams[0].city, teams[0].nickname, score[0],
                       teams[1].city, teams[1].nickname, score[1]))
            self.SetCellValue(i, 1, result)
            
        self.AutoSizeRows()
        self.AutoSizeColumns()
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
            self.teamList.Append(team.city + " " + team.nickname)
        self.teamList.SetStringSelection(teamChoice)
        
        self.gameList.OnUpdate(book, self.gameList.GetFilter())

    def OnTeamFilter(self, event):
        if event.GetSelection() == 0:
            self.gameList.OnUpdate(self.book, lambda x: True)
        else:
            team = [ x for x in self.book.IterateTeams() ][event.GetSelection() - 1]
            self.gameList.OnUpdate(self.book,
                                   lambda x: team.team_id in x.GetTeams())
