#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for listing/filtering list of games in a scorebook
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

from wxutils import FormattedStaticText
from dialogboxview import BoxscoreViewDialog
from gameeditor import GameEditor

class GameListCtrl(wx.ListCtrl):
    def __init__(self, parent):
        wx.ListCtrl.__init__(self, parent, wx.ID_ANY,
                             style = wx.LC_VIRTUAL | wx.LC_REPORT | wx.LC_SINGLE_SEL)
        self.filter = lambda x: True

        self.InsertColumn(0, "Date")
        self.InsertColumn(1, "#")
        self.InsertColumn(2, "Visitors")
        self.InsertColumn(3, "Home")
        self.InsertColumn(4, "Win")
        self.InsertColumn(5, "Loss")
        self.InsertColumn(6, "Save")

        self.SetColumnWidth(0, 100)
        self.SetColumnWidth(1, 25)
        self.SetColumnWidth(2, 200)
        self.SetColumnWidth(3, 200)
        self.SetColumnWidth(4, 150)
        self.SetColumnWidth(5, 150)
        self.SetColumnWidth(6, 150)

        item = wx.ListItem()
        item.m_format = wx.LIST_FORMAT_CENTRE
        item.m_mask = wx.LIST_MASK_FORMAT
        for col in range(7):  self.SetColumn(col, item)

        wx.EVT_LIST_ITEM_ACTIVATED(self, self.GetId(), self.OnItemActivate)
        wx.EVT_LIST_KEY_DOWN(self, self.GetId(), self.OnKeyDown)
        
    def OnUpdate(self, book, f):
        """
        Update grid to list all games in scorebook 'book' matching
        filter function 'f'
        """
        self.book = book
        self.games = [ g for g in book.Games(f) ]
        self.SetItemCount(book.NumGames(f))

    def OnGetItemText(self, item, col):
        game = self.games[item]
        if col == 0:
            return game.GetDate()
        elif col == 1:
            return { 0: "", 1: "1", 2: "2" }[game.GetNumber()]
        elif col == 2:
            return (self.book.GetTeam(game.GetTeam(0)).GetName() + " " +
                    str(game.GetScore()[0]))
        elif col == 3:
            return (self.book.GetTeam(game.GetTeam(1)).GetName() + " " +
                    str(game.GetScore()[1]))
        elif col == 4:
            pitcher = game.GetWinningPitcher()
            if pitcher != "":
                return self.book.GetPlayer(pitcher).GetName()
            else:
                return ""
        elif col == 5:
            pitcher = game.GetLosingPitcher()
            if pitcher != "":
                return self.book.GetPlayer(pitcher).GetName()
            else:
                return ""
        else:
            pitcher = game.GetSavePitcher()
            if pitcher != "":
                return self.book.GetPlayer(pitcher).GetName()
            else:
                return ""

    def OnItemActivate(self, event):
        game = self.games[event.GetIndex()]
        teams = [ self.book.GetTeam(t) for t in game.GetTeams() ]
        doc = GameEditor(game, teams[0], teams[1])

        #dialog = BoxscoreViewDialog(self, doc)
        #dialog.ShowModal()

        # Hackish.  Should really do this via an event mechanism
        wx.GetApp().GetTopWindow().EditGame(doc)
        

    def OnKeyDown(self, event):
        if event.GetKeyCode() == wx.WXK_DELETE:
            game = self.games[event.GetIndex()]
            dialog = wx.MessageDialog(self,
                                      "Are you sure you want to delete this game?",
                                      "Confirm delete",
                                      wx.OK | wx.CANCEL)
            if dialog.ShowModal() == wx.ID_OK:
                self.book.RemoveGame(game)
                self.GetParent().GetGrandParent().OnUpdate()
        else:
            event.Skip()

    def GetFilter(self):  return self.filter


class GameListPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)

        filterSizer = wx.BoxSizer(wx.HORIZONTAL)
        filterSizer.Add(FormattedStaticText(self, "Show games involving"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.teamList = wx.Choice(self, wx.ID_ANY, size = (250, -1))
        self.teamList.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.teamList.Clear()
        self.teamList.Append("all teams")
        self.teamList.SetSelection(0)
        filterSizer.Add(self.teamList, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        self.gameList = GameListCtrl(self)

        sizer = wx.BoxSizer(wx.VERTICAL)

        toolSizer = wx.BoxSizer(wx.HORIZONTAL)
        toolSizer.Add(filterSizer, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(toolSizer, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(self.gameList, 1, wx.EXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        wx.EVT_CHOICE(self, self.teamList.GetId(), self.OnTeamFilter)

    def OnUpdate(self, book):
        self.book = book
        teamChoice = self.teamList.GetStringSelection()
        self.teamList.Clear()
        self.teamList.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.teamList.Append("all teams")
        for team in book.Teams():
            self.teamList.Append(team.GetName())
        self.teamList.SetStringSelection(teamChoice)
        
        self.gameList.OnUpdate(book, self.gameList.GetFilter())

    def OnTeamFilter(self, event):
        if event.GetSelection() == 0:
            self.gameList.OnUpdate(self.book, lambda x: True)
        else:
            team = [ x for x in self.book.Teams() ][event.GetSelection() - 1]
            self.gameList.OnUpdate(self.book,
                                   lambda x: team.GetID() in x.GetTeams())
