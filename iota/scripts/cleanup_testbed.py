#! /usr/bin/python3
import os
import sys
import argparse

from threading import Thread

topdir = os.path.dirname(sys.argv[0]) + '/../../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)

cmdargs = argparse.ArgumentParser(description='IOTA Harness')
cmdargs.add_argument('--rerun', dest='rerun', action='store_true',
                     help='Re-run mode, Skips copy and few init steps.')
cmdargs.add_argument('--testbed', dest='testbed_json', default="/warmd.json",
                     help='Testbed JSON file')
GlobalOptions = cmdargs.parse_args()

SSHCMD = "sshpass -p vm ssh -o StrictHostKeyChecking=no vm@"

rerun_pre_cleanup_commands = [
    "rm -rf /home/vm/images",
    "mkdir -p /home/vm/images",
    "mv /tmp/iota/*.tgz /home/vm/images/",
]

cleanup_commands = [
    "/tmp/iota/INSTALL.sh --clean-only",
    "systemctl stop pen-cmd",
    "docker rm -fv \$(docker ps -aq)",
    "docker system prune -f",
    "rm /etc/hosts",
    "pkill iota*",
    "rm -rf /tmp/iota*",
    "docker ps",
    "docker rmi -f \$(docker images -aq)",
    "rm -rf /var/run/naples",
    "iptables -F",
    "systemctl restart docker",
]

rerun_post_cleanup_commands = [
    "mkdir -p /tmp/iota",
    "mv /home/vm/images/* /tmp/iota/",
    "chown -R vm /tmp/iota",
    "chgrp -R vm /tmp/iota",
]

def run_commands(cmdlist, node_ip):
    for cmd in cmdlist:
        fullcmd = "%s%s \"sudo %s\"" % (SSHCMD, node_ip, cmd)
        print("  %s" % fullcmd)
        os.system(fullcmd)
    return

def cleanup_node(node_ip):
    print("Cleaning up %s" % node_ip)
    if GlobalOptions.rerun:
        run_commands(rerun_pre_cleanup_commands, node_ip)
    run_commands(cleanup_commands, node_ip)
    if GlobalOptions.rerun:
        run_commands(rerun_post_cleanup_commands, node_ip)
    return

import iota.harness.infra.utils.parser as parser
from multiprocessing.dummy import Pool as ThreadPool 
tbspec = parser.JsonParse(GlobalOptions.testbed_json)
pool = ThreadPool(len(tbspec.Instances.__dict__.values())) 
results = pool.map(cleanup_node, tbspec.Instances.__dict__.values())
