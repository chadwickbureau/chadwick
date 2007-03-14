#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A dialog box for setting and editing a lineup
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
from libchadwick import *

def FormattedStaticText(parent, label, size=wx.DefaultSize,
                        style=wx.ALIGN_CENTER | wx.ST_NO_AUTORESIZE):
    ctrl = wx.StaticText(parent, wx.ID_STATIC, label,
                         wx.DefaultPosition, size, style)
    ctrl.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD))
    return ctrl

class WeatherPanel(wx.Panel):
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent, wx.ID_ANY)
        
        box = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_STATIC, "Weather"),
                                wx.VERTICAL)
        grid = wx.FlexGridSizer(5)
        grid.AddGrowableCol(1)
        grid.Add(FormattedStaticText(self, "Temperature"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.temp = wx.TextCtrl(self, wx.ID_ANY,
                                doc.GetGame().GetInfo("temp"))
        grid.Add(self.temp, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        grid.Add(FormattedStaticText(self, "Wind direction"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.windDir = wx.Choice(self, -1, wx.DefaultPosition, wx.DefaultSize,
                                 [ "Unknown",
                                   "Out to left", "Out to center",
                                   "Out to right",
                                   "Left to right", "Right to left",
                                   "In from left", "In from center",
                                   "In from right" ])
        winddirs = [ "unknown", "tolf", "tocf", "torf",
                     "ltor", "rtol", "fromlf", "fromcf", "fromrf" ]
        self.windDir.SetSelection(winddirs.index(doc.GetGame().GetInfo("winddir")))
        grid.Add(self.windDir, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Wind speed"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.windSpeed = wx.TextCtrl(self, wx.ID_ANY,
                                     doc.GetGame().GetInfo("windspeed"))
        grid.Add(self.windSpeed, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        
        grid.Add(FormattedStaticText(self, "Sky"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        skyList = [ "Unknown", "Sunny", "Cloudy",
                    "Overcast", "Night", "Dome" ]
        self.sky = wx.Choice(self, wx.ID_ANY,
                             wx.DefaultPosition, wx.DefaultSize,
                             skyList)
        skyCond = doc.GetGame().GetInfo("sky")
        self.sky.SetSelection(0)
        for sky in skyList:
            if sky.lower() == skyCond.lower():
                self.sky.SetStringSelection(sky)
                break
        grid.Add(self.sky, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Field"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        fieldList = [ "Unknown", "Dry", "Damp", "Wet", "Soaked" ]
        self.field = wx.Choice(self, wx.ID_ANY,
                               wx.DefaultPosition, wx.DefaultSize,
                               fieldList)
        fieldCond = doc.GetGame().GetInfo("fieldcond")
        self.field.SetSelection(0)
        for field in fieldList:
            if field.lower() == fieldCond.lower():
                self.field.SetStringSelection(field)
                break
        grid.Add(self.field, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        box.Add(grid, 0, wx.ALL, 5)
        
        self.SetSizer(box)
        self.Layout()

    def UpdateDocument(self, doc):
        doc.GetGame().SetInfo("temp", str(self.temp.GetValue()))
        doc.GetGame().SetInfo("sky",
                              str(self.sky.GetStringSelection()).lower())
        doc.GetGame().SetInfo("fieldcond",
                              str(self.field.GetStringSelection()).lower())
        doc.GetGame().SetInfo("windspeed",
                              str(self.windSpeed.GetValue()))
        winddirs = [ "unknown", "tolf", "tocf", "torf",
                     "ltor", "rtol", "fromlf", "fromcf", "fromrf" ]
        doc.GetGame().SetInfo("winddir",
                              winddirs[self.windDir.GetSelection()])
        

class ScorerPanel(wx.Panel):
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        box = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_STATIC, "Scorers"),
                                wx.VERTICAL)

        grid = wx.FlexGridSizer(4)
        grid.AddGrowableCol(1)
        grid.Add(FormattedStaticText(self, "Scorer"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.scorer = wx.TextCtrl(self, wx.ID_ANY,
                                  doc.GetGame().GetInfo("scorer"))
        grid.Add(self.scorer, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        
        grid.Add(FormattedStaticText(self, "Translator"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.translator = wx.TextCtrl(self, wx.ID_ANY,
                                      doc.GetGame().GetInfo("translator"))
        grid.Add(self.translator, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Inputter"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.inputter = wx.TextCtrl(self, wx.ID_ANY,
                                    doc.GetGame().GetInfo("inputter"))
        grid.Add(self.inputter, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "How scored"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        scoredList = [ "Unknown", "Park", "Radio", "TV" ]
        self.howScored = wx.Choice(self, wx.ID_ANY,
                                   wx.DefaultPosition, wx.DefaultSize,
                                   scoredList)
        scored = doc.GetGame().GetInfo("howscored")
        self.howScored.SetSelection(0)
        for how in scoredList:
            if how.lower() == scored.lower():
                self.howScored.SetStringSelection(how)
                break
        grid.Add(self.howScored, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        
        box.Add(grid, 0, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(box)
        self.Layout()

    def UpdateDocument(self, doc):
        doc.GetGame().SetInfo("scorer", str(self.scorer.GetValue()))
        doc.GetGame().SetInfo("translator",
                              str(self.translator.GetValue()))
        doc.GetGame().SetInfo("inputter",
                              str(self.inputter.GetValue()))
        doc.GetGame().SetInfo("howscored",
                              str(self.howScored.GetStringSelection()).lower())


class UmpirePanel(wx.Panel):
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        box = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_STATIC, "Umpires"),
                                wx.VERTICAL)

        grid = wx.FlexGridSizer(4)
        grid.AddGrowableCol(1)

        grid.Add(FormattedStaticText(self, "Home plate"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.home = wx.TextCtrl(self, wx.ID_ANY,
                                doc.GetGame().GetInfo("umphome"))
        grid.Add(self.home, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "First base"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.first = wx.TextCtrl(self, wx.ID_ANY,
                                 doc.GetGame().GetInfo("ump1b"))
        grid.Add(self.first, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Second base"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.second = wx.TextCtrl(self, wx.ID_ANY,
                                 doc.GetGame().GetInfo("ump2b"))
        grid.Add(self.second, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Third base"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.third = wx.TextCtrl(self, wx.ID_ANY,
                                 doc.GetGame().GetInfo("ump3b"))
        grid.Add(self.third, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)
        
        box.Add(grid, 0, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(box)
        self.Layout()

    def UpdateDocument(self, doc):
        doc.GetGame().SetInfo("umphome", str(self.home.GetValue()))
        doc.GetGame().SetInfo("ump1b", str(self.first.GetValue()))
        doc.GetGame().SetInfo("ump2b", str(self.second.GetValue()))
        doc.GetGame().SetInfo("ump3b", str(self.third.GetValue()))

class GeneralPanel(wx.Panel):
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        box = wx.StaticBoxSizer(wx.StaticBox(self, wx.ID_STATIC, "General"),
                                wx.VERTICAL)

        grid = wx.FlexGridSizer(4)
        grid.AddGrowableCol(1)

        grid.Add(FormattedStaticText(self, "Start time"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.startTime = wx.TextCtrl(self, wx.ID_ANY,
                                    doc.GetGame().GetInfo("starttime"))
        grid.Add(self.startTime, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Day/night"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        dayNightList = [ "Unknown", "Day", "Night" ]
        self.dayNight = wx.Choice(self, wx.ID_ANY,
                                  wx.DefaultPosition, wx.DefaultSize,
                                  dayNightList)
        day = doc.GetGame().GetInfo("daynight")
        self.dayNight.SetSelection(0)
        for entry in dayNightList:
            if entry.lower() == day.lower():
                self.dayNight.SetStringSelection(entry)
                break
        grid.Add(self.dayNight, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Time of game"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.timeOfGame = wx.TextCtrl(self, wx.ID_ANY,
                                      doc.GetGame().GetInfo("timeofgame"))
        grid.Add(self.timeOfGame, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Attendance"),
                 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.attendance = wx.TextCtrl(self, wx.ID_ANY,
                                      doc.GetGame().GetInfo("attendance"))
        grid.Add(self.attendance, 0, wx.ALL | wx.EXPAND | wx.ALIGN_CENTER, 5)

        box.Add(grid, 0, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(box)
        self.Layout()

    def UpdateDocument(self, doc):
        doc.GetGame().SetInfo("starttime",
                              str(self.startTime.GetValue()))
        doc.GetGame().SetInfo("daynight",
                              str(self.dayNight.GetStringSelection()).lower())
        doc.GetGame().SetInfo("timeofgame",
                              str(self.timeOfGame.GetValue()))
        doc.GetGame().SetInfo("attendance",
                              str(self.attendance.GetValue()))

class GamePropertyDialog(wx.Dialog):
    def __init__(self, parent, doc):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "Game properties")
        
        sizer = wx.BoxSizer(wx.VERTICAL)

        horizSizer = wx.BoxSizer(wx.HORIZONTAL)

        vertSizer = wx.BoxSizer(wx.VERTICAL)
        self.general = GeneralPanel(self, doc)
        vertSizer.Add(self.general, 0, wx.ALL | wx.EXPAND, 5)
        self.umpires = UmpirePanel(self, doc)
        vertSizer.Add(self.umpires, 0, wx.ALL | wx.EXPAND, 5)
        horizSizer.Add(vertSizer, 0, wx.ALL | wx.EXPAND, 5)

        vertSizer = wx.BoxSizer(wx.VERTICAL)
        self.weather = WeatherPanel(self, doc)
        vertSizer.Add(self.weather, 0, wx.ALL | wx.EXPAND, 5)
        self.scorer = ScorerPanel(self, doc)
        vertSizer.Add(self.scorer, 0, wx.ALL | wx.EXPAND, 5)
        horizSizer.Add(vertSizer, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(horizSizer, 0, wx.ALL | wx.EXPAND, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"), 0,
                        wx.ALL | wx.ALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)
        
        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)
        
    def UpdateDocument(self, doc):
        self.general.UpdateDocument(doc)
        self.umpires.UpdateDocument(doc)
        self.scorer.UpdateDocument(doc)
        self.weather.UpdateDocument(doc)
