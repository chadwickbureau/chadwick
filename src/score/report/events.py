class GrandSlamLog:
    """
    Compiles a list of all grand slams hit.
    """
    def __init__(self, cwf):
        self.cwf = cwf
        self.stats = [ ]

    def OnBeginGame(self, game, gameiter):  pass
    def OnSubstitution(self, game, gameiter): pass

    def OnEvent(self, game, gameiter):
        if (gameiter.GetEventData().event_type == cw.EVENT_HOMERUN and
            gameiter.GetRunner(1) != "" and
            gameiter.GetRunner(2) != "" and
            gameiter.GetRunner(3) != ""):
            inning = gameiter.GetInning()
            halfInning = gameiter.GetHalfInning()
            if halfInning == 0:
                team = game.GetTeams()[0]
                opp = game.GetTeams()[1]
                site = opp
            else:
                team = game.GetTeams()[1]
                opp = game.GetTeams()[0]
                site = team
                
            self.stats.append({ "date": game.GetDate(),
                                "number": game.GetNumber(),
                                
                                "inning": inning,
                                "halfInning": halfInning,
                                "batter": gameiter.GetBatter(),
                                "pitcher": gameiter.GetFielder(1-halfInning, 1),
                                "team": team, "opp": opp, "site": site })
        
    def OnEndGame(self, game, gameiter):  pass
   
    def __str__(self):
        self.stats.sort(lambda x, y: cmp(x["date"] + str(x["number"]),
                                         y["date"] + str(y["number"])))

        s = "\nDate       # Site Batter               Pitcher           Inning\n"
        for rec in self.stats:
            if rec["number"] == "0":  rec["number"] = " "
            batter = self.cwf.GetPlayer(rec["batter"])
            pitcher = self.cwf.GetPlayer(rec["pitcher"])

            s += ("%s %s %s  %-20s %-20s %d\n" % 
                  (rec["date"], rec["number"], rec["site"],
                   batter.GetFirstName()[0] + ". " + batter.GetLastName() + " (" + rec["team"] + ")",
                   pitcher.GetFirstName()[0] + ". " + pitcher.GetLastName() + " (" + rec["opp"] + ")",
                   rec["inning"]))
        return s

