#! /usr/bin/python3
import time
import iota.harness.api as api
#import iota.test.iris.config.netagent.objects.security_profile as sec_profile_obj
import pdb

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = []
    server,client  = pairs[0]
    naples = server
    if not server.IsNaples():
       naples = client
       if not client.IsNaples():
          return api.types.status.SUCCESS
       else:
          client, server = pairs[0]

    cmd_cookie = "%s(%s) --> %s(%s)" %\
                (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
    api.Logger.info("Starting clear & show stress test from %s" % (cmd_cookie))

    
    basecmd = 'iperf -p %d ' % api.AllocateTcpPort()
    proto   = 6
    timeout = 250 
    #tc.secprof = sec_profile_obj.gl_securityprofile_json_template
    #timeout = int(tc.secprof['security-profiles'][0]['spec']['timeouts']['tcp']) + \
    #          int(tc.secprof['security-profiles'][0]['spec']['timeouts']['tcp-close'])
    if tc.iterators.proto == 'udp':
        basecmd = 'iperf -u -p %d ' % api.AllocateUdpPort()
        proto   = 17
        timeout = 150
        #timeout = tc.security_profile['security-profiles'][0]['spec']['timeouts']['udp']

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    for cnt in range(tc.args.count):
        cmd_cookie = "iperf -s"
        api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "%s-s -t 300" % basecmd, background = True)
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "iperf -c "
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "%s -c %s -P 100" % (basecmd, server.ip_address))
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Show session"
        api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl show session")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Clear session"
        api.Trigger_AddNaplesCommand(req, naples.node_name, "/nic/bin/halctl clear session")
        tc.cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
