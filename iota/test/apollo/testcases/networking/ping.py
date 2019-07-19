#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.test.apollo.config.api as config_api

def Setup(tc):
    tc.skip = False

    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    req = None
    count = "5"
    interval = "0.2"
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        interval = "3"
    tc.cmd_cookies = []

    ping_base_cmd = "ping -A -W 1 -c %s -i %s -s %s " % (count, interval, tc.iterators.pktsize)
    ping6_base_cmd = "ping6 -A -W 1 -c %s -i %s -s %s " % (count, interval, tc.iterators.pktsize)
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if tc.iterators.ipaf == 'ipv6':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            ping_cmd = ping6_base_cmd + w2.ipv6_address
        else:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            ping_cmd = ping_base_cmd + w2.ip_address
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, ping_cmd)
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)


    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
