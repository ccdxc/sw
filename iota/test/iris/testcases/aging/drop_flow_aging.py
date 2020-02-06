#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.test.iris.testcases.aging.aging_utils import *
import pdb

GRACE_TIME = 5

def Setup(tc):
    tc.resp = None
    tc.config_update_fail = 0
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs(naples=True)
    tc.cmd_cookies = []
    server,client  = pairs[0]
    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          return api.types.status.SUCCESS
       else:
          client, server = pairs[0]

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting Iperf test from %s" % (cmd_cookie))

    cmd_cookie = "halctl clear session"
    api.Trigger_AddNaplesCommand(req, server.node_name, "/nic/bin/halctl clear session")
    tc.cmd_cookies.append(cmd_cookie)

    server_port = api.AllocateTcpPort()
    basecmd = 'iperf -p %d ' % server_port
    proto   = 6
    timeout_str = 'tcp-drop'
    if tc.iterators.proto == 'udp':
        server_port = api.AllocateUdpPort()
        basecmd = 'iperf -u -p %d ' % server_port
        proto   = 17
        timeout_str = 'udp-drop'
    #Step 0: Update the timeout in the config object
    update_timeout(timeout_str, tc.iterators.timeout)
    update_sgpolicy(client.ip_address, server.ip_address, tc.iterators.proto, server_port)

    profilereq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(profilereq, naples.node_name, "/nic/bin/halctl show nwsec profile --id 16")
    profcommandresp = api.Trigger(profilereq)
    cmd = profcommandresp.commands[-1]
    for command in profcommandresp.commands:
        api.PrintCommandResults(command)
    timeout = get_haltimeout(timeout_str, cmd)
    api.Logger.info("Hal timeout %s, tc timeout %s" % (timeout, timetoseconds(tc.iterators.timeout)))
    if (timeout != timetoseconds(tc.iterators.timeout)):
        api.Logger.errror("Found mismatch in HAL and testcase timeout")
        tc.config_update_fail = 1
        return api.types.status.FAILURE

    timeout += GRACE_TIME
    cmd_cookie = "iperf -s"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "%s -s -t 300 " % basecmd, background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "iperf -c "
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "%s -c %s " % (basecmd, server.ip_address))
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Before aging show session"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --ipproto %s | grep %s" % (proto, naples.ip_address))
    tc.cmd_cookies.append(cmd_cookie)

    #Get it from the config
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep %s" % timeout, timeout=300)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "After aging show session"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session --ipproto %s | grep %s" % (proto, naples.ip_address))
    tc.cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        api.Logger.error("Empty response")
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    grep_cmd = "TCP"
    if tc.iterators.proto == 'udp':
        grep_cmd = "UDP"

    if tc.config_update_fail == 1:
        api.Logger.error("Updating timeout failed.")
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            #This is expected so dont set failure for this case
            if tc.cmd_cookies[cookie_idx].find("After aging") != -1 or \
               tc.cmd_cookies[cookie_idx].find("Before aging") != -1:
               result = api.types.status.SUCCESS
            else:
               result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("clear session") != -1:
           if cmd.stdout != '':
              result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("After aging") != -1:
           #Check if sessions were aged
           if cmd.stdout.find(grep_cmd) != -1:
               result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
