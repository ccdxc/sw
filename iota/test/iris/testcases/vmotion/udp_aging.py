#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.testcases.vmotion.vm_utils  as vm_utils 
from iota.test.iris.testcases.aging.aging_utils import *
import pdb

GRACE_TIME=5
VMOTION_UDP_TIMEOUT = 100

def Setup(tc):
    tc.Nodes    = api.GetNaplesHostnames()
    tc.uuidMap  = api.GetNaplesNodeUuidMap()
    tc.wl       = None
    tc.old_node = None
    tc.new_node = None 
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.args.type == 'local_only':
        local_pairs = api.GetLocalWorkloadPairs()
    else:
        remote_pairs = api.GetRemoteWorkloadPairs()
    tc.skip_security_prof = getattr(tc.args, "skip_security_prof", False)
    tc.cmd_cookies       = []
    tc.cmd_cookies_after = []
    server,client  = local_pairs[0]
    wl1 = server
    wl2 = client
    tc.wl       = wl1
    tc.old_node = server.node_name

    if not server.IsNaples():
        return api.types.status.SUCCESS

    # remove the node being used to pick wl for traffic
    # pick the next node in the naples nodes
    naples_nodes = tc.Nodes[:]
    naples_nodes.remove(server.node_name)
    assert(len(naples_nodes) >= 1)
    new_node     = naples_nodes[0]
    tc.new_node  = new_node

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting Iperf test from %s" % (cmd_cookie))

    cmd_cookie = "halctl clear session"
    api.Trigger_AddNaplesCommand(req, server.node_name, "/nic/bin/halctl clear session")
    tc.cmd_cookies.append(cmd_cookie)

    server_port = api.AllocateUdpPort()
    basecmd = 'iperf -u '
    timeout_str = 'udp-timeout'
    if not tc.skip_security_prof:
        timeout = get_timeout(timeout_str)
    else:
        timeout = VMOTION_UDP_TIMEOUT

    #Step 0: Update the timeout in the config object
    if not tc.skip_security_prof:
        update_timeout(timeout_str, tc.iterators.timeout)

    #profilereq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #api.Trigger_AddNaplesCommand(profilereq, wl1.node_name, "/nic/bin/halctl show nwsec profile --id 11")
    #profcommandresp = api.Trigger(profilereq)
    #cmd = profcommandresp.commands[-1]
    #for command in profcommandresp.commands:
    #    api.PrintCommandResults(command)
    #timeout = get_haltimeout(timeout_str, cmd)
    #tc.config_update_fail = 0
    #if (timeout != timetoseconds(tc.iterators.timeout)):
    #    tc.config_update_fail = 1

    cmd_cookie = "start server"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "%s -p %s -s -t 180" % (basecmd, server_port), background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "start client"
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "%s -p %s -c %s -t 180" % (basecmd, server_port, server.ip_address), background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Before move show session"
    api.Trigger_AddNaplesCommand(req, wl1.node_name, "/nic/bin/halctl show session --dstip %s | grep UDP" % (server.ip_address))
    tc.cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)

    vm_utils.do_vmotion(tc, server, new_node)

    req_after = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    cmd_cookie = "After move show session"
    api.Trigger_AddNaplesCommand(req_after, wl1.node_name, "/nic/bin/halctl show session --dstip %s | grep UDP" % (server.ip_address))
    tc.cmd_cookies_after.append(cmd_cookie)

    #Get it from the config
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req_after, wl1.node_name, "sleep %s" % (timeout + GRACE_TIME + 180), timeout=300)
    tc.cmd_cookies_after.append(cmd_cookie)

    cmd_cookie = "After aging show session"
    api.Trigger_AddNaplesCommand(req_after, wl1.node_name, "/nic/bin/halctl show session --dstip %s | grep UDP" % (server.ip_address))
    tc.cmd_cookies_after.append(cmd_cookie)

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp   = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    trig_resp_after = api.Trigger(req_after)
    term_resp_after = api.Trigger_TerminateAllCommands(trig_resp_after)
    tc.resp_after   = api.Trigger_AggregateCommandsResponse(trig_resp_after, term_resp_after)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    #if tc.config_update_fail == 1:
    #    return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    grep_cmd = "UDP"

    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("clear session") != -1:
           if cmd.stdout != '':
              result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("Before move") != -1:
           #Session were not established ?
           if cmd.stdout.find("") == -1:
               result = api.types.status.FAILURE
        cookie_idx += 1

    cookie_idx = 0
    for cmd in tc.resp_after.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies_after[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            #This is expected so dont set failure for this case
            if tc.cmd_cookies_after[cookie_idx].find("After aging") != -1 and cmd.stdout == '':
               result = api.types.status.SUCCESS
            else:
               result = api.types.status.FAILURE
        elif tc.cmd_cookies_after[cookie_idx].find("After move") != -1:
            #Session were not established ?
            if cmd.stdout.find("") == -1:
                result = api.types.status.FAILURE
        elif tc.cmd_cookies_after[cookie_idx].find("After aging") != -1:
            #Check if sessions were aged
            if cmd.stdout.find(grep_cmd) != -1:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    time.sleep(5)
    if tc.GetStatus() != api.types.status.SUCCESS:
        api.Logger.info("verify failed, returning without teardown")
        return tc.GetStatus()
    vm_utils.vm_move_back(tc)
    return api.types.status.SUCCESS
