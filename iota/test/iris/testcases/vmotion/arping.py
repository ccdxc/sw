#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def print_workloads(wl_pairs):
    api.Logger.info("Printing workload pairs")
    for (node1,node2) in wl_pairs:
      api.Logger.info(" %s ---> %s" %(node1.workload_name, node2.workload_name))


def ArPing(tc):
    if tc.args.type == 'local_only':
        api.Logger.info("local_only test")
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    elif tc.args.type == 'both':
        api.Logger.info(" both local and remote test")
        tc.workload_pairs = api.GetLocalWorkloadPairs()
        tc.workload_pairs.extend(api.GetRemoteWorkloadPairs())
    else:
        api.Logger.info("remote_only test")
        tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    req = None
    interval = "0.2"
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        interval = "3"
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "%s %s %s %s" % (w1.node_name, w1.workload_name, w1.interface, w1.ip_address)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "arping -c  5 -U %s -I eth1" % (w1.ip_address))
        api.Logger.info("ArPing from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "%s %s %s %s" % (w2.node_name, w2.workload_name, w2.interface, w2.ip_address)
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                               "arping -c  5 -U %s -I eth1" % (w2.ip_address))
        api.Logger.info("ArPing from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("ArPing Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result
