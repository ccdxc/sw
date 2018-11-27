#! /usr/bin/python3
import os
import sys

import iota.harness.infra.store as store
import iota.harness.api as api

from multiprocessing.dummy import Pool as ThreadPool 

logdirs = [
    "/tmp/iota/*",
    "/naples/memtun.log",
]

def __collect_onenode(node):
    SSHCMD = "sshpass -p docker scp -r -o StrictHostKeyChecking=no root@"
    print("Collecting Logs for Node: %s" % node)
    localdir = "%s/iota/logs/%s/nodes/%s/" % (api.GetTopDir(), api.GetTestsuiteName(), node.Name())
    os.system("mkdir -p %s" % localdir)
    for logdir in logdirs:
        fullcmd = "%s%s:%s %s" % (SSHCMD, node.MgmtIpAddress(), logdir, localdir)
        print("  %s" % fullcmd)
        os.system(fullcmd)
    return

def CollectLogs():
    nodes = store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodes()
    pool = ThreadPool(len(nodes))
    results = pool.map(__collect_onenode, nodes)
