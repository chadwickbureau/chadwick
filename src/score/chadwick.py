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
import string, sys, os

from libchadwick import *
import scorebook
import panelstate
import icons

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
CW_MENU_REPORT_REGISTER = 2009
CW_MENU_REPORT_REGISTER_BATTING = 2010
CW_MENU_REPORT_REGISTER_PITCHING = 2011
CW_MENU_REPORT_REGISTER_FIELDING = 2012
CW_MENU_REPORT_TEAM = 2020
CW_MENU_REPORT_TEAM_TOTALS = 2021
CW_MENU_REPORT_TEAM_GAMELOG = 2022
CW_MENU_REPORT_TEAM_BATTING = 2023
CW_MENU_REPORT_TEAM_PITCHING = 2024
CW_MENU_REPORT_EVENTS = 2016
CW_MENU_REPORT_EVENTS_SLAMS = 2017

class ChadwickFrame(wxFrame):
    def __init__(self, parent):
        wxFrame.__init__(self, parent, -1, "Chadwick",
                         wxDefaultPosition, wxSize(800, 600))
        self.book = scorebook.ChadwickScorebook()
        self.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL))

        self.MakeMenus()
        self.CreateStatusBar()

        icon = wxIconFromXPMData(icons.baseball_xpm)
        self.SetIcon(icon)
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
        EVT_MENU(self, CW_MENU_REPORT_REGISTER_BATTING,
                 self.OnReportRegisterBatting)
        EVT_MENU(self, CW_MENU_REPORT_REGISTER_PITCHING,
                 self.OnReportRegisterPitching)
        EVT_MENU(self, CW_MENU_REPORT_REGISTER_FIELDING,
                 self.OnReportRegisterFielding)
        EVT_MENU(self, CW_MENU_REPORT_TEAM_TOTALS, self.OnReportTeamTotals)
        EVT_MENU(self, CW_MENU_REPORT_TEAM_GAMELOG, self.OnReportTeamGameLog)
        EVT_MENU(self, CW_MENU_REPORT_TEAM_BATTING, self.OnReportTeamBatting)
        EVT_MENU(self, CW_MENU_REPORT_TEAM_PITCHING, self.OnReportTeamPitching)
        EVT_MENU(self, CW_MENU_REPORT_EVENTS_SLAMS, self.OnReportEventsSlams)
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

        reportRegisterMenu = wxMenu()
        reportRegisterMenu.Append(CW_MENU_REPORT_REGISTER_BATTING,
                                  "&Batting", "Compile batting register")
        reportRegisterMenu.Append(CW_MENU_REPORT_REGISTER_PITCHING,
                                  "&Pitching", "Compile pitching register")
        reportRegisterMenu.Append(CW_MENU_REPORT_REGISTER_FIELDING,
                                  "&Fielding", "Compile fielding register")
        reportMenu.AppendMenu(CW_MENU_REPORT_REGISTER, "&Register",
                              reportRegisterMenu, "Compile registers")

        reportTeamMenu = wxMenu()
        reportTeamMenu.Append(CW_MENU_REPORT_TEAM_TOTALS,
                              "&Totals", "Compile team statistical totals")
        reportTeamMenu.Append(CW_MENU_REPORT_TEAM_GAMELOG,
                              "&Log", "Compile game logs for teams")
        reportTeamMenu.Append(CW_MENU_REPORT_TEAM_BATTING,
                              "&Batting", "Compile individual batting by team")
        reportTeamMenu.Append(CW_MENU_REPORT_TEAM_PITCHING,
                              "&Pitching", "Compile individual pitching by team")
        reportMenu.AppendMenu(CW_MENU_REPORT_TEAM, "&Team",
                              reportTeamMenu, "Compile team-by-team reports")

        reportEventsMenu = wxMenu()
        reportEventsMenu.Append(CW_MENU_REPORT_EVENTS_SLAMS,
                                "&Grand slams",
                                "Compile log of grand slam home runs")
        reportMenu.AppendMenu(CW_MENU_REPORT_EVENTS, "&Events",
                              reportEventsMenu, "Compile logs of events")
        
        
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

    def OnCommandLineFile(self, filename):
        """
        Try to load a scorebook specified on the command line
        """
        try:
            book = scorebook.ChadwickScorebook()
            book.Read(filename)
            self.book = book
        except:
            dialog = wxMessageDialog(self,
                                     "An error occurred in reading "
                                     + filename,
                                     "Error opening scorebook",
                                     wxOK | wxICON_ERROR)
            dialog.ShowModal()
            return

        self.OnUpdate()
            

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
        
        filename = self.fileHistory.GetHistoryFile(event.GetId() - wxID_FILE1)
        try:
            book = scorebook.ChadwickScorebook()
            book.Read(str(filename))
            self.book = book
            self.OnUpdate()
        except:
            dialog = wxMessageDialog(self,
                                     "An error occurred in reading " 
                                     + str(filename),
                                     "Error opening scorebook",
                                     wxOK | wxICON_ERROR)
            dialog.ShowModal()
            
            
    def OnFileSave(self, event):
        if not self.book.IsModified():  return
        
        # Remove a backup file
        try:
            os.remove(self.book.GetFilename() + "~")
        except:
            pass

        # Rename the original file to a backup
        try:
            os.rename(self.book.GetFilename(),
                      self.book.GetFilename() + "~")
        except:
            pass

        # Now try to write this file
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
                # We don't do any backup file writing here
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
                          rosters[0].GetID(), rosters[1].GetID())
        game.SetInfo("pitches", dialog.GetPitches())
        doc = GameEditor(game, rosters[0], rosters[1])

        for t in [0, 1]:
            # This gives a list of all games the team has already had entered
            prevGames = [ y for y in self.book.Games(lambda x: rosters[t].GetId() in x.GetTeams()) ]
            dialog = LineupDialog(self, 
                                  "Starting Lineup for %s" % 
                                  rosters[t].GetName())
            dialog.LoadRoster(doc.GetRoster(t), t, True)

            if len(prevGames) > 0:
                # Find the game that was previous to the current one.
                # If the game being entered was earlier than all others,
                # just use the first one
                if prevGames[0].GetDate() > game.GetDate():
                    pg = prevGames[0]
                else:
                    pg = None
                    for g in prevGames:
                        if g.GetDate() >= game.GetDate():
                            pg = g
                            break
                    if pg == None:  pg = prevGames[-1]

                # Set up the lineup dialog with the lineup from that game,
                # figuring it's probably a decent first guess as to
                # the lineup for this game
                tm = pg.GetTeams().index(rosters[t].GetID())
                for slot in range(1, 10):
                    rec = pg.GetStarter(tm, slot)
                    dialog.SetPlayerInSlot(slot, rec.name, rec.pos)
                
            if dialog.ShowModal() != wxID_OK:
                return
                
            for slot in range(9):
                player = dialog.GetPlayerInSlot(slot+1)
                doc.SetStarter(player.player_id, player.GetName(),
                               t, slot+1, dialog.GetPositionInSlot(slot+1))

            if dialog.HasDH():
                player = dialog.GetPlayerInSlot(10)
                doc.SetStarter(player.player_id, player.GetName(),
                               t, 0, 1)
                doc.GetGame().SetInfo("usedh", "true")
                
                                
        doc.BuildBoxscore()
        self.entryFrame = GameEntryFrame(self, doc) 
        self.entryFrame.SetDocument(doc)
        self.entryFrame.Show(true)
        
    def OnGameSave(self, event):
        self.book.AddGame(self.entryFrame.GetDocument().GetGame())
        self.entryFrame.Destroy()
        del self.entryFrame
        self.OnUpdate()

    def RunReport(self, message, title, acc):
        dialog = wxProgressDialog(message, message, 100, self,
                                  wxPD_APP_MODAL | wxPD_AUTO_HIDE |
                                  wxPD_CAN_ABORT | wxPD_ELAPSED_TIME |
                                  wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME)
        try:
            if statscan.ProcessFile(self.book, acc, dialog):
                dialog.Show(false)
                
                dialog = ReportDialog(self, title, 
                                      string.join([ str(x) for x in acc ],
                                                  "\n\n"))
                dialog.ShowModal()
            else:
                dialog.Show(false)
        except:
            dialog.Show(false)
        
            dialog = wxMessageDialog(self,
                                     "An internal error occurred in "
                                     "generating the report.\n"
                                     "Please send a bug report to "
                                     "the maintaner at "
                                     "turocy@econmail.tamu.edu\n"
                                     "It is helpful to include this scorebook "
                                     "as an "
                                     "attachment when you send the report.\n"
                                     "The problem deals only with this "
                                     "report: don't worry, "
                                     "your data is unaffected.\n"
                                     "We apologize for the inconvenience!\n",
                                     "Oops! There's a bug in Chadwick",
                                     wxOK | wxICON_EXCLAMATION)
            dialog.ShowModal()
        
    def OnReportRegisterBatting(self, event):
        self.RunReport("Compiling batting register", "Batting register",
                       [ statscan.BattingRegister(self.book) ])

    def OnReportRegisterPitching(self, event):
        self.RunReport("Compiling pitching register", "Pitching register",
                       [ statscan.PitchingRegister(self.book) ])

    def OnReportRegisterFielding(self, event):
        self.RunReport("Compiling fielding register", "Fielding register",
                       [ statscan.FieldingRegister(self.book, p+1)
                         for p in range(9) ])

    def OnReportTeamTotals(self, event):
        self.RunReport("Compiling team totals", "Team totals",
                       [ statscan.TeamRecordTotals(self.book),
                         statscan.TeamBattingTotals(self.book),
                         statscan.TeamPitchingTotals(self.book),
                         statscan.TeamFieldingTotals(self.book) ])

    def OnReportTeamGameLog(self, event):
        self.RunReport("Compiling team game logs", "Team game logs",
                       [ statscan.TeamGameLog(self.book) ])

    def OnReportTeamBatting(self, event):
        self.RunReport("Compiling team-by-team batting",
                       "Team-by-team batting",
                       [ statscan.TeamBattingRegister(self.book, t.GetID())
                         for t in self.book.Teams() ])

    def OnReportTeamPitching(self, event):
        self.RunReport("Compiling team-by-team pitching",
                       "Team-by-team pitching",
                       [ statscan.TeamPitchingRegister(self.book, t.GetID())
                         for t in self.book.Teams() ])

    def OnReportEventsSlams(self, event):
        self.RunReport("Compiling list of grand slams", "Grand slams",
                       [ statscan.GrandSlamLog(self.book) ])

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
        if len(sys.argv) >= 2:
            frame.OnCommandLineFile(sys.argv[1])
        frame.Show(true)
        self.SetTopWindow(frame)

        return true

    def GetSeasonPath(self):   return "/home/arbiter/sports/CL/"

app = ChadwickApp(0)
app.MainLoop()
