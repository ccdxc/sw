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

    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))
    
    cmd = req.commands.add()
    cmd.mode = topo_svc_pb2.COMMAND_BACKGROUND
    cmd.workload_name = w1.workload_name
    cmd.node_name = w1.node_name
    cmd.command = "iperf -s -t 30"
    
    cmd = req.commands.add()
    cmd.mode = topo_svc_pb2.COMMAND_FOREGROUND
    cmd.workload_name = w2.workload_name
    cmd.node_name = w2.node_name
    cmd.command = "iperf -c %s" % w1.ip_address
   
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    
    api.Logger.info("Iperf Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
