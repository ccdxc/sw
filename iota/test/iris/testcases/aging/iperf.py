#! /usr/bin/python3
import time
import iota.harness.api as api
#import iota.test.iris.config.netagent.objects.security_profile as sec_profile_obj
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
    api.Logger.info("Starting Iperf test from %s" % (cmd_cookie))

    if w1.IsNaples():
        cmd_cookie = "halctl clear session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl clear session")
        tc.cmd_cookies.append(cmd_cookie)

    basecmd = 'iperf '
    proto   = 6
    timeout = 250 
    #tc.secprof = sec_profile_obj.gl_securityprofile_json_template
    #timeout = int(tc.secprof['security-profiles'][0]['spec']['timeouts']['tcp']) + \
    #          int(tc.secprof['security-profiles'][0]['spec']['timeouts']['tcp-close'])
    if tc.iterators.proto == 'udp':
        basecmd = 'iperf -u '
        proto   = 17
        timeout = 150
        #timeout = tc.security_profile['security-profiles'][0]['spec']['timeouts']['udp']

    cmd_cookie = "iperf -s"
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "%s-s -t 300" % basecmd, background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "iperf -c "
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "%s -c %s" % (basecmd, w1.ip_address))
    tc.cmd_cookies.append(cmd_cookie)

    if w1.IsNaples():
        cmd_cookie = "Before aging show session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show session --ipproto %s | grep %s" % (proto, w1.ip_address))
        tc.cmd_cookies.append(cmd_cookie)

        #Get it from the config
        cmd_cookie = "sleep"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "sleep %s" % timeout, timeout=300)
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "After aging show session"
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show session --ipproto %s | grep %s" % (proto, w1.ip_address))
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
    grep_cmd = "TCP"
    if tc.iterators.proto == 'udp':
        grep_cmd = "UDP"

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
           if cmd.stdout.find("") == -1:
               result = api.types.status.FAILURE
        elif tc.cmd_cookies[cookie_idx].find("After aging") != -1:
           #Check if sessions were aged
           if cmd.stdout.find(grep_cmd) != -1:
               result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
