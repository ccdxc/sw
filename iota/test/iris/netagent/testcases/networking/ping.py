#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = topo_svc_pb2.TriggerMsg()
    req.trigger_op = topo_svc_pb2.EXEC_CMDS
    req.trigger_mode = topo_svc_pb2.TRIGGER_SERIAL
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Starting Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

        cmd = req.commands.add()
        cmd.mode = topo_svc_pb2.COMMAND_FOREGROUND
        cmd.workload_name = w1.workload_name
        cmd.node_name = w1.node_name
        #cmd.command = "ping -c 10 -w 100 -W 10 -i 2 %s" % w2_ip_address
        cmd.command = "ping -c 10 %s" % w2.ip_address
    
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    assert(len(tc.cmd_cookies) == len(tc.resp.commands))

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
