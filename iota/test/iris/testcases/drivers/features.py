#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify

tc_count = 0

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()

    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.host_intfs = {}
    for node in tc.nodes:
        tc.host_intfs[node] = api.GetWorkloadNodeHostInterfaces(node)

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
    w1 = tc.workload_pairs[0][0]
    w2 = tc.workload_pairs[0][1]

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))

    iperf_server_cmd = cmd_builder.iperf_server_cmd()
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           iperf_server_cmd, background = True)
    proto = getattr(tc.iterators, "proto", 'tcp')
    pktsize = getattr(tc.iterators, "pktsize", 512)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    iperf_client_cmd = cmd_builder.iperf_client_cmd(server_ip = w1.ip_address,
                            proto=proto, pktsize=pktsize, ipproto=ipproto)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           iperf_client_cmd)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS


def Verify(tc):
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
