#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.testcases.drivers.common as common
import iota.test.apulu.testcases.drivers.cmd_builder as cmd_builder
import iota.test.apulu.testcases.drivers.verify as verify

INTF_TEST_TYPE_HOST         = "host"

def getRemoteWorkloadPairs():
    pairs = []
    wpairs = api.GetRemoteWorkloadPairs()
    for w1, w2 in wpairs:
        if w1.encap_vlan == w2.encap_vlan:
            pairs.append((w1,w2))

    return pairs

workloads_pairs_map = {
    INTF_TEST_TYPE_HOST   : getRemoteWorkloadPairs(),
}

def _get_workloads(tc):
    test_type = INTF_TEST_TYPE_HOST
    pairs = workloads_pairs_map[test_type]
    return pairs[0][0], pairs[0][1]

def Setup(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS
    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.node_intfs = {}
    w1,w2 = _get_workloads(tc)
    tc.workloads = [w1, w2]

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
    return api.types.status.SUCCESS
