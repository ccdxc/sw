#! /usr/bin/python3
import os
import sys
import argparse

from threading import Thread
from multiprocessing.dummy import Pool as ThreadPool 

topdir = os.path.dirname(sys.argv[0]) + '/../../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)
import iota.harness.infra.utils.parser as parser

cmdargs = argparse.ArgumentParser(description='Cleanup Script')
cmdargs.add_argument('--testbed', dest='testbed_json', default="/warmd.json",
                     help='Testbed JSON file')
GlobalOptions = cmdargs.parse_args()

SSHCMD = "sshpass -p docker ssh -o StrictHostKeyChecking=no"
SCPCMD = "sshpass -p docker scp -o StrictHostKeyChecking=no"
def cleanup_node(node_ip):
    print("Cleaning up %s" % node_ip)
    os.system("%s %s/iota/scripts/cleanup_node.sh root@%s:/tmp/" % (SCPCMD, topdir, node_ip))
    os.system("%s root@%s source /tmp/cleanup_node.sh 2>&1" % (SSHCMD, node_ip))
    return

tbspec = parser.JsonParse(GlobalOptions.testbed_json)
pool = ThreadPool(len(tbspec.Instances))
mgmt_ips = []
for instance in tbspec.Instances:
    mgmt_ips.append(instance.NodeMgmtIP)
results = pool.map(cleanup_node, mgmt_ips)
