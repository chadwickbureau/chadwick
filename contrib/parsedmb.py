#
# This is a collection of useful routines for converting Diamond Mind
# boxscore/scoresheet files into a more Retrosheet-like format.
#

import sys

def isbatter(event):
    """
    Takes a primary (DMB) event code and determines if it's a batter event
    """
    return event[0].isdigit() or event in [ "K", "K+WP", "W", "IW", "S", "D", "T", "HR",
                                            "HBP", "CI" ]
                                           
def ishit(event):
    return event in [ "S", "D", "T", "HR" ]

def isreliever(event):
    """
    Returns True if the primary (DMB) event code appears to be a pitcher's name
    """
    if isbatter(event) or event in [ "WP", "BK", "SB2", "SB3", "SBH",
                                     "CS2", "CS3", "CSH" ]:
        return False
    else:
        return True

def ExtractAccountForm(lines):
    """
    Extract events from the "account form" portion of the DMB boxscore.
    Assumes that the next call of f.readline() will return the first line
    of the account form (the one for the leadoff hitter, directly following
    the header line).
    """

    rows = [ ]

    notes = { }

    for line in lines:
        if ":" in line:
            # This is a "notes" line
            parts = line.split()
            for part in parts:
                if ":" in part:
                    noteindex = int(part[:-1])
                else:
                    notes[noteindex] = part
            
        else:
            rows.append({ "batter": line[:18].strip(),
                          "position": line[18:20].strip(),
                          "columns": [ line[21:34].strip(),
                                       line[34:47].strip(),
                                       line[47:60].strip(),
                                       line[60:73].strip(),
                                       line[73:].strip() ] })


    data = [ ]
            
    for col in xrange(5):
        for row in rows:
            if row["batter"] != "":
                batter = row["batter"]
            play = row["columns"][col]
            if "note" in play:
                play = notes[int(play[5:])]

            if play != "":
                data.append([ batter, play.upper() ])

    return data

def Inningize(data):
    """
    Take a list of (batter, play) tuples and return a list of (batter, inning, play)
    tuples, with the ">" inning markers removed.
    """
    newdata = [ ]
    for (batter, play) in data:
        if ">" in play:
            inning = int(play.split(">")[0])
            play = play.split(">")[1]
        newdata.append([ batter, inning, play ])
    return newdata


def PrintPrimaries(events):
    for (batter, inning, play) in events:
        if "." in play:
            print play.split(".")[0]
        else:
            print play


def PrintEvents(events):
    for (batter, inning, play) in events:
        print play


def ExtractPrimaries(f):
    sheets = [ ]

    line = f.readline()

    while True:
        if line == "":  break

        if "------" in line:
            lines = [ ]

            line = f.readline()
            while "------" not in line and "IN OUT ER" not in line:
                lines.append(line)
                line = f.readline()
            
            sheets.append(ExtractAccountForm(lines))
            continue
        else:
            line = f.readline()


    visitors = [ ]
    for sheet in sheets[0::2]:
        visitors += sheet

    vislineup = set([ x[0] for x in visitors ])

    home = [ ]
    for sheet in sheets[1::2]:
        home += sheet

    homelineup = set([ x[0] for x in home ])

    PrintEvents(Inningize(visitors))
    PrintEvents(Inningize(home))


if __name__ == "__main__":
    # This demo simply extracts all the events from all the .box files
    # in the directory.
    #
    # Under unix, pipe this through 'sort' then 'uniq' to generate
    # all the unique event codes in a season.
    import glob
    
    for fn in glob.glob("*.box"):
        ExtractPrimaries(file(fn))

