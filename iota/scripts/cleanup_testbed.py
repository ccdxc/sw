#! /usr/bin/python3
import os
import sys
import argparse

topdir = os.path.dirname(sys.argv[0]) + '/../../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)

cmdargs = argparse.ArgumentParser(description='IOTA Harness')
cmdargs.add_argument('--rerun', dest='rerun',
                    action='store_true',
                    help='Re-run mode, Skips copy and few init steps.')
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
    "sudo iptables -F",
    "sudo systemctl restart docker",
]

rerun_post_cleanup_commands = [
    "mkdir -p /tmp/iota",
    "mv /home/vm/images/* /tmp/iota/",
    "chown -R vm /tmp/iota",
    "chgrp -R vm /tmp/iota",
]

def run_commands(cmdlist):
    for cmd in cmdlist:
        fullcmd = "%s%s \"sudo %s\"" % (SSHCMD, v, cmd)
        print("  %s" % fullcmd)
        os.system(fullcmd)
    return

import iota.harness.infra.utils.parser as parser
tbspec = parser.JsonParse("/warmd.json")
for k,v in tbspec.Instances.__dict__.items():
    print("Cleaning up %s" % v)
    if GlobalOptions.rerun:
        run_commands(rerun_pre_cleanup_commands)
    run_commands(cleanup_commands)
    if GlobalOptions.rerun:
        run_commands(rerun_post_cleanup_commands)
