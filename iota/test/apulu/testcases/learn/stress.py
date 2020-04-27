#! /usr/bin/python3
import random
import iota.harness.api as api
import iota.test.utils.arping as arp_utils
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils
import iota.test.apulu.utils.flow as flow_utils

def Setup(tc):
    tc.iterations = getattr(tc.args, "num_iterations", 1)
    tc.interval = getattr(tc.args, "interval", 5)
    tc.node = random.choice(api.GetNaplesHostnames())
    return api.types.status.SUCCESS

def Trigger(tc):
    i = 0
    workloads = api.GetWorkloads(tc.node)
    while i < tc.iterations:
        api.Logger.info(f"Starting iteration {i}")
        api.Logger.info(f"Clearing learn-db")
        if not learn_utils.ClearLearnData(tc.node):
            api.Logger.error("Failed to clear learn info at iteration %d" %i)
            return api.types.status.FAILURE
        api.Logger.info(f"Re-learning all endpoints")
        if not arp_utils.SendGratArp(workloads):
            api.Logger.error("Failed to send arp probes at iteration %d" %i)
            return api.types.status.FAILURE
        api.Logger.info(f"Sending some ARP replies")
        for j in range(2):
            arp_utils.SendArpReply(workloads)
            misc_utils.Sleep(2)
        learn_utils.DumpLearnData()
        if not learn_utils.ValidateLearnInfo(tc.node):
            api.Logger.error("Failed to validate learn data at iteration %d" %i)
            return api.types.status.FAILURE
        i += 1
        misc_utils.Sleep(tc.interval)
    return api.types.status.SUCCESS

def Verify(tc):
    misc_utils.Sleep(40) # letting remote mappings to get programmed
    if not learn_utils.ValidateLearnInfo():
        api.Logger.error("Learn validation failed")
        return api.types.status.FAILURE
    workload_pairs = config_api.GetWorkloadPairs(config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY, config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET)
    workload_pairs.extend(config_api.GetWorkloadPairs(config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY, config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET))
    workload_pairs.extend(config_api.GetWorkloadPairs(config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY, config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET))
    return conn_utils.ConnectivityTest(workload_pairs, [ 'icmp' ], [ 'ipv4' ], [ 64 ], 0, 'all')

def Teardown(tc):
    return flow_utils.clearFlowTable(None)
