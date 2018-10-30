#! /usr/bin/python3
import sys
import os
import pdb
import atexit

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
import iota.harness.infra.procs as procs

from iota.harness.infra.utils.logger import Logger as Logger

overall_timer = timeprofiler.TimeProfiler()
overall_timer.Start()

gl_server_process = None

def InitLogger():
    if glopts.GlobalOptions.debug:
        Logger.SetLoggingLevel(types.loglevel.DEBUG)
    elif glopts.GlobalOptions.verbose:
        Logger.SetLoggingLevel(types.loglevel.VERBOSE)
    else:
        Logger.SetLoggingLevel(types.loglevel.INFO)
    return

def __cleanup_testbed():
    Logger.info("Cleaning up Testbed, Logfile = cleanup.log")
    cmd = "./scripts/cleanup_testbed.py --testbed %s" % glopts.GlobalOptions.testbed_json
    if glopts.GlobalOptions.rerun:
        cmd = cmd + " --rerun"
    if os.system("%s > cleanup.log 2>&1" % cmd) != 0:
        Logger.info("Cleanup testbed failed.")
        sys.exit(1)
    return

def __start_server():
    global gl_server_process
    gl_server_process = procs.IotaProcess("%s/iota/bin/server/iota_server" % topdir,
                                          "%s/iota/server.log" % topdir)
    gl_server_process.Start()
    return

def __exit_cleanup():
    global gl_server_process
    Logger.info("ATEXIT: Stopping IOTA Server")
    gl_server_process.Stop()
    Logger.info("ATEXIT: Saving logs to iota_sanity_logs.tar.gz")
    os.system("scripts/savelogs.sh")
    return

def Main():
    atexit.register(__exit_cleanup)
    InitLogger()
    __start_server()
    __cleanup_testbed()
    engine.Main()
    return 0

if __name__ == '__main__':
    status = Main()
    overall_timer.Stop()
    print("\n\nOverall Runtime: " + overall_timer.TotalTime())
    sys.exit(status)
