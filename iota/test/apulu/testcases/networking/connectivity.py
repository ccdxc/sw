#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.utils.connectivity as conn_utils

def __get_workload_type(tc):
    type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY

    if not hasattr(tc.iterators, 'workload_type'):
        return type

    if tc.iterators.workload_type == 'local':
        type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY
    elif tc.iterators.workload_type == 'remote':
        type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY
    elif tc.iterators.workload_type == "igw":
        type = config_api.WORKLOAD_PAIR_TYPE_IGW_ONLY

    return type

def __get_workload_scope(tc):
    scope = config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET

    if not hasattr(tc.iterators, 'workload_scope'):
        return scope
    if tc.iterators.workload_scope == 'intra-subnet':
        scope = config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET
    elif tc.iterators.workload_scope == 'inter-subnet':
        scope = config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET
    elif tc.iterators.workload_scope == 'inter-vpc':
        scope = config_api.WORKLOAD_PAIR_SCOPE_INTER_VPC

    return scope

def Setup(tc):
    tc.workload_pairs = config_api.GetWorkloadPairs(__get_workload_type(tc), __get_workload_scope(tc))
    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    for pair in tc.workload_pairs:
        api.Logger.info("%s between %s and %s" % (tc.iterators.proto, pair[0].ip_address, pair[1].ip_address))

    tc.cmd_cookies, tc.resp = conn_utils.TriggerConnectivityTest(tc.workload_pairs, tc.iterators.proto, tc.iterators.ipaf, tc.iterators.pktsize)
    return api.types.status.SUCCESS

def Verify(tc):
    if conn_utils.VerifyConnectivityTest(tc.iterators.proto, tc.cmd_cookies, tc.resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    if tc.iterators.workload_type == "igw":
        return flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)

    return api.types.status.SUCCESS

def Teardown(tc):
    return flow_utils.clearFlowTable(tc.workload_pairs)
