import statline
import libchadwick as cw

class Batting:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        self.sorter = lambda x, y: cmp([ x.player.GetLastName(),
                                         x.player.GetFirstName() ],
                                       [ y.player.GetLastName(),
                                         y.player.GetFirstName() ])

    def GetName(self):  return "batting-register"

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)
                key = (player.player_id, game.GetTeam(t))
                if key not in self.stats:
                    self.stats[key] = statline.Batting(player=self.book.GetPlayer(key[0]),
                                                       team=self.book.GetTeam(key[1]))
            
                self.stats[key].games.add(game.GetGameID())

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec is not None:
            key = (rec.player_id, game.GetTeam(rec.team))
            if key not in self.stats:
                self.stats[key] = statline.Batting(player=self.book.GetPlayer(key[0]),
                                                   team=self.book.GetTeam(key[1]))
                
            self.stats[key].games.add(game.GetGameID())
            rec = rec.next


    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()
        
        batterId = gameiter.GetPlayer(team,
                                      gameiter.NumBatters(team) % 9 + 1)
        batter = self.stats[(batterId, game.GetTeam(team))]
        batter.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "":  continue
            runner = self.stats[(gameiter.GetRunner(base), game.GetTeam(team))]
            runner.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        pass

    def filter(self, crit):
        report = Batting(self.book)
        for (key, value) in self.stats.iteritems():
            if crit(value):
                report.stats[key] = value
        return report

    def __str__(self):
        stats = self.stats.values()
        stats.sort(self.sorter)

        s = ""
        for (i, stat) in enumerate(stats):
            if i % 20 == 0:
                s += "\nPlayer                Club    AVG   SLG   OBP   G  AB   R   H  TB 2B 3B HR RBI  BB IW  SO DP HP SH SF SB CS\n"
            

            if stat.player.GetBats() == "R":
                bats = " "
            elif stat.player.GetBats() == "L":
                bats = "*"
            elif stat.player.GetBats() == "S" or stat.player.GetBats() == "B":
                bats = "#"
            else:
                bats = " "
                
            s += ("%s%-19s   %3s  %s %s %s %3d %3d %3d %3d %3d %2d %2d %2d %3d %3d %2d %3d %2d %2d %2d %2d %2d %2d\n" %
                (bats, stat.player.GetSortName(), stat.team.GetID(),
                 ("%5.3f" % stat.avg).replace("0.", " .")
                  if stat.avg is not None else "   - ",
                 ("%5.3f" % stat.slg).replace("0.", " .")
                  if stat.slg is not None else "   - ",
                 ("%5.3f" % stat.obp).replace("0.", " .")
                  if stat.obp is not None else "   - ",
                 len(stat.games),
                 stat.ab, stat.r, stat.h, stat.tb,
                 stat.h2b, stat.h3b, stat.hr, stat.bi,
                 stat.bb, stat.ibb, stat.so, stat.gdp, stat.hp,
                 stat.sh, stat.sf, stat.sb, stat.cs))

        return s

    def __repr__(self):  return repr(self.stats)

    def derepr(self, text):
        x = eval(text)
        self.stats = { }
        for key in x:
            self.stats[key] = BattingStatline()
            self.stats[key].stats = x[key]


class Pitching:
    def __init__(self, book):
        self.stats = { }
        self.book = book
        self.sorter = lambda x, y: cmp([ x.player.GetLastName(),
                                         x.player.GetFirstName() ],
                                       [ y.player.GetLastName(),
                                         y.player.GetFirstName() ])

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)

                if player.pos == 1:
                    key = (player.player_id, game.GetTeam(t))
                    if key not in self.stats:
                        self.stats[key] = statline.Pitching(self.book.GetPlayer(key[0]),
                                                            self.book.GetTeam(key[1]))
                    
                    self.stats[key].games.add(game.GetGameID())
                    self.stats[key].gs += 1

            if game.GetStarter(t, 0) is not None:
                player = game.GetStarter(t, 0)
                key = (player.player_id, game.GetTeam(t))
                
                if key not in self.stats:
                    self.stats[key] = statline.Pitching(self.book.GetPlayer(key[0]),
                                                        self.book.GetTeam(key[1]))
                self.stats[key].games.add(game.GetGameID())
                self.stats[key].gs += 1
                

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec is not None:
            if rec.pos == 1:
                key = (rec.player_id, game.GetTeam(rec.team))
                if key not in self.stats:
                    self.stats[key] = statline.Pitching(self.book.GetPlayer(key[0]),
                                                        self.book.GetTeam(key[1]))
                self.stats[key].games.add(game.GetGameID())

            rec = rec.next

    def OnEvent(self, game, gameiter):
        team = gameiter.GetHalfInning()

        pitcherId = gameiter.GetFielder(1-team, 1)
        pitcher = self.stats[(pitcherId, game.GetTeam(1-team))]

        eventData = gameiter.GetEventData()
        pitcher.ProcessBatting(eventData)

        for base in [1,2,3]:
            if gameiter.GetRunner(base) == "": continue
            
            resppitcher = self.stats[(gameiter.GetRespPitcher(base),
                                      game.GetTeam(1-team))]
            resppitcher.ProcessRunning(eventData, base)

    def OnEndGame(self, game, gameiter):
        if gameiter.GetTeamScore(0) > gameiter.GetTeamScore(1):
            win_team = game.GetTeam(0)
            loss_team = game.GetTeam(1)
        else:
            win_team = game.GetTeam(1)
            loss_team = game.GetTeam(0)
            
        if game.GetWinningPitcher() != "":
            self.stats[(game.GetWinningPitcher(), win_team)].w += 1
        if game.GetLosingPitcher() != "":
            self.stats[(game.GetLosingPitcher(), loss_team)].l += 1
        if game.GetSavePitcher() != "":
            self.stats[(game.GetSavePitcher(), win_team)].sv += 1

        for t in [0, 1]:
            startP = game.GetStarterAtPos(t, 1).player_id
            endP = gameiter.GetFielder(t, 1)
            if startP == endP:
                # TODO: It's possible but rare to start and end game but
                # not pitch a complete game!
                self.stats[(startP, game.GetTeam(t))].cg += 1
                if gameiter.GetTeamScore(1-t) == 0:
                    self.stats[(startP, game.GetTeam(t))].sho += 1
            else:
                self.stats[(endP, game.GetTeam(t))].gf += 1

    def filter(self, crit):
        report = Pitching(self.book)
        for (key, value) in self.stats.iteritems():
            if crit(value):
                report.stats[key] = value
        return report

    def __str__(self):
        stats = self.stats.values()
        stats.sort(self.sorter)

        s = ""
        for (i, stat) in enumerate(stats):
            if stat.player.GetThrows() == "R":
                throws = " "
            elif stat.player.GetThrows() == "L":
                throws = "*"
            else:
                throws = " "

            if i % 20 == 0:
                s += "\nPlayer                Club   W- L   PCT    ERA  G GS CG SHO GF SV    IP TBF  AB   H HR SH SF HB  BB IW  SO WP BK\n"
            s += ("%s%-20s  %3s  %2d-%2d %s %s %2d %2d %2d  %2d %2d %2d %3d.%1d %3d %3d %3d %2d %2d %2d %2d %3d %2d %3d %2d %2d\n" % 
                (throws, stat.player.GetSortName(), stat.team.GetID(),
                 stat.w, stat.l,
                 ("%.3f" % stat.pct).replace("0.", " .")
                 if stat.pct is not None else "   - ",
                 ("%6.2f" % stat.era) if stat.era is not None else "    - ",
                 len(stat.games),
                 stat.gs, stat.cg, stat.sho, stat.gf, stat.sv,
                 stat.outs / 3, stat.outs % 3, stat.bf, stat.ab, stat.h,
                 stat.hr, stat.sh, stat.sf, stat.hb, stat.bb, stat.ibb,
                 stat.so, stat.wp, stat.bk))


        return s

class Fielding:
    def __init__(self, book, pos):
        self.stats = { }
        self.book = book
        self.pos = pos

    def OnBeginGame(self, game, gameiter):
        for t in [0, 1]:
            for slot in range(9):
                player = game.GetStarter(t, slot+1)

                if player.pos == self.pos:
                    if player.player_id not in self.stats:
                        self.stats[player.player_id] = FieldingStatline()
                    
                    if game.GetGameID() not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.GetGameID())
                    self.stats[player.player_id]["gs"] += 1

            if self.pos == 1 and game.GetStarter(t, 0) != None:
                player = game.GetStarter(t, 0)
                
                if player.player_id not in self.stats:
                    self.stats[player.player_id] = FieldingStatline()
                    if game.GetGameID() not in self.stats[player.player_id]["games"]:
                        self.stats[player.player_id]["games"].append(game.GetGameID())
                    self.stats[player.player_id]["gs"] += 1

    def OnSubstitution(self, game, gameiter):
        rec = gameiter.event.first_sub

        while rec != None:
            if rec.pos == self.pos:
                if rec.player_id not in self.stats:
                    self.stats[rec.player_id] = FieldingStatline()
                if game.GetGameID() not in self.stats[rec.player_id]["games"]:
                    self.stats[rec.player_id]["games"].append(game.GetGameID())

            rec = rec.next


    def OnEvent(self, game, gameiter):
        eventData = gameiter.GetEventData()
        team = gameiter.GetHalfInning()

        fielderId = gameiter.GetFielder(1-team, self.pos)
        fielder = self.stats[fielderId]

        fielder.ProcessFielding(eventData, self.pos)

    def OnEndGame(self, game, gameiter):
        pass

    def __str__(self):
        keys = self.stats.keys()
        keys.sort()

        posStr = [ "Pitcher", "Catcher", "First base",
                   "Second base", "Third base",
                   "Shortstop", "Left field", "Center field",
                   "Right field" ][self.pos - 1]
                   
        s = ""
        for (i,key) in enumerate(keys):
            stat = self.stats[key]
            player = self.book.GetPlayer(key)
            
            if i % 20 == 0:
                if self.pos == 2:
                    s += "\n%-20s   PCT   G  GS    INN   PO   A  E  DP TP  BIP  BF PB\n" % posStr
                else:
                    s += "\n%-20s   PCT   G  GS    INN   PO   A  E  DP TP  BIP  BF\n" % posStr

            if self.pos == 2:
                s += ("%-20s %s %3d %3d %4d.%1d %4d %3d %2d %3d %2d %4d %3d %2d\n" %
                      (player.GetSortName(),
                       FormatAverage(stat["po"] + stat["a"],
                                     stat["po"] + stat["a"] + stat["e"]),
                       len(stat["games"]), stat["gs"],
                       stat["outs"] / 3, stat["outs"] % 3,
                       stat["po"], stat["a"], stat["e"],
                       stat["dp"], stat["tp"],
                       stat["bip"], stat["bf"], stat["pb"]))
            else:
                s += ("%-20s %s %3d %3d %4d.%1d %4d %3d %2d %3d %2d %4d %3d\n" %
                      (player.GetSortName(),
                       FormatAverage(stat["po"] + stat["a"],
                                     stat["po"] + stat["a"] + stat["e"]),
                       len(stat["games"]), stat["gs"],
                       stat["outs"] / 3, stat["outs"] % 3,
                       stat["po"], stat["a"], stat["e"],
                       stat["dp"], stat["tp"],
                       stat["bip"], stat["bf"]))
        return s
