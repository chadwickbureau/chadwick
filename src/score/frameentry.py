#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# The top-level frame for entering and editing a game account
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

import icons
from dialoglineup import LineupDialog
from panelboxscore import BoxscorePanel
import panelstate
from panelnarrative import NarrativePanel

import sys

class GameEntryFrame(wxFrame):
    def __init__(self, parent):
        wxFrame.__init__(self, parent, -1, "Chadwick Game Entry",
                         wxDefaultPosition, wxSize(800, 600))

        icon = wxIconFromXPMData(icons.baseball_xpm)
        self.SetIcon(icon)

        self.notebook = wxNotebook(self, -1)

        self.statePanel = panelstate.StatePanel(self.notebook)
        self.notebook.AddPage(self.statePanel, "Current State")

        self.boxscorePanel = BoxscorePanel(self.notebook)
        self.notebook.AddPage(self.boxscorePanel, "Boxscore")

        self.narrativePanel = NarrativePanel(self.notebook)
        self.notebook.AddPage(self.narrativePanel, "Narrative")

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(self.notebook, 1, wxEXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()
        
        self.disabler = wxWindowDisabler(self)

        # System event handlers
        EVT_CLOSE(self, self.OnClickClose)

        # Handle an "update" event: make sure all child windows update
        EVT_BUTTON(self, panelstate.CW_BUTTON_UPDATE, self.OnUpdate)

        # When switching to the narrative, we rebuild it
        # (but only then, since doing it the current way is slowish)
        EVT_NOTEBOOK_PAGE_CHANGED(self, self.notebook.GetId(), self.OnNotebook)

    def OnClickClose(self, event):
        if event.CanVeto():
            dialog = wxMessageDialog(self,
                                     "The game has not been saved. "
                                     "Confirm close? (Changes will be lost!)",
                                     "Changes not saved",
                                     wxOK | wxCANCEL | wxICON_EXCLAMATION)
            if dialog.ShowModal() == wxID_CANCEL:
                event.Veto()
            else:
                event.Skip()
        else:
            event.Skip()

    def OnNotebook(self, event):
        if event.GetSelection() == 1:
            self.boxscorePanel.Rebuild()
        elif event.GetSelection() == 2:
            self.narrativePanel.Rebuild()

    def SetDocument(self, doc):
        self.doc = doc
        self.statePanel.SetDocument(doc)
        self.narrativePanel.SetDocument(doc)
        self.boxscorePanel.SetDocument(doc)
        self.SetGameTitle(doc)
        
    def GetDocument(self):   return self.doc
        
    def SetGameTitle(self, doc):
        teams = [ ]
        for t in [0, 1]:
            teams.append(doc.GetRoster(t).GetName())
        
        gameDate = doc.GetGame().GetDate()
        
        if doc.game.game_id[-1] == "0":
            gameNumber = ""
        elif doc.game.game_id[-1] == "1":
            gameNumber = " (first game)"
        elif doc.game.game_id[-1] == "2":
            gameNumber = " (second game)"
        
        self.SetTitle("Chadwick: " +
                      teams[0] + " at " + teams[1] + " on " + gameDate + gameNumber) 
        

    def OnUpdate(self, event):
        self.statePanel.OnUpdate()
        # We only update the boxscore and narrative windows when
        # they are shown

