#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A customized window for displaying a team's current lineup
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

import wx, wx.grid
from libchadwick import *

class LineupGrid(wx.ScrolledWindow):
    def __init__(self, parent, team):
        wx.ScrolledWindow.__init__(self, parent, wx.ID_ANY, size=(270, 265))

        self.SetBackgroundColour(wx.WHITE)
        self.team = team
        if self.team == 0:
            self.fgColor = wx.RED
        else:
            self.fgColor = wx.BLUE
        self.bgColors = [ wx.Colour(225, 225, 225), wx.WHITE ]
        self.outlineColor = wx.Colour(0, 150, 0)

        wx.EVT_PAINT(self, self.OnPaint)

    def SetDocument(self, doc):
        self.doc = doc
        self.OnUpdate()
        
    def OnPaint(self, event):
        dc = wx.PaintDC(self)
        self.DoPrepareDC(dc)
        self.Draw(dc)

    def DrawCenteredText(self, dc, text, x, y):
        w, h = dc.GetTextExtent(text)
        dc.DrawText(text, x - w/2, y - h/2)

    def DrawVCenteredText(self, dc, text, x, y):
        w, h = dc.GetTextExtent(text)
        dc.DrawText(text, x, y - h/2)

    def Draw(self, dc):
        if not hasattr(self, "doc"):  return

        memdc = wx.MemoryDC()
        bitmap = wx.EmptyBitmap(270, 265)
        memdc.SelectObject(bitmap)
        memdc.Clear()
        positions = [ "", "p", "c", "1b", "2b", "3b", "ss",
                      "lf", "cf", "rf", "dh", "ph", "pr" ]
        
        roster = self.doc.GetRoster(self.team)
        memdc.SetFont(wx.Font(11, wx.SWISS, wx.NORMAL, wx.BOLD))
        memdc.SetPen(wx.Pen(self.outlineColor, 1, wx.SOLID))

        height = 24

        memdc.SetTextForeground(self.fgColor)
        memdc.SetBrush(wx.Brush(wx.Colour(190, 190, 190), wx.SOLID))
        memdc.DrawRectangle(0, 0, 270, height)
        self.DrawCenteredText(memdc, roster.GetName(), 135, height/2)
        for slot in range(10):
            ycoord = (slot+1) * height + height/2

            if slot < 9:
                playerId = self.doc.GetCurrentPlayer(self.team, slot+1)
                if self.doc.gameiter.NumBatters(self.team) % 9 == slot:
                    memdc.SetTextForeground(wx.BLACK)
                else:
                    memdc.SetTextForeground(self.fgColor)
            else:
                playerId = self.doc.GetCurrentPlayer(self.team, 0)
                memdc.SetTextForeground(self.fgColor)
                
            if slot == 9:
                # Only draw the last one if the DH is in use
                if playerId != None:
                    memdc.SetBrush(wx.Brush(wx.Colour(210, 210, 210), wx.SOLID))
                else:
                    continue
            else:
                memdc.SetBrush(wx.Brush(self.bgColors[slot%2], wx.SOLID))

            memdc.DrawRectangle(0, (slot+1) * height, 31, height+1)
            memdc.DrawRectangle(30, (slot+1) * height, 201, height+1)
            memdc.DrawRectangle(230, (slot+1) * height, 40, height+1)

            if slot < 9:
                self.DrawCenteredText(memdc, "%d" % (slot+1),
                                      15, ycoord)
            else:
                self.DrawCenteredText(memdc, "P", 15, ycoord)

            player = roster.FindPlayer(playerId)

            self.DrawVCenteredText(memdc, player.GetName(),
                                   40, ycoord)

            if slot < 9:
                self.DrawCenteredText(memdc, 
                                      positions[self.doc.GetCurrentPosition(self.team, slot+1)],
                                      250, ycoord)
            else:
                self.DrawCenteredText(memdc, "p", 250, ycoord)
                
        dc.Blit(0, 0, 270, 265, memdc, 0, 0)


    def OnUpdate(self):
        self.Draw(wx.ClientDC(self))


