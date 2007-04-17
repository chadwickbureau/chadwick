#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# "About" dialog for Chadwick
# 
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2002-2007, Ted Turocy (drarbiter@gmail.com)
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

from wxutils import FormattedStaticText
import icons

class AboutDialog(wx.Dialog):
    def __init__(self, parent):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "About Chadwick...")

        topSizer = wx.BoxSizer(wx.VERTICAL)

        bitmap = wx.BitmapFromXPMData(icons.chadwick_xpm)
        topSizer.Add(wx.StaticBitmap(self, wx.ID_ANY, bitmap),
                     0, wx.ALL | wx.ALIGN_CENTER, 5)
                     
        ht = FormattedStaticText(self, "Chadwick, version 0.4.0")
        ht.SetFont(wx.Font(14, wx.SWISS, wx.NORMAL, wx.BOLD))
        topSizer.Add(ht, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "A baseball scorebook application"),
                     0, wx.ALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "http://chadwick.sourceforge.net"),
                     0, wx.ALL | wx.ALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "Copyright (C) 2005-2007"),
                     0, wx.TOP | wx.ALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "Theodore Turocy"),
                     0, wx.ALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "drarbiter@gmail.com"),
                     0, wx.BOTTOM | wx.ALIGN_CENTER, 5)

        topSizer.Add(FormattedStaticText(self, "This is free software,"),
                     0, wx.TOP | wx.ALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "distributed under the terms"),
                     0, wx.ALIGN_CENTER, 5)
        topSizer.Add(FormattedStaticText(self, "of the GNU General Public License"),
                     0, wx.BOTTOM | wx.ALIGN_CENTER, 5)

        topSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                     0, wx.ALL | wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)
        
        
