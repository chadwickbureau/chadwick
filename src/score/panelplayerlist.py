#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel for viewing and editing players
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

import string
import wx

from wxutils import FormattedStaticText

class NameValidator(wx.PyValidator):
    """
    This validates a text control as a valid name.  We permit
    letters, whitespace, and apostrophes.
    """
    def __init__(self, pyVar=None):
        wx.PyValidator.__init__(self)
        wx.EVT_CHAR(self, self.OnChar)

    def TransferToWindow(self):     return True
    def TransferFromWindow(self):   return True 
    def Clone(self):                return NameValidator()

    def Validate(self, win):
        textCtrl = self.GetWindow()
        val = str(textCtrl.GetValue())
        
        for x in val:
            if x not in string.letters and x not in [ " ", "'" ]:
                return False

        return True

    def OnChar(self, event):
        key = event.KeyCode()

        if key < wx.WXK_SPACE or key == wx.WXK_DELETE or key > 255:
            event.Skip()
            return

        if chr(key) in string.letters or chr(key) in [ " ", "'" ]:
            event.Skip()
            return

        if not wx.Validator_IsSilent():
            wx.Bell()

        # Returning without calling event.Skip eats the event before it
        # gets to the text control
        return


class AddPlayerDialog(wx.Dialog):
    def __init__(self, parent, book):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Add player")
        self.book = book

        self.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        sizer = wx.FlexGridSizer(6)

        sizer.Add(FormattedStaticText(self, "First name"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.firstName = wx.TextCtrl(self, wx.ID_ANY, "", size=(150, -1))
        self.firstName.SetValidator(NameValidator())
        sizer.Add(self.firstName, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Last name"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.lastName = wx.TextCtrl(self, -1, "", size=(150, -1))
        self.lastName.SetValidator(NameValidator())
        sizer.Add(self.lastName, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Bats"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.bats = wx.Choice(self, wx.ID_ANY, wx.DefaultPosition, (150, -1),
                              [ "Unknown", "Right", "Left", "Both" ])
        self.bats.SetSelection(1)
        sizer.Add(self.bats, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Throws"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.throws = wx.Choice(self, wx.ID_ANY, wx.DefaultPosition, (150, -1),
                                [ "Unknown", "Right", "Left" ])
        self.throws.SetSelection(1)
        sizer.Add(self.throws, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Team"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        teamList = [ team.GetName() for team in book.Teams() ]
        self.team = wx.Choice(self, wx.ID_ANY, wx.DefaultPosition, (150, -1),
                              teamList)
        self.team.SetSelection(0)
        sizer.Add(self.team, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Player ID"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.playerID = wx.TextCtrl(self, -1, "", size=(150, -1))
        # The player ID starts out blank, which is invalid
        self.playerID.SetBackgroundColour(wx.NamedColour("pink"))
        sizer.Add(self.playerID, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        generateButton = wx.Button(self, wx.ID_ANY, "Generate unique ID")
        generateButton.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        
        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(wx.ID_OK).Enable(False)
        self.FindWindowById(wx.ID_OK).SetDefault()

        topSizer = wx.BoxSizer(wx.VERTICAL)

        topSizer.Add(sizer, 0, wx.ALL, 5)
        topSizer.Add(generateButton, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        topSizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        wx.EVT_TEXT(self, self.playerID.GetId(), self.OnIDChange)
        wx.EVT_BUTTON(self, generateButton.GetId(), self.OnGenerateID)

    def OnGenerateID(self, event):
        self.playerID.SetValue(self.book.UniquePlayerID(self.GetFirstName(),
                                                        self.GetLastName()))
        self.playerID.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOW))
        self.FindWindowById(wx.ID_OK).Enable(True)

    def OnIDChange(self, event):
        playerID = self.GetPlayerID()
        if (playerID == "" or
            playerID in [ p.player_id for p in self.book.Players() ]):
            self.FindWindowById(wx.ID_OK).Enable(False)
            self.playerID.SetBackgroundColour(wx.NamedColour("pink"))
        else:
            self.FindWindowById(wx.ID_OK).Enable(True)
            self.playerID.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOW))
        
    def GetPlayerID(self):   return str(self.playerID.GetValue()).strip()
    def GetFirstName(self):  return str(self.firstName.GetValue()).strip()
    def GetLastName(self):   return str(self.lastName.GetValue()).strip()
    def GetBats(self):
        return [ "?", "R", "L", "B" ][self.bats.GetSelection()]
    def GetThrows(self):
        return [ "?", "R", "L" ][self.throws.GetSelection()]
    def GetTeam(self):
        return [ t for t in self.book.Teams() ][self.team.GetSelection()].GetID()
    

class EditPlayerDialog(wx.Dialog):
    def __init__(self, parent, player):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Edit player")

        self.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))
        
        sizer = wx.FlexGridSizer(5)

        sizer.Add(FormattedStaticText(self, "First name"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.firstName = wx.TextCtrl(self, wx.ID_ANY, player.GetFirstName(),
                                     size=(150, -1))
        self.firstName.SetValidator(NameValidator())
        sizer.Add(self.firstName, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Last name"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.lastName = wx.TextCtrl(self, wx.ID_ANY, player.GetLastName(),
                                    size=(150, -1))
        self.lastName.SetValidator(NameValidator())
        sizer.Add(self.lastName, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Bats"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.bats = wx.Choice(self, wx.ID_ANY,
                              wx.DefaultPosition, (150, -1),
                              [ "Unknown", "Right", "Left", "Both" ])
        self.bats.SetSelection({ "?": 0, "R": 1, "L": 2, "B": 3}[player.bats])
        sizer.Add(self.bats, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Throws"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.throws = wx.Choice(self, wx.ID_ANY,
                                wx.DefaultPosition, (150, -1),
                                [ "Unknown", "Right", "Left" ])
        self.throws.SetSelection({ "?": 0, "R": 1, "L": 2}[player.throws])
        sizer.Add(self.throws, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        #sizer.Add(FormattedStaticText(self, "Team"),
        #          0, wx.ALL | wx.ALIGN_CENTER, 5)
        #teamList = [ team.GetName() for team in book.Teams() ]
        #self.team = wx.Choice(self, -1, wx.DefaultPosition, wx.Size(150, -1),
        #                     teamList)
        #self.team.SetSelection(0)
        #sizer.Add(self.team, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(FormattedStaticText(self, "Player ID"),
                  0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.playerID = wx.TextCtrl(self, wx.ID_ANY, player.player_id,
                                    size=(150, -1))
        self.playerID.Enable(False)
        sizer.Add(self.playerID, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(wx.ID_OK).SetDefault()

        topSizer = wx.BoxSizer(wx.VERTICAL)

        topSizer.Add(sizer, 0, wx.ALL, 5)
        topSizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

    def GetFirstName(self):  return str(self.firstName.GetValue()).strip()
    def GetLastName(self):   return str(self.lastName.GetValue()).strip()
    def GetBats(self):
        return [ "?", "R", "L", "B" ][self.bats.GetSelection()]
    def GetThrows(self):
        return [ "?", "R", "L" ][self.throws.GetSelection()]
    

class PlayerListCtrl(wx.ListCtrl):
    def __init__(self, parent):
        wx.ListCtrl.__init__(self, parent, wx.ID_ANY,
                             style = wx.LC_VIRTUAL | wx.LC_REPORT | wx.LC_SINGLE_SEL)
        self.firstMenuID = 4000

        self.InsertColumn(0, "Player")
        self.InsertColumn(1, "Bats")
        self.InsertColumn(2, "Throws")
        self.InsertColumn(3, "ID")
        self.InsertColumn(4, "Teams")

        self.SetColumnWidth(0, 250)
        self.SetColumnWidth(1, 75)
        self.SetColumnWidth(2, 75)
        self.SetColumnWidth(3, 100)
        self.SetColumnWidth(4, 150)
        
        wx.EVT_LIST_ITEM_ACTIVATED(self, self.GetId(), self.OnItemActivated)
        wx.EVT_LIST_ITEM_RIGHT_CLICK(self, self.GetId(), self.OnRightClick)
        wx.EVT_MENU_RANGE(self, self.firstMenuID, self.firstMenuID + 100,
                          self.OnMenu)
        
    def OnUpdate(self, book):
        self.book = book
        self.SetItemCount(book.NumPlayers())

    def OnGetItemText(self, item, col):
        player = [x for x in self.book.Players()][item]
        if col == 0:
            return player.GetSortName()
        elif col == 1:
            return { "R": "Right", "L": "Left",
                     "B": "Both", "?": "Unknown" }[player.GetBats()]
        elif col == 2:
            return { "R": "Right", "L": "Left",
                     "B": "Both", "?": "Unknown" }[player.GetThrows()]
        elif col == 3:
            return player.GetID()
        else:
            return string.join(player.GetTeams(), ", ")
        

    def OnItemActivated(self, event):
        player = [x for x in self.book.Players()][event.GetIndex()]
        dialog = EditPlayerDialog(self, player)
        if dialog.ShowModal() == wx.ID_OK:
            self.book.ModifyPlayer(player.player_id,
                                   dialog.GetFirstName(),
                                   dialog.GetLastName(),
                                   dialog.GetBats(),
                                   dialog.GetThrows())
            self.GetParent().GetGrandParent().OnUpdate()
            return

    def OnRightClick(self, event):
        player = [x for x in self.book.Players()][event.GetIndex()]
        self.menuPlayer = player
        menu = wx.Menu("Add %s to roster of" % player.GetName())

        menuID = self.firstMenuID
        for t in self.book.Teams():
            if t.GetID() not in player.GetTeams():
                menu.Append(menuID, t.GetName(),
                            "Add to %s" % t.GetName())
                menuID += 1
                
        self.PopupMenu(menu)

    def OnMenu(self, event):
        menuID = self.firstMenuID

        for t in self.book.Teams():
            if t.GetID() not in self.menuPlayer.GetTeams():
                if menuID == event.GetId():
                    self.book.AddToTeam(self.menuPlayer.player_id,
                                        self.menuPlayer.GetFirstName(),
                                        self.menuPlayer.GetLastName(),
                                        self.menuPlayer.bats,
                                        self.menuPlayer.throws,
                                        t.GetID())
                    self.GetParent().GetGrandParent().OnUpdate()
                    return
                menuID += 1

class PlayerListPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        newPlayerButton = wx.Button(self, wx.ID_ANY, "Add player")
        newPlayerButton.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.playerList = PlayerListCtrl(self)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(newPlayerButton, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(self.playerList, 1, wx.EXPAND, 0)
        self.SetSizer(sizer)
        self.Layout()

        wx.EVT_BUTTON(self, newPlayerButton.GetId(), self.OnNewPlayer)

    def OnNewPlayer(self, event):
        dialog = AddPlayerDialog(self, self.book)

        if dialog.ShowModal() == wx.ID_OK:
            self.book.AddPlayer(dialog.GetPlayerID(),
                                dialog.GetFirstName(),
                                dialog.GetLastName(),
                                dialog.GetBats(),
                                dialog.GetThrows(),
                                dialog.GetTeam())
            self.GetGrandParent().OnUpdate()

    def OnUpdate(self, book):
        self.book = book
        self.playerList.OnUpdate(book)
