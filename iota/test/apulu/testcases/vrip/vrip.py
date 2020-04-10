#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.utils.vppctl as vpp_utils

def __clearErrorCounters():
    res = api.types.status.SUCCESS
    for node in api.GetNaplesHostnames():
        ret = vpp_utils.ParseClearCommand(node, "errors")
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run clear errors at node {node}")
            res = ret

    return res

def __parseErrCount(node, tups, indexPosList, sent_probes):
    ret = api.types.status.SUCCESS
    recvd_probes = 0
    for c in indexPosList:
        recvd_probes += int(tups[c - 1])

    if sent_probes > recvd_probes:
            api.Logger.error(f"received {recvd_probes} and expected {sent_probes}")
            return api.types.status.FAILURE

    return ret

def __parseErrReasons(node, tups, indexPosList, wl_scope):
    ret = api.types.status.SUCCESS
    node_reasons = dict()
    node_reasons['pds-arp-proxy'] = ['Reply', 'success']
    if wl_scope == config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET:
        node_reasons['pds-arp-proxy'] = ['Subnet', 'check', 'failed']
    node_reasons['ip4-icmp-input'] = ['echo', 'replies', 'sent']
    node_reasons['pds-flow-classify'] = ['Unknown', 'VR', 'IPv4']

    for c in indexPosList:
        index = c + 1
        for val in node_reasons[node]:
            if val != tups[index]:
                ret = api.types.status.FAILURE
                break
            index += 1

    return ret

def __verifyErrorsCmd(resp, proto, sent_probes, wl_scope):
    ret = api.types.status.SUCCESS
    if api.GlobalOptions.dryrun:
        return ret

    arp_node = 'pds-arp-proxy'
    icmp_node = 'ip4-icmp-input'
    flow_node = 'pds-flow-classify'

    node = flow_node
    if proto == 'arp':
        node = arp_node
    elif proto == 'icmp':
        node = icmp_node

    tups = resp.split()
    indexPosList = [ index for index, value in enumerate(tups) if value == node ]
    ret = __parseErrCount(node, tups, indexPosList, sent_probes)
    if ret != api.types.status.SUCCESS:
        return ret
    return __parseErrReasons(node, tups, indexPosList, wl_scope)

def __verifyVPPCtlErrors(proto='icmp', wl_scope= config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET,
        sent_probes=None):
    res = api.types.status.SUCCESS
    for node in api.GetNaplesHostnames():
        ret, resp = vpp_utils.ParseShowCommand(node, "errors")
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run show errors at node {node}")
            res = ret
        else:
            count = sent_probes.get(node, 0) if sent_probes else 0
            ret = __verifyErrorsCmd(resp, proto, count, wl_scope)
            if ret != api.types.status.SUCCESS:
                res = ret

    return res

def Setup(tc):
    tc.wl_scope = conn_utils.GetWorkloadScope(tc.iterators)
    tc.proto = getattr(tc.iterators, 'proto', 'icmp')
    tc.af = getattr(tc.iterators, 'ipaf', 'ipv4')
    tc.pktsize = getattr(tc.iterators, 'pktsize', 64)

    return api.types.status.SUCCESS

def Trigger(tc):
    __clearErrorCounters()

    tc.cmd_cookies, tc.resp, tc.sent_probes = conn_utils.ConnectivityVRIPTest(tc.proto, tc.af,
            tc.pktsize, tc.wl_scope)

    return api.types.status.SUCCESS

def Verify(tc):
    traffic_res = api.types.status.SUCCESS
    expected_exit_code = 0

    if len(tc.sent_probes.items()) <= 0:
        return api.types.status.SUCCESS

    if tc.proto == 'arp' and tc.wl_scope == config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET:
        expected_exit_code = 1
    if tc.proto in [ 'tcp', 'udp' ]:
        expected_exit_code = 1

    if tc.proto in ['tcp', 'udp']:
        if traffic_utils.verifyPing(tc.cmd_cookies, tc.resp, expected_exit_code) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    else:
        if conn_utils.VerifyConnectivityTest(tc.proto, tc.cmd_cookies,
                tc.resp, expected_exit_code) != api.types.status.SUCCESS:
            traffic_res = api.types.status.FAILURE

    verif_res = __verifyVPPCtlErrors(tc.proto, tc.wl_scope, tc.sent_probes)
    api.Logger.verbose("TestCaseVerifyStatus: {traffic_res} and {verif_res}")
    if traffic_res != api.types.status.SUCCESS:
        return traffic_res
    return verif_res

def Teardown(tc):
    return __clearErrorCounters()
