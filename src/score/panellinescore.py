#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel to display the current linescore of a game
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
from wxutils import FormattedStaticText

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

    if outs == 0:
        x += "0 outs"
    elif outs == 1:
        x += "1 out"
    else:
        x += "2 outs"

    return x

class LinescorePanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)
        
        box = wx.StaticBox(self, wx.ID_STATIC, "Linescore")
        box.SetBackgroundColour(wx.Colour(0, 150, 0))
        box.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        linescoreSizer = wx.FlexGridSizer(3)
        linescoreSizer.AddGrowableCol(0)

        self.teamName = [ FormattedStaticText(self, "", (200, -1),
                                              wx.ALIGN_LEFT)
                          for t in [0, 1] ]
        self.teamName[0].SetForegroundColour(wx.RED)
        self.teamName[1].SetForegroundColour(wx.BLUE)
        self.runsText = [ FormattedStaticText(self, "0", (30, -1))
                          for t in [0, 1] ]
        self.hitsText = [ FormattedStaticText(self, "0", (30, -1))
                          for t in [0, 1] ]
        self.errorsText = [ FormattedStaticText(self, "0", (30, -1))
                            for t in [0, 1] ]
        self.dpText = [ FormattedStaticText(self, "0", (30, -1))
                        for t in [0, 1] ]
        self.lobText = [ FormattedStaticText(self, "0", (30, -1))
                         for t in [0, 1] ]
                            
        for heading in [ "", "R", "H", "E", "DP", "LOB" ]:
            linescoreSizer.Add(FormattedStaticText(self, heading), 
                               0, wx.ALL | wx.ALIGN_CENTER, 5)

        for t in [0, 1]:
            linescoreSizer.Add(self.teamName[t], 1, wx.ALL | wx.ALIGN_LEFT, 5)
            linescoreSizer.Add(self.runsText[t], 0, wx.ALL | wx.ALIGN_CENTER, 5)
            linescoreSizer.Add(self.hitsText[t], 0, wx.ALL | wx.ALIGN_CENTER, 5)
            linescoreSizer.Add(self.errorsText[t], 0, wx.ALL | wx.ALIGN_CENTER, 5)
            linescoreSizer.Add(self.dpText[t], 0, wx.ALL | wx.ALIGN_CENTER, 5)
            linescoreSizer.Add(self.lobText[t], 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(linescoreSizer, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        self.SetSizer(sizer)
        self.Layout()
        
    def SetDocument(self, doc):
        self.doc = doc
        self.OnUpdate()
        
    def OnUpdate(self):
        for t in [0,1]:
            self.teamName[t].SetLabel(self.doc.GetRoster(t).GetName())
            self.runsText[t].SetLabel(str(self.doc.GetScore(t)))
            self.hitsText[t].SetLabel(str(self.doc.GetHits(t)))
            self.errorsText[t].SetLabel(str(self.doc.GetErrors(t)))
            self.dpText[t].SetLabel(str(self.doc.GetDoublePlays(t)))
            self.lobText[t].SetLabel(str(self.doc.GetLOB(t)))
        

