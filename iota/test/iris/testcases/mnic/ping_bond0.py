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
    api.Logger.info("Starting Ping test from %s" % (tc.cmd_descr))

    if w1.IsNaples() and w2.IsNaples():
        api.Trigger_AddNaplesCommand(req, w1.node_name, "ifconfig bond0 20.20.20.20 netmask 255.255.255.0 up")
        api.SetBondIp(w1.node_name, "20.20.20.20")
        api.Trigger_AddNaplesCommand(req, w2.node_name, "ifconfig bond0 20.20.20.21 netmask 255.255.255.0 up")
        api.SetBondIp(w2.node_name, "20.20.20.21")
        api.Trigger_AddNaplesCommand(req, w1.node_name, "ping -I bond0 -c3 20.20.20.21")
        tc.resp = api.Trigger(req)
    else:
        tc.resp = None

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS

    api.Logger.info("Ping Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

