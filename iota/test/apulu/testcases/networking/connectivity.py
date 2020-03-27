#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.utils.connectivity as conn_utils

def Setup(tc):
    tc.workload_pairs = config_api.GetWorkloadPairs(conn_utils.Get_workload_type(tc), conn_utils.Get_workload_scope(tc))
    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        if tc.iterators.workload_type == 'local' and tc.iterators.workload_scope == 'intra-subnet':
            # Currently we dont support local-to-local intra-subnet connectivity
            return api.types.status.SUCCESS
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
