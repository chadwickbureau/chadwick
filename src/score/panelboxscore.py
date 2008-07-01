#
# $Source$
# $Date$
# $Revision$
#
# DESCRIPTION:
# Panel to display boxscore
#
# This file is part of Chadwick, a library for baseball play-by-play and stats
# Copyright (C) 2005-2007, Ted Turocy (drarbiter@gmail.com)
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
import string

def iterate_statlines(list, f):
    for x in list:
        for y in x:
            if f(y): yield y
    raise StopIteration

def PositionList(rec):
    pos = [ "", "p", "c", "1b", "2b", "3b", "ss", "lf", "cf", "rf", "dh", "ph", "pr" ]
    return string.join([ pos[i] for i in rec["pos"] ], "-")


class TextBoxscore:
    def __init__(self, doc):
        self.doc = doc

    def AppendText(self, x):
        import sys
        sys.stdout.write(x)

    def PrintOffensiveCaption(self, stats, key, label):
        qual = [ x for x
                 in iterate_statlines(stats,
                                      lambda x: x[key] > 0) ]
        if len(qual) > 0:
            self.AppendText(label)
            strs = [ ]
            for rec in qual:
                x = rec["name"]
                if rec[key] > 1:   x += " %d" % rec[key]
                strs.append(x)
            self.AppendText(string.join(strs, ", "))
            self.AppendText("\n")

    def PrintPitchingCaption(self, stats, key, label):
        qual = filter(lambda x: x[key] > 0, stats)

        if len(qual) > 0:
            self.AppendText(label)
            strs = [ ]
            for rec in qual:
                x = rec["name"]
                if rec[key] > 1:  x += " %d" % rec[key]
                strs.append(x)
            self.AppendText(string.join(strs, ", "))
            self.AppendText("\n")

    def PrintHeading(self, box):
        self.AppendText("%s at %s\n" %
                             (self.doc.GetRoster(0).GetName(),
                              self.doc.GetRoster(1).GetName()))
        self.AppendText("Game of %s" % self.doc.GetGame().GetDate())
        number = self.doc.GetGame().GetNumber()
        if number == 1:
            self.AppendText(" (first game)\n")
        elif number == 2:
            self.AppendText(" (second game)\n")
        else:
            self.AppendText("\n")

        self.AppendText("\n");

    def PrintLinescore(self, box):
        self.AppendText("%-15s" % "")
        for i in range(len(box.byInnings[0])):
            self.AppendText(" %2d" % (i+1))
        self.AppendText("  -  R  H  E LOB DP\n")
        
        for t in [0, 1]:
            ros = self.doc.GetRoster(t)

            self.AppendText("%-15s" % ros.GetCity())

            for r in box.byInnings[t]:
                self.AppendText(" %2d" % r)

            if t == 1 and len(box.byInnings[1]) < len(box.byInnings[0]):
                self.AppendText("  x")
                
            self.AppendText("  - %2d %2d %2d  %2d %2d" %
                                 (self.doc.GetScore(t),
                                  self.doc.GetHits(t),
                                  self.doc.GetErrors(t),
                                  self.doc.GetLOB(t),
                                  box.GetDPs(t)))

            self.AppendText("\n")

        self.AppendText("\n")

    def PrintBatting(self, box):
        self.AppendText("\n              BATTING AND FIELDING\n\n")
        for t in [0, 1]:
            ros = self.doc.GetRoster(t)
            totals = { "ab":0, "r":0, "h":0, "bi":0, "bb":0, "so":0 }
            
            self.AppendText("%-30s ab  r  h bi bb so\n" %
                                 ros.GetCity())
            for slot in range(9):
                starter = True
                for rec in box.stats[t][slot]:
                    if starter:
                        x = "%-30s" % (rec["name"] + ", " + PositionList(rec))
                    else:
                        x = " %-29s" % (rec["name"] + ", " + PositionList(rec))
                    x += (" %2d %2d %2d %2d %2d %2d\n" %
                         (rec["ab"], rec["r"], rec["h"],
                          rec["bi"], rec["bb"], rec["so"]))
                    for stat in [ "ab", "r", "h", "bi", "bb", "so" ]:
                        totals[stat] += rec[stat]
                    starter = False
                    self.AppendText(x)

            self.AppendText("%-30s %2d %2d %2d %2d %2d %2d\n" %
                                 ("TOTALS", totals["ab"],
                                  totals["r"], totals["h"], totals["bi"],
                                  totals["bb"], totals["so"]))
            self.AppendText("\n")

            self.PrintOffensiveCaption(box.stats[t], "2b", "2B - ")
            self.PrintOffensiveCaption(box.stats[t], "3b", "3B - ")
            self.PrintOffensiveCaption(box.stats[t], "hr", "HR - ")
            self.PrintOffensiveCaption(box.stats[t], "gdp", "GDP - ")
            self.PrintOffensiveCaption(box.stats[t], "hp", "HBP - ")
            self.PrintOffensiveCaption(box.stats[t], "sh", "SH - ")
            self.PrintOffensiveCaption(box.stats[t], "sf", "SF - ")
            self.PrintOffensiveCaption(box.stats[t], "ibb", "IBB - ")
            self.PrintOffensiveCaption(box.stats[t], "sb", "SB - ")
            self.PrintOffensiveCaption(box.stats[t], "cs", "CS - ")
            self.PrintOffensiveCaption(box.stats[t], "e", "E - ")
            self.PrintOffensiveCaption(box.stats[t], "pb", "PB - ")
            
            self.AppendText("\n")

    def PrintPitching(self, box):
        self.AppendText("\n                   PITCHING\n\n")
        for t in [0, 1]:
            ros = self.doc.GetRoster(t)
            self.AppendText("%-25s   ip  h hr  r er bb so\n" %
                                 ros.GetCity())

            for rec in box.pitching[t]:
                decision = ""
                if rec["id"] == self.doc.GetGame().GetWinningPitcher():
                    decision = " (W)"
                elif rec["id"] == self.doc.GetGame().GetLosingPitcher():
                    decision = " (L)"
                elif rec["id"] == self.doc.GetGame().GetSavePitcher():
                    decision = " (S)"
                self.AppendText("%-25s %2d.%1d %2d %2d %2d %2d %2d %2d\n" %
                                     (rec["name"] + decision,
                                      rec["outs"] / 3, rec["outs"] % 3,
                                      rec["h"], rec["hr"],
                                      rec["r"], rec["er"],
                                      rec["bb"], rec["so"]))

            self.AppendText("\n")
            
            self.PrintPitchingCaption(box.pitching[t], "ibb", "IBB - ")
            self.PrintPitchingCaption(box.pitching[t], "wp", "WP - ")
            self.PrintPitchingCaption(box.pitching[t], "bk", "BK - ")
            self.PrintPitchingCaption(box.pitching[t], "hb", "HBP - ")

            self.AppendText("\n")
        
    
class BoxscorePanel(wx.Panel, TextBoxscore):
    def __init__(self, parent, doc):
        wx.Panel.__init__(self, parent)
        TextBoxscore.__init__(self, doc)

        self.text = wx.TextCtrl(self, wx.ID_ANY, "",
                                style = wx.TE_MULTILINE | wx.TE_READONLY)
        self.text.SetFont(wx.Font(10, wx.MODERN, wx.NORMAL, wx.NORMAL))
         
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.text, 1, wx.EXPAND, 0)

        self.SetSizer(sizer)
        self.Layout()


    def OnUpdate(self):    self.Rebuild()
    def AppendText(self, x):  self.text.AppendText(x)

    def Rebuild(self):
        self.text.Clear()
        box = self.doc.GetBoxscore()
        self.PrintHeading(box)
        self.PrintLinescore(box)
        self.PrintBatting(box)
        self.PrintPitching(box)
        
if __name__ == "__main__":
    import sys
    import scorebook
    import dw
    import game

    book = dw.Reader(sys.argv[1])
    date = sys.argv[2]

    for g in book.Games():
        if g.GetDate() == date:
            doc = game.Game(book, g,
                            book.GetTeam(g.GetTeam(0)),
                            book.GetTeam(g.GetTeam(1)))
            doc.BuildBoxscore()
            boxwriter = TextBoxscore(doc)
            box = doc.GetBoxscore()

            boxwriter.PrintHeading(box)
            boxwriter.PrintLinescore(box)
            boxwriter.PrintBatting(box)
            boxwriter.PrintPitching(box)

            print "\f",
            
            
            
