
import statline
import libchadwick as cw

class TeamRecord(object):
    def __init__(self, team):
        self.stats = { "team": team, "id": team.GetID(),
                       "city": team.GetCity(), "nickname": team.GetNickname(),
                       "g":0, "w":0, "l":0, "t":0,
                       "hw":0, "hl":0,  # home record
                       "rw":0, "rl":0,  # road record
                       "ow":0, "ol":0,  # one-run games record
                       "xw":0, "xl":0}  # extra-inning games record

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

    def __getattr__(self, attr):
        if attr == "stats":
            raise AttributeError
        elif attr == "pct":
            try:
                return 1.0*self.w/(self.w+self.l)
            except ZeroDivisionError:
                return None
        elif attr == "full_name":
            return " ".join([self.city, self.nickname])
        else:
            return self.stats[attr.lower()]

    def __setattr__(self, attr, value):
        if attr == "stats":
            object.__setattr__(self, attr, value)
        else:
            self.stats[attr.lower()] = value

    def __repr__(self):  return repr(self.stats)


class Standings:
    def __init__(self, book):
        self.stats = { }
        for team in book.Teams():
            self.stats[team.GetID()] = TeamRecord(team)

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass
    def OnEvent(self, game, gameiter):  pass

    def OnEndGame(self, game, gameiter):
        ids = game.GetTeams()
        scores = [ gameiter.GetTeamScore(t) for t in [0,1] ]

        self.stats[ids[0]]["g"] += 1
        self.stats[ids[1]]["g"] += 1

        if scores[0] > scores[1]:
            win = 0
        elif scores[1] > scores[0]:
            win = 1
        else:
            win = -1

        if win >= 0:
            self.stats[ids[win]]["w"] += 1
            self.stats[ids[1-win]]["l"] += 1

            if scores[0]-scores[1] in [-1, 1]:
                self.stats[ids[win]]["ow"] += 1
                self.stats[ids[1-win]]["ol"] += 1

            if gameiter.state.inning >= 10:
                self.stats[ids[win]]["xw"] += 1
                self.stats[ids[1-win]]["xl"] += 1

        if win == 0:
            self.stats[ids[0]]["rw"] += 1
            self.stats[ids[1]]["hl"] += 1
        elif win == 1:
            self.stats[ids[0]]["rl"] += 1
            self.stats[ids[1]]["hw"] += 1

    def __str__(self):
        teams = self.stats.values()
        teams.sort(lambda x, y: cmp(y.pct if y.pct is not None else 0.5,
                                    x.pct if x.pct is not None else 0.5))

        s = "\nClub                             G   W-  L    PCT   GB  HOME  AWAY  1RUN  XINN\n";
        for (i, stat) in enumerate(teams):
            GB2 = (teams[0].w-stat.w-teams[0].l+stat.l)
            if GB2 == 0:
                GB = "    "
            else:
                GB = "%4.1f" % (GB2*0.5)
                
            s += ("%-30s %3d %3d-%3d  %s %s %2d-%2d %2d-%2d %2d-%2d %2d-%2d\n" %
                (stat.full_name,
                 stat.g, stat.w, stat.l,
                 ("%5.3f" % stat.pct).replace("0.", " .") if stat.pct is not None else "   - ",
                 GB,
                 stat.hw, stat.hl,
                 stat.rw, stat.rl,
                 stat.ow, stat.ol,
                 stat.xw, stat.xl))

        return s

    def html(self, d, firsthalf=None):
        teams = self.stats.values()
        teams.sort(lambda x, y: cmp(y.pct if y.pct is not None else 0.5,
                                    x.pct if x.pct is not None else 0.5))

        s = '<table width=75% class=table-nolines cellborder=0>'
        s += '<tr><td colspan=9 nowrap valign=top style=background-color:#cccccc;border-bottom:1px solid #666666><b>Standing of clubs through games of %s</b></td></tr>' % d

        s += '<tr><th></th><th>W - L</th><th>PCT</th><th>GB</th><th>Home</th><th>Away</th><th>1-run</th><th>Extra</th></tr>'
        for (i, stat) in enumerate(teams):
            s += '<tr>'
            GB2 = (teams[0].w-stat.w-teams[0].l+stat.l)
            if GB2 == 0:
                GB = ''
            else:
                GB = '%4.1f' % (GB2*0.5)

            s += ("<td><b>%s</b></td> <td align=center>%3d-%3d</td> <td align=center>%s</td><td align=center>%s</td><td align=center>%2d-%2d</td><td align=center>%2d-%2d</td><td align=center>%2d-%2d</td><td align=center>%2d-%2d</td>\n" %
                (("x-"+stat.full_name) if stat.id==firsthalf else stat.full_name,
                 stat.w, stat.l,
                 ("%5.3f" % stat.pct).replace("0.", " .") if stat.pct is not None else "   - ",
                 GB,
                 stat.hw, stat.hl,
                 stat.rw, stat.rl,
                 stat.ow, stat.ol,
                 stat.xw, stat.xl))
            s += '</tr>'
        if firsthalf is not None:
            s += '<tr><td colspan=9 nowrap valign=top style=background-color:#cccccc;border-bottom:1px solid #666666><b>x</b>: First-half champion</td></tr>'

        s += '</table>'
        return s



class TeamBattingTotals:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        for team in book.Teams():
            self.stats[team.GetID()] = statline.Batting(team=team)

    def OnBeginGame(self, game, gameiter):
        for team in game.GetTeams():
            self.stats[team]["games"].add(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()

        batter = self.stats[game.GetTeams()[team]]
        batter.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "":  continue
            batter.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        for t in [0,1]:
            self.stats[game.GetTeams()[t]].lob += gameiter.GetTeamLOB(t)

    def __str__(self):
        teams = self.stats.values()
        teams.sort(lambda x, y: cmp(y.avg, x.avg))

        s = ""
        for (i, stat) in enumerate(teams):
            if i % 20 == 0:
                s += "\nClub              AVG   SLG   OBP   G   AB   R    H  2B 3B  HR RBI  BB IW   SO GDP HP SH SF  SB CS  LOB\n"

            
            s += ("%-15s %s %s %s %3d %4d %3d %4d %3d %2d %3d %3d %3d %2d %4d %3d %2d %2d %2d %3d %2d %4d\n" %
                (stat.team.GetCity(),
                 ("%5.3f" % stat.avg).replace("0.", " .") if stat.avg is not None else "   - ",
                 ("%5.3f" % stat.slg).replace("0.", " .") if stat.slg is not None else "   - ",
                 ("%5.3f" % stat.obp).replace("0.", " .") if stat.obp is not None else "   - ",
                 len(stat.games),
                 stat.ab, stat.r, stat.h,
                 stat.h2b, stat.h3b, stat.hr, stat.bi,
                 stat.bb, stat.ibb, stat.so, stat.gdp, stat.hp,
                 stat.sh, stat.sf, stat.sb, stat.cs, stat.lob))

        return s


class TeamPitchingTotals:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        for team in book.Teams():
            self.stats[team.GetID()] = statline.Pitching(team=team)

    def OnBeginGame(self, game, gameiter):
        for team in game.GetTeams():
            self.stats[team].games.add(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()

        pitcher = self.stats[game.GetTeams()[1-team]]

        pitcher.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "": continue
            pitcher.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        teams = game.GetTeams()
        if gameiter.GetTeamScore(0) > gameiter.GetTeamScore(1):
            self.stats[teams[0]].w += 1
            self.stats[teams[1]].l += 1
            if game.GetSavePitcher() != "":
                self.stats[teams[0]]["sv"] += 1
        elif gameiter.GetTeamScore(0) < gameiter.GetTeamScore(1):
            self.stats[teams[1]]["w"] += 1
            self.stats[teams[0]]["l"] += 1
            if game.GetSavePitcher() != "":
                self.stats[teams[1]]["sv"] += 1

        for t in [0, 1]:
            startP = game.GetStarterAtPos(t, 1).player_id
            endP = gameiter.GetFielder(t, 1)
            if startP == endP:
                # TODO: It's possible but rare to start and end game but
                # not pitch a complete game!
                self.stats[teams[t]]["cg"] += 1
            if gameiter.GetTeamScore(1-t) == 0:
                self.stats[teams[t]]["sho"] += 1

    def __str__(self):
        teams = self.stats.values()
        teams.sort(lambda x, y: cmp(x.era if x.era is not None else 9999.99,
                                    y.era if y.era is not None else 9999.99))

        s = ""

        for (i, stat) in enumerate(teams):
            if i % 20 == 0:
                s += "\nClub              W-  L   PCT   ERA   G CG SHO SV     IP  TBF   AB   H   R  ER  HR SH SF  BB IW   SO BK WP HB\n"
            s += ("%-15s %3d-%3d %s %s %3d %2d %3d %2d %4d.%1d %4d %4d %3d %3d %3d %3d %2d %2d %3d %2d %4d %2d %2d %2d\n" %
                 (stat.team.GetCity(), stat.w, stat.l,
                  ("%5.3f" % stat.pct).replace("0.", " .") if stat.pct is not None else "   - ",
                  ("%5.2f" % stat.era) if stat.era is not None else "   - ",
                  len(stat.games),
                  stat.cg, stat.sho, stat.sv,
                  stat.outs / 3, stat.outs % 3, stat.bf, stat.ab,
                  stat.h, stat.r, stat.er-stat.tur, stat.hr,
                  stat.sh, stat.sf,
                  stat.bb, stat.ibb, stat.so,
                  stat.bk, stat.wp, stat.hb))

        return s
        

class TeamFieldingTotals:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        for team in book.Teams():
            self.stats[team.GetID()] = statline.TeamFielding(team=team)

    def OnBeginGame(self, game, gameiter):
        for team in game.GetTeams():
            self.stats[team].games.add(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        pass

    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()
        fielder = self.stats[game.GetTeams()[1-team]]
        fielder.ProcessFielding(eventData)

    def OnEndGame(self, game, gameiter):
        pass

    def __str__(self):
        teams = self.stats.values()
        teams.sort(lambda x, y: cmp(y.pct, x.pct))

        s = ""
        for (i, stat) in enumerate(teams):
            if i % 20 == 0:
                s += "\nClub              PCT   G   PO    A   E  DP TP  BIP   BF   DER  SB  CS   SB% PB\n"
            
            s += ("%-15s %s %3d %4d %4d %3d %3d %2d %4d %4d %s %3d %3d %s %2d\n" %
                (stat.team.GetCity(),
                 ("%5.3f" % stat.pct).replace("0.", " .") if stat.pct is not None else "   - ", 
                 len(stat.games), stat.po, stat.a, stat.e,
                 stat.dp, stat.tp, stat.bip, stat.bf,
                 ("%5.3f" % stat.der).replace("0.", " .") if stat.der is not None else "   - ",
                 stat.sb, stat.cs,
                 ("%5.3f" % stat.sbpct).replace("0.", " .") if stat.sbpct is not None else "   - ",
                 stat.pb))

        return s


class TeamGameLog:
    def __init__(self, cwf):
        self.cwf = cwf
        self.stats = { }
        for team in cwf.Teams():
            self.stats[team.GetID()] = [ ]

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass
    def OnEvent(self, game, gameiter):  pass

    def OnEndGame(self, game, gameiter):
        ids = game.GetTeams()
        scores = [ gameiter.GetTeamScore(t) for t in [0,1] ]

        for t in [0,1]:
            self.stats[ids[t]].append({ "date": game.GetDate(),
                                        "number": game.GetNumber(),
                                        "teams": ids,
                                        "scores": scores,
                                        "innings": game.GetInnings(),
                                        "wp": game.GetWinningPitcher(),
                                        "lp": game.GetLosingPitcher(),
                                        "save": game.GetSavePitcher() })
   
    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        s = ""

        for team in keys:
            self.stats[team].sort(lambda x, y: cmp(x["date"] + str(x["number"]),
                                                   y["date"] + str(y["number"])))
            roster = self.cwf.GetTeam(team)

            s += "\nGame log for %s\n" % roster.GetName()

            wins = 0
            losses = 0
            
            for (i,game) in enumerate(self.stats[team]):
                if i % 20 == 0:
                    s += "\nDate       #  Opponent Result         Record  Win              Loss             Save\n"
                
                if game["wp"] != "":
                    wp = self.cwf.GetPlayer(game["wp"])
                    wpname = wp.GetFirstName()[:1] + ". " + wp.GetLastName()
                else:
                    wpname = ""
                    
                if game["lp"] != "":
                    lp = self.cwf.GetPlayer(game["lp"])
                    lpname = lp.GetFirstName()[:1] + ". " + lp.GetLastName()
                else:
                    lpname = ""
                    
                if game["save"] != "":
                    save = self.cwf.GetPlayer(game["save"])
                    savename = save.GetFirstName()[:1] + ". " + save.GetLastName()
                else:
                    savename = ""

                if game["innings"] == 9:
                    inningStr = ""
                else:
                    inningStr = "(%d)" % game["innings"]
                    
                if game["teams"][0] == team:
                    # Visitors
                    if game["scores"][0] > game["scores"][1]:
                        dec = "W"
                        wins += 1
                    elif game["scores"][0] < game["scores"][1]:
                        dec = "L"
                        losses += 1
                    else:
                        dec = " "
                
                    s += ("%s %s   at %s  %s %2d-%2d %-4s  %3d-%3d  %-16s %-16s %-16s\n" %
                          (game["date"], [ " ", "1", "2" ][game["number"]],
                           game["teams"][1], dec, 
                           game["scores"][0], game["scores"][1], inningStr,
                           wins, losses, wpname, lpname, savename))
                else:
                    # Home team
                    if game["scores"][0] > game["scores"][1]:
                        dec = "L"
                        losses += 1
                    elif game["scores"][0] < game["scores"][1]:
                        dec = "W"
                        wins += 1
                    else:
                        dec = " "

                    s += ("%s %s   vs %s  %s %2d-%2d %-4s  %3d-%3d  %-16s %-16s %-16s\n" %
                          (game["date"], [ " ", "1", "2" ][game["number"]],
                           game["teams"][0], dec,
                           game["scores"][1], game["scores"][0], inningStr,
                           wins, losses, wpname, lpname, savename))
                    
            s += "\n"

        return s

