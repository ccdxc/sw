#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.test.iris.testcases.aging.aging_utils import *
import pdb

GRACE_TIME=5

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()
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
    api.Logger.info("Starting UDP flood test from %s" % (cmd_cookie))

    cmd_cookie = "halctl clear session"
    api.Trigger_AddNaplesCommand(req, server.node_name, "/nic/bin/halctl clear session")
    tc.cmd_cookies.append(cmd_cookie)

    #Step 0: Update the timeout in the config object
    update_timeout('udp-timeout', '10s')

    timeout = timetoseconds('10s')

    cmd_cookie = "start server"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "hping3 %s --udp -p ++1 --flood"%(client.ip_address), background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Before aging show session"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session")
    tc.cmd_cookies.append(cmd_cookie)

    #Get it from the config
    cmd_cookie = "sleep"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "sleep 100", timeout=300)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "After aging show session"
    api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session")
    tc.cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
           if tc.cmd_cookies[cookie_idx].find("After aging show session") != -1:
               result = api.types.status.SUCCESS
           else:
               result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
