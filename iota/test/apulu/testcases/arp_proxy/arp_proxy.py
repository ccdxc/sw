#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.utils.vppctl as vpp_utils

def __clearErrorCounters():
    for node in api.GetNaplesHostnames():
        ret = vpp_utils.ParseClearCommand(node, "errors")
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run clear errors at node {node}")

    return ret

def __verifyErrorsCmd(resp, wl_type, sent_probes):
    ret = api.types.status.SUCCESS

    if api.GlobalOptions.dryrun:
        return ret

    arp_node = 'pds-arp-proxy'
    tups = resp.split()
    indexPosList = [ index for index, value in enumerate(tups) if value == arp_node ]

    recvd_probes = 0
    for indx in indexPosList:
        recvd_probes += int(tups[indx - 1])

    if sent_probes > int(recvd_probes):
        api.Logger.error(f"received {recvd_probes} and expected {sent_probes}")
        ret = api.types.status.FAILURE

    if wl_type == 'self':
        reason = ['Duplicate', 'address', 'detection', 'drops']
        count = 0
        for c in indexPosList:
            index = c + 1
            for val in reason:
                if val != tups[index]:
                    match = False
                    break
                else:
                    match = True
                index += 1
            if match:
                count += int(tups[int(c) - 1])

        if count != sent_probes:
            api.Logger.error(f"tuples: {tups} received less packets, expected {sent_probes} and recvd {count}")
            ret = api.types.status.FAILURE
    return ret

def __verifyVPPCtlErrors(wl_type=None, sent_probes=None):
    for node in api.GetNaplesHostnames():
        ret, resp = vpp_utils.ParseShowCommand(node, "errors")
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run show errors at node {node}")
        else:
            count = sent_probes.get(node, 0) if sent_probes else 0
            ret = __verifyErrorsCmd(resp, wl_type, count)

    return ret

def __getDuplicateWorkloads():
    wl_pairs = []
    for wl in api.GetWorkloads():
        wl_pairs.append((wl, wl))
    return wl_pairs

def Setup(tc):
    tc_name = tc.Name().rsplit('_', 1)[0]
    if tc_name == 'ARP_Self_AddressDetection':
        tc.wl_type = "self"
    else:
        tc.wl_type = conn_utils.GetWorkloadType(tc.iterators)
    tc.wl_scope = conn_utils.GetWorkloadScope(tc.iterators)

    tc.workload_pairs = list()
    if tc.wl_type == "self":
        tc.workload_pairs = __getDuplicateWorkloads()
    else:
        tc.workload_pairs = config_api.GetWorkloadPairs(tc.wl_type, tc.wl_scope)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.IGNORED

    return api.types.status.SUCCESS

def Trigger(tc):
    __clearErrorCounters()

    tc.args_type = getattr(tc.args, 'type', None)
    if tc.wl_type == "self":
        tc.args_type = 'DAD'

    for pair in tc.workload_pairs:
        api.Logger.debug(f"arp between {pair[0].ip_address} and {pair[1].ip_address}")

    tc.cmd_cookies, tc.resp, tc.sent_probes = conn_utils.ConnectivityARPingTest(tc.workload_pairs, tc.args_type)

    return api.types.status.SUCCESS

def Verify(tc):
    traffic_res = api.types.status.SUCCESS
    expected_exit_code = 0
    if tc.wl_scope == config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET or tc.args_type == 'DAD':
        expected_exit_code = 1

    if conn_utils.VerifyConnectivityTest('arp', tc.cmd_cookies,
            tc.resp, expected_exit_code) != api.types.status.SUCCESS:
        if tc.wl_type != "self":
            # check if packets are dropped instead of returning failure
            traffic_res = api.types.status.FAILURE

    verif_res =  __verifyVPPCtlErrors(tc.wl_type, tc.sent_probes)
    api.Logger.verbose("TestCaseVerifyStatus: {traffic_res} and {verif_res}")
    if traffic_res != api.types.status.SUCCESS:
        return traffic_res
    # DAD case, arping exits on first reply, so sent packets may not be equal to recvd packets
    if tc.wl_type != "self" and tc.args_type == 'DAD':
        return api.types.status.SUCCESS
    return verif_res

def Teardown(tc):
    return __clearErrorCounters()
