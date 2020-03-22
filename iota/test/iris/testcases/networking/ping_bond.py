#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
from iota.harness.infra.exceptions import *
import iota.test.iris.testcases.networking.utils as utils

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []
    interval = 0.1
    req = api.Trigger_CreateAllParallelCommandsRequest()

    for node, wl in utils.GetBondWorkloadPair():
        node_name, bond_ip = node.Name(), node.GetBondIp()
        # Ping Bond ===> Workload
        cmd_cookie = "%s(%s) --> %s(%s)" %(node_name, bond_ip, wl.workload_name, wl.ip_address)
        tc.cmd_cookies.append(cmd_cookie)
        api.Logger.info("%s"%(cmd_cookie))
        cmd = "ping -i %s -c 20 -s %d %s" % (interval, tc.iterators.pktsize, wl.ip_address)
        api.Trigger_AddNaplesCommand(req, node_name, cmd)
        # Ping Workload ===> Bond
        cmd_cookie = "%s(%s) --> %s(%s)" %(wl.workload_name, wl.ip_address, node_name, bond_ip)
        tc.cmd_cookies.append(cmd_cookie)
        api.Logger.info("%s"%(cmd_cookie))
        cmd = "ping -i %s -c 20 -s %d %s" % (interval, tc.iterators.pktsize, wl.ip_address)
        api.Trigger_AddHostCommand(req, wl.node_name, wl.workload_name, cmd)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            if api.GetNicMode() == 'hostpin' and tc.iterators.pktsize > 1024:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
