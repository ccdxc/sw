#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def Setup(tc):
    tc.tunnels = api.GetTestsuiteAttr("TUNNELS_MPLSUDP")
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateAllParallelCommandsRequest()
    tc.cmd_cookies = []

    for tunnel in tc.tunnels:
        w1 = tunnel.w1
        w2 = tunnel.w2

        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "ping -i 0.2 -c 20 -s %d %s" % (64, w2.ip_address))
        
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
