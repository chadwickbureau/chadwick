#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel showing the current configuration of runners
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

class RunnersPanel(wxPanel):
    def __init__(self, parent):
        wxPanel.__init__(self, parent, -1)

        box = wxStaticBox(self, wxID_STATIC, "Current state")
        box.SetBackgroundColour(wxColour(0, 150, 0))
        box.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        sizer = wxStaticBoxSizer(box, wxVERTICAL)
        
        self.inningText = FormattedStaticText(self, "")
        sizer.Add(self.inningText, 0, wxALL | wxEXPAND, 5)

        baseOutSizer = wxFlexGridSizer(5)

        self.runnerText = [ wxStaticText(self, wxID_STATIC, "",
                                         wxDefaultPosition,
                                         wxSize(200, -1),
                                         wxALIGN_CENTER | wxST_NO_AUTORESIZE)
                            for i in [0,1,2,3] ]
        for w in self.runnerText:
            w.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 3rd"),
                         0, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[3], 1, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(wxButton(self, CW_BUTTON_PINCH[3], "Pinch run"),
                         0, wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[3]).Enable(false)
        self.FindWindowById(CW_BUTTON_PINCH[3]).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 2nd"),
                         0, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[2], 1, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(wxButton(self, CW_BUTTON_PINCH[2], "Pinch run"),
                         0, wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[2]).Enable(false)
        self.FindWindowById(CW_BUTTON_PINCH[2]).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Runner on 1st"),
                         0, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[1], 1, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(wxButton(self, CW_BUTTON_PINCH[1], "Pinch run"),
                         0, wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[1]).Enable(false)
        self.FindWindowById(CW_BUTTON_PINCH[1]).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        baseOutSizer.Add(FormattedStaticText(self, "Batter"),
                         0, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(self.runnerText[0], 1, wxALL | wxALIGN_CENTER, 5)
        baseOutSizer.Add(wxButton(self, CW_BUTTON_PINCH[0], "Pinch hit"),
                         0, wxALL | wxALIGN_CENTER, 5)
        self.FindWindowById(CW_BUTTON_PINCH[0]).SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))

        sizer.Add(baseOutSizer, 1, wxALL | wxALIGN_CENTER, 5)
 
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
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(false)
            self.FindWindowById(CW_BUTTON_PINCH[0]).Enable(false)
            return
        else:
            self.inningText.SetLabel(GetInningLabel(self.doc.GetInning(),
                                                    self.doc.GetHalfInning(),
                                                    self.doc.GetOuts()))

        for base in [0,1,2,3]:
            if self.doc.GetHalfInning() == 0:
                self.runnerText[base].SetForegroundColour(wxRED)
            else:
                self.runnerText[base].SetForegroundColour(wxBLUE)

        playerId = self.doc.GetCurrentBatter()
        player = cw_roster_player_find(self.doc.GetRoster(self.doc.GetHalfInning()), playerId)
        self.runnerText[0].SetLabel(player.first_name + " " + player.last_name)

        for base in [1,2,3]:
            playerId = self.doc.GetCurrentRunner(base)
            if playerId == "":
                self.runnerText[base].SetLabel("")
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(false)
                continue

            player = cw_roster_player_find(self.doc.GetRoster(self.doc.GetHalfInning()), playerId)

            if player != None and not self.doc.IsLeadoff():
                self.runnerText[base].SetLabel(player.first_name + " " + player.last_name)
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(true)
            else:
                self.runnerText[base].SetLabel("")
                self.FindWindowById(CW_BUTTON_PINCH[base]).Enable(false)

