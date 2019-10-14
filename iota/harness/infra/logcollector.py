#! /usr/bin/python3
import json
import logging
import os
import re
import subprocess
import sys
import traceback

import iota.harness.infra.store as store
import iota.harness.api as api

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from multiprocessing.dummy import Pool as ThreadPool

logdirs = [
    "/pensando/iota/*.log",
    "/home/vm/nohup.out", #agent log if crashed
    "/naples/memtun.log",
]

class CollectLogNode:
    def __init__(self,name,ip):
        self.name=name
        self.mgmtIpAddress=ip
    def Name(self):
        return self.name
    def MgmtIpAddress(self):
        return self.mgmtIpAddress

def __collect_onenode(node):
    SSHCMD = "sshpass -p vm scp -r -o StrictHostKeyChecking=no vm@"
    msg="Collecting Logs for Node: %s (%s)" % (node.Name(), node.MgmtIpAddress())
    print(msg)
    Logger.debug(msg)
    tsName=''
    try: tsName=api.GetTestsuiteName()
    except: tsName='NA'
    localdir = "%s/logs/%s/nodes/%s/" % (GlobalOptions.logdir, tsName, node.Name())
    subprocess.call("mkdir -p %s" % localdir,shell=True,stdout=None,stderr=subprocess.STDOUT)
    for logdir in logdirs:
        permCmd = "sshpass -p vm ssh vm@" + node.MgmtIpAddress() + " sudo chown -R vm:vm " + logdir
        Logger.debug(permCmd)
        try:
            proc=subprocess.Popen(permCmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            output=proc.communicate()[0]
        except: 
            output=traceback.format_exc()
        Logger.debug(output)
        fullcmd = "%s%s:%s %s" % (SSHCMD, node.MgmtIpAddress(), logdir, localdir)
        Logger.debug(fullcmd)
        try: 
            proc=subprocess.Popen(fullcmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            output=proc.communicate()[0]
        except: 
            output=traceback.format_exc()
        Logger.debug(output)

def buildNodesFromTestbedFile(testbed):
    Logger.debug("building nodes from testbed file {0}".format(testbed))
    nodes=[]
    try:
        with open(testbed,"r") as topoFile:
            data=json.load(topoFile)
            for inst in data["Instances"]:
                if inst.get("Type","") == "bm" and "NodeMgmtIP" in inst and "Name" in inst:
                    nodes.append(CollectLogNode(inst["Name"],inst["NodeMgmtIP"]))
    except:
        msg="failed to build nodes from testbed file. error was:"
        msg+=traceback.format_exc()
        print(msg)
        Logger.debug(msg)
    return nodes

def CollectLogs():
    if GlobalOptions.dryrun or GlobalOptions.skip_logs: return
    nodes=[]
    try: nodes=store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodes()
    except:
        msg="failed to get nodes in call to CollectLogs. error was:"
        msg+=traceback.format_exc()
        Logger.debug(msg)
        msg='gathering node info from testbed json file'
        Logger.debug(msg)
        nodes=buildNodesFromTestbedFile(GlobalOptions.testbed_json)
    pool = ThreadPool(len(nodes))
    results = pool.map(__collect_onenode, nodes)


