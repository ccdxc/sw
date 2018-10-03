#! /usr/bin/python3
import sys
import os
import pdb

topdir = os.path.dirname(sys.argv[0]) + '/../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)
paths = [
    'iota/protos/pygen'
]
for p in paths:
    fullpath = os.path.join(topdir, p)
    sys.path.append(fullpath)


# This import will parse all the command line options.
import iota.harness.infra.glopts as glopts
glopts.GlobalOptions.topdir = topdir

import iota.harness.infra.types as types
import iota.harness.infra.utils.timeprofiler as timeprofiler
import iota.harness.infra.engine as engine

from iota.harness.infra.utils.logger import Logger as Logger

overall_timer = timeprofiler.TimeProfiler()
overall_timer.Start()

def InitLogger():
    if glopts.GlobalOptions.debug:
        Logger.SetLoggingLevel(types.loglevel.DEBUG)
    elif glopts.GlobalOptions.verbose:
        Logger.SetLoggingLevel(types.loglevel.VERBOSE)
    else:
        Logger.SetLoggingLevel(types.loglevel.INFO)
    return

def Main():
    InitLogger()

    engine.Main()
    return 0

if __name__ == '__main__':
    status = Main()
    overall_timer.Stop()
    print("Overall Runtime: " + overall_timer.TotalTime())
    sys.exit(status)
