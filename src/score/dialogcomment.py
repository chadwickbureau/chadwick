#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog to enter a comment in the game file
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

class CommentDialog(wxDialog):
    def __init__(self, parent):
        wxDialog.__init__(self, parent, -1, "Insert Comment...")

        self.text = wxTextCtrl(self, -1, "", wxDefaultPosition,
                               wxSize(400, -1))

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(self.text, 0, wxALL | wxALIGN_CENTER, 5)
        
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                        0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 1, wxALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)

    def GetComment(self):  return str(self.text.GetValue())
        

