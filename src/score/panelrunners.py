#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel showing the current configuration of runners
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
from libchadwick import *

from wxutils import FormattedStaticText

# IDs for controls
CW_BUTTON_PINCH = [ 1000, 1001, 1002, 1003 ] 

def GetInningLabel(inning, halfInning, outs):
    """
    Generate the text string corresponding to the particular
    inning, half inning, and number of outs.
    """
    if halfInning == 0:
        x = "Top of the "
    else:
        x = "Bottom of the "

    if inning % 10 == 1 and inning != 11:
        x += "%dst, " % inning
    elif inning % 10 == 2 and inning != 12:
        x += "%dnd, " % inning
    elif inning % 10 == 3 and inning != 13:
        x += "%drd, " % inning
    else:
        x += "%dth, " % inning

    x += ["0 outs", "1 out", "2 outs"][outs]
    return x

class RunnersPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        box = wx.StaticBox(self, wx.ID_STATIC, "Current state")
        box.SetBackgroundColour(wx.Colour(0, 150, 0))
        box.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        
        self.inningText = FormattedStaticText(self, "")
        sizer.Add(self.inningText, 0, wx.ALL | wx.EXPAND, 5)

        baseOutSizer = wx.FlexGridSizer(5)

        self.runnerText = [ wx.StaticText(self, wx.ID_STATIC, "",
                                          wx.DefaultPosition,
                                          (200, -1),
                                          wx.ALIGN_CENTER | wx.ST_NO_AUTORESIZE)
                            for i in [0,1,2,3] ]
        for w in self.runnerText:
            w.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 3rd"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[3], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(wx.Button(self, CW_BUTTON_PINCH[3], "Pinch run"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[3]).Enable(False)
        self.FindWindowById(CW_BUTTON_PINCH[3]).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 2nd"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[2], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(wx.Button(self, CW_BUTTON_PINCH[2], "Pinch run"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[2]).Enable(False)
        self.FindWindowById(CW_BUTTON_PINCH[2]).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 1st"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[1], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(wx.Button(self, CW_BUTTON_PINCH[1], "Pinch run"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[1]).Enable(False)
        self.FindWindowById(CW_BUTTON_PINCH[1]).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Batter"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[0], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(wx.Button(self, CW_BUTTON_PINCH[0], "Pinch hit"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[0]).SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))

        sizer.Add(baseOutSizer, 1, wx.ALL | wx.ALIGN_CENTER, 5)
 
        self.SetSizer(sizer)
        self.Layout()
        
    def SetDocument(self, doc):
        self.doc = doc
        self.OnUpdate()
        
    def OnUpdate(self):
        if self.doc.IsGameOver():
            self.inningText.SetLabel("The game is over")
            for ctrl in self.runnerText:
                ctrl.SetLabel("")
            for base in [1,2,3]:
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(False)
            self.FindWindowById(CW_BUTTON_PINCH[0]).Enable(False)
            return
        else:
            self.inningText.SetLabel(GetInningLabel(self.doc.GetInning(),
                                                    self.doc.GetHalfInning(),
                                                    self.doc.GetOuts()))

        for base in [0,1,2,3]:
            if self.doc.GetHalfInning() == 0:
                self.runnerText[base].SetForegroundColour(wx.RED)
            else:
                self.runnerText[base].SetForegroundColour(wx.BLUE)

        playerId = self.doc.GetCurrentBatter()
        player = self.doc.GetRoster(self.doc.GetHalfInning()).FindPlayer(playerId)
        self.runnerText[0].SetLabel(player.GetName())

        self.FindWindowById(CW_BUTTON_PINCH[0]).Enable(True)
        for base in [1,2,3]:
            playerId = self.doc.GetCurrentRunner(base)
            if playerId == "":
                self.runnerText[base].SetLabel("")
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(False)
                continue

            player = self.doc.GetRoster(self.doc.GetHalfInning()).FindPlayer(playerId)

            if player != None and not self.doc.IsLeadoff():
                self.runnerText[base].SetLabel(player.GetName())
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(True)
            else:
                self.runnerText[base].SetLabel("")
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(False)

