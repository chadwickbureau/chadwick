#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# The top-level frame for entering and editing a game account
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

import icons
from panelboxscore import BoxscorePanel
import panelstate
from panelnarrative import NarrativePanel
import game
import dialogdecision


class GameEntryFrame(wx.Frame):
    def __init__(self, parent, doc):
        wx.Frame.__init__(self, parent, title="Chadwick Game Entry",
                          size=(800, 600))
        self.doc = doc

        icon = wx.IconFromXPMData(icons.baseball_xpm)
        self.SetIcon(icon)

        self.notebook = wx.Notebook(self)

        self.statePanel = panelstate.StatePanel(self.notebook, doc)
        self.notebook.AddPage(self.statePanel, "Current State")

        self.boxscorePanel = BoxscorePanel(self.notebook, doc)
        self.notebook.AddPage(self.boxscorePanel, "Boxscore")

        self.narrativePanel = NarrativePanel(self.notebook, doc)
        self.notebook.AddPage(self.narrativePanel, "Narrative")

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.notebook, 1, wx.EXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()
        
        # Handle an "update" event: make sure all child windows update
        self.Bind(game.EVT_GAME_UPDATE, self.OnUpdate)

        # Handle a "close" event: set decisions
        self.Bind(wx.EVT_CLOSE, self.OnClose)

        # When switching to the narrative, we rebuild it
        # (but only then, since doing it the current way is slowish)
        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnNotebook, self.notebook)

        wx.PostEvent(self,
                     game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))
        

    def OnClose(self, event):
        if self.doc.GetScore(0) != self.doc.GetScore(1):
            dialog = dialogdecision.DecisionDialog(self, self.doc)
            if dialog.ShowModal() != wx.ID_OK:  return
            
            self.doc.SetInfo("wp", dialog.GetWinningPitcher())
            self.doc.SetInfo("save", dialog.GetSavePitcher())
            self.doc.SetInfo("lp", dialog.GetLosingPitcher())
        else:
            self.doc.SetInfo("wp", "")
            self.doc.SetInfo("save", "")
            self.doc.SetInfo("lp", "")
            
        for t in [0, 1]:
            for pitcher in self.doc.GetBoxscore().pitching[t]:
                self.doc.SetER(pitcher["id"], pitcher["er"])

        wx.PostEvent(self.GetParent(),
                     game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))
        event.Skip()

    def OnNotebook(self, event):
        if event.GetSelection() == 1:
            self.boxscorePanel.Rebuild()
        elif event.GetSelection() == 2:
            self.narrativePanel.Rebuild()

    def GetDocument(self):   return self.doc
        

    def OnUpdate(self, event):
        teams = [ self.doc.GetRoster(t).GetName() for t in [ 0, 1 ] ]
        gameDate = self.doc.GetGame().GetDate()
        
        if self.doc.GetInfo("number") == "0":
            gameNumber = ""
        elif self.doc.GetInfo("number") == "1":
            gameNumber = " (first game)"
        elif self.doc.GetInfo("number") == "2":
            gameNumber = " (second game)"
        
        self.SetTitle("Chadwick: " +
                      teams[0] + " at " + teams[1] + " on " + gameDate + gameNumber) 
        
        # We only update the boxscore and narrative windows when
        # they are shown
        self.statePanel.OnUpdate()

        # Let the event propagate upwards, so the game can be saved
        event.Skip()
