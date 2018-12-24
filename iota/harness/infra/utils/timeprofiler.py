#! /usr/bin/python3
import datetime

class TimeProfiler:
    def __init__(self):
        self.start = None
        self.end = None
        self.stop = None
        self.delta = None
        self.total = 0
        return

    def Start(self):
        self.start = datetime.datetime.now()
        return

    def Stop(self):
        self.stop = datetime.datetime.now()
        self.delta = (self.stop - self.start)
        self.total = self.delta.seconds
        return

    def TotalTime(self):
        if self.stop is None or self.start is None:
            return "NA"

        assert(self.total is not None)
        mins, secs = divmod(self.total, 60)
        hours, mins = divmod(mins, 60)
        total_time = "%02d:%02d:%02d.%03d" %\
                     (hours, mins, secs, self.delta.microseconds / 1000)
        return total_time
