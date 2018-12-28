#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def Setup(tc):
    tc.skip = False

    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetNicMode() == 'hostpin' and tc.iterators.ipaf == 'ipv6':
        api.Logger.info("Skipping Testcase: IPv6 not supported in hostpin mode.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if tc.iterators.ipaf == 'ipv6':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping6 -i 0.2 -c 20 -s %d %s" % (tc.iterators.pktsize, w2.ipv6_address))
        else:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping -i 0.2 -c 20 -s %d %s" % (tc.iterators.pktsize, w2.ip_address))
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    req2 = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for w in api.GetWorkloads():
        api.Trigger_AddCommand(req2, w.node_name, w.workload_name,
                               "ls /root/ > %s_ls.out" % w.workload_name)
    tc.resp2 = api.Trigger(req2)

    for w in api.GetWorkloads():
        api.CopyFromWorkload(w.node_name, w.workload_name,
                             [ '%s_ls.out' % w.workload_name ], tc.GetLogsDir())

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
