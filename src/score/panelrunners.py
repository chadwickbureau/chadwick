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

from wxutils import FormattedStaticText

import game


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
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent)
        self.doc = doc
        
        box = wx.StaticBox(self, wx.ID_STATIC, "Current state")
        box.SetBackgroundColour(wx.Colour(0, 150, 0))
        box.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        
        self.inningText = FormattedStaticText(self, "")
        sizer.Add(self.inningText, 0, wx.ALL | wx.EXPAND, 5)

        baseOutSizer = wx.GridSizer(rows=4, cols=2)

        self.runnerText = [ [ wx.Choice(self, size=(200,-1))
                              for i in [0,1,2,3] ] for t in [ 0, 1 ] ]


        baseOutSizer.Add(FormattedStaticText(self, "Runner on 3rd"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[0][3], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        self.runnerText[1][3].Show(False)
        
        for t in [ 0, 1 ]:
            self.Bind(wx.EVT_CHOICE, lambda event: self.OnPinchRun(event, 3),
                      self.runnerText[t][3])

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 2nd"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[0][2], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        self.runnerText[1][2].Show(False)
        
        for t in [ 0, 1 ]:
            self.Bind(wx.EVT_CHOICE, lambda event: self.OnPinchRun(event, 2),
                      self.runnerText[t][2])

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 1st"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[0][1], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        self.runnerText[1][1].Show(False)

        for t in [ 0, 1 ]:
            self.Bind(wx.EVT_CHOICE, lambda event: self.OnPinchRun(event, 1),
                      self.runnerText[t][1])

        baseOutSizer.Add(FormattedStaticText(self, "Batter"),
                         0, wx.ALL | wx.ALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[0][0], 1, wx.ALL | wx.ALIGN_CENTER, 5)
        self.runnerText[1][0].Show(False)
        
        for t in [ 0, 1 ]:
            self.Bind(wx.EVT_CHOICE, self.OnPinchHit, self.runnerText[t][0])

        self.lastHalfInning = 0
        
        sizer.Add(baseOutSizer, 1, wx.ALL | wx.ALIGN_CENTER, 5)
 
        self.SetSizer(sizer)
        self.Layout()
        
        for t in [ 0, 1 ]:
            names = [ player.GetSortName()
                      for player in self.doc.GetRoster(t).Players() ]
            for ctrl in self.runnerText[t]:
                ctrl.Clear()
                ctrl.AppendItems(names)

        self.OnUpdate()

    def OnPinchHit(self, event):
        team = self.doc.GetHalfInning()
        batter = self.doc.GetState().GetPlayer(team,
                                               self.doc.GetState().NumBatters(team) % 9 + 1)

        sub = [x for x in self.doc.GetRoster(self.doc.GetHalfInning()).Players()][event.GetSelection()]

        if batter != sub.GetID():
            slot = self.doc.GetState().GetSlot(team, batter)
            self.doc.AddSubstitute(sub, team, slot, 11)
            wx.PostEvent(self.GetParent(),
                         game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))

    def OnPinchRun(self, event, base):
        team = self.doc.GetHalfInning()
        runner = self.doc.GetState().GetRunner(base)

        sub = [x for x in self.doc.GetRoster(self.doc.GetHalfInning()).Players()][event.GetSelection()]
        
        if runner != sub.GetID():
            slot = self.doc.GetState().GetSlot(team, runner)
            self.doc.AddSubstitute(sub, team, slot, 12)
            wx.PostEvent(self.GetParent(),
                         game.GameUpdateEvent(self.GetId(), gameDoc=self.doc))
        
        
    def OnUpdate(self):
        if self.doc.IsGameOver():
            self.inningText.SetLabel("The game is over")
            for t in self.runnerText:
                for ctrl in t:
                    ctrl.Show(False)
            return
        else:
            self.inningText.SetLabel(GetInningLabel(self.doc.GetInning(),
                                                    self.doc.GetHalfInning(),
                                                    self.doc.GetOuts()))

        team = self.doc.GetHalfInning()
        roster = self.doc.GetRoster(team)

        if self.lastHalfInning != self.doc.GetHalfInning():
            for base in [0, 1, 2, 3]:
                self.GetSizer().Replace(self.runnerText[self.lastHalfInning][base],
                                        self.runnerText[1-self.lastHalfInning][base],
                                        True)
                self.runnerText[self.lastHalfInning][base].Show(False)
                self.runnerText[1-self.lastHalfInning][base].Show(True)
            self.lastHalfInning = self.doc.GetHalfInning()
            self.Layout()
        
        playerId = self.doc.GetCurrentBatter()
        player = roster.FindPlayer(playerId)
        self.runnerText[team][0].SetStringSelection(player.GetSortName())
        self.runnerText[team][0].SetFont(wx.Font(10, wx.SWISS,
                                                 wx.NORMAL, wx.BOLD))
        
        for (base, ctrl) in zip([1,2,3], self.runnerText[team][1:]):
            playerId = self.doc.GetCurrentRunner(base)
            if playerId == "":
                ctrl.Show(False)
                continue

            player = roster.FindPlayer(playerId)

            if player != None and not self.doc.IsLeadoff():
                ctrl.SetStringSelection(player.GetSortName())
                ctrl.Show(True)
                ctrl.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
            else:
                ctrl.Show(False)

        for ctrl in self.runnerText[team]:
            if self.doc.GetHalfInning() == 0:
                ctrl.SetForegroundColour(wx.RED)
            else:
                ctrl.SetForegroundColour(wx.BLUE)

