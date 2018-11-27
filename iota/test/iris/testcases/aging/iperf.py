#! /usr/bin/python3
import time
import iota.harness.api as api
import pdb

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    tc.cmd_cookies = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Ping test from %s" % (cmd_cookie))

    if w1.IsNaples():
        cmd_cookie = "halctl clear session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl clear session")
        tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "iperf -s"
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "iperf -s -t 300 -u", background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "iperf -c "
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "iperf -c %s -u" % w1.ip_address)
    tc.cmd_cookies.append(cmd_cookie)

    if w1.IsNaples():
        cmd_cookie = "Before aging show session UDP"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show session | grep UDP")
        tc.cmd_cookies.append(cmd_cookie)

        #Get it from the config
        cmd_cookie = "sleep 60"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "sleep 60")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "After aging show session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show session | grep UDP")
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
            #This is expected so dont set failure for this case
            if tc.cmd_cookies[cookie_idx].find("After aging") != -1 and cmd.stdout == '':
                break
            result = api.types.status.FAILURE
        if tc.cmd_cookies[cookie_idx].find("clear session") != -1:
           if cmd.stdout != '':
              result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("Before aging") != -1:
           #Session were not established ?
           if cmd.stdout.find("UDP") == -1:
               result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("After aging") != -1:
           #Check if sessions were aged
           if cmd.stdout.find("UDP") != -1:
               result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
