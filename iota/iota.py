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
if glopts.GlobalOptions.logdir is None:
    glopts.GlobalOptions.logdir = "%s/iota" % topdir

import iota.harness.infra.types as types
import iota.harness.infra.utils.timeprofiler as timeprofiler
import iota.harness.infra.engine as engine
import iota.harness.infra.procs as procs

from iota.harness.infra.utils.logger import Logger as Logger

overall_timer = timeprofiler.TimeProfiler()
overall_timer.Start()

gl_srv_process = None

def MoveOldLogs():
    for i in range(256):
        oldlogdir = '%s/iota/oldlogs%d' % (glopts.GlobalOptions.topdir, i)
        if not os.path.exists(oldlogdir): break
    os.system("mkdir %s" % oldlogdir)
    os.system("mv %s/iota/*.log %s/" % (glopts.GlobalOptions.topdir, oldlogdir))
    os.system("mv %s/iota/logs %s/" % (glopts.GlobalOptions.topdir, oldlogdir))
    os.system("mv %s/iota/iota_sanity_logs.tar.gz %s/" % (glopts.GlobalOptions.topdir, oldlogdir))

def InitLogger():
    if glopts.GlobalOptions.debug:
        Logger.SetLoggingLevel(types.loglevel.DEBUG)
    elif glopts.GlobalOptions.verbose:
        Logger.SetLoggingLevel(types.loglevel.VERBOSE)
    else:
        Logger.SetLoggingLevel(types.loglevel.INFO)
    return

def __start_server():
    global gl_srv_process
    srv_binary = "%s/iota/bin/server/iota_server" % topdir
    srv_logfile = "%s/server.log" % glopts.GlobalOptions.logdir
    srv_args = "--port %d" % int(glopts.GlobalOptions.svcport)
    if glopts.GlobalOptions.dryrun:
        srv_args += " --stubmode"
    gl_srv_process = procs.IotaProcess("%s %s" % (srv_binary, srv_args), srv_logfile)
    gl_srv_process.Start()
    return

def __exit_cleanup():
    global gl_srv_process
    Logger.debug("ATEXIT: Stopping IOTA Server")
    gl_srv_process.Stop()
    if glopts.GlobalOptions.dryrun or glopts.GlobalOptions.skip_logs:
        return
    Logger.info("Saving logs to iota_sanity_logs.tar.gz")
    os.system("%s/iota/scripts/savelogs.sh %s" % (topdir, topdir))
    return

def Main():
    atexit.register(__exit_cleanup)
    InitLogger()
    __start_server()
    ret = engine.Main()
    return ret

if __name__ == '__main__':
    #MoveOldLogs()
    status = Main()
    overall_timer.Stop()
    print("Overall Runtime  : " + overall_timer.TotalTime())
    print("Overall Status   : %s" % types.status.str(status).title())
    print("\n\n")
    sys.exit(status)
