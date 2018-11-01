#! /usr/bin/python3
import time
import iota.harness.api as api

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))
   
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "iperf -s -t 300", background = True)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "iperf -c %s" % w1.ip_address)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    
    api.Logger.info("Iperf Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
