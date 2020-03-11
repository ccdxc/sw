#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    elif tc.args.type == 'remote_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
    elif tc.args.type == 'igw_only':
        tc.workload_pairs = config_api.GetWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_IGW_ONLY,
            wl_pair_scope = config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    for pair in tc.workload_pairs:
        api.Logger.info("pinging between %s and %s" % (pair[0].ip_address, pair[1].ip_address))
    tc.cmd_cookies, tc.resp = traffic_utils.pingWorkloads(tc.workload_pairs, tc.iterators.ipaf, tc.iterators.pktsize)
    return api.types.status.SUCCESS

def Verify(tc):
    
    if  traffic_utils.verifyPing(tc.cmd_cookies, tc.resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    if tc.args.type != 'igw_only':
        return flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)
    return api.types.status.SUCCESS

def Teardown(tc):
    return flow_utils.clearFlowTable(tc.workload_pairs)
