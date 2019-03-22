#! /usr/bin/python3
import os
import sys

import iota.harness.infra.store as store
import iota.harness.api as api

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from multiprocessing.dummy import Pool as ThreadPool

logdirs = [
    "/pensando/iota/*",
    "/home/vm/nohup.out", #agent log if crashed
    "/naples/memtun.log",
]


def __collect_onenode(node):
    SSHCMD = "sshpass -p vm scp -r --exclude='*.ova'  --exclude='*.vmdk' -o StrictHostKeyChecking=no vm@"
    print("Collecting Logs for Node: %s (%s)" % (node.Name(), node.MgmtIpAddress()))
    localdir = "%s/logs/%s/nodes/%s/" % (GlobalOptions.logdir, api.GetTestsuiteName(), node.Name())
    os.system("mkdir -p %s" % localdir)
    for logdir in logdirs:
        permCmd = "sshpass -p vm ssh vm@" + node.MgmtIpAddress() + " -t sudo chown -R vm:vm " + logdir
        os.system(permCmd)
        fullcmd = "%s%s:%s %s" % (SSHCMD, node.MgmtIpAddress(), logdir, localdir)
        print("  %s" % fullcmd)
        os.system(fullcmd)

def CollectLogs():
    if GlobalOptions.dryrun or GlobalOptions.skip_logs: return
    nodes = store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodes()
    pool = ThreadPool(len(nodes))
    results = pool.map(__collect_onenode, nodes)
