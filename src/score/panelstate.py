#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Implementation of panel showing the current state of a game
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
import libchadwick as cw

# Import of dialogs used
import dialoggame
import dialoglineup

# Import of controls comprising this panel
from wxutils import FormattedStaticText, LEDCtrl
import gridlineup
import panellinescore
import panelrunners

import game

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
        wx.Panel.__init__(self, parent)
        self.doc = doc

        self.lineups = [ gridlineup.LineupGrid(self, doc, t) for t in [0, 1] ]
        lineupSizer = wx.BoxSizer(wx.VERTICAL)
        lineupSizer.Add(self.lineups[0], 0, wx.ALL, 5)
        lineupSizer.Add(self.lineups[1], 0, wx.ALL, 5)

        stateSizer = wx.BoxSizer(wx.VERTICAL)
        self.linescore = panellinescore.LinescorePanel(self, doc)
        stateSizer.Add(self.linescore, 0, wx.ALL | wx.EXPAND, 5)

        self.runners = panelrunners.RunnersPanel(self, doc)
        stateSizer.Add(self.runners, 0, wx.ALL | wx.EXPAND, 5)

        if doc.GetGame().GetInfo("pitches") == "pitches":
            pitchesSizer = wx.BoxSizer(wx.HORIZONTAL)
            pitchesSizer.Add(FormattedStaticText(self, "Pitches"),
                             0, wx.ALL | wx.ALIGN_CENTER, 5)

            self.pitches = wx.TextCtrl(self, size=(150, -1))
            self.pitches.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
            self.Bind(wx.EVT_TEXT, self.OnPitchesText, self.pitches)
            pitchesSizer.Add(self.pitches, 0, wx.ALL | wx.ALIGN_CENTER, 5)
            stateSizer.Add(pitchesSizer, 0, wx.ALL | wx.EXPAND, 5)

        playTextSizer = wx.BoxSizer(wx.HORIZONTAL)
        playTextSizer.Add(FormattedStaticText(self, "Play text"),
                          0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.playText = wx.TextCtrl(self, size=(250, 25),
                                    style=wx.TE_PROCESS_ENTER)
        self.playText.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.Bind(wx.EVT_TEXT, self.OnPlayText, self.playText)
        self.Bind(wx.EVT_TEXT_ENTER, self.OnPlayEnter, self.playText)
        playTextSizer.Add(self.playText, 1, wx.ALL | wx.ALIGN_CENTER, 5)

        self.ledCtrl = LEDCtrl(self, size=(25, 25))
        # playText starts out empty
        self.ledCtrl.SetColor(wx.WHITE)
        self.ledCtrl.SetToolTipString("The play string is empty.")
        playTextSizer.Add(self.ledCtrl, 0,
                          wx.LEFT | wx.RIGHT | wx.ALIGN_CENTER, 0)
        
        stateSizer.Add(playTextSizer, 0, wx.ALL | wx.EXPAND, 5)

        commentSizer = wx.BoxSizer(wx.HORIZONTAL)
        commentSizer.Add(FormattedStaticText(self, "Comment"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.comment = wx.TextCtrl(self, size=(300, 25),
                                   style=wx.TE_PROCESS_ENTER)
        self.comment.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.Bind(wx.EVT_TEXT_ENTER, self.OnCommentEnter, self.comment)
        commentSizer.Add(self.comment, 1, wx.ALL | wx.ALIGN_CENTER, 5)
        stateSizer.Add(commentSizer, 0, wx.ALL | wx.EXPAND, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)

        button = wx.Button(self, label="Defensive substitution")
        button.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        buttonSizer.Add(button, 0, wx.ALL, 5)
        self.Bind(wx.EVT_BUTTON, self.OnDefensiveSub, button)

        button = wx.Button(self, label="Undo last play")
        button.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        buttonSizer.Add(button, 0, wx.ALL, 5)
        self.Bind(wx.EVT_BUTTON, self.OnUndo, button)

        button = wx.Button(self, label="Edit game info")
        button.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        buttonSizer.Add(button, 0, wx.ALL, 5)
        self.Bind(wx.EVT_BUTTON, self.OnProperties, button)

        stateSizer.Add(buttonSizer, 1, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(lineupSizer, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(stateSizer, 1, wx.ALL | wx.ALIGN_CENTER, 5) 

        self.SetSizer(sizer)
        self.Layout()
        
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

        if cw.IsValidPlay(x.upper()):
            self.ledCtrl.SetColor(wx.GREEN)
            self.ledCtrl.SetToolTipString("The play string is valid.")

        else:
            self.ledCtrl.SetColor(wx.RED)
            self.ledCtrl.SetToolTipString("The play string is not valid.")
        
            
    def OnPlayEnter(self, event):
        play = str(self.playText.GetValue()).upper()
        if play == "" or not cw.IsValidPlay(play):
            wx.Bell()
        else:
            if hasattr(self, "pitches"):
                pitches = str(self.pitches.GetValue())
                self.doc.AddPlay(GetCount(pitches), pitches, play)
                self.pitches.Clear()
            else:
                self.doc.AddPlay("??", "", play)

            self.playText.Clear()
            self.ledCtrl.SetColor(wx.WHITE)
            self.ledCtrl.SetToolTipString("The play string is empty.")
            
            wx.PostEvent(self.GetParent(),
                         game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))

    def OnCommentEnter(self, event):
        self.doc.AddComment(str(self.comment.GetValue()).replace('"', "'").replace("\n", " ").replace("\r", " "))
        self.comment.SetValue("")
        wx.PostEvent(self.GetParent(),
                     game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))
        

    def OnUndo(self, event):
        self.doc.DeletePlay()
        self.playText.Clear()
        wx.PostEvent(self.GetParent(),
                     game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))

    def OnDefensiveSub(self, event):
        team = 1 - self.doc.GetHalfInning()
        dialog = dialoglineup.LineupDialog(self,
                                           "Defensive lineup for %s" %
                                           self.doc.GetRoster(team).GetName())

        hasDH = (self.doc.GetState().GetPlayer(team, 0) != None)

        dialog.LoadRoster(self.doc.GetScorebook(),
                          self.doc.GetRoster(team), team, hasDH)
        dialog.LoadLineup(self.doc, team)

        if dialog.ShowModal() == wx.ID_OK:
            dialog.WriteChanges(self.doc, team)
            wx.PostEvent(self.GetParent(),
                         game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))

    def OnProperties(self, event):
        dialog = dialoggame.GamePropertyDialog(self, self.doc)
        if dialog.ShowModal() == wx.ID_OK:
            dialog.UpdateDocument(self.doc)
            wx.PostEvent(self.GetParent(),
                         game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))

    def OnUpdate(self):
        self.linescore.OnUpdate()
        self.runners.OnUpdate()
        for ctrl in self.lineups: ctrl.OnUpdate()
        
        # Only allow play text entry if defense is valid
        self.playText.Enable(not self.doc.IsGameOver())
        if hasattr(self, "pitches"):
            self.pitches.Enable(not self.doc.IsGameOver())


        for slot in range(9):
            pos = self.doc.GetCurrentPosition(1-self.doc.GetHalfInning(),
                                              slot+1)
            if pos == 0 or pos > 10:
                self.playText.Enable(False)
                self.ledCtrl.SetColor(wx.Colour(255, 255, 0))
                self.ledCtrl.SetToolTipString("The defensive alignment is incomplete.")
                if hasattr(self, "pitches"):
                    self.pitches.Enable(False)
                return
            
        if hasattr(self, "pitches") and self.pitches.IsEnabled():
            self.pitches.SetFocus()
        elif self.playText.IsEnabled():
            self.playText.SetFocus()

        if self.playText.GetValue() == "":
            self.ledCtrl.SetColor(wx.WHITE)
            self.ledCtrl.SetToolTipString("The play string is empty.")
        elif cw.IsValidPlay(self.playText.GetValue()):
            self.ledCtrl.SetColor(wx.GREEN)
            self.ledCtrl.SetToolTipString("The play string is valid.")
        else:
            self.ledCtrl.SetColor(wx.RED)
            self.ledCtrl.SetToolTipString("The play string is not valid.")
