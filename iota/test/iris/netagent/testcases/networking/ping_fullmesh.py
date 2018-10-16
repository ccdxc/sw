#! /usr/bin/python3
import time
import pdb

import iota.harness.api as api
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2

def Setup(tc):
    tc.workloads = api.GetWorkloads()
    return api.types.status.SUCCESS

def __get_workload_ip_from_prefix(prefix):
    return prefix.split('/')[0]

def Trigger(tc):
    req = topo_svc_pb2.TriggerMsg()
    req.trigger_op = topo_svc_pb2.EXEC_CMDS
    req.trigger_mode = topo_svc_pb2.TRIGGER_SERIAL

    for w1 in tc.workloads:
        for w2 in tc.workloads:
            if w1 == w2: continue
            api.Logger.info("Starting Ping test from %s --> %s" % (w1.workload_name, w2.workload_name))
            cmd = req.commands.add()
            cmd.mode = topo_svc_pb2.COMMAND_FOREGROUND
            cmd.workload_name = w1.workload_name
            cmd.command = "ping -c 10 %s" % __get_workload_ip_from_prefix(w2.ip_address)
            cmd.node_name = w1.node_name
    
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
