#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.interface as interface
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify
import iota.test.iris.utils.naples_workloads as workloads
import iota.test.iris.utils.iperf as iperf


INTF_TEST_TYPE_OOB_1G       = "oob-1g"
INTF_TEST_TYPE_IB_100G      = "inb-100g"
INTF_TEST_TYPE_INT_MGMT     = "int-mgmt"
INTF_TEST_TYPE_HOST         = "host"

taggedWorkload = False

def getIntMgmtWorkloadPairs():
    pairs = []
    host_wls = workloads.GetIntMgmtHostWorkloads()
    naples_wls = workloads.GetIntMgmtNaplestWorkloads()
    for w1 in host_wls:
        for w2 in naples_wls:
            if w1.node_name == w2.node_name:
                pairs.append((w1,w2))
    return pairs

def getRemoteWorkloadPairs():
    pairs = []
    wpairs = api.GetRemoteWorkloadPairs()
    for w1, w2 in wpairs:
        if taggedWorkload:
            if w1.encap_vlan != 0 and w1.encap_vlan == w2.encap_vlan:
                pairs.append((w1,w2))
        else:
            if w1.encap_vlan == 0 and w2.encap_vlan == 0:
                pairs.append((w1,w2))

    return pairs

workloads_pairs_map = {
    INTF_TEST_TYPE_HOST   : getRemoteWorkloadPairs(),
    INTF_TEST_TYPE_OOB_1G : workloads.GetOobMgmtRemoteWorkloadPairs(),
    INTF_TEST_TYPE_IB_100G : workloads.GetInbandMgmtRemoteWorkloadPairs(),
    INTF_TEST_TYPE_INT_MGMT : getIntMgmtWorkloadPairs(),
}

def _get_workloads(tc):
    test_type = getattr(tc.args, "test-type", INTF_TEST_TYPE_HOST)
    global taggedWorkload
    if getattr(tc.iterators, 'vlantag', 'off') == 'on':
        taggedWorkload = True
    else:
        taggedWorkload = False

    pairs = workloads_pairs_map[test_type]
    return pairs[0][0], pairs[0][1]

def Setup(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS
    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.node_intfs = {}
    w1,w2 = _get_workloads(tc)
    tc.workloads = [w1, w2]
    #for node in tc.nodes:
    #    tc.node_intfs[node] = interface.GetNodeInterface(node)

    if getattr(tc.args, 'restart', False):
        ret = api.RestartNodes(tc.nodes)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Node restart failed")
            return api.types.status.FAILURE


    api.Logger.info("Setting driver features")
    if common.setup_features(tc) != api.types.status.SUCCESS:
        api.Logger.info("Setting driver features :Failed")
        return api.types.status.FAILURE

    api.Logger.info("Setting driver features : Success")

    if getattr(tc.args, 'capture_pcap', False):
        if common.start_pcap_capture(tc) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Trigger(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS

    # Determine where the commands will be run - host or Naples.
    test_type = getattr(tc.args, "test-type", INTF_TEST_TYPE_HOST)
    is_naples_cmd = True
    if test_type == INTF_TEST_TYPE_HOST:
        is_naples_cmd = False

    req1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    req2 = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    w1 = tc.workloads[0]
    w2 = tc.workloads[1]
    
    proto = getattr(tc.iterators, "proto", 'tcp')
    number_of_iperf_threads = getattr(tc.args, "iperfthreads", 1)
    pktsize = getattr(tc.iterators, "pktsize", 512)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')

    server_ip = w1.ip_address
    client_ip = w2.ip_address

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.interface, server_ip, w2.interface, client_ip)

    api.Logger.info("Starting Iperf(%s/%s pktsize=%d) test from %s"
                    % (proto, ipproto, pktsize, tc.cmd_descr))

    for i in range(number_of_iperf_threads):
        if proto == 'tcp':
            port = api.AllocateTcpPort()
        else:
            port = api.AllocateUdpPort()
 
        iperf_server_cmd = iperf.ServerCmd(port, naples = is_naples_cmd)
        api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name, iperf_server_cmd, background = True)

        iperf_client_cmd = iperf.ClientCmd(server_ip, port, time=10,
                                 proto=proto, jsonOut=True, ipproto=ipproto, num_of_streams=number_of_iperf_threads,
                                 pktsize=pktsize, client_ip=client_ip, naples = is_naples_cmd)
        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name, iperf_client_cmd)

    trig_resp1 = api.Trigger(req1)
    time.sleep(10)
    tc.resp = api.Trigger(req2)

    term_resp1 = api.Trigger_TerminateAllCommands(trig_resp1)

    return api.types.status.SUCCESS


def Verify(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS
    if getattr(tc.args, 'capture_pcap', False):
        ret = common.stop_pcap_capture(tc)
        if ret != api.types.status.SUCCESS:
            api.Logger.info("pcap caputre failed")
            return ret

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

        if cmd.exit_code != 0:
            api.Logger.error("Iperf client exited with error")
        if iperf.ConnectionTimedout(cmd.stdout):
            api.Logger.error("Connection timeout, ignoring for now")
            continue
        if iperf.ControlSocketClosed(cmd.stdout):
            api.Logger.error("Control socket cloned, ignoring for now")
            continue
        if iperf.ServerTerminated(cmd.stdout):
            api.Logger.error("Iperf server terminated")
            return api.types.status.FAILURE
        if not iperf.Success(cmd.stdout):
            api.Logger.error("Iperf failed", iperf.Error(cmd.stdout))
            return api.types.status.FAILURE

    return verify.driver_feature_verify(tc)

def Teardown(tc):
    #interface.RestoreIntMmgmtInterfaceConfig()
    return api.types.status.SUCCESS
