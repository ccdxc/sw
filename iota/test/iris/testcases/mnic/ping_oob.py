#! /usr/bin/python3
import time
import iota.harness.api as api
def Setup(tc):
    tc.config_only = getattr(tc.args, "verify", "yes")
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()
    #mgmt_ip = api.GetNaplesMgmtIpAddresses()
    mgmt_ip = ["192.168.68.1"]
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Node1: %s(%s) <--> Node2: %s(%s)" %\
            (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Ping test between %s" % (tc.cmd_descr))

    if w1.IsNaples() or w2.IsNaples():
        if w1.IsNaples():
            for if_ip in mgmt_ip:
                ping_cmd = "ping -I oob_mnic0 -c3 %s" % (if_ip)
            api.Trigger_AddNaplesCommand(req, w1.node_name, "dhclient oob_mnic0")
            api.Trigger_AddNaplesCommand(req, w1.node_name, "ifconfig oob_mnic0")
            api.Trigger_AddNaplesCommand(req, w1.node_name, ping_cmd)

        if w2.IsNaples():
            for if_ip in mgmt_ip:
                ping_cmd = "ping -I oob_mnic0 -c3 %s" % (if_ip)
            api.Trigger_AddNaplesCommand(req, w2.node_name, "dhclient oob_mnic0")
            api.Trigger_AddNaplesCommand(req, w2.node_name, "ifconfig oob_mnic0")
            api.Trigger_AddNaplesCommand(req, w2.node_name, ping_cmd)
        tc.resp = api.Trigger(req)
    else:
        tc.resp = None
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.config_only == "no":
        return api.types.status.SUCCESS
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

