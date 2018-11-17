#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import pdb

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "%s(%s) --> %s(%s)" %\
                     (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Starting Ping test from %s" % (cmd_cookie))

        cmd_cookie = "halctl clear session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl clear session")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "ping -c 10"
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "ping -c 10 %s" % w2.ip_address)
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Before aging show session ICMP"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show session | grep ICMP")
        tc.cmd_cookies.append(cmd_cookie)         

        #Get it from the config
        cmd_cookie = "sleep 12"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "sleep 12")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "After aging show session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show session | grep ICMP")
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0: 
            #This is expected so dont set failure for this case
            if tc.cmd_cookies[cookie_idx].find("After aging") != -1 and cmd.stdout == '':
                break
            result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("clear session") != -1:
           if cmd.stdout != '':
              result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("Before aging") != -1:
           #Session were not established ?
           if cmd.stdout.find("ICMP") == -1:
               result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("After aging") != -1:
           #Check if sessions were aged
           if cmd.stdout.find("ICMP") != -1:
               result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
