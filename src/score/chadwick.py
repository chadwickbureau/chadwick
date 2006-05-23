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

import string, sys, os

import wx, wx.grid

from libchadwick import *
import scorebook
import panelstate
import icons

from gameeditor import GameEditor, CreateGame
from frameentry import GameEntryFrame
from panelgamelist import GameListCtrl, GameListPanel
from panelplayerlist import PlayerListPanel
from panelteamlist import TeamListPanel

from dialogimport import ImportDialog
from dialognewgame import NewGameDialog
from dialoglineup import LineupDialog
from dialogreport import ReportDialog
from dialogabout import AboutDialog

import statscan

from wxutils import FormattedStaticText

class YearDialog(wx.Dialog):
    def __init__(self, parent):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, "New scorebook")

        topSizer = wx.BoxSizer(wx.VERTICAL)
        
        yearSizer = wx.BoxSizer(wx.HORIZONTAL)
        yearSizer.Add(FormattedStaticText(self, "Year"),
                      0, wx.ALL | wx.ALIGN_CENTER, 5)
                      
        self.year = wx.TextCtrl(self, wx.ID_ANY, "2005", size=(125, -1))
        yearSizer.Add(self.year, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        topSizer.Add(yearSizer, 0, wx.ALL, 5)
        
        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        topSizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

        wx.EVT_TEXT(self, self.year.GetId(), self.OnChangeYear)
        
    def OnChangeYear(self, event):
        try:
            if int(str(self.year.GetValue())) > 0:
                self.FindWindowById(wx.ID_OK).Enable(True)
            else:
                self.FindWindowById(wx.ID_OK).Enable(False)
        except ValueError:
            self.FindWindowById(wx.ID_OK).Enable(False)
    
    def GetYear(self):  return int(str(self.year.GetValue()))

class ChoosePlayerDialog(wx.Dialog):
    def __init__(self, parent, title, book):
        wx.Dialog.__init__(self, parent, wx.ID_ANY, title)

        topSizer = wx.BoxSizer(wx.VERTICAL)

        self.playerList = wx.ListBox(self, wx.ID_ANY, style=wx.LB_SINGLE)
        self.players = [ ]
        for (pl,player) in enumerate(book.Players()):
            self.playerList.Append("%s (%s)" % (player.GetSortName(),
                                                player.GetID()))
            self.players.append(player.GetID())

        self.playerList.SetSelection(0)
        topSizer.Add(self.playerList, 0, wx.ALL | wx.ALIGN_CENTER, 5)

        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add(wx.Button(self, wx.ID_CANCEL, "Cancel"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        buttonSizer.Add(wx.Button(self, wx.ID_OK, "OK"),
                        0, wx.ALL | wx.ALIGN_CENTER, 5)
        topSizer.Add(buttonSizer, 0, wx.ALIGN_RIGHT, 5)

        self.SetSizer(topSizer)
        self.Layout()
        topSizer.SetSizeHints(self)

    def GetPlayerID(self):
        return self.players[self.playerList.GetSelection()]

# IDs for our menu command events
CW_MENU_FILE_IMPORT = 2008
CW_MENU_REPORT_REGISTER = 2009
CW_MENU_REPORT_REGISTER_BATTING = 2010
CW_MENU_REPORT_REGISTER_PITCHING = 2011
CW_MENU_REPORT_REGISTER_FIELDING = 2012
CW_MENU_REPORT_TEAM = 2020
CW_MENU_REPORT_TEAM_TOTALS = 2021
CW_MENU_REPORT_TEAM_GAMELOG = 2022
CW_MENU_REPORT_TEAM_BATTING = 2023
CW_MENU_REPORT_TEAM_PITCHING = 2024
CW_MENU_REPORT_PLAYER = 2030
CW_MENU_REPORT_PLAYER_DAILY_BATTING = 2031
CW_MENU_REPORT_EVENTS = 2016
CW_MENU_REPORT_EVENTS_SLAMS = 2017
CW_MENU_REPORT_EVENTS_BIGGAME = 2018

# This is duplicated from panelgamelist. We need to refactor!
CW_MENU_GAME_NEW = 2000

class ChadwickFrame(wx.Frame):
    def __init__(self, parent):
        wx.Frame.__init__(self, parent, wx.ID_ANY, "Chadwick", size=(800, 600))
        self.book = scorebook.ChadwickScorebook()
        self.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        self.MakeMenus()
        self.CreateStatusBar()

        icon = wx.IconFromXPMData(icons.baseball_xpm)
        self.SetIcon(icon)
        sizer = wx.BoxSizer(wx.VERTICAL)
        
        notebook = wx.Notebook(self, wx.ID_ANY)
        self.gameList = GameListPanel(notebook)
        notebook.AddPage(self.gameList, "Games")
        
        self.teamList = TeamListPanel(notebook)
        notebook.AddPage(self.teamList, "Teams")

        self.playerList = PlayerListPanel(notebook)
        notebook.AddPage(self.playerList, "Players")
        
        sizer.Add(notebook, 1, wx.EXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()

        wx.EVT_MENU(self, wx.ID_NEW, self.OnFileNew)
        wx.EVT_MENU(self, wx.ID_OPEN, self.OnFileOpen)
        wx.EVT_MENU_RANGE(self, wx.ID_FILE1, wx.ID_FILE9, self.OnFileMRU)
        wx.EVT_MENU(self, wx.ID_SAVE, self.OnFileSave)
        wx.EVT_MENU(self, wx.ID_SAVEAS, self.OnFileSaveAs)
        wx.EVT_MENU(self, CW_MENU_FILE_IMPORT, self.OnFileImport)
        wx.EVT_MENU(self, wx.ID_EXIT, self.OnFileExit)
        wx.EVT_MENU(self, wx.ID_ABOUT, self.OnHelpAbout)
        wx.EVT_BUTTON(self, CW_MENU_GAME_NEW, self.OnGameNew)
        wx.EVT_MENU(self, CW_MENU_REPORT_REGISTER_BATTING,
                    self.OnReportRegisterBatting)
        wx.EVT_MENU(self, CW_MENU_REPORT_REGISTER_PITCHING,
                    self.OnReportRegisterPitching)
        wx.EVT_MENU(self, CW_MENU_REPORT_REGISTER_FIELDING,
                    self.OnReportRegisterFielding)
        wx.EVT_MENU(self, CW_MENU_REPORT_TEAM_TOTALS, self.OnReportTeamTotals)
        wx.EVT_MENU(self, CW_MENU_REPORT_TEAM_GAMELOG, self.OnReportTeamGameLog)
        wx.EVT_MENU(self, CW_MENU_REPORT_TEAM_BATTING, self.OnReportTeamBatting)
        wx.EVT_MENU(self, CW_MENU_REPORT_TEAM_PITCHING, self.OnReportTeamPitching)
        wx.EVT_MENU(self, CW_MENU_REPORT_PLAYER_DAILY_BATTING,
                    self.OnReportPlayerDailyBatting)
        wx.EVT_MENU(self, CW_MENU_REPORT_EVENTS_SLAMS, self.OnReportEventsSlams)
        wx.EVT_MENU(self, CW_MENU_REPORT_EVENTS_BIGGAME,
                    self.OnReportEventsBigGame)
        wx.EVT_BUTTON(self, panelstate.CW_BUTTON_SAVE, self.OnGameSave)
        wx.EVT_CLOSE(self, self.OnClickClose)

        self.OnUpdate()


    def MakeMenus(self):
        fileMenu = wx.Menu()
        fileMenu.Append(wx.ID_NEW, "&New", "Create a new scorebook")
        fileMenu.Append(wx.ID_OPEN, "&Open", "Open a saved scorebook")
        fileMenu.Append(wx.ID_SAVE, "&Save", "Save the current scorebook")
        fileMenu.Append(wx.ID_SAVEAS, "Save &as",
                        "Save the scorebook to a different file")
        fileMenu.AppendSeparator()
        fileMenu.Append(CW_MENU_FILE_IMPORT, "&Import",
                        "Import games from another scorebook")
        fileMenu.AppendSeparator()
        fileMenu.Append(wx.ID_EXIT, "&Exit", "Close Chadwick")

        self.fileHistory = wx.FileHistory()
        self.fileHistory.Load(wx.Config("Chadwick"))
        self.fileHistory.UseMenu(fileMenu)
        self.fileHistory.AddFilesToMenu()
        
        reportMenu = wx.Menu()

        reportRegisterMenu = wx.Menu()
        reportRegisterMenu.Append(CW_MENU_REPORT_REGISTER_BATTING,
                                  "&Batting", "Compile batting register")
        reportRegisterMenu.Append(CW_MENU_REPORT_REGISTER_PITCHING,
                                  "&Pitching", "Compile pitching register")
        reportRegisterMenu.Append(CW_MENU_REPORT_REGISTER_FIELDING,
                                  "&Fielding", "Compile fielding register")
        reportMenu.AppendMenu(CW_MENU_REPORT_REGISTER, "&Register",
                              reportRegisterMenu, "Compile registers")

        reportTeamMenu = wx.Menu()
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


        reportPlayerMenu = wx.Menu()
        reportPlayerMenu.Append(CW_MENU_REPORT_PLAYER_DAILY_BATTING,
                                "Daily &batting",
                                "Compile game-by-game batting for player")
        reportMenu.AppendMenu(CW_MENU_REPORT_PLAYER, "&Player",
                              reportPlayerMenu,
                              "Compile reports by player")
        
        reportEventsMenu = wx.Menu()
        reportEventsMenu.Append(CW_MENU_REPORT_EVENTS_BIGGAME,
                                "&Big games",
                                "Compile a log of notable individual performanceS")
        reportEventsMenu.Append(CW_MENU_REPORT_EVENTS_SLAMS,
                                "&Grand slams",
                                "Compile log of grand slam home runs")
        reportMenu.AppendMenu(CW_MENU_REPORT_EVENTS, "&Events",
                              reportEventsMenu, "Compile logs of events")
        
        
        helpMenu = wx.Menu()
        helpMenu.Append(wx.ID_ABOUT, "&About", "About Chadwick")

        menuBar = wx.MenuBar()
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

        dialog = wx.MessageDialog(self,
                                  "There are unsaved changes to "
                                  "the scorebook.  Continue?",
                                  "Warning: unsaved changes",
                                  wx.OK | wx.CANCEL | wx.ICON_EXCLAMATION)
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
            dialog = wx.MessageDialog(self,
                                      "An error occurred in reading "
                                      + filename,
                                      "Error opening scorebook",
                                      wx.OK | wx.ICON_ERROR)
            dialog.ShowModal()
            return

        self.OnUpdate()
            

    def OnFileNew(self, event):
        if not self.CheckUnsaved():  return

        dialog = YearDialog(self)
        if dialog.ShowModal() == wx.ID_OK:
            self.book = scorebook.ChadwickScorebook(dialog.GetYear())
            self.OnUpdate()

    def OnFileOpen(self, event):
        if not self.CheckUnsaved():  return
        
        dialog = wx.FileDialog(self, "Scorebook to open...",
                               "", "",
                               "Chadwick scorebooks (*.chw)|*.chw|"
                               "Retrosheet zipfiles (*.zip)|*.zip|"
                               "All files (*.*)|*.*")
        if dialog.ShowModal() == wx.ID_OK:
            try:
                book = scorebook.ChadwickScorebook()
                book.Read(str(dialog.GetPath()))
                self.book = book
                self.OnUpdate()
                self.fileHistory.AddFileToHistory(dialog.GetPath())
            except:
                dialog = wx.MessageDialog(self,
                                          "An error occurred in reading "
                                          + str(dialog.GetPath()),
                                          "Error opening scorebook",
                                          wx.OK | wx.ICON_ERROR)
                dialog.ShowModal()

    def OnFileMRU(self, event):
        if not self.CheckUnsaved():  return
        
        filename = self.fileHistory.GetHistoryFile(event.GetId() - wx.ID_FILE1)
        try:
            book = scorebook.ChadwickScorebook()
            book.Read(str(filename))
            self.book = book
            self.OnUpdate()
        except:
            dialog = wx.MessageDialog(self,
                                      "An error occurred in reading " 
                                      + str(filename),
                                      "Error opening scorebook",
                                      wx.OK | wx.ICON_ERROR)
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
            dialog = wx.MessageDialog(self,
                                      "An error occurred in writing "
                                      + self.book.GetFilename(),
                                      "Error saving scorebook",
                                      wx.OK | wx.ICON_ERROR)
            dialog.ShowModal()


    def OnFileSaveAs(self, event):
        dialog = wx.FileDialog(self, "Scorebook to save...",
                               "", "",
                               "Chadwick scorebooks (*.chw)|*.chw|"
                               "All files (*.*)|*.*",
                               wx.SAVE | wx.OVERWRITE_PROMPT)
        if dialog.ShowModal() == wx.ID_OK:
            try:
                # We don't do any backup file writing here
                self.book.Write(str(dialog.GetPath()))
                self.fileHistory.AddFileToHistory(dialog.GetPath())
                self.OnUpdate()
            except:
                dialog = wx.MessageDialog(self,
                                          "An error occurred in writing "
                                          + str(dialog.GetPath()),
                                          "Error saving scorebook",
                                          wx.OK | wx.ICON_ERROR)
                dialog.ShowModal()

    def OnFileImport(self, event):
        dialog = wx.FileDialog(self, "Scorebook to open...",
                               "", "",
                               "Chadwick scorebooks (*.chw)|*.chw|"
                               "Retrosheet zipfiles (*.zip)|*.zip|"
                               "All files (*.*)|*.*")
        if dialog.ShowModal() == wx.ID_OK:
            try:
                book = scorebook.ChadwickScorebook()
                book.Read(str(dialog.GetPath()))
            except:
                dialog = wx.MessageDialog(self,
                                          "An error occurred in reading "
                                          + str(dialog.GetPath()),
                                          "Error opening scorebook",
                                          wx.OK | wx.ICON_ERROR)
                dialog.ShowModal()
                return

        dialog = ImportDialog(self, book)
        if dialog.ShowModal() == wx.ID_OK:
            self.book.ImportGames(book, dialog.GetSelectedGames())
            self.OnUpdate()

    def OnFileExit(self, event):
        if not self.CheckUnsaved():  return
        self.fileHistory.Save(wx.Config("Chadwick"))
        global app
        app.ExitMainLoop()
        
    def OnClickClose(self, event):
        if not self.CheckUnsaved():  return
        self.fileHistory.Save(wx.Config("Chadwick"))
        global app
        app.ExitMainLoop()

    def OnHelpAbout(self, event):
        dialog = AboutDialog(self)
        dialog.ShowModal()
        
    def OnGameNew(self, event):
        dialog = NewGameDialog(self, self.book)
        if dialog.ShowModal() != wx.ID_OK:
            return
            
        rosters = [ self.book.GetTeam(dialog.GetTeam(t)) for t in [0, 1] ]

        game = CreateGame(dialog.GetGameId(),
                          rosters[0].GetID(), rosters[1].GetID())
        game.SetInfo("pitches", dialog.GetPitches())
        doc = GameEditor(game, rosters[0], rosters[1])

        for t in [0, 1]:
            # This gives a list of all games the team has already had entered
            prevGames = [ y for y in self.book.Games(lambda x: rosters[t].GetID() in x.GetTeams()) ]
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
                
            if dialog.ShowModal() != wx.ID_OK:
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
        self.entryFrame.Show(True)
        
    def OnGameSave(self, event):
        self.book.AddGame(self.entryFrame.GetDocument().GetGame())
        self.entryFrame.Destroy()
        del self.entryFrame
        self.OnUpdate()

    def RunReport(self, message, title, acc):
        dialog = wx.ProgressDialog(message, message, 100, self,
                                   wx.PD_APP_MODAL | wx.PD_AUTO_HIDE |
                                   wx.PD_CAN_ABORT | wx.PD_ELAPSED_TIME |
                                   wx.PD_ESTIMATED_TIME | wx.PD_REMAINING_TIME)
        try:
            if statscan.ProcessFile(self.book, acc, monitor=dialog):
                dialog.Show(False)
                
                dialog = ReportDialog(self, title, 
                                      string.join([ str(x) for x in acc ],
                                                  "\n\n"))
                dialog.ShowModal()
            else:
                dialog.Show(False)
        except:
            dialog.Show(False)
        
            dialog = wx.MessageDialog(self,
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
                                      wx.OK | wx.ICON_EXCLAMATION)
            dialog.ShowModal()
        
    def OnReportRegisterBatting(self, event):
        report = self.book.GetReport("batting-register")
        if report != None:
            dialog = ReportDialog(self, "Batting register", str(report))
            dialog.ShowModal()
        else:
            report = statscan.BattingRegister(self.book)
            self.RunReport("Compiling batting register", "Batting register",
                           [ report ])
            self.book.AddReport(report)

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

    def OnReportPlayerDailyBatting(self, event):
        dialog = ChoosePlayerDialog(self, "Choose player", self.book)
        if dialog.ShowModal() == wx.ID_OK:
            self.RunReport("Compiling daily batting for %s" %
                           self.book.GetPlayer(dialog.GetPlayerID()).GetName(),
                           "Daily batting",
                           [ statscan.BattingDailies(self.book,
                                                     dialog.GetPlayerID()) ])

    def OnReportEventsSlams(self, event):
        self.RunReport("Compiling list of grand slams", "Grand slams",
                       [ statscan.GrandSlamLog(self.book) ])

    def OnReportEventsBigGame(self, event):
        self.RunReport("Compiling list of notable individual performances",
                       "Notable individual performances",
                       [ statscan.MultiHRLog(self.book),
                         statscan.MultiHitLog(self.book),
                         statscan.MultiStrikeoutLog(self.book) ])

    def OnUpdate(self):
        title = "Chadwick: [%s] %d" % (self.book.GetFilename(),
                                       self.book.GetYear())
        if self.book.IsModified():
            title += " (unsaved changes)"
            
        self.SetTitle(title)
        self.teamList.OnUpdate(self.book)
        self.gameList.OnUpdate(self.book)
        self.playerList.OnUpdate(self.book)

        
class ChadwickApp(wx.App):
    def OnInit(self):
        frame = ChadwickFrame(None)
        if len(sys.argv) >= 2:
            frame.OnCommandLineFile(sys.argv[1])
        frame.Show(True)
        self.SetTopWindow(frame)

        return True

    def GetSeasonPath(self):   return "/home/arbiter/sports/CL/"

app = ChadwickApp(0)
app.MainLoop()
