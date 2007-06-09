#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Top-level application classes for Chadwick graphical interface
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

import string, sys, os

import wx, wx.grid

import scorebook
import dw             # For Retrosheet/DiamondWare import and export
import panelstate
import icons

import game
from frameentry import GameEntryFrame
from panelgamelist import GameListCtrl, GameListPanel
import panelteamlist

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


class ChadwickFrame(wx.Frame):
    def __init__(self, parent):
        wx.Frame.__init__(self, parent, title="Chadwick", size=(800, 600))
        self.book = scorebook.Scorebook()
        self.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        self.MakeMenus()
        self.CreateStatusBar()

        icon = wx.IconFromXPMData(icons.baseball_xpm)
        self.SetIcon(icon)
        
        self.gameList = GameListPanel(self)

        self.Bind(game.EVT_GAME_UPDATE, self.OnGameUpdate)
        self.Bind(wx.EVT_CLOSE, self.OnClickClose)

        self.OnUpdate()


    def MakeMenus(self):
        fileMenu = wx.Menu()
        fileMenu.Append(wx.ID_NEW, "&New", "Create a new scorebook")
        self.Connect(wx.ID_NEW, -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnFileNew)

        fileMenu.Append(wx.ID_OPEN, "&Open", "Open a saved scorebook")
        self.Connect(wx.ID_OPEN, -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnFileOpen)

        fileMenu.Append(wx.ID_SAVE, "&Save", "Save the current scorebook")
        self.Connect(wx.ID_SAVE, -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnFileSave)

        fileMenu.Append(wx.ID_SAVEAS, "Save &as",
                        "Save the scorebook to a different file")
        self.Connect(wx.ID_SAVEAS, -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnFileSaveAs)

        fileMenu.AppendSeparator()
        item = fileMenu.Append(wx.NewId(), "&Import",
                               "Import games from another scorebook")
        self.Connect(item.GetId(), -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnFileImport)

        fileMenu.AppendSeparator()
        fileMenu.Append(wx.ID_EXIT, "&Exit", "Close Chadwick")
        self.Connect(wx.ID_EXIT, -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnFileExit)

        self.fileHistory = wx.FileHistory()
        self.fileHistory.Load(wx.Config("Chadwick"))
        self.fileHistory.UseMenu(fileMenu)
        self.fileHistory.AddFilesToMenu()

        self.Connect(wx.ID_FILE1, wx.ID_FILE9, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnFileMRU)

        editMenu = wx.Menu()
        item = editMenu.Append(wx.NewId(), "New &game",
                               "Add a new game to the scorebook")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnEditGameNew)


        item = editMenu.Append(wx.NewId(), "&Teams",
                               "Add teams and edit team information")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnEditTeams)

                               
        reportMenu = wx.Menu()

        reportRegisterMenu = wx.Menu()
        item = reportRegisterMenu.Append(wx.NewId(), "&Batting",
                                         "Compile batting register")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportRegisterBatting)

        item = reportRegisterMenu.Append(wx.NewId(), "&Pitching",
                                         "Compile pitching register")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportRegisterPitching)

        item = reportRegisterMenu.Append(wx.NewId(), "&Fielding",
                                         "Compile fielding register")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportRegisterFielding)

        reportMenu.AppendMenu(wx.NewId(), "&Register",
                              reportRegisterMenu, "Compile registers")

        reportTeamMenu = wx.Menu()

        item = reportTeamMenu.Append(wx.NewId(), "&Totals",
                                     "Compile team statistical totals")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportTeamTotals)

        item = reportTeamMenu.Append(wx.NewId(), "&Log",
                                     "Compile game logs for teams")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportTeamGameLog)
        
        item = reportTeamMenu.Append(wx.NewId(), "&Batting",
                                     "Compile individual batting by team")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportTeamBatting)

        item = reportTeamMenu.Append(wx.NewId(), "&Pitching",
                                     "Compile individual pitching by team")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportTeamPitching)

        reportMenu.AppendMenu(wx.NewId(), "&Team",
                              reportTeamMenu, "Compile team-by-team reports")


        reportPlayerMenu = wx.Menu()
        item = reportPlayerMenu.Append(wx.NewId(), "Daily &batting",
                                       "Compile game-by-game batting for player")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportPlayerDailyBatting)
        
        reportMenu.AppendMenu(wx.NewId(), "&Player", reportPlayerMenu,
                              "Compile reports by player")
        
        reportEventsMenu = wx.Menu()
        item = reportEventsMenu.Append(wx.NewId(), "&Big games",
                                       "Compile a log of notable individual performances")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportEventsBigGame)

        item = reportEventsMenu.Append(wx.NewId(), "&Grand slams",
                                       "Compile log of grand slam home runs")
        self.Connect(item.GetId(), -1, wx.wxEVT_COMMAND_MENU_SELECTED,
                     self.OnReportEventsSlams)

        reportMenu.AppendMenu(wx.NewId(), "&Events",
                              reportEventsMenu, "Compile logs of events")
        
        
        helpMenu = wx.Menu()
        helpMenu.Append(wx.ID_ABOUT, "&About", "About Chadwick")
        self.Connect(wx.ID_ABOUT, -1,
                     wx.wxEVT_COMMAND_MENU_SELECTED, self.OnHelpAbout)

        menuBar = wx.MenuBar()
        menuBar.Append(fileMenu, "&File")
        menuBar.Append(editMenu, "&Edit")
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
            self.book = dw.Reader(filename)
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
        if self.CheckUnsaved() == wx.ID_CANCEL:  return

        dialog = YearDialog(self)
        if dialog.ShowModal() == wx.ID_OK:
            self.book = scorebook.Scorebook(dialog.GetYear())
            self.OnUpdate()

    def OnFileOpen(self, event):
        if self.CheckUnsaved() == wx.ID_CANCEL:  return
        
        dialog = wx.FileDialog(self, "Scorebook to open...",
                               "", "",
                               "Chadwick scorebooks (*.chw)|*.chw|"
                               "Retrosheet zipfiles (*.zip)|*.zip|"
                               "All files (*.*)|*.*")
        if dialog.ShowModal() == wx.ID_OK:
            try:
                self.book = dw.Reader(str(dialog.GetPath()))
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
        if self.CheckUnsaved() == wx.ID_CANCEL:  return
        
        filename = self.fileHistory.GetHistoryFile(event.GetId() - wx.ID_FILE1)
        try:
            self.book = dw.Reader(str(filename))
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
            dw.Writer(self.book, self.book.GetFilename())
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
                dw.Writer(self.book, str(dialog.GetPath()))
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
                book = dw.Reader(str(dialog.GetPath()))
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
        if self.CheckUnsaved() == wx.ID_CANCEL:  return
        self.fileHistory.Save(wx.Config("Chadwick"))
        global app
        app.ExitMainLoop()
        
    def OnClickClose(self, event):
        if self.CheckUnsaved() == wx.ID_CANCEL:  return
        self.fileHistory.Save(wx.Config("Chadwick"))
        global app
        app.ExitMainLoop()

    def OnHelpAbout(self, event):
        dialog = AboutDialog(self)
        dialog.ShowModal()
        
    def OnEditGameNew(self, event):
        dialog = NewGameDialog(self, self.book)
        if dialog.ShowModal() != wx.ID_OK:
            return
            
        rosters = [ self.book.GetTeam(dialog.GetTeam(t)) for t in [0, 1] ]

        thegame = game.CreateGame(dialog.GetGameId(),
                                  rosters[0].GetID(), rosters[1].GetID())
        thegame.SetInfo("pitches", dialog.GetPitches())
        doc = game.Game(self.book, thegame, rosters[0], rosters[1])

        for t in [0, 1]:
            # This gives a list of all games the team has already had entered
            prevGames = [ y for y in self.book.Games()
                          if rosters[t].GetId() in [ y.GetTeam(0),
                                                     y.GetTeam(1) ] ]
            dialog = LineupDialog(self, 
                                  "Starting Lineup for %s" % 
                                  rosters[t].GetName())
            dialog.LoadRoster(self.book, doc.GetRoster(t), t, True)

            if len(prevGames) > 0:
                # Find the game that was previous to the current one.
                # If the game being entered was earlier than all others,
                # just use the first one
                if prevGames[0].GetDate() > thegame.GetDate():
                    pg = prevGames[0]
                else:
                    pg = None
                    for g in prevGames:
                        if g.GetDate() >= thegame.GetDate():
                            pg = g
                            break
                    if pg == None:  pg = prevGames[-1]

                # Set up the lineup dialog with the lineup from that game,
                # figuring it's probably a decent first guess as to
                # the lineup for this game
                tm = pg.GetTeams().index(rosters[t].GetID())
                for slot in range(1, 10):
                    rec = pg.GetStarter(tm, slot)
                    dialog.SetPlayerInSlot(slot,
                                           rosters[t].GetPlayer(rec.player_id).GetSortName(),
                                           rec.pos)
                
            if dialog.ShowModal() != wx.ID_OK:
                return
                
            for slot in range(9):
                player = dialog.GetPlayerInSlot(slot+1)
                doc.SetStarter(player.GetID(), player.GetName(),
                               t, slot+1, dialog.GetPositionInSlot(slot+1))

            if dialog.HasDH():
                player = dialog.GetPlayerInSlot(10)
                doc.SetStarter(player.GetID(), player.GetName(),
                               t, 0, 1)
                doc.GetGame().SetInfo("usedh", "true")
                

        self.EditGame(doc)
            

    def OnEditTeams(self, event):
        panelteamlist.TeamListDialog(self, self.book).ShowModal()

    def EditGame(self, g):
        g.BuildBoxscore()
        frame = GameEntryFrame(self, g)
        frame.Show(True)
        

    def OnGameUpdate(self, event):
        self.SaveGame(event.gameDoc)
        
    def SaveGame(self, gameDoc):
        self.book.SetGame(gameDoc.GetGame())
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
                                      "drarbiter@gmail.com\n"
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
            #if self.book.GetFilename() == "untitled.chw":
            title += " (unsaved changes)"
            #else:
            #    self.OnFileSave(wx.CommandEvent())
            
        self.SetTitle(title)
        self.gameList.OnUpdate(self.book)

        
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
