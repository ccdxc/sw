#! /usr/bin/python3

import time
import iota.harness.api as api
import iota.test.iris.testcases.security.utils as utils
import iota.test.iris.testcases.aging.aging_utils as timeout_utils
from iota.test.iris.utils import vmotion_utils

'''
 Session Limit Proto : Traffic being tested for non interruption
 tcp    -> icmp traffic
 udp    -> tcp traffic
 icmp   -> udp traffic
'''

def SetTestSettings(tc):
    tc.proto = tc.iterators.proto
    if tc.proto == 'udp':
        tc.flood_proto = 'tcp'
        tc.timeout_field = 'tcp-connection-setup'
        tc.metric_field = 'num_tcp_half_open_sessions'
        test_timeout = '60s'
    elif tc.proto == 'icmp':
        tc.flood_proto = 'udp'
        tc.timeout_field = 'udp-timeout'
        test_timeout = '200s'
        tc.metric_field = 'num_udp_sessions'
    elif tc.proto == 'tcp':
        tc.flood_proto = 'icmp'
        tc.timeout_field = 'icmp-timeout'
        test_timeout = '200s'
        tc.metric_field = 'num_icmp_sessions'
    else:
        return api.types.status.FAILURE
    tc.timeout = timeout_utils.get_timeout_val(tc.timeout_field)
    if tc.timeout == api.types.status.FAILURE:
        return api.types.status.FAILURE
    return timeout_utils.update_timeout(tc.timeout_field, test_timeout)

def RestoreSettings(tc):
    if tc.proto not in ["tcp", "icmp", "udp"]:
        return api.types.status.FAILURE
    return timeout_utils.update_timeout(tc.timeout_field, tc.timeout)

def SetTrafficGeneratorCommand(tc, req):
    cmd_cookie = "hping flood"
    if tc.proto == 'udp':
        cmd = "hping3 %s -i u2000 -S -p 80 --flood" \
                %(tc.wc_client.ip_address)
    elif tc.proto == 'icmp':
        cmd = "hping3 %s --udp -p ++1 --flood" \
                %(tc.wc_client.ip_address)
    else:
        cmd = "hping3 -1 --flood --rand-source %s" \
            %(tc.wc_client.ip_address)

    api.Trigger_AddCommand(req, tc.wc_server.node_name, \
            tc.wc_server.workload_name, cmd, background = True)
    tc.cmd_cookies.append(cmd_cookie)


def Setup(tc):
    if SetTestSettings(tc) is not api.types.status.SUCCESS:
        return api.types.status.FAILURE
    if utils.SetSessionLimit('all', 0) is not api.types.status.SUCCESS:
        return api.types.status.FAILURE

    (server, client) = utils.GetServerClientSinglePair(kind=tc.iterators.kind)
    if server is None:
        return api.types.status.FAILURE
    tc.wc_server = server
    tc.wc_client = client
    if getattr(tc.args, 'vmotion_enable', False):
        wloads = [server]
        vmotion_utils.PrepareWorkloadVMotion(tc, wloads)

    return api.types.status.SUCCESS


def Trigger(tc):
    tc.cmd_cookies = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                (tc.wc_server.workload_name, tc.wc_server.ip_address,
                        tc.wc_client.workload_name, tc.wc_client.ip_address)
    api.Logger.info("Starting %s Session test from %s" % (tc.proto, cmd_cookie))

    utils.clearNaplesSessions(node_name=tc.wc_server.node_name)

    metrics = utils.GetDelphiSessionSummaryMetrics(tc.wc_server.node_name)
    api.Logger.info("Before Session summary metrics for %s => %s" % \
            (tc.wc_server.node_name, metrics))

    #Step 0: Update the session limit in the config object
    utils.SetSessionLimit(tc.proto, 100)

    if tc.flood_proto == 'tcp':
        cmd_cookie = "iptable drop rule"
        api.Trigger_AddCommand(req, tc.wc_client.node_name, \
                tc.wc_client.workload_name, \
                "iptables -A INPUT -p tcp --destination-port 80 -j DROP", \
                background = True)
        tc.cmd_cookies.append(cmd_cookie)

    SetTrafficGeneratorCommand(tc, req)

    cmd_cookie = "show sessions"
    api.Trigger_AddNaplesCommand(req, tc.wc_server.node_name, "/nic/bin/halctl show session")
    tc.cmd_cookies.append(cmd_cookie)
    trig_resp = api.Trigger(req)

    if tc.flood_proto == 'tcp':
        cmd_cookie = "iptable states"
        api.Trigger_AddCommand(req, tc.wc_client.node_name, \
                tc.wc_client.workload_name, \
                "iptables -L -v", background = True)
        tc.cmd_cookies.append(cmd_cookie)

    #give some time for the traffic to pass
    time.sleep(5)

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    api.Logger.info("Verify.")

    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadRestore(tc)

    if tc.resp is None:
        api.Logger.info("Null response from aggregartecommands")
        return api.types.status.FAILURE

    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)

    metrics = utils.GetDelphiSessionSummaryMetrics(tc.wc_server.node_name)
    api.Logger.info("After Session summary metrics for %s => %s" % \
            (tc.wc_server.node_name, metrics))

    limit = utils.GetSessionLimit(tc.proto)
    if limit == api.types.status.FAILURE or limit == 0:
        api.Logger.error("Session Limit invalid : %d"%limit)
        return api.types.status.FAILURE

    if metrics[tc.metric_field] <= limit:
        api.Logger.error("%s : %d Expected : %d" % \
                (tc.metric_field, metrics[tc.metric_field], limit))
        return api.types.status.FAILURE
    api.Logger.info("%s : %d is more than the %s limit : %d" % \
            (tc.metric_field, metrics[tc.metric_field], \
            tc.proto, limit))
    api.Logger.info("%s session Limit Success"%tc.proto)
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    if utils.SetSessionLimit('all', 0) is not api.types.status.SUCCESS:
        return api.types.status.FAILURE
    if tc.flood_proto == 'tcp':
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        api.Trigger_AddCommand(req, tc.wc_client.node_name, \
                tc.wc_client.workload_name, \
                "iptables -D INPUT -p tcp --destination-port 80 -j DROP", \
                background = True)
        trig_resp = api.Trigger(req)
    return RestoreSettings(tc)
