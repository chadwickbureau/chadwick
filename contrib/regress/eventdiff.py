import tool

class EventData(dict):
    def __init__(self, d):
        dict.__init__(self, d)

    def _check_putouts(self, other):
        putoutsX = [ 0 for pos in range(10) ]
        putoutsY = [ 0 for pos in range(10) ]

        for key in [ "Putout1", "Putout2", "Putout3" ]:
            putoutsX[int(self[key])] += 1
            putoutsY[int(other[key])] += 1

        return putoutsX == putoutsY

    def _check_assists(self, other):
        assistsX = [ 0 for pos in range(10) ]
        assistsY = [ 0 for pos in range(10) ]

        for key in [ "Assist1", "Assist2", "Assist3", "Assist4", "Assist5" ]:
            assistsX[int(self[key])] += 1
            assistsY[int(other[key])] += 1

        return assistsX == assistsY

    def diff(self, other):
        diffkeys = [ ]
        for key in self:
            if key in [ "PR1", "PR2", "PR3" ]: continue
            if key == "RespPitcher1" and self["Runner1"] == "": continue
            if key == "RespPitcher2" and self["Runner2"] == "": continue
            if key == "RespPitcher3" and self["Runner3"] == "": continue
            if key.startswith("Putout") and self._check_putouts(other):
                continue
            if key.startswith("Assist") and self._check_assists(other):
                continue
            if self[key] != other[key]:
                diffkeys.append(key)
        return diffkeys
        

class EventToolProcess(tool.ToolProcess):
    def __init__(self, tool_path, tool_name, year):
        tool.ToolProcess.__init__(self, tool_path, tool_name, year)

    @property
    def command_line(self):
        return "%s -y %s -f 0-96 %s*.EV?" % (self.tool_path,
                                             self.year, self.year)

    @property
    def rowclass(self):  return EventData

    @property
    def header(self):
        return [ "GameID", "VisTeam", "Inning", "BattingTeam",
                 "Outs", "Balls", "Strikes", "Pitches", "VisScore", "HomeScore",
                 "Batter", "BatterHand", "ResBatter", "ResBatterHand",
                 "Pitcher", "PitcherHand", "ResPitcher", "ResPitcherHand",
                 "Catcher", "FirstBase", "SecondBase", "ThirdBase", "Shortstop",
                 "LeftField", "CenterField", "RightField",
                 "Runner1", "Runner2", "Runner3",
                 "EventText", "LeadoffFlag", "PinchhitFlag",
                 "DefensivePos", "LineupPos", "EventType", "BatterEventFlag",
                 "ABFlag", "HitValue", "SHFlag", "SFFlag", "OutsOnPlay",
                 "DPFlag", "TPFlag", "RBIOnPlay", "WPFlag", "PBFlag",
                 "FieldedBy", "BattedBallType", "BuntFlag", "FoulFlag",
                 "HitLocation", "NumErrors", "Error1Player", "Error1Type",
                 "Error2Player", "Error2Type", "Error3Player","Error3Type",
                 "BatterDest", "Runner1Dest", "Runner2Dest", "Runner3Dest",
                 "PlayOnBatter", "PlayOnRunner1", "PlayOnRunner2", "PlayOnRunner3",
                 "SB2Flag", "SB3Flag", "SBHFlag", "CS2Flag", "CS3Flag", "CSHFlag",
                 "PO1Flag", "PO2Flag", "PO3Flag",
                 "RespPitcher1", "RespPitcher2", "RespPitcher3", "NewGameFlag", "EndGameFlag",
                 "PR1", "PR2", "PR3", "PR1Removed", "PR2Removed", "PR3Removed",
                 "BatterRemoved", "PosBatterRemoved",
                 "Putout1", "Putout2", "Putout3",
                 "Assist1", "Assist2", "Assist3", "Assist4", "Assist5",
                 "EventNumber" ]


class EventDiff(object):
    def __init__(self, key, t1, t2):
        self.t1 = t1
        self.t2 = t2
        self.key = key
        
    @property
    def context(self):
        return "%s %2s %s %s" % (self.t1["GameID"], self.t1["Inning"],
                                 self.t1["Batter"], self.t1["EventText"])

    @property
    def tool1(self):
        return self.t1[self.key]

    @property
    def tool2(self):
        return self.t2[self.key]
        

class EventDiffEngine(tool.DiffEngine):
    @property
    def diff_object(self):  return EventDiff
    

def run_diff(tool1_path, tool2_path, data_path, year):
    tool1 = EventToolProcess(tool1_path, "Chadwick", year)
    tool2 = EventToolProcess(tool2_path, "BEVENT", year)
    engine = EventDiffEngine()
    tool.run_diff(engine, tool1, tool2, data_path)
    
if __name__ == "__main__":
    import sys
    run_diff(sys.argv[1], sys.argv[2], "/home/dataczar/git/retrosheet/event/regular",
             sys.argv[3])

    

