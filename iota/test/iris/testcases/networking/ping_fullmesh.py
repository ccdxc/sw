#! /usr/bin/python3

import iota.harness.api as api
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2

def Setup(tc):
    # Prepare a ConfigQueryMsg()
    req = cfg_svc_pb2.ConfigQueryMsg()
    req.view = cfg_svc_pb2.CFG_DB_VIEW_CFG_SVC
    req.kind = "ENDPOINT"
    resp = api.QueryConfig(req)
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        Logger.error("Failed to query config: ",
                     types_pb2.APIResponseType.Name(resp.api_response.api_status))
        return api.types.status.FAILURE

    # Store the config objects in testcase.
    tc.endpoints = resp.configs
    return api.types.status.SUCCESS

def Trigger(tc):
    req = topo_svc_pb2.TriggerMsg()
    req.trigger_op = topo_svc_pb2.EXEC_CMDS

    for ep1 in tc.endpoints:
        for ep2 in tc.endpoints:
            if ep1 == ep2: continue
            api.Logger.info("Starting Ping test from %s --> %s" % (ep1.workload_name, ep2.workload_name))
            cmd = req.commands.add()
            cmd.mode = topo_svc_pb2.COMMAND_FOREGROUND
            cmd.workload_name = ep1.workload_name
            cmd.command = "ping -c 10 %s" % ep2.ip_address
    
    tc.resp = api.Trigger(req)
    if tc.resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to run Trigger: ",
                         types_pb2.APIResponseType.Name(tc.resp.api_response.api_status))
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Command failed on workload: %s" % cmd.workload_name)
            api.Logger.error("- Command = %s" % cmd.command)
            api.Logger.error("- stdout = %s" % cmd.stdout)
            api.Logger.error("- stderr = %s" % cmd.stderr)
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
