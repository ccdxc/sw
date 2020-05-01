#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils
from iota.harness.infra.glopts import GlobalOptions

def Setup(tc):
    tc.skip = False

    nodes = api.GetWorkloadNodeHostnames()
    tc.naples_node0 = nodes[0]
    tc.naples_node1 = nodes[1]

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    req = None
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
 
    tc.cmd_cookies = []
    if tc.args.type == 'remote_only':
        tc.control_ep_pairs = config_api.GetControlEPPairs(
            pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)

    if len(tc.control_ep_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no VCN Endpoint pairs.")
        return api.types.status.FAILURE
 
    for pair in tc.control_ep_pairs:
        api.Logger.verbose("Ping between %s and %s" % (pair[0].ip_addresses[0], pair[1].ip_addresses[0]))
    if pair[0].node_name == tc.naples_node0:
        cmd_cookie = "ip netns exec %s ping %s -c 10 -s %d" % ("vcn", pair[1].ip_addresses[0], tc.iterators.pktsize)
    else:
        cmd_cookie = "ip netns exec %s ping %s -c 10 -s %d" % ("vcn", pair[0].ip_addresses[0], tc.iterators.pktsize)

    api.Trigger_AddNaplesCommand(req, tc.naples_node0, cmd_cookie)
    api.Logger.info("Ping test %s" % (cmd_cookie))
    tc.cmd_cookies.append(cmd_cookie)
    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    if  traffic_utils.verifyPing(tc.cmd_cookies, tc.resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
