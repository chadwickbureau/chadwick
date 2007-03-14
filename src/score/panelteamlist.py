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

import wx 
from libchadwick import *

from wxutils import FormattedStaticText

class AddTeamDialog(wx.Dialog):
    def __init__(self, parent, book):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Add team")
        self.book = book

        self.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        sizer = wx.FlexGridSizer(4)

        sizer.Add(FormattedStaticText(self, "City"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.city = wx.TextCtrl(self, wx.ID_ANY, "", size=(150, -1))
        sizer.Add(self.city, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Nickname"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.nickname = wx.TextCtrl(self, wx.ID_ANY, "", size=(150, -1))
        sizer.Add(self.nickname, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team ID"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.teamID = wx.TextCtrl(self, wx.ID_ANY, "", size=(150, -1))
        # A blank team ID is invalid, so flag this as invalid
        self.teamID.SetBackgroundColour(wx.NamedColour("pink"))
        sizer.Add(self.teamID, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "League"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.league = wx.TextCtrl(self, wx.ID_ANY, "", size=(150, -1))
        sizer.Add(self.league, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(wx.ID_OK).Enable(False)

        topSizer = wx.BoxSizer(wx.VERTICAL)

        topSizer.Add(sizer, 0, wx.ALL, 5)
        topSizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        wx.EVT_TEXT(self, self.teamID.GetId(), self.OnTeamIDChange)

    def OnTeamIDChange(self, event):
        if str(self.teamID.GetValue()) == "":
            self.FindWindowById(wx.ID_OK).Enable(False)
            self.teamID.SetBackgroundColour(wx.NamedColour("pink"))
        elif (str(self.teamID.GetValue()) in
              [ x.GetID() for x in self.book.Teams() ]):
            self.FindWindowById(wx.ID_OK).Enable(False)
            self.teamID.SetBackgroundColour(wx.NamedColour("pink"))
        else:
            self.FindWindowById(wx.ID_OK).Enable(True)
            self.teamID.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOW))
              
    def GetCity(self):       return str(self.city.GetValue())
    def GetNickname(self):   return str(self.nickname.GetValue())
    def GetTeamID(self):     return str(self.teamID.GetValue())
    def GetLeague(self):     return str(self.league.GetValue())


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

class TeamListCtrl(wx.ListCtrl):
    def __init__(self, parent):
        wx.ListCtrl.__init__(self, parent, -1,
                            style = wx.LC_VIRTUAL | wx.LC_REPORT | wx.LC_SINGLE_SEL)

        self.InsertColumn(0, "City")
        self.InsertColumn(1, "Nickname")
        self.InsertColumn(2, "Team ID")
        self.InsertColumn(3, "League")
        
        self.SetColumnWidth(0, 150)
        self.SetColumnWidth(1, 150)
        self.SetColumnWidth(2, 100)
        self.SetColumnWidth(3, 100)

        item = wx.ListItem()
        item.m_format = wx.LIST_FORMAT_CENTRE
        item.m_mask = wx.LIST_MASK_FORMAT
        for col in [2, 3]:  self.SetColumn(col, item)

        wx.EVT_LIST_ITEM_ACTIVATED(self, self.GetId(), self.OnItemActivate)

    def OnUpdate(self, book):
        self.book = book
        self.SetItemCount(book.NumTeams())

    def OnGetItemText(self, item, col):
        team = [x for x in self.book.Teams()][item]
        if col == 0:
            return team.GetCity()
        elif col == 1:
            return team.GetNickname()
        elif col == 2:
            return team.GetID()
        else:
            return team.GetLeague()
        
    def OnItemActivate(self, event):
        team = [x for x in self.book.Teams()][event.GetIndex()]

        dialog = EditTeamDialog(self, self.book, team)
        if dialog.ShowModal() == wx.ID_OK:
            self.book.ModifyTeam(team.GetID(),
                                 dialog.GetCity(),
                                 dialog.GetNickname(),
                                 dialog.GetLeague())
            dialog.UpdateTeam()
            self.GetParent().GetGrandParent().OnUpdate()

class TeamListPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        newTeamButton = wx.Button(self, wx.ID_ANY, "Add team")
        newTeamButton.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.teamList = TeamListCtrl(self)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(newTeamButton, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(self.teamList, 1, wx.EXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        wx.EVT_BUTTON(self, newTeamButton.GetId(), self.OnAddTeam)

    def OnUpdate(self, book):
        self.book = book
        self.teamList.OnUpdate(book)

    def OnAddTeam(self, event):
        dialog = AddTeamDialog(self, self.book)

        if dialog.ShowModal() == wx.ID_OK:
            self.book.AddTeam(dialog.GetTeamID(),
                              dialog.GetCity(), dialog.GetNickname(),
                              dialog.GetLeague())
            self.GetGrandParent().OnUpdate()
