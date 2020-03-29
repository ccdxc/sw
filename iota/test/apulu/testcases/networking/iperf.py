#! /usr/bin/python3
import time

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils
from apollo.config.objects.policy import SupportedIPProtos as IPProtos
from iota.harness.infra.glopts import GlobalOptions
import apollo.config.objects.nat_pb as nat_pb

def Setup(tc):
    tc.num_streams = getattr(tc.args, "num_streams", 1)
    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    elif tc.args.type == 'remote_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
    elif tc.args.type == 'igw_napt_only':
        tc.workload_pairs = config_api.GetWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_IGW_NAPT_ONLY,
            wl_pair_scope = config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET)
        tc.nat_port_blocks = config_api.GetAllNatPortBlocks()
        tc.nat_pre_stats = {}
        tc.nat_pre_stats['icmp'] = nat_pb.NatPbStats()
        tc.nat_pre_stats['udp'] = nat_pb.NatPbStats()
        tc.nat_pre_stats['tcp'] = nat_pb.NatPbStats()
        for pb in tc.nat_port_blocks:
            stats = pb.GetStats()
            tc.nat_pre_stats[pb.ProtoName].Add(stats)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    for pair in tc.workload_pairs:
        api.Logger.info("iperf between %s and %s" % (pair[0].ip_address, pair[1].ip_address))
    tc.cmd_cookies, tc.resp = traffic_utils.iperfWorkloads(tc.workload_pairs, tc.iterators.ipaf, \
            tc.iterators.protocol, tc.iterators.pktsize, num_of_streams=tc.num_streams)
    return api.types.status.SUCCESS

def Verify(tc):
    res = traffic_utils.verifyIPerf(tc.cmd_cookies, tc.resp)
    if tc.args.type != 'igw_napt_only':
        return flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)
    else:
        if tc.iterators.protocol == 'udp':
            num_tcp_flows = 1
            num_udp_flows = tc.num_streams
        else:
            num_tcp_flows = tc.num_streams + 1
            num_udp_flows = 0
        post_stats = {}
        post_stats['icmp']= nat_pb.NatPbStats()
        post_stats['tcp']= nat_pb.NatPbStats()
        post_stats['udp']= nat_pb.NatPbStats()
        for pb in tc.nat_port_blocks:
            stats = pb.GetStats()
            post_stats[pb.ProtoName].Add(stats)
        if GlobalOptions.dryrun:
            return api.types.status.SUCCESS

        if post_stats['tcp'].InUseCount - tc.nat_pre_stats['tcp'].InUseCount != num_tcp_flows:
            api.Logger.error(f"NAT in use tcp count did not go up as expected {tc.nat_pre_stats['tcp'].InUseCount}:{post_stats['tcp'].InUseCount}:{num_tcp_flows}")
            return api.types.status.FAILURE

        if post_stats['udp'].InUseCount - tc.nat_pre_stats['udp'].InUseCount != num_udp_flows:
            api.Logger.error(f"NAT in use udp count did not go up as expected {tc.nat_pre_stats['udp'].InUseCount}:{post_stats['udp'].InUseCount}:{num_udp_flows}")
            return api.types.status.FAILURE

        if post_stats['tcp'].SessionCount - tc.nat_pre_stats['tcp'].SessionCount != num_tcp_flows:
            api.Logger.error(f"NAT session tcp count did not go up as expected {tc.nat_pre_stats['tcp'].SessionCount}:{post_stats['tcp'].SessionCount}:{num_tcp_flows}")
            return api.types.status.FAILURE

        if post_stats['udp'].SessionCount - tc.nat_pre_stats['udp'].SessionCount != num_udp_flows:
            api.Logger.error(f"NAT session udp count did not go up as expected {tc.nat_pre_stats['udp'].SessionCount}:{post_stats['udp'].SessionCount}:{num_udp_flows}")
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    # some issue running iperf in iota, previous test iperf flows still visible
    # inspite of clear flows. Cannot reproduce when running manually. For now
    # only clear flows after the last NAPT test case
    if tc.iterators.protocol == 'udp':
        return flow_utils.clearFlowTable(tc.workload_pairs)
    return api.types.status.SUCCESS
