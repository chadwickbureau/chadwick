import csv
import itertools
import subprocess

class ToolProcess(object):
    def __init__(self, tool_path, tool_name, year):
        self.tool_path = tool_path
        self.tool_name = tool_name
        self.year = year

    def __iter__(self):
        process = subprocess.Popen(self.command_line, shell=True,
                                   stdout=subprocess.PIPE)
        for row in csv.DictReader(process.stdout, self.header):
            yield self.rowclass(row)
            


class DiffEngine(object):
    def calculate(self, tool1, tool2):
        diffs = { }
        for (x, y) in itertools.izip(tool1, tool2):
            fields = x.diff(y)
            for key in fields:
                if key not in diffs: diffs[key] = [ ]
                diffs[key].append(self.diff_object(key, x, y))
        return diffs


def run_diff(engine, tool1, tool2, data_dir):
    import os
    os.chdir(data_dir)
    diffs = engine.calculate(tool1, tool2)

    for key in sorted(diffs.keys()):
        print "%-55s %-10s %-10s" % (key, tool1.tool_name, tool2.tool_name)
        print "-"*77

        for d in diffs[key]:
            print "%-55s %-10s %-10s" % \
                  (d.context, d.tool1, d.tool2)
        print
