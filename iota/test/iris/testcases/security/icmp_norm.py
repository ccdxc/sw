#! /usr/bin/python3
import pdb
import re
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.utils.hal_show as hal_show_utils

icmp_pkt_count = 5000
icmp_pkt_size  = 2500

def Setup(tc):
    if tc.args.type != 'remote_only':
        api.Logger.info("This test runs only for remote workloads. Ignoring test for non-remote type")
        return api.types.status.SUCCESS

    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Running ICMP Normalization tests ...")
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        # pick Naples workload as the source for hping
        if not w1.IsNaples():
            continue

        # get drop stats before test
        naples_cmd = r"/nic/bin/halctl show system statistics drop ingress | grep 'Drop ICMP/ICMPv6 Fragment Pkt'"
        api.Trigger_AddNaplesCommand(req, w1.node_name, naples_cmd)
        tc.cmd_cookies.append("ICMP Fragment Drops at start of test")

        cmd_cookie = "Checking connectivity, sending ping from %s(%s) --> %s(%s)" % \
                (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        tc.cmd_cookies.append(cmd_cookie)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "ping -c 1 -s 128 %s" %(w2.ip_address))

        cmd_cookie = "%s(%s) --> %s(%s)" % \
                (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Injecting ICMP fragment pkt from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "hping3 --icmp -i u1000 -c %s -d %s %s" %(icmp_pkt_count, icmp_pkt_size, w2.ip_address))

        # get drop stats after test
        naples_cmd = r"/nic/bin/halctl show system statistics drop ingress | grep 'Drop ICMP/ICMPv6 Fragment Pkt'"
        api.Trigger_AddNaplesCommand(req, w1.node_name, naples_cmd)
        tc.cmd_cookies.append("ICMP Fragment Drops at end of test")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    api.Logger.info("Validating ...")
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cmd_resp_len = len(tc.resp.commands)
    cookie_idx = 0

    while cookie_idx < cmd_resp_len:

        icmp_frag_drop_start = 0
        icmp_frag_drop_end = 0
        if "Fragment Drops at start of test" in tc.cmd_cookies[cookie_idx]:
            cmd = tc.resp.commands[cookie_idx]
            for line in cmd.stdout.split('\n'):
                if "Fragment" in line:
                    for s in line.split():
                        if s.isdigit():
                            icmp_frag_drop_start = int(s)
        if "Fragment Drops at end of test" in tc.cmd_cookies[cookie_idx+3]:
            cmd = tc.resp.commands[cookie_idx+3]
            for line in cmd.stdout.split('\n'):
                if "Fragment" in line:
                    for s in line.split():
                        if s.isdigit():
                            icmp_frag_drop_end = int(s)

        api.PrintCommandResults(tc.resp.commands[cookie_idx])
        api.Logger.info(tc.cmd_cookies[cookie_idx+1])
        api.PrintCommandResults(tc.resp.commands[cookie_idx+1])
        api.PrintCommandResults(tc.resp.commands[cookie_idx+2])
        api.PrintCommandResults(tc.resp.commands[cookie_idx+3])

        # drop is 2 times the number of packets sent
        if (icmp_frag_drop_start == icmp_frag_drop_end):
            api.Logger.info("No change in ICMP fragment drop, possible connectivity issues b/w few workloads")
        if (icmp_frag_drop_start + (2 * icmp_pkt_count)) != icmp_frag_drop_end:
            result = api.types.status.FAILURE
            api.Logger.info("Test Failed, ICMP frag drop before: %s, after: %s" %(icmp_frag_drop_start,icmp_frag_drop_end))
        cookie_idx += 4

    if result != api.types.status.FAILURE:
        api.Logger.info("Test Passed")

    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
