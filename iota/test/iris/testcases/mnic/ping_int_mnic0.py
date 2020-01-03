#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.utils.naples_host as utils
import iota.harness.infra.store as store
import re

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS

    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Node1: %s(%s) <--> Node2: %s(%s)" %\
            (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting ping test for int_mnic0")

    if w1.IsNaples():
        ip = store.GetPrimaryIntNicMgmtIp()
        ip=re.sub('\.1$','.2',ip)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "ping -I int_mnic0 -c3 " + ip)

    if w2.IsNaples():
        ip = store.GetPrimaryIntNicMgmtIp()
        ip=re.sub('\.1$','.2',ip)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "ping -I int_mnic0 -c3 " + ip)

    trig_resp = api.Trigger(req)

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS

    api.Logger.info("DPS: Ping Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

