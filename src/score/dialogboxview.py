#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog to display boxscore and game log for a stored game
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

from panelnarrative import NarrativePanel
from panelboxscore import BoxscorePanel

class BoxscoreViewDialog(wxDialog):
    def __init__(self, parent, doc):
        title = (doc.GetRoster(0).GetCity() + " at " + 
                 doc.GetRoster(1).GetCity() + " on " +
                 doc.GetGame().GetDate())
        if doc.GetGame().GetNumber() == 1:
            title += " (first game)"
        elif doc.GetGame().GetNumber() == 2:
            title += " (second game)"

        wxDialog.__init__(self, parent, -1, title,
                          wxDefaultPosition, wxSize(800, 600))

        sizer = wxBoxSizer(wxVERTICAL)

        notebook = wxNotebook(self, -1)

        boxscore = BoxscorePanel(notebook)
        notebook.AddPage(boxscore, "Boxscore")

        narrative = NarrativePanel(notebook)
        notebook.AddPage(narrative, "Narrative")

        doc.BuildBoxscore()
        boxscore.SetDocument(doc)
        narrative.SetDocument(doc)
        
        sizer.Add(notebook, 1, wxALL | wxEXPAND, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()

        
