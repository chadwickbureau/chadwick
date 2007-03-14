#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Dialog for showing reports
# 
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2005-2007, Ted Turocy (drarbiter@gmail.com)
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

import wx

class ReportDialog(wx.Dialog):
    def __init__(self, parent, title, contents):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, title, size=(800, 600))

        self.text = wx.TextCtrl(self, wx.ID_ANY, contents,
                                wx.DefaultPosition, wx.DefaultSize,
                                wx.TE_MULTILINE | wx.TE_READONLY | wx.HSCROLL)
        self.text.SetFont(wx.Font(10, wx.MODERN, wx.NORMAL, wx.NORMAL))

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.text, 1, wx.ALL | wx.EXPAND, 5)
        
        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()

