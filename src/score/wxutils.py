#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Miscellaneous utility routines and classes extending wxWidgets
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

def FormattedStaticText(parent, label, size=wxDefaultSize,
                        style=wxALIGN_CENTER | wxST_NO_AUTORESIZE):
    """
    Creates a wxStaticText object with a standard format:
    bold Swiss text and centered by default.
    """
    ctrl = wxStaticText(parent, wxID_STATIC, label,
                        wxDefaultPosition, size, style)
    ctrl.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
    return ctrl


