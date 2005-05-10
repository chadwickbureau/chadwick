#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for displaying and editing teams
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

class AddTeamDialog(wxDialog):
    def __init__(self, parent, book):
        wxDialog.__init__(self, parent, -1, "Add team")
        self.book = book

        self.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))

        sizer = wxFlexGridSizer(4)

        sizer.Add(FormattedStaticText(self, "City"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.city = wxTextCtrl(self, -1, "",
                               wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.city, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Nickname"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.nickname = wxTextCtrl(self, -1, "",
                                   wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.nickname, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team ID"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.teamID = wxTextCtrl(self, -1, "",
                                 wxDefaultPosition, wxSize(150, -1))
        # A blank team ID is invalid, so flag this as invalid
        self.teamID.SetBackgroundColour(wxNamedColour("pink"))
        sizer.Add(self.teamID, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "League"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.league = wxTextCtrl(self, -1, "",
                                 wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.league, 0, wxALL | wxALIGN_CENTER, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(wxID_OK).Enable(false)

        topSizer = wxBoxSizer(wxVERTICAL)

        topSizer.Add(sizer, 0, wxALL, 5)
        topSizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        EVT_TEXT(self, self.teamID.GetId(), self.OnTeamIDChange)

    def OnTeamIDChange(self, event):
        if str(self.teamID.GetValue()) == "":
            self.FindWindowById(wxID_OK).Enable(false)
            self.teamID.SetBackgroundColour(wxNamedColour("pink"))
        elif (str(self.teamID.GetValue()) in
              [ x.GetID() for x in self.book.Teams() ]):
            self.FindWindowById(wxID_OK).Enable(false)
            self.teamID.SetBackgroundColour(wxNamedColour("pink"))
        else:
            self.FindWindowById(wxID_OK).Enable(true)
            self.teamID.SetBackgroundColour(wxSystemSettings_GetColour(wxSYS_COLOUR_WINDOW))
              
    def GetCity(self):       return str(self.city.GetValue())
    def GetNickname(self):   return str(self.nickname.GetValue())
    def GetTeamID(self):     return str(self.teamID.GetValue())
    def GetLeague(self):     return str(self.league.GetValue())


class EditTeamDialog(wxDialog):
    def __init__(self, parent, book, team):
        wxDialog.__init__(self, parent, -1, "Edit team")

        self.book = book
        self.team = team
        
        self.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))

        sizer = wxFlexGridSizer(2)

        sizer.Add(FormattedStaticText(self, "City"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.city = wxTextCtrl(self, -1, team.GetCity(),
                               wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.city, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Nickname"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.nickname = wxTextCtrl(self, -1, team.GetNickname(),
                                   wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.nickname, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team ID"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.teamID = wxTextCtrl(self, -1, team.GetID(),
                                 wxDefaultPosition, wxSize(150, -1))
        self.teamID.Enable(False)
        sizer.Add(self.teamID, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "League"),
                  0, wxALL | wxALIGN_CENTER, 5)
        self.league = wxTextCtrl(self, -1, team.GetLeague(),
                                 wxDefaultPosition, wxSize(150, -1))
        sizer.Add(self.league, 0, wxALL | wxALIGN_CENTER, 5)


        playerBox = wxStaticBoxSizer(wxStaticBox(self, wxID_STATIC,
                                                 "Players on team"),
                                     wxHORIZONTAL)
        self.players = wxListView(self, -1, wxDefaultPosition,
                                  wxSize(250, 150))
        self.players.InsertColumn(0, "Name (ID)", width=230)
        self.playerIDs = [ x.GetID() for x in team.Players() ]
        for (pl,player) in enumerate(team.Players()):
            self.players.InsertStringItem(pl,
                                          "%s (%s)" % (player.GetSortName(),
                                                       player.GetID()))
            self.players.SetItemTextColour(pl, wxBLUE)
            
        playerBox.Add(self.players, 0, wxALL, 5)
        
        allPlayerBox = wxStaticBoxSizer(wxStaticBox(self, wxID_STATIC,
                                                    "All players"),
                                        wxHORIZONTAL)
        self.allPlayers = wxListView(self, -1, wxDefaultPosition,
                                     wxSize(250, 150), style=wxLC_REPORT)
        self.allPlayers.InsertColumn(0, "Name (ID)", width=230)
        self.allPlayerIDs = [ x.GetID() for x in book.Players() ]
        for (pl,player) in enumerate(book.Players()):
            self.allPlayers.InsertStringItem(pl,
                                             "%s (%s)" % (player.GetSortName(),
                                                          player.GetID()))
            if team.FindPlayer(player.GetID()) != None:
                self.allPlayers.SetItemTextColour(pl, wxBLUE)
            else:
                self.allPlayers.SetItemTextColour(pl, wxBLACK)

        EVT_LIST_ITEM_ACTIVATED(self, self.allPlayers.GetId(),
                                self.OnPlayerActivated)
            
        allPlayerBox.Add(self.allPlayers, 0, wxALL, 5)


        playerSizer = wxBoxSizer(wxHORIZONTAL)
        playerSizer.Add(playerBox, 0, wxALL, 5)
        playerSizer.Add(allPlayerBox, 0, wxALL, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)

        topSizer = wxBoxSizer(wxVERTICAL)

        topSizer.Add(sizer, 0, wxALL, 5)
        topSizer.Add(playerSizer, 0, wxALL, 5)
        topSizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

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
                                           wxColour(0, 192, 0))
            self.players.EnsureVisible(self.playerIDs.index(playerID))
            self.allPlayers.SetItemTextColour(event.GetIndex(), wxBLUE)

    def UpdateTeam(self):
        for p in self.playerIDs:
            if self.team.FindPlayer(p) == None:
                player = self.book.GetPlayer(p)
                self.book.AddToTeam(p,
                                    player.GetFirstName(), player.GetLastName(),
                                    player.GetBats(), player.GetThrows(),
                                    self.team.GetID())

class TeamListCtrl(wxListCtrl):
    def __init__(self, parent):
        wxListCtrl.__init__(self, parent, -1,
                            style = wxLC_VIRTUAL | wxLC_REPORT | wxLC_SINGLE_SEL)

        self.InsertColumn(0, "City")
        self.InsertColumn(1, "Nickname")
        self.InsertColumn(2, "Team ID")
        self.InsertColumn(3, "League")
        
        self.SetColumnWidth(0, 150)
        self.SetColumnWidth(1, 150)
        self.SetColumnWidth(2, 100)
        self.SetColumnWidth(3, 100)

        item = wxListItem()
        item.m_format = wxLIST_FORMAT_CENTRE
        item.m_mask = wxLIST_MASK_FORMAT
        for col in [2, 3]:  self.SetColumn(col, item)

        EVT_LIST_ITEM_ACTIVATED(self, self.GetId(), self.OnItemActivate)

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
        if dialog.ShowModal() == wxID_OK:
            self.book.ModifyTeam(team.GetID(),
                                 dialog.GetCity(),
                                 dialog.GetNickname(),
                                 dialog.GetLeague())
            dialog.UpdateTeam()
            self.GetParent().GetGrandParent().OnUpdate()

class TeamListPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        newTeamButton = wxButton(self, -1, "Add team")
        newTeamButton.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.teamList = TeamListCtrl(self)

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(newTeamButton, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(self.teamList, 1, wxEXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        EVT_BUTTON(self, newTeamButton.GetId(), self.OnAddTeam)

    def OnUpdate(self, book):
        self.book = book
        self.teamList.OnUpdate(book)

    def OnAddTeam(self, event):
        dialog = AddTeamDialog(self, self.book)

        if dialog.ShowModal() == wxID_OK:
            self.book.AddTeam(dialog.GetTeamID(),
                              dialog.GetCity(), dialog.GetNickname(),
                              dialog.GetLeague())
            self.GetGrandParent().OnUpdate()
