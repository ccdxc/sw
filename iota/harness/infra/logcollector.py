#! /usr/bin/python3
import copy
import json
import logging
import os
import pdb
import re
import subprocess
import sys
import traceback

import iota.harness.infra.store as store
import iota.harness.api as api
import iota.protos.pygen.iota_types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.common as common
import iota.harness.infra.types as types
from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from multiprocessing.dummy import Pool as ThreadPool


logdirs = [
    "/pensando/iota/*.log",
    "/home/vm/nohup.out", #agent log if crashed
    "/naples/memtun.log",
    "/var/log/vmkernel.log",
]

class CollectLogNode:
    def __init__(self,name,ip,username,password):
        self.name=name
        self.ip=ip
        self.username=username
        self.password=password
    def Name(self):
        return self.name

def __collect_onenode(node):
    SSHCMD = "sshpass -p {0} scp -r -o StrictHostKeyChecking=no {1}@".format(node.password,node.username)
    msg="Collecting Logs for Node: %s (%s)" % (node.Name(), node.ip)
    print(msg)
    Logger.debug(msg)
    tsName=''
    try: tsName=api.GetTestsuiteName()
    except: tsName='NA'
    localdir = "%s/logs/%s/nodes/%s/" % (GlobalOptions.logdir, tsName, node.Name())
    subprocess.call("mkdir -p %s" % localdir,shell=True,stdout=None,stderr=subprocess.STDOUT)
    for logdir in logdirs:
        permCmd = "sshpass -p vm ssh vm@" + node.ip + " sudo chown -R vm:vm " + logdir
        Logger.debug(permCmd)
        try:
            proc=subprocess.Popen(permCmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            output=proc.communicate()[0]
        except: 
            output=traceback.format_exc()
        Logger.debug(output)
        fullcmd = "%s%s:%s %s" % (SSHCMD, node.ip, logdir, localdir)
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
            if 'Vars' in data['Provision'] and getattr(data['Provision']['Vars'],"EsxUsername",None):
                username = data['Provision']['Vars']['EsxUsername']
                password = data['Provision']['Vars']['EsxPassword']
            else:
                username = data['Provision']['Username']
                password = data['Provision']['Password']
            for inst in data["Instances"]:
                if inst.get("Type","") == "bm" and "NodeMgmtIP" in inst and "Name" in inst: 
                    nodes.append(CollectLogNode(inst["Name"],inst["NodeMgmtIP"],username,password))
    except:
        msg="failed to build nodes from testbed file. error was:"
        msg+=traceback.format_exc()
        Logger.debug(msg)
    return nodes

def CollectLogs():
    if GlobalOptions.dryrun or GlobalOptions.skip_logs: return
    nodes=[]
    try: 
        nodes=copy.deepcopy(store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodes())
        for node in nodes:
            if getattr(node,"_Node__esx_username",None):
                node.username = node._Node__esx_username
                node.password = node._Node__esx_password
            else:
                node.username = getattr(node,"_Node__vmUser","vm")
                node.password = getattr(node,"_Node__vmPassword","vm")
            node.ip = node._Node__ip_address
    except:
        msg = traceback.format_exc()
        Logger.debug(msg)
        msg = 'topo not setup yet, gathering node info from testbed json file'
        Logger.debug(msg)
        nodes=buildNodesFromTestbedFile(GlobalOptions.testbed_json)
    pool = ThreadPool(len(nodes))
    results = pool.map(__collect_onenode, nodes)

def CollectTechSupport(tsName):
    try:
        #global __CURREN_TECHSUPPORT_CNT
        #__CURREN_TECHSUPPORT_CNT = __CURREN_TECHSUPPORT_CNT + 1
        Logger.info("Collecting techsupport for testsuite {0}".format(tsName))
        tsName=re.sub('\W','_',tsName)
        logDir=GlobalOptions.logdir
        if not logDir.endswith('/'):
            logDir += '/'
        logDir += 'techsupport/'
        if not os.path.exists(logDir):
            os.mkdir(logDir)
        nodes = api.GetNaplesHostnames()
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in nodes:
            Logger.info("Techsupport for node: %s" % n)
            common.AddPenctlCommand(req, n, "system tech-support -b %s-tech-support" % (n))
        resp = api.Trigger(req)
        result = types.status.SUCCESS
        for n,cmd in zip(nodes,resp.commands):
            #api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                Logger.error("Failed to execute penctl system tech-support on node: %s. err: %d" % (n, cmd.exit_code))
                result = types.status.FAILURE
                continue
            # Copy tech support tar out
            # TAR files are created at: pensando/iota/entities/node1_host/<test_case>
            ntsn = "%s-tech-support.tar.gz" % (n)
            resp = api.CopyFromHost(n, [ntsn], logDir)
            if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
                Logger.error("Failed to copy techsupport file %s from node: %s" % (ntsn, n) )
                result = types.status.FAILURE
                continue
            os.rename(logDir+ntsn,logDir+tsName+'_'+ntsn)
        #if __CURREN_TECHSUPPORT_CNT > __MAX_TECHSUPPORT_PER_RUN:
        #    return types.status.CRITICAL
        return result
    except AttributeError:
        Logger.debug('failed to collect tech support. node list not setup yet')
    except:
        Logger.debug('failed to collect tech support. error was: {0}'.format(traceback.format_exc()))
        return types.status.CRITICAL

