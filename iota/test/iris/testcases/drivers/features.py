#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.interface as interface
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify

def Setup(tc):
    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.node_intfs = {}
    for node in tc.nodes:
        tc.node_intfs[node] = interface.GetNodeInterface(node)

    if getattr(tc.args, 'restart', False):
        ret = api.RestartNodes(tc.nodes)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Node restart failed")
            return api.types.FAILURE

    test_type = getattr(tc.args, "test-type", interface.INTF_TEST_TYPE_HOST)
    ret = interface.ConfigureInterfaces(tc, test_type)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Set interface failed")
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

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (tc.intf1.Name(), tc.intf1.GetIP(), tc.intf2.Name(), tc.intf2.GetIP())
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))

    iperf_server_cmd = cmd_builder.iperf_server_cmd()
    tc.intf1.AddCommand(req, iperf_server_cmd, background = True)
    proto = getattr(tc.iterators, "proto", 'tcp')
    pktsize = getattr(tc.iterators, "pktsize", 512)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    iperf_client_cmd = cmd_builder.iperf_client_cmd(server_ip = tc.intf1.GetIP(),
                            proto=proto, pktsize=pktsize, ipproto=ipproto)
    tc.intf2.AddCommand(req, iperf_client_cmd)


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
