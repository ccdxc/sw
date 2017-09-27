#! /usr/bin/python3
import timeit

class TimeProfiler:
    def __init__(self, name):
        self.name = name
        self.start = None
        self.end = None
        self.total = None

    def Start(self):
        self.start = timeit.default_timer()
        return

    def Stop(self):
        self.stop = timeit.default_timer()
        self.total = self.stop - self.start
        return

    def Show(self):
        assert(self.total is not None)
        mins, secs = divmod(self.total, 60)
        hours, mins = divmod(mins, 60)
        print("- %-8s : %02d:%02d:%02d" %\
              (self.name, hours, mins, secs))
        return

InitTimeProfiler    = TimeProfiler('Init')
ConfigTimeProfiler  = TimeProfiler('Config')
TestTimeProfiler    = TimeProfiler('Tests')
TotalTimeProfiler   = TimeProfiler('Total')

def Show():
    InitTimeProfiler.Show()
    ConfigTimeProfiler.Show()
    TestTimeProfiler.Show()
    TotalTimeProfiler.Show()
