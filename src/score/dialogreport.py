#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog for showing reports
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

class ReportDialog(wxDialog):
    def __init__(self, parent, title, contents):
        wxDialog.__init__(self, parent, -1, title,
                          wxDefaultPosition, wxSize(800, 600))

        self.text = wxTextCtrl(self, -1, contents,
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY)
        self.text.SetFont(wxFont(10, wxMODERN, wxNORMAL, wxNORMAL))

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(self.text, 1, wxALL | wxEXPAND, 5)
        
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()

