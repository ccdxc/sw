#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils

def verifyVPPFlow(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    
    if tc.iterators.ipaf != "ipv4":
        return api.types.status.SUCCESS
    log_file = "/var/log/pensando/vpp_flow.log"
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        api.Logger.info("Checking ping %s <-> %s in vpp flow logs" % (
            w1.ip_address, w2.ip_address))
        command = "date"
        command = "grep -c 'ip, source: %s:0, destination: %s' %s" % (
            w1.ip_address, w2.ip_address, log_file)
        api.Trigger_AddNaplesCommand(req, w1.node_name, command)

    # Give a chance for the consumer to catchup
    time.sleep(1)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("verifyVPPFlow failed")
        return api.types.status.FAILURE
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("verifyVPPFlow command failed: %s" % (cmd))
            api.Logger.error("verifyVPPFlow resp: %s" % (resp))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies, tc.resp = traffic_utils.pingWorkloads(tc.workload_pairs, tc.iterators.ipaf, tc.iterators.pktsize)
    return api.types.status.SUCCESS

def Verify(tc):
    
    if  traffic_utils.verifyPing(tc.cmd_cookies, tc.resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return verifyVPPFlow(tc)

def Teardown(tc):
    return api.types.status.SUCCESS
