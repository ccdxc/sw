#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils
from apollo.config.objects.policy import SupportedIPProtos as IPProtos

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    elif tc.args.type == 'remote_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
    elif tc.args.type == 'igw_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_IGW_ONLY)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.FAILURE

#    import pdb; pdb.set_trace()
    tc.selected_objs = []
    if tc.iterators.action == 'deny':
        for pair in tc.workload_pairs:
            objs = config_api.GetPolicyObjectsByWorkload(pair[0])
            for obj in objs:
                for rule in obj.rules:
                    if rule.L3Match.Proto == IPProtos.ICMP:
                        tc.selected_objs.append(obj)
                        obj.Update()

    return api.types.status.SUCCESS

def Trigger(tc):
    flow_utils.clearFlowTable(tc.workload_pairs)
#    import pdb; pdb.set_trace()
    for pair in tc.workload_pairs:
        api.Logger.info("iperf between %s and %s" % (pair[0].ip_address, pair[1].ip_address))
    tc.cmd_cookies, tc.resp = traffic_utils.iperfWorkloads(tc.workload_pairs, tc.iterators.ipaf, tc.iterators.protocol, tc.iterators.pktsize)
    return api.types.status.SUCCESS

def Verify(tc):
    res = traffic_utils.verifyIPerf(tc.cmd_cookies, tc.resp)
    if tc.iterators.action == 'deny':
        #Deny Case, success is failure!
        if res == api.types.status.SUCCESS:
            return api.types.status.FAILURE
        res = flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)
        if  res == api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return api.types.status.SUCCESS
    else:
        #Allow Case, expect to succeed
        if  res != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)

def Teardown(tc):
    for obj in tc.selected_objs:
        obj.RollbackUpdate()

    return flow_utils.clearFlowTable(tc.workload_pairs)
