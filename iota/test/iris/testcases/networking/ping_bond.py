#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
from iota.harness.infra.exceptions import *

def Setup(tc):
    res = [True for node_name in api.GetNaplesHostnames() if api.GetBondIp(node_name)]
    tc.skip = False if any(res) else True
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip:
        api.Logger.info("Testcase is skipped in absence of valid Bond IP")
        return api.types.status.SUCCESS

    tc.cmd_cookies = []
    interval = 0.1
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node_name in api.GetNaplesHostnames():
        bond_ip = api.GetBondIp(node_name)
        api.Logger.info("Bond IP : %s on %s"%(bond_ip, node_name))
        if bond_ip == None:
            api.Logger.info("Not valid IP on Bond, Skipping...")
            continue

        for node_name_2 in api.GetNaplesHostnames():
            # Workload on same host can not ping Bond
            if node_name == node_name_2:
                continue

            for wl in api.GetWorkloads(node_name_2):
                # Tagged vlan check
                if wl.uplink_vlan != 0:
                    continue

                # Ping Bond ===> Workload
                cmd_cookie = "%s(%s) --> %s(%s)" %\
                             (node_name, bond_ip, wl.workload_name, wl.ip_address)
                tc.cmd_cookies.append(cmd_cookie)
                api.Logger.info("%s"%(cmd_cookie))
                cmd = "ping -i %s -c 20 -s %d %s" % (interval, tc.iterators.pktsize, wl.ip_address)
                api.Trigger_AddNaplesCommand(req, node_name_2, cmd)

                # Ping Workload ===> Bond
                cmd_cookie = "%s(%s) --> %s(%s)" %\
                             (wl.workload_name, wl.ip_address, node_name, bond_ip)
                tc.cmd_cookies.append(cmd_cookie)
                api.Logger.info("%s"%(cmd_cookie))
                cmd = "ping -i %s -c 20 -s %d %s" % (interval, tc.iterators.pktsize, wl.ip_address)
                api.Trigger_AddHostCommand(req, wl.node_name, wl.workload_name, cmd)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip:
        api.Logger.info("Testcase is skipped in absence of valid Bond IP")
        return api.types.status.SUCCESS

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
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
