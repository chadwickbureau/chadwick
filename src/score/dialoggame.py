#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# A dialog box for setting and editing a lineup
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

def FormattedStaticText(parent, label, size=wxDefaultSize,
                        style=wxALIGN_CENTER | wxST_NO_AUTORESIZE):
    ctrl = wxStaticText(parent, wxID_STATIC, label,
                        wxDefaultPosition, size, style)
    ctrl.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
    return ctrl

class WeatherPanel(wxPanel):
    def __init__(self, parent, doc):
        wxPanel.__init__(self, parent, -1)
        
        box = wxStaticBoxSizer(wxStaticBox(self, wxID_STATIC,
                                      "Weather"),
                               wxVERTICAL)
        grid = wxFlexGridSizer(5)
        grid.AddGrowableCol(1)
        grid.Add(FormattedStaticText(self, "Temperature"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.temp = wxTextCtrl(self, -1,
                               doc.GetGame().GetInfo("temp"))
        grid.Add(self.temp, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)
        grid.Add(FormattedStaticText(self, "Wind direction"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.windDir = wxChoice(self, -1, wxDefaultPosition, wxDefaultSize,
                                [ "Unknown",
                                  "Out to left", "Out to center", "Out to right",
                                  "Left to right", "Right to left",
                                  "In from left", "In from center", "In from right" ])
        winddirs = [ "unknown", "tolf", "tocf", "torf",
                     "ltor", "rtol", "fromlf", "fromcf", "fromrf" ]
        self.windDir.SetSelection(winddirs.index(doc.GetGame().GetInfo("winddir")))
        grid.Add(self.windDir, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Wind speed"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.windSpeed = wxTextCtrl(self, -1,
                                    doc.GetGame().GetInfo("windspeed"))
        grid.Add(self.windSpeed, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)
        
        grid.Add(FormattedStaticText(self, "Sky"),
                 0, wxALL | wxALIGN_CENTER, 5)
        skyList = [ "Unknown", "Sunny", "Cloudy",
                    "Overcast", "Night", "Dome" ]
        self.sky = wxChoice(self, -1, wxDefaultPosition, wxDefaultSize,
                            skyList)
        skyCond = doc.GetGame().GetInfo("sky")
        self.sky.SetSelection(0)
        for sky in skyList:
            if sky.lower() == skyCond.lower():
                self.sky.SetStringSelection(sky)
                break
        grid.Add(self.sky, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Field"),
                 0, wxALL | wxALIGN_CENTER, 5)
        fieldList = [ "Unknown", "Dry", "Damp", "Wet", "Soaked" ]
        self.field = wxChoice(self, -1,
                              wxDefaultPosition, wxDefaultSize,
                              fieldList)
        fieldCond = doc.GetGame().GetInfo("fieldcond")
        self.field.SetSelection(0)
        for field in fieldList:
            if field.lower() == fieldCond.lower():
                self.field.SetStringSelection(field)
                break
        grid.Add(self.field, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        box.Add(grid, 0, wxALL, 5)
        
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
        

class ScorerPanel(wxPanel):
    def __init__(self, parent, doc):
        wxPanel.__init__(self, parent, -1)

        box = wxStaticBoxSizer(wxStaticBox(self, wxID_STATIC, "Scorers"),
                               wxVERTICAL)

        grid = wxFlexGridSizer(4)
        grid.AddGrowableCol(1)
        grid.Add(FormattedStaticText(self, "Scorer"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.scorer = wxTextCtrl(self, -1,
                                 doc.GetGame().GetInfo("scorer"))
        grid.Add(self.scorer, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)
        
        grid.Add(FormattedStaticText(self, "Translator"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.translator = wxTextCtrl(self, -1,
                                     doc.GetGame().GetInfo("translator"))
        grid.Add(self.translator, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Inputter"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.inputter = wxTextCtrl(self, -1,
                                   doc.GetGame().GetInfo("inputter"))
        grid.Add(self.inputter, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "How scored"),
                 0, wxALL | wxALIGN_CENTER, 5)
        scoredList = [ "Unknown", "Park", "Radio", "TV" ]
        self.howScored = wxChoice(self, -1,
                                  wxDefaultPosition, wxDefaultSize,
                                  scoredList)
        scored = doc.GetGame().GetInfo("howscored")
        self.howScored.SetSelection(0)
        for how in scoredList:
            if how.lower() == scored.lower():
                self.howScored.SetStringSelection(how)
                break
        grid.Add(self.howScored, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)
        
        box.Add(grid, 0, wxALL | wxEXPAND, 5)

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


class UmpirePanel(wxPanel):
    def __init__(self, parent, doc):
        wxPanel.__init__(self, parent, -1)

        box = wxStaticBoxSizer(wxStaticBox(self, wxID_STATIC, "Umpires"),
                               wxVERTICAL)

        grid = wxFlexGridSizer(4)
        grid.AddGrowableCol(1)

        grid.Add(FormattedStaticText(self, "Home plate"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.home = wxTextCtrl(self, -1,
                               doc.GetGame().GetInfo("umphome"))
        grid.Add(self.home, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "First base"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.first = wxTextCtrl(self, -1,
                                doc.GetGame().GetInfo("ump1b"))
        grid.Add(self.first, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Second base"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.second = wxTextCtrl(self, -1,
                                 doc.GetGame().GetInfo("ump2b"))
        grid.Add(self.second, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Third base"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.third = wxTextCtrl(self, -1,
                                doc.GetGame().GetInfo("ump3b"))
        grid.Add(self.third, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        box.Add(grid, 0, wxALL | wxEXPAND, 5)

        self.SetSizer(box)
        self.Layout()

    def UpdateDocument(self, doc):
        doc.GetGame().SetInfo("umphome", str(self.home.GetValue()))
        doc.GetGame().SetInfo("ump1b", str(self.first.GetValue()))
        doc.GetGame().SetInfo("ump2b", str(self.second.GetValue()))
        doc.GetGame().SetInfo("ump3b", str(self.third.GetValue()))

class GeneralPanel(wxPanel):
    def __init__(self, parent, doc):
        wxPanel.__init__(self, parent, -1)

        box = wxStaticBoxSizer(wxStaticBox(self, wxID_STATIC, "General"),
                               wxVERTICAL)

        grid = wxFlexGridSizer(4)
        grid.AddGrowableCol(1)

        grid.Add(FormattedStaticText(self, "Start time"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.startTime = wxTextCtrl(self, -1,
                                    doc.GetGame().GetInfo("starttime"))
        grid.Add(self.startTime, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Day/night"),
                 0, wxALL | wxALIGN_CENTER, 5)
        dayNightList = [ "Unknown", "Day", "Night" ]
        self.dayNight = wxChoice(self, -1,
                                 wxDefaultPosition, wxDefaultSize,
                                 dayNightList)
        day = doc.GetGame().GetInfo("daynight")
        self.dayNight.SetSelection(0)
        for entry in dayNightList:
            if entry.lower() == day.lower():
                self.dayNight.SetStringSelection(entry)
                break
        grid.Add(self.dayNight, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Time of game"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.timeOfGame = wxTextCtrl(self, -1,
                                     doc.GetGame().GetInfo("timeofgame"))
        grid.Add(self.timeOfGame, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        grid.Add(FormattedStaticText(self, "Attendance"),
                 0, wxALL | wxALIGN_CENTER, 5)
        self.attendance = wxTextCtrl(self, -1,
                                     doc.GetGame().GetInfo("attendance"))
        grid.Add(self.attendance, 0, wxALL | wxEXPAND | wxALIGN_CENTER, 5)

        box.Add(grid, 0, wxALL | wxEXPAND, 5)

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

class GamePropertyDialog(wxDialog):
    def __init__(self, parent, doc):
        wxDialog.__init__(self, parent, -1, "Game properties")
        
        sizer = wxBoxSizer(wxVERTICAL)

        horizSizer = wxBoxSizer(wxHORIZONTAL)

        vertSizer = wxBoxSizer(wxVERTICAL)
        self.general = GeneralPanel(self, doc)
        vertSizer.Add(self.general, 0, wxALL | wxEXPAND, 5)
        self.umpires = UmpirePanel(self, doc)
        vertSizer.Add(self.umpires, 0, wxALL | wxEXPAND, 5)
        horizSizer.Add(vertSizer, 0, wxALL | wxEXPAND, 5)

        vertSizer = wxBoxSizer(wxVERTICAL)
        self.weather = WeatherPanel(self, doc)
        vertSizer.Add(self.weather, 0, wxALL | wxEXPAND, 5)
        self.scorer = ScorerPanel(self, doc)
        vertSizer.Add(self.scorer, 0, wxALL | wxEXPAND, 5)
        horizSizer.Add(vertSizer, 0, wxALL | wxEXPAND, 5)

        sizer.Add(horizSizer, 0, wxALL | wxEXPAND, 5)

        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)
        
        self.SetSizer(sizer)
        self.Layout()
        sizer.SetSizeHints(self)
        
    def UpdateDocument(self, doc):
        self.general.UpdateDocument(doc)
        self.umpires.UpdateDocument(doc)
        self.scorer.UpdateDocument(doc)
        self.weather.UpdateDocument(doc)
