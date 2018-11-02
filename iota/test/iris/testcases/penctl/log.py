#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "Checking NMD logs"
        api.Logger.info("Starting NMD log test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "setenv PENETHDEV tun0")
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "penctl get logs -m nmd")
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    assert(len(tc.cmd_cookies) == len(tc.resp.commands))

    for cmd in tc.resp.commands:
        api.Logger.info("Logs for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
