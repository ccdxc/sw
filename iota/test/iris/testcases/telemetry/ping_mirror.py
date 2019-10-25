#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import re
import time

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

    iter_num = 0
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Starting Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                               "tcpdump -nni %s ether proto arp" % (w2.interface), background=True)
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                               "tcpdump -nni %s ip proto gre" % (w2.interface), background=True)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "ping -f -c 50 -s %d %s" % (tc.iterators.pktsize, w2.ip_address))
        iter_num = iter_num + 1
    trig_resp = api.Trigger(req)
    time.sleep(10)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    #import pdb; pdb.set_trace()    
    result = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
            api.Logger.info('Setting API FAILURE for command: ' + cmd.command)
        if 'tcpdump' in cmd.command:
            # Check stdout first
            matchObj = re.search( r'(.*) GREv0, length 158(.*)', cmd.stdout, 0)
            if matchObj is None:
                result = api.types.status.FAILURE
    return result
    # Capture packets based on num pkts. Commented out due to iota issue where stdout
    # is not returned correctly
'''
        if 'tcpdump' in cmd.command:
            # Check stdout first
            matchObj = re.search( r'(.*) packets received by filter', cmd.stdout, 0)
            if matchObj is None:
                # Check stderr
                matchObj = re.search( r'(.*) packets received by filter', cmd.stderr, 0)
            if matchObj is not None:
                api.Logger.info('Matching string: ' + matchObj.group())
                if int(matchObj.group(1)) == 0:
                    api.Logger.info('No packets captured! Return failure!!')
                    result = api.types.status.FAILURE
            else:
                api.Logger.info('matchObj is None! Return failure!!')
                result = api.types.status.FAILURE
'''

def Teardown(tc):
    return api.types.status.SUCCESS
