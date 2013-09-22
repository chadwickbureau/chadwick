import tool

class GameData(dict):
    def __init__(self, d):
        dict.__init__(self, d)

    def diff(self, other):
        return [ key for key in self if self[key] != other[key] ]


class GameToolProcess(tool.ToolProcess):
    def __init__(self, tool_path, tool_name, year):
        tool.ToolProcess.__init__(self, tool_path, tool_name, year)

    @property
    def command_line(self):
        return "%s -y %s -f 0-83 %s*.EV?" % (self.tool_path,
                                             self.year, self.year)

    @property
    def rowclass(self):  return GameData

    @property
    def header(self):
        return [ "GameID", "Date", "GameNumber", "DayOfWeek",
                 "StartTime", "DHUsed", "DayNight", "VisTeam", "HomeTeam",
                 "Site", "VisPitcher", "HomePitcher",
                 "UmpHome", "Ump1B", "Ump2B", "Ump3B", "UmpLF", "UmpRF",
                 "Attendance", "Scorer", "Translator", "Inputter",
                 "InputTime", "EditTime", "HowScored", "Pitches",
                 "Temperature", "WindDir", "WindSpeed", "FieldCond",
                 "Precipitation", "Sky",
                 "TimeOfGame", "Innings", "VisScore", "HomeScore",
                 "VisHits", "HomeHits", "VisErrors", "HomeErrors",
                 "VisLOB", "HomeLOB", "Win", "Loss", "Save", "GWRBI",
                 "VisBatter1", "VisPos1",
                 "VisBatter2", "VisPos2",
                 "VisBatter3", "VisPos3",
                 "VisBatter4", "VisPos4",
                 "VisBatter5", "VisPos5",
                 "VisBatter6", "VisPos6",
                 "VisBatter7", "VisPos7",
                 "VisBatter8", "VisPos8",
                 "VisBatter9", "VisPos9",
                 "HomeBatter1", "HomePos1",
                 "HomeBatter2", "HomePos2",
                 "HomeBatter3", "HomePos3",
                 "HomeBatter4", "HomePos4",
                 "HomeBatter5", "HomePos5",
                 "HomeBatter6", "HomePos6",
                 "HomeBatter7", "HomePos7",
                 "HomeBatter8", "HomePos8",
                 "HomeBatter9", "HomePos9",
                 "VisFinishPitcher", "HomeFinishPitcher" ]


class GameDiff(object):
    def __init__(self, key, t1, t2):
        self.t1 = t1
        self.t2 = t2
        self.key = key
        
    @property
    def context(self):
        return self.t1["GameID"]

    @property
    def tool1(self):
        return self.t1[self.key]

    @property
    def tool2(self):
        return self.t2[self.key]
        

class GameDiffEngine(tool.DiffEngine):
    @property
    def diff_object(self):  return GameDiff
    

def run_diff(tool1_path, tool2_path, data_path, year):
    tool1 = GameToolProcess(tool1_path, "Chadwick", year)
    tool2 = GameToolProcess(tool2_path, "BGAME", year)
    engine = GameDiffEngine()
    tool.run_diff(engine, tool1, tool2, data_path)
    
if __name__ == "__main__":
    import sys
    run_diff(sys.argv[1], sys.argv[2],
             "/home/dataczar/git/retrosheet/event/regular",
             sys.argv[3])
           
