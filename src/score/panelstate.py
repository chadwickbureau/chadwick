#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Implementation of panel showing the current state of a game
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
from libchadwick import *

# Import of dialogs used
import dialogcomment
import dialogdecision
import dialoggame
import dialoglineup

# Import of controls comprising this panel
from wxutils import FormattedStaticText
import gridlineup
import panellinescore
import panelrunners

# IDs for controls
CW_PLAYTEXT_CTRL = 1010
CW_BUTTON_DEFSUB = 1011
CW_BUTTON_SAVE = 1012
CW_BUTTON_COMMENT = 1013
CW_BUTTON_PROPERTIES = 1014
CW_BUTTON_UNDO = 1015

# This isn't actually a control -- it's a "fake" button to trigger
# updates to all relevant windows
CW_BUTTON_UPDATE = 1016

class StatePanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        self.lineups = [ gridlineup.LineupGrid(self, t) for t in [0, 1] ]
        lineupSizer = wxBoxSizer(wxVERTICAL)
        lineupSizer.Add(self.lineups[0], 0, wxALL, 5)
        lineupSizer.Add(self.lineups[1], 0, wxALL, 5)

        stateSizer = wxBoxSizer(wxVERTICAL)
        self.linescore = panellinescore.LinescorePanel(self)
        stateSizer.Add(self.linescore, 0, wxALL | wxEXPAND, 5)

        self.runners = panelrunners.RunnersPanel(self)
        stateSizer.Add(self.runners, 0, wxALL | wxEXPAND, 5)

        playTextSizer = wxBoxSizer(wxHORIZONTAL)
        playTextSizer.Add(FormattedStaticText(self, "Play text"),
                          0, wxALL | wxALIGN_CENTER, 5)
        self.playText = wxTextCtrl(self, CW_PLAYTEXT_CTRL, "",
                                   wxDefaultPosition, wxSize(250, 25),
                                   wxTE_PROCESS_ENTER)
        self.playText.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        playTextSizer.Add(self.playText, 1, wxALL | wxALIGN_CENTER, 5)

        self.playTextValid = FormattedStaticText(self, "")
        self.playTextValid.SetForegroundColour(wxRED)
        playTextSizer.Add(self.playTextValid, 0,
                          wxALL | wxALIGN_CENTER, 5)

        stateSizer.Add(playTextSizer, 0, wxALL | wxEXPAND, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        
        buttonSizer.Add(wxButton(self, CW_BUTTON_DEFSUB,
                                 "Defensive substitution"),
                        0, wxALL, 5)
        self.FindWindowById(CW_BUTTON_DEFSUB).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        buttonSizer.Add(wxButton(self, CW_BUTTON_COMMENT,
                                 "Insert comment"),
                        0, wxALL, 5)
        self.FindWindowById(CW_BUTTON_COMMENT).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        buttonSizer.Add(wxButton(self, CW_BUTTON_UNDO,
                                 "Undo last play"),
                         0, wxALL, 5)
        self.FindWindowById(CW_BUTTON_UNDO).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        stateSizer.Add(buttonSizer, 1, wxALL | wxALIGN_CENTER, 5)

        button2Sizer = wxBoxSizer(wxHORIZONTAL)

        button2Sizer.Add(wxButton(self, CW_BUTTON_PROPERTIES,
                                  "Edit game information"), 
                         0, wxALL, 5)
        self.FindWindowById(CW_BUTTON_PROPERTIES).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        button2Sizer.Add(wxButton(self, CW_BUTTON_SAVE,
                                  "Save game and exit"),
                         0, wxALL, 5)
        self.FindWindowById(CW_BUTTON_SAVE).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        stateSizer.Add(button2Sizer, 1, wxALL | wxALIGN_CENTER, 5)

        sizer = wxBoxSizer(wxHORIZONTAL)
        sizer.Add(lineupSizer, 0, wxALL | wxALIGN_CENTER, 5)
        sizer.Add(stateSizer, 1, wxALL | wxALIGN_CENTER, 5) 

        self.SetSizer(sizer)
        self.Layout()

        # Events propagating up from buttons on runners panel
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[3], self.OnPinchRun3)
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[2], self.OnPinchRun2)
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[1], self.OnPinchRun1)
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[0], self.OnPinchHit)

        # Events from controls on this panel
        EVT_TEXT(self, CW_PLAYTEXT_CTRL, self.OnPlayText)
        EVT_TEXT_ENTER(self, CW_PLAYTEXT_CTRL, self.OnPlayEnter)
        EVT_BUTTON(self, CW_BUTTON_DEFSUB, self.OnDefensiveSub)
        EVT_BUTTON(self, CW_BUTTON_SAVE, self.OnSave)
        EVT_BUTTON(self, CW_BUTTON_COMMENT, self.OnComment)
        EVT_BUTTON(self, CW_BUTTON_PROPERTIES, self.OnProperties)
        EVT_BUTTON(self, CW_BUTTON_UNDO, self.OnUndo)
        
    def SetDocument(self, doc):
        self.doc = doc
        self.linescore.SetDocument(doc)
        self.runners.SetDocument(doc)
        self.lineups[0].SetDocument(doc)
        self.lineups[1].SetDocument(doc)
        self.OnUpdate()

    def OnPlayText(self, event):
        """
        Event handler for change in play text.  Automatically
        uppercases text if it isn't already.  If it is uppercase,
        does nothing.
        """
        x = str(self.playText.GetValue())
        if x != "":
            #self.playTextValid.SetLabel("Valid")
            self.playTextValid.SetForegroundColour(wxColour(0, 150, 0))
        else:
            #self.playTextValid.SetLabel("Invalid")
            self.playTextValid.SetForegroundColour(wxRED)
            event.Skip()
            return
        
        if x.upper() != x:
            y = self.playText.GetInsertionPoint()
            self.playText.SetValue(x.upper())
            self.playText.SetInsertionPoint(y)
        else:
            event.Skip()
            
    def OnPlayEnter(self, event):
        play = str(self.playText.GetValue()).upper()
        if play == "": 
            return
        self.doc.AddPlay(play)

        self.playText.Clear()
        wxPostEvent(self.GetParent(),
                    wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                   CW_BUTTON_UPDATE))

    def OnUndo(self, event):
        self.doc.DeletePlay()
        self.playText.Clear()
        wxPostEvent(self.GetParent(),
                    wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                   CW_BUTTON_UPDATE))

    def OnDefensiveSub(self, event):
        team = 1 - self.doc.GetHalfInning()
        dialog = dialoglineup.LineupDialog(self,
                                           "Defensive lineup for %s %s" %
                                           (self.doc.GetRoster(team).city,
                                            self.doc.GetRoster(team).nickname))

        dialog.LoadRoster(self.doc.GetRoster(team), team)
        dialog.LoadLineup(self.doc, team)

        if dialog.ShowModal() == wxID_OK:
            dialog.WriteChanges(self.doc, team)
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchHit(self, event):
        team = self.doc.GetHalfInning()
        batter = cw_gameiter_get_player(self.doc.gameiter, team,
                                        cw_gameiter_num_batters(self.doc.gameiter, team) % 9 + 1)
        dialog = dialoglineup.PinchDialog(self, "Choose pinch hitter")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wxID_OK:
            dialog.WriteChanges(self.doc, batter,
                                self.doc.GetHalfInning(), 11)
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchRun3(self, event):
        dialog = dialoglineup.PinchDialog(self, "Choose pinch runner at third")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wxID_OK:
            dialog.WriteChanges(self.doc,
                                cw_gameiter_get_runner(self.doc.gameiter, 3),
                                self.doc.GetHalfInning(), 12)
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchRun2(self, event):
        dialog = dialoglineup.PinchDialog(self, "Choose pinch runner at second")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wxID_OK:
            dialog.WriteChanges(self.doc,
                                cw_gameiter_get_runner(self.doc.gameiter, 2),
                                self.doc.GetHalfInning(), 12)
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchRun1(self, event):
        dialog = dialoglineup.PinchDialog(self, "Choose pinch runner at first")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wxID_OK:
            dialog.WriteChanges(self.doc,
                                cw_gameiter_get_runner(self.doc.gameiter, 1),
                                self.doc.GetHalfInning(), 12)
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnSave(self, event):
        if self.doc.GetScore(0) != self.doc.GetScore(1):
            dialog = dialogdecision.DecisionDialog(self, self.doc)
            if dialog.ShowModal() != wxID_OK:  return
            
            cw_game_info_set(self.doc.game, "wp", dialog.GetWinningPitcher())
            cw_game_info_set(self.doc.game, "save", dialog.GetSavePitcher())
            cw_game_info_set(self.doc.game, "lp", dialog.GetLosingPitcher())
            
        for t in [0, 1]:
            for pitcher in self.doc.boxscore.pitching[t]:
                cw_game_set_er(self.doc.game,
                               pitcher["id"], pitcher["er"])
        event.Skip()

    def OnComment(self, event):
        dialog = dialogcomment.CommentDialog(self)
        if dialog.ShowModal() == wxID_OK:
            self.doc.AddComment(dialog.GetComment())
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

            
    def OnProperties(self, event):
        dialog = dialoggame.GamePropertyDialog(self, self.doc)
        if dialog.ShowModal() == wxID_OK:
            dialog.UpdateDocument(self.doc)
            wxPostEvent(self.GetParent(),
                        wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnUpdate(self):
        self.linescore.OnUpdate()
        self.runners.OnUpdate()
        for ctrl in self.lineups: ctrl.OnUpdate()
        
        # Only allow play text entry if defense is valid
        self.playText.Enable(not self.doc.IsGameOver())
        for slot in range(9):
            if self.doc.GetCurrentPosition(1-self.doc.GetHalfInning(),
                                           slot+1) > 10:
                self.playText.Enable(false)
        if self.playText.IsEnabled():
            self.playText.SetFocus()
