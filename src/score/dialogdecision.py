#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog to award decisions in a game
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

class DecisionDialog(wx.Dialog):
    def __init__(self, parent, doc):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Award Decisions...")
        self.doc = doc
        
        if doc.GetScore(0) > doc.GetScore(1):
            winner = 0
        else:
            winner = 1

        fgColors = [ wx.RED, wx.BLUE ]
        
        self.wps = [ x for x in doc.boxscore.pitching[winner] ]
        self.lps = [ x for x in doc.boxscore.pitching[1-winner] ]
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        
        gridSizer = wx.FlexGridSizer(3)

        gridSizer.Add(FormattedStaticText(self, "Winning pitcher"),
                      0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.wp = wx.Choice(self, wx.ID_ANY, size=(300, -1))
        for x in self.wps:  self.wp.Append(x["name"])
        self.wp.SetForegroundColour(fgColors[winner])
        self.wp.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        gridSizer.Add(self.wp, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        
        gridSizer.Add(FormattedStaticText(self, "Save"),
                      0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.save = wx.Choice(self, wx.ID_ANY, size=(300, -1))
        if len(self.wps) == 1:
            self.save.Enable(False)
        else:
            self.save.Append("(none)")
            for x in self.wps: self.save.Append(x["name"])
            self.save.SetSelection(0)
        self.save.SetForegroundColour(fgColors[winner])
        self.save.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        gridSizer.Add(self.save, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        gridSizer.Add(FormattedStaticText(self, "Losing pitcher"),
                      0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.lp = wx.Choice(self, wx.ID_ANY, size=(300, -1))
        for x in self.lps: self.lp.Append(x["name"])
        self.lp.SetForegroundColour(fgColors[1-winner])
        self.lp.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
        gridSizer.Add(self.lp, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        sizer.Add(gridSizer, 0, wx.ALL, 5)
        
        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"), 0,
                        wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 1, wx.ALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)
        
    def GetWinningPitcher(self):
        return self.wps[self.wp.GetSelection()]["id"]
        
    def GetSavePitcher(self):
        if not self.save.IsEnabled() or self.save.GetSelection() == 0:
            return ""
        else:
            return self.wps[self.save.GetSelection()-1]["id"]
            
    def GetLosingPitcher(self):
        return self.lps[self.lp.GetSelection()]["id"]
        
        

