#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.config.api as config_api
import apollo.config.objects.vnic as vnic
import apollo.config.objects.subnet as subnet
import vpc_pb2 as vpc_pb2

def TriggerConnectivityTestAll(proto="icmp", af="ipv4", pktsize=128, sec_ip_test_type="all"):
    wl_pairs = []
    for wl_type in [config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY, \
                    config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY]:
        for wl_scope in [config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET, \
                         config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET] :

            if wl_type == config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY and \
               wl_scope == config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET:
                continue
            wl_pairs += config_api.GetWorkloadPairs(wl_type, wl_scope)

    return TriggerConnectivityTest(wl_pairs, proto, af, pktsize, sec_ip_test_type)

def TriggerConnectivityTest(workload_pairs, proto, af, pktsize, sec_ip_test_type='none'):
    cmd_cookies = []
    resp = None
    if proto == 'icmp':
        cmd_cookies, resp = traffic_utils.pingWorkloads(workload_pairs, af, pktsize, sec_ip_test_type=sec_ip_test_type)
    elif proto in ['tcp','udp']:
        cmd_cookies, resp = traffic_utils.iperfWorkloads(workload_pairs, af, proto, pktsize, "1K", 1, 1)
    else:
        api.Logger.error("Proto %s unsupported" % proto)

    return cmd_cookies, resp

def VerifyConnectivityTest(proto, cmd_cookies, resp, expected_exit_code=0):
    if proto == 'icmp' or proto == 'arp':
        if traffic_utils.verifyPing(cmd_cookies, resp, expected_exit_code) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if proto in ['tcp','udp']:
        if traffic_utils.verifyIPerf(cmd_cookies, resp) != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

# Testcases which want to test connectivity after a trigger, through
# different protocol packets with multiple packet sizes can use this API.
def ConnectivityTest(workload_pairs, proto_list, ipaf_list, pktsize_list, expected_exit_code=0, sec_ip_test_type='none'):
    cmd_cookies = []
    resp = None
    for af in ipaf_list:
        for proto in proto_list:
            for pktsize in pktsize_list:
                cmd_cookies, resp = TriggerConnectivityTest(workload_pairs, proto, af, pktsize, sec_ip_test_type)
                return VerifyConnectivityTest(proto, cmd_cookies, resp, expected_exit_code)
    return api.types.status.SUCCESS

def GetWorkloadType(iterators=None):
    type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY

    if not hasattr(iterators, 'workload_type'):
        return type

    if iterators.workload_type == 'local':
        type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY
    elif iterators.workload_type == 'remote':
        type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY
    elif iterators.workload_type == 'igw':
        type = config_api.WORKLOAD_PAIR_TYPE_IGW_NAPT_ONLY

    return type

def GetWorkloadScope(iterators=None):
    scope = config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET

    if not hasattr(iterators, 'workload_scope'):
        return scope
    if iterators.workload_scope == 'intra-subnet':
        scope = config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET
    elif iterators.workload_scope == 'inter-subnet':
        scope = config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET
    elif iterators.workload_scope == 'inter-vpc':
        scope = config_api.WORKLOAD_PAIR_SCOPE_INTER_VPC

    return scope

def ConnectivityARPingTest(workload_pairs, args=None):
    # default probe count is 3
    probe_count = 3
    sent_probes = dict()
    if args == 'DAD':
        cmd_cookies, resp = traffic_utils.ARPingWorkloads(workload_pairs, False, True)
    elif args == 'Update':
        cmd_cookies, resp = traffic_utils.ARPingWorkloads(workload_pairs, True)
    else:
        cmd_cookies, resp = traffic_utils.ARPingWorkloads(workload_pairs)

    for pair in workload_pairs:
        wl = pair[0]
        cur_cnt = sent_probes.get(wl.node_name, 0)
        sent_probes.update({wl.node_name: cur_cnt + probe_count})

    return cmd_cookies, resp, sent_probes

def ConnectivityVRIPTest(proto='icmp', af='ipv4', pktsize=64,
        scope=config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET, args=None):

    cmd_cookies = []
    cmd = None
    # default probe count is 3
    probe_count = 3
    sent_probes = dict()

    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    naplesHosts = api.GetNaplesHostnames()
    vnics = []
    subnets = []
    for node in naplesHosts:
        vnics.extend(vnic.client.Objects(node))
        subnets.extend(subnet.client.Objects(node))

    if scope == config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET:
        for vnic1 in vnics:
            if vnic1.SUBNET.VPC.Type == vpc_pb2.VPC_TYPE_CONTROL:
                continue
            wl = config_api.FindWorkloadByVnic(vnic1)
            assert(wl)
            dest_ip = vnic1.SUBNET.GetIPv4VRIP()
            cmd = traffic_utils.PingCmdBuilder(wl, dest_ip, proto, af, pktsize, args, probe_count)
            api.Logger.info(f" VR_IP on {wl.node_name}: {cmd}")
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
            cmd_cookies.append(cmd)
            cur_cnt = sent_probes.get(wl.node_name, 0)
            sent_probes.update({wl.node_name: cur_cnt + probe_count})
    else:
        for vnic1 in vnics:
            if vnic1.SUBNET.VPC.Type == vpc_pb2.VPC_TYPE_CONTROL:
                continue
            wl = config_api.FindWorkloadByVnic(vnic1)
            assert(wl)
            for subnet1 in subnets:
                if subnet1.VPC.Type == vpc_pb2.VPC_TYPE_CONTROL:
                    continue
                if subnet1.Node != vnic1.Node:
                    continue
                if scope == config_api.WORKLOAD_PAIR_SCOPE_INTER_SUBNET and (vnic1.SUBNET.GID() == subnet1.GID()):
                    continue
                dest_ip = subnet1.GetIPv4VRIP()
                cmd = traffic_utils.PingCmdBuilder(wl, dest_ip, proto, af, pktsize, args, probe_count)
                api.Logger.info(f" VRIP on {wl.node_name}: {cmd} ")
                api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
                cmd_cookies.append(cmd)
                cur_cnt = sent_probes.get(wl.node_name, 0)
                sent_probes.update({wl.node_name: cur_cnt + probe_count})

    resp = api.Trigger(req)

    return cmd_cookies, resp, sent_probes
