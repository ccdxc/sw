#! /usr/bin/python3
import os
import sys
import argparse

from threading import Thread

topdir = os.path.dirname(sys.argv[0]) + '/../../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)

cmdargs = argparse.ArgumentParser(description='IOTA Log Collector')
cmdargs.add_argument('--testbed', dest='testbed_json', default="/warmd.json",
                     help='Testbed JSON file')
GlobalOptions = cmdargs.parse_args()

SSHCMD = "sshpass -p docker scp -r -o StrictHostKeyChecking=no root@"

logdirs = [
    "/tmp/iota/*",
    "/naples/memtun.log",
]

def run_commands(cmdlist, node_name):
    global topdir
    localdir = "%s/iota/logs/nodes/%s/" % (topdir, node_name)
    os.system("mkdir -p %s" % localdir)
    for logdir in logdirs:
        fullcmd = "%s%s:%s %s" % (SSHCMD, node_name, logdir, localdir)
        print("  %s" % fullcmd)
        os.system(fullcmd)
    return

def collect_logs(node_name):
    print("Collecting Logs for Node: %s" % node_name)
    run_commands(logdirs, node_name)
    return

import iota.harness.infra.utils.parser as parser
from multiprocessing.dummy import Pool as ThreadPool 
#os.system("yum install -y sshpass")
tbspec = parser.JsonParse(GlobalOptions.testbed_json)
pool = ThreadPool(len(tbspec.Instances))
nodes = []
for instance in tbspec.Instances:
    nodes.append(instance.Name)
results = pool.map(collect_logs, nodes)
