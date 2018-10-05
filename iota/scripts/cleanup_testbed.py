#! /usr/bin/python3
import os
import sys

topdir = os.path.dirname(sys.argv[0]) + '/../../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)

SSHCMD = "sshpass -p vm ssh -o StrictHostKeyChecking=no vm@"
commands = [
    "pkill iota*",
    "rm -rf /tmp/iota",
]

import iota.harness.infra.utils.parser as parser
tbspec = parser.JsonParse("/warmd.json")
for k,v in tbspec.Instances.__dict__.items():
    print("Cleaning up %s" % v)
    for cmd in commands:
        fullcmd = "%s%s \"%s\"" % (SSHCMD, v, cmd)
        print("  %s" % fullcmd)
        os.system(fullcmd)
    
