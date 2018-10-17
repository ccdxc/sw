#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2

def Setup(tc):
    tc.workloads = api.GetWorkloads()
    return api.types.status.SUCCESS

def __get_workload_ip_from_prefix(prefix):
    return prefix.split('/')[0]

def __is_workload_pair_valid(tc, w1, w2):
    if id(w1) == id(w2):
        return False
    if tc.args.type == 'local_only' and w1.node_name != w2.node_name:
        return False
    if tc.args.type == 'remote_only' and w1.node_name == w2.node_name:
        return False
    return True

def Trigger(tc):
    req = topo_svc_pb2.TriggerMsg()
    req.trigger_op = topo_svc_pb2.EXEC_CMDS
    req.trigger_mode = topo_svc_pb2.TRIGGER_SERIAL
    tc.cmd_cookies = []

    for w1 in tc.workloads:
        for w2 in tc.workloads:
            if not __is_workload_pair_valid(tc, w1, w2): continue
            w1_ip_address = __get_workload_ip_from_prefix(w1.ip_address)
            w2_ip_address = __get_workload_ip_from_prefix(w2.ip_address)
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1_ip_address, w2.workload_name, w2_ip_address)
            api.Logger.info("Starting Ping test from %s" % (cmd_cookie))
            tc.cmd_cookies.append(cmd_cookie)

            cmd = req.commands.add()
            cmd.mode = topo_svc_pb2.COMMAND_FOREGROUND
            cmd.workload_name = w1.workload_name
            cmd.command = "ping -c 10 -w 100 -W 10 -i 2 %s" % w2_ip_address
            cmd.node_name = w1.node_name
    
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    assert(len(tc.cmd_cookies) == len(tc.resp.commands))

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
