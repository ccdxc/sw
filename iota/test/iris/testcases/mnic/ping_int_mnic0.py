#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.utils.naples_host as utils

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

    if w1.IsNaples():
        api.Trigger_AddNaplesCommand(req, w1.node_name, "ifconfig int_mnic0 40.40.40.40 netmask 255.255.255.0 up")

    host_mgmt_intfs = utils.GetHostInternalMgmtInterfaces(w1.node_name)

    #ToDo: Need to enhance this to iterate over N mgmt interfaces on host
    w1_cmd = "ifconfig %s 40.40.40.41 netmask 255.255.255.0 up" % host_mgmt_intfs[0]
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           w1_cmd)

    if w2.IsNaples():
        api.Trigger_AddNaplesCommand(req, w2.node_name, "ifconfig int_mnic0 50.50.50.50 netmask 255.255.255.0 up")

    host_mgmt_intfs = utils.GetHostInternalMgmtInterfaces(w1.node_name)

    #ToDo: Need to enhance this to iterate over N mgmt interfaces on host
    w2_cmd = "ifconfig %s 50.50.50.51 netmask 255.255.255.0 up" % host_mgmt_intfs[0]

    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           w2_cmd)

    if w1.IsNaples() and w2.IsNaples():
        api.Trigger_AddNaplesCommand(req, w1.node_name, "ping -I int_mnic0 -c3 40.40.40.41")
        api.Trigger_AddNaplesCommand(req, w2.node_name, "ping -I int_mnic0 -c3 50.50.50.51")

    trig_resp = api.Trigger(req)

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
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

