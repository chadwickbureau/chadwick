import libchadwick as cw

class Batting(object):
    def __init__(self, player=None, team=None):
        self.stats = { "player": player, "team": team,
                       "games": set(),
                       "ab":0, "r":0, "h":0,
                       "h2b":0, "h3b":0, "hr":0, "bi":0,
                       "bb":0, "ibb":0, "so":0,
                       "gdp":0, "hp":0, "sh":0, "sf":0,
                       "sb":0, "cs":0, "lob":0 }

    def ProcessBatting(self, eventData):
        if eventData.IsOfficialAB(): self.stats["ab"] += 1

        if eventData.event_type == cw.EVENT_SINGLE:
            self.h += 1
        elif eventData.event_type == cw.EVENT_DOUBLE:
            self.h += 1
            self.h2b += 1
        elif eventData.event_type == cw.EVENT_TRIPLE:
            self.h += 1
            self.h3b += 1
        elif eventData.event_type == cw.EVENT_HOMERUN:
            self.h += 1
            self.hr += 1
        elif eventData.event_type == cw.EVENT_WALK:
            self.bb += 1
        elif eventData.event_type == cw.EVENT_INTENTIONALWALK:
            self.bb += 1
            self.ibb += 1
        elif eventData.event_type == cw.EVENT_STRIKEOUT:
            self.so += 1
        elif eventData.event_type == cw.EVENT_HITBYPITCH:
            self.hp += 1

        self.bi += eventData.GetRBI()

        if eventData.sh_flag > 0:
            self.sh += 1
        if eventData.sf_flag > 0:
            self.sf += 1
        if eventData.gdp_flag > 0:
            self.gdp += 1

        if eventData.GetAdvance(0) >= 4: self.r += 1

    def ProcessRunning(self, eventData, base):
        destBase = eventData.GetAdvance(base)
        if destBase >= 4:  self.stats["r"] += 1
        if eventData.GetSBFlag(base) > 0: self.stats["sb"] += 1
        if eventData.GetCSFlag(base) > 0: self.stats["cs"] += 1

    def __add__(self, x):
        y = BattingStatline()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

    def __getattr__(self, attr):
        if attr == "stats":
            raise AttributeError
        elif attr == "avg":
            try:
                return 1.0*self.h/self.ab
            except ZeroDivisionError:
                return None
        elif attr == "pa":
            return self.ab+self.bb+self.hp+self.sh+self.sf
        elif attr == "tb":
            return self.h+self.h2b+2*self.h3b+3*self.hr
        elif attr == "slg":
            try:
                return 1.0*self.tb/self.ab
            except ZeroDivisionError:
                return None
        elif attr == "obp":
            try:
                return 1.0*(self.h+self.bb+self.hp)/(self.ab+self.bb+self.hp+self.sf)
            except ZeroDivisionError:
                return None
        else:
            return self.stats[attr.lower()]

    def __setattr__(self, attr, value):
        if attr == "stats":
            object.__setattr__(self, attr, value)
        else:
            self.stats[attr.lower()] = value

    def __repr__(self):  return repr(self.stats)

class Pitching(object):
    def __init__(self, player=None, team=None):
        self.stats = { "player": player, "team": team, "games": set(),
                       "gs":0, "cg":0, "sho":0, "gf":0,
                       "w":0, "l":0, "sv":0,
                       "outs":0, "bf":0, "r":0, "er":0, "tur":0,
                       "ab": 0, "h":0, "h2b":0, "h3b":0, "hr":0,
                       "sh": 0, "sf": 0,
                       "bb":0, "ibb":0, "so":0,
                       "wp":0, "bk":0, "hb":0 }

    def ProcessBatting(self, eventData):
        self.stats["outs"] += eventData.GetOuts()

        if eventData.IsBatterEvent():
            self.stats["bf"] += 1
            if eventData.IsOfficialAB():  self.ab += 1

        if eventData.event_type == cw.EVENT_SINGLE:
            self.stats["h"] += 1
        elif eventData.event_type == cw.EVENT_DOUBLE:
            self.stats["h"] += 1
            self.stats["h2b"] += 1
        elif eventData.event_type == cw.EVENT_TRIPLE:
            self.stats["h"] += 1
            self.stats["h3b"] += 1
        elif eventData.event_type == cw.EVENT_HOMERUN:
            self.stats["h"] += 1
            self.stats["hr"] += 1
        elif eventData.event_type == cw.EVENT_WALK:
            self.stats["bb"] += 1
        elif eventData.event_type == cw.EVENT_INTENTIONALWALK:
            self.stats["bb"] += 1
            self.stats["ibb"] += 1
        elif eventData.event_type == cw.EVENT_STRIKEOUT:
            self.stats["so"] += 1
        elif eventData.event_type == cw.EVENT_HITBYPITCH:
            self.stats["hb"] += 1
        elif eventData.event_type == cw.EVENT_BALK:
            self.stats["bk"] += 1

        if eventData.wp_flag > 0:
            self.stats["wp"] += 1
        if eventData.sh_flag > 0:   self.sh += 1
        if eventData.sf_flag > 0:   self.sf += 1

        destBase = eventData.GetAdvance(0)
        if destBase < 4: return

        if destBase == 4:
            self.stats["r"] += 1
            self.stats["er"] += 1
        elif destBase == 5:
            self.stats["r"] += 1
        elif destBase == 6:
            self.stats["r"] += 1
            self.stats["er"] += 1
            self.stats["tur"] += 1

    def ProcessRunning(self, eventData, base):
        destBase = eventData.GetAdvance(base)
        if destBase < 4: return

        if destBase == 4:
            self.stats["r"] += 1
            self.stats["er"] += 1
        elif destBase == 5:
            self.stats["r"] += 1
        elif destBase == 6:
            self.stats["r"] += 1
            self.stats["er"] += 1
            self.stats["tur"] += 1
        
    def __add__(self, x):
        y = PitchingStatline()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

    def __getattr__(self, attr):
        if attr == "stats":
            raise AttributeError
        elif attr == "era":
            try:
                return 27.0*self.er/self.outs
            except ZeroDivisionError:
                return None
        elif attr == "pct":
            try:
                return 1.0*self.w/(self.w+self.l)
            except ZeroDivisionError:
                return None
        else:
            return self.stats[attr.lower()]

    def __setattr__(self, attr, value):
        if attr == "stats":
            object.__setattr__(self, attr, value)
        else:
            self.stats[attr.lower()] = value



class Fielding(object):
    def __init__(self, player=None, team=None):
        self.stats = { "player": player, "team": team, "games": set(),
                       "gs":0, "outs":0, "bip":0, "bf":0,
                       "po":0, "a":0, "e":0, "dp":0, "tp":0, "pb":0,
                       "sb":0, "cs":0 }

    def ProcessFielding(self, eventData, pos):
        self.outs += eventData.GetOuts()
        po = eventData.GetPutouts(pos)
        self.po += po
        a = eventData.GetAssists(pos)
        self.a += a
        self.e += eventData.GetErrors(pos)

        if eventData.dp_flag and po + a > 0:
            self.dp += 1
        elif eventData.tp_flag and po + a > 0:
            self.tp += 1

        if eventData.fielded_by == pos and eventData.GetOuts() > 0:
            self.bf += 1
        if (eventData.fielded_by > 0 or
            eventData.event_type in [cw.EVENT_SINGLE,
                                     cw.EVENT_DOUBLE,
                                     cw.EVENT_TRIPLE]):
            self.bip += 1

        if eventData.pb_flag:    self.pb += 1
        for base in [1, 2, 3]:
            if eventData.GetSBFlag(base):  self.sb += 1
            if eventData.GetCSFlag(base):  self.cs += 1

        
    def __add__(self, x):
        y = Fielding(self.player, self.team)
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

    def __getattr__(self, attr):
        if attr == "stats":
            raise AttributeError
        elif attr == "pct":
            try:
                return 1.0*(self.po+self.a)/(self.po+self.a+self.e)
            except ZeroDivisionError:
                return None
        elif attr == "rf":
            try:
                return 27.0*self.bf/self.outs
            except ZeroDivisionError:
                return None
        elif attr == "sbpct":
            try:
                return 1.0*self.sb/(self.sb+self.cs)
            except ZeroDivisionError:
                return None
        else:
            return self.stats[attr.lower()]

    def __setattr__(self, attr, value):
        if attr == "stats":
            object.__setattr__(self, attr, value)
        else:
            self.stats[attr.lower()] = value


class TeamFielding(object):
    def __init__(self, team):
        self.stats = { "team": team, "games": set(),
                       "gs":0, "outs":0, "bip":0, "bf":0,
                       "po":0, "a":0, "e":0, "dp":0, "tp":0, "pb":0,
                       "sb":0, "cs":0 }

    def ProcessFielding(self, eventData):
        self.po += eventData.GetOuts()
        self.a += eventData.num_assists
        self.e += eventData.num_errors

        if eventData.dp_flag:    self.dp += 1
        elif eventData.tp_flag:  self.tp += 1

        if eventData.fielded_by > 0 and eventData.GetOuts() > 0:
            self.bf += 1
        if (eventData.fielded_by > 0 or
            eventData.event_type in [cw.EVENT_SINGLE,
                                     cw.EVENT_DOUBLE,
                                     cw.EVENT_TRIPLE]):
            self.bip += 1

        if eventData.pb_flag:  self.pb += 1

        for base in [1, 2, 3]:
            if eventData.GetSBFlag(base):  self.sb += 1
            if eventData.GetCSFlag(base):  self.cs += 1



    def __add__(self, x):
        y = TeamFielding()
        for key in self.stats:
            y.stats[key] = self.stats[key] + x.stats[key]
        return y

    def __iadd__(self, x):
        for key in self.stats: self.stats[key] += x.stats[key]
        return self

    def __getitem__(self, attr):  return self.stats[attr]
    def __setitem__(self, attr, value):  self.stats[attr] = value

    def __getattr__(self, attr):
        if attr == "stats":
            raise AttributeError
        elif attr == "pct":
            try:
                return 1.0*(self.po+self.a)/(self.po+self.a+self.e)
            except ZeroDivisionError:
                return None
        elif attr == "der":
            try:
                return 1.0*self.bf/self.bip
            except ZeroDivisionError:
                return None
        elif attr == "sbpct":
            try:
                return 1.0*self.sb/(self.sb+self.cs)
            except ZeroDivisionError:
                return None
        else:
            return self.stats[attr.lower()]

    def __setattr__(self, attr, value):
        if attr == "stats":
            object.__setattr__(self, attr, value)
        else:
            self.stats[attr.lower()] = value


