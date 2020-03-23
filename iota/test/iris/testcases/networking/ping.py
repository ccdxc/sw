#! /usr/bin/python3
import pdb
import re
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
from iota.harness.infra.exceptions import *
from iota.test.iris.utils import vmotion_utils

def parseEthtool(text, intName):
    found = re.search("Link detected: ([\S]+)", text, re.M)
    if not found:
        api.Logger.debug("failed to find link state in output of ethtool: {0}".format(text))
        return None
    else:
        api.Logger.debug("link state for {0} was {1}".format(intName, found.group(1)))
        res = True if found.group(1)=="yes" else False
        return (intName,res)

def parseArp(text, ip):
    found = re.search(".*\(("+ip+")\) at ([\S]+) ([\S]+) on ([\S]+)\n", text, re.M)
    if not found:
        api.Logger.debug("no arp entry for ip {0}".format(ip))
    else:
        api.Logger.debug("found arp entry {0}".format(found.groups()))

def runCmd(nodeName, cmd):
    api.Logger.debug("sending command {0} to node {1}".format(cmd, nodeName))
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, nodeName, cmd)
    return api.Trigger(req)

def checkArp(nodeName, ip):
    cmd = "arp -na | grep {0}".format(ip)
    output = runCmd(nodename, cmd)
    return parseArp(output, ip)

def checkLink(nodeName, intName):
    cmd = "ethtool " + intName
    output = runCmd(nodename, cmd)
    return parseEthtool(output, intName)
        
def runIfconfig(nodeName, intName):
    cmd = "ifconfig " + intName
    output = runCmd(nodename, cmd)
    api.Logger.debug(output)

def debugPing(cmd, intName):
    runIfconfig(cmd.node_name, intName)
    foundIp = re.search(".* ([\S]+$)",cmd.command)
    if foundIp:
        ip = foundIp.group(1)
        if re.search("[\d]+\.[\d]+\.[\d]+\.[\d]+", ip):
            checkArp(cmd.node_name, ip)
        else:
            api.Logger.debug("skipping arp check for v6 address {0}".format(ip))
    else:
        api.Logger.debug("failed to determine ip address from ping command {0}".format(cmd.command))
    return checkLink(cmd.node_name, intName)

 
def Setup(tc):
    tc.skip = False

    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetNicMode() == 'hostpin' and tc.iterators.ipaf == 'ipv6':
        api.Logger.info("Skipping Testcase: IPv6 not supported in hostpin mode.")
        tc.skip = True

    if getattr(tc.args, 'vmotion_enable', False):
        wloads = []
        for idx, pair in enumerate(tc.workload_pairs):
            if idx % 2:
                continue
            w2 = pair[1]
            wloads.append(w2)
        vmotion_utils.PrepareWorkloadVMotion(tc, wloads)

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    req = None
    interval = "0.2"
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        interval = "3"
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if tc.iterators.ipaf == 'ipv6':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping6 -i %s -c 20 -s %d %s" % (interval, tc.iterators.pktsize, w2.ipv6_address))
        else:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping -i %s -c 20 -s %d %s" % (interval, tc.iterators.pktsize, w2.ip_address))
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadRestore(tc)

    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            if api.GetNicMode() == 'hostpin' and tc.iterators.pktsize > 1024:
                result = api.types.status.SUCCESS
            else:
                try:
                    found = re.search(cmd.node_name+"_(.*)_subif_(.*$)",cmd.entity_name)
                    if found:
                        debugPing(cmd, found.group(1))
                    else:
                        api.Logger.debug("failed to determine interface name from {0}".format(cmd.entity_name))
                except:
                    api.Logger.debug("failed to debug {0} ping issue".format(cmd.entity_name))
                result = api.types.status.FAILURE
            #for future use
            #elif tc.args.type == 'local_only':
            #    return api.types.status.CRITICAL
            #else:
            #    raise OfflineTestbedException
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
