#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def SendGratArp(wl_list):
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for wl in wl_list:
        cmd = f"arping -c 3 -U {wl.ip_address} -I {wl.interface}"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        api.Logger.debug(f"GratArp from {wl.node_name} {wl.workload_name}"
                         f" {wl.interface} {wl.ip_address}")
        # send  with secondary IPs too
        for sec_ip_addr in wl.sec_ip_addresses:
            cmd = f"arping -c 3 -U {sec_ip_addr} -I {wl.interface}"
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
            api.Logger.debug(f"GratArp from {wl.node_name} {wl.workload_name} {wl.interface} {sec_ip_addr}")

    resp = api.Trigger(req)
    if resp is None:
        return False

    return True

def SendArpReply(wl_list):
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for wl in wl_list:
        cmd = f"arping -c 3 -A -s {wl.ip_address} -I {wl.interface} 0.0.0.0"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        api.Logger.debug(f"ArpReply from {wl.node_name} {wl.workload_name}"
                         f" {wl.interface} {wl.ip_address}")
        # send arping with secondary IPs too
        for sec_ip_addr in wl.sec_ip_addresses:
            cmd = f"arping -c 3 -A -s {sec_ip_addr} -I {wl.interface} 0.0.0.0"
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
            api.Logger.debug(f"ArpReply from {wl.node_name} {wl.workload_name}"
                             f" {wl.interface} {sec_ip_addr}")

    resp = api.Trigger(req)
    if resp is None:
        return False

    return True

def ArPing(tc):
    if type == 'local_only':
        api.Logger.info("local_only test")
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        api.Logger.info("remote_only test")
        tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

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
        cmd_cookie = "%s %s %s %s" % (w1.node_name, w1.workload_name, w1.interface, w1.ip_address)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "arping -c  5 -U %s -I %s" % (w1.ip_address, w1.interface))
        api.Logger.info("ArPing from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "%s %s %s %s" % (w2.node_name, w2.workload_name, w2.interface, w2.ip_address)
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                               "arping -c  5 -U %s -I %s" % (w2.ip_address, w2.interface))
        api.Logger.info("ArPing from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("ArPing Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result
