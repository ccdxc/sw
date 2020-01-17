#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.interface as interface
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify
import iota.test.iris.utils.naples_workloads as workloads
import iota.test.utils.ionic_utils as ionic_utils
 
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

    req1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    req2 = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    w1 = tc.workloads[0]
    w2 = tc.workloads[1]
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.interface, w1.ip_address, w2.interface, w2.ip_address)
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))

    proto = getattr(tc.iterators, "proto", 'tcp')

    number_of_iperf_threads = getattr(tc.args, "iperfthreads", 1)

    pktsize = getattr(tc.iterators, "pktsize", 512)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')

    for i in range(number_of_iperf_threads):
        if proto == 'tcp':
            port = api.AllocateTcpPort()
        else:
            port = api.AllocateUdpPort()

        iperf_server_cmd = cmd_builder.iperf_server_cmd(port = port)
        api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name, iperf_server_cmd, background = True)

        iperf_client_cmd = cmd_builder.iperf_client_cmd(server_ip = w1.ip_address, port = port,
                                 proto=proto, pktsize=pktsize, ipproto=ipproto)
        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name, iperf_client_cmd)

    trig_resp1 = api.Trigger(req1)
    trig_resp2 = api.Trigger(req2)
    term_resp1 = api.Trigger_TerminateAllCommands(trig_resp1)

    response = api.Trigger_AggregateCommandsResponse(trig_resp1, term_resp1)
    tc.resp = api.Trigger_AggregateCommandsResponse(response, trig_resp2)

    status = ionic_utils.checkForIonicError(w1.node_name)
    if status != api.types.status.SUCCESS:
        api.Logger.error("Error on iperf server")

    status = ionic_utils.checkForIonicError(w2.node_name)
    if status != api.types.status.SUCCESS:
        api.Logger.error("Error on iperf client")

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
            return api.types.status.FAILURE

    return verify.driver_feature_verify(tc)

def Teardown(tc):
    #interface.RestoreIntMmgmtInterfaceConfig()
    return api.types.status.SUCCESS
