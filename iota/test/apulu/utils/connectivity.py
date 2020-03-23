#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.traffic as traffic_utils

def TriggerConnectivityTest(workload_pairs, proto, af, pktsize):
    cmd_cookies = []
    resp = None
    if proto == 'icmp':
        cmd_cookies, resp = traffic_utils.pingWorkloads(workload_pairs, af, pktsize)
    elif proto in ['tcp','udp']:
        cmd_cookies, resp = traffic_utils.iperfWorkloads(workload_pairs, af, proto, pktsize, "10G", 1, 1)
    else:
        api.Logger.error("Proto %s unsupported" % proto)
    return cmd_cookies, resp

def VerifyConnectivityTest(proto, cmd_cookies, resp):
    if proto == 'icmp':
        if traffic_utils.verifyPing(cmd_cookies, resp) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if proto in ['tcp','udp']:
        if traffic_utils.verifyIPerf(cmd_cookies, resp) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

# Testcases which want to test connectivity after a trigger, through
# different protocol packets with multiple packet sizes can use this API.
def ConnectivityTest(workload_pairs, proto_list, ipaf_list, pktsize_list):
    cmd_cookies = []
    resp = None
    for af in ipaf_list:
        for proto in proto_list:
            for pktsize in pktsize_list:
                cmd_cookies, resp = TriggerConnectivityTest(workload_pairs, proto, af, pktsize)
                return VerifyConnectivityTest(proto, cmd_cookies, resp)
    return api.types.status.SUCCESS
