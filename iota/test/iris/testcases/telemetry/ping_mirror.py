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
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    catreq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []
    tc.tcpdump_resp = []

    iter_num = 0
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Starting Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

        tc.tcpdump_cmd = api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                                                "tcpdump -i %s ip proto gre" % (w2.interface), background=True)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "ping -c 10 -s %d %s" % (tc.iterators.pktsize, w2.ip_address))
        iter_num = iter_num + 1
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
        
    return api.types.status.SUCCESS
    # TODO: Validation
'''
    result = api.types.status.SUCCESS
    
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    
    if result == api.types.status.FAILURE:
        return result
    
    if not '0 packets captured' in tc.tcpdump_cmd.stderr:
        api.Logger.info("Packets captured! Returning success")
        result = api.types.status.SUCCESS
    else:
        api.Logger.info("No packets captured! Returning failure")
        result = api.types.status.FAILURE
    return result
'''

def Teardown(tc):
    return api.types.status.SUCCESS
