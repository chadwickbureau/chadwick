#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog to award decisions in a game
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
from wxutils import FormattedStaticText

class DecisionDialog(wxDialog):
    def __init__(self, parent, doc):
        wxDialog.__init__(self, parent, -1, "Award Decisions...")
        self.doc = doc
        
        if doc.GetScore(0) > doc.GetScore(1):
            winner = 0
        else:
            winner = 1

        fgColors = [ wxRED, wxBLUE ]
        
        self.wps = [ x for x in doc.boxscore.pitching[winner] ]
        self.lps = [ x for x in doc.boxscore.pitching[1-winner] ]
        
        sizer = wxBoxSizer(wxVERTICAL)
        
        gridSizer = wxFlexGridSizer(3)

        gridSizer.Add(FormattedStaticText(self, "Winning pitcher"),
                      0, wxALL | wxALIGN_CENTER, 5)
        self.wp = wxChoice(self, -1, wxDefaultPosition, wxSize(300, -1))
        for x in self.wps:  self.wp.Append(x["name"])
        self.wp.SetForegroundColour(fgColors[winner])
        self.wp.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        gridSizer.Add(self.wp, 0, wxALL | wxALIGN_CENTER, 5)
        
        gridSizer.Add(FormattedStaticText(self, "Save"),
                      0, wxALL | wxALIGN_CENTER, 5)
        self.save = wxChoice(self, -1, wxDefaultPosition, wxSize(300, -1))
        if len(self.wps) == 1:
            self.save.Enable(false)
        else:
            self.save.Append("(none)")
            for x in self.wps: self.save.Append(x["name"])
            self.save.SetSelection(0)
        self.save.SetForegroundColour(fgColors[winner])
        self.save.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        gridSizer.Add(self.save, 0, wxALL | wxALIGN_CENTER, 5)

        gridSizer.Add(FormattedStaticText(self, "Losing pitcher"),
                      0, wxALL | wxALIGN_CENTER, 5)
        self.lp = wxChoice(self, -1, wxDefaultPosition, wxSize(300, -1))
        for x in self.lps: self.lp.Append(x["name"])
        self.lp.SetForegroundColour(fgColors[1-winner])
        self.lp.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        gridSizer.Add(self.lp, 0, wxALL | wxALIGN_CENTER, 5)

        sizer.Add(gridSizer, 0, wxALL, 5)
        
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                        0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 1, wxALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)
        
    def GetWinningPitcher(self):
        return self.wps[self.wp.GetSelection()]["id"]
        
    def GetSavePitcher(self):
        if self.save.IsEnabled() == false or self.save.GetSelection() == 0:
            return ""
        else:
            return self.wps[self.save.GetSelection()-1]["id"]
            
    def GetLosingPitcher(self):
        return self.lps[self.lp.GetSelection()]["id"]
        
        

