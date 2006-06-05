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

import wx
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
CW_PITCHES_CTRL = 1009
CW_PLAYTEXT_CTRL = 1010
CW_BUTTON_DEFSUB = 1011
CW_BUTTON_SAVE = 1012
CW_BUTTON_COMMENT = 1013
CW_BUTTON_PROPERTIES = 1014
CW_BUTTON_UNDO = 1015

# This isn't actually a control -- it's a "fake" button to trigger
# updates to all relevant windows
CW_BUTTON_UPDATE = 1016


def GetCount(pitches):
    """
    Returns the count associated with a pitch sequence.
    Note that the last pitch is not counted.
    """

    b = 0
    s = 0
    for c in pitches[:-1]:
        if c in [ 'B', 'I', 'P', 'V' ]:
            b += 1
        elif c in [ 'C', 'F', 'K', 'L', 'M', 'O', 'Q',
                    'R', 'S', 'T' ]:
            s += 1
    return "%d%d" % (b,s)

class StatePanel(wx.Panel):
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        self.lineups = [ gridlineup.LineupGrid(self, t) for t in [0, 1] ]
        lineupSizer = wx.BoxSizer(wx.VERTICAL)
        lineupSizer.Add(self.lineups[0], 0, wx.ALL, 5)
        lineupSizer.Add(self.lineups[1], 0, wx.ALL, 5)

        stateSizer = wx.BoxSizer(wx.VERTICAL)
        self.linescore = panellinescore.LinescorePanel(self)
        stateSizer.Add(self.linescore, 0, wx.ALL | wx.EXPAND, 5)

        self.runners = panelrunners.RunnersPanel(self)
        stateSizer.Add(self.runners, 0, wx.ALL | wx.EXPAND, 5)

        if doc.GetGame().GetInfo("pitches") == "pitches":
            pitchesSizer = wx.BoxSizer(wx.HORIZONTAL)
            pitchesSizer.Add(FormattedStaticText(self, "Pitches"),
                             0, wx.ALL | wx.ALIGN_CENTER, 5)

            self.pitches = wx.TextCtrl(self, CW_PITCHES_CTRL, "",
                                       size=(150, -1))
            self.pitches.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
            pitchesSizer.Add(self.pitches, 0, wx.ALL | wx.ALIGN_CENTER, 5)
            stateSizer.Add(pitchesSizer, 0, wx.ALL | wx.EXPAND, 5)

        playTextSizer = wx.BoxSizer(wx.HORIZONTAL)
        playTextSizer.Add(FormattedStaticText(self, "Play text"),
                          0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.playText = wx.TextCtrl(self, CW_PLAYTEXT_CTRL, "",
                                   wx.DefaultPosition, wx.Size(250, 25),
                                   wx.TE_PROCESS_ENTER)
        self.playText.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        # playText starts out empty, which is invalid...
        self.playText.SetBackgroundColour(wx.NamedColour("pink"))
        playTextSizer.Add(self.playText, 1, wx.ALL | wx.ALIGN_CENTER, 5)

        stateSizer.Add(playTextSizer, 0, wx.ALL | wx.EXPAND, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        
        buttonSizer.Add(wx.Button(self, CW_BUTTON_DEFSUB,
                                 "Defensive substitution"),
                        0, wx.ALL, 5)
        self.FindWindowById(CW_BUTTON_DEFSUB).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        buttonSizer.Add(wx.Button(self, CW_BUTTON_COMMENT,
                                 "Insert comment"),
                        0, wx.ALL, 5)
        self.FindWindowById(CW_BUTTON_COMMENT).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        buttonSizer.Add(wx.Button(self, CW_BUTTON_UNDO,
                                 "Undo last play"),
                         0, wx.ALL, 5)
        self.FindWindowById(CW_BUTTON_UNDO).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        stateSizer.Add(buttonSizer, 1, wx.ALL | wx.ALIGN_CENTER, 5)

        button2Sizer = wx.BoxSizer(wx.HORIZONTAL)

        button2Sizer.Add(wx.Button(self, CW_BUTTON_PROPERTIES,
                                  "Edit game information"), 
                         0, wx.ALL, 5)
        self.FindWindowById(CW_BUTTON_PROPERTIES).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        button2Sizer.Add(wx.Button(self, CW_BUTTON_SAVE,
                                  "Save game and exit"),
                         0, wx.ALL, 5)
        self.FindWindowById(CW_BUTTON_SAVE).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        stateSizer.Add(button2Sizer, 1, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(lineupSizer, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(stateSizer, 1, wx.ALL | wx.ALIGN_CENTER, 5) 

        self.SetSizer(sizer)
        self.Layout()

        # Events propagating up from buttons on runners panel
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[3], self.OnPinchRun3)
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[2], self.OnPinchRun2)
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[1], self.OnPinchRun1)
        EVT_BUTTON(self, panelrunners.CW_BUTTON_PINCH[0], self.OnPinchHit)

        # Events from controls on this panel
        EVT_TEXT(self, CW_PITCHES_CTRL, self.OnPitchesText)
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

    def OnPitchesText(self, event):
        x = str(self.pitches.GetValue())
        if x.upper() != x:
            y = self.pitches.GetInsertionPoint()
            self.pitches.SetValue(x.upper())
            self.pitches.SetInsertionPoint(y)

    def OnPlayText(self, event):
        """
        Event handler for change in play text.  Automatically
        uppercases text if it isn't already.  If it is uppercase,
        does nothing.
        """
        x = str(self.playText.GetValue())
        if x.upper() != x:
            y = self.playText.GetInsertionPoint()
            self.playText.SetValue(x.upper())
            self.playText.SetInsertionPoint(y)

        if IsValidPlay(x.upper()):
            self.playText.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOW))
        else:
            self.playText.SetBackgroundColour(wx.NamedColour("pink"))
        
            
    def OnPlayEnter(self, event):
        play = str(self.playText.GetValue()).upper()
        if play == "" or not IsValidPlay(play):
            wx.Bell()
        else:
            if hasattr(self, "pitches"):
                pitches = str(self.pitches.GetValue())
                self.doc.AddPlay(GetCount(pitches), pitches, play)
                self.pitches.Clear()
            else:
                self.doc.AddPlay("??", "", play)

            self.playText.Clear()
            self.playText.SetBackgroundColour(wx.NamedColour("pink"))
            
            wx.PostEvent(self.GetParent(),
                        wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnUndo(self, event):
        self.doc.DeletePlay()
        self.playText.Clear()
        wx.PostEvent(self.GetParent(),
                    wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                   CW_BUTTON_UPDATE))

    def OnDefensiveSub(self, event):
        team = 1 - self.doc.GetHalfInning()
        dialog = dialoglineup.LineupDialog(self,
                                           "Defensive lineup for %s" %
                                           self.doc.GetRoster(team).GetName())

        hasDH = (self.doc.GetState().GetPlayer(team, 0) != None)

        dialog.LoadRoster(self.doc.GetRoster(team), team, hasDH)
        dialog.LoadLineup(self.doc, team)

        if dialog.ShowModal() == wx.ID_OK:
            dialog.WriteChanges(self.doc, team)
            wx.PostEvent(self.GetParent(),
                        wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchHit(self, event):
        team = self.doc.GetHalfInning()
        batter = self.doc.gameiter.GetPlayer(team,
                                             self.doc.gameiter.NumBatters(team) % 9 + 1)
        dialog = dialoglineup.PinchDialog(self, "Choose pinch hitter")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wx.ID_OK:
            dialog.WriteChanges(self.doc, batter,
                                self.doc.GetHalfInning(), 11)
            wx.PostEvent(self.GetParent(),
                        wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchRun3(self, event):
        dialog = dialoglineup.PinchDialog(self, "Choose pinch runner at third")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wx.ID_OK:
            dialog.WriteChanges(self.doc,
                                self.doc.gameiter.GetRunner(3),
                                self.doc.GetHalfInning(), 12)
            wx.PostEvent(self.GetParent(),
                        wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchRun2(self, event):
        dialog = dialoglineup.PinchDialog(self, "Choose pinch runner at second")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wx.ID_OK:
            dialog.WriteChanges(self.doc,
                                self.doc.gameiter.GetRunner(2),
                                self.doc.GetHalfInning(), 12)
            wx.PostEvent(self.GetParent(),
                        wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnPinchRun1(self, event):
        dialog = dialoglineup.PinchDialog(self, "Choose pinch runner at first")
        dialog.LoadRoster(self.doc.GetRoster(self.doc.GetHalfInning()),
                          self.doc.GetHalfInning())

        if dialog.ShowModal() == wx.ID_OK:
            dialog.WriteChanges(self.doc,
                                self.doc.gameiter.GetRunner(1),
                                self.doc.GetHalfInning(), 12)
            wx.PostEvent(self.GetParent(),
                        wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                       CW_BUTTON_UPDATE))

    def OnSave(self, event):
        if self.doc.GetScore(0) != self.doc.GetScore(1):
            dialog = dialogdecision.DecisionDialog(self, self.doc)
            if dialog.ShowModal() != wx.ID_OK:  return
            
            self.doc.game.SetInfo("wp", dialog.GetWinningPitcher())
            self.doc.game.SetInfo("save", dialog.GetSavePitcher())
            self.doc.game.SetInfo("lp", dialog.GetLosingPitcher())
            
        for t in [0, 1]:
            for pitcher in self.doc.boxscore.pitching[t]:
                self.doc.game.SetER(pitcher["id"], pitcher["er"])
        event.Skip()

    def OnComment(self, event):
        dialog = dialogcomment.CommentDialog(self)
        if dialog.ShowModal() == wx.ID_OK:
            self.doc.AddComment(dialog.GetComment())
            wx.PostEvent(self.GetParent(),
                         wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                         CW_BUTTON_UPDATE))

            
    def OnProperties(self, event):
        dialog = dialoggame.GamePropertyDialog(self, self.doc)
        if dialog.ShowModal() == wx.ID_OK:
            dialog.UpdateDocument(self.doc)
            wx.PostEvent(self.GetParent(),
                         wx.CommandEvent(wx.EVT_COMMAND_BUTTON_CLICKED,
                                         CW_BUTTON_UPDATE))

    def OnUpdate(self):
        self.linescore.OnUpdate()
        self.runners.OnUpdate()
        for ctrl in self.lineups: ctrl.OnUpdate()
        
        # Only allow play text entry if defense is valid
        self.playText.Enable(not self.doc.IsGameOver())
        if hasattr(self, "pitches"):
            self.pitches.Enable(not self.doc.IsGameOver())

        for slot in range(9):
            if self.doc.GetCurrentPosition(1-self.doc.GetHalfInning(),
                                           slot+1) > 10:
                self.playText.Enable(False)
                if hasattr(self, "pitches"):
                    self.pitches.Enable(False)
                    
        if hasattr(self, "pitches") and self.pitches.IsEnabled():
            self.pitches.SetFocus()
        elif self.playText.IsEnabled():
            self.playText.SetFocus()
