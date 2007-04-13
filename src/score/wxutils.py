#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Miscellaneous utility routines and classes extending wxWidgets
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

def FormattedStaticText(parent, label, size=wx.DefaultSize,
                        style=wx.ALIGN_CENTER | wx.ST_NO_AUTORESIZE):
    """
    Creates a wxStaticText object with a standard format:
    bold Swiss text and centered by default.
    """
    ctrl = wx.StaticText(parent, wx.ID_STATIC, label,
                         wx.DefaultPosition, size, style)
    ctrl.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
    return ctrl


