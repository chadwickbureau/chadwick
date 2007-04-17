#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog to display boxscore and game log for a stored game
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

from panelnarrative import NarrativePanel
from panelboxscore import BoxscorePanel

class BoxscoreViewDialog(wx.Dialog):
    def __init__(self, parent, doc):
        title = (doc.GetRoster(0).GetCity() + " at " + 
                 doc.GetRoster(1).GetCity() + " on " +
                 doc.GetGame().GetDate())
        if doc.GetGame().GetNumber() == 1:
            title += " (first game)"
        elif doc.GetGame().GetNumber() == 2:
            title += " (second game)"

        wx.Dialog.__init__(self, parent, wx.ID_ANY, title, size=(800, 600))

        sizer = wx.BoxSizer(wx.VERTICAL)

        notebook = wx.Notebook(self, wx.ID_ANY)

        boxscore = BoxscorePanel(notebook)
        notebook.AddPage(boxscore, "Boxscore")

        narrative = NarrativePanel(notebook)
        notebook.AddPage(narrative, "Narrative")

        doc.BuildBoxscore()
        boxscore.SetDocument(doc)
        narrative.SetDocument(doc)
        
        sizer.Add(notebook, 1, wx.ALL | wx.EXPAND, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"), 0,
                        wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()

        
