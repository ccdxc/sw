#! /usr/bin/python3
import pdb
import re

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def parsePingResult(output):
    found = re.search(", 0% packet loss", output)
    return found

def extractPingStats(output):
    pkt_tx = 0
    loss_percent = 0

    m = re.search(r'\.*(\d+) packets transmitted.*, (\d+)% packet loss\.*', output)
    if m:
        pkt_tx = int(m.group(1))
        loss_percent = int(m.group(2))
    return (pkt_tx, loss_percent)

def calcPingPktLossDuration(pkt_tx, interval, loss_percent):
    loss_duration = (pkt_tx * loss_percent / 100) * float(interval)
    return loss_duration


def TestPing(tc, input_type, ipaf, pktsize, interval="0.2", count=20, deadline=0, pktlossverif=False, background=False):
    if input_type == 'user_input':
        api.Logger.info("user_input")
    elif input_type == 'local_only':
        api.Logger.info("local_only test")
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        api.Logger.info("remote_only test")
        tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetNicMode() == 'hostpin' and ipaf == 'ipv6':
        api.Logger.info("Skipping Testcase: IPv6 not supported in hostpin mode.")
        return api.types.status.SUCCESS

    req = None
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    tc.cmd_cookies = []

    deadline_str = ""
    count_str = ""
    cmd_timeout = 60
    if deadline:
        deadline_str = "-w {}".format(deadline)
        cmd_timeout = deadline + 5
    else:
        count_str = "-c {}".format(count)
        cmd_timeout = (count * float(interval)) + 5

    cmd_timeout = 60 if cmd_timeout < 60 else cmd_timeout

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if ipaf == 'ipv6':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping6 -q -i %s %s -s %d %s %s" % (interval, count_str,
                                   pktsize, w2.ipv6_address, deadline_str),
                                   background=background, timeout=cmd_timeout)
        else:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping -q -i %s %s -s %d %s %s" % (interval, count_str,
                                   pktsize, w2.ip_address, deadline_str),
                                   background=background, timeout=cmd_timeout)

        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    if not background:
        cookie_idx = 0
        for cmd in tc.resp.commands:
            api.Logger.info("ping results for %s" % (tc.cmd_cookies[cookie_idx]))
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                if api.GetNicMode() == 'hostpin' and pktsize > 1024:
                    result = api.types.status.SUCCESS
                else:
                    result = api.types.status.FAILURE
            elif pktlossverif:
                ping_result = parsePingResult(cmd.stdout)
                if not ping_result:
                    api.Logger.info("ping failed in packet loss verification")
                    result = api.types.status.FAILURE
                    break
            cookie_idx += 1
    else:
        tc.bg_cmd_cookies = tc.cmd_cookies
        tc.bg_cmd_resp   = tc.resp

    return result

def TestTerminateBackgroundPing(tc, pktsize, pktlossverif=False):

    result = api.types.status.SUCCESS
    term_resp = api.Trigger_TerminateAllCommands(tc.bg_cmd_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(tc.bg_cmd_resp, term_resp)

    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("ping results for %s" % (tc.bg_cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            if api.GetNicMode() == 'hostpin' and pktsize > 1024:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        elif pktlossverif:
            ping_result = parsePingResult(cmd.stdout)
            if not ping_result:
                api.Logger.info("Background ping failed in packet loss verification")
                result = api.types.status.FAILURE

        cookie_idx += 1
    return result

def GetMaxPktLossDuration(tc, interval):
    pkt_loss_duration = 0

    for cmd in tc.resp.commands:
        (pkt_tx, loss_percent) = extractPingStats(cmd.stdout)
        if loss_percent != 0:
            loss_duration = calcPingPktLossDuration(pkt_tx, interval, loss_percent)
            if loss_duration > pkt_loss_duration:
                pkt_loss_duration = loss_duration

    return pkt_loss_duration
