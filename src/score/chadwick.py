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
from dialognewgame import NewGameDialog
from dialoglineup import LineupDialog

from dialogboxview import BoxscoreViewDialog

import statscan


class TeamListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 4)
        self.SetColLabelValue(0, "Team ID")
        self.SetColLabelValue(1, "City")
        self.SetColLabelValue(2, "Nickname")
        self.SetColLabelValue(3, "League ID")
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.EnableEditing(false)

    def OnUpdate(self, book):
        if self.GetNumberRows() > book.NumTeams():
            self.DeleteRows(self.GetNumberRows() - book.NumTeams())
        elif self.GetNumberRows() < book.NumTeams():
            self.InsertRows(0, book.NumTeams() - self.GetNumberRows())

        for (i,team) in enumerate(book.IterateTeams()):
            self.SetCellValue(i, 0, team.team_id)
            self.SetCellAlignment(i, 0, wxALIGN_CENTER, wxALIGN_CENTER)
            self.SetCellValue(i, 1, team.city)
            self.SetCellValue(i, 2, team.nickname)
            self.SetCellValue(i, 3, team.league)
            self.SetCellAlignment(i, 3, wxALIGN_CENTER, wxALIGN_CENTER)
            
        self.AutoSizeRows()
        self.AutoSizeColumns()
        self.AdjustScrollbars()


class GameListGrid(wxGrid):
    def __init__(self, parent):
        wxGrid.__init__(self, parent, -1)
        self.CreateGrid(0, 4)
        self.SetColLabelValue(0, "Game ID")
        self.SetColLabelValue(1, "Date")
        self.SetColLabelValue(2, "Visitors")
        self.SetColLabelValue(3, "Home")
        self.SetDefaultCellFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
        self.EnableEditing(false)

        EVT_GRID_CELL_LEFT_DCLICK(self, self.OnLeftDoubleClick)

    def OnUpdate(self, book):
        self.book = book
        if self.GetNumberRows() > book.NumGames():
            self.DeleteRows(self.GetNumberRows() - book.NumGames())
        elif self.GetNumberRows() < book.NumGames():
            self.InsertRows(0, book.NumGames() - self.GetNumberRows())

        for (i,game) in enumerate(book.IterateGames()):
            self.SetCellValue(i, 0, game.game_id)
            self.SetCellAlignment(i, 0, wxALIGN_CENTER, wxALIGN_CENTER)
            self.SetCellValue(i, 1, cw_game_info_lookup(game, "date"))
            self.SetCellAlignment(i, 1, wxALIGN_CENTER, wxALIGN_CENTER)
            self.SetCellValue(i, 2, cw_game_info_lookup(game, "visteam"))
            self.SetCellAlignment(i, 2, wxALIGN_CENTER, wxALIGN_CENTER)
            self.SetCellValue(i, 3, cw_game_info_lookup(game, "hometeam"))
            self.SetCellAlignment(i, 3, wxALIGN_CENTER, wxALIGN_CENTER)

        self.AutoSizeRows()
        self.AutoSizeColumns()
        self.AdjustScrollbars()

    def OnLeftDoubleClick(self, event):
        for game in self.book.IterateGames():
            if game.game_id == str(self.GetCellValue(event.GetRow(), 0)):
                vis = str(self.GetCellValue(event.GetRow(), 2))
                home = str(self.GetCellValue(event.GetRow(), 3))
                doc = GameEditor(game,
                                 self.book.GetTeam(vis),
                                 self.book.GetTeam(home))
                dialog = BoxscoreViewDialog(self, doc)
                dialog.ShowModal()
                return

class ReportDialog(wxDialog):
    def __init__(self, parent, title, contents):
        wxDialog.__init__(self, parent, -1, title,
                          wxDefaultPosition, wxSize(800, 600))

        self.text = wxTextCtrl(self, -1, contents,
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY)
        self.text.SetFont(wxFont(10, wxMODERN, wxNORMAL, wxNORMAL))

        sizer = wxBoxSizer(wxVERTICAL)
        sizer.Add(self.text, 1, wxALL | wxEXPAND, 5)
        
        buttonSizer = wxBoxSizer(wxHORIZONTAL)
        buttonSizer.Add(wxButton(self, wxID_OK, "OK"), 0,
                        wxALL | wxALIGN_CENTER, 5)
        sizer.Add(buttonSizer, 0, wxALIGN_RIGHT, 5)

        self.SetSizer(sizer)
        self.Layout()

# IDs for our menu command events
CW_MENU_GAME_NEW = 2000
CW_MENU_REPORT_BATTING = 2010
CW_MENU_REPORT_PITCHING = 2011
CW_MENU_REPORT_FIELDING = 2012

class ChadwickFrame(wxFrame):
    def __init__(self, parent):
        wxFrame.__init__(self, parent, -1, "Chadwick",
                         wxDefaultPosition, wxSize(800, 600))
        self.book = scorebook.ChadwickScorebook()

        self.MakeMenus()
        self.CreateStatusBar()

        sizer = wxBoxSizer(wxVERTICAL)
        
        notebook = wxNotebook(self, -1)
        self.teamList = TeamListGrid(notebook)
        notebook.AddPage(self.teamList, "Teams")
        
        self.gameList = GameListGrid(notebook)
        notebook.AddPage(self.gameList, "Games")
        
        sizer.Add(notebook, 1, wxEXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()

        EVT_MENU(self, wxID_OPEN, self.OnFileOpen)
        EVT_MENU_RANGE(self, wxID_FILE1, wxID_FILE9, self.OnFileMRU)
        EVT_MENU(self, wxID_SAVE, self.OnFileSave)
        EVT_MENU(self, wxID_EXIT, self.OnFileExit)
        EVT_MENU(self, CW_MENU_GAME_NEW, self.OnGameNew)
        EVT_MENU(self, CW_MENU_REPORT_BATTING, self.OnReportBatting)
        EVT_MENU(self, CW_MENU_REPORT_PITCHING, self.OnReportPitching)
        EVT_MENU(self, CW_MENU_REPORT_FIELDING, self.OnReportFielding)
        EVT_BUTTON(self, panelstate.CW_BUTTON_SAVE, self.OnGameSave)
        EVT_CLOSE(self, self.OnClickClose)


    def MakeMenus(self):
        fileMenu = wxMenu()
        fileMenu.Append(wxID_NEW, "&New", "Create a new scorebook")
        fileMenu.Append(wxID_OPEN, "&Open", "Open a saved scorebook")
        fileMenu.Append(wxID_SAVE, "&Save", "Save the current scorebook")
        fileMenu.AppendSeparator()
        fileMenu.Append(wxID_EXIT, "&Exit", "Close Chadwick")

        self.fileHistory = wxFileHistory()
        self.fileHistory.Load(wxConfig("Chadwick"))
        self.fileHistory.UseMenu(fileMenu)
        self.fileHistory.AddFilesToMenu()
        
        gameMenu = wxMenu()
        gameMenu.Append(CW_MENU_GAME_NEW, "New game", "Enter a new game")

        reportMenu = wxMenu()
        reportMenu.Append(CW_MENU_REPORT_BATTING, "Batting",
                          "Show batting statistics")
        reportMenu.Append(CW_MENU_REPORT_PITCHING, "Pitching",
                          "Show pitching statistics")
        reportMenu.Append(CW_MENU_REPORT_FIELDING, "Fielding",
                          "Show fielding statistics")
        
        helpMenu = wxMenu()
        helpMenu.Append(wxID_ABOUT, "&About", "About Chadwick")

        menuBar = wxMenuBar()
        menuBar.Append(fileMenu, "&File")
        menuBar.Append(gameMenu, "&Game")
        menuBar.Append(reportMenu, "&Report")
        menuBar.Append(helpMenu, "&Help")

        self.SetMenuBar(menuBar)

    def OnFileOpen(self, event):
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
        self.book = scorebook.ChadwickScorebook()
        self.book.Read(str(self.fileHistory.GetHistoryFile(event.GetId() - wxID_FILE1)))
        self.OnUpdate()
            
    def OnFileSave(self, event):
        dialog = wxFileDialog(self, "Scorebook to save...",
                              "", "",
                              "Chadwick scorebooks (*.chw)|*.chw|"
                              "All files (*.*)|*.*",
                              wxSAVE | wxOVERWRITE_PROMPT)
        if dialog.ShowModal() == wxID_OK:
            try:
                self.book.Write(str(dialog.GetPath()))
                self.fileHistory.AddFileToHistory(dialog.GetPath())
            except:
                dialog = wxMessageDialog(self,
                                         "An error occurred in writing "
                                         + str(dialog.GetPath()),
                                         "Error saving scorebook",
                                         wxOK | wxICON_ERROR)
                dialog.ShowModal()

    def OnFileExit(self, event):
        if self.book.IsModified():
            dialog = wxMessageDialog(self,
                                     "There are unsaved changes to "
                                     "the scorebook.  Continue?",
                                     "Warning: unsaved changes",
                                     wxOK | wxCANCEL | wxICON_EXCLAMATION)
            if dialog.ShowModal() == wxID_OK:
                self.Close()
        self.Close()

    def OnClickClose(self, event):
        if event.CanVeto() and self.book.IsModified():
            dialog = wxMessageDialog(self,
                                     "There are unsaved changes to "
                                     "the scorebook.  Continue?",
                                     "Warning: unsaved changes",
                                     wxOK | wxCANCEL | wxICON_EXCLAMATION)
            if dialog.ShowModal() == wxID_CANCEL:
                event.Veto()
                
        self.fileHistory.Save(wxConfig("Chadwick"))
        event.Skip()
        
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
            
    def OnUpdate(self):
        self.teamList.OnUpdate(self.book)
        self.gameList.OnUpdate(self.book)

class ChadwickApp(wxApp):
    def OnInit(self):
        frame = ChadwickFrame(NULL)
        frame.Show(true)
        self.SetTopWindow(frame)

        return true

    def GetSeasonPath(self):   return "/home/arbiter/sports/CL/"

app = ChadwickApp(0)
app.MainLoop()
