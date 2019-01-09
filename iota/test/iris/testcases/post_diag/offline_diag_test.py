#! /usr/bin/python3
import time
import iota.harness.api as api
def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()
    mgmt_ip = api.GetNaplesMgmtIpAddresses()
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Node1: %s(%s) <--> Node2: %s(%s)" %\
            (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Offline Diags for %s" % (tc.cmd_descr))

    cmd1 = "LD_LIBRARY_PATH=/platform/lib:/nic/lib /nic/bin/diag_test offline &> /var/log/offline_diags_report.txt"
    cmd2 = "! grep FAIL /var/log/offline_diags_report.txt"

    if w1.IsNaples() or w2.IsNaples():
        if w1.IsNaples():
            api.Trigger_AddNaplesCommand(req, w1.node_name, cmd1)
            api.Trigger_AddNaplesCommand(req, w1.node_name, cmd2)

        if w2.IsNaples():
            api.Trigger_AddNaplesCommand(req, w2.node_name, cmd1)
            api.Trigger_AddNaplesCommand(req, w2.node_name, cmd2)

        tc.resp = api.Trigger(req)
    else:
        tc.resp = None

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

