#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Top-level application classes for Chadwick graphical interface
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
from wxPython.grid import *
import string

from libchadwick import *
import scorebook
import panelstate

from gameeditor import GameEditor, CreateGame
from frameentry import GameEntryFrame
from panelgamelist import *
from panelplayerlist import PlayerListPanel
from panelteamlist import TeamListPanel

from dialognewgame import NewGameDialog
from dialoglineup import LineupDialog
from dialogreport import ReportDialog
from dialogabout import AboutDialog

import statscan

from wxutils import FormattedStaticText

class YearDialog(wxDialog):
    def __init__(self, parent):
        wxDialog.__init__(self, parent, -1, "New scorebook")

        topSizer = wxBoxSizer(wxVERTICAL)
        
        yearSizer = wxBoxSizer(wxHORIZONTAL)
        yearSizer.Add(FormattedStaticText(self, "Year"),
                      0, wxALL | wxALIGN_CENTER, 5)
                      
        self.year = wxTextCtrl(self, -1, "2005",
                               wxDefaultPosition, wxSize(125, -1))
        yearSizer.Add(self.year, 0, wxALL | wxALIGN_CENTER, 5)
        topSizer.Add(yearSizer, 0, wxALL, 5)
        
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_CANCEL, "Cancel"),
                                 0, wxALL | wxALIGN_CENTER, 5)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        topSizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        EVT_TEXT(self, self.year.GetId(), self.OnChangeYear)
        
    def OnChangeYear(self, event):
        try:
            if int(str(self.year.GetValue())) > 0:
                self.FindWindowById(wxID_OK).Enable(true)
            else:
                self.FindWindowById(wxID_OK).Enable(false)
        except ValueError:
            self.FindWindowById(wxID_OK).Enable(false)
    
    def GetYear(self):  return int(str(self.year.GetValue()))

# IDs for our menu command events
CW_MENU_REPORT_BATTING = 2010
CW_MENU_REPORT_PITCHING = 2011
CW_MENU_REPORT_FIELDING = 2012
CW_MENU_REPORT_TEAM = 2013

class ChadwickFrame(wxFrame):
    def __init__(self, parent):
        wxFrame.__init__(self, parent, -1, "Chadwick",
                         wxDefaultPosition, wxSize(800, 600))
        self.book = scorebook.ChadwickScorebook()

        self.MakeMenus()
        self.CreateStatusBar()

        sizer = wxBoxSizer(wxVERTICAL)
        
        notebook = wxNotebook(self, -1)
        self.gameList = GameListPanel(notebook)
        notebook.AddPage(self.gameList, "Games")
        
        self.teamList = TeamListPanel(notebook)
        notebook.AddPage(self.teamList, "Teams")

        self.playerList = PlayerListPanel(notebook)
        notebook.AddPage(self.playerList, "Players")
        
        sizer.Add(notebook, 1, wxEXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()

        EVT_MENU(self, wxID_NEW, self.OnFileNew)
        EVT_MENU(self, wxID_OPEN, self.OnFileOpen)
        EVT_MENU_RANGE(self, wxID_FILE1, wxID_FILE9, self.OnFileMRU)
        EVT_MENU(self, wxID_SAVE, self.OnFileSave)
        EVT_MENU(self, wxID_SAVEAS, self.OnFileSaveAs)
        EVT_MENU(self, wxID_EXIT, self.OnFileExit)
        EVT_MENU(self, wxID_ABOUT, self.OnHelpAbout)
        EVT_BUTTON(self, CW_MENU_GAME_NEW, self.OnGameNew)
        EVT_MENU(self, CW_MENU_REPORT_BATTING, self.OnReportBatting)
        EVT_MENU(self, CW_MENU_REPORT_PITCHING, self.OnReportPitching)
        EVT_MENU(self, CW_MENU_REPORT_FIELDING, self.OnReportFielding)
        EVT_MENU(self, CW_MENU_REPORT_TEAM, self.OnReportTeam)
        EVT_BUTTON(self, panelstate.CW_BUTTON_SAVE, self.OnGameSave)
        EVT_CLOSE(self, self.OnClickClose)

        self.OnUpdate()


    def MakeMenus(self):
        fileMenu = wxMenu()
        fileMenu.Append(wxID_NEW, "&New", "Create a new scorebook")
        fileMenu.Append(wxID_OPEN, "&Open", "Open a saved scorebook")
        fileMenu.Append(wxID_SAVE, "&Save", "Save the current scorebook")
        fileMenu.Append(wxID_SAVEAS, "Save &as",
                        "Save the scorebook to a different file")
        fileMenu.AppendSeparator()
        fileMenu.Append(wxID_EXIT, "&Exit", "Close Chadwick")

        self.fileHistory = wxFileHistory()
        self.fileHistory.Load(wxConfig("Chadwick"))
        self.fileHistory.UseMenu(fileMenu)
        self.fileHistory.AddFilesToMenu()
        
        reportMenu = wxMenu()
        reportMenu.Append(CW_MENU_REPORT_BATTING, "Batting",
                          "Show batting statistics")
        reportMenu.Append(CW_MENU_REPORT_PITCHING, "Pitching",
                          "Show pitching statistics")
        reportMenu.Append(CW_MENU_REPORT_FIELDING, "Fielding",
                          "Show fielding statistics")
        reportMenu.Append(CW_MENU_REPORT_TEAM, "Team",
                          "Show team statistics")
        
        helpMenu = wxMenu()
        helpMenu.Append(wxID_ABOUT, "&About", "About Chadwick")

        menuBar = wxMenuBar()
        menuBar.Append(fileMenu, "&File")
        menuBar.Append(reportMenu, "&Report")
        menuBar.Append(helpMenu, "&Help")

        self.SetMenuBar(menuBar)

    def CheckUnsaved(self):
        """
        Returns True if it's OK to close a scorebook, either because
        it's not modified, or the user wants to abandon changes.
        """
        if self.book.IsModified() == False:  return True

        dialog = wxMessageDialog(self,
                                 "There are unsaved changes to "
                                 "the scorebook.  Continue?",
                                 "Warning: unsaved changes",
                                 wxOK | wxCANCEL | wxICON_EXCLAMATION)
        result = dialog.ShowModal()
        dialog.Destroy()
        return result

    def OnFileNew(self, event):
        if not self.CheckUnsaved():  return

        dialog = YearDialog(self)
        if dialog.ShowModal() == wxID_OK:
            self.book = scorebook.ChadwickScorebook(dialog.GetYear())
            self.OnUpdate()

    def OnFileOpen(self, event):
        if not self.CheckUnsaved():  return
        
        dialog = wxFileDialog(self, "Scorebook to open...",
                              "", "",
                              "Chadwick scorebooks (*.chw)|*.chw|"
                              "Retrosheet zipfiles (*.zip)|*.zip|"
                              "All files (*.*)|*.*")
        if dialog.ShowModal() == wxID_OK:
            try:
                book = scorebook.ChadwickScorebook()
                book.Read(str(dialog.GetPath()))
                self.book = book
                self.OnUpdate()
                self.fileHistory.AddFileToHistory(dialog.GetPath())
            except:
                dialog = wxMessageDialog(self,
                                         "An error occurred in reading "
                                         + str(dialog.GetPath()),
                                         "Error opening scorebook",
                                         wxOK | wxICON_ERROR)
                dialog.ShowModal()

    def OnFileMRU(self, event):
        if not self.CheckUnsaved():  return
        
        self.book = scorebook.ChadwickScorebook()
        self.book.Read(str(self.fileHistory.GetHistoryFile(event.GetId() - wxID_FILE1)))
        self.OnUpdate()
            
    def OnFileSave(self, event):
        try:
            self.book.Write(self.book.GetFilename())
            self.OnUpdate()
        except:
            dialog = wxMessageDialog(self,
                                     "An error occurred in writing "
                                     + self.book.GetFilename(),
                                     "Error saving scorebook",
                                     wxOK | wxICON_ERROR)
            dialog.ShowModal()


    def OnFileSaveAs(self, event):
        dialog = wxFileDialog(self, "Scorebook to save...",
                              "", "",
                              "Chadwick scorebooks (*.chw)|*.chw|"
                              "All files (*.*)|*.*",
                              wxSAVE | wxOVERWRITE_PROMPT)
        if dialog.ShowModal() == wxID_OK:
            try:
                self.book.Write(str(dialog.GetPath()))
                self.fileHistory.AddFileToHistory(dialog.GetPath())
                self.OnUpdate()
            except:
                dialog = wxMessageDialog(self,
                                         "An error occurred in writing "
                                         + str(dialog.GetPath()),
                                         "Error saving scorebook",
                                         wxOK | wxICON_ERROR)
                dialog.ShowModal()

    def OnFileExit(self, event):
        if not self.CheckUnsaved():  return
        self.fileHistory.Save(wxConfig("Chadwick"))
        global app
        app.ExitMainLoop()
        
    def OnClickClose(self, event):
        if not self.CheckUnsaved():  return
        self.fileHistory.Save(wxConfig("Chadwick"))
        global app
        app.ExitMainLoop()

    def OnHelpAbout(self, event):
        dialog = AboutDialog(self)
        dialog.ShowModal()
        
    def OnGameNew(self, event):
        dialog = NewGameDialog(self, self.book)
        if dialog.ShowModal() != wxID_OK:
            return
            
        rosters = [ self.book.GetTeam(dialog.GetTeam(t)) for t in [0, 1] ]

        game = CreateGame(dialog.GetGameId(),
                          rosters[0].team_id, rosters[1].team_id)
        doc = GameEditor(game, rosters[0], rosters[1])

        for t in [0, 1]:
            dialog = LineupDialog(self, 
                                  "Starting Lineup for %s" % 
                                  (rosters[t].city + " " + rosters[t].nickname))
            dialog.LoadRoster(doc.GetRoster(t), t, True)
            if dialog.ShowModal() != wxID_OK:
                return
                
            for slot in range(9):
                player = dialog.GetPlayerInSlot(slot+1)
                doc.SetStarter(player.player_id,
                               player.first_name + " " + player.last_name,
                               t, slot+1, dialog.GetPositionInSlot(slot+1))

            if dialog.HasDH():
                player = dialog.GetPlayerInSlot(10)
                doc.SetStarter(player.player_id,
                               player.first_name + " " + player.last_name,
                               t, 0, 1)
                cw_game_info_set(doc.GetGame(), "usedh", "true")
                
                                
        doc.BuildBoxscore()
        self.entryFrame = GameEntryFrame(self) 
        self.entryFrame.SetDocument(doc)
        self.entryFrame.Show(true)
        
    def OnGameSave(self, event):
        self.book.AddGame(self.entryFrame.GetDocument().GetGame())
        self.entryFrame.Destroy()
        del self.entryFrame
        self.OnUpdate()

    def OnReportBatting(self, event):
        bs = statscan.BattingAccumulator()
        busyInfo = wxBusyInfo("Generating batting statistics... Please be patient!")
        statscan.ProcessFile(self.book, [ bs ])
        del busyInfo

        dialog = ReportDialog(self, "Batting statistics",
                              str(bs))
        dialog.ShowModal()

    def OnReportPitching(self, event):
        ps = statscan.PitchingAccumulator()
        busyInfo = wxBusyInfo("Generating pitching statistics... Please be patient!")
        statscan.ProcessFile(self.book, [ ps ])
        del busyInfo

        dialog = ReportDialog(self, "Pitching statistics",
                              str(ps))
        dialog.ShowModal()

    def OnReportFielding(self, event):
        fs = [ statscan.FieldingAccumulator(p+1) for p in range(9) ]
        busyInfo = wxBusyInfo("Generating fielding statistics... Please be patient!")
        statscan.ProcessFile(self.book, fs)
        del busyInfo

        dialog = ReportDialog(self, "Fielding statistics",
                              string.join([ str(x) for x in fs ], "\n\n"))
        dialog.ShowModal()
            
    def OnReportTeam(self, event):
        acc = [ statscan.RecordAccumulator(self.book),
                statscan.TeamBattingAccumulator(self.book),
                statscan.TeamPitchingAccumulator(self.book),
                statscan.TeamFieldingAccumulator(self.book) ]
        busyInfo = wxBusyInfo("Generating team statistics... Please be patient!")
        statscan.ProcessFile(self.book, acc)
        del busyInfo

        dialog = ReportDialog(self, "Team statistics",
                              string.join([ str(x) for x in acc ], "\n\n"))
        dialog.ShowModal()

    def OnUpdate(self):
        title = "Chadwick: [%s] %d" % (self.book.GetFilename(),
                                       self.book.GetYear())
        if self.book.IsModified():
            title += " (unsaved changes)"
            
        self.SetTitle(title)
        self.teamList.OnUpdate(self.book)
        self.gameList.OnUpdate(self.book)
        self.playerList.OnUpdate(self.book)

        
class ChadwickApp(wxApp):
    def OnInit(self):
        frame = ChadwickFrame(NULL)
        frame.Show(true)
        self.SetTopWindow(frame)

        return true

    def GetSeasonPath(self):   return "/home/arbiter/sports/CL/"

app = ChadwickApp(0)
app.MainLoop()
