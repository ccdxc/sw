#! /usr/bin/python3
import timeit

class TimeProfiler:
    def __init__(self):
        self.start = None
        self.end = None
        self.total = 0
        return

    def Start(self):
        self.start = timeit.default_timer()
        return

    def Stop(self):
        self.stop = timeit.default_timer()
        self.total += (self.stop - self.start)
        return

    def TotalTime(self):
        assert(self.total is not None)
        mins, secs = divmod(self.total, 60)
        hours, mins = divmod(mins, 60)
        total_time = "%02d:%02d:%02d" %\
                     (hours, mins, secs)
        return total_time
