#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# "About" dialog for Chadwick
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
import icons

class AboutDialog(wxDialog):
    def __init__(self, parent):
        wxDialog.__init__(self, parent, -1, "About Chadwick...")

        topSizer = wxBoxSizer(wxVERTICAL)

        bitmap = wxBitmapFromXPMData(icons.chadwick_xpm)
        topSizer.Add(wxStaticBitmap(self, -1, bitmap),
                     0, wxALL | wxALIGN_CENTER, 5)
                     
        ht = FormattedStaticText(self, "Chadwick, version 0.3.0")
        ht.SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD))
        topSizer.Add(ht, 0, wxALL | wxALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "A baseball scorebook application"),
                     0, wxALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "http://chadwick.sourceforge.net"),
                     0, wxALL | wxALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "Copyright (C) 2005"),
                     0, wxTOP | wxALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "Theodore Turocy"),
                     0, wxALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "turocy@econmail.tamu.edu"),
                     0, wxBOTTOM | wxALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "This is free software,"),
                     0, wxTOP | wxALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "distributed under the terms"),
                     0, wxALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "of the GNU General Public License"),
                     0, wxBOTTOM | wxALIGN_CENTER, 5)

        topSizer.Add(wxButton(self, wxID_OK, "OK"),
                     0, wxALL | wxALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)
        
        
